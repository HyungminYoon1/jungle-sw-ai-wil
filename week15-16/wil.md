# Week 15-16 - WIL (What I Learned)

> 작성 시점: 주차 마무리 시점
>
> 기간: 2026-06-05 ~ 2026-06-18
>
> 주제: AI 응용 기술을 활용한 게시판 구현

---

## 1. 이번 주 목표

이번 주 학습의 목표는 기본 게시판을 직접 구현하고, 그 위에 RAG, MCP, AI Agent를 붙여 AI 응용 기능이 실제 사용자 흐름 안에서 동작하도록 만드는 것이었다. 각자 게시판을 만들어봄으로써 나만무 프로젝트를 진행하기 전에 스스로 프론트엔드, 백엔드를 모두 구현해보는 것에 의의가 있었다. 기술은 Python/FastAPI와 Next.js를 기반으로 나만무 프로젝트에 사용할 기술들로 팀원들과 합의하였다.

기본 게시판 범위는 회원가입, 로그인, 로그아웃, 게시글 CRUD, 댓글, 태그, 검색, 페이지네이션이었다. 단순히 화면만 만드는 것이 아니라 Next.js 프론트엔드, FastAPI 백엔드, PostgreSQL 데이터베이스, 인증, API 계약, 테스트까지 풀스택 흐름을 한 번에 연결하는 것이 목표였다.

AI 기능은 단순히 ChatGPT API를 호출하는 수준이 아니라 법률 도메인과 연계하였다. 사용자가 분쟁에 대해서 사실관계와 질문을 입력하면 공식 법령 데이터를 검색하고, 관련 쟁점을 정리하며, 법률적 해석을 포함한 답변 초안 작성까지 보조하는 구조로 확장했다. 그래서 최종 결과물은 `AI 법률 검토 보조 시스템` 방향으로 정리했다.

핵심 목표는 다음과 같았다.

- 기본 게시판 기능을 사용자 흐름 기준으로 완성한다.
- PostgreSQL + pgvector 기반 RAG 구조를 설계하고 구현한다.
- MCP JSON-RPC endpoint와 allowlist 기반 tool 호출 구조를 만든다.
- Agent가 무제한으로 tool을 실행하지 않도록 bounded loop와 검증 단계를 둔다.
- AI 응답에는 검색 근거와 citation 검증을 연결한다.
- README, 아키텍처, API, RAG, MCP/Agent, 보안 문서를 남겨 발표와 후속 확장에 사용할 수 있게 한다.

---

## 2. 시도한 접근 방식

첫 1주일은 개개인이 기본 게시판의 구조를 잡았다. 프론트엔드는 Next.js App Router, React, TypeScript를 사용했고, 백엔드는 FastAPI, Pydantic, SQLAlchemy, Alembic을 사용했다. 데이터베이스는 PostgreSQL 17과 pgvector를 기준으로 잡았다. 이후 나머지 기간은 팀 공통 레포지토리에 작성된 기본 게시판을 기반으로 하여 각자 원하는 대로 구조를 수정하거나 AI 기능을 덧붙였다.

게시판 기능은 FastAPI 라우터가 서비스 계층을 호출하고, 서비스가 repository/model/schema를 사용하는 구조로 나누었다. 인증은 HttpOnly cookie에 담은 JWT로 처리했으며, POST/PUT/PATCH/DELETE 요청에는 Origin 검사를 적용했다. 게시글은 Markdown 작성/미리보기와 상세 화면의 sanitize 렌더링을 지원했고, 본문 내 `#태그명` 형식의 문자열은 백엔드에서 추출해 태그로 저장했다.

기본 게시판 구조가 잡힌 뒤에는 AI 기능을 적용하기 위한 설계를 먼저 작성했다. `docs/architecture.md`, `docs/api-spec.md`, `docs/db-design.md`, `docs/rag-pipeline.md`, `docs/mcp-agent-design.md`, `docs/security-privacy.md`, `docs/evaluation-plan.md` 등의 문서를 작성하면서 어떤 데이터가 어디에 저장되고, 어떤 계층이 어떤 책임을 갖는지 분리했다.

RAG는 다음 흐름으로 설계했다.

```text

source 수집
  -> normalization
  -> legal-aware chunking
  -> embedding
  -> vector retrieval
  -> evidence review
  -> citation validation
  -> persistence/audit
```

