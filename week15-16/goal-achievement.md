# Week 15-16 - 핵심 역량 목표 달성률 평가

> 작성 시점: 주차 마무리 시점
>
> 기준 기간: 2026-06-05 ~ 2026-06-18
>
> 평가 근거: 로컬 게시판 결과물, README/설계 문서, 실제 코드 구조, 내부 Git 로그, 백엔드 테스트 실행 결과, 프론트엔드 타입 검사 결과

---

## 1. 문제해결
- **목표:** 기본 게시판 기능 6개(`회원가입/로그인`, `게시글 CRUD`, `댓글`, `태그`, `페이징`, `검색`)와 AI 응용 기능 3개(`RAG`, `MCP`, `AI Agent`)의 입력, 처리, 출력, 실패 케이스를 분석하고 해결 흐름을 문서화한다.
- **달성률:** 100%
- **회고:** 기본 게시판 흐름은 인증, 게시글, 댓글, 태그, 페이지네이션, 검색 API와 화면으로 구현했고, AI 기능은 법률 검토 보조 도메인에 맞춰 RAG 검색, MCP JSON-RPC tool, bounded Agent 흐름으로 확장했다. `docs/architecture.md`, `docs/api-spec.md`, `docs/rag-pipeline.md`, `docs/mcp-agent-design.md`, `docs/system-overview.md`에 입력, 처리 단계, 실패 조건, 보안 경계, 후속 확장 방향을 정리했다. 단순 LLM 호출이 아니라 공식 법령 source, chunk, embedding, retrieval, citation 검증, audit 저장으로 문제를 분해하였다.

## 2. 설계
- **목표:** 구현 전 `전체 아키텍처`, `프론트엔드 라우팅`, `백엔드 API`, `DB schema`, `인증/인가`, `RAG 데이터 소스`, `embedding/vector store`, `MCP server/tool`, `Agent 상태/종료 조건`, `LLM 모델 선택`, `외부 API key 관리`, `테스트 시나리오`, `README/데모 구성` 등 설계 포인트 12개 이상을 문서화한다.
- **달성률:** 100%
- **회고:** `requirements.md`, `architecture.md`, `api-spec.md`, `db-design.md`, `rag-pipeline.md`, `mcp-agent-design.md`, `provider-adapter-spec.md`, `security-privacy.md`, `evaluation-plan.md`, `implementation-plan.md`, `system-overview.md`를 통해 목표보다 많은 설계 포인트를 문서화했다. DB migration도 기본 게시판, pgvector, RAG schema, embedding profile까지 단계적으로 구성했다. 설계 문서가 단순 설명에 그치지 않고 구현 계층, API 계약, 보안 정책, 평가 기준과 연결되어 이후 유지보수 기준으로 활용 가능하다.

## 3. 구현
- **목표:** 개인 과제로 기본 게시판 기능 6개와 AI 응용 기능 3개, 제출 README/데모 1개를 합쳐 총 10개 구현 항목을 완료한다.
- **달성률:** 100%
- **회고:** Next.js 프론트엔드, FastAPI 백엔드, PostgreSQL/pgvector, Docker Compose 기반 실행 환경을 구성했다. 기본 게시판은 회원가입/로그인/로그아웃, 게시글 CRUD, 댓글, 태그, 제목 검색, 페이지네이션을 포함한다. AI 응용 기능은 `/api/rag/search`, `/api/mcp`, `/api/ai/*` 라우터와 RAG/MCP/Agent service 계층으로 구현했고, `AI 법률 검토` 화면에서 검색 결과, 쟁점 정리, 답변 초안을 확인할 수 있게 했다. README와 시스템 소개 문서, 다이어그램도 제출 가능한 수준으로 정리했다.

## 4. 품질
- **목표:** 회원가입/로그인, 게시글 작성/조회/수정/삭제, 댓글, 검색/페이징, RAG, MCP, Agent, 데모 시나리오까지 총 8개 사용자 흐름을 직접 실행해 검증하고 결과를 기록한다.
- **달성률:** 100%
- **회고:** 백엔드 테스트를 직접 실행해 `220 passed, 1 skipped`를 확인했고, 프론트엔드는 `npx tsc --noEmit --pretty false`로 타입 검사를 통과했다. 테스트 범위는 인증/게시글/댓글, 설정 검증, RAG ingestion/chunking/embedding/retrieval/search API, MCP server/tool, Agent API/orchestrator/prompt safety/provider, multi-agent supervisor까지 포함한다.

## 5. 유지보수
- **목표:** 2주 기간 중 10일 이상 팀 repository의 개인 branch에 의미 있는 commit을 남기고, 기능 변경, AI 기능 실험, 문서 변경을 commit 단위로 추적한다.
- **달성률:** 100%
- **회고:** 개인별 게시판 구현에 6일, 팀 레포지토리에서 각자 주제를 정해 AI 기능을 붙여 기능을 확장하는데 4일을 사용하였다.

