# AI 활용 원칙 (Week 10)

## 과제별 AI 사용 범위

| 과제 | AI 사용 | 필수 조건 |
|------|---------|-----------|
| **PintOS Project 2 - User Programs** | 학습 보조 중심 | ELF loader, syscall, file descriptor, user pointer validation, fork/exec/wait 흐름을 직접 설명할 수 있어야 함 |
| **테스트 실패 분석** | 제한적 허용 | 실패 로그 해석, 디버깅 방향, 관련 코드 위치 추정까지만 활용하고 수정 내용은 직접 검증 |
| **주간 공유 발표 / WIL** | 허용 | 구현 내용과 트러블슈팅을 과장 없이 정리 |
| **퀴즈 / 제한 환경 실습** | 미허용 | AI·인터넷 사용 제한 환경 가정 |

## 우리 팀 선택 (PintOS 학습 과제)

| 수준 | 허용 | 선택 |
|------|------|------|
| 일부 사용 | 공식 문서 이해, 용어 정리, 오류 메시지 해석 | □ |
| 학습 보조 | 위 + 테스트 실패 원인 분석, 디버깅 방향, 코드 리뷰 관점 정리 | ☑ |
| 코드 생성 | 위 + 핵심 구현 코드 생성 | □ |

- PintOS Project 2는 user/kernel boundary와 process/file 자원 관리를 직접 이해하는 것이 핵심이므로, AI를 코드 생성기가 아니라 학습·검증 보조 도구로 사용한다.

## 핵심 원칙

- AI가 설명한 내용은 반드시 KAIST PintOS 문서, 코드, 테스트 결과와 대조한다.
- `argument passing`, `syscall`, `user pointer validation`, `file descriptor`, `fork`, `exec`, `wait` 핵심 구현은 이해 없이 복사하지 않는다.
- 테스트 통과 여부보다 왜 통과했는지, 어떤 user/kernel boundary를 검증했는지, 어떤 자원을 정리했는지 설명할 수 있어야 한다.
- AI가 제안한 디버깅 방향은 팀 PR 리뷰와 테스트 재실행으로 검증한 뒤 반영한다.
- 퀴즈나 제한 환경에서는 AI를 사용하지 않는다.

## 허용하는 것

- PintOS 공식 문서의 어려운 개념 요약 요청
- ELF loader, user mode/kernel mode, syscall entry, virtual memory layout 개념 설명 요청
- file descriptor table, stdin/stdout, process parent-child sync, fork/exec/wait 흐름 설명 요청
- user pointer, bad pointer, boundary 테스트 실패 로그 해석과 의심 코드 경로 정리
- PR 리뷰 체크리스트, 발표 자료, WIL 문장 정리 보조

## 허용하지 않는 것

- Project 2 핵심 구현 코드를 이해 없이 AI로 생성해 그대로 붙여 넣기
- 테스트 실패 원인을 확인하지 않고 AI가 제안한 수정만 적용하기
- 설명할 수 없는 syscall, pointer validation, process control, fd table 코드를 main branch에 병합하기
- 퀴즈 또는 제한 환경에서 AI 사용하기

## 예외 상황

- Docker, Ubuntu 22.04, 빌드 도구, 테스트 실행 명령처럼 환경 설정 문제는 AI 도움을 받을 수 있다.
- 단, 환경 설정 도움과 별개로 PintOS 핵심 구현 로직은 팀원이 직접 이해하고 PR에서 설명해야 한다.

