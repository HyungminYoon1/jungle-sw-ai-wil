# AI Agent 학습 문서

> 작성일: 2026-06-16  
> 목적: AI Agent의 정의, 구성요소, 주요 아키텍처 패턴, 구현 시 판단 기준을 학습용으로 정리한다.  
> 전제: 이 문서는 LangChain/LangGraph, OpenAI Agents SDK, Anthropic의 agentic system 개념을 기준으로 한다.

---

## 0. 핵심 결론

AI Agent는 단순히 “LLM을 호출하는 챗봇”이 아니다.

가장 실무적인 정의는 다음과 같다.

> **AI Agent는 사용자의 목표를 입력받아, LLM이 다음 행동을 선택하고, 도구/API/파일/DB/검색/메모리 같은 외부 자원과 상호작용하며, 실행 결과를 관찰해 다음 단계를 조정하는 상태 기반 다단계 소프트웨어 컴포넌트이다.**

짧게 표현하면 다음과 같다.

```text
AI Agent = LLM + Instruction + Tools + Context/State + Runtime Loop + Guardrails
```

중요한 구분은 다음이다.

| 구분 | 핵심 |
|---|---|
| 챗봇 | 답변 생성 중심 |
| RAG 챗봇 | 문서 검색 후 답변 생성 |
| Workflow | 미리 정해진 절차를 따라 실행 |
| Agent | 모델이 상황에 따라 다음 행동과 도구 사용을 선택 |
| Multi-Agent | 여러 Agent가 역할을 나눠 협력 |

---

## 1. AI Agent의 정의

### 1.1 최소 정의

OpenAI Agents SDK 관점에서 Agent는 다음에 가깝다.

> LLM에 instructions, tools, handoffs, guardrails, structured outputs 같은 실행 동작을 결합한 단위.

즉, 단순 모델 호출이 아니라 **모델이 사용할 수 있는 규칙, 도구, 출력 형식, 위임 방식, 보호 장치가 함께 묶인 실행 단위**다.

### 1.2 엄격한 정의

더 엄격하게 보면 AI Agent는 다음 조건을 충족해야 한다.

1. 목표 또는 작업 지시가 있다.
2. 단순 응답이 아니라 다음 행동을 선택한다.
3. 외부 도구나 환경과 상호작용한다.
4. 도구 실행 결과를 다시 관찰한다.
5. 관찰 결과에 따라 다음 행동을 조정한다.
6. 종료 조건이 있다.

이를 루프로 표현하면 다음과 같다.

```text
Goal
 ↓
Reason / Plan
 ↓
Act: Tool Call, API Call, Search, Code Execution
 ↓
Observe: Tool Result
 ↓
Update State
 ↓
Decide Next Action or Stop
```

### 1.3 Agentic System과 Agent의 차이

“Agent”라는 용어는 업계에서 완전히 고정된 표준 정의가 아니다. 따라서 다음처럼 구분하는 것이 정확하다.

| 용어 | 의미 |
|---|---|
| Agentic System | LLM과 도구를 이용해 어느 정도 자율적으로 작업하는 넓은 범주의 시스템 |
| Workflow | 사전에 정해진 코드 경로를 따라 LLM과 도구를 실행하는 시스템 |
| Agent | LLM이 자신의 절차와 도구 사용을 동적으로 결정하는 시스템 |

Anthropic의 구분을 적용하면:

```text
Workflow:
- 실행 순서가 코드로 정해져 있음
- 예: 검색 → 요약 → 검증 → 답변

Agent:
- 다음에 무엇을 할지 LLM이 판단
- 예: 검색할지, 계산할지, 사용자에게 질문할지, API를 호출할지 선택
```

---

## 2. AI Agent의 구성요소

## 2.1 Model

Agent의 판단을 수행하는 LLM 또는 AI 모델이다.

역할:

- 사용자 목표 해석
- 다음 행동 선택
- 도구 호출 인자 생성
- 결과 해석
- 최종 응답 생성

주의점:

- 모델이 강하다고 해서 자동으로 좋은 Agent가 되지는 않는다.
- 도구 설명, 컨텍스트 설계, 상태 관리, 실패 처리 품질이 더 중요할 수 있다.

---

## 2.2 Instruction

Agent의 역할, 제한, 출력 형식, 도구 사용 기준을 정의하는 지시문이다.

예시:

```text
너는 법률 정보 기반 게시판 답변 초안 생성 Agent다.
사용자 글에서 사실관계와 쟁점을 분리하고,
국가법령정보센터 API와 판례 검색 도구를 사용해 근거를 찾은 뒤,
단정적 법률 판단을 피하고 출처 기반 초안을 작성하라.
```

좋은 instruction의 조건:

- 역할이 명확해야 한다.
- 사용 가능한 도구와 사용 조건이 분명해야 한다.
- 금지해야 할 행동이 명시되어야 한다.
- 출력 형식이 안정적이어야 한다.
- 실패 시 동작이 정의되어야 한다.

---

## 2.3 Tools

Agent가 외부 세계에 영향을 주거나 정보를 가져오기 위해 사용하는 기능이다.

예시:

| Tool | 용도 |
|---|---|
| Web Search | 최신 정보 검색 |
| File Search | 문서 검색 |
| DB Query Tool | 데이터베이스 조회 |
| Email Tool | 이메일 작성/발송 |
| Calendar Tool | 일정 조회/생성 |
| Code Interpreter | 코드 실행 |
| MCP Tool | MCP Server가 노출한 외부 서비스 호출 |

도구는 Agent에게 “행동 능력”을 부여한다. 도구가 없으면 Agent는 대부분 대화형 추론 시스템에 가깝다.

---

