# Week 13-14 mini GPT 구현 계획

> 기간: 2026-05-25(월) 밤 ~ 2026-06-03(수)
>
> 목표: PyTorch 기반 mini GPT 구현, 단계별 테스트 통과, smoke/light 학습, REPORT 작성
>
> 전체 일정 인덱스: [Week 13-14 구현 계획](./implementation-plan.md)

---

## 1. mini GPT 일정

| 날짜 | 목표 | 세부 작업 | 확인할 테스트/산출물 |
|------|------|-----------|----------------------|
| 05-26(화) | GPT 과제 구조 파악 및 BPE 시작 | `README.md`, `REPORT.md`, `src/`, `tests/` 구조 확인. NSMC 데이터 준비. UTF-8 byte-level BPE 구조 학습 및 구현 시작 | 구현 순서 메모, 데이터 준비 확인 |
| 05-27(수) | BPE 완료 및 Dataset/Embedding 구현 | `BPETokenizer`의 special token, train, encode/decode, save/load 구현. `GPTDataset`, `create_dataloader`, `InputEmbedding` 구현 | `test_bpe.py`, `test_dataset.py` |
| 05-28(목) | Attention 구현 | Q/K/V projection, head split, scaled dot-product attention, causal mask, output projection 구현 | `test_attention.py` |
| 05-29(금) | GPT 모델 구성 요소 구현 | `LayerNorm`, `GELU`, `FeedForward`, `TransformerBlock`, `GPTModel`, `generate_text_simple` 구현 | `test_model.py` |
| 05-30(토) | Train/Checkpoint/Generation 구현 | batch loss, loader loss, checkpoint save/load, temperature/top-k generation, pretraining loop 구현 | `test_train.py` |
| 05-31(일) | 부재 | 작업 계획 없음. 전날 진행 상태와 실패 테스트를 남겨둔다 | 진행 로그 |
| 06-01(월) | Fine-tuning 구현 및 밀린 작업 보완 | NSMC 감성 분류 데이터 생성, `ReviewSentimentDataset`, `GPTForSequenceClassification`, train/evaluate 구현. 05-30까지 미완료된 테스트 보완 | `test_finetune.py`, 미완료 테스트 보완 |
| 06-02(화) | 전체 테스트 및 smoke/light 학습 | 전체 테스트 실행. smoke 설정으로 사전 학습 실행. 가능하면 light 설정 실험. generation sample, train/val loss, fine-tuning 결과 기록 | 전체 `pytest tests/ -v`, 학습 로그, 생성 샘플 |
| 06-03(수) | REPORT 정리 및 제출 준비 | BPE, 데이터, 모델 구조, pretraining, generation, fine-tuning 결과 정리. 실패/보완 지점과 남은 리스크 기록 | GPT `REPORT.md` 초안, 발표/제출 메모 |

---

## 2. mini GPT 교재 학습 계획

mini GPT는 교재 전체를 먼저 완독하고 구현에 들어가는 방식으로 진행하지 않는다. 2026-05-25 밤에 전체 파이프라인을 먼저 훑고, 이후에는 당일 구현할 테스트 범위에 맞춰 필요한 장을 좁고 깊게 학습한다. 단, 매일 마지막에는 오늘 구현한 부품이 전체 GPT 흐름에서 어디에 들어가는지 shape와 함께 기록한다.

교재 기준은 『밑바닥부터 만들면서 배우는 LLM』 목차를 따른다. 교재에서는 `tiktoken`을 사용하는 부분이 있지만, `gpt-lab` 과제에서는 외부 tokenizer를 사용할 수 없으므로 BPE는 `src/bpe.py`와 `tests/test_bpe.py` 기준의 UTF-8 byte-level BPE로 직접 구현한다.

