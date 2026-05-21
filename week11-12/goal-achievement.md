# Week 11-12 - 핵심 역량 목표 달성률 평가

> 작성 시점: 주차 마무리 시점

2주 동안 진행한 PintOS Project 3 - Virtual Memory 목표에 대한 달성률을 핵심 역량별로 기입합니다.
이번 평가는 `local/week11-12_pintos_vm_implementation/week11-team-03-pintos-vm`의 문서, 커밋 로그, 코드, `pintos/vm/build/tests/**/*.result` 테스트 결과를 기준으로 작성했습니다.

---

## 1. 문제해결
- **목표:** Virtual Memory 핵심 주제 12개(`Virtual Memory`, `Page Table`, `Supplemental Page Table`, `Page Fault`, `Lazy Loading`, `Stack Growth`, `Frame Table`, `Page Replacement`, `Swap In/Out`, `mmap/munmap`, `Anonymous/File-backed Page`, `Project 2 Regression`)를 학습하고 Project 3 구현 흐름에 적용
- **달성률:** 100% (12 / 12)
- **회고:** SPT가 PML4에 없는 page의 장기 상태를 보관하고, page fault가 발생했을 때 lazy loading, stack growth, swap in, file-backed read로 복구하는 흐름을 구현에 연결했다. frame table과 second-chance eviction, anon/file-backed swap in/out, `mmap()`/`munmap()` cleanup까지 확인하면서 VM이 단순한 page fault 처리기가 아니라 process lifecycle과 파일 시스템 회귀까지 함께 책임지는 구조라는 점을 이해했다.


## 2. 설계
- **목표:** Project 3 구현 전 **SPT entry 구조, page type별 lifecycle, lazy load initializer, page fault 분기 조건, stack growth 조건, frame table 소유권, eviction victim 정책, swap slot 관리, mmap/munmap cleanup, fork/exit VM 자원 처리, 테스트 전략** 등 설계 포인트 **11개 이상**을 이슈·PR·발표 메모에 명시
- **달성률:** 100% (11 / 11)
- **회고:** `docs/week11-12-implementation-plan.md`, `docs/week11-12-vm-implementation-flow.md`, `docs/week11-12-collaboration.md`에 SPT key, page/frame 연결, aux lifetime, dirty write-back, mmap file reference, swap slot owner, fork 시 page type별 복제 정책을 정리했다. 특히 `VM_UNINIT + VM_ANON`, loaded `VM_ANON`, swap-out `VM_ANON`, `VM_FILE` mmap page가 fork와 cleanup에서 서로 다르게 처리되어야 한다는 기준을 문서와 발표 자료에 남겼다.


## 3. 구현
- **목표:** Project 3 핵심 구현 8개(`supplemental page table`, `page fault handling`, `lazy loading`, `stack growth`, `anonymous page/swap`, `file-backed page`, `mmap/munmap`, `fork/exit VM cleanup`) 완료
- **달성률:** 100% (8 / 8)
- **회고:** `pintos/vm/vm.c`, `pintos/vm/anon.c`, `pintos/vm/file.c`, `pintos/userprog/process.c`, `pintos/userprog/exception.c`, `pintos/userprog/syscall.c`를 중심으로 VM 필수 범위를 구현했다. SPT hash table, page claim, lazy segment load, stack growth, SPT copy/kill, frame eviction, swap slot bitmap, `mmap()`/`munmap()`, dirty write-back, fd table과 process cleanup 회귀 대응까지 연결되었고 VM 필수 테스트 46개가 모두 `PASS`로 확인되었다.


## 4. 품질
- **목표:** CSV 기준 필수·회귀 테스트 140개(Project 3 핵심 46개 + Project 2/User Programs 회귀 63개 + Filesys 회귀 13개 + Threads 회귀 18개) 통과, Extra `cow-simple`은 별도 확인
- **달성률:** 100% (140 / 140)
- **회고:** `pintos/vm/build/tests/**/*.result` 기준으로 목표 산식에 포함된 필수·회귀 테스트 140개가 모두 `PASS`였다. Project 3 VM root 테스트 46개, User Programs 회귀 63개, Filesys base 회귀 13개, Threads alarm/priority 회귀 18개를 확인했다. Extra 범위인 `pintos/vm/build/tests/vm/cow/cow-simple.result`는 `FAIL`이었지만 목표 산식에서 분리된 선택 과제로 기록했다.