## 2.4 Context / State

Agent가 현재 작업을 수행하기 위해 참조하는 정보다.

| 개념 | 의미 |
|---|---|
| Context | 이번 판단에 모델에게 전달되는 정보 |
| State | 시스템이 보관하는 현재 작업 상태 |
| Memory | 세션 또는 장기적으로 유지되는 정보 |

예시:

```text
Context:
- 사용자의 현재 질문
- 최근 대화 일부
- 검색된 문서 조각
- 도구 실행 결과

State:
- 현재 단계: issue_analysis
- 검색된 법령 목록
- 판례 검색 성공 여부
- 검증 실패 사유

Memory:
- 사용자의 프로젝트 주제
- 자주 쓰는 출력 형식
- 선호하는 기술 스택
```

핵심은 **모든 정보를 항상 넣는 것이 아니라, 필요한 정보만 넣는 것**이다.

---

## 2.5 Runtime Loop

Agent를 일반 LLM 호출과 구분하는 핵심이다.

기본 루프:

```text
while not done:
    1. 현재 목표와 상태를 모델에 전달
    2. 모델이 다음 행동을 선택
    3. 도구 호출 또는 응답 생성
    4. 도구 결과 관찰
    5. 상태 업데이트
    6. 종료 조건 판단
```

주의점:

- 루프가 무한히 돌지 않도록 최대 반복 횟수를 둔다.
- 도구 호출 실패 시 재시도 정책을 둔다.
- 위험한 도구는 사람 승인 단계를 둔다.

---

## 2.6 Guardrails

Agent가 잘못된 행동을 하지 않도록 막는 규칙과 검증 장치다.

예시:

| Guardrail | 설명 |
|---|---|
| 입력 검증 | 사용자의 요청이 허용 가능한지 검사 |
| 출력 검증 | JSON schema, 금지 표현, 출처 누락 검사 |
| 도구 권한 제한 | 특정 도구는 읽기 전용으로 제한 |
| 승인 단계 | 이메일 발송, 결제, 삭제 등은 사람 승인 필요 |
| 정책 필터 | 법률·의료·금융 등 고위험 답변에서 단정 방지 |

Agent의 위험은 “틀린 답변”뿐 아니라 **잘못된 외부 행동**에서 나온다. 예를 들어 이메일을 잘못 보내거나, DB를 잘못 수정하거나, 권한 없는 API를 호출하는 문제가 발생할 수 있다.

---

## 2.7 Observability / Evaluation

운영 가능한 Agent에는 로그와 평가가 필요하다.

기록해야 할 것:

- 사용자 입력
- 선택한 도구
- 도구 호출 인자
- 도구 결과
- 중간 추론 요약
- 최종 응답
- 실패 지점
- 토큰 사용량
- 지연 시간

평가해야 할 것:

| 평가 항목 | 질문 |
|---|---|
| 정확성 | 근거와 답변이 일치하는가? |
| 도구 선택 | 적절한 도구를 호출했는가? |
| 비용 | 불필요한 모델 호출이 많은가? |
| 안전성 | 위험한 행동을 승인 없이 수행하지 않았는가? |
| 재현성 | 같은 입력에 대해 일관된 결과를 내는가? |
| 추적 가능성 | 어느 단계에서 오류가 발생했는지 알 수 있는가? |

---

# 3. Workflow와 Agent의 차이

## 3.1 Workflow

Workflow는 실행 순서가 코드로 고정되어 있다.

예시:

```text
1. 사용자 질문 수신
2. 검색 쿼리 생성
3. 벡터 DB 검색
4. 검색 결과 재정렬
5. 답변 생성
6. 출처 검증
```

장점:

- 예측 가능하다.
- 디버깅이 쉽다.
- 비용 통제가 쉽다.
- 프로덕션 운영에 적합하다.

단점:

- 예외 상황에 덜 유연하다.
- 모델이 자유롭게 전략을 바꾸기 어렵다.

## 3.2 Agent

Agent는 모델이 다음 행동을 선택한다.

예시:

```text
사용자 질문을 보고 Agent가 판단:
- 검색이 필요한가?
- DB 조회가 필요한가?
- 사용자에게 추가 질문해야 하는가?
- 코드 실행이 필요한가?
- 이미 답할 수 있는가?
```

장점:

- 유연하다.
- 모호한 요청이나 다양한 작업에 대응하기 좋다.
- 사전에 모든 절차를 정의하기 어려운 작업에 적합하다.

단점:

- 예측 가능성이 낮다.
- 비용과 지연 시간이 증가할 수 있다.
- 디버깅이 어렵다.
- 안전장치가 부족하면 위험하다.

## 3.3 실무 기준

```text
정해진 절차가 있다 → Workflow
절차를 모델이 선택해야 한다 → Agent
여러 역할이 명확히 나뉜다 → Multi-Agent
근거 검증과 안정성이 중요하다 → Workflow + Agent 혼합
```

---

# 4. Multi-Agent 패턴 개요

Multi-Agent는 여러 Agent를 단순히 많이 붙이는 것이 아니다. 핵심은 **책임 경계, 컨텍스트 분리, 도구 권한 분리, 실행 흐름 제어**다.

대표 패턴:

1. Subagents
2. Handoffs
3. Skills
4. Router
5. Custom Workflow

각 패턴은 다음 질문에 대한 답이 다르다.

```text
누가 다음 작업을 결정하는가?
상태는 어디에 저장되는가?
사용자와 직접 대화하는 Agent는 누구인가?
여러 Agent를 병렬로 실행할 수 있는가?
각 Agent에게 어떤 컨텍스트를 전달하는가?
실패하면 어디서 복구하는가?
```

---