| 날짜 | 교재에서 볼 범위 | 학습 목표 | 구현/기록 연결 |
|------|------------------|-----------|----------------|
| 05-25(월) 밤 | 1.3 LLM의 구축 단계, 1.4 트랜스포머 구조 소개, 1.6 GPT 구조 자세히 살펴보기, 1.7 대규모 언어 모델 만들기. 2.2~2.8은 흐름 중심으로 훑기. 3.3, 3.5, 3.6과 4.1, 4.5, 4.6은 구조 지도 수준으로 확인 | `text -> token id -> input/target -> embedding -> transformer block -> logits -> loss -> generation/fine-tuning` 전체 흐름 파악 | `gpt-lab`의 `README.md`, `src/`, `tests/` 구조와 교재 흐름을 연결해 구현 순서 메모 작성 |
| 05-26(화) | 2.2 텍스트 토큰화하기, 2.3 토큰을 토큰 ID로 변환하기, 2.4 특수 문맥 토큰 추가하기, 2.5 바이트 페어 인코딩 | 텍스트를 token id로 바꾸는 이유, special token, byte-level BPE merge, encode/decode 복원 조건 이해 | `BPETokenizer`의 special token, `train`, `encode`, `decode`, `save/load` 구현. 한글/영문 혼합 문장의 encode/decode 복원 예시 기록 |
| 05-27(수) | 2.6 슬라이딩 윈도로 데이터 샘플링하기, 2.7 토큰 임베딩 만들기, 2.8 단어 위치 인코딩하기 | next-token prediction용 input/target shift, context window, token embedding과 position embedding의 shape 이해 | `GPTDataset`, `create_dataloader`, `InputEmbedding` 구현. `(batch, seq_len)`이 `(batch, seq_len, emb_dim)`으로 바뀌는 흐름 기록 |
| 05-28(목) | 3.3 셀프 어텐션, 3.4 훈련 가능한 가중치를 가진 셀프 어텐션, 3.5 코잘 어텐션, 3.6 멀티 헤드 어텐션 | Q/K/V projection, attention score, softmax 축, causal mask, head split/concat shape 이해 | `MultiHeadAttention` 구현. attention weight shape `(batch, heads, seq_len, seq_len)`와 미래 토큰 mask 동작 기록 |
| 05-29(금) | 4.1 LLM 구조 구현하기, 4.2 층 정규화, 4.3 GELU/FeedForward, 4.4 숏컷 연결, 4.5 트랜스포머 블록, 4.6 GPT 모델, 4.7 텍스트 생성하기 | LayerNorm, GELU, FeedForward, residual connection, TransformerBlock, GPTModel의 입출력 shape 유지 이유 이해 | `LayerNorm`, `GELU`, `FeedForward`, `TransformerBlock`, `GPTModel`, `generate_text_simple` 구현. block 전후 shape와 logits shape 기록 |
| 05-30(토) | 5.1 텍스트 생성 모델 평가하기, 5.2 LLM 훈련하기, 5.3 디코딩 전략, 5.4 파이토치로 모델 로드하고 저장하기 | logits에서 cross entropy loss 계산, train/val loss, temperature, top-k sampling, checkpoint 저장/로드 흐름 이해 | batch loss, loader loss, checkpoint save/load, generation, pretraining loop 구현. smoke 학습 설정과 generation sample 기록 |
| 06-01(월) | 6.1 미세 튜닝 방법, 6.2 데이터셋 준비, 6.3 데이터 로더, 6.5 분류 헤드 추가, 6.6 분류 손실과 정확도, 6.7 지도 학습 데이터로 미세 튜닝 | GPT backbone 위에 classification head를 붙이는 방식, 문장 대표 hidden state 선택, 분류 loss/accuracy 계산 이해 | `ReviewSentimentDataset`, `GPTForSequenceClassification`, train/evaluate 구현. fine-tuning loss 또는 accuracy 기록 |
| 06-02(화) | 5장, 6장 구현 범위 재확인. 선택 실험이 필요하면 부록 D의 warmup, cosine decay, gradient clipping은 참고만 한다 | 전체 테스트와 smoke/light 학습 결과를 기준으로 남은 리스크 파악 | 전체 `pytest tests/ -v`, smoke/light 학습, generation sample, fine-tuning 결과를 REPORT에 정리 |
| 06-03(수) | 1~6장 중 REPORT 설명에 필요한 부분만 재확인 | 구현한 구성 요소를 발표/보고서 언어로 설명할 수 있게 정리 | BPE, Dataset, Attention, GPTModel, Train, Fine-tuning 결과와 실패/보완 지점 정리 |

---

## 3. mini GPT 협업 방식과 하루 운영

mini GPT 기간인 2026-05-26부터 2026-06-03까지는 각자 개인 브랜치에서 당일 구현 범위의 테스트를 먼저 통과시키는 것을 기본으로 한다. 팀 코어타임에는 구현 방식, 테스트 결과, shape 흐름, 남은 실패 원인을 비교하고, 합의한 구현을 main에 병합한다.

| 시간 | 활동 | mini GPT 기준 |
|------|------|---------------|
| 10:00-11:50 | 개인 학습 | 당일 교재 범위와 `tests/test_*.py` 요구사항을 먼저 확인 |
| 13:00-14:00 | 학습 내용 토의 | 개념, 입력/출력 shape, 구현 리스크, 테스트 통과 기준 공유 |
| 14:00-18:00 | 개인 구현 | 개인 브랜치에서 당일 TODO 구현 및 관련 테스트 최소 1회 실행 |
| 19:00-20:00 | 구현 결과 비교 | 코드 구조, edge case, 테스트 결과, 설명 가능 여부 비교 |
| 20:00-21:00 | 병합 후보 결정 및 main 반영 | 먼저 통과한 코드가 아니라 팀이 설명 가능하고 유지보수하기 쉬운 구현을 선택 |
| 21:00-22:00 | 병합 후 테스트/수정/기록 | main에서 당일 테스트 재실행, 실패 보완, REPORT/WIL 메모 작성 |
| 22:00 이후 | 개인 학습 및 복습 | 오늘 구현한 부품이 전체 GPT 흐름에서 어디에 들어가는지 정리 |

`BPE`, `Attention`, `GPTModel`, `Train/Generation`처럼 구현 리스크가 큰 날에는 19:00-20:30까지 개인 구현을 연장할 수 있다. 이 경우에도 22:00 전에는 main 기준 테스트 결과, 병합 여부, 남은 실패 원인을 기록한다.

---

## 4. mini GPT 완료 기준

- BPE, Dataset, Attention, Model, Train, Fine-tuning 테스트를 단계별로 실행한다.
- 전체 `pytest tests/ -v`를 실행한다.
- BPE encode/decode 복원 예시를 기록한다.
- smoke 또는 light 학습 결과를 기록한다.
- generation sample을 남긴다.
- fine-tuning 결과를 loss 또는 accuracy 기준으로 기록한다.
- GPT `REPORT.md`에 구현 현황, 테스트 결과, 데이터, BPE, 모델 구조, 학습/fine-tuning 결과를 포함한다.
