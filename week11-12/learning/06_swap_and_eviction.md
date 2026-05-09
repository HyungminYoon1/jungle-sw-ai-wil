# Swap과 Eviction

이 문서는 물리 메모리 frame이 부족할 때 page를 내보내고 다시 가져오는 eviction과 swap을 설명한다.

## 1. 왜 eviction이 필요한가

물리 메모리는 제한되어 있다. 사용자 프로그램들이 많은 page를 실제로 접근하면 user pool의 frame이 부족해질 수 있다.

이때 커널은 둘 중 하나를 선택해야 한다.

```text
나쁜 선택
  -> frame이 없으니 무조건 실패 또는 panic

VM의 선택
  -> 지금 덜 필요한 frame을 골라 비우고 재사용
```

frame을 비우기 위해 그 안의 page 내용을 다른 곳에 저장하거나 버리는 과정을 eviction이라고 한다.

## 2. Frame table

victim frame을 고르려면 현재 어떤 frame들이 사용 중인지 알아야 한다. 이를 위해 frame table이 필요하다.

frame table의 각 entry는 최소한 다음 정보를 가진다.

- frame의 kernel virtual address
- 현재 frame을 차지한 page
- frame list/hash 연결용 element
- eviction 정책에 필요한 상태

```text
frame table
  frame A -> page 0x401000
  frame B -> page 0x7ffff000
  frame C -> page 0x500000
```

frame table은 보통 시스템 전체 자원이므로 전역 자료구조로 둔다. 여러 thread가 접근할 수 있으므로 lock도 고려해야 한다.

## 3. Eviction의 기본 흐름

`vm_get_frame()`이 새 frame을 얻으려 했는데 user pool이 비어 있다고 하자.

그때 흐름은 다음과 같다.

```text
1. frame table에서 victim frame 선택
2. victim frame에 연결된 page 확인
3. page type에 맞게 swap_out(page) 호출
4. victim page의 pml4 mapping 제거
5. victim page는 "메모리에 없음" 상태가 됨
6. 비워진 frame을 새 page에 재사용
```

중요한 점은 eviction 후에도 SPT entry는 남아 있어야 한다는 것이다. 나중에 그 page에 다시 접근하면 page fault가 발생하고, SPT를 통해 swap in할 수 있어야 한다.

구현할 때는 pml4 mapping 제거를 누가 책임지는지 한 곳으로 정해야 한다. `vm_evict_frame()`에서 지울 수도 있고, 각 page type의 `swap_out()` 안에서 지울 수도 있다. 두 군데에 흩어져 있으면 중복 정리나 누락을 찾기 어려워진다.

## 4. Victim selection

어떤 frame을 내보낼지는 page replacement policy가 결정한다.

간단한 정책은 FIFO나 clock algorithm이다.

초심자는 clock algorithm을 다음처럼 이해하면 된다.

```text
frame들을 원형으로 둘러본다.
accessed bit가 1이면 최근 사용된 page로 보고 0으로 내린 뒤 지나간다.
accessed bit가 0이면 victim 후보로 고른다.
```

이 정책은 최근 접근된 page를 바로 내보내지 않으려는 단순한 근사다.

## 5. Accessed bit

CPU는 page를 읽거나 쓰면 page table entry의 accessed bit를 1로 만든다.

운영체제는 이 bit를 보고 최근 사용 여부를 추정할 수 있다.

```text
accessed == true
  -> 최근 사용된 page일 가능성 큼

accessed == false
  -> victim으로 고려 가능
```

Pintos에서는 `pml4_is_accessed()`와 `pml4_set_accessed()`를 사용할 수 있다.

## 6. Dirty bit

CPU는 page에 write가 발생하면 dirty bit를 1로 만든다.

dirty bit는 특히 file-backed page에서 중요하다. file-backed page는 clean이면 backing file에서 다시 읽을 수 있고, dirty이면 파일에 변경 내용을 반영해야 하기 때문이다.

```text
anonymous page
  -> dirty 여부와 무관하게 eviction 시 현재 내용을 swap disk에 저장해야 함

dirty file-backed page
  -> backing file에 write-back해야 함

clean file-backed page
  -> 파일에서 다시 읽으면 되므로 write-back이 필요 없을 수 있음
```

Pintos에서는 `pml4_is_dirty()`와 `pml4_set_dirty()`를 사용할 수 있다.

## 7. Alias 문제

Pintos에서는 사용자 virtual page와 커널 virtual address가 같은 frame을 가리키는 alias 관계가 생길 수 있다.

CPU는 실제 접근에 사용된 page table entry의 accessed/dirty bit만 갱신한다.

즉 커널이 user data를 kernel virtual address로만 만지면 user page의 dirty bit가 기대대로 안 켜질 수 있다.

