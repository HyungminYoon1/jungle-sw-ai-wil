# Week 17-21 - WIL (What I Learned)

> 작성 시점: 나만무 종료 시점
>
> 기간: 2026-06-19 ~ 2026-07-25
>
> 프로젝트: Nodease
>
> 주제: 기업 내부 AI Workflow와 LLMOps 운영 플랫폼 구현

---

## 1. 이번 나만무 프로젝트의 목표

이번 프로젝트의 목표는 기업이 쉽게 AX 도입을 할 수 있도록 지원하는 플랫폼을 만드는 것이었다.
이를 위해 우선 Workflow를 이용해서 업무 자동화를 실현하는 기존의 플랫폼을 탐색하였고, 이를 기업용 AI 운영 요구사항에 맞게 확장·리팩터링하기로 했다. 거기에 더하여 기업에서 AI Workflow를 실제 업무에 적용할 때 무엇이 더 필요한지를 살펴보았더니, 권한 기반의 지식 접근, 비용 통제, 근거 추적, 개인정보 보호, 보안, 장애 복구 문제 등이 존재했고, 이를 통합해서 하나의 운영 플랫폼 안에서 다룰 필요가 있다는 것을 인지하게 되었다.

팀은 기존의 Moduly 제품을 기반으로 새 제품을 만들기로 방향을 정했고, 제품명은 Nodease라는 이름으로 지었다. 기업용으로 만들고, 비개발자를 주 타겟으로 두며, 그들이 쉽고 편하게 사용할 수 있어야 했다. 
사용자는 자연어로 쉽게 Workflow 초안을 만들고, 조직 권한이 적용된 사내 지식을 검색하며, 실행 이후에는 비용과 품질, 감사 기록과 보안 알림을 확인할 수 있어야 한다는 기능 요구사항이 도출되었고 이를 중심으로 다음 네 축의 핵심 도메인으로 정리했다.

- 자연어 요청으로 Workflow graph를 구성하는 Agent Builder
- 조직·팀·사용자 권한을 실행 시점까지 적용하는 Knowledge/RAG
- 실행 이력과 권한 변경을 추적하는 Audit, Trace, Security Alert
- LLM 노드의 비용과 품질을 비교하는 Cost Optimizer와 모델 라우팅

나는 이 중 Knowledge/RAG 도메인을 중심으로 설계 및 구현을 담당했다. 처음에는 문서를 parsing, chunking, embedding하고 vector search로 적절한 대상을 찾는 RAG의 성능에 대해서만 생각했지만, 프로젝트를 진행하면서 기업형 RAG의 핵심은 검색 알고리즘만이 아니라는 것을 알게 됐다. 누가 어떤 문서를 사용할 수 있는지, 문서가 변경·삭제됐을 때 언제 검색에서 제외되는지, 검색 근거가 답변과 감사 기록에 어떻게 연결되는지까지 함께 설계해야 했다.

개인 학습 목표는 다음과 같았다.

- RAG의 수집부터 검색·답변·출처 표시까지 전체 흐름을 구현 수준에서 이해
- KB(Knowledge Base), Document, Chunk, Knowledge Collection의 책임과 생명주기를 구분
- Workflow 저장·배포·실행 단계에서 같은 권한 계약을 유지
- 비동기 worker, 재시도, 동시성, 멱등성과 부분 실패를 고려
- 보안과 운영 요구사항을 문서, ADR, 테스트와 코드에 일관되게 반영
- AI를 코드 생성 도구로만 사용하지 않고 설계 검토와 오류 분석, 테스트 보강에 활용한 뒤 직접 검증

---

## 2. 시도한 접근 방식

### 기존 시스템을 먼저 읽고 목표 구조를 분리

프로젝트 초반에는 기존 Moduly 코드와 Nodease의 목표가 섞여 있었다. 코드에는 Workflow 생성·실행·배포, 기본 Knowledge 기능과 LLM credential 기능이 있었지만, 문서에는 아직 구현되지 않은 운영 목표도 함께 적혀 있었다.

