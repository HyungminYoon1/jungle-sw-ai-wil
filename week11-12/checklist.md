# Week 11-12 - 제출물 체크리스트

> 주차별 제출물과 과제 진행 상황을 점검합니다.

---

## 주차 시작 (금요일)

| 항목 | 완료 | 비고 |
|------|:----:|------|
| 핵심 역량 목표 수립 (goal.md) | ☑ | 작성 완료 |
| AI 활용 원칙 수립 (ai-principles.md) | ☑ | 작성 완료 |
| 이번 주 할 일(업무) 검토 | ☑ | PintOS Project 3 - Virtual Memory 공지 확인 |
| 팀 협업 룰/팁 정리 및 합의 (team-collaboration.md) | ☑ | 작성 및 합의 완료 |
| 팀 GitHub Projects 주소 제출 | ☑ | 첫날 금요일 24:00 제출 |
| 팀 GitHub repository 주소 제출 | ☑ | 첫날 금요일 24:00 제출 |
| PintOS 기초 특강 - VM 참석 | ☑ | 금요일 오전 10시 |
| PintOS 실습 환경 확인 | ☑ | Ubuntu 22.04 LTS x86_64 / Docker 환경 확인 |
| 테스트 실행 도구 확인 | ☑ | `make check`, 개별 `.result` target 확인 |

---

## 1주차 진행

| 과제/항목 | 완료 | 비고 |
|-----------|:----:|------|
| KAIST GitBook Project 3 Virtual Memory 학습 | ☑ | SPT, lazy loading, stack growth, mmap, swap 흐름 학습 |
| Project 3 FAQ 학습 | ☑ | 구현 전 요구사항과 테스트 의도 확인 |
| Appendix Memory Allocation 학습 | ☑ | frame, page allocator, swap 흐름과 연결 |
| Appendix Virtual Address 학습 | ☑ | user/kernel address, page alignment 확인 |
| Appendix Page Table 학습 | ☑ | PML4와 SPT 역할 구분 |
| 기존 Project 2 코드와 VM scaffold 확인 | ☑ | `vm/`, `userprog/`, `threads/` |
| supplemental page table 설계 | ☑ | page type, owner, lifecycle |
| page fault handler 설계 | ☑ | lazy load, stack growth, invalid access |
| uninit page / lazy loading 구현 | ☑ | `lazy-*`, `page-*` 기반 |
| stack growth 구현 | ☑ | `pt-grow-*` |
| anonymous page 기본 구현 | ☑ | swap 연계 전 기본 anon lifecycle 구현 |
| file-backed page 기본 설계 | ☑ | mmap 전 file-backed metadata 설계 |
| 1주차 테스트 실패 로그 기록 | ☑ | 실패명, 원인, 수정, 재실행 결과 기록 |
| 화요일 퀴즈 준비 | ☑ | PintOS, C언어 |

---

## 2주차 진행

| 과제/항목 | 완료 | 비고 |
|-----------|:----:|------|
| frame table 설계 및 구현 | ☑ | frame allocation, owner page |
| page replacement 정책 구현 | ☑ | second-chance eviction victim selection |
| swap in/out 구현 | ☑ | swap slot bitmap 관리 |
| mmap 구현 | ☑ | address validation, file-backed page |
| munmap 구현 | ☑ | dirty write-back, page cleanup |
| fork 시 page/SPT 처리 구현 | ☑ | child page lifecycle |
| process exit 시 VM 자원 정리 | ☑ | SPT, frame, swap, mmap |
| Project 2 userprog 회귀 확인 | ☑ | 63개 CSV 항목 PASS |
| filesys / threads 회귀 확인 | ☑ | Filesys 13개, Threads alarm/priority 18개 PASS |
| 주간 공유 발표 자료 초안 작성 | ☑ | 구현, 트러블슈팅, 회고 |
| WIL 초안 작성 | ☑ | WEEK11-12 태그, 작성 완료로 가정 |
| 동료피드백 준비 | ☑ | 2주차 목요일 발제 시간 |

---

## Project 3 테스트 체크리스트

### Page Table / Stack Growth

