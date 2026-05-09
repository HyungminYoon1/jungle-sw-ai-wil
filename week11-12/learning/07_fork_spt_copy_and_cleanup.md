# fork의 SPT 복사와 자원 정리

이 문서는 Project 3에서 `fork()`와 process exit가 왜 어려워지는지 설명한다. VM에서는 page, frame, aux, file, swap slot 같은 자원이 서로 연결되어 있으므로 복사와 해제 기준이 중요하다.

## 1. Project 2의 fork와 Project 3의 fork

Project 2에서 fork는 부모의 실행 context와 page table을 복사하는 문제에 가까웠다.

Project 3에서는 부모의 주소 공간이 단순히 pml4 mapping만으로 설명되지 않는다. lazy page, swapped page, mmap page처럼 아직 frame에 없거나 파일과 연결된 page들이 SPT에 들어 있기 때문이다.

따라서 fork는 부모의 SPT를 자식의 SPT로 복사해야 한다.

```text
parent SPT
  page A: loaded anon
  page B: uninit lazy file
  page C: mmap file page

fork()
  -> child SPT에 같은 의미의 page들을 생성
```

## 2. `supplemental_page_table_copy()`

GitBook 기준 함수는 다음과 같다.

```c
bool supplemental_page_table_copy (struct supplemental_page_table *dst,
    struct supplemental_page_table *src);
```

이 함수는 부모 SPT의 각 page를 순회하면서 자식 SPT에 대응 page를 만들어야 한다.

하지만 page 상태에 따라 복사 방식이 다르다.

## 3. Uninit page 복사

uninit page는 아직 실제 frame이 없는 lazy page다.

복사할 때는 자식에도 같은 lazy loading 정보를 가진 page를 만들어야 한다.

```text
parent uninit page
  -> file, offset, read_bytes, zero_bytes, writable 정보 보유

child page 생성
  -> 같은 내용을 독립 aux로 보유
  -> GitBook 요구에 따라 즉시 claim
```

주의할 점은 aux를 얕은 복사하면 안 된다는 것이다. 부모와 자식이 같은 aux 포인터를 공유하면 한쪽 initialize/destroy가 다른 쪽 정보를 망가뜨릴 수 있다.

GitBook은 uninit page를 할당하고 즉시 claim해야 한다고 설명한다. 따라서 흐름은 "lazy metadata를 복사해 child page를 만들고, 곧바로 claim해서 실제 frame 내용을 준비한다"로 이해하면 된다. 즉 child 쪽에 같은 aux를 남겨 두는 것이 목적이 아니라, 부모의 주소 공간 의미를 child가 독립적으로 갖게 만드는 것이 목적이다.

## 4. Loaded anonymous page 복사

이미 frame에 올라온 anonymous page는 자식에게 같은 내용을 가진 새 frame을 만들어 주어야 한다.

```text
parent page -> parent frame contents

child page 생성
child frame claim
parent frame 내용을 child frame으로 memcpy
```

이렇게 해야 부모와 자식이 같은 가상 주소에 같은 초기 내용을 갖지만, 이후 수정은 서로 독립된다.

Copy-on-Write를 구현하지 않는 기본 범위에서는 frame을 공유하면 안 된다.

## 5. File-backed page와 mmap lifetime

초심자가 가장 많이 헷갈리는 구분은 실행 파일 lazy page와 `mmap()` file-backed page다.

| 구분 | 보통 page type | fork/exec에서 볼 점 |
|------|------|------|
| 실행 파일 lazy page | `VM_ANON` | `fork()` 시 주소 공간 복사 대상 |
| `mmap()` page | `VM_FILE` | `munmap()`/exit 때 dirty write-back, `exec()` 뒤에는 이전 주소 공간과 함께 제거 |

mmap page는 파일 참조, offset, read bytes, zero bytes, writable, mapping 범위 같은 정보를 유지해야 한다.

`fork()`가 부모의 SPT를 복사하는 시점에는 mmap page도 부모 주소 공간의 일부이므로 복사 정책을 정해야 한다. 하지만 `exec()`가 성공하면 child는 새 executable의 주소 공간으로 바뀌므로, 이전에 fork로 복사되었던 mapping은 더 이상 보이면 안 된다. `mmap-inherit` 테스트는 이 지점을 확인한다.

파일 object를 공유할지 `file_reopen()`으로 독립 참조를 만들지는 구현 정책과 Pintos file layer의 lifetime을 함께 고려해야 한다. 중요한 것은 parent의 `close()`나 `munmap()`이 child mapping을 깨지 않게 하는 것이다.

## 6. Swapped page 복사

부모 page가 swap out되어 frame에 없을 수 있다.

이 경우 복사 전략은 더 신중해야 한다.

가능한 접근은 다음과 같다.

- 부모 page를 swap in한 뒤 child frame에 복사한다.
- swap slot 내용을 child의 별도 swap slot이나 frame으로 복제한다.

초심자 구현에서는 먼저 fork 관련 테스트가 어느 단계에서 필요한지 보고, lazy/loaded path를 안정화한 뒤 swap-fork를 다루는 편이 좋다.

## 7. Process exit와 SPT kill

프로세스가 종료되면 그 프로세스가 가진 모든 VM 자원을 정리해야 한다.

GitBook 기준 함수는 다음과 같다.