그래서 기능을 추가하기 전에 `docs/PRD.md`, `docs/architecture.md`, `docs/data_model.md`, 기능별 requirements/API/component/test 문서와 ADR(아키텍처 결정 기록)을 기준으로 현재 구현과 목표를 구분했다. 중요한 정책은 ADR로 남기고, 기능별 동작은 requirements와 test case로 내려보내는 구조를 사용했다. 나만무 발표가 있기 전까지 최종적으로 72개의 ADR이 누적되었으며, 문서가 항상 완벽하게 최신 상태였던 것은 아니지만 설계의 정합성을 유지하고, 정책 충돌을 발견하며, 잘못 구현된 부분을 수정할 수 있는 기준점 역할을 했다.

### 설계와 정책 결정에 집중하고 구현·실험·검증은 AI 에이전트에 위임

이번 프로젝트에서는 직접 코드를 구현하지 않고 Codex를 사용해 구현을 AI 에이전트에 위임하였다.
내가 관여한 부분은 전체 아키텍처 리팩토링 방향, 담당 도메인 설계, 다른 도메인과 충돌하는 정책에 대한 ADR, 리팩토링 방향과 이슈 간 의존성 결정이었다. 각 모듈 구현에 필요한 세부 결정은 에이전트가 내리고 로컬 결정 로그에 기록하게 했다. 나는 전체 맥락과 상위 정책을 중심으로 판단하고, 모르는 개념과 선택 이유를 질문하는 탑다운 방식으로 학습과 구현을 진행했다.

에이전트 여러 개를 병렬로 배치해 구현, 테스트 작성, 코드 리뷰와 실험을 진행했지만, 나는 주로 추상화된 결정 로그와 결과 요약을 확인했다. 실제 코드 diff, 테스트 실행 과정과 결과를 직접 검증하지 않았으므로 구현 세부사항에 대한 검증 책임을 충분히 수행했다고 볼 수 없다. 이 방식은 넓은 범위를 빠르게 탐색하는 데는 효과적이었지만, AI가 만든 결과를 직접 이해하고 검증한 뒤 반영한다는 초기 원칙에는 미치지 못했다.


### 작은 이슈와 단계별 커밋으로 변경 범위 관리

Knowledge/RAG는 Client, Gateway, Shared model, Workflow Engine, Log System과 모두 연결된다. 처음에는 한 문제를 고치면서 인접한 문제까지 함께 수정하는 경우가 많았고, 그 결과 PR 범위가 커지고 새로운 리뷰 지적이 반복됐다.

이후에는 Linear 이슈별로 범위를 정하고 `local/mba-*` 문서에 overview, 구현 계획, 테스트 계획, 결정 로그를 작성했다. 하나의 PR 안에서도 schema, service, runtime, client, 문서 변경을 의미 단위 커밋으로 나누었다. 구현 전에는 재현 테스트를 먼저 만들고, 관련 도메인의 빠른 테스트를 통과시킨 뒤 PR 직전에 필요한 통합 검증을 수행하는 방향으로 개발 절차를 보완했다.

### RAG를 데이터 파이프라인과 권한 파이프라인으로 함께 설계

Knowledge ingestion은 다음 흐름으로 이해하고 구현했다.

```text
원본 등록
  -> 외부 통신 및 콘텐츠 안전 경계
  -> parsing / normalization / privacy masking
  -> chunking
  -> embedding
  -> document version 준비
  -> active version 전환
  -> retrieval candidate 생성
  -> 실행 주체 권한 재검증
  -> LLM prompt에 근거 주입
  -> 답변과 citation 표시
```

중요한 점은 parsing이나 embedding 성공만으로 문서를 검색 가능하게 만들지 않는 것이었다. 새 버전의 chunk와 embedding이 모두 준비된 뒤 active pointer를 전환해야 하며, 실패한 새 버전 때문에 기존 정상 버전이 사라지면 안 된다. 검색 시점에도 저장 당시의 권한을 그대로 믿지 않고 현재 조직 membership, KB 사용 권한, Collection routing과 source ACL 상태를 다시 확인해야 했다.

