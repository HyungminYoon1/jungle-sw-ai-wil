# Week 15-16 구현 계획

> 기준 기간: 2026-06-05 ~ 2026-06-18
>
> 과제: AI 응용 기술을 활용한 게시판 구현
>
> 수정 기준: 2026-06-10까지는 AI 기능이 붙지 않은 기본 게시판 템플릿을 각자 만들며 React, FastAPI, PostgreSQL 기반 게시판 제작에 익숙해진다. 2026-06-11부터는 새 팀 공통 repository의 main을 기준으로 각자 branch를 분기해 개인 주제별 AI 기능을 붙인다.

---

## 1. 전체 진행 방식

이번 주차 구현은 두 단계로 나누어 진행한다.

| 단계 | 기간 | 목표 | AI 사용 기준 |
|------|------|------|--------------|
| 1단계: 기본 게시판 숙련 | 2026-06-05 ~ 2026-06-10 | 특별한 주제가 필요 없는 기본 게시판 템플릿을 각자 구현 | AI 사용 최소화 |
| 2단계: AI 기능 확장 | 2026-06-11 ~ 2026-06-18 | 새 팀 공통 템플릿 repository에서 개인 branch별 RAG/MCP/Agent 구현 | AI 기능 구현 보조 허용 |

1단계의 목적은 완성도 높은 서비스를 만드는 것이 아니라, React 화면, FastAPI API, PostgreSQL DB가 연결되는 기본 게시판 흐름을 직접 익히는 것이다.

2단계의 목적은 공통 템플릿 위에서 각자 다른 주제와 AI 기능을 실험하고, RAG, MCP, Agent의 입력, 처리, 출력, 실패 케이스를 설명 가능한 형태로 구현하는 것이다.

---

## 2. 확정 사항과 미정 사항

| 구분 | 내용 |
|------|------|
| 과제 성격 | 개인 과제 |
| 1단계 작업 방식 | 각자 AI 없는 기본 게시판 템플릿 구현 |
| 2단계 repository | 2026-06-11부터 사용할 새 팀 공통 repository |
| 2단계 branch 방식 | main에서 각자 branch를 최초 1회 분기 |
| branch 병합 기준 | 개인 branch는 다시 main으로 합치지 않음 |
| 프론트엔드 | React |
| 백엔드 | FastAPI |
| 데이터베이스 | PostgreSQL |
| Vector store | PostgreSQL + pgvector |
| LLM/API | OpenAI API |
| 개인 README 위치 | 미정, 차후 팀 결정 후 반영 |
| 발표자 | 진행 상황을 보고 차후 결정 |

미정 사항은 구현을 막는 요소가 아니므로, 1단계에서는 기본 게시판 사용자 흐름을 직접 구현하고, 2단계에서 개인 AI 기능 범위를 확정한다.

---

## 3. 1단계 아키텍처: 기본 게시판

1단계에서는 AI 기능을 붙이지 않는다.

```text
사용자
-> React
-> FastAPI router
-> Service
-> Repository
-> PostgreSQL
-> FastAPI response
-> React 화면 반영
```

역할 분리는 다음 기준으로 유지한다.

| 영역 | 역할 |
|------|------|
| React | 화면, 사용자 입력, API 호출, 응답 데이터 표시 |
| FastAPI router | HTTP 요청/응답, 인증 확인, 입력 검증 |
| Service | 게시판 비즈니스 로직, 권한 판단 |
| Repository | PostgreSQL 조회/저장 |
| PostgreSQL | 사용자, 게시글, 댓글, 태그 저장 |

1단계에서는 RAG, MCP, Agent 코드를 구현하지 않는다. 필요하다면 2단계 확장을 고려한 폴더 이름이나 placeholder 정도만 둘 수 있지만, 기본 게시판 구현보다 우선하지 않는다.

---

## 4. 2단계 아키텍처: AI 기능 확장

2단계에서는 새 팀 공통 템플릿 repository의 main에서 개인 branch를 분기한 뒤, 각자 주제에 맞춰 AI 기능을 붙인다.

