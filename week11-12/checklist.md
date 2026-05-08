# Week 11-12 - 제출물 체크리스트

> 주차별 제출물과 과제 진행 상황을 점검합니다.

---

## 주차 시작 (금요일)

| 항목 | 완료 | 비고 |
|------|:----:|------|
| 핵심 역량 목표 수립 (goal.md) | □ | |
| AI 활용 원칙 수립 (ai-principles.md) | □ | |
| 이번 주 할 일(업무) 검토 | □ | PintOS Project 3 - Virtual Memory 공지 확인 |
| 팀 협업 룰/팁 정리 및 합의 (team-collaboration.md) | □ | |
| 팀 GitHub Projects 주소 제출 | □ | 첫날 금요일 24:00, 팀당 1명 |
| 팀 GitHub repository 주소 제출 | □ | 첫날 금요일 24:00, 팀당 1명 |
| PintOS 기초 특강 - VM 참석 | □ | 금요일 오전 10시 |
| PintOS 실습 환경 확인 | □ | Ubuntu 22.04 LTS x86_64 또는 Docker |
| 테스트 실행 도구 확인 | □ | 기본 테스트 또는 pintos-util |

---

## 1주차 진행

| 과제/항목 | 완료 | 비고 |
|-----------|:----:|------|
| KAIST GitBook Project 3 Virtual Memory 학습 | □ | |
| Project 3 FAQ 학습 | □ | |
| Appendix Memory Allocation 학습 | □ | |
| Appendix Virtual Address 학습 | □ | |
| Appendix Page Table 학습 | □ | |
| 기존 Project 2 코드와 VM scaffold 확인 | □ | `vm/`, `userprog/`, `threads/` |
| supplemental page table 설계 | □ | page type, owner, lifecycle |
| page fault handler 설계 | □ | lazy load, stack growth, invalid access |
| uninit page / lazy loading 구현 | □ | `lazy-*`, `page-*` 기반 |
| stack growth 구현 | □ | `pt-grow-*` |
| anonymous page 기본 구현 | □ | swap 전 기반 |
| file-backed page 기본 설계 | □ | mmap 전 기반 |
| 1주차 테스트 실패 로그 기록 | □ | 실패명, 원인, 수정, 재실행 결과 |
| 화요일 퀴즈 준비 | □ | PintOS, C언어 |

---

## 2주차 진행

| 과제/항목 | 완료 | 비고 |
|-----------|:----:|------|
| frame table 설계 및 구현 | □ | frame allocation, owner page |
| page replacement 정책 구현 | □ | eviction victim selection |
| swap in/out 구현 | □ | swap slot 관리 |
| mmap 구현 | □ | address validation, file-backed page |
| munmap 구현 | □ | dirty write-back, page cleanup |
| fork 시 page/SPT 처리 구현 | □ | child page lifecycle |
| process exit 시 VM 자원 정리 | □ | SPT, frame, swap, mmap |
| Project 2 userprog 회귀 확인 | □ | 63개 CSV 항목 |
| filesys / threads 회귀 확인 | □ | CSV 포함 항목 |
| 주간 공유 발표 자료 초안 작성 | □ | 구현, 트러블슈팅, 회고 |
| WIL 초안 작성 | □ | WEEK11-12 태그 |
| 동료피드백 준비 | □ | 2주차 목요일 발제 시간 |

---

## Project 3 테스트 체크리스트

### Page Table / Stack Growth

| 테스트 | 완료 | 비고 |
|--------|:----:|------|
| pt-grow-stack | □ | |
| pt-grow-bad | □ | |
| pt-big-stk-obj | □ | |
| pt-bad-addr | □ | |
| pt-bad-read | □ | |
| pt-write-code | □ | |
| pt-write-code2 | □ | |
| pt-grow-stk-sc | □ | |

### Page / Lazy Loading

| 테스트 | 완료 | 비고 |
|--------|:----:|------|
| page-linear | □ | |
| page-parallel | □ | |
| page-merge-seq | □ | |
| page-merge-par | □ | |
| page-merge-stk | □ | |
| page-merge-mm | □ | |
| page-shuffle | □ | |
| lazy-file | □ | |
| lazy-anon | □ | |

### mmap / munmap

| 테스트 | 완료 | 비고 |
|--------|:----:|------|
| mmap-read | □ | |
| mmap-close | □ | |
| mmap-unmap | □ | |
| mmap-overlap | □ | |
| mmap-twice | □ | |
| mmap-write | □ | |
| mmap-ro | □ | |
| mmap-exit | □ | |
| mmap-shuffle | □ | |
| mmap-bad-fd | □ | |
| mmap-clean | □ | |
| mmap-inherit | □ | |
| mmap-misalign | □ | |
| mmap-null | □ | |
| mmap-over-code | □ | |
| mmap-over-data | □ | |
| mmap-over-stk | □ | |
| mmap-remove | □ | |
| mmap-zero | □ | |
| mmap-bad-fd2 | □ | |
| mmap-bad-fd3 | □ | |
| mmap-zero-len | □ | |
| mmap-off | □ | |
| mmap-bad-off | □ | |
| mmap-kernel | □ | |

### Swap

| 테스트 | 완료 | 비고 |
|--------|:----:|------|
| swap-file | □ | |
| swap-anon | □ | |
| swap-iter | □ | |
| swap-fork | □ | |

---

## 회귀 / Extra 체크

| 구분 | 테스트 | 완료 | 비고 |
|------|--------|:----:|------|
| Project 2 회귀 | User Programs 63개 | □ | `args-*`부터 `bad-*`까지 CSV 기준 |
| Filesys 회귀 | `lg-*`, `sm-*`, `syn-*` 13개 | □ | |
| Threads 회귀 | `alarm-*` 6개 | □ | |
| Threads 회귀 | `priority-*` 12개 | □ | |
| Extra | cow-simple | □ | Project 3 extra |

---

## 주차 종료 (2주차 목요일)

| 항목 | 완료 | 비고 |
|------|:----:|------|
| 주간 공유 발표 자료 제출 | □ | 목요일 정오, 팀당 1명 |
| 주간 공유 발표 준비 | □ | 목요일 오전 10시, 개인별 2분 발표 |
| 프로젝트 구현 및 트러블슈팅 정리 | □ | 발표 자료 포함 |
| 핵심 역량 목표 달성률 평가 (goal-achievement.md) | □ | |
| WIL 작성 (wil.md) | □ | |
| WIL 블로그 포럼 등록 | □ | WEEK11-12 태그 |
| 11-12주차 동료피드백 | □ | 2주차 목요일 오후 1시 발제 시간 |
| 운영진 티타임 참석 | □ | 2주차 목요일 오후 3시 |
