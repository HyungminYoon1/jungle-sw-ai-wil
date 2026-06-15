# LangGraph 학습자료

작성일: 2026-06-15  
대상: Python 기반 LLM 애플리케이션 / RAG / AI Agent 시스템을 학습하는 개발자  
전제: Python 기본 문법, 함수, 딕셔너리, 타입 힌트, LLM API 호출 개념을 알고 있다고 가정한다.

---

## 0. 결론 요약

LangGraph는 LLM 에이전트의 실행 흐름을 **상태(State), 노드(Node), 엣지(Edge), 그래프(Graph)** 로 명시적으로 구성하는 오케스트레이션 프레임워크다.

단순히 “LLM에게 프롬프트를 보내고 답을 받는 도구”가 아니다. LangGraph의 핵심 목적은 다음이다.

1. 복잡한 에이전트 실행 흐름을 코드로 통제한다.
2. LLM 호출, 도구 호출, 조건 분기, 반복 루프를 명시적으로 설계한다.
3. 중간 상태를 저장해서 대화 지속성, 장애 복구, 휴먼 승인, 장기 실행 작업을 지원한다.
4. RAG, 멀티 에이전트, 검증 루프, 사람 승인 워크플로우 같은 복잡한 구조를 유지보수 가능한 형태로 만든다.

짧게 말하면, LangGraph는 **LLM 애플리케이션용 상태 기반 워크플로우 런타임**이다.

---

## 1. LangGraph를 이해하기 전에 필요한 배경

### 1.1 일반 LLM 호출

가장 단순한 구조는 다음과 같다.

```text
사용자 입력 → LLM 호출 → 응답 반환
```

이 구조는 간단하지만 다음 요구사항이 생기면 한계가 생긴다.

- 질문에 따라 검색할지 말지 결정해야 한다.
- 검색 결과가 부족하면 다시 검색해야 한다.
- 답변 전에 검증 에이전트가 검토해야 한다.
- 중간에 사람이 승인해야 한다.
- 작업이 실패하면 중간 지점부터 재개해야 한다.
- 여러 에이전트가 역할을 나누어야 한다.

이 경우 단순 함수 체인만으로는 실행 흐름이 복잡해지고, 디버깅도 어려워진다.

### 1.2 LangChain과 LangGraph의 차이

| 구분 | LangChain | LangGraph |
|---|---|---|
| 중심 역할 | 모델, 프롬프트, 도구, 체인 추상화 | 에이전트 실행 흐름 오케스트레이션 |
| 주요 관심사 | LLM 호출을 쉽게 연결 | 상태, 분기, 루프, 체크포인트, 재개 |
| 적합한 경우 | 단순 체인, 일반 도구 호출 | 복잡한 에이전트, RAG 루프, 멀티 에이전트 |
| 구조 | 체인 또는 에이전트 추상화 | 그래프 기반 제어 흐름 |

LangGraph는 LangChain 생태계에 속하지만, LangChain을 반드시 사용해야 하는 것은 아니다. 공식 문서도 LangGraph를 장시간 실행되는 상태 있는 에이전트를 만들기 위한 저수준 오케스트레이션 런타임으로 설명한다.

---

## 2. LangGraph의 핵심 개념

## 2.1 State

`State`는 그래프 전체가 공유하는 상태 객체다.

예를 들어 RAG 에이전트라면 다음과 같은 값들이 State에 들어갈 수 있다.

```python
from typing_extensions import NotRequired, TypedDict


class State(TypedDict):
    question: str
    route: NotRequired[str]
    retrieved_docs: NotRequired[list[str]]
    answer: NotRequired[str]
    retry_count: NotRequired[int]
```

State는 “현재 그래프 실행이 알고 있는 모든 중요한 정보”라고 볼 수 있다.

학습용 예제에서는 간단히 `route: str`, `answer: str`처럼 적기도 하지만, 실제 타입 검사를 엄격하게 적용한다면 처음 입력에 없는 값은 `NotRequired` 또는 `Optional`로 표현하는 편이 자연스럽다. 또는 input schema와 output schema를 분리해서 “입력으로 받는 값”과 “그래프 내부에서 채워지는 값”을 나눌 수 있다.

중요한 설계 원칙은 다음이다.

1. 모든 데이터를 State에 넣지 않는다.
2. 다음 노드가 판단하는 데 필요한 값만 넣는다.
3. 외부 DB에 저장해야 하는 영속 데이터와 그래프 실행 중 임시 상태를 구분한다.
4. 리스트 누적이 필요한 값은 reducer를 고려한다.

---

## 2.2 Node

`Node`는 실제 작업을 수행하는 함수다.

노드는 보통 다음 형태를 가진다.

```python
def some_node(state: State):
    # 현재 state를 읽고
    # 필요한 작업을 수행한 뒤
    # 업데이트할 state 일부를 반환한다.
    return {"some_key": "some_value"}
```

노드의 특징은 다음과 같다.

- 입력: 현재 State
- 출력: State의 일부 업데이트
- 역할: LLM 호출, 검색, 분류, 검증, 저장, API 호출 등

공식 API 문서 기준으로 `StateGraph`의 노드는 `State -> Partial<State>` 형태로 이해하면 된다.

---

## 2.3 Edge

`Edge`는 다음에 어떤 노드를 실행할지 결정한다.

엣지는 두 종류로 나눌 수 있다.

### 일반 엣지

항상 같은 다음 노드로 이동한다.

```python
builder.add_edge("node_a", "node_b")
```

### 조건부 엣지

현재 State를 보고 다음 노드를 선택한다.

```python
builder.add_conditional_edges(
    "classify_question",
    route_question,
    {
        "rag": "rag_node",
        "direct": "direct_node",
    },
)
```

조건부 엣지는 LangGraph에서 분기, 반복, 에이전트 루프를 만드는 핵심 도구다.

---

## 2.4 START와 END

LangGraph에는 특수 노드가 있다.

```python
from langgraph.graph import START, END
```

- `START`: 그래프 실행 시작점
- `END`: 그래프 실행 종료점

예:

```python
builder.add_edge(START, "answer_node")
builder.add_edge("answer_node", END)
```

---

## 2.5 Graph

Graph는 State, Node, Edge를 조합한 실행 가능한 구조다.

```python
builder = StateGraph(State)
builder.add_node("answer_node", answer_node)
builder.add_edge(START, "answer_node")
builder.add_edge("answer_node", END)
graph = builder.compile()
```

`compile()`은 그래프 구조를 검증하고 실행 가능한 객체로 만든다. 컴파일 단계에서 checkpointer, store, breakpoint 같은 런타임 옵션도 설정할 수 있다.

---

## 3. LangGraph의 실행 모델

LangGraph는 그래프 기반으로 실행된다.

```text
START
  ↓
Node A 실행
  ↓
State 업데이트
  ↓
Edge 판단
  ↓
Node B 또는 Node C 실행
  ↓
State 업데이트
  ↓
END 또는 루프
```