# 5. Subagents 패턴

## 5.1 개념

Subagents는 **메인 Agent 또는 Supervisor Agent가 여러 전문 Agent를 도구처럼 호출하는 구조**다.

```text
User
 ↓
Supervisor Agent
 ├─ Calendar Agent
 ├─ Email Agent
 ├─ Research Agent
 └─ DB Agent
 ↓
Final Answer
```

## 5.2 작동 방식

1. 사용자가 요청한다.
2. Supervisor가 작업을 분해한다.
3. 필요한 Subagent를 선택한다.
4. 각 Subagent에게 필요한 입력만 전달한다.
5. Subagent가 결과를 반환한다.
6. Supervisor가 결과를 합성한다.

예시:

```text
사용자: 다음 주 가능한 회의 시간을 찾고 참석자에게 안내 메일 초안을 작성해줘.

Supervisor:
- Calendar Agent에게 일정 조회 요청
- Contact Agent에게 참석자 정보 확인 요청
- Email Agent에게 메일 초안 작성 요청
- 최종 결과를 사용자에게 전달
```

## 5.3 장점

| 장점 | 설명 |
|---|---|
| 컨텍스트 격리 | 각 Agent가 필요한 정보만 본다. |
| 도구 권한 분리 | Calendar Agent는 Calendar Tool만, DB Agent는 DB Tool만 사용할 수 있다. |
| 병렬 실행 | 서로 독립적인 작업을 동시에 실행할 수 있다. |
| 유지보수성 | 각 Agent를 독립적으로 수정할 수 있다. |
| 중앙 통제 | 최종 판단은 Supervisor가 수행한다. |

## 5.4 한계

| 한계 | 설명 |
|---|---|
| 호출 수 증가 | Supervisor와 Subagent 호출이 누적된다. |
| 비용 증가 | 모델 호출과 토큰 사용량이 늘 수 있다. |
| Supervisor 병목 | 모든 판단이 중앙에 몰릴 수 있다. |
| 중복 작업 | 역할 경계가 불명확하면 여러 Agent가 같은 일을 한다. |
| 디버깅 복잡도 | 오류 발생 지점을 추적해야 한다. |

## 5.5 적합한 경우

```text
- 도메인별 역할이 명확하다.
- 여러 도구를 권한별로 분리해야 한다.
- 여러 작업을 병렬로 처리해야 한다.
- 최종 품질을 중앙에서 통제해야 한다.
```

## 5.6 부적합한 경우

```text
- 단순 Q&A
- 도구가 1~2개뿐인 작업
- 한 번의 LLM 호출로 충분한 작업
- 역할 경계가 불명확한 작업
```

---

# 6. Handoffs 패턴

## 6.1 개념

Handoffs는 **현재 Agent가 다른 Agent에게 제어권을 넘기는 구조**다.

```text
User
 ↓
Triage Agent
 ↓ handoff
Verification Agent
 ↓ handoff
Refund Agent
 ↓
User
```

Subagents와의 차이:

- Subagents: Supervisor가 계속 중앙에 남아 있다.
- Handoffs: 제어권 자체가 다른 Agent로 넘어간다.

## 6.2 작동 방식

1. 현재 Agent가 사용자 요청을 처리한다.
2. 특정 조건이 충족되면 다음 Agent로 전환한다.
3. 새 Agent가 이후 대화를 이어받는다.
4. 필요하면 다시 다른 Agent로 넘긴다.

예시:

```text
고객지원 시스템

Triage Agent:
- 문제 유형 파악

Warranty Agent:
- 보증 ID 확인

Refund Agent:
- 환불 가능 여부 판단

Resolution Agent:
- 최종 안내
```

## 6.3 장점

| 장점 | 설명 |
|---|---|
| 상태 전이에 강함 | 단계별 진행이 필요한 업무에 적합하다. |
| 사용자 직접 응대 | 각 단계 Agent가 사용자에게 직접 질문할 수 있다. |
| 순차 제약 표현 | A를 확인한 뒤 B를 진행하는 구조에 적합하다. |
| 대화형 업무에 적합 | 상담, 접수, 심사, 승인 흐름에 잘 맞는다. |

## 6.4 한계

| 한계 | 설명 |
|---|---|
| 병렬 처리에 약함 | 기본적으로 순차 흐름이다. |
| 잘못된 전환 위험 | 엉뚱한 Agent로 넘어가면 품질이 떨어진다. |
| 긴 대화 부담 | 대화 이력이 길어지면 컨텍스트가 커진다. |
| 중앙 검수 약화 | Supervisor가 항상 최종 검수하지 않을 수 있다. |

## 6.5 적합한 경우

```text
- 고객지원
- 민원 접수
- 환불 처리
- 단계별 신청서 작성
- 사용자의 추가 입력을 단계적으로 받아야 하는 업무
```

## 6.6 부적합한 경우

```text
- 병렬 리서치
- 여러 데이터 소스를 동시에 검색해야 하는 작업
- 중앙에서 최종 품질 검수를 반드시 해야 하는 업무
```

---

# 7. Skills 패턴

## 7.1 개념

Skills는 여러 Agent를 만드는 대신, **하나의 Agent가 필요한 전문 지식, 절차서, 프롬프트, 예시를 필요할 때 로드하는 방식**이다.

```text
Single Agent
 ├─ SQL Skill
 ├─ Code Review Skill
 ├─ Legal Draft Skill
 └─ Report Writing Skill
```

엄밀히 말하면 Skills는 “멀티 에이전트”라기보다 **단일 Agent + 모듈형 전문 지식 로딩 구조**에 가깝다.

## 7.2 작동 방식

