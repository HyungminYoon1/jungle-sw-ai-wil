# SPT, Page Fault, Frame Claim

이 문서는 Project 3의 가장 기본 뼈대인 보조 페이지 테이블, page fault 처리, frame claim을 설명한다.

## 1. 왜 SPT부터 구현하는가

page fault가 발생했을 때 커널은 먼저 "이 fault를 복구할 수 있는가"를 판단해야 한다.

이 판단의 기준이 SPT다.

```text
fault_addr = 0x401234
  -> pg_round_down(fault_addr) = 0x401000
  -> SPT에서 0x401000 page 조회
  -> page가 있으면 복구 가능성 있음
  -> page가 없으면 잘못된 접근일 가능성 큼
```

SPT가 없으면 커널은 이 주소가 실행 파일에서 lazy load되어야 하는 page인지, stack growth 후보인지, 아예 잘못된 주소인지 알 수 없다.

## 2. SPT가 저장해야 하는 핵심 정보

구현 방식은 팀이 정하지만, 초심자는 최소한 다음 정보를 떠올릴 수 있어야 한다.

| 정보 | 필요한 이유 |
|------|------|
| user virtual page address | 어떤 page인지 식별 |
| page type | uninit, anon, file 중 어떤 동작을 할지 결정 |
| writable 여부 | write fault를 허용할지 판단 |
| frame 포인터 | 현재 물리 frame에 올라와 있는지 추적 |
| lazy load aux | 아직 로드되지 않은 page의 파일 정보 |
| swap 위치 | swap out된 page를 다시 가져올 위치 |
| mmap 파일 정보 | dirty page write-back에 필요 |

SPT의 key는 보통 page-aligned virtual address다.

## 3. 관련 함수

GitBook이 요구하는 기본 SPT 함수는 다음 세 개다.

```c
void supplemental_page_table_init (struct supplemental_page_table *spt);
struct page *spt_find_page (struct supplemental_page_table *spt, void *va);
bool spt_insert_page (struct supplemental_page_table *spt, struct page *page);
```

각 함수의 역할은 단순하다.

| 함수 | 역할 |
|------|------|
| `supplemental_page_table_init` | 새 process의 SPT 자료구조 초기화 |
| `spt_find_page` | 주소에 해당하는 page 찾기 |
| `spt_insert_page` | 새 page를 SPT에 넣되 중복은 거부 |

`spt_find_page`는 인자로 받은 `va`를 그대로 비교하면 안 된다. `0x401000`과 `0x401234`는 같은 page에 속할 수 있으므로, 내부에서 `pg_round_down()`을 적용하는 구조가 안전하다.

## 4. SPT lifecycle

SPT는 process address space를 설명하는 자료구조이므로 process lifecycle과 함께 움직인다.

| 시점 | 해야 할 일 | 주로 보는 함수 |
|------|------|------|
| 새 process 시작 | 빈 SPT 초기화 | `supplemental_page_table_init()` |
| executable load | ELF segment를 lazy page로 SPT에 등록 | `load_segment()`, `vm_alloc_page_with_initializer()` |
| page fault | SPT에서 page를 찾고 claim | `vm_try_handle_fault()`, `vm_claim_page()` |
| stack growth | 새 stack page를 SPT에 추가하고 즉시 claim | `vm_stack_growth()` |
| `fork()` | parent SPT 의미를 child SPT로 복사 | `supplemental_page_table_copy()` |
| `exec()` 성공 | 이전 address space의 SPT 자원 제거 후 새 executable SPT 구성 | `process_exec()`, `supplemental_page_table_kill()` |
| process exit | 남은 page, frame, aux, swap slot, mmap write-back 정리 | `process_exit()`, `supplemental_page_table_kill()` |

초심자는 "SPT entry를 언제 만들고 언제 지우는가"를 먼저 따라가면 VM 전체 흐름이 잘 보인다. page fault가 성공하려면 이미 SPT에 설명서가 있거나, stack growth처럼 새 설명서를 만들 수 있어야 한다.

## 5. Page fault 처리의 기본 흐름

Project 3에서는 `userprog/exception.c`의 `page_fault()`가 VM 계층의 `vm_try_handle_fault()`로 처리를 넘긴다.