공식 문서에 따르면 LangGraph는 메시지 전달 기반으로 동작하며, 실행은 discrete “super-step” 단위로 진행된다. 여러 노드가 같은 단계에서 실행될 수도 있고, 모든 노드가 비활성 상태가 되고 전달 중인 메시지가 없으면 그래프 실행이 종료된다.

실무적으로는 다음 정도로 이해하면 충분하다.

```text
노드는 일을 한다.
엣지는 다음 일을 정한다.
State는 작업 중간 결과를 보존한다.
Graph는 전체 실행 흐름을 통제한다.
```

---

## 4. 설치

```bash
pip install -U langgraph
```

LLM 모델과 LangChain 통합까지 사용할 경우 보통 다음 패키지도 함께 설치한다.

```bash
pip install -U langgraph langchain langchain-openai
```

RAG 예제까지 구성한다면 다음과 같은 패키지가 추가될 수 있다.

```bash
pip install -U langgraph "langchain[openai]" langchain-text-splitters bs4 requests
```

실제 프로젝트에서는 사용하는 모델 제공자, 벡터 DB, 문서 로더에 따라 의존성이 달라진다.

---

## 5. 최소 예제: 단일 노드 그래프

LLM 없이 LangGraph 구조만 확인하는 예제다.

```python
from typing_extensions import TypedDict
from langgraph.graph import StateGraph, START, END


class State(TypedDict):
    question: str
    answer: str


def answer_node(state: State):
    question = state["question"]
    return {
        "answer": f"'{question}'에 대한 예시 답변입니다."
    }


builder = StateGraph(State)

builder.add_node("answer_node", answer_node)
builder.add_edge(START, "answer_node")
builder.add_edge("answer_node", END)

graph = builder.compile()

result = graph.invoke({
    "question": "LangGraph가 무엇인가요?"
})

print(result)
```

예상 결과:

```python
{
    "question": "LangGraph가 무엇인가요?",
    "answer": "'LangGraph가 무엇인가요?'에 대한 예시 답변입니다."
}
```

중요한 점은 `answer_node()`가 전체 State를 반환하지 않고, 업데이트할 값인 `answer`만 반환한다는 것이다.

---

## 6. 조건부 분기 예제

질문 유형에 따라 RAG 검색을 할지, 직접 답변할지 분기하는 예제다.

```python
from typing import Literal
from typing_extensions import TypedDict
from langgraph.graph import StateGraph, START, END


class State(TypedDict):
    question: str
    route: str
    answer: str


def classify_question(state: State):
    question = state["question"]

    if "최신" in question or "검색" in question or "출처" in question:
        return {"route": "rag"}

    return {"route": "direct"}


def route_question(state: State) -> Literal["rag", "direct"]:
    return state["route"]


def rag_node(state: State):
    return {
        "answer": f"'{state['question']}'에 대해 외부 문서를 검색한 뒤 답변합니다."
    }


def direct_node(state: State):
    return {
        "answer": f"'{state['question']}'에 대해 모델의 기본 지식으로 답변합니다."
    }


builder = StateGraph(State)

builder.add_node("classify_question", classify_question)
builder.add_node("rag_node", rag_node)
builder.add_node("direct_node", direct_node)

builder.add_edge(START, "classify_question")

builder.add_conditional_edges(
    "classify_question",
    route_question,
    {
        "rag": "rag_node",
        "direct": "direct_node",
    },
)

builder.add_edge("rag_node", END)
builder.add_edge("direct_node", END)

graph = builder.compile()

print(graph.invoke({
    "question": "LangGraph 최신 문서를 검색해서 설명해주세요."
}))
```

이 예제의 핵심은 `route_question()`이다. 이 함수가 현재 State를 보고 다음 노드를 결정한다.

---

## 7. 루프 구조: Agent Loop

LangGraph에서 에이전트 루프는 일반적으로 다음 구조를 가진다.

```text
START
  ↓
LLM 호출
  ↓
도구 호출 필요?
  ├─ 예 → Tool Node → 다시 LLM 호출
  └─ 아니오 → END
```

이 구조는 다음 상황에서 사용된다.

- LLM이 검색 도구를 호출해야 하는지 판단한다.
- 도구 결과를 받은 뒤 다시 LLM이 답변을 생성한다.
- 필요하면 여러 번 도구를 호출한다.
- 더 이상 도구 호출이 필요 없으면 종료한다.

개념 코드:

```python
def should_continue(state):
    last_message = state["messages"][-1]

    if getattr(last_message, "tool_calls", None):
        return "tools"

    return END


builder.add_edge(START, "llm")
builder.add_conditional_edges("llm", should_continue, ["tools", END])
builder.add_edge("tools", "llm")
```

이런 루프는 LangGraph의 가장 대표적인 사용 패턴이다.

---

## 8. State 업데이트와 Reducer

State의 각 key는 노드가 반환한 값으로 업데이트된다.

기본 동작은 “덮어쓰기”다.

```python
return {"answer": "새 답변"}
```

그러면 기존 `answer` 값은 새 값으로 대체된다.

하지만 메시지 목록처럼 누적이 필요한 값은 reducer를 사용해야 한다.

예:

```python
import operator
from typing import Annotated
from typing_extensions import TypedDict


class State(TypedDict):
    messages: Annotated[list[str], operator.add]
```

이 경우 여러 노드가 `messages`에 값을 추가하면 리스트가 덮어쓰기되지 않고 누적된다.

Reducer는 기존 값과 새 업데이트를 받아 병합된 값을 반환하는 함수다. 따라서 단순 리스트 누적에는 `operator.add`가 충분할 수 있지만, 채팅 메시지에는 보통 LangGraph의 메시지 전용 reducer를 쓰는 편이 낫다.

```python
from typing import Annotated
from typing_extensions import TypedDict
from langchain_core.messages import AnyMessage
from langgraph.graph.message import add_messages


class State(TypedDict):
    messages: Annotated[list[AnyMessage], add_messages]
```

`add_messages`는 새 메시지를 누적하면서도 메시지 ID가 같은 경우 교체할 수 있고, dict 형태의 메시지를 LangChain message 객체로 역직렬화하는 처리도 지원한다. 대화형 에이전트에서는 직접 reducer를 붙이기보다 `MessagesState`를 사용하는 경우가 많다.

주의할 점:

```python
class State(TypedDict):
    messages: list[str]
```

이렇게 reducer 없이 정의하면, 새 `messages`가 기존 메시지를 대체할 수 있다. 대화 기록을 다룰 때 흔한 실수다.

---

## 9. MessagesState

채팅 기반 에이전트에서는 직접 `messages` 상태를 정의하는 대신 `MessagesState`를 사용할 수 있다.

개념적으로는 다음과 같은 상태다.

```python
{
    "messages": [
        {"role": "user", "content": "..."},
        {"role": "assistant", "content": "..."}
    ]
}
```

