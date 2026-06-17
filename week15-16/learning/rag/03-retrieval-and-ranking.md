# Retrieval과 Ranking

## Retrieval의 목적

Retrieval은 사용자 질문에 답하는 데 필요한 근거 chunk를 찾는 단계입니다.

RAG에서 LLM이 좋은 답을 하려면 먼저 좋은 근거가 들어와야 합니다. 그래서 retrieval 품질은 generation 품질의 상한선을 결정합니다.

## Vector search

Vector search는 query와 chunk를 embedding vector로 바꾼 뒤, vector 공간에서 가까운 항목을 찾는 방식입니다.

기본 흐름:

```text
사용자 query
  -> query embedding
  -> chunk embedding과 거리 계산
  -> 가까운 chunk top-k 반환
```

장점:

- 같은 단어가 없어도 의미가 비슷하면 찾을 수 있습니다.
- 자연어 질문에 잘 맞습니다.
- 유의어와 표현 차이에 강합니다.

단점:

- 정확한 키워드, 숫자, 조문번호 검색은 약할 수 있습니다.
- embedding model 품질에 의존합니다.
- vector 공간이 다르면 비교할 수 없습니다.

## Distance metric

vector 유사도 계산에는 여러 metric이 있습니다.

| Metric | 의미 | 특징 |
| --- | --- | --- |
| cosine similarity | 방향 유사도 | 텍스트 embedding에서 흔히 사용 |
| dot product | 내적 | model에 따라 사용 |
| Euclidean distance | 거리 | vector 크기의 영향을 받음 |

중요한 것은 embedding model이 권장하는 metric을 사용하는 것입니다.

## Top-k

Top-k는 검색 결과를 몇 개 가져올지 정하는 값입니다.

```text
top_k = 5
```

작게 잡으면:

- prompt가 짧아집니다.
- precision이 높을 수 있습니다.
- 필요한 근거가 빠질 수 있습니다.

크게 잡으면:

- recall이 높아질 수 있습니다.
- prompt가 길어집니다.
- 노이즈가 증가합니다.
- 비용과 latency가 증가합니다.

실무에서는 top-k를 고정하기보다 문서 유형, query 유형, reranker 유무에 따라 조정하기도 합니다.

## Metadata filtering

metadata filter는 검색 범위를 줄이는 장치입니다.

예:

```text
document_type = statute
effective_date <= 사건 발생일
jurisdiction = KR
source_type = official
```

장점:

- 무관한 문서를 줄입니다.
- 오래된 문서나 잘못된 corpus를 제외할 수 있습니다.
- 사용자 권한에 맞는 문서만 검색할 수 있습니다.

주의:

- filter가 너무 강하면 관련 문서가 빠집니다.
- metadata 품질이 낮으면 검색 품질도 낮아집니다.

## Keyword search

Keyword search는 단어 일치 기반 검색입니다.

예:

- BM25
- PostgreSQL full-text search
- Elasticsearch

장점:

- 정확한 용어, 코드, 조문번호, 사건번호 검색에 강합니다.
- 결과 설명이 상대적으로 쉽습니다.

단점:

- 유의어와 표현 차이에 약합니다.
- 자연어 질문과 문서 표현이 다르면 놓칠 수 있습니다.

## Hybrid search

Hybrid search는 vector search와 keyword search를 결합합니다.

예:

```text
vector 결과 top 50
keyword 결과 top 50
  -> score 결합
  -> reranking
  -> final top 8
```

결합 방식:

- score normalization
- reciprocal rank fusion
- weighted sum
- reranker 기반 재정렬

법률, 계약, 기술 문서처럼 정확한 용어가 중요한 분야에서는 hybrid search가 유리한 경우가 많습니다.

## Query rewriting

사용자 질문을 검색에 적합한 query로 바꾸는 과정입니다.

예:

사용자 질문:

```text
집주인이 보증금을 안 돌려주면 어떻게 해야 하나요?
```

검색 query:

```text
임대차 보증금 반환 지연 임대인 반환 의무
```

query rewriting은 LLM을 사용할 수도 있고, 규칙 기반으로 할 수도 있습니다.

주의:

- 원래 질문의 의미를 바꾸면 안 됩니다.
- rewrite query와 원본 query를 모두 로그에 남기면 디버깅에 좋습니다.

