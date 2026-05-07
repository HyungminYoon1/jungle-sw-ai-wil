# Week 10 - 핵심 역량 목표 달성률 평가

> 작성 시점: 주차 마무리 시점

한 주가 마무리되는 시점에 목표에 대한 달성률을 핵심 역량별로 기입합니다.
이번 평가는 `local/week9_10_pintos_threads_userprograms_implementation/week09-team-07-pintos-threads`의 문서, 커밋 로그, 코드, `pintos/userprog/build/tests/userprog/*.result` 테스트 결과를 기준으로 작성했습니다.

---

## 1. 문제해결
- **목표:** PintOS User Programs 핵심 주제 10개(`User Program`, `User mode`, `Kernel mode`, `ELF loader`, `argument passing`, `system call`, `file descriptor`, `user pointer validation`, `fork/exec/wait`, `Virtual Memory Layout`)를 학습하고 Project 2 구현 흐름에 적용
- **달성률:** 100% (10 / 10)
- **회고:** `process_create_initd()`부터 `load()`, argument stack 구성, `syscall_entry` 이후 `intr_frame` 기반 syscall dispatch, fd table, user pointer 검증, `fork/exec/wait` 동기화까지 Project 2 핵심 흐름을 코드와 테스트에 연결했다. user/kernel boundary를 잘못 다루면 커널이 사용자 포인터에 의해 무너질 수 있다는 점을 bad pointer와 boundary 테스트를 통해 확인했다.


## 2. 설계
- **목표:** Project 2 구현 전 **argument stack 구성, syscall dispatch, user address 검증, file descriptor table, process parent-child sync, fork context 복제, exec/load 실패 처리, write deny 정책, 테스트 전략** 등 설계 포인트 **9개 이상**을 이슈·PR·발표 메모에 명시
- **달성률:** 100% (9 / 9)
- **회고:** `docs/week10-implementation-plan.md`와 `docs/week10-collaboration.md`에 구현 순서, 작업 분할, 병합 기준, 테스트 운영 방식을 정리했다. 특히 argument passing을 syscall보다 먼저 안정화하는 이유, fd table이 process/thread 상태에 있어야 하는 이유, parent-child lifecycle을 `child_status`와 semaphore로 맞추는 이유를 문서와 PR 설명 자료에 남겼다.


## 3. 구현
- **목표:** Project 2 핵심 구현 7개(`argument passing`, `halt/exit`, `user pointer validation`, `file descriptor table`, `file syscalls`, `fork/exec/wait`, `robustness 처리`) 완료
- **달성률:** 100% (7 / 7)
- **회고:** `process.c`와 `syscall.c`를 중심으로 argument passing, 기본 syscall, fd table, `create/open/read/write/close`, `fork/exec/wait`, 실행 파일 write deny, bad pointer 및 boundary 대응을 구현했다. `fork-read`, `fork-close`, `exec-read`, `multi-*`, `rox-*`, `bad-*`가 모두 `PASS`로 확인되어 userprog 핵심 구현은 완료 기준을 충족했다.


## 4. 품질
- **목표:** CSV 기준 Project 2 필수 테스트 76개 통과
- **달성률:** 84% (64 / 76)
- **회고:** `pintos/userprog/build/tests/userprog/**/*.result`에 남아 있는 User Programs 테스트 결과 중 64개가 `PASS`였다. 이 집계에는 `no-vm/multi-oom.result`의 `PASS`를 포함했다. 다만 목표 산식의 분모인 76개 중 `lg-*`, `sm-*`, `syn-*` 12개는 해당 경로에서 개별 `.result` 파일을 확인하지 못해 통과로 계산하지 않았다.

### 테스트 세부 결과

| 구분 | 결과 | 확인한 테스트 |
|------|:----:|------|
| Argument Passing / Basic Syscall | 7 / 7 PASS | `args-none`, `args-single`, `args-multiple`, `args-many`, `args-dbl-space`, `halt`, `exit` |
| File Syscall - Create / Open / Close | 17 / 17 PASS | `create-normal`, `create-empty`, `create-null`, `create-bad-ptr`, `create-long`, `create-exists`, `create-bound`, `open-normal`, `open-missing`, `open-boundary`, `open-empty`, `open-null`, `open-bad-ptr`, `open-twice`, `close-normal`, `close-twice`, `close-bad-fd` |
| File Syscall - Read / Write | 12 / 12 PASS | `read-normal`, `read-bad-ptr`, `read-boundary`, `read-zero`, `read-stdout`, `read-bad-fd`, `write-normal`, `write-bad-ptr`, `write-boundary`, `write-zero`, `write-stdin`, `write-bad-fd` |
| Process Control | 16 / 16 PASS | `fork-once`, `fork-multiple`, `fork-recursive`, `fork-read`, `fork-close`, `fork-boundary`, `exec-once`, `exec-arg`, `exec-boundary`, `exec-missing`, `exec-bad-ptr`, `exec-read`, `wait-simple`, `wait-twice`, `wait-killed`, `wait-bad-pid` |
| Robustness / Userprog Interaction | 12 / 12 PASS | `multi-recurse`, `multi-child-fd`, `multi-oom`, `rox-simple`, `rox-child`, `rox-multichild`, `bad-read`, `bad-write`, `bad-read2`, `bad-write2`, `bad-jump`, `bad-jump2` |
| CSV 기준 미확인 항목 | 0 / 12 PASS | `lg-create`, `lg-full`, `lg-random`, `lg-seq-block`, `lg-seq-random`, `sm-create`, `sm-full`, `sm-random`, `sm-seq-block`, `sm-seq-random`, `syn-remove`, `syn-write` |