LangGraph의 RAG 튜토리얼도 `MessagesState`를 사용해서 메시지 목록을 그래프 상태로 다룬다.

`MessagesState`는 개념적으로 다음 정의와 비슷하다.

```python
from typing import Annotated
from typing_extensions import TypedDict
from langchain_core.messages import AnyMessage
from langgraph.graph.message import add_messages


class MessagesState(TypedDict):
    messages: Annotated[list[AnyMessage], add_messages]
```

따라서 `return {"messages": [response]}`처럼 새 메시지만 반환해도 기존 대화 기록에 누적된다. 직접 `messages: list[...]`를 정의하면 기본 동작은 덮어쓰기이므로 같은 결과가 나오지 않을 수 있다.

사용 예시:

```python
from langgraph.graph import MessagesState


def call_model(state: MessagesState):
    messages = state["messages"]
    response = model.invoke(messages)
    return {"messages": [response]}
```

---

## 10. Persistence: Checkpointer와 Store

LangGraph의 Persistence는 크게 두 가지로 나뉜다.

| 구분 | Checkpointer | Store |
|---|---|---|
| 저장 대상 | 그래프 State 스냅샷 | 애플리케이션 정의 데이터 |
| 범위 | 하나의 thread | 여러 thread / 장기 데이터 |
| 용도 | 대화 연속성, 재개, 휴먼 승인, 장애 복구 | 사용자 선호, 사실, 공유 지식 |
| 접근 방식 | `thread_id` 기반 | key-value 또는 namespace 기반 |

---

## 10.1 Checkpointer

Checkpointer는 그래프 실행 중간 상태를 저장한다.

개발용 예제:

```python
from langgraph.checkpoint.memory import InMemorySaver

checkpointer = InMemorySaver()

graph = builder.compile(checkpointer=checkpointer)

config = {
    "configurable": {
        "thread_id": "user-1"
    }
}

result = graph.invoke(
    {"messages": [{"role": "user", "content": "내 이름은 민수야."}]},
    config=config,
)
```

같은 `thread_id`로 다시 호출하면 checkpointer가 해당 thread의 checkpoint를 기준으로 상태를 관리한다.

후속 대화를 이어갈 때는 보통 plain dict를 다시 입력한다.

```python
result = graph.invoke(
    {"messages": [{"role": "user", "content": "내 이름이 뭐라고 했지?"}]},
    config=config,
)
```

이때 graph는 새 입력을 `START`부터 처리하지만, 같은 thread의 checkpointed state를 활용한다. `messages`가 `MessagesState` 또는 `add_messages` reducer로 정의되어 있어야 이전 메시지와 새 메시지가 자연스럽게 누적된다.

반대로 사람 승인 등으로 `interrupt()`에서 멈춘 실행을 재개할 때는 plain dict가 아니라 `Command(resume=...)`를 사용한다. 일반 후속 대화를 `Command(update=...)`로 이어가려고 하면 마지막 checkpoint에서 재개하려고 하기 때문에 이미 종료된 graph에서는 기대와 다르게 동작할 수 있다.

주의:

- `InMemorySaver`는 개발·테스트용이다.
- 서버 재시작 시 메모리가 사라질 수 있다.
- 운영 환경에서는 DB 기반 checkpointer를 고려해야 한다.
- checkpointer를 사용할 때는 실행 config에 `thread_id`를 명시해야 한다.

---

## 10.2 Store

Store는 thread를 넘어서는 장기 데이터를 저장할 때 사용한다.

예:

- 사용자 선호
- 반복적으로 사용하는 프로필 정보
- 장기 기억
- 여러 대화에서 공유되는 지식

개념 코드:

```python
from langgraph.store.memory import InMemoryStore

store = InMemoryStore()

graph = builder.compile(store=store)
```

Checkpointer와 Store는 동시에 사용할 수 있다.

```python
graph = builder.compile(
    checkpointer=checkpointer,
    store=store,
)
```

---

## 11. Human-in-the-loop

LangGraph는 사람의 승인이나 개입이 필요한 워크플로우에 적합하다.

예:

```text
사용자 요청
  ↓
초안 생성
  ↓
위험도 검토
  ↓
사람 승인 필요?
  ├─ 예 → 실행 중단 후 승인 대기
  └─ 아니오 → 자동 처리
  ↓
승인 후 재개
```

이 구조가 가능한 이유는 그래프 상태가 checkpointer에 저장되기 때문이다.

적합한 사례:

- 이메일 자동 발송 전 승인
- 결제·환불 처리 전 승인
- 법률·의료·재무 고위험 답변 검토
- 고객지원 답변 초안 검수
- 코드 변경사항 병합 전 리뷰

설계 기준:

1. 승인 전 단계와 승인 후 단계를 다른 노드로 나눈다.
2. 외부 부작용이 있는 작업은 승인 이후 노드에서만 수행한다.
3. 승인 상태를 State에 명확히 저장한다.
4. 재개 시 중복 실행되어도 문제가 없도록 idempotency를 고려한다.

실제 LangGraph에서는 `interrupt()`로 실행을 멈추고, 같은 `thread_id`에서 `Command(resume=...)`로 다시 시작한다.

```python
from typing_extensions import NotRequired, TypedDict
from langgraph.checkpoint.memory import InMemorySaver
from langgraph.graph import StateGraph, START, END
from langgraph.types import Command, interrupt


class State(TypedDict):
    draft: str
    approved: NotRequired[bool]
    send_status: NotRequired[str]


def review_node(state: State):
    decision = interrupt({
        "message": "이 초안을 발송해도 되나요?",
        "draft": state["draft"],
    })

    return {"approved": decision["approved"]}


def send_node(state: State):
    if not state["approved"]:
        return {"send_status": "rejected"}

    # 실제 이메일 발송, 결제, DB 변경 같은 side effect는 여기에서 수행한다.
    return {"send_status": "sent"}


builder = StateGraph(State)
builder.add_node("review", review_node)
builder.add_node("send", send_node)
builder.add_edge(START, "review")
builder.add_edge("review", "send")
builder.add_edge("send", END)

graph = builder.compile(checkpointer=InMemorySaver())

config = {"configurable": {"thread_id": "approval-1"}}

paused = graph.invoke({"draft": "답변 초안입니다."}, config=config)

resumed = graph.invoke(
    Command(resume={"approved": True}),
    config=config,
)
```

주의할 점은 resume 시 `interrupt()`가 호출된 노드가 처음부터 다시 실행된다는 것이다. 따라서 `interrupt()` 앞에 이메일 발송, 결제, DB insert 같은 side effect를 두면 재개 시 중복 실행될 수 있다.

---

## 12. Subgraph

Subgraph는 그래프 안에 들어가는 또 다른 그래프다.

개념적으로는 다음과 같다.

```text
Parent Graph
  ├─ Node A
  ├─ Subgraph X
  │   ├─ Node X1
  │   └─ Node X2
  └─ Node B
```

