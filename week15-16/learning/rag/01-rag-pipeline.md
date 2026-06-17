# RAG 파이프라인

## 전체 흐름

RAG는 하나의 기능처럼 보이지만 내부적으로는 여러 단계가 연결된 파이프라인입니다.

```text
문서 수집
  -> parsing
  -> normalization
  -> chunking
  -> embedding
  -> indexing
  -> retrieval
  -> reranking
  -> prompt assembly
  -> generation
  -> citation validation
  -> logging/evaluation
```

각 단계는 입력과 출력이 명확해야 합니다. 그래야 나중에 품질 문제를 디버깅할 수 있습니다.

## 1. 문서 수집

문서 수집은 RAG corpus에 들어갈 원천 자료를 확보하는 단계입니다.

입력 예:

- PDF
- HTML
- XML
- Markdown
- 일반 텍스트
- API 응답
- 데이터베이스 row
- 사용자가 업로드한 파일

저장해야 할 정보:

- source provider
- source URL
- external ID
- 수집 시각
- 문서 유형
- 원문
- 원문 checksum
- 라이선스 또는 이용 조건

원칙:

- 원문을 가능한 보존합니다.
- 재수집 가능하도록 source metadata를 남깁니다.
- 이용 조건이 불명확한 scraping은 피합니다.
- secret API key는 저장하지 않습니다.

## 2. Parsing

Parsing은 원천 파일이나 응답에서 텍스트와 구조를 추출하는 단계입니다.

예:

- PDF에서 text 추출
- HTML에서 본문만 추출
- XML tag에서 조문 구조 추출
- Word 문서에서 heading과 paragraph 추출

주의할 점:

- PDF는 시각적 순서와 text 추출 순서가 다를 수 있습니다.
- 스캔 PDF는 OCR이 필요합니다.
- 표, 각주, 머리말, 꼬리말이 본문에 섞일 수 있습니다.
- parsing 실패 시 원문을 버리면 안 됩니다.

## 3. Normalization

Normalization은 검색과 중복 판정이 가능하도록 텍스트를 정리하는 단계입니다.

일반 처리:

- 줄바꿈 정리
- 연속 공백 정리
- 제어 문자 제거
- Unicode normalization
- 불필요한 header/footer 제거
- 문서 구조 보존

중요한 점은 "정리"와 "훼손"을 구분하는 것입니다. 법률, 계약, 정책 문서에서는 조항 번호나 문장 경계가 의미를 가질 수 있으므로 과도하게 단순화하면 안 됩니다.

권장 저장값:

- raw_text
- normalized_text
- raw_checksum
- normalized_checksum

## 4. Chunking

Chunking은 긴 문서를 검색 가능한 작은 단위로 나누는 단계입니다.

chunk는 두 역할을 가집니다.

- retrieval 단위
- citation 단위

좋은 chunk는 다음 조건을 만족합니다.

- 하나의 의미 단위가 지나치게 쪼개지지 않습니다.
- 너무 길어서 검색 precision을 낮추지 않습니다.
- citation으로 보여줄 때 사용자가 이해할 수 있습니다.
- 문서 내 위치와 source metadata를 유지합니다.

## 5. Embedding

Embedding은 text를 vector로 바꾸는 단계입니다.

```text
"계약 해지 통보" -> [0.012, -0.134, 0.552, ...]
```

embedding vector는 의미적으로 비슷한 텍스트끼리 가까운 위치에 놓이도록 만들어집니다.

저장해야 할 정보:

- embedding provider
- embedding model
- dimension
- distance metric
- vector
- embedding 생성 시각
- embedding 대상 text checksum
- 실패 상태

주의:

- model이 바뀌면 vector 공간도 바뀝니다.
- dimension이 다르면 같은 vector index에서 비교할 수 없습니다.
- 같은 문서를 여러 model로 embedding할 수 있습니다.

## 6. Indexing

Indexing은 검색을 빠르게 하기 위해 DB나 검색 엔진에 자료 구조를 만드는 단계입니다.

vector index 예:

- HNSW
- IVFFlat

keyword index 예:

- PostgreSQL full-text search
- Elasticsearch
- OpenSearch
- BM25 기반 검색

초기 개발에서는 index 없이 정확한 결과부터 확인하고, 데이터가 커질 때 index를 추가하는 것이 안전합니다.

## 7. Retrieval

Retrieval은 사용자 질문과 관련된 chunk를 찾는 단계입니다.

기본 흐름:

```text
사용자 질문
  -> query embedding
  -> vector similarity search
  -> top-k chunk 반환
```

검색 품질을 높이기 위해 다음을 추가할 수 있습니다.

- metadata filter
- query rewriting
- hybrid search
- reranking
- threshold filtering

## 8. Reranking

Reranking은 1차 검색 결과를 다시 정렬하는 단계입니다.

1차 retrieval은 빠른 후보 생성에 가깝습니다. reranking은 더 비싸지만 정확한 방식으로 상위 결과를 재정렬합니다.

예:

- cross-encoder reranker
- LLM 기반 relevance 판단
- rule 기반 source priority
- 최신성 가중치

## 9. Prompt assembly

검색된 chunk를 LLM prompt에 넣는 단계입니다.

좋은 prompt assembly는 다음을 명확히 합니다.

- 사용자의 질문
- 사용 가능한 근거
- 답변 형식
- citation 규칙
- 모르면 모른다고 말하는 규칙
- 근거 밖 추론 제한

검색 결과를 instruction처럼 넣으면 prompt injection 위험이 있습니다. 검색된 문서는 instruction이 아니라 evidence data로 취급해야 합니다.

## 10. Generation

LLM이 최종 답변을 생성하는 단계입니다.

RAG generation에서는 다음이 중요합니다.

- 근거 chunk를 실제로 사용했는지
- citation이 맞는지
- 근거가 부족할 때 추측하지 않는지
- 사용자에게 필요한 형식으로 답하는지

## 11. Citation validation

Citation validation은 모델이 만든 citation이 실제 검색 결과에 있는지 검증하는 단계입니다.

검증 예:

- 답변의 chunk ID가 이번 run에서 검색된 chunk인지 확인
- source URL이 실제 source metadata와 일치하는지 확인
- 인용한 문장이 chunk 내용에 존재하는지 확인
- citation 없는 법률/정책 주장을 제거하거나 한계로 표시

## 12. Logging과 evaluation

RAG는 운영하면서 계속 개선해야 하므로 기록이 중요합니다.

저장할 것:

- 사용자 query
- retrieval 결과 chunk ID
- score
- embedding profile
- prompt version
- generation provider/model
- 최종 답변
- 실패 원인

저장하지 말아야 할 것:

- API key
- Authorization header
- raw JWT
- 불필요한 개인정보
- 전체 provider raw response

## 디버깅 순서

RAG 답변이 나쁠 때는 다음 순서로 확인합니다.

1. 원문 문서가 corpus에 들어갔는가
2. normalization 후 중요한 문구가 사라지지 않았는가
3. chunk가 적절히 나뉘었는가
4. embedding이 생성되었는가
5. query가 올바른 profile로 embedding되었는가
6. top-k에 관련 chunk가 들어왔는가
7. reranking 또는 filter가 관련 chunk를 제거하지 않았는가
8. prompt에 필요한 근거가 들어갔는가
9. 모델이 근거를 따르도록 지시되었는가
10. citation 검증이 있는가