```text
React
-> FastAPI API
-> Agent
-> RAG retriever
-> pgvector 검색
-> MCP client/server
-> 외부 API
-> OpenAI API
-> 분석 결과 저장
-> React 화면 표시
```

AI 기능 역할은 다음과 같이 분리한다.

| 영역 | 역할 |
|------|------|
| RAG module | 데이터 소스 선정, chunking, embedding, pgvector retrieval, context 구성 |
| MCP module | 외부 API를 tool 형태로 연결, JSON-RPC 요청/응답, 실패 처리 |
| Agent module | 사용자 목표 해석, tool 선택, 상태 관리, 종료 조건 관리 |
| AI service | RAG/MCP/Agent 흐름을 FastAPI API와 연결 |

---

## 5. 1단계 기본 게시판 구현 계획

2026-06-10까지 구현할 기본 게시판은 특별한 주제를 갖지 않는다. 목적은 게시판 제작 흐름에 익숙해지는 것이다.

| 기능 | 구현 내용 |
|------|-----------|
| 회원가입 | 이메일/비밀번호 또는 팀 기준 계정 생성, 입력 검증 |
| 로그인 | 인증 토큰 또는 세션 생성, 인증 상태 유지 |
| 로그아웃 | 인증 상태 정리 |
| 게시글 생성 | 제목, 본문, 태그 입력 |
| 게시글 조회 | 목록, 상세, 작성자 정보, 태그 표시 |
| 게시글 수정/삭제 | 작성자 권한 확인 |
| 댓글 | 생성, 조회, 수정, 삭제, 권한 확인 |
| 태그 | 게시글 태그 등록, 태그별 조회 |
| 페이징 | 목록 페이지 단위 조회 |
| 검색 | 제목, 본문, 태그 기준 검색 |

완료 기준:

- React 화면에서 회원가입부터 게시글, 댓글, 검색, 페이징까지 직접 실행한다.
- FastAPI route, service, repository, DB model의 역할을 설명할 수 있다.
- PostgreSQL에 어떤 테이블이 있고 어떤 관계를 갖는지 설명할 수 있다.
- AI 도움 없이 또는 최소한의 도움만 받아 구현 흐름을 이해한다.

---

## 6. 1단계 DB 설계 계획

기본 게시판 테이블은 다음을 기준으로 한다.

| 테이블 | 목적 |
|--------|------|
| users | 사용자 계정 |
| posts | 게시글 |
| comments | 댓글 |
| tags | 태그 |
| post_tags | 게시글과 태그 연결 |

1단계에서는 AI 분석 결과, embedding, agent 실행 기록 테이블을 필수로 만들지 않는다.

다만 2단계에서 확장할 수 있도록 다음 질문을 남긴다.

```text
게시글과 댓글 중 어떤 데이터를 embedding할 것인가?
AI 분석 결과는 게시글별로 저장할 것인가, 사용자 요청별로 저장할 것인가?
외부 API에서 가져온 자료의 출처와 조회 시각은 어디에 저장할 것인가?
```

---

## 7. 2단계 RAG 구현 계획

RAG의 목표는 게시판 내부 데이터 또는 개인 주제 데이터에서 관련 근거를 찾고, 그 근거를 LLM 답변에 반영하는 것이다.

구현 흐름:

```text
게시글/댓글/참조 자료 저장
-> chunk 생성
-> OpenAI embedding 생성
-> pgvector에 원문 메타데이터와 embedding 저장
-> 사용자 질문 또는 게시글 내용 입력
-> 입력 embedding 생성
-> pgvector 유사도 검색
-> 검색 결과를 context로 구성
-> OpenAI API로 답변 생성
-> 근거와 함께 화면 표시
```

우선 구현할 기능 후보:

| 기능 | 설명 |
|------|------|
| 유사 게시글 추천 | 작성 중인 글 또는 상세 글과 비슷한 기존 글 추천 |
| 지식 Q&A | 게시판 데이터 또는 외부 참조 자료 기반 질문 답변 |
| 중복 글 방지 | 새 글 작성 시 유사도가 높은 기존 글 안내 |

완료 기준:

- 어떤 데이터를 embedding했는지 설명할 수 있다.
- retrieved context를 확인할 수 있다.
- 검색 결과가 없을 때의 응답을 처리한다.
- 답변에 근거 제목, 링크, 출처 정보를 함께 표시한다.

---

## 8. 2단계 MCP 구현 계획

MCP의 목표는 외부 API를 AI가 사용할 수 있는 tool로 연결하는 것이다.

외부 서비스는 개인 아이디어에 맞춰 2026-06-11 이후 최종 선택한다. API 접근이 어렵거나 과제 범위에 비해 무거우면 공개 날씨 API, GitHub API, URL metadata API 등 작은 외부 서비스로 축소한다.

예상 MCP tool:

| Tool | 목적 |
|------|------|
| search_external_sources | 외부 API에서 관련 자료 검색 |
| get_external_source_detail | 선택한 자료의 상세 내용 조회 |
| summarize_external_source | 외부 자료를 게시판에 표시할 수 있게 요약 |

구현 흐름:

```text
Agent 또는 FastAPI service
-> MCP client
-> JSON-RPC request
-> MCP server
-> 외부 API 호출
-> JSON-RPC response
-> Agent 또는 RAG context에 반영
```

완료 기준:

- tool name, input schema, output schema를 설명할 수 있다.
- 최소 1개 외부 서비스 호출이 실제로 동작한다.
- 외부 API 실패, 권한 오류, 빈 결과를 처리한다.
- API key가 필요한 경우 값은 `.env`에만 두고 출력하지 않는다.

---

## 9. 2단계 Agent 구현 계획

Agent의 목표는 사용자의 게시글 또는 질문을 받아 필요한 tool을 선택하고, RAG/MCP 결과를 조합해 최종 응답을 만드는 것이다.

우선 구현할 Agent 기능:

| 기능 | 설명 |
|------|------|
| 게시글 분석 Agent | 게시글 내용을 요약하고 쟁점 또는 키워드를 추출 |
| 근거 검색 Agent | RAG와 MCP tool 중 필요한 도구를 선택 |
| 작성 보조 Agent | 유사 글, 외부 자료, 추천 태그를 바탕으로 초안 또는 보완점을 제안 |

상태 관리:

| 상태 | 내용 |
|------|------|
| user_input | 사용자 질문 또는 게시글 본문 |
| intent | 사용자가 원하는 작업 |
| search_queries | Agent가 생성한 검색어 |
| tool_results | RAG/MCP 호출 결과 |
| final_answer | 최종 응답 |
| stop_reason | 정상 종료, 최대 반복 도달, tool 실패 등 |

안전 기준:

- 최대 tool 호출 횟수를 제한한다.
- DB 저장, 수정, 삭제는 Agent가 자동 실행하지 않는다.
- 근거가 부족하면 단정하지 않고 추가 확인 질문을 만든다.
- tool 실패 시 실패 사유와 대체 응답을 제공한다.

---

## 10. API 설계 계획

1단계 기본 게시판 API:

| Method | Path | 목적 |
|--------|------|------|
| POST | `/api/auth/signup` | 회원가입 |
| POST | `/api/auth/login` | 로그인 |
| GET | `/api/posts` | 게시글 목록/검색/페이징 |
| POST | `/api/posts` | 게시글 생성 |
| GET | `/api/posts/{post_id}` | 게시글 상세 |
| PATCH | `/api/posts/{post_id}` | 게시글 수정 |
| DELETE | `/api/posts/{post_id}` | 게시글 삭제 |
| POST | `/api/posts/{post_id}/comments` | 댓글 생성 |
| PATCH | `/api/posts/{post_id}/comments/{comment_id}` | 댓글 수정 |
| DELETE | `/api/posts/{post_id}/comments/{comment_id}` | 댓글 삭제 |

2단계 AI 확장 API 후보:

| Method | Path | 목적 |
|--------|------|------|
| GET | `/api/posts/{post_id}/ai/similar` | 유사 게시글 추천 |
| POST | `/api/posts/{post_id}/ai/analyze` | Agent 기반 분석 요청 |
| GET | `/api/ai/requests/{request_id}` | AI 분석 상태 조회 |
| GET | `/api/ai/results/{result_id}` | AI 분석 결과 조회 |