사용 이유:

- 복잡한 워크플로우를 작은 단위로 분리
- 멀티 에이전트 구조에서 각 에이전트를 독립 그래프로 구성
- 테스트와 재사용성 개선
- 큰 그래프의 가독성 개선

Subgraph를 사용할 때는 parent graph와 subgraph가 어떻게 상태를 주고받는지 먼저 정해야 한다.

| 방식 | 사용 상황 |
|---|---|
| node 안에서 subgraph invoke | parent와 subgraph의 state schema가 다르거나 변환이 필요할 때 |
| compiled subgraph를 node로 추가 | parent와 subgraph가 공유 state key를 읽고 쓸 때 |

Subgraph persistence는 `compile(checkpointer=...)` 설정에 따라 의미가 달라진다.

| 설정 | 의미 |
|---|---|
| `checkpointer=None` 또는 생략 | 기본값. 호출마다 subgraph 내부 상태는 새로 시작하지만, parent checkpointer를 통해 한 호출 안의 interrupt와 durable execution은 지원한다. |
| `checkpointer=True` | 같은 thread에서 subgraph 내부 상태도 호출 간 누적된다. subagent가 독립적인 multi-turn memory를 가져야 할 때 사용한다. |
| `checkpointer=False` | subgraph checkpoint를 쓰지 않는 stateless 실행이다. interrupt나 durable execution도 기대하지 않는다. |

즉 “parent graph의 checkpointer가 subgraph에도 전파된다”는 말은 기본값에서 일부 맞지만, subgraph 내부 상태가 항상 thread 단위로 계속 유지된다는 뜻은 아니다. 호출 간 내부 상태를 유지하려면 명시적으로 per-thread persistence를 선택해야 한다.

---

## 12.1 Command

조건부 엣지는 “라우팅 함수가 다음 노드를 결정한다”는 구조다.

하지만 어떤 노드가 state를 업데이트하면서 동시에 다음 노드까지 결정해야 하는 경우가 있다. 이때 `Command`를 사용할 수 있다.

```python
from typing import Literal
from langgraph.types import Command


def classify_and_route(state: State) -> Command[Literal["rag_node", "direct_node"]]:
    question = state["question"]

    if "검색" in question or "출처" in question:
        return Command(
            update={"route": "rag"},
            goto="rag_node",
        )

    return Command(
        update={"route": "direct"},
        goto="direct_node",
    )
```

사용 기준:

| 상황 | 권장 방식 |
|---|---|
| state 업데이트 없이 다음 노드만 고른다 | conditional edge |
| state 업데이트와 routing을 한 함수에서 같이 처리한다 | `Command(update=..., goto=...)` |
| interrupt 이후 외부 입력으로 재개한다 | `Command(resume=...)` |
| subgraph 내부에서 parent graph의 노드로 이동한다 | `Command(..., graph=Command.PARENT)` |

주의할 점은 `Command(goto=...)`를 반환하는 노드에 일반 `add_edge()`도 연결해두면 둘 다 실행될 수 있다는 것이다. 한 노드의 다음 이동 방식은 static edge, conditional edge, `Command` 중 하나로 명확히 정하는 편이 좋다.

---

## 12.2 Send

`Send`는 실행 시점에 동적으로 여러 작업을 만들 때 사용한다.

예를 들어 문서 10개를 각각 평가한 뒤 결과를 모아야 한다면, 미리 edge 10개를 정의할 수 없다. 이때 조건부 edge에서 `Send` 객체 목록을 반환하면 같은 노드를 여러 입력으로 병렬 실행할 수 있다.

```python
from langgraph.types import Send


def fanout_documents(state: State):
    return [
        Send(
            "grade_one_document",
            {
                "question": state["question"],
                "document": document,
            },
        )
        for document in state["documents"]
    ]


builder.add_conditional_edges("retrieve_documents", fanout_documents)
```

`Send`는 map-reduce 패턴에 자주 쓰인다.

```text
retrieve_documents
  ↓
grade_one_document x N
  ↓
collect_grades
```

여러 병렬 노드의 결과를 하나의 state key에 모으려면 reducer가 필요하다. 예를 들어 `graded_documents`를 리스트로 누적하려면 `Annotated[..., operator.add]` 같은 reducer를 정의해야 한다.

---

## 13. RAG에서 LangGraph를 쓰는 방법

단순 RAG는 다음 구조다.

```text
사용자 질문 → 문서 검색 → 답변 생성
```

이 정도면 LangGraph 없이도 구현할 수 있다.

하지만 다음 구조라면 LangGraph가 유리하다.

```text
사용자 질문
  ↓
질문 분류
  ↓
검색 필요 여부 판단
  ├─ 검색 불필요 → 직접 답변
  └─ 검색 필요 → 검색 쿼리 생성
                  ↓
                문서 검색
                  ↓
                검색 결과 관련성 평가
                  ↓
                충분한가?
                  ├─ 아니오 → 질문 재작성 후 재검색
                  └─ 예 → 답변 생성
                              ↓
                            출처 검증
                              ↓
                            최종 답변
```

LangGraph 공식 RAG 튜토리얼도 “검색할지 직접 답할지 LLM이 결정하는 retrieval agent”를 예시로 든다.

---

## 14. RAG 그래프 설계 예시

State:

```python
from typing_extensions import TypedDict


class RAGState(TypedDict):
    question: str
    rewritten_query: str
    documents: list[str]
    answer: str
    relevance_score: float
    retry_count: int
```

그래프 구조:

```text
START
  ↓
classify_question
  ↓
조건부 분기
  ├─ direct_answer → END
  └─ rewrite_query
        ↓
      retrieve_documents
        ↓
      grade_documents
        ↓
      조건부 분기
        ├─ retry_search → rewrite_query
        └─ generate_answer → verify_answer → END
```

노드 책임:

| 노드 | 책임 |
|---|---|
| `classify_question` | 검색 필요 여부 판단 |
| `rewrite_query` | 검색용 쿼리 생성 |
| `retrieve_documents` | 벡터 DB / 검색 API 호출 |
| `grade_documents` | 검색 결과의 관련성 평가 |
| `generate_answer` | 검색 문서 기반 답변 생성 |
| `verify_answer` | 출처 누락, 환각, 논리 오류 검증 |
| `retry_search` | 재검색 횟수 증가 및 조건 판단 |

중요한 점은 “검색 → 답변”을 한 노드에 몰아넣지 않는 것이다. 각 단계를 분리해야 관찰, 재시도, 테스트, 디버깅이 쉬워진다.

---

## 15. 멀티 에이전트에서 LangGraph를 쓰는 방법

멀티 에이전트 구조는 여러 역할의 에이전트를 노드 또는 subgraph로 표현할 수 있다.

예:

