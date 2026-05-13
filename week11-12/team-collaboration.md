# 팀 협업 룰 (Week 11-12)

> 작성 시점: 주차 시작 시
>
> 필요 시 언제든 팀 논의를 거쳐 수정할 수 있습니다.

---

## 1. 일정·만남

| 항목 | 우리 팀 규칙 |
|------|--------------|
| 출근 시간 | 팀 합의 후 확정 |
| 스크럼 미팅 | 매일 오전, 전날 통과/실패 테스트와 오늘 공동 세션 목표 공유 |
| 점심 식사 | 팀 합의 후 확정 |
| 저녁 식사 | 팀 합의 후 확정 |
| 코어타임 | VM 핵심 설계, 실패 테스트, PR 리뷰 중심으로 운영 |
| 코칭실 사용 | page fault, mmap, swap처럼 막힌 주제는 코칭실 예약 후 공동 점검 |
| PintOS 기초 특강 | 금요일 오전 10시, Virtual Memory |
| 퀴즈 | 화요일 오후 2시-3시, PintOS/C언어 |
| 첫날 제출 | 2026-05-08(금) 자정까지 팀 GitHub Projects 주소와 팀 repository 주소 제출 |
| 주간 공유 발표 | 2026-05-21(목) 오전 10시. 팀 발표 자료와 노트북은 1개로 준비하고 개인별 2분 발표 |
| 최종 제출 | 2026-05-21(목) 정오까지 주간 공유 발표 자료와 WIL 제출 |
| 차주 발제 | 2주차 목요일 오후 1시 |
| 운영진 티타임 | 2주차 목요일 오후 3시 |


## 2. 소통 방식

| 항목 | 우리 팀 규칙 |
|------|--------------|
| 연락 채널 | 슬랙, 카톡 |
| 응답 기준 | 당일 확인 원칙, 막힌 테스트는 실시간 공유 |
| 긴급 연락 | 전화통화 |
| 테스트 실패 공유 | 실패 테스트명, 재현 명령, 기대 결과, 실제 결과, 최근 수정 commit, 의심 함수, 다음 가설을 함께 공유 |


## 3. 작업·진행 관리

| 항목 | 우리 팀 규칙 |
|------|--------------|
| GitHub Projects | Project 3 학습, 구현, 테스트, 발표 자료, 회귀 테스트 일정 관리 |
| 진행 상황 공유 | 슬랙 + 스크럼 미팅 + 월·화 4인 페어 프로그래밍 + 수요일부터 2명:2명 페어 프로그래밍 + 코어타임 PR 리뷰 |
| 이슈·태스크 관리 | SPT, frame claim, lazy loading, page fault, stack growth, SPT copy/kill, mmap/munmap, eviction/swap, 회귀 테스트, Extra COW로 분리 |
| 테스트 관리 | 통과/실패 테스트 목록과 재현 명령을 매일 갱신 |
| 회귀 관리 | Project 2 User Programs, Threads alarm/priority, filesys/base 회귀 테스트를 별도 묶음으로 관리 |


## 4. 코드·리뷰

| 항목 | 우리 팀 규칙 |
|------|--------------|
| 브랜치 전략 | `team`에서 공동 작업 후 PR로 `dev`에 병합 -> 검증된 `dev`만 PR로 `main`에 병합 |
| PR 리뷰 | 관련 테스트, 수정 파일/함수, 구현 의도, 자원 lifecycle, 회귀 가능성을 함께 설명 |
| 머지 기준 | 관련 테스트 통과 + Driver와 공동 검토자가 구현 의도 설명 가능 + Driver가 아니었던 팀원 1명 이상 리뷰 |
| `main` branch | 팀 최종 결과물 branch로 유지, 직접 push 금지 |
| 충돌 해결 | 충돌 파일 담당자가 설명 후 팀 논의로 해결 |


## 5. 공통 모듈 설계 원칙

10주차에는 업무 분장 전 GPT가 생성한 공통 skeleton 위에서 구현이 진행되면서, 필요한 것보다 많은 helper와 interface가 생겼다. 11~12주차에는 같은 문제를 줄이기 위해 아래 원칙을 따른다.

- 공통 모듈을 만들기 전에 팀이 먼저 필요한 함수와 구조체를 최소 단위로 정한다.
- 각 공통 함수는 "누가 호출하는지", "무엇을 입력으로 받는지", "실패 시 무엇을 반환하거나 정리하는지"를 PR에 적는다.
- AI는 공통 모듈 초안 생성보다 설계 누락, 실패 케이스, 코드 리뷰 체크리스트 확인에 사용한다.
- 설명할 수 없는 helper는 병합하지 않는다.
- 테스트 하나를 통과시키기 위한 임시 helper라면 임시 범위와 제거 계획을 남긴다.