1. 사용자 요청을 분석한다.
2. 필요한 skill을 선택한다.
3. 해당 skill의 문서, 규칙, 예시, 템플릿을 로드한다.
4. 동일한 Agent가 작업을 계속 수행한다.

예시:

```text
사용자: 이 SQL 쿼리 성능을 개선해줘.

Agent:
- SQL Optimization Skill 로드
- 실행 계획, 인덱스, 조인 순서 기준으로 분석
- 개선 쿼리 제안
```

## 7.3 Skill에 들어갈 수 있는 것

```text
/skills/sql_optimization
 ├─ instruction.md
 ├─ examples.md
 ├─ checklist.md
 └─ output_template.md

/skills/legal_draft
 ├─ instruction.md
 ├─ citation_rules.md
 ├─ risk_checklist.md
 └─ output_template.md
```

Skill 구성요소:

| 구성요소 | 설명 |
|---|---|
| Instruction | 해당 skill의 역할과 규칙 |
| Examples | 좋은 입력/출력 예시 |
| Checklist | 검증 기준 |
| Template | 출력 형식 |
| Domain Rules | 도메인별 금지/권장 규칙 |

## 7.4 장점

| 장점 | 설명 |
|---|---|
| 단순한 구조 | Agent 수를 늘리지 않아도 된다. |
| 비용 절감 가능 | 별도 Agent 호출이 줄어든다. |
| 상태 유지 쉬움 | 하나의 Agent가 대화를 계속 담당한다. |
| 도메인 지식 재사용 | skill 폴더를 재사용할 수 있다. |

## 7.5 한계

| 한계 | 설명 |
|---|---|
| 컨텍스트 증가 | 많은 skill을 동시에 로드하면 토큰이 증가한다. |
| 역할 격리 약함 | 하나의 Agent가 모든 판단을 한다. |
| 병렬 처리 약함 | 여러 전문 작업을 동시에 분산 처리하기 어렵다. |
| skill 선택 오류 | 잘못된 skill을 로드하면 성능이 떨어진다. |

## 7.6 적합한 경우

```text
- Agent는 하나로 충분하지만 도메인별 절차가 필요한 경우
- SQL, 코드 리뷰, 보고서 작성처럼 규칙과 템플릿이 중요한 경우
- 에이전트 수를 늘리기보다 지식 모듈을 관리하고 싶은 경우
```

## 7.7 부적합한 경우

```text
- 도구 권한을 강하게 분리해야 하는 경우
- 여러 작업을 병렬로 수행해야 하는 경우
- 팀별로 Agent를 독립 개발해야 하는 경우
```

---

# 8. Router 패턴

## 8.1 개념

Router는 **입력 요청을 분류한 뒤 적절한 Agent, Tool, Knowledge Source로 보내는 구조**다.

```text
User
 ↓
Router
 ├─ GitHub Agent
 ├─ Notion Agent
 ├─ Slack Agent
 └─ Database Agent
 ↓
Synthesizer
```

## 8.2 작동 방식

1. 사용자 요청을 받는다.
2. Router가 요청 유형을 분류한다.
3. 하나 또는 여러 Agent/Tool로 보낸다.
4. 결과를 합성한다.
5. 최종 응답을 만든다.

예시:

```text
사용자: API 인증 방식이 어디 문서에 있나요?

Router:
- GitHub README 검색
- Notion 내부 문서 검색
- Slack 과거 논의 검색

Synthesizer:
- 세 결과를 종합해 답변
```

## 8.3 Router 구현 방식

| 방식 | 설명 | 장점 | 단점 |
|---|---|---|---|
| Rule-based Router | if/else 규칙 기반 | 빠르고 예측 가능 | 유연성 낮음 |
| LLM Router | LLM이 의미를 보고 분류 | 유연함 | 비용과 오류 가능성 |
| Embedding Router | 임베딩 유사도로 분류 | 빠르고 확장 가능 | 복잡한 의도 처리 한계 |
| Hybrid Router | 규칙 + LLM + 임베딩 조합 | 균형적 | 구현 복잡도 증가 |

## 8.4 Subagents와 Router의 차이

| 항목 | Subagents | Router |
|---|---|---|
| 중심 역할 | Supervisor Agent | Routing Step |
| 판단 방식 | LLM이 동적으로 작업 분해 | 명시적 분류 로직 |
| 병렬 실행 | 가능 | 명시적으로 설계하기 쉬움 |
| 적합한 상황 | 작업 분해가 필요한 경우 | 요청 분류 기준이 비교적 명확한 경우 |

## 8.5 장점

| 장점 | 설명 |
|---|---|
| 명시적 제어 | 어떤 요청이 어디로 가는지 추적 가능하다. |
| 병렬 검색 | 여러 지식소스를 동시에 조회할 수 있다. |
| 비용 절감 가능 | 관련 없는 Agent 호출을 줄일 수 있다. |
| 지식소스 분리 | GitHub, Notion, Slack, DB 등을 독립적으로 관리 가능하다. |

## 8.6 한계

| 한계 | 설명 |
|---|---|
| 라우팅 오류 | 잘못 분류하면 정답 품질이 크게 낮아진다. |
| 복합 요청 처리 | 하나의 요청이 여러 도메인을 걸치면 분해가 필요하다. |
| 상태 유지 약함 | 기본 Router는 보통 stateless하다. |
| 결과 합성 필요 | 여러 결과를 충돌 없이 합쳐야 한다. |

## 8.7 적합한 경우

```text
- 여러 지식 저장소를 검색해야 한다.
- 요청 유형별로 처리 방식이 명확하다.
- 병렬 검색 후 합성이 필요하다.
- 도메인별 Agent 또는 Tool을 분리해야 한다.
```