```text
START
  ↓
Planner Agent
  ↓
Research Agent
  ↓
Writer Agent
  ↓
Critic Agent
  ↓
충분한가?
  ├─ 아니오 → Research Agent 또는 Writer Agent로 회귀
  └─ 예 → END
```

각 에이전트 역할:

| 에이전트 | 역할 |
|---|---|
| Planner | 작업 분해, 계획 수립 |
| Researcher | 자료 검색, 근거 수집 |
| Writer | 최종 응답 초안 작성 |
| Critic | 오류, 누락, 환각, 출처 검증 |
| Executor | 외부 API 호출 또는 실제 작업 수행 |

주의할 점:

1. 에이전트를 너무 많이 만들면 비용과 지연 시간이 증가한다.
2. 역할이 겹치면 오히려 품질이 떨어진다.
3. Planner가 잘못된 계획을 세우면 전체 흐름이 왜곡된다.
4. Critic은 반드시 명확한 평가 기준을 가져야 한다.
5. 무한 루프 방지를 위해 `max_retry`, `max_steps`를 둔다.

---

## 16. 법률 RAG 게시판 답변 초안 시스템 예시

연습용 프로젝트 예시로 다음 구조를 생각할 수 있다.

```text
사용자 분쟁 글 작성
  ↓
case_parser
  ↓
issue_identifier
  ↓
law_retriever
  ↓
case_law_retriever
  ↓
legal_reasoning_drafter
  ↓
risk_checker
  ↓
final_answer_generator
```

State 예시:

```python
class LegalRAGState(TypedDict):
    user_post: str
    facts: list[str]
    legal_issues: list[str]
    required_sources: list[str]
    laws: list[str]
    precedents: list[str]
    draft_answer: str
    risk_flags: list[str]
    final_answer: str
```

노드별 책임:

| 노드 | 책임 |
|---|---|
| `case_parser` | 사용자 글에서 사실관계 추출 |
| `issue_identifier` | 법적 쟁점 후보 추출 |
| `law_retriever` | 법령 API 검색 |
| `case_law_retriever` | 판례 검색 |
| `legal_reasoning_drafter` | 근거 기반 답변 초안 작성 |
| `risk_checker` | 단정적 법률 판단, 변호사 조언 오인 가능성 점검 |
| `final_answer_generator` | 최종 답변 생성 |

주의:

이런 시스템은 학습용으로는 적합하지만, 실제 법률 서비스를 운영할 경우 법률광고, 변호사법, 개인정보, 민감정보, 책임 고지, 출처 정확성 문제가 생길 수 있다.

---

## 17. 노드 설계 기준

LangGraph에서 노드 크기는 중요하다.

공식 문서의 “Thinking in LangGraph”도 노드 세분화가 복원력, 관찰성, 테스트 용이성과 관련된다고 설명한다. 노드가 너무 크면 실패 시 해당 노드 전체를 다시 실행해야 한다.

권장 기준:

| 기준 | 설명 |
|---|---|
| 한 노드 한 책임 | 검색, 분류, 생성, 저장을 한 노드에 몰지 않는다. |
| 외부 API는 별도 노드 | 실패, 재시도, 타임아웃을 독립적으로 처리하기 쉽다. |
| LLM 판단은 관찰 가능하게 분리 | 라우팅 결과를 State에 저장한다. |
| 부작용은 idempotent하게 | 재실행되어도 중복 결제, 중복 발송이 없어야 한다. |
| 테스트 가능한 단위로 분리 | 각 노드를 함수 단위로 테스트할 수 있어야 한다. |

나쁜 예:

```python
def everything_node(state):
    # 질문 분류
    # 검색
    # 답변 생성
    # DB 저장
    # 이메일 발송
    # 로그 기록
    return {...}
```

좋은 예:

```text
classify_question
retrieve_documents
generate_answer
save_answer
send_notification
```

---

## 18. Side Effect 설계

Side effect란 외부 세계를 변경하는 작업이다.

예:

- DB insert/update
- 이메일 발송
- 결제 요청
- Git commit
- 파일 삭제
- 외부 API POST 호출

LangGraph에서 side effect는 특히 주의해야 한다. checkpointer를 사용하는 경우 장애 복구나 재개 과정에서 특정 노드가 다시 실행될 수 있기 때문이다.

설계 원칙:

1. side effect 노드는 가능한 한 작게 만든다.
2. 요청 ID, 작업 ID, idempotency key를 사용한다.
3. 이미 처리된 작업인지 DB에서 확인한다.
4. LLM 판단과 실제 실행을 분리한다.
5. 사람 승인 후 실행되도록 한다.

예:

```python
def send_email_node(state):
    email_id = state["email_id"]

    if email_already_sent(email_id):
        return {"send_status": "already_sent"}

    send_email(...)
    mark_email_as_sent(email_id)

    return {"send_status": "sent"}
```

---

## 19. 에러 처리와 재시도

LangGraph 자체가 모든 장애를 자동으로 해결해주는 것은 아니다. 각 노드의 실패 가능성을 설계해야 한다.

고려할 항목:

| 실패 유형 | 대응 |
|---|---|
| LLM API 오류 | retry, fallback model |
| 검색 API 오류 | timeout, retry, graceful degradation |
| DB 오류 | transaction, retry, rollback |
| 잘못된 LLM 출력 | structured output, schema validation |
| 검색 결과 부족 | query rewrite, retry limit |
| 무한 루프 | max_retry, max_steps |

일시적인 네트워크 오류나 rate limit처럼 같은 작업을 다시 시도하면 해결될 수 있는 문제는 node에 retry policy를 붙일 수 있다.

```python
from langgraph.types import RetryPolicy


builder.add_node(
    "retrieve_documents",
    retrieve_documents,
    retry_policy=RetryPolicy(
        max_attempts=3,
        initial_interval=1.0,
    ),
)
```

다만 retry policy는 모든 오류의 해결책이 아니다. 잘못된 LLM 출력, 검색 결과 부족, 사용자의 추가 정보 필요 같은 문제는 state에 실패 원인을 남기고 다른 노드로 routing하거나 `interrupt()`로 사람 입력을 받는 식으로 설계해야 한다.

루프가 있는 graph는 recursion limit도 고려해야 한다. LangGraph는 한 번의 실행에서 허용되는 최대 super-step 수를 넘으면 `GraphRecursionError`를 발생시킨다.

```python
result = graph.invoke(
    input_state,
    config={
        "recursion_limit": 50,
        "configurable": {
            "thread_id": "thread-1",
        },
    },
)
```

`recursion_limit`은 `configurable` 안에 넣지 않고 config의 최상위 키로 전달한다. 이것은 비정상 루프의 최후 안전장치일 뿐이고, graph 자체에도 `retry_count`, `max_steps`, fallback 경로 같은 명시적 종료 조건을 두는 편이 좋다.

State에 실패 정보를 넣을 수도 있다.

```python
class State(TypedDict):
    question: str
    answer: str
    errors: list[str]
    retry_count: int
```

---

## 20. Streaming