## 6. 작업 진행 기준

작업은 담당자별 장기 분업이 아니라 Virtual Memory 구현 흐름과 테스트 묶음을 기준으로 함께 순차 진행한다. 2026-05-11(월)과 2026-05-12(화)는 4명이 한 화면에서 페어 프로그래밍으로 진행했고, 2026-05-13(수)부터는 2명:2명으로 나누어 페어 프로그래밍을 진행한다. 4인 세션에서는 Driver 1명과 공동 검토자 3명을 두고, 2인 페어에서는 Driver 1명과 Navigator 1명을 두며, 역할은 30-60분 단위로 교체한다.

2026-05-08(금)은 개인 학습과 환경 세팅, 2026-05-09(토)과 2026-05-10(일)은 `fork()`와 VM 전체 흐름을 개인적으로 학습하는 날로 두고, 실제 VM 코드 공동 구현은 2026-05-11(월)부터 2026-05-20(수)까지 진행한다. 2026-05-17(일)은 공동 구현 범위에서 제외하고, 2026-05-21(목)은 최종 발표와 제출 정리에 집중한다.

### 6.1 매일 공통 루틴

| 구간 | 할 일 | 산출물 |
|------|------|------|
| 시작 30분 | 전날 diff, 실패 테스트, panic log, `.output` 확인. 월요일 첫 시작은 공유된 User Programs 통과 commit/branch 고정과 smoke test로 대체 | 오늘 첫 번째 실패 지점 1개 확정 |
| 학습 60-90분 | GitBook과 `local/week11-12_study_docs`에서 오늘 구현 범위만 읽기 | 팀원이 답해야 할 질문 3-5개 정리 |
| 구현 세션 1 | 월·화는 Driver 1명이 작은 함수 단위로 작성하고 3명이 설계·cleanup·테스트 조건 확인. 수요일부터는 각 페어가 Driver/Navigator로 나누어 같은 목표를 병렬 진행 | 빌드 가능한 작은 변경 |
| 테스트 루프 | 가까운 테스트 1-3개 실행 후 같은 묶음으로 확장 | 통과/실패 테스트와 재현 명령 기록 |
| 구현 세션 2 | Driver 교체 후 같은 실패 지점을 이어서 처리하고, 2명:2명 운영 시 페어 간 변경 범위와 실패 지점을 공유 | PR 후보 변경 묶음 |
| 종료 30분 | 바뀐 구조체 필드, 자원 소유권, 남은 실패 원인 정리 | PR 설명/WIL/이슈에 남길 문장 |

### 6.2 현재 구현 기준선과 날짜별 학습·구현 계획

2026-05-13 현재 git 이력 기준으로 실제 구현된 범위와 아직 골격인 범위를 먼저 고정한다. 이 표는 테스트 통과를 단정하지 않는다. 실제 테스트 이름, 재현 명령, 통과/실패 결과는 실행한 뒤 PR, 이슈, WIL에 따로 기록한다.

- 2026-05-11 실제 구현: SPT 해시 테이블 구조, hash callback, `supplemental_page_table_init()`, `spt_find_page()`, `spt_insert_page()`, `spt_remove_page()` 기초 구현.
- 2026-05-12 실제 구현: `vm_alloc_page_with_initializer()`, `struct page.writable`, `vm_get_frame()`, `vm_claim_page()`, `vm_do_claim_page()`, VM용 첫 stack page claim, `uninit_destroy()`의 aux 기본 해제.
- 현재 미구현 또는 골격: `lazy_load_segment()` aux 구성과 file read, `vm_try_handle_fault()` 검증/claim 연결, `vm_stack_growth()`, `supplemental_page_table_copy()`, `supplemental_page_table_kill()`, frame table/eviction, anon swap, file-backed page, `mmap()`/`munmap()`.