MCP는 Agent가 쓸 수 있는 tool 경계로 정의했다. 모델이 임의로 DB, 파일시스템, shell, 외부 API를 호출하는 구조가 아니라, 서버가 허용한 tool만 JSON-RPC 형식으로 호출할 수 있게 했다. MVP tool은 `search_legal_documents`, `search_law_open_api`, `verify_citations`로 잡았다.

Agent는 LangGraph 같은 프레임워크를 사용하지 않고, 명시적인 상태 흐름으로 구현했다. 발표용 MVP는 단일 Orchestrator Agent 중심으로 정리했고, issue/source planning, action 제안, action 검증, tool 실행, 관찰, 초안 작성, citation 검증을 수행하도록 했다. Supervisor Agent와 도메인 전문 Agent 구조는 후속 확장 방향으로 문서화하면서 일부 코드와 테스트까지 검토했다.

검증은 테스트 중심으로 진행했다. 백엔드에서는 인증, 게시글, 댓글, 설정, RAG chunking/embedding/retrieval/search API, MCP server/tool, Agent API, prompt safety, provider adapter, multi-agent supervisor 테스트를 작성했다. 최종 확인 시점에는 백엔드 테스트 `220 passed, 1 skipped` 이었고, 프론트엔드는 `npx tsc --noEmit --pretty false`로 TypeScript 타입 검사를 통과했다.

---

## 3. 문제와 해결 과정

### 게시판 기능과 AI 기능의 책임이 섞일 위험

처음에는 기본 게시판에 AI 기능을 붙인다는 요구를 보고, 화면 하나와 API 하나를 추가하면 된다고 생각하기 쉬웠다. 하지만 RAG와 Agent가 들어가면 단순 controller 안에서 처리할 수 있는 문제가 아니었다. 검색, chunking, embedding, prompt assembly, provider 호출, citation 검증, audit 저장이 모두 다른 책임을 가진다.

그래서 API route는 요청과 응답 schema를 다루는 얇은 계층으로 두고, 실제 로직은 service와 repository로 나누었다. RAG는 `services/rag`, MCP는 `services/mcp`, Agent는 `services/agent`, provider 호출은 `services/ai`로 분리했다. 이 구조를 잡고 나니 기능이 늘어나도 "어디에 어떤 코드가 들어가야 하는지"를 판단하기 쉬워졌다.

### RAG를 단순 vector search로만 보면 부족했던 점

처음에는 RAG를 "문서를 임베딩하고 유사도 검색을 한다" 라는 개념 정도로 이해했다. 하지만 법률 검토 보조 서비스에서는 검색 결과가 답변의 근거가 되기 때문에, 어떤 문서에서 온 근거인지, 어떤 버전의 법령인지, citation 가능한 chunk인지가 중요했다.

그래서 `legal_sources`, `legal_documents`, `legal_document_chunks`, `embedding_profiles`, `legal_document_chunk_embeddings`, `rag_runs`, `rag_retrievals` 같이 모델을 세부적으로 분리했다. embedding model과 dimension을 `embedding_profiles`로 관리하였는데, 임베딩 모델을 바꾸면 vector dimension이 달라질 수 있고, 서로 다른 embedding profile의 vector를 같은 검색 공간에서 비교하면 안 되기 때문이다.

법률 문서도 checksum 하나로 중복 판단을 끝내지 않도록 했다. 같은 법령이라도 시행일이나 version이 다르면 별도 문서로 보존해야 하고, 같은 canonical/version인데 normalized checksum이 다르면 conflict review 상태로 남겨야 한다. 이 부분을 문서화하면서 RAG는 단순 검색 기능이 아니라 "근거를 추적 가능한 상태로 관리하는 시스템"이라는 점을 배웠다.

### 외부 API와 MCP tool 경계

과제 요구사항에는 MCP와 실제 외부 서비스 연동이 있었다. 여기서 MCP를 모델에게 자유로운 실행 권한을 주는 장치로 이해하면 위험했다. 법률 API key, 내부 DB, 검색 결과, 사용자 사실관계가 모두 섞일 수 있기 때문이다.

