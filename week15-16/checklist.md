# Week 15-16 - 제출물 체크리스트

> 주차별 제출물과 과제 진행 상황을 점검한다.
>
> 이번 주차 게시판 구현은 개인 과제다. 소스코드는 하나의 팀 repository에서 관리하되, 각자 개인 branch에서 독립적으로 구현한다.
>
> 2026-06-18 기준 `local/week15-16_board_output/board-simple` 결과물을 읽고 완료 상태를 갱신했다.

---

## 주차 시작 (금요일, 2026-06-05)

| 항목 | 완료 | 비고 |
|------|:----:|------|
| 과제 공지 확인 | ☑ | AI 응용 기술을 활용한 게시판 구현 |
| 핵심 역량 목표 수립 | ☑ | `goal.md` |
| AI 활용 원칙 수립 | ☑ | `ai-principles.md` |
| 개인 구현 계획 수립 | ☑ | `implementation-plan.md` |
| 팀 repository 접근 권한 확인 | ☑ | `board-simple` 내부 Git 저장소 확인 |
| 개인 branch 생성 | ☑ | `project/hyungmin` |
| 팀 GitHub Projects 또는 개인 담당 이슈 준비 | ☑ | 개인 branch, 구현 계획, 문서 기준 태스크 관리 |
| 비밀값 관리 원칙 확인 | ☑ | `.env*` ignore, secret 값 미출력 |
| AWS 비용 모니터링 확인 | ☑ | 비용/리소스 종료 기준 문서화, 실제 비용 값은 출력하지 않음 |

---

## 기술 스택 / 개발 환경

| 항목 | 완료 | 비고 |
|------|:----:|------|
| 프론트엔드 선택 | ☑ | Next.js App Router, React, TypeScript |
| 백엔드 선택 | ☑ | FastAPI |
| 데이터베이스 선택 | ☑ | PostgreSQL 17 |
| LLM 모델 선택 | ☑ | OpenAI provider adapter |
| RAG vector store 선택 | ☑ | pgvector |
| MCP 외부 서비스 선택 | ☑ | 국가법령정보센터 Open API |
| Agent 구현 방식 선택 | ☑ | bounded Orchestrator Agent, MCP tool registry |
| 로컬/클라우드 실행 환경 구성 | ☑ | Docker Compose, Caddy proxy, `.next` 빌드 산출물 확인 |

---

## 기본 게시판 기능

| 항목 | 완료 | 비고 |
|------|:----:|------|
| 회원가입 구현 | ☑ | 이메일/비밀번호, nickname |
| 로그인 구현 | ☑ | HttpOnly cookie 기반 인증 |
| 로그아웃 구현 | ☑ | 인증 cookie 삭제 |
| 게시글 생성 | ☑ | 제목, 본문, Markdown, 태그 추출 |
| 게시글 목록/상세 조회 | ☑ | 목록/상세 화면 |
| 게시글 수정/삭제 | ☑ | 작성자 권한 확인 |
| 댓글 생성/조회 | ☑ | 게시글 상세와 연결, View more |
| 댓글 수정/삭제 | ☑ | API 구현, 작성자 권한 확인 |
| 태그 기능 | ☑ | `#태그명` 추출 및 표시 |
| 페이징 | ☑ | 게시글 페이지네이션, 댓글 offset/limit |
| 검색 | ☑ | 제목 검색 |

---

## AI 응용 기능

| 항목 | 완료 | 비고 |
|------|:----:|------|
| RAG 기능 설계 | ☑ | `docs/rag-pipeline.md`, `docs/architecture.md` |
| RAG 기능 구현 | ☑ | `/api/rag/search`, ingestion/chunking/embedding/retrieval |
| RAG 실패 케이스 확인 | ☑ | 검색 결과 없음, provider 실패, citation 검증 테스트 |
| MCP server 설계 | ☑ | `docs/mcp-agent-design.md` |
| MCP 기능 구현 | ☑ | `/api/mcp`, `tools/list`, `tools/call` |
| MCP 실패 케이스 확인 | ☑ | unknown tool, argument validation, timeout/error mapping 테스트 |
| Agent 기능 설계 | ☑ | bounded loop, state, action validation, stop condition |
| Agent 기능 구현 | ☑ | `/api/ai/agent-runs`, dispute issues, answer drafts, full analysis |
| Agent 실패 케이스 확인 | ☑ | max tool calls, provider failure, prompt injection, citation repair 테스트 |
| AI 기능 데모 시나리오 작성 | ☑ | `AI 법률 검토` 화면, 검색 결과/쟁점/초안 병렬 표시 |

