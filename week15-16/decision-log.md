# Week 15-16 Decision Log

> 15-16주차 게시판 과제와 나만무 팀 준비 과정에서 나중에 설명해야 할 주요 결정을 기록한다.
>
> 작은 UI 선택이나 변수명 같은 세부 구현 선택은 기록하지 않고, 일정, repository 운영, branch 전략, 기술 스택처럼 협업과 발표/회고에 영향을 주는 결정만 남긴다.

---

## 2026-06-05 - 기술 스택은 React + FastAPI + PostgreSQL + pgvector + OpenAI API

- Status: Decided
- Context: 이번 과제는 React 프론트엔드, 선택한 백엔드 프레임워크, 관계형 DB, 상용 LLM, RAG/MCP/Agent 기능을 요구한다. 팀은 나만무 본 프로젝트 준비까지 고려해 공통으로 익힐 기술을 정해야 했다.
- Decision: 기술 스택은 React + FastAPI + PostgreSQL + pgvector + OpenAI API로 정한다.
- Reason: FastAPI는 Python 기반 AI 기능 구현과 연결하기 쉽고, PostgreSQL과 pgvector를 함께 사용하면 기본 게시판 데이터와 vector search 기반 RAG 확장을 같은 DB 흐름 안에서 다룰 수 있다.
- Consequence: 팀원은 동일한 웹/DB/AI 기반을 학습한다. PostgreSQL, pgvector, FastAPI 구조를 이해해야 하며, OpenAI API key와 비용 관리 원칙을 지켜야 한다.

---

## 2026-06-06 - 6월 10일까지 AI 없는 기본 게시판을 먼저 구현

- Status: Decided
- Context: RAG, MCP, Agent를 붙이기 전에 React, FastAPI, PostgreSQL 기반의 기본 게시판 요청-응답 흐름에 익숙해질 시간이 필요하다.
- Decision: 2026-06-10까지는 AI 기능이 붙지 않은 기본 게시판 템플릿을 각자 먼저 구현한다.
- Reason: 기본 게시판 구조를 충분히 이해하지 못한 상태에서 AI 기능을 붙이면 API, DB, 인증/인가, 화면 흐름을 설명하기 어려워진다.
- Consequence: AI 기능 구현 시간은 줄어들 수 있지만, 이후 공통 템플릿 기반으로 RAG/MCP/Agent를 붙일 때 구조를 더 명확히 이해할 수 있다.

---

## 2026-06-06 - 6월 11일부터 새 공통 repo에서 개인 branch 분기

- Status: Decided
- Context: 6월 10일까지 각자 기본 게시판 구현에 익숙해진 뒤, 6월 11일부터는 팀 공통 템플릿을 기준으로 AI 기능을 확장해야 한다.
- Decision: 2026-06-11부터 새 팀 공통 repository의 main을 기준으로 각자 개인 branch를 분기한다.
- Reason: 공통 템플릿을 기준으로 시작하면 기본 게시판 구조는 맞추면서도, 개인 branch에서 각자 주제에 맞는 AI 기능을 독립적으로 구현할 수 있다.
- Consequence: 개인별 구현 결과를 비교하기 쉬워진다. 다만 branch 분기 이후에는 각자 구현 방향이 달라지므로 개인 branch 관리와 README 정리가 중요하다.

---

## 2026-06-06 - 개인 branch는 main에 다시 병합하지 않음

- Status: Decided
- Context: 이번 과제는 팀 공동 구현이 아니라 개인 과제다. 각자 RAG, MCP, Agent 기능과 주제가 달라질 수 있다.
- Decision: main에서 개인 branch를 최초 분기한 뒤, 개인 branch의 작업은 main에 다시 병합하지 않는다.
- Reason: 개인 과제의 독립성을 유지하고, 서로 다른 AI 기능 구현이 공통 템플릿을 오염시키거나 충돌시키는 것을 막기 위해서다.
- Consequence: main은 공통 템플릿 기준으로 유지된다. 각 개인 branch는 제출/데모/README 기준으로 독립 관리해야 한다.

