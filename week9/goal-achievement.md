# Week 9 - 핵심 역량 목표 달성률 평가

> 작성 시점: 주차 마무리 시점

한 주가 마무리되는 시점에 목표에 대한 달성률을 핵심 역량별로 기입합니다.
이번 평가는 `local/week9_pintos_team_code/week09-team-07-pintos-threads` 코드와 `303_7팀_9주차_발표자료.pdf`를 기준으로 작성했습니다.

---

## 1. 문제해결
- **목표:** PintOS Threads 핵심 주제 10개(`threads`, `timer sleep`, `scheduler`, `synchronization`, `semaphore`, `lock`, `condition variable`, `priority scheduling`, `priority donation`, `MLFQS`)를 학습하고 Project 1 구현 흐름에 적용
- **달성률:** 60% (6 / 10)
- **회고:** `timer_sleep()`의 busy waiting 제거, `sleep_list`와 `wakeup_tick` 기반 alarm clock, `ready_list` 우선순위 정렬, preemption, semaphore waiters 우선순위 처리까지는 코드에 적용했다. 다만 condition variable 우선순위 처리, priority donation, MLFQS는 테스트 기준으로 완료되지 못해 핵심 주제 전체 적용에는 도달하지 못했다.


## 2. 설계
- **목표:** Project 1 구현 전 **alarm clock 대기 구조, ready list 우선순위 정렬, preemption 조건, priority donation 전파 방식, semaphore/lock/condvar 우선순위 처리, MLFQS 계산 흐름, 테스트 전략** 등 설계 포인트 **7개 이상**을 이슈·PR·발표 메모에 명시
- **달성률:** 100% (7 / 7)
- **회고:** 구현 계획 문서와 발표 자료에서 alarm, priority scheduling, synchronization waiters, donation, MLFQS, 테스트 분할, 병합 기준을 설계 포인트로 정리했다. 실제 구현 완료 범위는 제한적이었지만, 무엇을 어떤 순서로 구현해야 하는지와 실패 시 의심 지점을 문서화했다.


## 3. 구현
- **목표:** Project 1 핵심 구현 5개(`alarm clock`, `priority scheduling`, `priority donation`, `synchronization primitive 우선순위 처리`, `MLFQS`) 완료
- **달성률:** 40% (2 / 5)
- **회고:** `alarm clock`과 기본 `priority scheduling`은 구현되어 관련 테스트를 통과했다. `synchronization primitive`는 semaphore waiters 우선순위 처리는 반영했지만 `priority-condvar`가 실패해 전체 완료로 보기는 어렵고, `priority donation`과 `MLFQS`는 코드상 미구현 상태로 남았다.


## 4. 품질
- **목표:** Project 1 테스트 27개(`alarm-*` 6개 + `priority-*` 12개 + `mlfqs-*` 9개) 통과
- **달성률:** 37% (10 / 27)
- **회고:** `alarm-*` 6개는 모두 PASS였고, priority 테스트 중 `priority-change`, `priority-fifo`, `priority-preempt`, `priority-sema` 4개를 통과했다. `priority-condvar`와 `priority-donate-*` 7개는 실패했고, MLFQS 9개는 발표 자료 기준 전체 미완료로 정리되어 품질 목표는 부분 달성에 그쳤다.


## 5. 유지보수
- **목표:** 이번 주(금~목) **7일 중 5일 이상** 팀 repository 개인 branch에 의미 있는 commit을 남기고, PR 또는 리뷰 기록으로 구현·테스트 변경 사항을 추적
- **달성률:** 100% (5 / 5)
- **회고:** 팀 repository에는 2026-04-25부터 2026-04-29까지 5일간 커밋 기록이 남아 있고, alarm clock, priority scheduling, semaphore waiters, 브랜치 정책, 문서 작업이 의미 단위로 추적되었다. 브랜치 보호와 PR 흐름도 발표 자료에서 공유했다.