큰 흐름은 다음과 같다.

```text
page_fault()
  -> fault address 확인
  -> user/kernel fault 정보 확인
  -> vm_try_handle_fault(f, addr, user, write, not_present)
       -> 잘못된 주소인지 검사
       -> SPT에서 page 찾기
       -> write 권한 검사
       -> page가 있으면 claim
       -> stack growth 후보면 stack 확장
       -> 처리 불가능하면 false
  -> true면 사용자 프로그램 재개
  -> false면 process 종료
```

여기서 중요한 것은 `page_fault()`가 성공적으로 반환하면 사용자 프로그램이 같은 명령을 다시 시도한다는 점이다. 커널이 page table mapping을 제대로 만들어 두었다면 두 번째 실행은 성공한다.

## 6. Project 2 user pointer validation과 달라지는 점

Project 2에서는 user pointer가 현재 page table에 없으면 대부분 잘못된 포인터로 보고 종료해도 됐다. Project 3에서는 이 기준만으로 부족하다.

예를 들어 syscall 인자로 받은 buffer가 아직 lazy page에 있거나, 정상적인 stack growth 후보라면 지금 pml4 mapping이 없어도 복구 가능한 접근일 수 있다.

```text
read(fd, buffer, size)
  -> buffer page가 아직 frame에 없음
  -> pml4_get_page()만 보면 실패
  -> VM에서는 page fault 또는 SPT/stack 기준으로 복구 가능성 확인 필요
```

따라서 user buffer 검증은 다음을 고려해야 한다.

- buffer가 여러 page에 걸칠 수 있으므로 page 단위로 확인한다.
- kernel address, NULL, permission violation은 여전히 거부한다.
- SPT에 있는 lazy page라면 claim될 수 있다.
- SPT에 없어도 stack growth 기준을 만족하면 새 page를 만들 수 있다.
- syscall 처리 중 kernel mode fault가 나면 `intr_frame->rsp`가 user stack pointer가 아닐 수 있으므로 syscall 진입 시점의 user `rsp`를 저장해 두는 방식이 필요할 수 있다.

즉 VM 이후의 user pointer validation은 "지금 매핑되어 있는가"가 아니라 "정상적으로 매핑되거나 확장될 수 있는가"를 묻는 방향으로 바뀐다.

## 7. Page fault에서 반드시 거를 것

다음 접근은 보통 복구하면 안 된다.

- `addr == NULL`
- kernel virtual address 접근
- not-present fault가 아닌데 일반 lazy loading처럼 처리하는 경우
- read-only page에 write하려는 경우
- SPT에도 없고 stack growth로도 볼 수 없는 주소

특히 writable 검사는 중요하다. 실행 코드 영역처럼 읽기 전용으로 매핑된 page에 쓰기를 시도하면 process를 종료해야 한다.

## 8. Frame claim이란 무엇인가

SPT에 page가 있다는 것은 "이 주소에 page가 있어야 한다"는 뜻이다. 하지만 아직 실제 물리 frame이 붙어 있다는 뜻은 아니다.

frame claim은 page에 실제 frame을 붙이고 page table mapping을 만드는 작업이다.

```text
SPT page
  -> frame 할당
  -> page->frame 연결
  -> frame->page 연결
  -> pml4_set_page()로 user page와 frame mapping
  -> swap_in() 또는 lazy load로 내용 채우기
```

## 9. 관련 함수

GitBook 기준으로 frame claim의 핵심 함수는 다음 세 개다.

```c
static struct frame *vm_get_frame (void);
bool vm_claim_page (void *va);
bool vm_do_claim_page (struct page *page);
```

| 함수 | 역할 |
|------|------|
| `vm_get_frame` | user pool에서 frame 하나를 가져온다 |
| `vm_claim_page` | va로 SPT page를 찾아 claim한다 |
| `vm_do_claim_page` | 실제 frame 연결과 page table mapping을 수행한다 |

`vm_claim_page()`는 public wrapper에 가깝다. 주소만 받고 SPT에서 page를 찾은 뒤 `vm_do_claim_page()`로 넘긴다.

`vm_do_claim_page()`는 이미 찾은 `struct page`를 실제 frame에 연결한다.

