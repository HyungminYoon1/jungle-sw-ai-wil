# RAG 개요

## RAG란 무엇인가

RAG는 Retrieval-Augmented Generation의 약자입니다. 한국어로는 보통 검색 증강 생성이라고 부릅니다.

LLM은 학습된 지식을 바탕으로 문장을 생성하지만, 다음 한계가 있습니다.

- 최신 정보나 사내 문서처럼 학습 데이터에 없는 내용을 모를 수 있습니다.
- 특정 문서의 정확한 문구, 조항, 수치, 근거를 기억하지 못할 수 있습니다.
- 모르는 내용을 그럴듯하게 만들어내는 hallucination이 발생할 수 있습니다.
- 답변의 근거가 어디에서 왔는지 추적하기 어렵습니다.

RAG는 이 문제를 줄이기 위해 모델에게 바로 답하게 하지 않고, 먼저 관련 문서를 검색한 뒤 그 검색 결과를 근거로 답변하게 만드는 구조입니다.

```text
사용자 질문
  -> 관련 문서 검색
  -> 검색 결과를 prompt에 포함
  -> LLM이 근거 기반 답변 생성
```

핵심은 LLM을 지식 저장소로만 쓰지 않고, 외부 문서 저장소와 결합한다는 점입니다.

## RAG가 필요한 경우

RAG는 다음 상황에서 특히 유용합니다.

- 문서가 자주 바뀝니다.
- 답변에 근거 문서가 필요합니다.
- 사내 문서, 계약서, 정책 문서, 법령, 판례처럼 특정 corpus를 기준으로 답해야 합니다.
- 사용자가 "어디에 근거한 답변인지" 확인해야 합니다.
- 모델을 매번 fine-tuning하기 어렵거나 비용이 큽니다.

반대로 다음 경우에는 RAG가 꼭 필요하지 않을 수 있습니다.

- 일반 상식형 질문만 처리합니다.
- 근거 문서 추적이 중요하지 않습니다.
- 검색할 외부 문서가 없습니다.
- 매우 짧고 고정된 지식만 필요합니다.

## RAG와 fine-tuning의 차이

RAG와 fine-tuning은 목적이 다릅니다.

| 구분 | RAG | Fine-tuning |
| --- | --- | --- |
| 주요 목적 | 외부 지식 검색과 근거 제공 | 모델의 말투, 형식, 작업 습관 조정 |
| 지식 업데이트 | 문서 DB와 검색 색인을 갱신하면 됨 | 모델을 다시 학습해야 할 수 있음 |
| 출처 추적 | 상대적으로 쉬움 | 어렵거나 별도 설계 필요 |
| 비용 구조 | 검색/임베딩/저장 비용 | 학습 비용과 모델 운영 비용 |
| 적합한 예 | 법률 문서 검색, 사내 문서 QA | 특정 답변 형식, 분류 기준, 스타일 학습 |

실무에서는 둘을 함께 쓰기도 합니다.

예를 들어 RAG로 최신 문서를 검색하고, fine-tuning된 모델로 답변 형식이나 도메인별 작성 습관을 맞출 수 있습니다.

다만 RAG의 지식 업데이트도 단순히 원문 파일만 바꾸면 끝나는 것은 아닙니다. 문서가 바뀌면 보통 parsing, normalization, chunking, embedding, indexing, version 관리가 함께 갱신되어야 합니다.

## RAG의 기본 구성요소

RAG 시스템은 보통 다음 구성요소를 가집니다.

```text
문서 수집
  -> 문서 정제
  -> chunking
  -> embedding
  -> vector 저장소
  -> query embedding
  -> retrieval
  -> reranking/filtering
  -> prompt 구성
  -> generation
  -> citation/evaluation
```

각 단계는 독립적으로 품질에 영향을 줍니다. RAG의 성능 문제는 "모델이 나쁘다"로 단순화하면 안 됩니다. 실제로는 chunking, metadata, retrieval query, reranking, prompt, evaluation 중 하나가 원인인 경우가 많습니다.

## RAG에서 자주 발생하는 실패

### 관련 문서를 못 찾음

검색 단계가 실패한 경우입니다. 이때 LLM은 좋은 답을 만들 수 없습니다.

원인:

- chunk가 너무 크거나 작습니다.
- embedding model이 도메인 문서를 잘 표현하지 못합니다.
- query가 사용자의 질문 그대로라 검색에 부적합합니다.
- metadata filter가 너무 강합니다.
- 중요한 문서가 index되지 않았습니다.

### 관련 문서는 찾았지만 답변이 틀림

generation 단계 또는 prompt 구성 문제가 원인일 수 있습니다.

원인:

- 검색 결과가 너무 많아 모델이 핵심 근거를 놓칩니다.
- 상충하는 문서가 함께 들어갔지만 우선순위가 없습니다.
- prompt가 근거 없는 추론을 허용합니다.
- citation 요구가 약합니다.

### 답변은 맞지만 근거가 부정확함

RAG에서는 답변 내용만이 아니라 근거 연결도 품질입니다.

원인:

- chunk ID와 citation mapping이 약합니다.
- 문서 일부만 보고 전체 문서의 결론처럼 답합니다.
- retrieval 결과와 generation 결과의 citation 검증이 없습니다.

## RAG 설계의 핵심 원칙

1. 검색이 실패하면 근거 기반 답변 품질이 크게 낮아집니다.
2. MVP에서는 chunk가 검색 단위이면서 citation 단위가 될 수 있습니다.
3. metadata는 vector만큼 중요합니다.
4. 서로 다른 embedding model이나 dimension의 vector를 같은 공간에서 비교하면 안 됩니다.
5. 답변 품질은 top-k 정확도, citation 정확도, prompt 정책이 함께 결정합니다.
6. RAG는 한 번에 완성하기보다 평가 데이터를 두고 반복 개선해야 합니다.

실무에서는 검색 단위와 사용자에게 보여줄 인용 단위가 다를 수 있습니다. 예를 들어 작은 chunk로 검색한 뒤, 답변에는 해당 chunk가 속한 조문, 문단, 페이지, 판례 판단 부분처럼 더 큰 parent 단위를 함께 보여주는 방식이 가능합니다.

## 최소 RAG 예시

```text
1. 문서를 paragraph 단위로 나눈다.
2. 각 paragraph를 embedding한다.
3. vector DB에 저장한다.
4. 사용자 질문을 embedding한다.
5. 가장 가까운 paragraph 5개를 찾는다.
6. LLM에게 "아래 근거만 사용해서 답하라"고 요청한다.
7. 답변과 사용한 paragraph ID를 함께 저장한다.
```

이것이 가장 단순한 RAG입니다. 실무에서는 여기에 normalization, metadata filter, hybrid search, reranking, citation validation, audit log가 추가됩니다.
