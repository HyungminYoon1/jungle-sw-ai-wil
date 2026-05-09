# Pintos VM 학습 로드맵

이 문서는 KAIST Pintos GitBook의 `Project 3: Virtual Memory`를 처음 읽는 사람이 어떤 순서로 공부하면 좋은지 안내하는 로드맵이다.

Virtual Memory는 Project 1의 thread, Project 2의 user program 위에 올라간다. 따라서 VM만 따로 떼어 외우기보다, 사용자 프로그램이 실행되다가 page fault를 내고, 커널이 그 fault를 복구하거나 종료시키는 전체 흐름으로 이해해야 한다.

## 1. 이번 프로젝트의 한 문장 요약

Project 3의 목표는 사용자 프로그램에게 "필요한 메모리가 있는 것처럼" 보이게 만들고, 실제 물리 메모리는 커널이 뒤에서 필요한 순간에 할당, 로딩, 축출, 복구하게 만드는 것이다.

조금 더 풀면 다음 일을 구현한다.

- 실행 파일의 모든 내용을 처음부터 메모리에 올리지 않고, 실제 접근할 때 읽는다.
- 사용자 주소에서 page fault가 났을 때 죽일 fault인지 복구할 fault인지 판단한다.
- 각 user virtual page가 어디에서 왔는지 보조 페이지 테이블에 기록한다.
- 물리 프레임이 부족하면 어떤 page를 내보내고, 필요할 때 다시 가져온다.
- stack이 정상적으로 커지는 접근이면 새 stack page를 만든다.
- 파일을 memory map해서 일반 메모리처럼 읽고 쓸 수 있게 한다.
- `fork`, `exit`, `munmap`에서 page, frame, file, swap slot을 정리한다.

## 2. 추천 학습 순서

VM은 단어가 많지만, 공부 순서는 구현 의존성을 따르면 된다.

```text
기본 주소 개념
  -> page / frame / page table / SPT
  -> page fault 처리
  -> frame claim
  -> lazy loading
  -> anonymous page
  -> stack growth
  -> mmap과 file-backed page
  -> eviction과 swap
  -> fork 복사와 exit 정리
  -> 테스트 전략
```

이 순서가 중요한 이유는 SPT가 없으면 page fault를 복구할 근거가 없고, frame claim이 없으면 page를 실제 물리 메모리에 연결할 수 없으며, lazy loading이 안정되어야 사용자 프로그램 실행 경로가 살아나기 때문이다.

## 3. 문서 구성

| 파일 | 공부할 내용 |
|------|------|
| `01_vm_big_picture_and_terms.md` | VM 큰 그림, page, frame, page table, SPT, swap slot |
| `02_spt_page_fault_and_frame.md` | SPT 구현 방향, page fault 처리, frame claim |
| `03_lazy_loading_and_anon_pages.md` | uninit page, lazy loading, anonymous page |
| `04_stack_growth.md` | stack growth 판단 기준과 구현 흐름 |
| `05_mmap_and_file_backed_pages.md` | mmap, munmap, file-backed page, dirty write-back |
| `06_swap_and_eviction.md` | frame eviction, swap in/out, accessed/dirty bit |
| `07_fork_spt_copy_and_cleanup.md` | fork 시 SPT 복사, process exit 시 정리 |
| `08_vm_test_strategy.md` | 구현 순서별 테스트 전략 |
| `09_vm_review_questions.md` | 팀원이 함께 답해볼 확인 질문 |

## 4. 먼저 읽을 공식 문서

이 학습 자료는 아래 문서를 기준으로 풀어 쓴 보조 자료다. 구현 판단은 항상 저장소 공식 문서와 GitBook을 우선한다.

- `README.md`
- `docs/week11-12-collaboration.md`
- `docs/week11-12-implementation-plan.md`
- `깃북/04_project3_virtual_memory/01_introduction`
- `깃북/04_project3_virtual_memory/02_vm_management`
- `깃북/04_project3_virtual_memory/03_anon`
- `깃북/04_project3_virtual_memory/04_stack_growth`
- `깃북/04_project3_virtual_memory/05_memory_mapped_files`
- `깃북/04_project3_virtual_memory/06_swapping`
- `깃북/06_appendix/04_virtual_address`
- `깃북/06_appendix/05_page_table`