---

# 9. Custom Workflow 패턴

## 9.1 개념

Custom Workflow는 **LangGraph 같은 그래프 기반 오케스트레이션으로 실행 흐름을 직접 설계하는 방식**이다.

```text
Start
 ↓
Query Rewrite
 ↓
Retrieve
 ↓
Rerank
 ↓
Reason
 ↓
Verify
 ↓
Answer
```

## 9.2 구성요소

| 구성요소 | 의미 |
|---|---|
| State | workflow 전체가 공유하는 상태 |
| Node | 실행 단위. LLM 호출, 검색, 검증, 도구 실행 등 |
| Edge | 다음 노드로 이동하는 연결 |
| Conditional Edge | 조건에 따른 분기 |
| Loop | 실패 시 재시도 |
| Parallel Branch | 병렬 실행 |
| Checkpoint | 중간 상태 저장 |
| Human-in-the-loop | 사람이 승인해야 다음 단계 진행 |

## 9.3 작동 방식

예시 RAG workflow:

```text
1. 사용자 질문 수신
2. 질문 재작성
3. 문서 검색
4. 검색 결과 재정렬
5. 근거 기반 답변 생성
6. 출처 누락 검증
7. 최종 답변 반환
```

조건 분기 예시:

```text
검색 결과가 충분함 → 답변 생성
검색 결과가 부족함 → 검색 쿼리 재작성 후 재검색
위험한 답변임 → 사람 검토 요청
```

## 9.4 장점

| 장점 | 설명 |
|---|---|
| 실행 흐름 통제 | 어떤 순서로 실행되는지 명확하다. |
| 디버깅 용이 | 어느 노드에서 실패했는지 추적하기 쉽다. |
| 재시도 가능 | 검색 실패, 검증 실패 시 루프를 설계할 수 있다. |
| 안정성 강화 | guardrail, human review를 특정 단계에 넣을 수 있다. |
| 혼합 가능 | Router, Subagents, Skills를 노드로 넣을 수 있다. |

## 9.5 한계

| 한계 | 설명 |
|---|---|
| 구현 복잡도 | 단순 Agent보다 설계 비용이 크다. |
| 과설계 위험 | 단순 작업에 쓰면 비용만 증가한다. |
| 유연성 감소 | 흐름을 너무 고정하면 예외 상황에 약하다. |
| schema 설계 필요 | 각 노드의 입력/출력을 명확히 해야 한다. |

## 9.6 적합한 경우

```text
- RAG 파이프라인
- 법률/의료/금융처럼 근거 검증이 중요한 작업
- 실패 시 재시도나 분기가 필요한 작업
- 승인 단계가 필요한 업무
- 운영 로그와 디버깅이 중요한 프로덕션 시스템
```

---

# 10. 다섯 패턴 비교표

| 패턴 | 제어권 | 상태 유지 | 병렬 처리 | 컨텍스트 격리 | 사용자 직접 응대 | 적합한 작업 |
|---|---|---|---|---|---|---|
| Subagents | Supervisor | 중간 | 강함 | 강함 | 약함 | 리서치, 업무 자동화, 도메인 분리 |
| Handoffs | 현재 Agent | 강함 | 약함 | 중간 | 강함 | 고객지원, 상담, 단계별 접수 |
| Skills | 단일 Agent | 강함 | 약함 | 약함~중간 | 강함 | SQL, 코드리뷰, 보고서, 도메인 절차 |
| Router | Router | 낮음 | 강함 | 강함 | 약함 | 멀티소스 검색, 분류 기반 처리 |
| Custom Workflow | 코드/그래프 | 설계 가능 | 설계 가능 | 설계 가능 | 설계 가능 | RAG, 검증형 시스템, 승인 플로우 |

---

# 11. 패턴 선택 기준

## 11.1 가장 먼저 할 질문

```text
1. 단일 LLM 호출로 충분한가?
2. RAG만 붙이면 충분한가?
3. 도구가 너무 많아졌는가?
4. 도메인별 instruction이 길어졌는가?
5. 병렬 실행이 필요한가?
6. 사용자와 단계별 대화가 필요한가?
7. 실행 흐름을 강하게 통제해야 하는가?
8. 실패 시 재시도와 검증이 필요한가?
9. 외부 상태를 변경하는 위험한 도구가 있는가?
10. 비용과 지연 시간을 감당할 수 있는가?
```

## 11.2 선택 규칙

```text
단순 답변 → 일반 LLM 또는 RAG
도메인 규칙만 추가 → Skills
여러 지식소스 검색 → Router
여러 전문 작업 병렬 실행 → Subagents
단계별 대화와 상태 전이 → Handoffs
검증, 재시도, 승인, RAG 파이프라인 → Custom Workflow
```

## 11.3 실무 권장 순서

처음부터 Multi-Agent로 시작하지 않는다.

권장 순서:

```text
1. 단일 LLM 호출
2. 단일 Agent + Tool
3. 단일 Agent + RAG
4. Skills로 도메인 지식 모듈화
5. Router로 지식소스 분리
6. Subagents로 역할 분리
7. Custom Workflow로 검증과 운영 안정화
```

복잡한 Multi-Agent를 먼저 만들면 다음 문제가 생긴다.

- 디버깅이 어렵다.
- 호출 수가 많아진다.
- 비용이 증가한다.
- 실패 원인이 불명확해진다.
- 각 Agent의 역할이 겹친다.

---

# 12. 프로젝트 적용 예시: 법률 정보 기반 게시판 AI 답변 초안 시스템

## 12.1 문제 정의

사용자가 게시판에 분쟁 관련 글을 올리고 “AI 답변 초안 생성”을 요청하면, AI Agent가 다음을 수행한다.