LangGraph는 스트리밍에도 사용할 수 있다.

스트리밍이 필요한 경우:

- 긴 답변을 생성할 때 사용자에게 중간 결과를 보여주고 싶다.
- 각 노드의 실행 상태를 UI에 표시하고 싶다.
- 에이전트가 어떤 단계를 거치는지 관찰하고 싶다.

개념:

```python
for chunk in graph.stream(input_state):
    print(chunk)
```

실무에서는 어떤 정보를 받고 싶은지에 따라 `stream_mode`를 지정한다.

```python
for chunk in graph.stream(
    input_state,
    stream_mode="updates",
):
    print(chunk)
```

또는 비동기 방식:

```python
async for chunk in graph.astream(input_state):
    print(chunk)
```

최신 stream-mode API에서는 `version="v2"`를 지정하면 여러 stream mode를 함께 사용해도 일관된 형태의 chunk를 받을 수 있다.

```python
for part in graph.stream(
    input_state,
    stream_mode=["updates", "messages"],
    version="v2",
):
    if part["type"] == "updates":
        print("state update:", part["data"])
    elif part["type"] == "messages":
        print("message chunk:", part["data"])
```

Human-in-the-loop처럼 interrupt를 UI에서 처리해야 하는 경우에는 `graph.stream_events(..., version="v3")`를 사용하면 interrupt 여부와 resume 처리를 더 명확히 다룰 수 있다.

실제 사용 시에는 stream mode와 version에 따라 출력 형태가 달라질 수 있으므로, UI 코드에서는 출력 스키마를 먼저 고정하고 구현해야 한다.

---

## 21. LangSmith와 Observability

LangGraph를 운영 환경에서 사용하면 관찰성이 중요하다.

관찰해야 할 항목:

- 어떤 노드가 실행되었는가
- 각 노드의 입력과 출력은 무엇인가
- 어느 조건부 엣지가 선택되었는가
- LLM 호출 비용과 latency는 얼마인가
- 어떤 검색 결과가 답변에 사용되었는가
- 실패한 노드는 어디인가

LangSmith는 LangGraph 실행 경로, state transition, LLM 호출, 평가, 디버깅을 추적하는 데 사용할 수 있다.

---

## 22. LangGraph가 적합한 경우와 부적합한 경우

### 적합한 경우

- 복잡한 RAG 파이프라인
- 멀티 에이전트 시스템
- 도구 호출 루프
- 휴먼 승인 워크플로우
- 장시간 실행되는 에이전트
- 상태를 저장하고 재개해야 하는 작업
- 실패 복구가 필요한 자동화

### 부적합한 경우

- 단순 챗봇
- 단순 프롬프트 호출
- 검색 한 번 후 답변하는 간단한 RAG
- 상태 관리가 거의 없는 API 래퍼
- 그래프 구조가 오히려 코드 복잡도를 높이는 경우

판단 기준:

```text
분기, 루프, 상태 저장, 재개, 승인, 멀티 에이전트가 없다면
LangGraph 없이 시작하는 편이 더 단순하다.
```

---

## 23. 대안 기술 비교

| 도구 | 목적 | LangGraph와의 차이 |
|---|---|---|
| 직접 API 호출 | 단순 LLM 호출 | 가장 단순하지만 흐름 관리 직접 구현 필요 |
| LangChain LCEL | 체인 구성 | 그래프 기반 상태·루프에는 LangGraph가 유리 |
| LangChain Agent | 일반 도구 호출 에이전트 | LangGraph보다 고수준, 구조 커스터마이징은 제한적 |
| CrewAI | 역할 기반 멀티 에이전트 | 더 추상화되어 있지만 세밀한 상태 제어는 LangGraph가 유리할 수 있음 |
| AutoGen | 멀티 에이전트 대화 | 대화형 에이전트 협업에 강점 |
| Temporal | 일반 워크플로우 엔진 | LLM 특화는 아니지만 내구성 있는 업무 워크플로우에 강함 |
| Airflow | 배치 파이프라인 | 데이터 파이프라인에 적합, 대화형 에이전트와는 목적이 다름 |

GPT의 견해:{LangGraph는 “에이전트를 쉽게 만드는 도구”라기보다 “복잡해진 에이전트를 통제 가능한 소프트웨어 구조로 만드는 도구”에 가깝다. 초기에 너무 빨리 도입하면 과설계가 될 수 있지만, RAG 검증 루프나 멀티 에이전트가 들어가는 순간 유용성이 커진다.}

---

## 24. 프로젝트 구조 예시

실제 프로젝트에서는 다음처럼 나눌 수 있다.

```text
app/
  main.py
  graph/
    __init__.py
    state.py
    nodes.py
    edges.py
    builder.py
  services/
    llm_service.py
    retrieval_service.py
    document_service.py
  repositories/
    conversation_repository.py
  schemas/
    request.py
    response.py
  tests/
    test_nodes.py
    test_graph.py
```

각 파일 역할:

| 파일 | 역할 |
|---|---|
| `state.py` | State 타입 정의 |
| `nodes.py` | 노드 함수 정의 |
| `edges.py` | 라우팅 함수 정의 |
| `builder.py` | 그래프 조립 및 compile |
| `services/` | LLM, 검색, 외부 API 호출 |
| `repositories/` | DB 접근 |
| `tests/` | 노드 및 그래프 테스트 |

---

## 25. 실전 구현 순서

LangGraph 프로젝트는 다음 순서로 구현하는 것이 좋다.

```text
1. 문제를 그래프로 표현할 가치가 있는지 판단한다.
2. State에 들어갈 값을 정의한다.
3. 전체 흐름을 ASCII 다이어그램으로 그린다.
4. 각 Node의 책임을 한 문장으로 정의한다.
5. Edge와 조건부 Edge를 설계한다.
6. LLM 없이 더미 함수로 그래프를 먼저 실행한다.
7. 그 다음 LLM 호출과 도구 호출을 붙인다.
8. Checkpointer를 붙인다.
9. 실패, 재시도, 중복 실행을 테스트한다.
10. LangSmith 또는 로그로 실행 경로를 관찰한다.
```

처음부터 LLM API를 붙이면 디버깅 포인트가 너무 많아진다. 먼저 순수 Python 함수로 그래프 흐름을 검증하는 것이 낫다.

---

## 26. 자주 발생하는 실수

### 실수 1. State에 모든 것을 넣는다

State가 비대해지면 노드 간 결합도가 올라간다.

해결:

- 다음 노드가 필요한 값만 넣는다.
- 장기 저장 데이터는 Store나 DB로 분리한다.

### 실수 2. 노드 하나에 너무 많은 책임을 넣는다

디버깅, 재시도, 관찰이 어려워진다.

해결:

- LLM 판단, 검색, 저장, 외부 실행을 분리한다.

### 실수 3. 무한 루프를 만든다

조건부 엣지로 루프를 만들 때 종료 조건이 없으면 위험하다.