| 테스트 | 완료 | 비고 |
|--------|:----:|------|
| pt-grow-stack | ☑ | PASS |
| pt-grow-bad | ☑ | PASS |
| pt-big-stk-obj | ☑ | PASS |
| pt-bad-addr | ☑ | PASS |
| pt-bad-read | ☑ | PASS |
| pt-write-code | ☑ | PASS |
| pt-write-code2 | ☑ | PASS |
| pt-grow-stk-sc | ☑ | PASS |

### Page / Lazy Loading

| 테스트 | 완료 | 비고 |
|--------|:----:|------|
| page-linear | ☑ | PASS |
| page-parallel | ☑ | PASS |
| page-merge-seq | ☑ | PASS |
| page-merge-par | ☑ | PASS |
| page-merge-stk | ☑ | PASS |
| page-merge-mm | ☑ | PASS |
| page-shuffle | ☑ | PASS |
| lazy-file | ☑ | PASS |
| lazy-anon | ☑ | PASS |

### mmap / munmap

| 테스트 | 완료 | 비고 |
|--------|:----:|------|
| mmap-read | ☑ | PASS |
| mmap-close | ☑ | PASS |
| mmap-unmap | ☑ | PASS |
| mmap-overlap | ☑ | PASS |
| mmap-twice | ☑ | PASS |
| mmap-write | ☑ | PASS |
| mmap-ro | ☑ | PASS |
| mmap-exit | ☑ | PASS |
| mmap-shuffle | ☑ | PASS |
| mmap-bad-fd | ☑ | PASS |
| mmap-clean | ☑ | PASS |
| mmap-inherit | ☑ | PASS |
| mmap-misalign | ☑ | PASS |
| mmap-null | ☑ | PASS |
| mmap-over-code | ☑ | PASS |
| mmap-over-data | ☑ | PASS |
| mmap-over-stk | ☑ | PASS |
| mmap-remove | ☑ | PASS |
| mmap-zero | ☑ | PASS |
| mmap-bad-fd2 | ☑ | PASS |
| mmap-bad-fd3 | ☑ | PASS |
| mmap-zero-len | ☑ | PASS |
| mmap-off | ☑ | PASS |
| mmap-bad-off | ☑ | PASS |
| mmap-kernel | ☑ | PASS |

### Swap

| 테스트 | 완료 | 비고 |
|--------|:----:|------|
| swap-file | ☑ | PASS |
| swap-anon | ☑ | PASS |
| swap-iter | ☑ | PASS |
| swap-fork | ☑ | PASS |

---

## 회귀 / Extra 체크

| 구분 | 테스트 | 완료 | 비고 |
|------|--------|:----:|------|
| Project 2 회귀 | User Programs 63개 | ☑ | `args-*`부터 `bad-*`까지 CSV 기준 PASS |
| Filesys 회귀 | `lg-*`, `sm-*`, `syn-*` 13개 | ☑ | PASS |
| Threads 회귀 | `alarm-*` 6개 | ☑ | PASS |
| Threads 회귀 | `priority-*` 12개 | ☑ | PASS |
| Extra | cow-simple | □ | FAIL, 필수 범위와 분리된 Project 3 extra |

---

## 주차 종료 (2주차 목요일)

| 항목 | 완료 | 비고 |
|------|:----:|------|
| 주간 공유 발표 자료 제출 | ☑ | 목요일 정오, 팀당 1명 |
| 주간 공유 발표 준비 | ☑ | 목요일 오전 10시, 개인별 2분 발표 |
| 프로젝트 구현 및 트러블슈팅 정리 | ☑ | 발표 자료 포함 |
| 핵심 역량 목표 달성률 평가 (goal-achievement.md) | ☑ | 작성 완료 |
| WIL 작성 (wil.md) | ☑ | 작성 완료로 가정 |
| WIL 블로그 포럼 등록 | ☑ | WEEK11-12 태그 |
| 11-12주차 동료피드백 | ☑ | 2주차 목요일 오후 1시 발제 시간 |
| 운영진 티타임 참석 | ☑ | 2주차 목요일 오후 3시 |