## 10. `vm_get_frame()`에서 주의할 점

사용자 page용 frame은 반드시 user pool에서 가져온다.

```c
palloc_get_page (PAL_USER);
```

초기 구현에서는 frame이 부족하면 `PANIC("todo")`처럼 표시할 수 있지만, 최종적으로는 eviction과 swap으로 연결해야 한다.

또한 frame table을 도입하면 새로 얻은 frame을 전역 frame list나 table에 등록해야 한다. 그래야 나중에 victim을 고를 수 있다.

## 11. `vm_do_claim_page()`의 순서

템플릿 구현에서는 보통 다음 순서로 작성하기 쉽다.

```text
1. vm_get_frame()으로 frame 얻기
2. page->frame = frame
3. frame->page = page
4. pml4_set_page(current->pml4, page->va, frame->kva, writable)
5. page 타입에 맞는 swap_in(page, frame->kva) 호출
6. 실패하면 중간 연결 정리
```

GitBook의 개념 흐름은 "frame을 얻고, 파일이나 swap에서 데이터를 채우고, page table entry를 연결한다"에 가깝다. 반면 KAIST Pintos 템플릿에서는 `page->operations->swap_in()`이 `page`와 `kva`를 받기 때문에, 먼저 frame과 page를 연결하고 `pml4_set_page()`까지 한 뒤 `swap_in()`을 호출하는 구현도 흔하다.

따라서 초심자는 순서 자체보다 성공 조건과 실패 정리를 기준으로 보면 된다.

- 함수가 성공으로 끝나면 page, frame, pml4 mapping, 실제 page 내용이 모두 일관되어야 한다.
- `pml4_set_page()`가 실패하면 frame과 page 연결을 되돌리고 frame을 반환해야 한다.
- `swap_in()` 또는 lazy load가 실패하면 이미 만든 pml4 mapping을 지우고 frame과 page 연결도 정리해야 한다.
- 실패 경로에서 page와 frame이 반쯤 연결된 상태로 남으면 같은 page fault가 반복되거나 나중에 eviction에서 잘못된 page를 건드릴 수 있다.

## 12. 흔한 실수

### Fault address를 그대로 SPT key로 쓰는 경우

같은 page 안의 다른 offset에서 fault가 나면 lookup이 실패한다. SPT key는 page 시작 주소로 정규화해야 한다.

### 중복 insert를 허용하는 경우

같은 virtual page가 SPT에 두 번 들어가면 page fault, munmap, process exit에서 어느 entry를 기준으로 정리할지 꼬인다.

### Frame만 할당하고 page table mapping을 안 하는 경우

frame에 내용을 채워도 `pml4_set_page()`가 안 되면 CPU는 여전히 주소 변환을 못 하므로 같은 fault가 반복된다.

### `PAL_USER`를 빼먹는 경우

user page를 kernel pool에서 할당하면 테스트가 불안정해진다.

### 실패 경로를 정리하지 않는 경우

frame을 얻은 뒤 mapping에 실패하면 frame을 반납하고 page/frame 포인터도 정리해야 한다.

## 13. 디버깅할 때 볼 것

page fault가 계속 반복되면 다음을 확인한다.

- fault address가 user address인가?
- `pg_round_down(fault_addr)`가 SPT에 있는가?
- `page->va`가 page-aligned 되어 있는가?
- page의 writable 값과 fault의 write 여부가 맞는가?
- `pml4_set_page()`가 true를 반환했는가?
- `page->frame`과 `frame->page`가 서로를 가리키는가?
- `swap_in()` 또는 lazy load가 성공했는가?

## 14. 확인 질문

1. Project 3에서 page fault가 항상 process 종료가 아닌 이유는 무엇인가?
2. SPT lookup 전에 `pg_round_down()`이 필요한 이유는 무엇인가?
3. `vm_claim_page()`와 `vm_do_claim_page()`의 차이는 무엇인가?
4. frame table이 나중에 eviction에 필요한 이유는 무엇인가?
5. page table mapping을 만든 뒤 page fault handler가 반환하면 어떤 일이 일어나는가?
6. Project 3에서 user pointer validation이 Project 2보다 어려워지는 이유는 무엇인가?