### 테스트를 계층별 책임에 맞게 분리

서비스 단위 테스트에서는 권한 판정, 상태 전이와 오류 매핑을 빠르게 검증했다. SQLAlchemy query의 JOIN과 PostgreSQL 의미는 가짜 Query만으로 증명할 수 없으므로, query 구조 테스트와 disposable PostgreSQL 계약 테스트를 구분했다. Client에서는 KB 선택 유지·제거 정책, 오류 상태, Citation 표시와 Workflow 실행 결과 UI를 검증했다. Workflow Engine에서는 no-evidence, 권한 회수, runtime context, retrieval 실패와 LLM 호출 여부를 검증했다.

전체 테스트를 매번 반복하기보다 변경 경계에 맞는 최소 테스트를 먼저 실행했다. 실제 PostgreSQL, migration, 전체 E2E와 배포 검증은 CI 또는 최종 검증 단계로 분리했다. 이 방식을 통해 테스트 시간을 줄이고, 불필요한 토큰 낭비를 막을 수 있었다.

---

## 3. 문제와 해결 과정

### Knowledge Base, Document, Collection의 경계가 모호했던 문제

초기 UI와 데이터 모델에서는 하나의 KB에 여러 독립 문서를 넣을 수 있었다. 기술적으로는 가능하지만 KB 단위로 권한과 생명주기를 관리하면서 여러 문서가 들어가면 한 문서만 별도로 권한 회수·삭제·갱신하기 어려워진다. 사용자는 KB를 문서 자체로 이해할 수도 있고 문서 모음으로 이해할 수도 있어 UI 의미도 불명확했다.

최종 baseline에서는 신규 manual 등록을 `source item 1개 = Document 1개 = document-level KB 1개`로 좁혔다. 여러 KB를 함께 탐색하거나 운영해야 할 때는 Knowledge Collection을 사용한다. Collection은 grouping, routing, UX와 운영 단위이지만 하위 KB의 content 권한을 자동으로 상속하지 않는다.

이 구분으로 다음 질문에 일관되게 답할 수 있게 됐다.

- Collection에 접근할 수 있어도 하위 KB 사용 권한이 없으면 해당 KB는 검색하지 않는다.
- Workflow에 선택되지 않은 KB는 사용자가 개인적으로 권한을 가지고 있어도 자동 검색하지 않는다.
- 한 KB의 문서를 삭제하거나 새 버전으로 교체해도 다른 KB의 생명주기에는 영향을 주지 않는다.

Legacy one-to-many 구조는 기존 데이터를 깨뜨리지 않기 위해 읽기 호환성을 남겼다. 모든 기존 데이터를 즉시 파괴적으로 변환하는 대신 신규 생성 규칙을 먼저 제한하고, 데이터 정리는 별도 migration과 운영 승인 범위로 분리했다.

### 권한이 UI, 저장, 배포와 runtime에서 서로 달라지는 문제

LLM 노드에서 KB를 선택할 수 있다는 사실이 실제 실행 가능성을 보장하지 않았다. Builder에서 목록이 보이고 Workflow에 저장되더라도 실행 시점에 사용자 membership이 제거되거나 KB 권한이 회수될 수 있다. 반대로 Workflow owner 권한을 runtime 사용자에게 조용히 적용하면 권한 우회가 된다.

이를 해결하기 위해 권한을 다음 단계로 나누었다.

1. 관리 화면은 KB `read`, `write`, `manage`, `content_read`와 조직 단위 Knowledge 관리 권한을 확인한다.
2. Builder는 사용자에게 노출 가능한 safe candidate와 runtime 경고만 사용한다.
3. 배포 preflight는 배포 유형과 연결된 Knowledge가 함께 실행 가능한지 확인한다.
4. Workflow Engine은 queue에서 전달된 조직 정보를 그대로 신뢰하지 않고 canonical deployment context를 재구성한다.
5. retrieval 직전 현재 execution subject의 membership, Collection route, KB `use`, source ACL을 다시 평가한다.
6. 최종 evidence를 prompt에 넣기 전에 권한과 visibility를 다시 확인한다.