해결:

```python
if state["retry_count"] >= 3:
    return "fallback_answer"
```

운영 코드에서는 graph 실행 config에 `recursion_limit`도 설정해서 비정상 루프가 무한히 계속되지 않도록 한다.

### 실수 4. reducer를 지정하지 않고 메시지를 누적하려 한다

기존 메시지가 덮어써질 수 있다.

해결:

- `MessagesState` 사용
- 또는 `Annotated[..., reducer]` 사용

### 실수 5. 외부 부작용을 재실행 가능하게 만들지 않는다

장애 복구 시 이메일이 두 번 발송되거나 DB가 중복 저장될 수 있다.

해결:

- idempotency key 사용
- 처리 완료 여부 확인
- side effect 노드 분리

---

## 27. 테스트 전략

LangGraph 테스트는 세 단계로 나누는 것이 좋다.

### 27.1 노드 단위 테스트

```python
def test_classify_question_routes_to_rag():
    state = {"question": "최신 LangGraph 문서를 검색해주세요"}
    result = classify_question(state)
    assert result["route"] == "rag"
```

### 27.2 라우팅 함수 테스트

```python
def test_route_question():
    state = {"route": "direct"}
    assert route_question(state) == "direct"
```

### 27.3 그래프 통합 테스트

```python
def test_graph_direct_answer():
    result = graph.invoke({
        "question": "LangGraph가 무엇인가요?"
    })
    assert "answer" in result
```

LLM 호출이 들어가는 테스트는 mock 또는 fixture를 사용하는 편이 낫다.

---

## 28. 성능 고려사항

LangGraph 자체보다 보통 다음 요소가 병목이 된다.

| 병목 | 설명 |
|---|---|
| LLM latency | 모델 응답 시간이 가장 큰 비중을 차지하는 경우가 많다. |
| 검색 latency | 벡터 DB, 검색 API, 외부 문서 API가 느릴 수 있다. |
| 반복 루프 | 재검색·재생성이 많으면 비용과 시간이 증가한다. |
| 멀티 에이전트 | 여러 LLM 호출이 누적된다. |
| 체크포인트 저장 | DB 기반 persistence 사용 시 저장 비용이 생긴다. |

최적화 방법:

1. 불필요한 LLM 노드를 줄인다.
2. 라우팅은 가능하면 규칙 기반으로 먼저 처리한다.
3. 검색 결과 평가를 모든 문서에 대해 LLM으로 하지 않는다.
4. retry 횟수를 제한한다.
5. 캐시를 사용한다.
6. 병렬 실행 가능한 노드는 병렬화한다.

---

## 29. 보안 고려사항

LangGraph는 실행 흐름을 도와주는 도구이지 보안 솔루션이 아니다.

주의해야 할 보안 이슈:

- Prompt injection
- Tool injection
- 검색 문서 내 악성 지시문
- 외부 API 권한 오남용
- 개인정보 State 저장
- 로그에 민감정보 노출
- 사람이 승인해야 할 작업을 자동 실행

대응:

1. 도구 권한을 최소화한다.
2. 검색 문서 내용을 시스템 지시보다 낮은 신뢰도로 취급한다.
3. 외부 실행 전 validation 노드를 둔다.
4. 민감정보는 State와 로그에 불필요하게 저장하지 않는다.
5. 고위험 작업은 human-in-the-loop로 처리한다.

---

## 30. 학습 로드맵

### 1단계: 그래프 기본 구조

목표:

- StateGraph 생성
- Node 추가
- Edge 추가
- invoke 실행

실습:

- 단일 노드 그래프 만들기
- 두 노드 순차 실행 그래프 만들기

### 2단계: 조건부 분기

목표:

- `add_conditional_edges` 이해
- 라우팅 함수 작성
- state 업데이트와 routing을 함께 처리할 때 `Command` 사용
- 동적 fan-out이 필요할 때 `Send` 사용

실습:

- 질문을 `rag`, `direct`, `reject`로 분류하는 그래프 만들기
- 여러 문서를 `Send`로 나누어 평가하고 reducer로 결과 모으기

### 3단계: 루프

목표:

- 반복 실행 구조 이해
- 종료 조건 설계

실습:

- 검색 결과가 부족하면 최대 3번 재검색하는 그래프 만들기

### 4단계: LLM 연결

목표:

- 노드 안에서 LLM 호출
- structured output 사용

실습:

- 질문 분류를 규칙 기반에서 LLM 기반으로 변경

### 5단계: RAG 연결

목표:

- 검색 노드 구현
- 문서 관련성 평가 노드 구현

실습:

- 로컬 문서 기반 RAG 그래프 만들기

### 6단계: Persistence

목표:

- Checkpointer 사용
- thread_id 기반 대화 지속성 이해
- 일반 후속 입력과 `Command(resume=...)` 재개의 차이 이해

실습:

- 같은 thread_id로 이전 대화 기억하기

### 7단계: Human-in-the-loop

목표:

- 승인 대기 구조 이해
- 중단 후 재개 설계
- `interrupt()`와 `Command(resume=...)` 사용

실습:

- 이메일 초안 생성 후 승인되면 발송하는 구조 설계

### 8단계: 운영 설계

목표:

- 로그, 테스트, retry, idempotency 설계
- `RetryPolicy`, `recursion_limit`, stream mode 설정

실습:

- 실패하는 검색 API를 mock으로 만들고 재시도 처리
- streaming 출력 형식을 고정하고 UI에서 처리하기

---

## 31. 연습 문제

### 문제 1. 기본 그래프

다음 State를 가진 그래프를 만들어라.

```python
class State(TypedDict):
    name: str
    greeting: str
```

입력:

```python
{"name": "민수"}
```

출력:

```python
{"name": "민수", "greeting": "안녕하세요, 민수님"}
```

---

### 문제 2. 조건부 분기

질문에 “최신”, “검색”, “출처”가 포함되면 `rag_node`로 보내고, 아니면 `direct_node`로 보내라.

---

### 문제 3. 재시도 루프

검색 결과가 비어 있으면 최대 3번까지 query를 재작성하고 다시 검색하라.

State 예시:

```python
class State(TypedDict):
    question: str
    query: str
    documents: list[str]
    retry_count: int
    answer: str
```

---

### 문제 4. 검증 노드 추가

답변 생성 후 `verify_answer` 노드를 추가하라.

검증 기준:

- 답변이 비어 있지 않은가
- 출처가 포함되어 있는가
- “확실합니다” 같은 과도한 단정 표현이 있는가

---

### 문제 5. 멀티 에이전트 구조

다음 역할을 각각 노드로 구현하라.

- Planner
- Researcher
- Writer
- Critic

Critic이 `pass=False`를 반환하면 Writer로 되돌아가게 하라.

---

## 32. 실무 체크리스트

프로젝트에 LangGraph를 도입하기 전에 다음 질문에 답해야 한다.