### 테스트 세부 결과

| 구분 | 결과 | 확인한 테스트 |
|------|:----:|------|
| Project 3 - Page Table / Stack Growth | 8 / 8 PASS | `pt-grow-stack`, `pt-grow-bad`, `pt-big-stk-obj`, `pt-bad-addr`, `pt-bad-read`, `pt-write-code`, `pt-write-code2`, `pt-grow-stk-sc` |
| Project 3 - Page / Lazy Loading | 9 / 9 PASS | `page-linear`, `page-parallel`, `page-merge-seq`, `page-merge-par`, `page-merge-stk`, `page-merge-mm`, `page-shuffle`, `lazy-file`, `lazy-anon` |
| Project 3 - mmap / munmap | 25 / 25 PASS | `mmap-read`, `mmap-close`, `mmap-unmap`, `mmap-overlap`, `mmap-twice`, `mmap-write`, `mmap-ro`, `mmap-exit`, `mmap-shuffle`, `mmap-bad-fd`, `mmap-clean`, `mmap-inherit`, `mmap-misalign`, `mmap-null`, `mmap-over-code`, `mmap-over-data`, `mmap-over-stk`, `mmap-remove`, `mmap-zero`, `mmap-bad-fd2`, `mmap-bad-fd3`, `mmap-zero-len`, `mmap-off`, `mmap-bad-off`, `mmap-kernel` |
| Project 3 - Swap | 4 / 4 PASS | `swap-file`, `swap-anon`, `swap-iter`, `swap-fork` |
| Project 2 회귀 - User Programs | 63 / 63 PASS | `args-*`, file syscall, process control, `multi-*`, `rox-*`, `bad-*` |
| Filesys 회귀 | 13 / 13 PASS | `lg-*`, `sm-*`, `syn-read`, `syn-remove`, `syn-write` |
| Threads 회귀 | 18 / 18 PASS | `alarm-*`, `priority-*` |
| Extra | 0 / 1 PASS | `cow-simple`은 `FAIL`, 필수 범위와 분리 |


## 5. 유지보수
- **목표:** 2주 기간 중 **14일 중 10일 이상** 팀 repository 개인 branch에 의미 있는 commit을 남기고, PR 또는 리뷰 기록으로 구현·테스트 변경 사항을 추적
- **달성률:** 100% (11 / 10, 100%로 계산)
- **회고:** 2026-05-08, 2026-05-11부터 2026-05-16, 2026-05-18부터 2026-05-21까지 총 11일의 커밋 기록을 확인했다. SPT, lazy loading, page fault, stack growth, SPT cleanup, fork copy, mmap, dirty write-back, eviction, swap-out anon fork copy, 최종 문서화가 기능 단위로 남아 있어 변경 추적이 가능했다.


## 6. 협업
- **목표:** 팀 GitHub Projects 준비(1개) + 팀 repository 준비(1개) + 개인 branch 작업(1개) + 코어타임 PR 리뷰(1개) + 테스트 실패 공유 로그(1개) + 주간 공유 발표 자료 통합(1개) = **6개 완료**
- **달성률:** 100% (6 / 6)
- **회고:** `docs/week11-12-collaboration.md`에 팀 repository, 브랜치 흐름, PR 리뷰 기준, 테스트 실패 공유 방식, 발표 제출 기준을 정리했다. 2026-05-11과 2026-05-12에는 4인 공동 구현으로 SPT와 VM core를 맞췄고, 2026-05-13 이후에는 2명:2명 페어 프로그래밍으로 lazy loading, fork, mmap, swap을 나누어 구현했다. 최종 팀 PPT와 개인 2분 발표 자료도 완성했다.


## 7. 태도
- **목표:** 공통 6개(목표 수립, 달성률 평가, AI 원칙, 업무 검토, WIL, 팀 협업 룰) 중 **6개 완료**
- **달성률:** 100% (6 / 6)
- **회고:** 목표 수립, AI 활용 원칙, 체크리스트 기반 업무 검토, 팀 협업 룰, 구현 기록, 달성률 평가를 정리했다. 마감 전에는 단순 테스트 통과보다 page, frame, aux, file reference, swap slot의 소유권과 해제 경로를 설명할 수 있는지에 초점을 두고 발표와 문서를 정리했다.


