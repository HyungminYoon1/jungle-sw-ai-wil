# Virtual Memory 큰 그림과 기본 용어

이 문서는 Project 3를 읽기 전에 반드시 알아야 하는 VM 기본 용어를 정리한다. 초심자는 여기서 단어를 정확히 잡아야 뒤의 page fault, lazy loading, mmap, swap이 덜 어렵다.

## 1. Virtual Memory는 왜 필요한가

사용자 프로그램은 자기에게 넓고 연속적인 메모리 공간이 있는 것처럼 실행된다. 하지만 실제 컴퓨터의 물리 메모리는 제한되어 있고, 여러 프로세스가 동시에 나누어 써야 한다.

Virtual Memory는 이 간극을 메워 주는 운영체제 기능이다.

```text
사용자 프로그램 관점
  "나는 0x400000부터 stack 끝까지 내 메모리를 갖고 있다."

커널 관점
  "그 주소 중 실제 접근한 일부만 물리 frame에 올려 두고,
   나머지는 파일이나 swap에서 필요할 때 가져오자."
```

즉 VM은 메모리를 실제보다 더 유연하게 보이게 만드는 장치다.

## 2. Page

page는 가상 주소 공간을 4KB 단위로 자른 것이다.

Pintos에서 page size는 `PGSIZE`이고 값은 4096바이트다.

```text
0x401000 ~ 0x401fff  -> 하나의 page
0x402000 ~ 0x402fff  -> 다음 page
```

page 시작 주소는 4096의 배수여야 한다. 이것을 page-aligned라고 한다.

어떤 주소가 속한 page의 시작 주소를 얻을 때는 `pg_round_down(va)`를 사용한다.

```text
va = 0x401234
pg_round_down(va) = 0x401000
```

SPT에서 page를 찾을 때는 보통 fault address 그대로가 아니라 `pg_round_down(fault_addr)`를 key로 쓴다. 같은 page 안의 어느 byte에서 fault가 나도 같은 page를 찾아야 하기 때문이다.

## 3. Frame

frame은 물리 메모리를 4KB 단위로 자른 것이다.

page는 가상 메모리의 단위이고, frame은 실제 메모리의 단위다.

```text
user virtual page 0x401000
  -> physical frame A
```

사용자 프로그램은 frame을 직접 알지 못한다. CPU와 커널이 page table을 통해 가상 주소를 실제 frame으로 바꿔 준다.

Pintos에서 사용자 프로그램용 frame은 반드시 user pool에서 가져와야 한다.

```c
palloc_get_page (PAL_USER);
```

`PAL_USER`를 빼면 kernel pool에서 가져올 수 있고, 그러면 VM 테스트에서 예상하지 못한 문제가 생길 수 있다.

## 4. Page table

page table은 CPU가 가상 주소를 물리 주소로 바꾸기 위해 사용하는 자료구조다.

Pintos x86-64에서는 이 page table을 `pml4`라고 부른다.

```text
user virtual address
  -> pml4 lookup
  -> physical frame
  -> 실제 메모리 접근
```

자주 쓰는 함수는 다음과 같다.

| 함수 | 의미 |
|------|------|
| `pml4_set_page(pml4, upage, kpage, writable)` | user page를 frame에 연결 |
| `pml4_get_page(pml4, uaddr)` | 주소가 이미 매핑되어 있는지 확인 |
| `pml4_clear_page(pml4, upage)` | page table에서 매핑 제거 |
| `pml4_is_dirty(pml4, vpage)` | page가 write된 적 있는지 확인 |
| `pml4_is_accessed(pml4, vpage)` | page가 접근된 적 있는지 확인 |

page table은 주소 변환에는 충분하지만, Project 3의 모든 정보를 담기에는 부족하다.

## 5. Supplemental Page Table

Supplemental Page Table, 줄여서 SPT는 커널이 page마다 추가 정보를 저장하는 자료구조다.

page table이 "이 가상 page가 지금 어떤 frame에 연결되어 있는가"를 알려준다면, SPT는 "이 가상 page는 원래 무엇이어야 하는가"를 알려준다.

예를 들어 다음 정보를 SPT에 둘 수 있다.

- 이 page의 user virtual address
- page 타입: uninit, anonymous, file-backed
- writable 여부
- lazy loading에 필요한 파일, offset, read bytes, zero bytes
- 현재 연결된 frame
- swap out되었다면 swap slot 위치
- mmap page라면 write-back해야 할 file 정보

page fault가 발생하면 커널은 SPT를 보고 다음을 판단한다.

```text
이 주소는 원래 존재해야 하는 page인가?
그렇다면 내용을 어디에서 가져와야 하는가?
파일인가, swap인가, zero page인가?
쓰기 접근이 허용되는가?
```