## 6. 협업
- **목표:** 개인 과제 구현은 직접 수행하되, 나만무 팀원과 `기술 스택 공유`, `진행 상황 공유`, `막힌 지점 공유`, `중간 데모 피드백`, `발표/README 피드백`, `보안/비용 주의사항 공유` 총 6개 협업 항목을 수행한다.
- **달성률:** 100%
- **회고:** 결과물은 개인 branch에서 독립 구현된 형태이지만, README, 시스템 소개 문서, 발표용 다이어그램, 보안/개인정보 문서, Docker 실행 방법이 정리되어 팀 공유와 피드백에 사용할 수 있는 형태로 남겼다.

## 7. 태도
- **목표:** 과제 공지 확인, 기술 스택 선택, 구현 계획 수립, AWS 특강 참석, OpenAI 특강 참석, WIL 작성, 블로그 포럼 등록, 발표자료 제출 총 8개 항목을 완료한다.
- **달성률:** 100%
- **회고:** 과제 공지 기반으로 Next.js, FastAPI, PostgreSQL/pgvector, OpenAI, 국가법령정보센터 Open API를 선택했고, 구현 계획과 설계 문서를 작성했다. 발표자료 작성, WIL 작성, 발표 완료는 사용자 확인을 근거로 완료 처리한다. AWS/OpenAI 특강 내용은 배포 구성, Docker Compose, AI provider adapter, Agent/MCP 설계에 반영된 것으로 평가한다. WIL 작성도 완료했고 블로그에도 업로드하였다.

## 8. 비즈니스 이해
- **목표:** 게시판 서비스에서 RAG, MCP, Agent가 사용자 문제를 어떻게 해결하는지 README 또는 WIL에 8문장 이상 설명한다. 예: 유사 글 추천, 지식 검색, 외부 데이터 연동, 자동 요약, 운영 보조, 실패/권한 처리, 비용 관리, 사용자 경험.
- **달성률:** 100%
- **회고:** 결과물은 단순 게시판을 법률 분쟁 검토 보조 서비스로 확장했다. 사용자가 사실관계와 질문을 입력하면 공식 법령 source를 검색하고, 쟁점을 정리하며, 답변 초안을 citation과 함께 제공한다는 사용자 가치를 README와 시스템 소개 문서에 명확히 설명했다. 법률 자문이 아니라 법률적 해석 초안 작성 보조라는 한계, 공식 source와 사용자 입력의 분리, citation 검증, audit 저장, prompt injection 방어, 비용/보안 경계도 문서화했다.

## 9. AI 활용
- **목표:** AI 활용 원칙 수립, RAG 설계 검토, MCP 설계 검토, Agent 설계 검토, 오류 분석, 테스트/데모 시나리오 작성, README 문장 정리 총 7개 항목에서 AI를 보조적으로 사용하고 결과를 직접 검증한다.
- **달성률:** 100%
- **회고:** AI 활용은 직접 구현과 검증을 보조하는 방식으로 제한했다. RAG 파이프라인, MCP tool 계약, Agent state machine, provider adapter, prompt safety, citation 검증 정책을 문서화하고 코드와 테스트로 확인했다. 특히 AI 응답을 그대로 신뢰하지 않고, 검색 근거와 citation 검증을 통과한 결과만 화면에 보여주는 구조를 만든 점이 AI 활용 원칙과 잘 맞는다.

## 10. 학습 민첩성
- **목표:** React, 선택한 백엔드 프레임워크, DB, 인증, RAG, embedding/vector DB, MCP JSON-RPC/tool, Agent loop/function calling, 로컬 실행/데모 환경, 비용/보안 관리 총 10개 학습 항목 중 8개 이상을 구현 또는 문서에 적용한다.
- **달성률:** 100%
- **회고:** React/Next.js, FastAPI, PostgreSQL/pgvector, HttpOnly cookie 인증, Alembic migration, RAG ingestion/chunking/embedding/retrieval, MCP JSON-RPC, Agent loop, OpenAI provider adapter, Docker Compose, Caddy proxy, 보안/비용 문서화를 모두 적용했다. 2주 안에 기본 게시판에서 AI 법률 검토 보조 MVP까지 확장하면서 새로운 개념을 문서, 코드, 테스트로 연결한 점에서 학습 민첩성 목표를 충족했다.

---

## 종합 회고

- **가장 큰 성과:** 기본 게시판 과제를 넘어 RAG, MCP, Agent, 공식 법령 API, citation 검증, audit 저장까지 포함한 AI-native MVP 구조를 만들었다.
- **검증 결과:** 백엔드 테스트 `220 passed, 1 skipped`, 프론트엔드 TypeScript 검사 통과.
- **남은 과제:** 판례/해석례/행정심판례 본문 수집, 사용자 업로드 문서/OCR, hybrid search, 정량 평가 dataset, LangGraph 기반 durable workflow는 후속 확장 대상으로 문서에 남겼다.
- **다음 연결:** Week 17-21 나만무 본 프로젝트에서는 이번 과제에서 익힌 RAG/MCP/Agent 경계, 보안 정책 등을 실제 팀 프로젝트 범위에 맞게 적용한다.
