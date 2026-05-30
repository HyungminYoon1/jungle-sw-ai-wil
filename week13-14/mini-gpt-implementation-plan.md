# Week 13-14 mini GPT 구현 계획

> 기간: 2026-05-26(화) ~ 2026-06-03(수)
>
> 목표: PyTorch 기반 mini GPT 구현, 단계별 테스트 통과, smoke/light 학습, REPORT 작성
>
> 전체 일정 인덱스: [Week 13-14 구현 계획](./implementation-plan.md)

---

## 1. mini GPT 일정

| 날짜 | 목표 | 학습/작업 범위 | 구현 파일/함수 | 확인할 테스트/산출물 |
|------|------|----------------|----------------|----------------------|
| 05-26(화) | GPT 과제 구조 파악 및 BPE 핵심 구현 시작 | `README.md`, `REPORT.md`, `src/`, `tests/` 구조 확인. NSMC 데이터 준비. UTF-8 byte-level BPE 구조 학습 | `src/bpe.py`: `_init_special_tokens`, `encode`, `decode`, `save`, `load`, `train` 초안. 필요 helper: `_replace_pair`, token 직렬화/역직렬화, merge token byte 복원 | 구현 순서 메모, 데이터 준비 확인. 가능하면 `pytest tests/test_bpe.py -v -k "TestSpecialTokens or TestBPETokenizer"` |
| 05-27(수) | BPE 완료 및 Dataset/Embedding 구현 | BPE merge 학습 보완. input/target shift, context window, token/position embedding 학습 | `src/bpe.py`: `train` 보완 및 전체 BPE 테스트 통과. `src/dataset.py`: `GPTDataset.__init__`, `__len__`, `__getitem__`, `create_dataloader`. `src/embeddings.py`: `InputEmbedding.__init__`, `forward` | `pytest tests/test_bpe.py -v`, `pytest tests/test_dataset.py -v` |
| 05-28(목) | Attention 구현 | Q/K/V projection, head split, scaled dot-product attention, causal mask, output projection 학습 | `src/attention.py`: `MultiHeadAttention.__init__`, `forward` | `pytest tests/test_attention.py -v` |
| 05-29(금) | GPT 모델 구성 요소 구현 | LayerNorm, GELU, FeedForward, residual connection, TransformerBlock, GPTModel, greedy generation 학습 | `src/model.py`: `LayerNorm.forward`, `GELU.forward`, `FeedForward.__init__/forward`, `TransformerBlock.__init__/forward`, `GPTModel.__init__/forward`, `generate_text_simple` | `pytest tests/test_model.py -v` |
| 05-30(토) | Train/Checkpoint/Generation 개인 구현 | 팀원별로 `train.py` 미완료 함수를 각자 구현. batch/loader loss, checkpoint, generation, pretraining loop 학습 | `src/train.py`: `calc_loss_batch`, `calc_loss_loader`, `save_checkpoint`, `load_checkpoint`, `generate`, `generate_and_print_sample`, `evaluate_model`, `train_model` | 개인 브랜치에서 `pytest tests/test_train.py -v`, 남은 실패 테스트와 구현 쟁점 기록 |
| 05-31(일) | 부재일 개인 선택 학습/구현 정리 | 공통 작업 계획은 없음. 개인별로 가능하면 `train.py` 구현을 마무리하고 월요일 리뷰를 위해 코드 구조, 테스트 결과, 설명 포인트 정리 | 개인 선택: `src/train.py` 최종 점검, 교재 5장 구조와 다른 부분, edge case, checkpoint/generation 흐름 메모 | 개인 브랜치 `pytest tests/test_train.py -v` 결과, 리뷰용 메모, PR 또는 비교 대상 branch 준비 |
| 06-01(월) | Train 코드 리뷰/머지 및 Fine-tuning 1차 구현 | 오전에는 각자 구현한 `train.py`를 리뷰하고 main 병합. 오후부터 Chapter 6.1~6.7 전체를 학습한 뒤 `finetune.py` 전체 TODO 1차 구현과 테스트 통과를 시도 | 오전: `src/train.py` 병합. 오후/야간: `src/finetune.py`의 `make_sentiment_dataset`, `ReviewSentimentDataset.__getitem__`, `GPTForSequenceClassification.__init__/forward`, `train_epoch_sentiment`, `evaluate_sentiment` 1차 구현 | main 기준 `pytest tests/test_train.py -v`, 가능하면 `pytest tests/test_finetune.py -v` 통과. 미해결 실패는 원인과 06-02 오전 보완 계획 기록 |
| 06-02(화) | Fine-tuning 오전 마감 및 실험 착수 | 오전에는 `finetune.py` 보완, 리뷰, main 병합, 테스트를 마감한다. 오후에는 팀 논의로 실험 방식, 공통 baseline, 기록 양식, 실행 환경을 확정하고 실제 실험을 시작한다 | 오전: `src/finetune.py` 미해결 TODO/실패 테스트 보완, main 병합, `pytest tests/test_finetune.py -v`, 가능하면 전체 `pytest tests/ -v`. 오후/야간: 실험 방식 합의, 데이터/BPE cache/checkpoint/log 경로 점검, Smoke 또는 Basic baseline 실행 | fine-tuning 병합 결과, 전체 테스트 결과, 실험 운영 방식, 실행 환경 점검 결과, Smoke/Basic baseline 로그, generation sample |
| 06-03(수) | 실험 결과 검증 및 REPORT/제출 준비 | 화요일 실험 결과를 바탕으로 유망 설정을 추가 검증하고 최종 후보 모델을 정한다. BPE, 데이터, 모델 구조, pretraining, generation, fine-tuning 결과를 정리한다 | `REPORT.md`, 발표/제출 메모. 필요 시 `gpt-lab.ipynb` 실행 순서 확인. 추가 실험은 learning rate/dropout/model size 중 한 변수씩만 비교 | GPT `REPORT.md` 초안, Smoke/Basic 실험 로그, 최종 후보 모델 로그, 발표/제출 메모 |