인증된 내부 챗봇은 로그인 사용자를 execution subject로 사용한다. 공개 챗봇처럼 실행 주체가 없는 표면은 Workflow owner 권한으로 fallback하지 않고 anonymous public-only로 낮춘다. Public Collection에 연결됐더라도 private KB나 public exposure 승인이 없는 source-managed KB는 검색 후보에서 제외한다.

이 과정에서 권한은 한 번 검사하는 API middleware가 아니라 저장부터 runtime까지 이어지는 계약이라는 점을 배웠다.

### 근거가 없을 때 LLM이 답을 만들어내는 문제

RAG를 연결했는데 검색 근거가 없는 경우에도 LLM을 호출하면 일반 지식이나 추측으로 답할 수 있다. 사용자는 그 답이 사내 문서에서 나온 것인지 구분하기 어렵다. 특히 권한이 없어서 근거가 제외된 경우 추측 답변을 허용하면 권한 경계의 의미가 약해진다.

그래서 RAG가 활성화된 LLM 노드는 authorized evidence가 없으면 provider 호출을 건너뛰고 no-evidence 결과를 반환하는 fail-closed 정책을 적용했다. 사용자 메시지는 "문서가 없거나 접근 권한이 없습니다"처럼 존재 여부와 권한 거부 여부를 구분해 노출하지 않는 방향으로 정리했다.

반대로 일부 KB 조회만 timeout되고 나머지 허용된 KB에서 충분한 근거를 찾은 경우에는 전체 요청을 항상 실패시키지 않고 partial result로 처리할 수 있게 설계했다. 이때 실패한 숨은 KB의 ID, 이름, 정확한 개수나 내부 경로는 노출하지 않는다.

### Citation과 이를 추적하는 Lineage를 같은 것으로 생각했던 문제

처음에는 답변에 문서 제목을 표시하면 출처 기능이 완성된다고 생각했다. 하지만 문서의 출처 자체를 의미하는 Citation과 그 출처에 대한 근거를 남기는 내부 기록인 Lineage는 목적이 다르다.

- Citation은 최종 사용자가 답변의 근거 문서를 확인하기 위한 표시다.
- Lineage는 내부 운영자가 source, document version, chunk와 실행 당시 evidence 관계를 역추적하기 위한 기록이다.

Workflow와 Chatbot에는 사용자용 Citation projection을 추가하되 hidden KB ID, raw source URL, 내부 ACL과 restricted title이 노출되지 않도록 safe label만 전달했다. Durable prompt trace에는 RAG 원문을 다시 복사하지 않고 redacted marker와 안전한 summary만 남기도록 했다. 검색 품질 분석에 필요한 순위도 Collection child KB마다 다시 시작하는 내부 rank 대신 최종 병합 evidence의 전역 순위를 사용하는 방향으로 정리했다.

사용자 Citation 표시는 구현됐지만, 모든 외부 connector와 장기 보존 정책을 포함한 privileged lineage 저장소는 완성하지 못했다. 이 범위는 후속 과제로 남아 있다.

### 비동기 ingestion의 부분 성공과 재시도 문제

문서 처리 중 parsing은 성공했지만 embedding이 실패하거나, DB 반영 뒤 worker가 종료되거나, 같은 task가 중복 전달될 수 있다. 단순히 Celery retry를 적용하면 동일 문서를 중복 처리하거나 active version을 여러 번 전환할 수 있다.

그래서 ingestion 작업은 durable 상태와 idempotency key를 기준으로 처리하고, lease/fencing, outbox, recovery scanner와 dead-letter 상태를 구분하는 방향으로 보강했다. 새 artifact가 모두 준비되기 전에는 retrieval-visible 상태로 전환하지 않고, 활성화 transaction과 후속 cleanup event의 관계를 명시했다.