## 5. 유지보수
- **목표:** 이번 주(금~목) **7일 중 5일 이상** 팀 repository 개인 branch에 의미 있는 commit을 남기고, PR 또는 리뷰 기록으로 구현·테스트 변경 사항을 추적
- **달성률:** 100% (7 / 5, 100%로 계산)
- **회고:** 2026-05-01부터 2026-05-07까지 매일 의미 있는 커밋을 남겼다. 임시 대기 흐름, argument passing, syscall dispatcher, exec/fork/wait, read 검증, 실행 파일 write deny, 파일 위치 이동 syscall 등 변경이 기능 단위로 작성되었고 PR 병합도 하였다.


## 6. 협업
- **목표:** 팀 GitHub Projects 준비(1개) + 개인 branch 작업(1개) + 매일 코어타임 PR 리뷰(1개) + 테스트 실패 공유 로그(1개) + 주간 공유 발표 자료 통합(1개) = **5개 완료**
- **달성률:** 100% (5 / 5)
- **회고:** `docs/week10-collaboration.md`에 GitHub Projects, 브랜치 전략, PR 리뷰 기준, 테스트 실패 공유 방식을 정리했다. 개인 브랜치 작업과 PR 병합 내역을 커밋 로그에 남겼고, rox 및 fork/exec/wait 설명 자료와 주간 공유 발표 자료를 작성하였다.


## 7. 태도
- **목표:** 공통 6개(목표 수립, 달성률 평가, AI 원칙, 업무 검토, WIL, 팀 협업 룰) 중 **6개 완료**
- **달성률:** 100% (6 / 6)
- **회고:** 목표 수립, AI 활용 원칙, 체크리스트 기반 업무 검토, 팀 협업 룰, 달성률 평가를 정리했다.


## 8. 비즈니스 이해
- **목표:** User Programs 구현이 운영체제의 **사용자 프로그램 실행, user/kernel boundary, system call API, 파일 자원 관리, 프로세스 생명주기 관리**와 어떤 관련이 있는지 WIL 또는 발표 자료에 **5문장 이상** 명시
- **달성률:** 100% (5 / 5)
- **회고:** User Programs 구현을 통해 운영체제가 사용자 프로그램을 안전하게 실행하고, system call API로 커널 기능을 제한적으로 노출하며, 파일 descriptor와 process lifecycle을 커널 자원으로 관리한다는 점을 정리했다. 특히 실행 중인 파일에 대한 write deny와 user pointer validation은 안정성과 보안 요구로 연결된다.


## 9. AI 활용
- **목표:** AI 활용 원칙 수립(1개) + Project 2 개념 설명 검증(1개) + 테스트 실패 분석 또는 코드 리뷰 보조 활용 기록(1개) = **3개 완료**
- **달성률:** 100% (3 / 3)
- **회고:** AI를 학습·검증 보조 도구로 제한하는 원칙을 세웠다. 구현 설명 자료에는 argument passing, syscall, fork/exec/wait, rox 흐름을 개념 검증과 리뷰 관점으로 정리하였다.


## 10. 학습 민첩성
- **목표:** `syscall entry`, `intr_frame`, `argument stack`, `ELF loading`, `file descriptor table`, `stdin/stdout`, `bad pointer`, `boundary check`, `fork`, `exec`, `wait`, `deny write executable`을 포함해 **12개 이상 개념/구현 포인트**를 학습 후 적용
- **달성률:** 100% (12 / 12)
- **회고:** syscall 진입 레지스터와 `intr_frame`, user stack layout, ELF loading, fd table, 표준 입출력, pointer/boundary 검증, fork 주소 공간 복제, exec 이미지 교체, wait 종료 상태 회수, 실행 파일 write deny까지 학습한 내용을 구현과 테스트에 연결했다. 개념 학습이 바로 실패 테스트 분류와 수정 순서 결정에 사용되었다.


---

### 주간 요약

| # | 역량 | 달성률 |
|---|------|--------|
| 1 | 문제해결 | 100% |
| 2 | 설계 | 100% |
| 3 | 구현 | 100% |
| 4 | 품질 | 84% |
| 5 | 유지보수 | 100% |
| 6 | 협업 | 100% |
| 7 | 태도 | 100% |
| 8 | 비즈니스 이해 | 100% |
| 9 | AI 활용 | 100% |
| 10 | 학습 민첩성 | 100% |
| | **평균** | **98%** |

- **상위 역량:** 문제해결, 설계, 구현, 유지보수, 협업, 태도, 비즈니스 이해, AI 활용, 학습 민첩성
- **개선 필요:** 품질 산식의 분모 76개 중 결과 파일이 확인되지 않은 12개 테스트의 실행 근거 확보
- **핵심 완료 항목:** `args-*`, 기본 syscall, file syscall, user pointer validation, `fork/exec/wait`, `multi-*`, `rox-*`, `bad-*`