해결 방향은 둘 중 하나다.

- user page와 kernel alias 양쪽 bit를 모두 확인한다.
- 커널이 사용자 데이터를 가능한 user virtual address로 접근하게 설계한다.

프로젝트 구현에서는 팀이 선택한 접근법을 일관되게 유지해야 한다.

## 8. Anonymous page swap

anonymous page는 backing file이 없다. 따라서 eviction할 때 내용을 swap disk에 저장해야 한다.

여기서 dirty bit를 file-backed page처럼 해석하면 안 된다. anonymous page는 clean으로 보이더라도 파일에서 다시 읽어 복구할 원본이 없다. eviction하려면 현재 frame 내용을 swap slot에 저장해야 한다.

### swap out

```text
anon_swap_out(page)
  -> swap bitmap에서 빈 slot 찾기
  -> frame 내용을 swap disk slot에 기록
  -> page->anon에 slot 정보 저장
  -> pml4 mapping 제거
  -> frame 연결 해제
```

### swap in

```text
anon_swap_in(page, kva)
  -> page->anon에 저장된 slot 확인
  -> swap disk에서 kva로 읽기
  -> swap slot 해제
  -> page는 다시 메모리에 있음
```

swap slot은 page가 실제로 swap out될 때 할당하고, swap in되면 해제해야 한다.

## 9. File-backed page swap

file-backed page는 backing file이 있다.

따라서 eviction 시 dirty하면 파일에 기록하고, clean하면 그냥 버려도 나중에 파일에서 다시 읽을 수 있다.

```text
file_backed_swap_out(page)
  -> dirty 확인
  -> dirty하면 file_write_at()
  -> write-back했다면 dirty bit clear
  -> mapping 제거
  -> frame 연결 해제

file_backed_swap_in(page, kva)
  -> file_read_at()
  -> 나머지 zero fill
```

주의할 점은 두 가지다.

- mmap page의 마지막 page에서 파일 길이를 넘어선 zero 영역은 파일에 쓰면 안 된다.
- file-backed page를 파일에 write-back했다면 dirty bit를 정리해야 한다. 그렇지 않으면 이미 반영한 변경을 다음 eviction이나 destroy에서 다시 dirty로 판단할 수 있다.

## 10. Swap table

swap table은 swap disk의 어떤 slot이 사용 중인지 추적한다.

보통 bitmap이 잘 맞는다.

```text
slot 0: used
slot 1: free
slot 2: used
```

swap disk는 block device이므로 page 하나를 여러 sector로 나누어 읽고 써야 한다. `PGSIZE`가 4096이고 block sector가 512바이트라면 page 하나는 8 sector다.

## 11. Swap full이면 어떻게 하는가

GitBook은 스왑 슬롯을 새로 할당할 수 없는데 어떤 frame도 축출할 수 없다면 kernel panic을 허용한다.

실제 OS는 더 복잡한 정책을 쓰지만, Pintos Project 3 범위에서는 swap full을 복구하는 일반 정책까지 만들 필요는 없다.

## 12. 흔한 실수

### Eviction 후 SPT entry까지 지우는 경우

SPT entry를 지우면 나중에 page fault가 났을 때 복구할 근거가 사라진다. eviction은 "메모리에서 내보내기"이지 "page 자체 삭제"가 아니다.

### Swap slot을 미리 예약하는 경우

swap slot은 lazy하게 할당해야 한다. process 시작 시점에 page마다 swap slot을 잡아 두면 안 된다.

### Swap in 후 slot을 해제하지 않는 경우

slot 누수가 생겨 나중에 swap full이 빨리 발생한다.

### Dirty 확인 없이 file-backed page를 쓰는 경우

clean page는 write-back할 필요가 없다. dirty page만 파일에 기록해야 한다.

### Write-back 뒤 dirty bit를 그대로 두는 경우

이미 파일에 반영한 page를 계속 dirty로 판단해 중복 write-back하거나 cleanup 경로에서 상태를 잘못 해석할 수 있다.

### pml4 mapping을 제거하지 않는 경우

eviction 후에도 page table이 옛 frame을 가리키면, frame 재사용 후 심각한 메모리 오염이 생긴다.

## 13. 확인 질문

1. frame table이 없으면 eviction에서 어떤 문제가 생기는가?
2. eviction 후에도 SPT entry가 남아 있어야 하는 이유는 무엇인가?
3. anonymous page와 file-backed page의 swap out 차이는 무엇인가?
4. accessed bit는 victim selection에 어떻게 쓰이는가?
5. swap slot은 언제 할당하고 언제 해제해야 하는가?
6. file-backed page를 write-back한 뒤 dirty bit를 정리해야 하는 이유는 무엇인가?