Workflow schedule에서도 같은 문제가 있었다. 여러 Gateway replica가 동일 schedule을 dispatch하거나 Celery message가 중복 전달될 수 있으므로 `(schedule_id, scheduled_for)` 단위 claim과 worker admission을 한 번만 허용했다. 다만 외부 이메일, HTTP, ticket API까지 수학적인 exactly-once를 보장하는 것은 아니며, 각 adapter가 idempotency key를 지원해야 한다는 한계도 문서에 남겼다.

### 보안을 강화하면서 정상 기능까지 막는 문제

외부 문서, LLM provider, Gmail, GitHub, Slack과 원격 파일은 모두 인터넷으로 나간다. URL 문자열만 검사한 뒤 일반 HTTP client로 연결하면 DNS rebinding, private address 접근, redirect와 credential 유출 위험이 생길 수 있다. 반대로 모든 외부 통신을 단순 차단하면 제품 기능이 동작하지 않는다.

그래서 보안을 다층으로 나누었다.

- 애플리케이션의 operation-bound policy가 어떤 기능이 어떤 provider endpoint, method, timeout과 payload 상한을 사용할지 결정한다.
- guarded transport가 DNS 결과, 실제 연결 주소, TLS hostname, redirect, response size와 오류 redaction을 검증한다.
- Squid egress proxy와 NetworkPolicy가 애플리케이션 guard를 우회한 직접 외부 연결을 제한한다.
- Gateway, Workflow Worker, Knowledge Worker가 각각 공통 outbound 계약을 사용한다.

HTTPS와 CORS도 코드 계약과 실제 운영 검증을 구분했다. 프로덕션 HTTPS-only와 CORS allowlist 제약, Helm TLS 지원은 구현됐지만 실제 EC2 Ingress와 인증서 연결, 운영 origin은 별도의 배포 환경 검증이 필요했다. 보안 설계가 있다는 이유만으로 실제 운영 검증까지 완료됐다고 표시하면 안 된다는 점을 배웠다.

### 코드 리뷰에서 문제가 반복적으로 발견된 원인

프로젝트 중 Codex 코드 리뷰에서 권한 우회, stale state, raw metadata 노출, migration head, 부분 실패와 CI 문제가 반복적으로 발견됐다. 단순히 테스트 수가 적어서라기보다 도메인 간 계약이 여러 위치에 분산된 것이 근본 원인이었다.

예를 들어 KB selectable 여부를 Client, Gateway와 Workflow Engine이 서로 다른 기준으로 판단하거나, preflight는 차단하지만 runtime resolver가 같은 정책을 적용하지 않는 문제가 있었다. 테스트에서 가짜 Query가 JOIN 호출을 무시해 실제 PostgreSQL 의미 오류를 찾지 못하는 경우도 있었다.

이를 개선하기 위해 schema readiness, Knowledge query service, runtime candidate resolver, permission registry, deployment runtime policy와 retrieval metadata helper를 책임별로 분리했다. 테스트도 service 분기 테스트, query 구조 테스트, PostgreSQL 통합 테스트, API와 browser flow 검증으로 역할을 나눴다. 테스트를 많이 만드는 것보다 정책의 권위자가 하나인지, 모든 소비 경계가 그 권위자를 사용하는지를 먼저 확인하는 것이 중요했다.

---

## 4. 구현된 범위와 남은 한계

프로젝트 종료 시점에 팀이 실제 구현한 주요 범위는 다음과 같다.
나는 이 중 Knowledge/RAG와 Workflow 연결 경계를 중심으로 설계·구현 책임을 맡았다.

### 구현된 주요 범위

