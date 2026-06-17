# RAG 설계 체크리스트

## 1. 문제 정의

- RAG가 필요한 문제인가
- 검색해야 할 문서 corpus가 명확한가
- 답변에 citation이 필요한가
- 최신성이나 버전이 중요한가
- 사용자가 답변 근거를 직접 확인해야 하는가

## 2. 문서 수집

- 문서 source가 합법적이고 이용 조건이 명확한가
- 원문을 저장하는가
- source URL 또는 external ID를 저장하는가
- 수집 시각을 저장하는가
- raw checksum을 계산하는가
- 중복 문서를 어떻게 처리할지 정했는가

## 3. Parsing과 normalization

- 파일 유형별 parser가 있는가
- parsing 실패 시 원문을 보존하는가
- OCR이 필요한 문서를 구분하는가
- normalization 기준이 문서 의미를 훼손하지 않는가
- normalized checksum을 저장하는가
- 문서 구조를 metadata로 보존하는가

## 4. Chunking

- 문서 유형별 chunk 기준이 있는가
- retrieval chunk와 citation/display 단위를 어떻게 설계할지 정했는가
- chunk 크기 상한이 있는가
- 너무 짧은 chunk 병합 정책이 있는가
- overlap이 필요한가
- parent-child retrieval이 필요한 문서 유형인가
- chunk_index가 안정적인가
- chunk metadata가 충분한가

## 5. Embedding

- embedding provider/model/dimension을 profile로 관리하는가
- 같은 profile의 vector끼리만 비교하는가
- dimension을 코드에 하드코딩하지 않았는가
- content checksum으로 stale 여부를 판단하는가
- embedding 실패 상태를 저장하는가
- provider 오류를 안전한 오류로 변환하는가
- API key를 로그에 남기지 않는가

## 6. Vector 저장소

- vector column 또는 vector DB가 dimension 전략과 맞는가
- index 없이 baseline 정확도를 먼저 확인했는가
- index 추가 후 결과 품질이 달라지지 않는지 확인했는가
- profile별 index 전략이 있는가
- 백업과 migration 전략이 있는가

## 7. Retrieval

- query embedding profile이 chunk embedding profile과 같은가
- top-k 기본값이 평가로 정해졌는가
- metadata filter가 필요한가
- 복합 쟁점을 query decomposition으로 나누어야 하는가
- keyword search가 필요한가
- hybrid search를 고려했는가
- reranking이 필요한가
- 검색 결과 압축 또는 evidence compression이 필요한가
- retrieval 결과를 run 단위로 저장하는가

## 8. Prompt assembly

- 검색 결과를 evidence로 취급하는가
- 문서 내부 명령문을 instruction으로 따르지 않게 했는가
- citation 규칙이 명확한가
- 답변 형식이 명확한가
- 근거 부족 시 답변 정책이 있는가
- prompt version을 관리하는가

## 9. Generation

- generation provider와 embedding provider를 분리했는가
- model metadata를 저장하는가
- provider timeout을 설정했는가
- provider 오류를 사용자에게 안전하게 전달하는가
- raw provider response를 불필요하게 저장하지 않는가

## 10. Citation

- citation ID가 실제 retrieval 결과에 있는가
- source title과 URL을 함께 제공하는가
- citation 없는 핵심 주장을 탐지하는가
- citation ID 존재 여부와 주장-근거 정합성을 구분해 검증하는가
- 모델이 임의 source를 만들 수 없게 했는가
- citation 검증 실패 시 어떻게 처리할지 정했는가

## 11. Evaluation

- retrieval 평가셋이 있는가
- generation 평가셋이 있는가
- Recall@k를 측정하는가
- citation accuracy를 측정하는가
- 답변 groundedness를 평가하는가
- 회귀 테스트에 사용할 fixture가 있는가

## 12. 보안과 개인정보

- secret을 출력하지 않는가
- auth token을 저장하지 않는가
- 사용자 개인정보를 최소 저장하는가
- prompt injection 대응이 있는가
- 파일 업로드 size limit이 있는가
- 외부 API timeout과 rate limit 처리가 있는가
- 사용자별 권한 filter가 필요한가

## 13. 운영

- embedding job retry 정책이 있는가
- failed/stale 상태를 재처리할 수 있는가
- model deprecation 시 새 profile로 전환할 수 있는가
- 비용과 latency를 측정하는가
- observability 로그가 있는가
- admin이 ingestion 상태를 확인할 수 있는가

## 14. 최소 구현 순서

처음부터 완성형 RAG를 만들 필요는 없습니다. 다음 순서로 작게 검증하는 것이 좋습니다.

```text
1. 작은 fixture corpus 준비
2. normalization 구현
3. chunking 구현
4. mock embedding 저장
5. vector retrieval baseline
6. retrieval 평가셋 작성
7. prompt assembly
8. generation 연결
9. citation validation
10. OpenAI 등 실제 provider 연결
11. hybrid search와 reranking 개선
12. 운영용 retry, audit, admin 기능 추가
```

## 15. 가장 중요한 질문

RAG 설계에서 계속 확인해야 할 질문은 하나입니다.

```text
이 답변은 어떤 근거에서 나왔고, 그 근거를 다시 확인할 수 있는가?
```

이 질문에 답할 수 없다면 RAG의 핵심 장점인 재현성과 신뢰성이 약해집니다.