```text
[도입 판단]
- 단순 체인으로 충분하지 않은가?
- 분기, 루프, 재시도, 휴먼 승인이 필요한가?
- 상태 저장과 재개가 필요한가?

[State 설계]
- State에 꼭 필요한 값만 들어가는가?
- 장기 데이터와 단기 상태가 분리되어 있는가?
- 메시지 누적에는 reducer 또는 MessagesState를 사용하는가?

[Node 설계]
- 각 노드는 하나의 책임만 갖는가?
- 외부 API 호출 노드는 분리되어 있는가?
- 노드 단위 테스트가 가능한가?

[Edge 설계]
- 모든 조건부 분기에 종료 경로가 있는가?
- 무한 루프 방지 조건이 있는가?
- 라우팅 결과가 관찰 가능한가?
- `Command`를 쓰는 노드에 불필요한 static edge를 같이 연결하지 않았는가?
- `Send`로 병렬 fan-out한 결과를 모을 reducer가 있는가?

[운영]
- checkpointer가 설정되어 있는가?
- production에서 InMemory 저장소에 의존하지 않는가?
- interrupt 재개는 같은 thread_id와 `Command(resume=...)`를 사용하는가?
- `RetryPolicy`와 `recursion_limit`이 필요한 graph에 설정되어 있는가?
- streaming 출력 mode와 version을 명확히 정했는가?
- side effect가 idempotent한가?
- 로그에 민감정보가 남지 않는가?
- LangSmith 또는 별도 tracing이 있는가?
```

---

## 33. 핵심 용어 정리

| 용어 | 의미 |
|---|---|
| State | 그래프 전체가 공유하는 현재 상태 |
| Node | 실제 작업을 수행하는 함수 |
| Edge | 다음 노드를 결정하는 연결 |
| Conditional Edge | 조건에 따라 다음 노드를 선택하는 엣지 |
| START | 그래프 시작점 |
| END | 그래프 종료점 |
| StateGraph | 공유 State를 중심으로 노드들이 통신하는 그래프 클래스 |
| Reducer | State 업데이트 병합 방식 |
| add_messages | 메시지 누적과 메시지 ID 기반 교체를 처리하는 메시지 전용 reducer |
| MessagesState | `messages` key와 메시지 reducer를 미리 갖춘 채팅용 State |
| Checkpointer | thread 단위 그래프 상태 저장 장치 |
| Store | 장기 기억 또는 애플리케이션 데이터 저장 장치 |
| Thread | 하나의 대화 또는 실행 흐름 단위 |
| Subgraph | 그래프 안에서 노드처럼 사용되는 하위 그래프 |
| Command | state update, routing, interrupt resume 등을 표현하는 제어 객체 |
| Send | 실행 시점에 여러 노드 호출을 동적으로 만드는 fan-out 객체 |
| Interrupt | 실행을 중단하고 외부 입력을 기다리는 기능 |
| RetryPolicy | 일시적 실패에 대해 노드 실행을 재시도하는 정책 |
| Recursion limit | 한 graph 실행에서 허용되는 최대 super-step 수 |
| Stream mode | streaming으로 받을 출력 종류를 정하는 설정 |
| Human-in-the-loop | 사람 승인 또는 개입을 포함하는 실행 구조 |
| Agent Loop | LLM과 도구 호출을 반복하는 구조 |

---

## 34. 최소 템플릿

새 LangGraph 프로젝트를 시작할 때 사용할 수 있는 최소 템플릿이다.

```python
from typing_extensions import TypedDict
from langgraph.graph import StateGraph, START, END


class State(TypedDict):
    input: str
    output: str


def process_node(state: State):
    return {
        "output": f"processed: {state['input']}"
    }


def build_graph():
    builder = StateGraph(State)
    builder.add_node("process", process_node)
    builder.add_edge(START, "process")
    builder.add_edge("process", END)
    return builder.compile()


if __name__ == "__main__":
    graph = build_graph()
    result = graph.invoke({"input": "hello"})
    print(result)
```

---

## 35. 추천 학습 순서 요약

```text
1. StateGraph 최소 예제 실행
2. 조건부 엣지 구현
3. Command와 Send로 동적 제어 흐름 구현
4. 루프 구현과 recursion_limit 설정
5. MessagesState로 채팅 기록 누적
6. LLM 노드 연결
7. Tool node 연결
8. Checkpointer로 thread memory 추가
9. interrupt와 Command(resume=...)로 human-in-the-loop 구현
10. RAG 그래프 구현
11. Critic/Verifier 노드 추가
12. 멀티 에이전트 또는 subgraph로 확장
13. RetryPolicy, streaming, observability 보강
```

---

## 36. 참고 자료

아래 자료는 작성일 기준 공식 문서와 API 레퍼런스를 우선했다.

1. LangGraph Overview  
   https://docs.langchain.com/oss/python/langgraph/overview

2. LangGraph Graph API  
   https://docs.langchain.com/oss/python/langgraph/graph-api

3. StateGraph API Reference  
   https://reference.langchain.com/python/langgraph/graph/state/StateGraph

4. Persistence  
   https://docs.langchain.com/oss/python/langgraph/persistence

5. Add Memory  
   https://docs.langchain.com/oss/python/langgraph/add-memory

6. Thinking in LangGraph  
   https://docs.langchain.com/oss/python/langgraph/thinking-in-langgraph

7. Build a custom RAG agent with LangGraph  
   https://docs.langchain.com/oss/python/langgraph/agentic-rag

8. Checkpointers  
   https://docs.langchain.com/oss/python/langgraph/checkpointers

9. Stores  
   https://docs.langchain.com/oss/python/langgraph/stores

10. Interrupts  
   https://docs.langchain.com/oss/python/langgraph/interrupts

11. Subgraphs  
   https://docs.langchain.com/oss/python/langgraph/use-subgraphs

12. Streaming  
   https://docs.langchain.com/oss/python/langgraph/streaming

13. Workflows and agents  
   https://docs.langchain.com/oss/python/langgraph/workflows-agents

---

## 37. 마지막 정리

LangGraph를 배울 때 가장 중요한 관점은 “그래프” 자체가 아니다.

핵심은 다음이다.

```text
LLM 애플리케이션을 단순 프롬프트 호출이 아니라
상태를 가진 실행 시스템으로 설계하는 것
```

따라서 LangGraph 학습의 본질은 다음 질문에 답하는 능력이다.

1. 무엇을 State에 저장할 것인가?
2. 어떤 작업을 Node로 분리할 것인가?
3. 어떤 조건에서 다음 Node로 이동할 것인가?
4. 실패하거나 중단되었을 때 어디서 재개할 것인가?
5. LLM의 판단을 어디까지 신뢰하고, 어디서 검증할 것인가?

이 질문에 답할 수 있으면 LangGraph를 단순 예제 수준이 아니라 실제 RAG·에이전트 시스템 설계 도구로 사용할 수 있다.