- Workflow 생성·편집·배포와 Agent Builder 기반 초안 생성
- PostgreSQL/pgvector 기반 Knowledge ingestion과 retrieval
- document-level KB와 Knowledge Collection 관리
- 조직·팀·사용자 기준 KB/Collection 권한과 위임 관리
- LLM 노드의 명시 KB 및 Collection routing 연결
- 실행 주체 기반 runtime candidate 제한과 anonymous public-only fallback
- no-evidence 시 LLM 추측 호출 차단
- Workflow와 Chatbot의 안전한 Citation 표시
- Audit, Trace, Security Alert와 민감 payload redaction 경계
- LLM usage ledger, 비용 비교와 모델 라우팅 실험
- schedule dispatch admission과 provider 지원 수준별 외부 부수효과 재실행 차단 경계
- operation-bound outbound guard와 proxy-only egress 보조 계층
- 기능별 단위·API·Client·PostgreSQL 계약 테스트와 CI 품질 게이트

### 부분 완료 또는 후속 범위

- 최신 데모 corpus에 대한 Recall, MRR, NDCG 실측 결과 확정
- 실제 사용자 5-10명을 대상으로 한 설명 없는 사용성 테스트와 정성 피드백
- 전체 모바일 viewport, 접근성, 페이지 로딩과 사용자 체감 응답 시간 검증
- 실제 EC2 HTTPS 인증서, 운영 CORS origin과 인증서 만료 검증
- Client에 남아 있는 사용자 입력 `console.log` 등 디버깅 로그 정리
- 모든 external source connector의 source ACL freshness와 live-linked retrieval 완결
- 모든 외부 provider의 idempotency 지원을 포함한 end-to-end exactly-once
- 삭제된 문서의 장기 Citation/Lineage 보존과 privileged 조회 정책 완결
- Knowledge Skill과 회사 ontology/context model의 구체적인 관리 UI와 lifecycle
- full LLM token streaming. 현재 SSE 기반 실행 이벤트와 reconnect는 있지만 provider token을 그대로 전달하는 streaming은 아니다.

Knowledge 공식 문서에는 구현 baseline과 장기 목표가 함께 있으며 일부 문서는 여전히 Draft 상태다. 따라서 문서에 요구사항이 있다는 사실만으로 기능이 구현됐다고 판단하지 않고, 코드와 테스트의 `Verified Against` 기준을 함께 확인해야 한다.

---

## 5. 새로 배운 점

- 기업형 RAG의 핵심은 vector similarity만이 아니라 권한, version, lifecycle, provenance와 감사 가능성이다.
- Knowledge Collection은 하위 KB의 권한을 묶어 주는 역할이 아니라 검색 범위와 운영 편의를 위한 routing 단위다.
- Builder에서 선택 가능하다는 사실과 runtime에서 사용할 수 있다는 사실은 다르다. 실행 직전 권한을 다시 확인해야 한다.
- Workflow owner 권한을 익명 또는 다른 사용자 실행에 fallback하면 편리해 보여도 권한 우회가 된다.
- 근거가 없는 RAG에서는 LLM 호출을 생략하는 것이 품질 저하가 아니라 보안과 신뢰성을 위한 제품 정책일 수 있다.
- Citation은 사용자 표시이고 Lineage는 내부 역추적이다. 두 데이터의 공개 범위와 보존 기간이 달라야 한다.
- 새 document version은 준비가 끝난 뒤 원자적으로 활성화해야 기존 정상 검색을 보존할 수 있다.
- Celery의 재시도는 멱등성을 자동으로 보장하지 않는다. durable claim, idempotency key, lease와 outcome unknown 정책이 필요하다.
- 애플리케이션 guard와 egress proxy는 서로 대체 관계가 아니다. 전자는 요청 의미를, 후자는 네트워크 우회를 통제한다.
- 단위 테스트의 Fake는 서비스 분기를 검증할 수 있지만 실제 JOIN, FK, transaction과 PostgreSQL 의미를 증명하지 못한다.
- 테스트 개수보다 저장·관리·preflight·runtime·lifecycle·audit가 같은 정책을 공유하는지가 더 중요하다.
- 문서에서 목표와 구현을 구분하지 않으면 발표와 코드 리뷰에서 실제 기능을 과장하게 된다.

---

## 6. AI 활용과 협업 회고