---

## 2. mini GPT 교재 학습 계획

mini GPT는 교재 전체를 먼저 완독하고 구현에 들어가는 방식으로 진행하지 않는다. 2026-05-26부터 당일 구현할 테스트 범위에 맞춰 필요한 장을 좁고 깊게 학습한다. 단, 첫날인 2026-05-26에는 BPE 구현 전에 전체 파이프라인을 먼저 훑고, 매일 마지막에는 오늘 구현한 부품이 전체 GPT 흐름에서 어디에 들어가는지 shape와 함께 기록한다.

교재 기준은 『밑바닥부터 만들면서 배우는 LLM』 목차를 따른다. 교재에서는 `tiktoken`을 사용하는 부분이 있지만, `gpt-lab` 과제에서는 외부 tokenizer를 사용할 수 없으므로 BPE는 `src/bpe.py`와 `tests/test_bpe.py` 기준의 UTF-8 byte-level BPE로 직접 구현한다.

| 날짜 | 교재에서 볼 범위 | 학습 목표 | 구현/기록 연결 |
|------|------------------|-----------|----------------|
| 05-26(화) | 1.3 LLM의 구축 단계, 1.4 트랜스포머 구조 소개, 1.6 GPT 구조 자세히 살펴보기, 1.7 대규모 언어 모델 만들기. 이후 2.2 텍스트 토큰화하기, 2.3 토큰을 토큰 ID로 변환하기, 2.4 특수 문맥 토큰 추가하기, 2.5 바이트 페어 인코딩 | `text -> token id -> input/target -> embedding -> transformer block -> logits -> loss -> generation/fine-tuning` 전체 흐름을 먼저 잡고, 텍스트를 token id로 바꾸는 이유, special token, byte-level BPE merge, encode/decode 복원 조건 이해 | `gpt-lab`의 `README.md`, `src/`, `tests/` 구조와 교재 흐름을 연결해 구현 순서 메모 작성. `BPETokenizer`의 special token, `train`, `encode`, `decode`, `save/load` 구현. 한글/영문 혼합 문장의 encode/decode 복원 예시 기록 |
| 05-27(수) | 2.6 슬라이딩 윈도로 데이터 샘플링하기, 2.7 토큰 임베딩 만들기, 2.8 단어 위치 인코딩하기 | next-token prediction용 input/target shift, context window, token embedding과 position embedding의 shape 이해 | `GPTDataset`, `create_dataloader`, `InputEmbedding` 구현. `(batch, seq_len)`이 `(batch, seq_len, emb_dim)`으로 바뀌는 흐름 기록 |
| 05-28(목) | 3.3 셀프 어텐션, 3.4 훈련 가능한 가중치를 가진 셀프 어텐션, 3.5 코잘 어텐션, 3.6 멀티 헤드 어텐션 | Q/K/V projection, attention score, softmax 축, causal mask, head split/concat shape 이해 | `MultiHeadAttention` 구현. attention weight shape `(batch, heads, seq_len, seq_len)`와 미래 토큰 mask 동작 기록 |
| 05-29(금) | 4.1 LLM 구조 구현하기, 4.2 층 정규화, 4.3 GELU/FeedForward, 4.4 숏컷 연결, 4.5 트랜스포머 블록, 4.6 GPT 모델, 4.7 텍스트 생성하기 | LayerNorm, GELU, FeedForward, residual connection, TransformerBlock, GPTModel의 입출력 shape 유지 이유 이해 | `LayerNorm`, `GELU`, `FeedForward`, `TransformerBlock`, `GPTModel`, `generate_text_simple` 구현. block 전후 shape와 logits shape 기록 |
| 05-30(토) | 5.1 텍스트 생성 모델 평가하기, 5.2 LLM 훈련하기, 5.3 디코딩 전략, 5.4 파이토치로 모델 로드하고 저장하기 | logits에서 cross entropy loss 계산, train/val loss, temperature, top-k sampling, checkpoint 저장/로드 흐름 이해 | batch loss, loader loss, checkpoint save/load, generation, pretraining loop 개인 구현. 실패 테스트와 구현 쟁점 기록 |
| 05-31(일) | 부재일이므로 공통 학습 계획 없음. 개인 선택으로 5장 구현 범위 재확인 가능. 특히 5.2 `train_model_simple`, 5.3 top-k/temperature, 5.4 checkpoint 흐름 복습 | 개인별로 작성한 `train.py`를 교재 구조와 비교하고, 월요일 리뷰에서 설명할 수 있게 정리 | 개인 선택: `src/train.py` 구현 마무리, `evaluate_model`, generation, checkpoint/resume, train/eval 모드 전환 설명 메모 |
| 06-01(월) | 오전에는 5장 구현 리뷰. 오후에는 6.1 미세 튜닝 방법, 6.2 데이터셋 준비, 6.3 데이터 로더, 6.5 분류 헤드 추가, 6.6 분류 손실과 정확도, 6.7 지도 학습 데이터로 미세 튜닝까지 학습 | 오전: `train.py` 구조와 edge case 합의. 오후/야간: GPT backbone 위에 classification head를 붙이는 방식, 문장 대표 hidden state 선택, 분류 loss/accuracy, train/evaluate loop 이해 | 오전 main 기준 `train.py` 병합. 오후/야간 `ReviewSentimentDataset`, `GPTForSequenceClassification`, `train_epoch_sentiment`, `evaluate_sentiment` 1차 구현 |
| 06-02(화) | 오전에는 6.1~6.7을 구현 관점에서 재확인하며 fine-tuning 테스트 리스크를 마감. 오후에는 5장 학습 루프와 generation 결과 기록 방식, Smoke/Basic baseline 설정 재확인 | 오전: fine-tuning train/evaluate 루프와 테스트 실패 원인 보완. 오후: 팀 논의 후 실험 운영 방식, baseline, 비교 변수, 실행 환경, 기록 양식 확정 | 오전 `test_finetune.py` 통과와 전체 테스트 실행. 오후부터 합의한 방식으로 Smoke 또는 Basic baseline 실행, generation sample, fine-tuning 결과 기록 |
| 06-03(수) | 1~6장 중 REPORT 설명에 필요한 부분만 재확인. 선택 실험이 필요하면 부록 D의 warmup, cosine decay, gradient clipping은 참고만 한다 | 구현한 구성 요소를 발표/보고서 언어로 설명하고, 실험 결과, 최종 후보 선정 이유, 남은 리스크를 정리 | BPE, Dataset, Attention, GPTModel, Train, Fine-tuning 결과와 Smoke/Basic 실험 비교, 최종 후보 모델, 실패/보완 지점 정리 |

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
| 22:00-23:00 | 개인 학습 및 복습 | 오늘 구현한 부품이 전체 GPT 흐름에서 어디에 들어가는지 정리 |