## 5. 공부할 때 잡아야 하는 관점

VM은 "자료구조 하나 만들기"가 아니다. 주소 하나에 대해 커널이 다음 질문에 답할 수 있게 만드는 프로젝트다.

```text
이 주소는 사용자 주소인가?
이 주소가 속한 page는 무엇인가?
그 page는 SPT에 등록되어 있는가?
등록되어 있다면 어디에서 내용을 가져와야 하는가?
쓰기 접근이면 writable page인가?
아직 frame이 없다면 어떤 frame을 붙일 것인가?
frame이 부족하면 무엇을 내보낼 것인가?
process가 끝나면 이 page와 관련된 자원을 누가 해제하는가?
```

이 질문에 대답하기 위해 SPT, frame table, swap table, page operations가 필요하다.

## 6. 초심자가 자주 헷갈리는 점

### Page와 frame은 다르다

page는 user virtual address 공간의 4KB 단위 영역이다. frame은 실제 물리 메모리의 4KB 공간이다.

사용자 프로그램은 page만 본다. 커널은 page가 어느 frame에 연결되어 있는지 관리한다.

### Page table과 SPT도 다르다

page table은 CPU가 주소 변환에 쓰는 하드웨어 형식의 자료구조다.

SPT는 커널이 page fault를 처리하기 위해 따로 들고 있는 설명서다. 예를 들어 "이 page는 실행 파일의 offset 4096부터 읽어 와야 한다" 같은 정보는 page table만으로 표현하기 어렵다.

### Page fault는 항상 버그가 아니다

Project 2에서는 page fault가 대부분 잘못된 접근이었다. Project 3에서는 일부 page fault가 정상 동작이다.

예를 들어 lazy loading page에 처음 접근하면 아직 물리 frame이 없으므로 page fault가 난다. 이때 커널이 파일에서 내용을 읽어 와 frame에 채우고 page table을 연결하면 사용자 프로그램은 같은 명령을 다시 실행해 계속 진행할 수 있다.

### `PAL_USER`가 중요하다

사용자 프로그램용 frame은 user pool에서 할당해야 한다. kernel pool을 써버리면 커널이 써야 할 메모리가 줄어들어 엉뚱한 실패가 생길 수 있다.

## 7. 구현 파일 큰 지도

| 파일 | 주로 보는 이유 |
|------|------|
| `pintos/include/vm/vm.h` | `struct page`, `struct frame`, `struct supplemental_page_table` |
| `pintos/vm/vm.c` | SPT, frame claim, page fault, stack growth, eviction |
| `pintos/include/vm/uninit.h`, `pintos/vm/uninit.c` | lazy page가 실제 page type으로 바뀌는 흐름 |
| `pintos/include/vm/anon.h`, `pintos/vm/anon.c` | anonymous page와 swap |
| `pintos/include/vm/file.h`, `pintos/vm/file.c` | file-backed page, mmap, munmap |
| `pintos/userprog/exception.c` | page fault entry point |
| `pintos/userprog/process.c` | ELF lazy loading, setup_stack, fork SPT copy |
| `pintos/userprog/syscall.c` | mmap/munmap syscall, user pointer validation |
| `pintos/include/threads/thread.h` | thread별 SPT와 user rsp 저장 |

## 8. 학습 체크포인트

아래 질문에 답할 수 있으면 다음 문서로 넘어가도 된다.

1. page와 frame의 차이를 설명할 수 있는가?
2. page table만으로 부족해서 SPT가 필요한 이유를 설명할 수 있는가?
3. page fault가 정상 동작일 수 있는 예를 들 수 있는가?
4. lazy loading이 실행 파일 로딩 방식을 어떻게 바꾸는지 말할 수 있는가?
5. process가 종료될 때 SPT가 왜 필요한지 설명할 수 있는가?