AI는 요구사항 정리, 기술 조사, 설계와 ADR 검토, 코드 구현, 오류와 CI 실패 분석, 테스트 케이스 보강, UX 문구와 발표 Q&A 정리에 사용했다. 기획, 기술 조사, 설계와 발표자료는 내용을 이해하고 판단했지만, 구현과 테스트는 에이전트의 결정 로그와 결과 요약에 의존했다. 실제 코드 경로와 테스트 결과를 직접 검증하지 않았기 때문에 AI 활용 범위는 넓었어도 검증 수준은 목표에 미치지 못했다.

이 과정에서 AI 리뷰가 문제를 계속 찾는 이유도 배웠다. 리뷰 요청을 여러 번 보내는 것만으로 품질이 올라가는 것이 아니라, 처음부터 이슈 범위와 불변조건을 명확히 하고 실패 테스트를 먼저 작성해야 한다. 중앙 정책을 만들었더라도 실제 API와 runtime이 그 정책을 사용하지 않으면 구조는 개선되지 않는다.

협업 측면에서는 PRD, architecture, feature 문서, ADR과 CI를 공통 언어로 사용하려고 했다. 반면 프로젝트 범위가 빠르게 커지면서 팀원 사이의 도메인 계약을 구현 전에 충분히 고정하지 못했고, 코드 리뷰와 장애·리스크 공유가 일관된 절차로 정착하지 못한 점은 아쉬웠다. 멘토링 자료 사전 공유와 피드백 기록도 더 규칙적으로 관리할 필요가 있었다.

---

## 7. 목표 달성 평가

최종 체크리스트와 목표 산식에 따른 종합 달성률은 88%로 평가했다. 문제 정의, 설계, 구현과 유지보수는 목표를 달성했다. 반면 AI 활용은 구현·테스트 결과를 직접 검증하지 못해 67%로 평가했다. 품질에서도 CI, 계약 테스트와 예외 처리를 보강했지만 사용자 테스트, 모바일·반응형, 실제 배포 HTTPS/CORS와 검색 품질 실측이 부족했다.

가장 큰 성과는 문서를 임베딩해 검색하는 기능을 만든 것이 아니라, 기업 데이터가 Workflow 실행에 들어가기까지 필요한 권한과 생명주기 경계를 이해하고 구현한 것이다. 반대로 가장 아쉬운 점은 제품 범위를 넓게 잡으면서 사전 계약보다 사후 코드 리뷰에 의존한 비중이 커졌다는 점이다.

발표와 포스터 세션을 완료하면서 구현된 기능과 목표 설계를 구분해 설명하는 것이 중요하다는 점도 확인했다. 동작하는 happy path만 보여주는 것보다, 어떤 조건에서 fail-closed하고 무엇이 아직 미완료인지 말할 수 있어야 기업용 AI 시스템의 신뢰성을 설명할 수 있었다.

---

## 8. 앞으로의 개선 계획

1. 최신 평가 corpus와 judgment를 확정하고 Retrieval의 Recall@K, MRR, NDCG와 no-evidence 품질을 측정
2. 실제 사용자 테스트를 진행해 권한 요청, Workflow 생성, RAG 답변, Citation과 관리자 화면에서 막히는 지점을 기록
3. Client debug log를 제거하고 실제 배포의 HTTPS 인증서, CORS origin, 만료와 비용 상태를 검증
4. Source-managed connector의 ACL freshness, revocation과 public exposure approval lifecycle을 구체화
5. Citation과 내부 Lineage의 보존·삭제·권한 계약을 완성
6. 도메인 간 공통 정책은 API, preflight와 runtime이 실제로 사용하도록 architecture test와 PostgreSQL 계약 테스트로 보호
7. 기능 구현 전에 요구사항, 실패 조건과 테스트 증거를 먼저 연결해 반복적인 사후 수정을 줄이기

이 경험을 바탕으로 앞으로는 데이터 권한, 분산 실행, 보안, 비용과 운영 자동화까지 함께 설계할 수 있는 백엔드·AI 플랫폼 엔지니어로 성장하고 싶다.

---

## 참고 자료

- [Nodease 레포지토리](https://github.com/nodease/mbased/)