`BPE`, `Attention`, `GPTModel`, `Train/Generation`처럼 구현 리스크가 큰 날에는 19:00-21:00까지 개인 구현을 연장할 수 있다. 이 경우에도 23:00 전에는 main 기준 테스트 결과, 병합 여부, 남은 실패 원인을 기록한다.

06-01부터 06-03까지는 구현 일정이 마감 단계에 들어가므로 위의 일일 공통 루틴을 그대로 적용하지 않고, 아래 날짜별 세부 시간표를 우선 적용한다.

### 3.1 06-01(월) 세부 일정: Train 리뷰/머지와 Fine-tuning 1차 구현

| 시간 | 활동 | 산출물 |
|------|------|--------|
| 10:00-10:50 | 주말에 각자 완료한 `train.py` 코드 리뷰. loss 계산, checkpoint, generation, `train_model` 구조를 함수별로 비교 | 병합 후보 구현 결정, 수정 필요 목록 |
| 10:50-11:20 | main 병합 전 최종 수정 및 충돌 해결 | 병합 가능한 `train.py` |
| 11:20-11:50 | main 기준 `pytest tests/test_train.py -v` 재실행, 실패 시 즉시 보완 | main 기준 train 테스트 결과 |
| 13:00-15:00 | Chapter 6.1~6.7 핵심 학습. fine-tuning 목적, 데이터셋 구조, classification head, 마지막 token hidden state, loss/accuracy, train/evaluate loop 흐름 정리 | Chapter 6 핵심 메모, 구현 전 확인 질문 |
| 15:00-16:00 | `tests/test_finetune.py` 요구사항 분석. 필요한 함수, 입력/출력 shape, 테스트 통과 기준 확인 | `finetune.py` 구현 순서와 shape 메모 |
| 16:00-18:00 | `finetune.py` 구현 착수. dataset 생성, review dataset, classifier wrapper, forward 초안 작성 | `make_sentiment_dataset`, `ReviewSentimentDataset`, `GPTForSequenceClassification` 초안 |
| 19:00-21:30 | `finetune.py` 전체 TODO 1차 구현 완료 목표. `train_epoch_sentiment`, `evaluate_sentiment`까지 작성하고 팀원별 구현 방식 비교 | `train_epoch_sentiment`, `evaluate_sentiment` 포함 1차 구현 |
| 21:30-23:00 | `pytest tests/test_finetune.py -v` 실행 및 실패 보완. 통과하지 못하면 실패 원인과 다음날 보완 항목 기록 | `test_finetune.py` 통과 또는 미해결 실패 목록, train 병합 결과, fine-tuning 진행 로그 |