| 날짜 | 핵심 목표 | 작성/수정 모듈 | 대표 함수·구조체 | 상태/산출물 |
|------|------|------|------|------|
| 2026-05-09(토) | 개인 학습: `fork()`와 Project 2 process 흐름 복습 | 코드 수정 없음. `pintos/userprog/process.c`, `pintos/userprog/syscall.c`, `pintos/include/threads/thread.h` 읽기 | `process_fork()`, `__do_fork()`, `process_exec()`, `process_exit()`, `process_wait()`, fd table, child status 구조 | fork에서 register, fd table, address space가 각각 어디서 복제되는지 질문 목록 작성 |
| 2026-05-10(일) | 개인 학습: VM 큰 흐름, SPT, lazy loading, page fault 예습 | 코드 수정 없음. `10_pintos_vm_flow_mermaid.md`, GitBook Project 3, `pintos/vm/*` 읽기 | `vm_alloc_page_with_initializer()`, `vm_try_handle_fault()`, `vm_do_claim_page()`, `lazy_load_segment()`, `vm_stack_growth()` | SPT, page, frame, pml4의 역할 차이와 fault 복구 흐름 질문 목록 작성 |
| 2026-05-11(월) | 공유된 통과 commit 고정 후 SPT 기본 구조 정리 | `pintos/include/vm/vm.h`, `pintos/vm/vm.c` | `struct supplemental_page_table`, `struct page`, `struct frame`, `supplemental_page_table_init()`, `spt_find_page()`, `spt_insert_page()`, `spt_remove_page()` | 실제 구현 완료. SPT key가 `pg_round_down(va)` 기준임을 설명하고 중복 insert/삭제 경로를 기록 |
| 2026-05-12(화) | VM core와 첫 stack page 구현 | `pintos/include/vm/vm.h`, `pintos/vm/vm.c`, `pintos/vm/uninit.c`, `pintos/userprog/process.c` | `struct page.writable`, `uninit_destroy()`, `vm_get_frame()`, `vm_claim_page()`, `vm_do_claim_page()`, `setup_stack()` | 실제 구현 완료. 단, lazy loading과 page fault handler가 아직 연결되지 않아 사용자 프로그램 실행 검증은 다음 단계에서 진행 |
| 2026-05-13(수) | ELF lazy loading과 page fault 진입 경로 복구 | `pintos/userprog/process.c`, `pintos/userprog/exception.c`, `pintos/vm/vm.c`, `pintos/vm/anon.c` | lazy load aux 구조체, `load_segment()`, `lazy_load_segment()`, `anon_initializer()`, `vm_try_handle_fault()`, `page_fault()` | 진행 목표. aux 생성/해제, file read/zero fill, SPT lookup, writable 검사, claim 성공/실패 경로를 한 흐름으로 연결 |
| 2026-05-14(목) | page fault 검증 안정화와 stack growth 준비 | `pintos/userprog/exception.c`, `pintos/userprog/syscall.c`, `pintos/include/threads/thread.h`, `pintos/vm/vm.c` | syscall 경로 user rsp 저장 필드, user buffer validation helper, `vm_try_handle_fault()`, `vm_stack_growth()` | 13일 목표가 안정된 뒤 진행. invalid fault, write-protected fault, stack 후보 fault를 분리해 기록 |
| 2026-05-15(금) | SPT kill/copy와 fork 기반 page lifecycle 복구 | `pintos/vm/vm.c`, `pintos/userprog/process.c`, `pintos/include/vm/vm.h`, PR/WIL 문서 | `supplemental_page_table_kill()`, `supplemental_page_table_copy()`, `vm_dealloc_page()`, `process_fork()`, `__do_fork()` | page/frame/aux 해제 책임과 fork 복제 정책을 먼저 닫고, 실제 테스트 결과는 실행 후 별도 기록 |
| 2026-05-16(토) | stack growth 완성, frame table과 eviction 준비 | `pintos/vm/vm.c`, `pintos/include/vm/vm.h` | `vm_stack_growth()`, frame table 자료구조, frame list/lock, victim 후보 순회 함수, accessed/dirty bit 확인 지점 | stack 확장 허용 범위와 frame table 소유권을 문서화. eviction은 설계와 골격을 먼저 확정 |
| 2026-05-18(월) | mmap/munmap 기본과 file-backed page 등록 | `pintos/userprog/syscall.c`, `pintos/vm/file.c`, `pintos/include/vm/file.h`, `pintos/include/threads/thread.h` | `mmap()`, `munmap()`, `do_mmap()`, `do_munmap()`, `file_backed_initializer()`, `file_backed_swap_in()`, `file_backed_destroy()`, mmap range metadata | SPT copy/kill과 stack growth가 정리된 뒤 진입. fd close와 mapping lifetime 분리 여부를 설계 기록 |
| 2026-05-19(화) | dirty write-back, swap, eviction 통합 | `pintos/userprog/syscall.c`, `pintos/vm/vm.c`, `pintos/vm/anon.c`, `pintos/vm/file.c`, `pintos/userprog/process.c` | dirty bit 확인, `file_write_at()` write-back, `vm_get_victim()`, `vm_evict_frame()`, `anon_swap_in()`, `anon_swap_out()`, `file_backed_swap_out()`, swap block, swap slot bitmap | mmap lifetime이 정리된 뒤 진행. swap slot, frame, file-backed write-back 소유권을 PR에 기록 |
| 2026-05-20(수) | VM 전체 회귀, 제출 안정화, 발표/WIL 정리 | 전체 영향 파일, PR/WIL/발표 자료 | 전체 test command, 실패 로그 요약, lifecycle 체크리스트, rollback 필요한 임시 helper 목록 | 실제 실행 결과 중심으로 통과/실패 목록을 정리하고, 남은 실패는 원인/재현 명령/다음 조치를 기록 |
| 2026-05-21(목) | 최종 공유와 제출 | 발표 자료, WIL, GitHub Projects | 팀 발표 자료, 개인별 2분 발표 내용, WIL URL | 오전 10시 주간 공유, 정오까지 발표 자료와 WIL 제출, 오후 동료피드백/티타임 준비 |