## 8. 비즈니스 이해
- **목표:** Virtual Memory 구현이 운영체제의 **메모리 효율, 지연 적재, 프로세스 격리, page fault 복구, 파일 매핑, swap 기반 자원 확장**과 어떤 관련이 있는지 WIL 또는 발표 자료에 **6문장 이상** 명시
- **달성률:** 100% (6 / 6)
- **회고:** Virtual Memory는 실행 파일과 mmap 파일을 필요한 시점에만 읽어 메모리 사용량을 줄이고, page fault를 정상 복구 경로로 사용해 지연 적재를 가능하게 한다. SPT와 PML4를 분리해 process별 주소 공간 의미와 실제 하드웨어 mapping을 따로 관리하며, fork에서는 부모와 자식이 독립된 frame을 갖도록 복제해야 프로세스 격리가 유지된다. frame이 부족할 때는 eviction과 swap으로 물리 메모리보다 큰 작업 집합을 다룰 수 있고, file-backed page의 dirty write-back은 메모리 변경을 파일 영속성과 연결한다.


## 9. AI 활용
- **목표:** AI 활용 원칙 수립(1개) + Project 3 개념 설명 검증(1개) + 테스트 실패 분석 또는 코드 리뷰 보조 활용 기록(1개) + AI 제안 코드/설계의 직접 설명 검증 기록(1개) = **4개 완료**
- **달성률:** 100% (4 / 4)
- **회고:** `week11-12/ai-principles.md`에서 AI를 학습·검토 보조로 제한하는 원칙을 세웠다. SPT, page fault, lazy loading, stack growth, mmap, swap 흐름은 KAIST PintOS 문서와 코드, 테스트 결과로 검증했고, AI는 실패 로그 해석, 의심 코드 경로 정리, 리뷰 체크리스트 점검, 발표/WIL 문장 정리에 제한적으로 사용했다. 핵심 구현 코드는 설명 없이 복사하지 않는다는 기준을 유지했다.


## 10. 학습 민첩성
- **목표:** `vm_try_handle_fault`, `struct page`, `supplemental_page_table`, `uninit page`, `anon page`, `file-backed page`, `frame table`, `eviction`, `swap slot`, `mmap`, `munmap`, `stack growth`, `dirty bit`, `fork page copy`를 포함해 **14개 이상 개념/구현 포인트**를 학습 후 적용
- **달성률:** 100% (14 / 14)
- **회고:** `vm_try_handle_fault()`의 fault 분기, `struct page`와 type별 operations, SPT hash table, `VM_UNINIT` lazy metadata, anon swap state, file-backed mmap metadata, frame table과 victim selection, swap slot bitmap, dirty bit write-back, stack growth 조건, fork page copy 정책을 학습한 뒤 구현에 반영했다. 특히 swap-out된 anon page를 fork할 때 부모 swap slot을 소비하지 않고 자식 frame에 읽기 복사해야 한다는 점을 마지막 보완 항목으로 정리했다.


---

### 주간 요약

| # | 역량 | 달성률 |
|---|------|--------|
| 1 | 문제해결 | 100% |
| 2 | 설계 | 100% |
| 3 | 구현 | 100% |
| 4 | 품질 | 100% |
| 5 | 유지보수 | 100% |
| 6 | 협업 | 100% |
| 7 | 태도 | 100% |
| 8 | 비즈니스 이해 | 100% |
| 9 | AI 활용 | 100% |
| 10 | 학습 민첩성 | 100% |
| | **평균** | **100%** |

- **상위 역량:** 문제해결, 설계, 구현, 품질, 유지보수, 협업, 태도, 비즈니스 이해, AI 활용, 학습 민첩성
- **개선 필요:** Extra COW(`cow-simple`)는 필수 범위와 분리되어 실패 상태로 남았으므로, 추후 copy-on-write 정책을 별도 설계해야 한다.
- **핵심 완료 항목:** SPT, lazy loading, page fault handling, stack growth, fork/SPT copy, anon swap, frame eviction, file-backed page, `mmap()`/`munmap()`, dirty write-back, Project 1/2 및 filesys 회귀 확인