## Query decomposition

복합적인 사실관계는 하나의 query로 검색하면 중요한 쟁점이 누락될 수 있습니다.

예:

```text
임대차 보증금 미반환 + 협박 문자
```

이 경우 하나의 검색어로 처리하기보다 다음처럼 나누는 것이 더 안전할 수 있습니다.

```text
1. 임대차 보증금 반환 수리비 공제
2. 임차인 원상회복 누수 노후 배관
3. 가족에게 찾아가겠다는 문자 협박 강요
```

이 방식은 쟁점별 recall을 높이는 데 도움이 됩니다. 다만 query가 늘어나면 검색 비용, reranking 비용, 중복 제거 부담이 증가합니다.

## Parent-child retrieval

검색에 좋은 단위와 사용자에게 보여주기 좋은 단위는 다를 수 있습니다.

예:

```text
child chunk: 조문 제1항, 판례 판단 중 한 문단
parent unit: 전체 조문, 판례의 판단 부분, 문서의 section
```

작은 child chunk로 검색하면 precision이 좋아질 수 있고, parent unit을 함께 보여주면 사용자가 문맥을 이해하기 쉽습니다. 법률 문서에서는 조문, 항, 호가 나뉘어 검색되더라도 citation은 전체 조문이나 관련 항목 묶음으로 보여주는 편이 더 자연스러운 경우가 많습니다.

## Reranking

Reranking은 후보 검색 결과를 다시 정렬하는 단계입니다.

1차 retrieval은 빠르게 많이 가져오는 역할을 하고, reranking은 더 정확히 상위 결과를 고르는 역할을 합니다.

방법:

- cross-encoder model
- LLM relevance scoring
- metadata priority
- source trust score
- 최신성 score

Reranking은 비용이 들기 때문에 보통 top 50 또는 top 100 후보에만 적용합니다.

## Score 해석

vector score는 절대적인 정답 확률이 아닙니다.

주의:

- model마다 score scale이 다릅니다.
- 같은 query 안에서는 상대 비교가 가능하지만, 다른 query 간 직접 비교는 어렵습니다.
- threshold는 데이터셋으로 검증해야 합니다.
- DB나 라이브러리에 따라 score의 방향이 다를 수 있습니다. 예를 들어 cosine similarity는 높을수록 유사하지만, pgvector의 cosine distance는 낮을수록 가깝습니다.

## Retrieval evaluation

retrieval을 평가하려면 정답 근거가 표시된 평가 데이터가 필요합니다.

평가 지표:

- Recall@k: 정답 chunk가 top-k 안에 들어왔는가
- Precision@k: top-k 중 관련 chunk 비율
- MRR: 첫 정답 chunk의 순위
- nDCG: 관련도 등급을 고려한 ranking 품질

RAG 시스템에서는 generation 평가보다 먼저 retrieval 평가를 해야 합니다. 정답 근거가 검색되지 않으면 LLM이 자체 지식으로 답할 수는 있어도, 근거 기반 답변을 보장하기 어렵기 때문입니다.

## 흔한 retrieval 문제

### 관련 문서가 top-k에 없음

가능한 원인:

- chunking 문제
- embedding model 부적합
- query 표현 문제
- metadata filter 과도
- index 누락

### 비슷한 chunk가 반복됨

가능한 원인:

- overlap이 큼
- 중복 문서 제거 부족
- 같은 문서의 유사 chunk가 너무 많음

대응:

- document-level diversity
- chunk deduplication
- max chunks per document 제한

### 오래된 문서가 상위에 나옴

가능한 원인:

- 최신성 metadata가 ranking에 반영되지 않음
- 버전 관리가 없음

대응:

- effective_date filter
- version priority
- source trust score

## Retrieval 설계 체크리스트

- query와 chunk가 같은 embedding profile을 사용하는가
- metadata filter가 필요한가
- top-k가 너무 작거나 크지 않은가
- 복합 쟁점은 query decomposition이 필요한가
- keyword search가 필요한 도메인인가
- reranking이 필요한가
- 검색 chunk와 citation/display 단위를 분리해야 하는가
- 같은 문서에서 너무 많은 chunk가 나오지 않는가
- retrieval 결과를 run 단위로 저장하는가
- score와 rank를 나중에 재현할 수 있는가