## 6. 협업
- **목표:** 팀 GitHub Projects/Repository 준비(1개) + 개인 branch 작업(1개) + 매일 코어타임 PR 리뷰(1개) + 주간 공유 발표 자료 통합(1개) = **4개 완료**
- **달성률:** 100% (4 / 4)
- **회고:** 팀 repository와 GitHub Projects를 준비했고, `main <- dev <- 개인 브랜치` 흐름으로 작업했다. 발표 자료에는 브랜치 보호 정책, GitHub Projects 분류, alarm과 priority 구현 흐름, 팀 회고가 통합되어 협업 산출물은 목표를 충족했다.


## 7. 태도
- **목표:** 공통 6개(목표 수립, 달성률 평가, AI 원칙, 업무 검토, WIL, 팀 협업 룰) 중 **6개 완료**
- **달성률:** 100% (6 / 6)
- **회고:** 목표 수립, AI 활용 원칙, 체크리스트 기반 업무 검토, 팀 협업 룰, 달성률 평가를 정리했고, WIL은 주차 마무리 산출물로 작성할 예정이다.


## 8. 비즈니스 이해
- **목표:** PintOS Threads 구현이 운영체제의 **스레드 스케줄링, 동기화, race condition 방지, 우선순위 역전 해결**과 어떤 관련이 있는지 WIL 또는 발표 자료에 **4문장 이상** 명시
- **달성률:** 50% (2 / 4)
- **회고:** 발표 자료 2쪽, 6-8쪽, 10-13쪽에서 busy waiting을 blocking 구조로 바꾼 이유, ready list가 priority 기준으로 관리되어야 하는 이유, semaphore waiters의 우선순위 처리, 일반 thread context와 interrupt context의 yield 방식 차이를 설명했다. 다만 목표에 포함된 race condition 방지와 우선순위 역전 해결은 별도 문장으로 충분히 정리되지 않았고, priority donation도 구현되지 않았으므로 달성 항목에서 제외했다.


## 9. AI 활용
- **목표:** AI 활용 원칙 수립(1개) + PintOS 개념 설명 검증(1개) + 테스트 실패 분석 또는 코드 리뷰 보조 활용 기록(1개) = **3개 완료**
- **달성률:** 100% (3 / 3)
- **회고:** AI 활용 원칙에서 PintOS 핵심 구현은 학습 보조 중심으로 제한했다. 발표 자료에는 Codex를 활용한 GitHub Projects 이슈 분류, CodeTour 기반 코드 구조 파악, GPT를 활용한 개념 확인과 학습 보조 사례가 정리되어 있다.


## 10. 학습 민첩성
- **목표:** `thread lifecycle`, `context switch`, `timer interrupt`, `busy waiting`, `semaphore`, `lock`, `condition variable`, `priority inversion`, `priority donation`, `nested donation`, `fixed-point arithmetic`, `MLFQS`를 포함해 **12개 이상 개념/구현 포인트**를 학습 후 적용
- **달성률:** 42% (5 / 12)
- **회고:** `thread lifecycle`, `context switch`, `timer interrupt`, `busy waiting`, `semaphore`는 구현 흐름과 트러블슈팅에 적용했다. `lock`, `condition variable`, `priority inversion`, `priority donation`, `nested donation`, `fixed-point arithmetic`, `MLFQS`는 코드와 테스트 기준으로 적용 완료라고 보기 어려워 달성 항목에서 제외했다.


---

### 주간 요약

| # | 역량 | 달성률 |
|---|------|--------|
| 1 | 문제해결 | 60% |
| 2 | 설계 | 100% |
| 3 | 구현 | 40% |
| 4 | 품질 | 37% |
| 5 | 유지보수 | 100% |
| 6 | 협업 | 100% |
| 7 | 태도 | 100% |
| 8 | 비즈니스 이해 | 50% |
| 9 | AI 활용 | 100% |
| 10 | 학습 민첩성 | 42% |
| | **평균** | **73%** |

- **상위 역량:** 설계, 유지보수, 협업, 태도, AI 활용
- **개선 필요:** 구현, 품질, 비즈니스 이해, 학습 민첩성
- **핵심 미완료 항목:** `priority-condvar`, `priority donation`, `MLFQS`