---

## 품질 / 보안 / 비용

| 항목 | 완료 | 비고 |
|------|:----:|------|
| 기본 게시판 사용자 흐름 테스트 | ☑ | `test_auth_posts_comments.py` 포함 |
| RAG 사용자 흐름 테스트 | ☑ | RAG 관련 테스트 및 UI 화면 확인 |
| MCP 사용자 흐름 테스트 | ☑ | MCP server/tool 테스트 |
| Agent 사용자 흐름 테스트 | ☑ | Agent API/orchestrator 테스트 |
| `.env` 미커밋 확인 | ☑ | `.env*` ignore 확인, 값 미출력 |
| API key 권한 범위 확인 | ☑ | provider key는 환경변수에서만 읽도록 구성 |
| 로그 내 비밀값 미노출 확인 | ☑ | prompt safety, redaction 정책/테스트 확인 |
| AWS/OpenAI 사용량 확인 | ☑ | 비용 폭주 방지 설정과 rate limit 문서화, 실제 사용량 값은 출력하지 않음 |
| 사용하지 않는 리소스 종료 계획 | ☑ | `docker compose down`, `docker compose down -v`, 리소스 종료 기준 문서화 |

---

## 특강 / 학습 일정

| 항목 | 완료 | 비고 |
|------|:----:|------|
| AWS 클라우드 서비스 특강 참석 | ☑ | 2026-06-09 오전 10시 |
| AWS 실습 내용 정리 | ☑ | Docker/Caddy/AWS EC2 배포 토폴로지 문서화 |
| OpenAI Agent 워크샵 참석 | ☑ | 2026-06-12 오후 2시 |
| OpenAI Agent 워크샵 내용 정리 | ☑ | Agent 구조, tool 사용, provider adapter, prompt safety 반영 |

---

## README / 발표 / 제출

| 항목 | 완료 | 비고 |
|------|:----:|------|
| README - 프로젝트 개요 | ☑ | AI 법률 검토 보조 시스템 |
| README - 주요 구현 기능 | ☑ | 기본 게시판, RAG, MCP, Agent |
| README - 전체 아키텍처 구조 | ☑ | FE/BE/DB/AI/외부 API |
| README - RAG 기능 구조 | ☑ | source, chunk, embedding, retrieval, citation |
| README - MCP 기능 구조 | ☑ | JSON-RPC endpoint, tool registry, 외부 법률 API |
| README - Agent 기능 구조 | ☑ | bounded Orchestrator Agent, audit |
| README - 데모 스크린샷 | ☑ | 발표용 다이어그램과 UI 흐름 문서화 |
| README - 회고/한계/개선 아이디어 | ☑ | 후속 확장 대상 명시 |
| 발표자료 작성 | ☑ | 아키텍처, 데모, 회고 포함. 작성 완료로 반영 |
| 발표 준비 | ☑ | 2026-06-18 오전 10시 발표 완료 |
| WIL 작성 | ☑ | 작성 완료로 반영 |
| WIL 블로그 포럼 등록 | □ | WEEK15-16 태그, 등록 여부 별도 확인 필요 |

---

## 나만무 팀 공유

| 항목 | 완료 | 비고 |
|------|:----:|------|
| 개인 기술 스택 공유 | ☑ | Next.js, FastAPI, PostgreSQL/pgvector, OpenAI |
| 진행 상황 공유 | ☑ | README, 시스템 소개, 구현 문서 |
| 막힌 지점 공유 | ☑ | RAG 검색 품질, citation, prompt injection, 배포 리스크 문서화 |
| 중간 데모 피드백 | ☑ | 발표 완료 및 데모 흐름 반영 |
| README/발표 피드백 | ☑ | 발표용 다이어그램과 문서 보완 |
| 보안/비용 주의사항 공유 | ☑ | API key, `.env`, 외부 API, rate limit, 리소스 종료 |
