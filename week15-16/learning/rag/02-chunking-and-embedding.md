# Chunking과 Embedding

## Chunking이 중요한 이유

RAG에서 chunking은 문서를 작게 나누는 단순 작업이 아닙니다. 검색 품질과 citation 품질을 결정하는 핵심 설계입니다.

chunk가 너무 크면:

- 검색 결과가 느슨해집니다.
- 하나의 chunk 안에 여러 주제가 섞입니다.
- LLM prompt에 불필요한 정보가 많이 들어갑니다.
- citation이 부정확해집니다.

chunk가 너무 작으면:

- 문맥이 끊깁니다.
- 답변에 필요한 조건이나 예외가 빠질 수 있습니다.
- 검색 결과가 파편화됩니다.
- top-k 안에 필요한 조각이 모두 들어오지 않을 수 있습니다.

## Chunk의 기본 단위

문서 유형에 따라 chunk 기준은 달라져야 합니다.

| 문서 유형 | 추천 기준 |
| --- | --- |
| 일반 문서 | heading, paragraph |
| 기술 문서 | section, subsection, code block |
| 계약서 | 조, 항, 호, 별지 |
| 법령 | 조문, 항, 호 |
| 판례 | 사건 개요, 쟁점, 판단, 결론 |
| FAQ | 질문과 답변 한 쌍 |
| 채팅 기록 | 발화 묶음 또는 주제 전환 단위 |

좋은 chunking은 문서의 의미 구조를 먼저 보고, 글자 수나 token 수는 보조 기준으로 사용합니다.

## 고정 길이 chunking

가장 단순한 방식은 일정 글자 수나 token 수로 자르는 것입니다.

예:

```text
max_chars = 1000
overlap_chars = 100
```

장점:

- 구현이 쉽습니다.
- 모든 문서에 일관되게 적용됩니다.
- 빠르게 baseline을 만들 수 있습니다.

단점:

- 문장이나 조항이 중간에 끊길 수 있습니다.
- 구조적 문서에는 부적합할 수 있습니다.
- citation 품질이 낮아질 수 있습니다.

## 구조 기반 chunking

문서의 heading, 조문, paragraph, XML tag 같은 구조를 기준으로 나누는 방식입니다.

장점:

- 의미 단위가 보존됩니다.
- citation이 자연스럽습니다.
- 법률, 계약, 정책 문서에 적합합니다.

단점:

- parser가 필요합니다.
- 문서 유형별 규칙이 필요합니다.
- 구조가 깨진 문서에서는 fallback이 필요합니다.

## Overlap의 의미

Overlap은 chunk 사이에 일부 텍스트를 겹치게 넣는 방식입니다.

```text
chunk 1: A B C D E
chunk 2: D E F G H
```

목적:

- 경계에서 문맥이 끊기는 문제 완화
- query와 관련 문구가 chunk 경계에 걸쳐 있을 때 recall 개선

주의:

- overlap이 너무 크면 중복 저장이 늘어납니다.
- 검색 결과에 비슷한 chunk가 반복될 수 있습니다.
- citation이 중복될 수 있습니다.

## Chunk metadata

chunk에는 본문만 저장하면 안 됩니다. 검색과 citation을 위해 metadata가 필요합니다.

예:

```text
document_id
chunk_index
title
heading
section_path
page_number
article_number
source_url
published_date
effective_date
document_type
checksum
```

metadata는 다음에 사용됩니다.

- filter
- citation
- source 표시
- 중복 제거
- reranking
- audit

## Embedding이란 무엇인가

Embedding은 텍스트를 숫자 vector로 변환한 것입니다.

의미가 비슷한 텍스트는 vector 공간에서 가까운 위치에 놓이도록 학습됩니다.

예:

```text
"임대차 보증금 반환"과 "전세보증금 돌려받기"는 가까운 vector가 될 가능성이 높습니다.
```

## Embedding model 선택 기준

embedding model을 고를 때 볼 요소:

- 지원 언어
- 도메인 적합성
- dimension
- 비용
- latency
- batch 처리 가능 여부
- 입력 token 제한
- deprecation 정책
- private data 처리 정책

한국어 문서라면 한국어 의미 검색 품질을 반드시 별도 평가해야 합니다.

## Dimension의 의미

dimension은 vector 길이입니다.

예:

```text
1536차원 vector = 숫자 1536개
3072차원 vector = 숫자 3072개
```

dimension이 크다고 항상 좋은 것은 아닙니다.

고차원 장점:

- 의미 표현력이 높을 수 있습니다.

고차원 단점:

- 저장 공간 증가
- 검색 비용 증가
- index 제약 증가
- latency 증가 가능성

## 서로 다른 embedding 공간

서로 다른 embedding model의 vector는 보통 같은 공간에서 직접 비교하면 안 됩니다.

예:

```text
model A의 1536차원 vector
model B의 1536차원 vector
```

둘 다 1536차원이라도 의미 공간이 다를 수 있습니다. 따라서 같은 retrieval ranking에서는 같은 provider/model/dimension/profile로 생성된 vector만 비교해야 합니다.

## Embedding profile

실무에서는 embedding 설정을 profile로 관리하는 것이 좋습니다.

예:

```text
profile_id: 1
provider: openai
model_name: text-embedding-3-small
dimensions: 1536
distance_metric: cosine
status: active
```

profile을 두면 다음이 쉬워집니다.

- model 변경
- dimension 변경
- A/B test
- re-embedding
- deprecation 대응
- 검색 재현성 확보

## Content checksum

embedding row에는 embedding 대상 text의 checksum을 저장하는 것이 좋습니다.

이유:

- chunk 본문이 바뀌었는지 확인할 수 있습니다.
- 기존 embedding이 stale인지 판단할 수 있습니다.
- 같은 chunk/profile에 중복 embedding을 만들지 않을 수 있습니다.

흐름:

```text
chunk.content 변경
  -> checksum 변경
  -> 기존 embedding stale
  -> 재임베딩 필요
```

## Embedding 실패 처리

embedding 실패는 원문 문서를 손상시키면 안 됩니다.

권장 상태:

```text
pending
embedded
failed
stale
```

실패 시 저장할 것:

- 실패 상태
- 안전하게 정리된 오류 메시지
- retry 가능 여부
- 대상 profile
- 대상 chunk

저장하지 말 것:

- API key
- Authorization header
- provider raw request 전문
- 불필요한 개인정보

## 실무 권장 흐름

```text
1. chunk 생성
2. embedding profile 선택
3. chunk content checksum 계산
4. chunk/profile embedding row 조회
5. 이미 embedded이고 checksum 같으면 skip
6. checksum 다르면 stale로 보고 재임베딩
7. provider batch embedding 호출
8. 응답 개수와 dimension 검증
9. vector 저장
10. 실패 row는 failed로 저장
```
