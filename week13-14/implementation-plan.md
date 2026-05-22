# Week 13-14 구현 계획

> 기준 일정: 2026-05-22(금) ~ 2026-06-03(수)
>
> 부재일: 2026-05-24(일), 2026-05-31(일)
>
> 목표: 2026-05-25까지 MNIST 과제 구현과 REPORT를 완료하고, 2026-05-26부터 2026-06-03까지 mini GPT 구현과 실험 기록을 진행한다.
>
> 참고: 2026-06-04는 GPT LLM 발표 및 최종 제출일이므로, 구현과 REPORT 초안은 2026-06-03까지 마무리하는 것을 기준으로 한다.

---

## 1. 전체 일정

| 기간 | 과제 | 목표 |
|------|------|------|
| 2026-05-22 ~ 2026-05-25 | MNIST 신경망 | NumPy 기반 신경망 구현, 테스트 통과, 정확도 실험, REPORT 작성 |
| 2026-05-24 | 부재 | 작업 계획 없음 |
| 2026-05-26 ~ 2026-06-03 | mini GPT LLM | BPE, Dataset, Attention, GPTModel, Train, Fine-tuning 구현 및 실험 기록 |
| 2026-05-31 | 부재 | 작업 계획 없음 |
| 2026-06-04 | 발표/최종 제출 | GPT LLM 발표, WIL 및 과제 repository 주소 제출 |

MNIST 기간은 짧으므로 2026-05-22에는 교재 1장부터 4.2장까지 학습한 내용을 기준으로 과제 구조와 핵심 개념을 정리한다. 2026-05-23에는 4장 나머지, 5장, 6장의 핵심을 학습하면서 기본 layer 구현을 병행한다.

---

## 2. MNIST 구현 계획

| 날짜 | 목표 | 세부 작업 | 확인할 테스트/산출물 |
|------|------|-----------|----------------------|
| 05-22(금) | 과제 구조 파악 및 교재 1~4.2장 학습 | `README.md`, `REPORT.md`, `src/`, `tests/` 구조 확인. 교재 1장~4.2장 기준으로 shape와 행렬곱, 뉴런/퍼셉트론, `Affine -> ReLU -> Softmax`, loss 개념 정리 | 구현 순서 메모, 4.2장까지 학습 기록, 막힌 개념 정리 |
| 05-23(토) | 4장 나머지/5~6장 학습 + 기본 구현 병행 | 교재 4장 나머지, 5장 오차역전파법, 6장 optimizer/BatchNorm/Dropout 핵심 학습. `ReLU`, `Softmax`, `Affine`, `cross_entropy_loss` 우선 구현. 시간이 남으면 `SGD`, `Adam`까지 구현 | `test_relu.py`, `test_softmax.py`, `test_affine.py`, `test_cross_entropy_loss.py`, 가능하면 `test_sgd.py`, `test_adam.py` |
| 05-24(일) | 부재 | 작업 계획 없음. 전날 진행 상태와 다음 시작 위치를 남겨둔다 | 진행 로그 |
| 05-25(월) | 남은 기본 구현 보완 + 네트워크/학습 구현 및 REPORT 작성 | 05-23에 미완료된 `SGD`, `Adam`을 먼저 보완. 이후 `NeuralNetwork`, `BatchNorm`, `Dropout`, `train`, `evaluate` 구현. 전체 테스트 실행, 정확도 실험, loss curve와 학습 설정 기록, REPORT 작성 | 미완료 기본 테스트, `test_neural_network.py`, `test_batchnorm.py`, `test_dropout.py`, `test_training.py`, `test_evaluate.py`, 전체 `pytest tests/ -v`, MNIST `REPORT.md` |

### MNIST 완료 기준

- 관련 테스트를 단계별로 통과한다.
- 전체 `pytest tests/ -v`를 실행한다.
- test accuracy 최소 95%, 가능하면 97% 이상을 목표로 한다.
- `REPORT.md`에 모델 구조, 학습 설정, 실험 환경, 정확도, loss curve, 회고를 기록한다.
- `Forward -> Loss -> Backward -> Update` 흐름을 설명할 수 있어야 한다.

---

## 3. mini GPT 구현 계획

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

### mini GPT 완료 기준

- BPE, Dataset, Attention, Model, Train, Fine-tuning 테스트를 단계별로 실행한다.
- 전체 `pytest tests/ -v`를 실행한다.
- BPE encode/decode 복원 예시를 기록한다.
- smoke 또는 light 학습 결과를 기록한다.
- generation sample을 남긴다.
- fine-tuning 결과를 loss 또는 accuracy 기준으로 기록한다.
- GPT `REPORT.md`에 구현 현황, 테스트 결과, 데이터, BPE, 모델 구조, 학습/fine-tuning 결과를 포함한다.

---

## 4. 매일 공통 루틴

1. 오늘 구현할 파일과 테스트를 먼저 정한다.
2. 관련 개념을 30~60분 안에 압축 학습한다.
3. TODO 함수 하나를 구현한다.
4. 해당 테스트 파일만 먼저 실행한다.
5. 통과하면 다음 구현 대상으로 넘어간다.
6. 하루 끝에 통과한 테스트, 실패한 테스트, 막힌 개념, 다음 시작 위치를 기록한다.

처음부터 전체 테스트만 실행하지 않는다.
현재 구현 대상 테스트를 먼저 통과시키고, 단계가 끝날 때 전체 테스트를 실행한다.

---

## 5. 부재 전후 원칙

| 시점 | 원칙 |
|------|------|
| 부재 전날 | 통과한 테스트, 실패한 테스트, 다음 시작 파일, 의심 함수, 참고할 개념을 기록 |
| 부재 당일 | 작업 계획 없음 |
| 복귀일 | 전체 맥락을 다시 잡기보다 전날 기록한 다음 시작 파일부터 진행 |

부재일 때문에 일정이 압축되어 있으므로, 리포트와 실험 기록은 마지막 날에 몰아서 쓰지 않는다.
테스트 통과나 실험 결과가 생길 때마다 바로 기록한다.

---

## 6. 우선순위

1. 필수 TODO 구현
2. 단계별 테스트 통과
3. 전체 테스트 통과
4. REPORT에 구현 현황과 실험 결과 기록
5. 정확도와 학습 품질 개선
6. 발표/WIL 정리

마감이 가까울수록 새로운 실험을 늘리기보다, 통과한 테스트와 직접 확인한 결과를 정확히 정리하는 것을 우선한다.