1. 사실관계 정리
2. 법적 쟁점 추출
3. 필요한 법령/판례/행정규칙 검색
4. 근거 기반 해석
5. 답변 초안 작성
6. 위험 표현과 출처 누락 검증

## 12.2 권장 아키텍처

이 경우 단순 챗봇이나 단일 Agent만으로는 부족할 가능성이 높다. 적합한 구조는 다음이다.

```text
Custom Workflow + Router + Subagents + Verifier
```

전체 흐름:

```text
User Post
 ↓
Issue Analysis Node
 ↓
Source Planning Node
 ↓
Router
 ├─ Law Retrieval Agent
 ├─ Case Retrieval Agent
 └─ Regulation Retrieval Agent
 ↓
Evidence Filter Node
 ↓
Legal Reasoning Agent
 ↓
Risk Verifier Node
 ↓
Draft Writer Node
 ↓
Final Draft
```

## 12.3 각 구성요소 역할

| 구성요소 | 역할 |
|---|---|
| Issue Analysis Node | 사실관계와 쟁점 분리 |
| Source Planning Node | 필요한 자료 유형 결정 |
| Law Retrieval Agent | 국가법령정보센터 API 조회 |
| Case Retrieval Agent | 판례 검색 |
| Regulation Retrieval Agent | 행정규칙, 고시 등 검색 |
| Evidence Filter Node | 관련 없는 자료 제거 |
| Legal Reasoning Agent | 근거 기반 해석 |
| Risk Verifier Node | 단정 표현, 출처 누락, 환각 위험 검토 |
| Draft Writer Node | 게시판 답변 초안 작성 |

## 12.4 왜 Custom Workflow가 필요한가

법률 답변은 순서가 중요하다.

```text
검색 없이 해석하면 환각 위험이 크다.
해석 없이 초안을 쓰면 일반론이 된다.
검증 없이 게시하면 단정적 법률 조언처럼 보일 수 있다.
```

따라서 다음과 같은 순차 흐름이 필요하다.

```text
사실관계 정리 → 검색 → 근거 평가 → 해석 → 초안 → 검증
```

## 12.5 안전장치

법률 정보 기반 시스템에서는 다음을 반드시 고려해야 한다.

| 위험 | 대응 |
|---|---|
| 근거 없는 법률 판단 | 출처 없는 판단 금지 |
| 최신 법령 미반영 | 법령 API 조회일 표시 |
| 판례 오인용 | 사건번호, 선고일, 법원 표시 |
| 단정적 조언 | “가능성이 있다”, “검토가 필요하다” 식으로 제한 |
| 사용자 피해 | 변호사 상담 필요 조건 명시 |
| 자동 게시 위험 | 최종 게시 전 사용자 승인 필요 |

---

# 13. MCP와 AI Agent의 관계

MCP(Model Context Protocol)는 Agent 자체가 아니라 **Agent가 외부 도구와 표준화된 방식으로 연결되도록 하는 프로토콜**에 가깝다.

구조:

```text
AI App / Agent Host
 ↓
MCP Client
 ↓ JSON-RPC
MCP Server
 ↓
External Service / API / DB / File System
```

예시:

```text
Legal Agent
 ↓
MCP Client
 ↓
Law API MCP Server
 ↓
국가법령정보센터 API
```

MCP를 쓰는 이유:

- 외부 서비스를 Agent 도구로 노출하기 쉽다.
- 도구 목록과 schema를 표준화할 수 있다.
- API Key와 권한 관리를 서버 쪽에 격리할 수 있다.
- 여러 Agent가 같은 MCP Server를 재사용할 수 있다.

주의점:

- MCP를 쓴다고 Agent 성능이 자동으로 좋아지지는 않는다.
- MCP Server의 도구 설계가 부실하면 Agent가 잘못 호출한다.
- 권한, rate limit, 에러 처리, schema 검증이 필요하다.

---

# 14. 구현 설계 템플릿

## 14.1 Agent 정의 템플릿

```yaml
agent_name: LegalReasoningAgent
purpose: 검색된 법령과 판례를 기반으로 사건 쟁점을 해석한다.
model: gpt-4.1 / gpt-5 계열 등
instructions:
  - 출처 없는 법률 판단을 하지 않는다.
  - 사실관계와 법률 판단을 구분한다.
  - 단정적 표현을 피한다.
  - 불확실한 부분은 명시한다.
tools:
  - law_search
  - case_search
  - regulation_search
input_schema:
  issue_summary: string
  facts: list[string]
  retrieved_sources: list[Source]
output_schema:
  legal_issues: list[string]
  reasoning: string
  uncertainty: list[string]
  cited_sources: list[string]
guardrails:
  - no_source_no_claim
  - human_approval_before_publish
```

## 14.2 Tool 정의 템플릿

```yaml
tool_name: law_search
purpose: 법령명을 기준으로 최신 법령 정보를 조회한다.
input:
  query: string
  law_type: optional[string]
output:
  law_name: string
  article: string
  content: string
  effective_date: string
  source_url: string
errors:
  - no_result
  - api_timeout
  - ambiguous_query
permission:
  mode: read_only
  api_key_location: server_env
```

## 14.3 Workflow 상태 템플릿

```yaml
state:
  user_post: string
  facts: list[string]
  issues: list[string]
  source_plan: list[string]
  retrieved_laws: list[Source]
  retrieved_cases: list[Source]
  reasoning_result: string
  draft_answer: string
  verification_errors: list[string]
  final_answer: string
```

---

# 15. 간단한 의사코드

## 15.1 Agent Loop