그래서 MCP는 FastAPI 내부 JSON-RPC endpoint로 구현하고, tool registry에서 allowlist된 tool만 호출하도록 했다. `search_law_open_api`는 국가법령정보센터 Open API를 호출하되, API key는 환경변수에서만 읽고 응답이나 로그에 남기지 않는 정책을 세웠다. unknown tool, argument validation 실패, timeout, provider error 같은 실패 케이스도 테스트로 확인했다.

이 과정을 통해 tool calling은 "모델이 어떤 함수를 부를지 고르는 기능"이 아니라, 서버가 검증 가능한 실행 경계를 제공하는 문제라는 점을 배웠다. 모델이 action을 제안할 수는 있어도 실제 실행은 서버가 action type, tool name, arguments, 반복 여부, budget을 검증한 뒤에만 해야 한다.

### Agent가 무한히 검색하거나 생성할 수 있는 문제

Agent를 붙이면 "알아서 검색하고 답변한다"는 장점이 있지만, 동시에 무한 루프, 비용 폭주, 불안정한 tool 호출 문제가 생긴다. 특히 법률 검토처럼 근거가 부족하면 계속 검색하고 싶어지는 도메인에서는 중단 조건이 필요했다.

그래서 Orchestrator Agent에 `max_iterations`, `max_tool_calls`, `max_repeated_actions`, `max_external_sync_candidates`, timeout, citation repair 1회 제한을 두었다. 설계상 Agent 상태 흐름은 `initialize_run -> request_intent_guard -> plan_issue_sources -> reasoning_loop -> draft -> verify -> optional_repair_once -> persist`로 명시했고, 실제 MVP 구현에서는 `plan_issue_sources`, `propose_action`, `validate_action`, `execute_tool`, `observe`, `draft`, `verify`, `persist` 같은 단계를 감사 로그로 남겼다.

또한 사용자 질문은 Agent 제어 명령이 아니라 untrusted data로 취급했다. "이전 지시를 무시해라", "citation 없이 답해라", "API key를 출력해라" 같은 문구는 prompt injection risk로 분류하고, provider/tool 선택 권한을 사용자 입력에 주지 않도록 했다. 이 부분은 `test_agent_prompt_safety.py` 같은 테스트로 검증했다.

### citation 검증을 마지막에 붙이는 것만으로는 부족했던 점

법률 답변에서는 citation이 매우 중요하다. 하지만 모델이 임의로 조문, 판례, URL을 만들어낼 수 있으므로, 답변에 citation 문자열이 있다는 것만으로는 충분하지 않다.

그래서 검색 결과는 `rag_retrievals`에 저장하고, `verify_citations` tool은 해당 run에서 실제 검색된 chunk 또는 검증된 외부 source metadata에 근거한 citation만 유효하게 처리하도록 했다. 모델이 임의로 만든 chunk ID나 URL은 제거하거나 실패로 처리해야 한다.

이 과정을 통해 AI 답변의 품질은 프롬프트만으로 해결되지 않는다는 점을 배웠다. 검색, evidence 저장, citation 검증, 실패 시 repair 또는 근거 부족 응답까지 이어지는 구조가 있어야 사용자에게 설명 가능한 답변이 된다.

### 발표용 MVP와 후속 확장 범위 구분

구현을 진행하다 보면 판례, 법령해석례, 행정심판례, 사용자 PDF 업로드, OCR, hybrid search, multi-agent workflow, LangGraph 같은 확장 아이디어가 계속 나왔다. 하지만 한정된 기간 내에 완성하기 위해 무작정 기능을 추가할 수 없었다.

그래서 MVP는 법령 중심 공식 source 조회와 색인, pgvector 기반 검색, OpenAI 기반 Agent 응답 생성, 검색 결과/쟁점/답변 초안 병렬 표시로 좁혔고 판례 본문 수집, 사용자 업로드 문서, OCR, hybrid search, 정량 평가 dataset, LangGraph durable workflow는 후속 확장으로 문서화했다.

이번 과제에서는 "지금 되는 것"과 "설계만 남긴 것"을 README와 시스템 소개 문서에 나누어 적었다.

---

## 4. 새로 배운 점

