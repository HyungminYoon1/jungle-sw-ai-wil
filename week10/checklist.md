# Week 10 - 제출물 체크리스트

> 주차별 제출물과 과제 진행 상황을 점검합니다.

---

## 주차 시작 (금요일)

| 항목 | 완료 | 비고 |
|------|:----:|------|
| 핵심 역량 목표 수립 (goal.md) | □ | |
| AI 활용 원칙 수립 (ai-principles.md) | □ | |
| 이번 주 할 일(업무) 검토 | □ | PintOS Project 2 - User Programs 공지 확인 |
| 팀 협업 룰/팁 정리 및 합의 (team-collaboration.md) | □ | |
| 팀 GitHub Projects 주소 제출 | □ | 금요일 24:00, 팀당 1명 |
| PintOS 기초 특강 - User Program 참석 | □ | 금요일 오전 10시 |
| PintOS 실습 환경 확인 | □ | Ubuntu 22.04 LTS x86_64 또는 Docker |
| 테스트 실행 도구 확인 | □ | 기본 테스트 또는 pintos-util |

---

## 주차 진행

| 과제/항목 | 완료 | 비고 |
|-----------|:----:|------|
| KAIST GitBook Project 2 User Programs 학습 | □ | |
| KAIST GitBook Project 2 FAQ 학습 | □ | |
| ELF loader / process 실행 흐름 학습 | □ | |
| User mode / Kernel mode 전환 흐름 학습 | □ | |
| system call 진입 흐름 학습 | □ | syscall handler |
| user pointer / user address 검증 설계 | □ | bad pointer, boundary |
| argument passing 설계 | □ | `args-*` |
| argument passing 구현 | □ | |
| halt / exit syscall 구현 | □ | |
| file descriptor table 설계 | □ | fd allocation, close |
| file syscall 구현 | □ | create/open/read/write/close |
| fork 구현 | □ | parent-child sync |
| exec 구현 | □ | load 실패 처리 |
| wait 구현 | □ | exit status, 중복 wait |
| executable write deny 처리 | □ | `rox-*` |
| robustness / bad pointer 테스트 대응 | □ | bad-read/write/jump |
| 화요일 퀴즈 준비 | □ | PintOS, C언어 |
| 매일 개인 branch 작업 | □ | 기능 단위 commit |
| 매일 코어타임 PR 리뷰 | □ | main 병합 전 리뷰 |
| 테스트 실패 로그 기록 | □ | 실패명, 원인, 수정, 재실행 결과 |
| 주간 공유 발표 자료 초안 작성 | □ | 구현, 트러블슈팅, 회고 |

---

## Project 2 테스트 체크리스트

### Argument Passing / Basic Syscall

| 테스트 | 완료 | 비고 |
|--------|:----:|------|
| args-none | □ | |
| args-single | □ | |
| args-multiple | □ | |
| args-many | □ | |
| args-dbl-space | □ | |
| halt | □ | |
| exit | □ | |

### File Syscall - Create / Open / Close

| 테스트 | 완료 | 비고 |
|--------|:----:|------|
| create-normal | □ | |
| create-empty | □ | |
| create-null | □ | |
| create-bad-ptr | □ | |
| create-long | □ | |
| create-exists | □ | |
| create-bound | □ | |
| open-normal | □ | |
| open-missing | □ | |
| open-boundary | □ | |
| open-empty | □ | |
| open-null | □ | |
| open-bad-ptr | □ | |
| open-twice | □ | |
| close-normal | □ | |
| close-twice | □ | |
| close-bad-fd | □ | |

### File Syscall - Read / Write

| 테스트 | 완료 | 비고 |
|--------|:----:|------|
| read-normal | □ | |
| read-bad-ptr | □ | |
| read-boundary | □ | |
| read-zero | □ | |
| read-stdout | □ | |
| read-bad-fd | □ | |
| write-normal | □ | |
| write-bad-ptr | □ | |
| write-boundary | □ | |
| write-zero | □ | |
| write-stdin | □ | |
| write-bad-fd | □ | |

### Process Control

| 테스트 | 완료 | 비고 |
|--------|:----:|------|
| fork-once | □ | |
| fork-multiple | □ | |
| fork-recursive | □ | |
| fork-read | □ | |
| fork-close | □ | |
| fork-boundary | □ | |
| exec-once | □ | |
| exec-arg | □ | |
| exec-boundary | □ | |
| exec-missing | □ | |
| exec-bad-ptr | □ | |
| exec-read | □ | |
| wait-simple | □ | |
| wait-twice | □ | |
| wait-killed | □ | |
| wait-bad-pid | □ | |

### Robustness / File Interaction

| 테스트 | 완료 | 비고 |
|--------|:----:|------|
| multi-recurse | □ | |
| multi-child-fd | □ | |
| rox-simple | □ | |
| rox-child | □ | |
| rox-multichild | □ | |
| bad-read | □ | |
| bad-write | □ | |
| bad-read2 | □ | |
| bad-write2 | □ | |
| bad-jump | □ | |
| bad-jump2 | □ | |
| lg-create | □ | |
| lg-full | □ | |
| lg-random | □ | |
| lg-seq-block | □ | |
| lg-seq-random | □ | |
| sm-create | □ | |
| sm-full | □ | |
| sm-random | □ | |
| sm-seq-block | □ | |
| sm-seq-random | □ | |
| syn-remove | □ | |
| syn-write | □ | |
| multi-oom | □ | |

---

## Project 1 회귀 / Extra 체크

| 구분 | 테스트 | 완료 | 비고 |
|------|--------|:----:|------|
| Threads 회귀 | alarm-* 6개 | □ | CSV 포함, 필요 시 확인 |
| Threads 회귀 | priority-* 12개 | □ | CSV 포함, 필요 시 확인 |
| Extra | dup2-simple | □ | extra |
| Extra | dup2-complex | □ | extra |

---

## 주차 종료 (목요일)

| 항목 | 완료 | 비고 |
|------|:----:|------|
| 주간 공유 발표 자료 제출 | □ | 목요일 정오, 팀당 1명 |
| 주간 공유 발표 준비 | □ | 목요일 오전 10시, 개인별 2분 발표 |
| 프로젝트 구현 및 트러블슈팅 정리 | □ | 발표 자료 포함 |
| 핵심 역량 목표 달성률 평가 (goal-achievement.md) | □ | |
| WIL 작성 (wil.md) | □ | |
| WIL 블로그 포럼 등록 | □ | WEEK10 태그 |
| 10주차 동료피드백 | □ | 차주 발제 시간 |