### 3.2 06-02(화) 세부 일정: Fine-tuning 오전 마감과 실험 착수

| 시간 | 활동 | 산출물 |
|------|------|--------|
| 10:00-11:50 | 월요일 미해결 `finetune.py` 실패 보완, 코드 리뷰, main 병합, `test_finetune.py` 및 가능하면 전체 테스트 실행. fine-tuning 구현은 오전 안에 마감한다 | `pytest tests/test_finetune.py -v` 통과, main 병합 결과, 가능하면 `pytest tests/ -v` 결과 |
| 13:00-14:00 | 실험 운영 방식 논의. 공동 목표 기반 병렬 실험, 개별 목표 기반 독립 실험, 혼합 방식 중 선택하고 baseline, 비교 변수, 기록 양식, BPE cache 사용 방식 확정 | 실험 운영 방식, 공통 baseline, 실험 배정 원칙, 로그 기록 양식 |
| 14:00-15:00 | 실행 환경 점검. 개인 PC/Colab에서 데이터, BPE vocabulary/cache, checkpoint/log 경로, GPU/CPU 환경 확인 | 팀원별 실행 환경 점검 결과, 실행 가능한 실험 명령 |
| 15:00-18:00 | 1차 실험 실행. 합의한 운영 방식에 따라 Smoke로 빠르게 sanity check를 하거나 Basic baseline부터 실행 | Smoke 또는 Basic baseline 로그, generation sample, 실행 시간 |
| 19:00-21:30 | 2차 실험 실행. 1차 결과를 보고 learning rate, dropout, 모델 크기 중 한 변수만 바꿔 비교하거나 유망 설정을 재실행 | comparison 로그 추가, 유망 설정 후보 |
| 21:30-23:00 | 실험 로그 수합, REPORT 반영, 다음날 검증할 최종 후보 설정 결정 | 실험 결과 표 초안, 최종 후보 1~2개, 다음날 실행 조건 |

### 3.3 06-03(수) 세부 일정: 실험 검증과 제출 정리

