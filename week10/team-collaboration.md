# 팀 협업 룰

> 작성 시점: 주차 시작 시 (매주 첫 스크럼 미팅에서 팀 논의 후 작성)
>
> 필요 시 언제든 팀 논의를 거쳐 수정할 수 있습니다.

---

## 1. 일정·만남

| 항목 | 우리 팀 규칙 |
|------|--------------|
| 출근 시간 | 미정 |
| 스크럼 미팅 | 10:30-10:40 |
| 점심 식사 | 11:50 |
| 저녁 식사 | 17:50 |
| 코어타임 | 필요시 코칭실 예약 |
| 코칭실 사용 | 코칭실 |
| PintOS 기초 특강 | 금요일 오전 10시, User Program |
| 퀴즈 | 화요일 14:00-15:00, PintOS/C언어 |
| 주간 공유 발표 | 목요일 오전 10시 |
| 차주 발제 | 목요일 오후 1시 |


## 2. 소통 방식

| 항목 | 우리 팀 규칙 |
|------|--------------|
| 연락 채널 | 슬랙, 카톡 |
| 응답 기준 | 당일 확인 원칙, 막힌 테스트는 실시간 공유 |
| 긴급 연락 | 전화통화 |
| 테스트 실패 공유 | 실패 테스트명, 재현 명령, 최근 수정 commit, 의심 함수, 다음 가설을 함께 공유 |


## 3. 작업·진행 관리

| 항목 | 우리 팀 규칙 |
|------|--------------|
| GitHub Projects | Project 2 학습, 구현, 테스트, 발표 자료 일정 관리 |
| 진행 상황 공유 | 슬랙 + 스크럼 미팅 + 코어타임 PR 리뷰 |
| 이슈·태스크 관리 | argument passing, syscall, user pointer validation, file descriptor, file syscall, fork/exec/wait, robustness, 발표 자료로 분리 |
| 테스트 관리 | 통과/실패 테스트 목록을 매일 갱신 |
| Project 1 회귀 | CSV에 포함된 alarm/priority 테스트는 필요 시 회귀 확인 |


## 4. 코드·리뷰

| 항목 | 우리 팀 규칙 |
|------|--------------|
| 브랜치 전략 | 개인 branch에서 작업 후 PR 생성 -> dev에 머지 -> 검증된 dev를 main에 머지 |
| PR 리뷰 | dev 머지 전에 핵심 로직, user pointer 검증, 파일 자원 정리, 테스트 결과를 함께 확인 |
| 머지 기준 | 관련 테스트 통과 + 구현 의도 설명 가능 + 팀원 1명 이상 리뷰 |
| main branch | 팀 최종 결과물 branch로 유지, 직접 push 금지 |
| 충돌 해결 | 충돌 파일 담당자가 설명 후 팀 논의로 해결 |


## 5. 코어타임 활동

| 항목 | 우리 팀 규칙 |
|------|--------------|
| PintOS 문서 학습 공유 | KAIST GitBook Project 2 User Programs, FAQ 핵심 내용 공유 |
| 테스트 실패 분석 | 실패 테스트명을 기준으로 원인 가설, 관련 코드, 수정 계획 공유 |
| 코드 리뷰 | syscall, process, file descriptor, user address validation 변경은 반드시 설명 후 리뷰 |
| 발표 준비 | 구현 및 트러블슈팅, 팀 회고를 주중에 누적 정리 |


## 6. 기타 협업 팁

- Project 2는 팀 최종 결과물이 하나의 main branch이므로, 개인 구현도 팀 코드 품질 기준에 맞춰 리뷰한다.
- 테스트를 통과한 경우에도 왜 통과했는지 설명할 수 없으면 병합하지 않는다.
- user pointer 검증, fd table, fork/exec/wait, file lock, executable write deny는 관련 테스트를 묶어서 리뷰한다.
- 작은 변경이라도 어떤 테스트를 통과시키기 위한 수정인지 commit 또는 PR에 남긴다.
- bad pointer 계열 테스트는 임시 조건문보다 일관된 user address 검증 정책으로 해결한다.
- 발표 자료는 마지막 날에 몰아서 만들지 않고, 구현·트러블슈팅이 발생할 때마다 근거를 누적한다.


## 7. 권장 진행 순서

1. `args-*`, `halt`, `exit`
2. `create/open/close`
3. `read/write`
4. `fork/exec/wait`
5. `rox-*`, `multi-*`, `bad-*`, `lg-*`, `sm-*`
6. Project 1 회귀 및 extra 테스트


## 8. 일일 팀 목표 설정

2026-05-01(금) | PintOS User Program 특강 참석 / 팀 GitHub Projects 제출 / 개인별 Project 2 User Programs 문서와 FAQ 학습 / 테스트 실행 환경 확인 / 76개 테스트 묶음 분류 / 2시간 자율 학습 후 30분 팀 토의 / argument passing 설계 착수

2026-05-02(토) | argument passing 구현 / syscall handler 기본 구조 확인

2026-05-03(일) | *팀 활동 없음(개인별 학습 및 코드 구현) / `args-*`, `halt`, `exit` 테스트 확인 / fd table 설계 / create-open-close 구현 / create/open/close 계열 테스트 확인 

2026-05-04(월) | read/write 구현 / stdin/stdout 처리 / user pointer 및 boundary 검증 / read/write, bad pointer 기본 테스트 확인 / fork 기본 구현

2026-05-05(화) | exec-load 실패 처리 / wait 기본 동기화 / fork/exec/wait 구현 및 기본 테스트 확인 / bad-* 구현 및 테스트 확인

2026-05-06(수) | 퀴즈 참여 / rox, multi, lg/sm/syn, multi-oom 등 robustness 테스트 점검 / Project 2 전체 회귀 테스트 / 발표 자료 준비

2026-05-07(목) | 주간 공유 발표 / WIL 작성 및 WEEK10 포럼 등록 / 남은 실패 테스트와 회고 정리 / 10주차 동료피드백


## 9. 각자 실시

PintOS 공식 문서 읽고 모르는 개념 질문 정리
개인 branch에서 기능 단위 구현 후 PR 생성
테스트 실패 시 실패명, 로그, 수정 가설 기록
코어타임 전에 오늘 병합할 PR과 남은 실패 테스트 공유
주간 발표에 넣을 구현 내용과 트러블슈팅 근거 정리