공통 템플릿의 실제 route가 다르면 위 구조를 그대로 강제하지 않고 기존 naming을 따른다.

---

## 11. React 화면 계획

1단계 기본 화면:

| 화면 | 내용 |
|------|------|
| 로그인/회원가입 | 인증 |
| 게시글 목록 | 검색, 페이징, 태그 |
| 게시글 작성 | 제목, 본문, 태그 |
| 게시글 상세 | 본문, 댓글 |
| 게시글 수정 | 작성자 권한 확인 후 수정 |

2단계 AI 확장 화면:

| 화면 | 내용 |
|------|------|
| AI 분석 결과 | 요약, 근거, 외부 자료, 추가 확인 사항 |
| 유사 게시글 추천 | RAG 검색 결과 |
| Agent 실행 상태 | tool 선택, 실패 처리, 종료 사유 |

---

## 12. 검증 계획

기능 완료는 코드 작성이 아니라 직접 실행 결과를 기준으로 판단한다.

| 구분 | 검증 항목 |
|------|-----------|
| 1단계 기본 게시판 | 회원가입, 로그인, 게시글 CRUD, 댓글, 태그, 검색, 페이징 |
| 1단계 구조 이해 | React, FastAPI, service/repository, PostgreSQL 흐름 설명 |
| 2단계 RAG | embedding 저장, 유사도 검색, context 확인, 답변 생성 |
| 2단계 MCP | JSON-RPC 요청/응답, 외부 API 성공/실패 처리 |
| 2단계 Agent | tool 선택 이유, 상태 변경, 최대 반복 제한, 종료 조건 |
| 보안 | `.env` 미커밋, 로그 내 비밀값 미노출 |
| 비용 | OpenAI/AWS 사용량 확인, 불필요한 리소스 종료 |

---

## 13. 일정

| 날짜 | 목표 |
|------|------|
| 2026-06-06 ~ 2026-06-08 | React, FastAPI, PostgreSQL 기본 흐름 학습 및 개인 기본 게시판 구현 시작 |
| 2026-06-09 | AWS 특강 참석, 로컬/클라우드 실행과 비용 관리 관점 정리 |
| 2026-06-10 | AI 없는 기본 게시판 템플릿 1차 완성, 사용자 흐름 점검 |
| 2026-06-11 | 새 팀 공통 repository에서 main 기준 개인 branch 분기, 개인 AI 주제 확정 |
| 2026-06-12 | OpenAI Agent 워크샵 참석, Agent 구조 보완 |
| 2026-06-13 ~ 2026-06-14 | RAG 설계/구현, pgvector retrieval 연결 |
| 2026-06-15 | MCP server/tool 구현, 외부 API 연동 |
| 2026-06-16 | Agent 통합, 실패 케이스 처리 |
| 2026-06-17 | README, 데모 시나리오, 발표 자료 정리 |
| 2026-06-18 | 발표 전 최종 점검 |

---

## 14. README와 발표 정리 기준

README에는 과제 공지의 제출 항목을 빠짐없이 반영한다.

| 항목 | 포함 내용 |
|------|-----------|
| 프로젝트 개요 | 서비스 목적, 사용자 문제 |
| 주요 구현 기능 | 기본 게시판, RAG, MCP, Agent |
| 전체 아키텍처 | React, FastAPI, PostgreSQL, pgvector, OpenAI API |
| 기본 게시판 구조 | 인증, 게시글, 댓글, 태그, 검색, 페이징 |
| RAG 구조 | 데이터 소스, embedding, vector 검색, 답변 생성 |
| MCP 구조 | MCP server, tool schema, 외부 API |
| Agent 구조 | 상태, tool 선택, 종료 조건 |
| 데모 | 스크린샷 1개 이상 또는 실제 시연 흐름 |
| 회고 | 1단계 기본 게시판 숙련, 2단계 AI 확장 한계와 개선 아이디어 |

공지 문서의 `WEEK13-14` 표기는 복붙 오기로 보고, 주차 문서와 제출 정리는 `WEEK15-16` 기준으로 작성한다.

