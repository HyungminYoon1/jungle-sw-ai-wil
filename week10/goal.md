# Week 10 - 핵심 역량 목표 설정

> 작성 시점: 주차 시작 시

이번 주 과제 특성에 맞춰 10가지 핵심 역량별 목표를 구체적으로 설정합니다.
**달성률 산정을 위해 목표는 수치화 가능한 형태로 작성합니다.**

---

## 1. 문제해결
- **목표:** PintOS User Programs 핵심 주제 10개(`User Program`, `User mode`, `Kernel mode`, `ELF loader`, `argument passing`, `system call`, `file descriptor`, `user pointer validation`, `fork/exec/wait`, `Virtual Memory Layout`)를 학습하고 Project 2 구현 흐름에 적용
- **달성률 산식:** (학습 후 적용한 핵심 주제 수 / 10) × 100


## 2. 설계
- **목표:** Project 2 구현 전 **argument stack 구성, syscall dispatch, user address 검증, file descriptor table, process parent-child sync, fork context 복제, exec/load 실패 처리, write deny 정책, 테스트 전략** 등 설계 포인트 **9개 이상**을 이슈·PR·발표 메모에 명시
- **달성률 산식:** (문서화한 설계 포인트 수 / 9) × 100


## 3. 구현
- **목표:** Project 2 핵심 구현 7개(`argument passing`, `halt/exit`, `user pointer validation`, `file descriptor table`, `file syscalls`, `fork/exec/wait`, `robustness 처리`) 완료
- **달성률 산식:** (완료한 구현 항목 수 / 7) × 100


## 4. 품질
- **목표:** CSV 기준 Project 2 필수 테스트 76개 통과
- **달성률 산식:** (통과한 Project 2 필수 테스트 수 / 76) × 100


## 5. 유지보수
- **목표:** 이번 주(금~목) **7일 중 5일 이상** 팀 repository 개인 branch에 의미 있는 commit을 남기고, PR 또는 리뷰 기록으로 구현·테스트 변경 사항을 추적
- **달성률 산식:** (commit 또는 PR 기록이 있는 일수 / 5) × 100 (100% 초과 시 100%로 계산)


## 6. 협업
- **목표:** 팀 GitHub Projects 준비(1개) + 개인 branch 작업(1개) + 매일 코어타임 PR 리뷰(1개) + 테스트 실패 공유 로그(1개) + 주간 공유 발표 자료 통합(1개) = **5개 완료**
- **달성률 산식:** (완료한 협업 항목 수 / 5) × 100


## 7. 태도
- **목표:** 공통 6개(목표 수립, 달성률 평가, AI 원칙, 업무 검토, WIL, 팀 협업 룰) 중 **6개 완료**
- **달성률 산식:** (완료한 공통 과제 수 / 6) × 100


## 8. 비즈니스 이해
- **목표:** User Programs 구현이 운영체제의 **사용자 프로그램 실행, user/kernel boundary, system call API, 파일 자원 관리, 프로세스 생명주기 관리**와 어떤 관련이 있는지 WIL 또는 발표 자료에 **5문장 이상** 명시
- **달성률 산식:** (명시한 문장 수 / 5) × 100


## 9. AI 활용
- **목표:** AI 활용 원칙 수립(1개) + Project 2 개념 설명 검증(1개) + 테스트 실패 분석 또는 코드 리뷰 보조 활용 기록(1개) = **3개 완료**
- **달성률 산식:** (완료한 AI 활용 항목 수 / 3) × 100


## 10. 학습 민첩성
- **목표:** `syscall entry`, `intr_frame`, `argument stack`, `ELF loading`, `file descriptor table`, `stdin/stdout`, `bad pointer`, `boundary check`, `fork`, `exec`, `wait`, `deny write executable`을 포함해 **12개 이상 개념/구현 포인트**를 학습 후 적용
- **달성률 산식:** (학습 후 적용 완료한 개념/구현 포인트 수 / 12) × 100


---

### 달성률 산식 참고
- 문제해결: User Programs 핵심 개념과 구현 흐름 단위로 카운트
- 설계: 이슈, PR 설명, 발표 메모에 남긴 설계 포인트 수 기준
- 구현: Project 2 핵심 기능 단위로 카운트
- 품질: CSV 기준 Project 2 필수 테스트 76개 통과 수 기준
- 유지보수: 일 단위 commit 또는 PR 기록 기준
- 협업/태도/AI 활용: 완료 항목 수 기준
- 비즈니스 이해: WIL 또는 발표 자료에 명시한 문장 수 기준