### 6.3 공동 구현 제외일

| 날짜 | 운영 방식 | 개인 권장 활동 |
|------|------|------|
| 2026-05-09(토) | 공동 구현 제외 | `fork()`, Project 2 process lifecycle, fd table 복습 |
| 2026-05-10(일) | 공동 구현 제외 | GitBook Project 3, VM flow diagram, SPT/page fault 질문 정리 |
| 2026-05-17(일) | 공동 구현 제외 | mmap/swap 설계 복습, 실패 로그 개인 분석, 발표/WIL 초안 작성 |

### 6.4 모듈 소유권 기록 규칙

- 새 구조체 필드를 추가하면 생성 시점, 소유자, 해제 시점, 실패 시 cleanup 경로를 PR에 적는다.
- `pintos/vm/vm.c`는 SPT, frame, eviction이 모두 모이는 파일이므로 단독 장기 수정하지 않는다.
- `pintos/userprog/process.c`는 load, stack setup, fork, exit가 함께 걸리므로 VM 변경 후 Project 2 회귀를 같이 확인한다.
- `pintos/userprog/syscall.c`는 user pointer validation과 `mmap`/`munmap`이 함께 걸리므로 invalid pointer 테스트와 mmap 테스트를 같은 날 확인한다.
- `pintos/vm/file.c`와 `pintos/vm/anon.c`는 page type별 swap in/out 책임을 나누고, frame free 책임과 중복되지 않게 정리한다.


## 7. 테스트 운영 원칙

- 테스트는 "끝났는지 체크하는 용도"가 아니라 "설계 가설이 맞는지 확인하는 도구"로 사용한다.
- 작은 기능 구현 후 가장 가까운 테스트 1-3개를 먼저 실행하고, 같은 묶음 테스트와 회귀 테스트로 넓힌다.
- 한 번에 여러 테스트가 깨지면 SPT와 page fault handler처럼 구현 의존성이 앞선 단계부터 본다.
- `lazy-*`가 실패하면 `load_segment()`, `lazy_load_segment()`, aux lifetime을 먼저 확인한다.
- `pt-*`가 실패하면 address validation, stack growth 조건, writable bit를 먼저 확인한다.
- `page-*`가 실패하면 lazy loading, SPT copy, fork 흐름, frame 관리, page lifecycle을 확인한다.
- `mmap-*`가 실패하면 syscall 인자 검증, address overlap, fd/file lifetime, dirty write-back, munmap cleanup을 확인한다.
- `swap-*`가 실패하면 frame victim 선정, swap slot bitmap, swap in/out cleanup, page 재적재를 확인한다.
- 회귀 테스트가 실패하면 Project 2 process/file descriptor lifecycle, Threads alarm/priority, filesys/base와 VM 변경의 상호작용을 확인한다.


## 8. 소통과 학습 공유

- 막힌 테스트는 오래 끌지 말고 바로 공유한다.
- 코어타임에서는 "무엇을 바꿨는지"보다 "왜 그렇게 설계했는지"를 우선 설명한다.
- SPT entry, frame, swap slot, mmap file처럼 owner가 있는 자원은 누가 언제 해제하는지 함께 설명한다.
- WIL에는 단순 결과보다 배운 개념, 실패 원인, 수정 근거, 다음 리스크를 남긴다.
- Virtual Memory 필수 범위가 안정되면 `cow-simple` Extra 진행 여부를 먼저 판단하고, 남는 시간에는 Project 1~3 복습, 포트폴리오/블로그 정리, 모의 면접, Project 4 진행 여부를 논의한다.