| 시간 | 활동 | 산출물 |
|------|------|--------|
| 10:00-11:50 | 화요일 실험 결과 리뷰. 유망 설정 1~2개를 추가 검증하거나 seed만 바꿔 재실행 | 최종 후보 검증 로그, loss/generation sample |
| 13:00-15:00 | 최종 후보 모델 학습 결과 확정. 필요하면 짧은 추가 실험 1회 실행 | 최종 후보 모델, checkpoint 경로, 최종 generation sample |
| 15:00-16:30 | 실험 결과 표와 그래프 정리. `gpt-lab.ipynb` 실행 순서와 제출 파일 확인 | 실험 비교 표, 노트북 실행 순서 메모, 제출 체크리스트 |
| 16:30-18:00 | 팀 코드 최종 리뷰, 전체 테스트 재확인 | 최종 `pytest tests/ -v` 결과 또는 남은 리스크 |
| 19:00-21:00 | 발표/제출 메모 작성, 최종 모델 선택 이유와 남은 실패/보완 지점 정리 | 발표 메모, 최종 모델 선택 근거, 리스크 목록 |
| 21:00-23:00 | 최종 문서 정리 및 제출 준비 | 최종 `REPORT.md`, 제출 메모 |

---

## 4. Smoke/Basic 학습 실행 기준

06-02 오전까지 fine-tuning 구현과 테스트를 마감하고, 06-02 오후부터 06-03까지는 실험 방식 합의, 실행 환경 세팅, 실제 실험, 결과 기록에 집중한다. Smoke는 학습 루프가 끝까지 도는지 빠르게 확인하는 안전장치로 사용한다. Basic은 REPORT에 사용할 핵심 실험 후보이지만, 전체 테스트와 Smoke가 안정적으로 끝난 뒤 실행한다. Light는 Basic 전 빠른 sanity check가 필요할 때만 선택적으로 사용한다.

| 구분 | 목적 | 권장 설정 | 기록할 내용 |
|------|------|-----------|-------------|
| Smoke | BPE와 한 배치 학습이 끝까지 도는지 빠르게 확인 | `corpus[:5000]`, `vocab_size=300`, `context_length=32` | train/val loss 1회 이상, generation sample 1개, 실행 시간, 실패 여부 |
| Light | Basic 전 빠른 sanity check가 필요할 때만 선택적으로 사용 | `corpus[:500000]`, `vocab_size=2000`, `context_length=64` | train/val loss 변화, generation sample, 실행 시간 |
| Basic | REPORT에 사용할 핵심 실험 후보. 운영 방식은 화요일 오후 팀 논의로 확정하고, 시간이 허용되면 baseline부터 실행 | `corpus[:1500000]`, `vocab_size=3000`, `context_length=128`, 3 epoch | train/val loss 변화, generation sample, checkpoint 경로, 실행 시간, 변경한 하이퍼파라미터 |

모델/학습 하이퍼파라미터는 과제 공지의 후보 안에서 선택하고 REPORT에 기록한다. 후보는 `batch_size` 2/4/8/16, `drop_rate` 0.0/0.1/0.2, `learning_rate` 1e-4/3e-4/5e-4, `n_layers` 1/2/4, `emb_dim` 64/128/192를 기준으로 한다. Basic 비교 실험은 한 번에 여러 값을 바꾸지 않고, baseline에서 한 변수만 바꿔 결과를 해석할 수 있게 한다.

권장 실험은 다음 기준으로 진행한다.

1. 전체 테스트와 Smoke 실행 결과를 먼저 확보한다.
2. Basic baseline은 시간이 허용되면 1회 이상 실행한다.
3. learning rate, dropout, 모델 크기 계열을 공동 목표로 나눌지, 각자 개별 목표로 탐색할지 화요일 오후에 결정한다.
4. 각 비교 실험은 baseline에서 한 변수만 변경한다.
5. 화요일 밤에는 loss, generation sample, 실행 시간을 수합해 수요일 최종 후보를 정한다.

REPORT에는 사용한 설정, 데이터 크기, 실행 환경, loss/generation/fine-tuning 결과를 그대로 기록한다. 시간이 부족하면 완료한 실험 수와 미실행 사유를 남은 리스크로 기록한다.

---

## 5. mini GPT 완료 기준

- BPE, Dataset, Attention, Model, Train, Fine-tuning 테스트를 단계별로 실행한다.
- 전체 `pytest tests/ -v`를 실행한다.
- BPE encode/decode 복원 예시를 기록한다.
- smoke 결과를 기록하고, 시간이 허용되면 Basic 학습 결과를 기록한다.
- generation sample을 남긴다.
- fine-tuning 결과를 loss 또는 accuracy 기준으로 기록한다.
- GPT `REPORT.md`에 구현 현황, 테스트 결과, 데이터, BPE, 모델 구조, 학습/fine-tuning 결과를 포함한다.
