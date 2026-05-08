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
| 주간 공유 발표 | 2026-05-14(목), 2026-05-21(목) 오전 10시. 팀 발표 자료와 노트북은 1개로 준비하고 개인별 2분 발표 |
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
| 진행 상황 공유 | 슬랙 + 스크럼 미팅 + 4인 공동 구현 세션 + 코어타임 PR 리뷰 |
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

작업은 담당자별 장기 분업이 아니라 Virtual Memory 구현 흐름과 테스트 묶음을 기준으로 4명이 함께 순차 진행한다. 한 세션에서는 Driver 1명과 공동 검토자 3명을 두고, 역할은 30-60분 단위로 교체한다.

| 작업 묶음 | 관련 테스트 | 중점 확인 포인트 |
|------|------|------|
| 기준선 / 회귀 상태 확인 | `args-*`, `halt`, `exit`, file syscall 일부 | Project 1/2 구현이 VM에서 전제가 되는 지점 |
| SPT / frame claim | 빌드, `spt_find_page` 흐름 리뷰 | `pg_round_down()`, SPT insert/find/remove, page-frame 연결 |
| Lazy loading | `lazy-file`, `lazy-anon`, 기본 userprog 테스트 | uninit page initializer, aux lifetime, ELF segment lazy load |
| Page fault / stack growth | `pt-bad-*`, `pt-write-code*`, `pt-grow-*` | valid fault와 invalid fault 구분, stack growth 조건 |
| SPT copy / fork cleanup | `fork-*`, `multi-*`, `page-linear`, `page-merge-*` | child SPT 처리, loaded/uninit page 복제, process exit 자원 정리 |
| mmap / file-backed page | `mmap-*`, `mmap-inherit`, `swap-file` | fd와 file object lifetime, dirty write-back, munmap cleanup |
| Eviction / swap | `page-shuffle`, `page-merge-*`, `swap-*` | frame table, victim selection, swap slot 할당/회수, swap in/out |
| 전체 회귀 테스트 | userprog, threads alarm/priority, filesys/base | Project 1/2와 기본 파일 시스템 동작 유지 |
| Extra 판단 | `cow-simple` | 필수 VM 안정화 이후 COW 진행 여부 결정 |


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