## 6. Swap slot

swap slot은 swap disk 안에 있는 4KB짜리 저장 공간이다.

물리 frame이 부족하면 커널은 어떤 page의 내용을 disk로 내보낼 수 있다. 이때 저장되는 곳이 swap slot이다.

```text
frame에 있던 anonymous page
  -> swap slot에 기록
  -> frame은 다른 page가 사용

나중에 그 page에 다시 접근
  -> page fault
  -> swap slot에서 frame으로 읽기
  -> swap slot 해제
```

swap slot은 미리 잡아 두면 안 된다. 실제로 page를 내보낼 때 할당하고, 다시 읽어 오면 해제한다.

## 7. Page type

KAIST Pintos 템플릿은 page를 타입별로 다룬다.

| 타입 | 의미 |
|------|------|
| `VM_UNINIT` | 아직 실제 타입으로 초기화되지 않은 lazy page |
| `VM_ANON` | 파일 backing이 없는 anonymous page |
| `VM_FILE` | 파일을 backing store로 가지는 file-backed page |
| `VM_PAGE_CACHE` | Project 4용이므로 지금은 무시 |

`struct page` 안에는 union이 있어서 한 시점에는 위 타입 중 하나의 세부 정보만 의미를 가진다.

여기서 "처음 데이터를 어디서 읽는가"와 "eviction 후 어디에서 복구하는가"를 구분해야 한다.

| page 예시 | 처음 내용을 채우는 곳 | 보통 page type | eviction 후 복구 기준 |
|------|------|------|------|
| ELF segment lazy page | 실행 파일 | `VM_ANON` | swap out되면 swap disk |
| stack page | 0으로 채움 | `VM_ANON` | swap out되면 swap disk |
| `mmap()` page | mmap 대상 파일 | `VM_FILE` | backing file |

실행 파일 lazy page는 page fault 때 파일에서 읽지만, 그 뒤 사용자가 수정할 수 있는 anonymous memory로 취급하는 것이 일반적이다. 반대로 `mmap()` page는 파일 자체가 backing store이므로 dirty page를 `munmap()`이나 exit 때 파일에 다시 써야 한다.

## 8. Page operation

page 타입마다 swap in, swap out, destroy 방식이 다르다.

anonymous page는 swap disk에 쓰고 읽어야 한다. file-backed page는 파일에서 읽고, dirty하면 파일에 다시 써야 한다.

그래서 `struct page_operations`가 있다.

```text
page->operations->swap_in
page->operations->swap_out
page->operations->destroy
```

이 방식은 C에서 함수 포인터로 page 타입별 동작을 나누는 구조다.

## 9. Page fault

page fault는 CPU가 어떤 가상 주소에 접근하려 했지만 현재 page table로는 접근할 수 없을 때 발생한다.

Project 3에서 page fault는 세 가지로 나누어 생각하면 좋다.

| 종류 | 처리 |
|------|------|
| lazy page에 처음 접근 | 파일 또는 zero로 frame을 채우고 복구 |
| swap out된 page에 접근 | swap disk에서 읽어 복구 |
| 잘못된 주소 접근 | process 종료 |

핵심은 모든 page fault를 무조건 죽이면 안 된다는 점이다. 반대로 모든 page fault를 복구하려고 해도 안 된다.

## 10. 전체 흐름 예시

사용자 프로그램이 실행 파일의 code/data page에 처음 접근한다고 하자.

```text
1. load()가 ELF segment를 읽는다.
2. 하지만 실제 파일 내용을 모두 메모리에 읽지는 않는다.
3. 각 page를 VM_UNINIT page로 SPT에 등록한다.
4. 사용자 프로그램이 해당 주소에 처음 접근한다.
5. page fault가 발생한다.
6. page_fault()가 vm_try_handle_fault()를 호출한다.
7. SPT에서 해당 page를 찾는다.
8. frame을 할당한다.
9. lazy_load_segment()가 파일에서 page 내용을 읽는다.
10. pml4_set_page()로 page와 frame을 연결한다.
11. page_fault()가 반환한다.
12. CPU가 fault가 났던 명령을 다시 실행하고 이번에는 성공한다.
```

이 흐름이 Project 3의 중심이다.

## 11. 확인 질문

1. page와 frame은 각각 어느 주소 공간의 단위인가?
2. SPT가 page table과 다른 정보를 가져야 하는 이유는 무엇인가?
3. page fault가 정상적인 상황일 수 있는 예시는 무엇인가?
4. `pg_round_down()`이 SPT lookup에서 중요한 이유는 무엇인가?
5. anonymous page와 file-backed page는 swap out할 때 무엇이 다른가?