```python
state = initialize_state(user_request)

for step in range(MAX_STEPS):
    decision = model.decide_next_action(state)

    if decision.type == "tool_call":
        result = call_tool(decision.tool_name, decision.arguments)
        state = update_state(state, result)

    elif decision.type == "ask_user":
        return ask_user(decision.question)

    elif decision.type == "final_answer":
        return decision.answer

    elif decision.type == "fail":
        return fallback_response(state)

return stop_due_to_max_steps(state)
```

## 15.2 Router

```python
def route(user_query):
    if is_code_related(user_query):
        return ["github_agent"]
    if is_policy_related(user_query):
        return ["notion_agent", "slack_agent"]
    if is_database_related(user_query):
        return ["database_agent"]
    return ["general_agent"]
```

## 15.3 Custom Workflow

```python
def legal_answer_workflow(user_post):
    facts, issues = analyze_issue(user_post)
    source_plan = plan_sources(facts, issues)

    laws = law_retrieval_agent(source_plan)
    cases = case_retrieval_agent(source_plan)
    regulations = regulation_retrieval_agent(source_plan)

    evidence = filter_evidence(laws, cases, regulations)
    reasoning = legal_reasoning_agent(facts, issues, evidence)
    draft = draft_writer_agent(reasoning)

    verification = verify_draft(draft, evidence)
    if verification.has_errors:
        draft = revise_draft(draft, verification)

    return draft
```

---

# 16. 실패 패턴

## 16.1 Agent를 너무 많이 만드는 경우

문제:

```text
IssueAgent
FactAgent
LawAgent
CaseAgent
SummaryAgent
ToneAgent
VerifierAgent
FinalAgent
```

역할이 명확하지 않으면 호출 수만 늘고 품질은 좋아지지 않는다.

대응:

```text
Agent를 나누기 전에 다음을 확인한다.
- 도구 권한이 다른가?
- 컨텍스트가 다른가?
- 병렬 실행이 필요한가?
- 독립적으로 테스트할 가치가 있는가?
```

## 16.2 모든 정보를 모든 Agent에게 전달하는 경우

문제:

- 토큰 비용 증가
- 컨텍스트 오염
- 불필요한 정보로 인한 판단 오류

대응:

```text
각 Agent에게 필요한 최소 정보만 전달한다.
```

예시:

```text
Law Retrieval Agent:
- 검색할 법령 키워드
- 쟁점 요약

Draft Writer Agent:
- 정리된 사실관계
- 검증된 근거
- 작성 톤
```

## 16.3 도구 설명이 모호한 경우

나쁜 예:

```text
search(query): 정보를 검색한다.
```

좋은 예:

```text
search_law(query, article_number=None):
대한민국 법령명 또는 조문 키워드를 기준으로 법령 정보를 검색한다.
판례 검색에는 사용하지 않는다.
```

## 16.4 검증 Agent만 추가하면 안전하다고 착각하는 경우

Verifier도 LLM이면 오류를 낼 수 있다.

대응:

- schema 기반 검증
- 출처 필드 필수화
- 위험 도구 승인 단계
- 로그 기반 사후 평가
- 테스트 케이스 구축

## 16.5 Agent에게 너무 큰 자율성을 주는 경우

위험:

- 불필요한 도구 호출
- 비용 폭증
- 외부 상태 오염
- 예측 불가능한 결과

대응:

```text
- read-only 도구부터 시작
- write 도구는 승인 필요
- 최대 반복 횟수 설정
- 허용 도구 목록 제한
- 실패 시 fallback 정의
```

---

# 17. 보안과 권한 관리

AI Agent는 외부 도구를 사용할 수 있으므로 일반 챗봇보다 보안 설계가 중요하다.

## 17.1 API Key 관리

원칙:

```text
- API Key를 프론트엔드에 노출하지 않는다.
- 서버 환경변수 또는 Secret Manager에 저장한다.
- Agent에게 key 자체를 전달하지 않는다.
- MCP Server나 Backend Tool이 key를 사용한다.
```

## 17.2 권한 분리

| 도구 | 권한 원칙 |
|---|---|
| 검색 도구 | read-only |
| DB 조회 | 기본 read-only |
| DB 수정 | 사람 승인 필요 |
| 이메일 발송 | 초안 생성과 발송 분리 |
| 파일 삭제 | 기본 금지 또는 승인 필요 |
| 결제/계약 | 반드시 사람 승인 |

## 17.3 Tool Input 검증

도구 호출 전 검증해야 할 것:

- 필수 인자 누락 여부
- 타입 오류
- 허용되지 않은 값
- SQL injection 가능성
- path traversal 가능성
- 과도한 요청 범위

## 17.4 감사 로그

외부 상태를 바꾸는 Agent는 반드시 감사 로그를 남겨야 한다.

```text
who: 어떤 사용자 요청인가
what: 어떤 도구를 호출했는가
when: 언제 실행했는가
input: 어떤 인자로 호출했는가
output: 어떤 결과가 나왔는가
approval: 사람 승인이 있었는가
```

---

# 18. 평가 체크리스트

## 18.1 기능 평가

```text
[ ] Agent가 올바른 도구를 선택하는가?
[ ] 도구 인자를 올바르게 생성하는가?
[ ] 도구 실패 시 복구하는가?
[ ] 중간 결과를 상태에 반영하는가?
[ ] 최종 답변이 사용자 목표와 일치하는가?
```

## 18.2 품질 평가

```text
[ ] 근거 없는 주장을 하지 않는가?
[ ] 검색 결과와 답변이 일치하는가?
[ ] 불확실성을 표시하는가?
[ ] 과장된 결론을 피하는가?
[ ] 출력 형식이 안정적인가?
```

## 18.3 비용 평가

