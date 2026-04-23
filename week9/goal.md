# Week 9 - 핵심 역량 목표 설정

> 📅 작성 시점: 주차 시작 시

이번 주 과제 특성에 맞춰 10가지 핵심 역량별 목표를 구체적으로 설정합니다.
**달성률 산정을 위해 목표는 수치화 가능한 형태로 작성합니다.**

---

## 1. 문제해결
- **목표:** PintOS Threads 핵심 주제 10개(`threads`, `timer sleep`, `scheduler`, `synchronization`, `semaphore`, `lock`, `condition variable`, `priority scheduling`, `priority donation`, `MLFQS`)를 학습하고 Project 1 구현 흐름에 적용
- **달성률 산식:** (학습 후 적용한 핵심 주제 수 / 10) × 100


## 2. 설계
- **목표:** Project 1 구현 전 **alarm clock 대기 구조, ready list 우선순위 정렬, preemption 조건, priority donation 전파 방식, semaphore/lock/condvar 우선순위 처리, MLFQS 계산 흐름, 테스트 전략** 등 설계 포인트 **7개 이상**을 이슈·PR·발표 메모에 명시
- **달성률 산식:** (문서화한 설계 포인트 수 / 7) × 100


## 3. 구현
- **목표:** Project 1 핵심 구현 5개(`alarm clock`, `priority scheduling`, `priority donation`, `synchronization primitive 우선순위 처리`, `MLFQS`) 완료
- **달성률 산식:** (완료한 구현 항목 수 / 5) × 100


## 4. 품질
- **목표:** Project 1 테스트 27개(`alarm-*` 6개 + `priority-*` 12개 + `mlfqs-*` 9개) 통과
- **달성률 산식:** (통과한 테스트 수 / 27) × 100


## 5. 유지보수
- **목표:** 이번 주(금~목) **7일 중 5일 이상** 팀 repository 개인 branch에 의미 있는 commit을 남기고, PR 또는 리뷰 기록으로 구현·테스트 변경 사항을 추적
- **달성률 산식:** (commit 또는 PR 기록이 있는 일수 / 5) × 100 (100% 초과 시 100%로 계산)


## 6. 협업
- **목표:** 팀 GitHub Projects/Repository 준비(1개) + 개인 branch 작업(1개) + 매일 코어타임 PR 리뷰(1개) + 주간 공유 발표 자료 통합(1개) = **4개 완료**
- **달성률 산식:** (완료한 협업 항목 수 / 4) × 100


## 7. 태도
- **목표:** 공통 6개(목표 수립, 달성률 평가, AI 원칙, 업무 검토, WIL, 팀 협업 룰) 중 **6개 완료**
- **달성률 산식:** (완료한 공통 과제 수 / 6) × 100


## 8. 비즈니스 이해
- **목표:** PintOS Threads 구현이 운영체제의 **스레드 스케줄링, 동기화, race condition 방지, 우선순위 역전 해결**과 어떤 관련이 있는지 WIL 또는 발표 자료에 **4문장 이상** 명시
- **달성률 산식:** (명시한 문장 수 / 4) × 100


## 9. AI 활용
- **목표:** AI 활용 원칙 수립(1개) + PintOS 개념 설명 검증(1개) + 테스트 실패 분석 또는 코드 리뷰 보조 활용 기록(1개) = **3개 완료**
- **달성률 산식:** (완료한 AI 활용 항목 수 / 3) × 100


## 10. 학습 민첩성
- **목표:** `thread lifecycle`, `context switch`, `timer interrupt`, `busy waiting`, `semaphore`, `lock`, `condition variable`, `priority inversion`, `priority donation`, `nested donation`, `fixed-point arithmetic`, `MLFQS`를 포함해 **12개 이상 개념/구현 포인트**를 학습 후 적용
- **달성률 산식:** (학습 후 적용 완료한 개념/구현 포인트 수 / 12) × 100


---

### 달성률 산식 참고
- 문제해결: PintOS Threads 핵심 개념과 구현 흐름 단위로 카운트
- 설계: 이슈, PR 설명, 발표 메모에 남긴 설계 포인트 수 기준
- 구현: Project 1 핵심 기능 단위로 카운트
- 품질: Project 1 테스트 통과 수 기준
- 유지보수: 일 단위 commit 또는 PR 기록 기준
- 협업/태도/AI 활용: 완료 항목 수 기준
- 비즈니스 이해: WIL 또는 발표 자료에 명시한 문장 수 기준