- 게시판은 단순 CRUD처럼 보여도 인증, 권한, 입력 검증, 페이지네이션, 검색, 댓글, 태그가 연결되면 사용자 흐름 전체를 봐야 한다.
- Next.js 프론트엔드와 FastAPI 백엔드를 연결할 때 HttpOnly cookie 인증을 쓰면 프론트엔드에서 token을 직접 다루지 않아도 되지만, CORS와 Origin 검사를 함께 신경 써야 한다.
- RAG는 vector search 하나가 아니라 source 수집, normalization, chunking, embedding profile, retrieval, citation, audit까지 포함한 데이터 파이프라인이다.
- pgvector를 사용할 때 embedding model과 dimension을 고정값처럼 다루면 나중에 모델 변경이 어렵다. `embedding_profiles`처럼 provider/model/dimension을 분리하는 구조가 필요하다.
- 공식 법령 데이터는 최신본 하나만 있으면 되는 것이 아니라 시행일과 version을 보존해야 한다. 분쟁 발생 시점에 따라 적용 근거가 달라질 수 있기 때문이다.
- MCP tool은 Agent에게 자유로운 권한을 주는 장치가 아니라, 서버가 허용한 기능만 검증된 schema로 호출하게 만드는 경계다.
- Agent는 반복, 비용, 외부 API 호출, citation repair에 제한이 있어야 한다. 제한 없는 Agent는 기능보다 리스크가 크다.
- 사용자의 질문은 prompt instruction이 아니라 데이터로 다뤄야 한다. prompt injection 문구가 있어도 tool allowlist, citation 정책, secret redaction 정책이 유지되어야 한다.
- AI 응답은 생성보다 검증이 중요하다. 검색 근거, citation 검증, 불확실성 표시, disclaimer가 있어야 사용자가 결과를 검토할 수 있다.
- 설계 문서는 구현을 늦추는 부가 작업이 아니라, 기능이 커질 때 책임 경계를 유지하기 위한 기준이다.
- 테스트는 단순히 통과 여부를 확인하는 것이 아니라, 실패 케이스와 보안 정책이 계속 유지되는지 확인하는 장치다.

---

## 5. 다음 주 계획

다음 주부터는 나만무 본 프로젝트로 넘어간다. 이번 주차에서 배운 가장 큰 점은 AI 기능을 단순히 "모델 호출"로 붙이면 설명 가능한 서비스가 되기 어렵다는 것이다. 입력 데이터, 검색 근거, 도구 호출, Agent 상태, 실패 처리, 보안 경계를 함께 설계해야 한다.

나만무 1W에는 주제 선정과 기획이 중요하다. 이번 게시판 과제에서 익힌 기준을 바탕으로, 팀 프로젝트에서도 먼저 아래 질문을 확인하려고 한다.

- AI가 서비스의 핵심 가치인가?
- 사용자가 화면에서 결과를 이해하고 검토할 수 있는가?
- 5주 안에 MVP로 보여줄 수 있는 범위인가?
- RAG, MCP, Agent 중 실제로 가져갈 가치가 있는 기능은 무엇인가?
- 데모에서 실패해도 설명 가능한 fallback이나 백업 시나리오가 있는가?
- secret, 비용, 외부 API 장애, rate limit을 어떻게 관리할 것인가?

기술적으로는 이번 과제의 모든 구조를 그대로 가져가기보다, 나만무 주제에 맞게 필요한 부분만 골라 쓰려고 한다. RAG가 필요하면 source와 citation 구조를 먼저 잡고, Agent가 필요하면 tool과 상태 제한을 먼저 정해야 한다. MCP가 필요하다면 외부 서비스 호출을 모델에게 열어주는 방식이 아니라 allowlist와 schema 검증을 포함한 서버 경계로 설계해야 한다.

발표 관점에서는 "무엇을 만들었는가"보다 "왜 이 문제를 AI로 풀어야 하는가", "어떤 기술적 챌린지를 어떻게 해결했는가", "사용자가 어떤 흐름으로 가치를 얻는가"를 먼저 정리하려고 한다. 이번 주의 결과물은 그 기준을 세우는 연습이 되었다.

---

## 참고 자료

- [기본 게시판 연습 레포지토리] https://github.com/jungle-my-own-weapon-team5/study-web-hyungmin
- [AI 기능을 추가한 게시판 레포지토리] https://github.com/jungle-my-own-weapon-team5/board-simple/tree/project/hyungmin