```text
[ ] 불필요한 모델 호출이 없는가?
[ ] 불필요한 도구 호출이 없는가?
[ ] context가 과도하게 크지 않은가?
[ ] 긴 문서를 요약/필터링 후 전달하는가?
[ ] 캐싱 가능한 결과를 재사용하는가?
```

## 18.4 안전성 평가

```text
[ ] write 작업은 승인 단계를 거치는가?
[ ] API Key가 노출되지 않는가?
[ ] 사용자 입력이 도구 호출 전에 검증되는가?
[ ] 로그가 남는가?
[ ] 실패 시 안전한 fallback이 있는가?
```

---

# 19. 학습 순서

## 19.1 1단계: 기본 개념

학습 목표:

```text
- LLM 호출과 Agent의 차이 이해
- Tool calling 이해
- Agent loop 이해
- Workflow와 Agent 차이 이해
```

실습:

```text
- 단일 LLM 호출 만들기
- 함수 하나를 tool로 연결하기
- tool result를 다시 모델에 넣어 답변 생성하기
```

## 19.2 2단계: 단일 Agent

학습 목표:

```text
- instruction 설계
- tool schema 설계
- state 관리
- error handling
```

실습:

```text
- 게시글 요약 Agent
- 검색 도구를 쓰는 Q&A Agent
- DB read-only 조회 Agent
```

## 19.3 3단계: RAG + Agent

학습 목표:

```text
- 문서 chunking
- embedding
- vector search
- reranking
- citation
- hallucination 방지
```

실습:

```text
- 과제 공지 문서 검색 Q&A
- 프로젝트 README 기반 질의응답
```

## 19.4 4단계: Multi-Agent 패턴

학습 목표:

```text
- Subagents
- Handoffs
- Skills
- Router
- Custom Workflow
```

실습:

```text
- Router로 GitHub/문서/DB 검색 분리
- Subagents로 검색/요약/검증 분리
- Custom Workflow로 RAG 파이프라인 구성
```

## 19.5 5단계: 운영 설계

학습 목표:

```text
- logging
- evaluation
- guardrails
- human approval
- cost control
- deployment
```

실습:

```text
- 도구 호출 로그 저장
- 실패 케이스 테스트
- 출처 누락 검증기 작성
- 승인 후 게시 기능 구현
```

---

# 20. 자주 하는 오해

## 오해 1. Agent는 무조건 자율적이어야 한다.

틀렸다. 실무 Agent는 대부분 제한된 권한과 제한된 도구 안에서 동작한다.

## 오해 2. Agent 수가 많을수록 성능이 좋다.

틀렸다. Agent 수가 늘면 호출 수, 비용, 디버깅 난이도가 증가한다.

## 오해 3. Multi-Agent는 LangGraph를 쓰면 자동으로 해결된다.

틀렸다. LangGraph는 실행 흐름을 구성하는 도구일 뿐이며, 역할 분리와 상태 설계는 개발자가 해야 한다.

## 오해 4. RAG가 있으면 환각이 사라진다.

틀렸다. 검색 결과가 부정확하거나, 모델이 근거를 잘못 해석하면 환각은 여전히 발생한다.

## 오해 5. 검증 Agent를 붙이면 안전하다.

부분적으로만 맞다. 검증 Agent도 오류를 낼 수 있으므로 schema 검증, 로그, 사람 승인, 테스트가 필요하다.

## 오해 6. MCP를 쓰면 Agent가 된다.

틀렸다. MCP는 도구 연결 프로토콜이다. Agent 여부는 목표, 도구 사용, 상태, 루프, 의사결정 구조에 달려 있다.

---

# 21. 최종 요약

AI Agent를 설계할 때 가장 중요한 것은 “어떤 모델을 쓸 것인가”보다 다음이다.

```text
1. 어떤 목표를 달성해야 하는가?
2. 어떤 도구가 필요한가?
3. 어떤 정보를 context로 줄 것인가?
4. 상태를 어디에 저장할 것인가?
5. 다음 행동을 누가 결정할 것인가?
6. 실패하면 어떻게 복구할 것인가?
7. 위험한 행동은 어떻게 통제할 것인가?
8. 비용과 지연 시간을 어떻게 제한할 것인가?
9. 결과를 어떻게 평가할 것인가?
```

가장 안전한 접근은 다음 순서다.

```text
단일 LLM → Tool 사용 → RAG → Skills → Router → Subagents → Custom Workflow
```

처음부터 복잡한 Multi-Agent를 만드는 것은 좋은 설계가 아닐 가능성이 높다. 먼저 단순한 구조로 문제를 해결하고, 실제로 다음 문제가 발생할 때 패턴을 추가하는 것이 낫다.

```text
도구가 많아졌다 → Router 또는 Subagents
프롬프트가 길어졌다 → Skills
단계별 상태가 필요하다 → Handoffs
검증과 재시도가 필요하다 → Custom Workflow
외부 서비스 표준 연결이 필요하다 → MCP Server
```

---

# 22. 참고 자료

- OpenAI Agents SDK - Agents: https://openai.github.io/openai-agents-python/agents/
- OpenAI Agents SDK - Tools: https://openai.github.io/openai-agents-python/tools/
- Anthropic - Building effective agents: https://www.anthropic.com/engineering/building-effective-agents
- LangChain - Multi-agent: https://docs.langchain.com/oss/python/langchain/multi-agent
- LangChain - Handoffs: https://docs.langchain.com/oss/python/langchain/multi-agent/handoffs
- LangGraph - Workflows and agents: https://docs.langchain.com/oss/python/langgraph/workflows-agents
- LangGraph - Overview: https://docs.langchain.com/oss/python/langgraph/overview