```c
void supplemental_page_table_kill (struct supplemental_page_table *spt);
```

이 함수는 SPT의 모든 page를 순회하며 page type에 맞는 destroy를 호출해야 한다.

```text
process_exit()
  -> supplemental_page_table_kill(&thread_current()->spt)
  -> pml4_destroy()
  -> 남은 process 자원 정리
```

정확한 호출 순서는 현재 코드와 템플릿을 따라야 하지만, SPT가 page별 VM 자원 정리의 중심이라는 점은 같다.

정리 책임은 다음처럼 나누어 생각하면 덜 헷갈린다.

| 단계 | 주 책임 | 예시 |
|------|------|------|
| `supplemental_page_table_kill()` | SPT를 순회하며 각 page destroy 호출 | uninit/anon/file page별 정리 진입 |
| page type `destroy()` | page type 내부 자원 정리 | aux 해제, dirty mmap write-back, file reference close, swap slot 해제 |
| pml4 정리 | 실제 page table 제거 | `pml4_destroy()` 또는 mapping clear |
| frame table 정리 | frame table entry와 page/frame 연결 정리 | frame list 제거, `palloc_free_page()` |

GitBook은 SPT kill 안에서 pml4나 palloc memory 자체를 직접 걱정하지 않아도 된다고 설명한다. 하지만 팀이 frame table을 직접 운영한다면, page destroy와 pml4 cleanup 뒤 frame table entry가 남지 않도록 별도 책임자를 정해야 한다.

주의할 점은 같은 frame을 두 번 해제하지 않는 것이다. KAIST Pintos의 `pml4_destroy()`는 page table을 내려가며 매핑된 물리 page도 해제하는 경로를 포함한다. 따라서 frame table 쪽에서도 `palloc_free_page()`를 직접 호출한다면, 팀 구현에서 "실제 frame free는 어느 함수가 담당하는가"를 하나로 정하고 다른 경로는 mapping 제거와 metadata 정리에만 집중해야 한다.

## 8. Destroy 함수의 역할

page type마다 destroy에서 정리해야 할 것이 다르다.

| page type | 정리할 수 있는 것 |
|------|------|
| `VM_UNINIT` | 아직 사용되지 않은 aux |
| `VM_ANON` | frame 연결, swap slot |
| `VM_FILE` | dirty write-back, file reference |

`vm_dealloc_page()`가 page struct 자체를 free하는 흐름이라면, 각 destroy 함수는 page type 내부 자원만 정리하는 책임을 갖는 것이 자연스럽다.

## 9. mmap cleanup

process가 `munmap()`을 직접 호출하지 않고 종료해도 모든 mmap은 해제되어야 한다.

즉 `process_exit()` 또는 SPT kill 과정에서 mmap file-backed page가 dirty하면 파일에 write-back해야 한다.

```text
exit
  -> mmap page dirty 확인
  -> dirty page write-back
  -> SPT entry 제거
  -> file close
```

명시적 `munmap()`과 exit cleanup이 같은 원칙을 공유해야 중복 버그가 줄어든다.

## 10. Frame cleanup

page가 frame을 가지고 있다면 cleanup 시 다음 연결을 끊어야 한다.

```text
page->frame = NULL
frame->page = NULL
pml4 mapping 제거
frame을 palloc_free_page()로 반환
frame table에서 제거
```

구현에 따라 일부는 `pml4_destroy()`가 처리할 수 있지만, frame table을 직접 운영한다면 frame table entry 정리가 빠지지 않아야 한다.

## 11. Swap slot cleanup

process가 종료될 때 어떤 anonymous page가 swap out된 상태라면, 그 swap slot은 더 이상 필요 없다.

따라서 SPT kill 중 page가 swap slot을 소유하고 있으면 bitmap에서 해제해야 한다.

swap slot을 해제하지 않으면 process가 사라졌는데도 swap 공간이 계속 사용 중으로 남는다.

## 12. 흔한 실수

### Parent와 child가 같은 aux 포인터를 공유하는 경우

한쪽 page가 initialize되며 aux를 free하면 다른 쪽은 dangling pointer를 갖게 된다.

### Loaded page를 SPT entry만 복사하고 frame 내용을 복사하지 않는 경우

child가 같은 주소를 읽을 때 부모와 같은 값을 보지 못한다.

### Exit에서 uninit page aux를 해제하지 않는 경우

한 번도 접근하지 않은 lazy page는 initialize되지 않는다. 그래도 aux는 해제되어야 한다.

### Dirty mmap page를 exit에서 write-back하지 않는 경우

`munmap()` 테스트는 통과해도 `mmap-exit` 계열 테스트에서 실패할 수 있다.

### Swap slot을 process 종료 시 해제하지 않는 경우

장기 테스트에서 swap slot 누수가 생긴다.

## 13. 확인 질문

1. fork에서 pml4만 복사하면 Project 3에서 부족한 이유는 무엇인가?
2. uninit page의 aux를 깊은 복사해야 하는 이유는 무엇인가?
3. Copy-on-Write를 구현하지 않는다면 loaded anon page는 어떻게 복사해야 하는가?
4. process exit에서 dirty mmap page는 어떻게 처리해야 하는가?
5. swap out된 page를 가진 process가 종료되면 swap slot은 어떻게 해야 하는가?
