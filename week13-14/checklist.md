# Week 13-14 - 제출물 체크리스트

> 주차별 제출물과 과제 진행 상황을 점검합니다.

---

## CSV 기준 이슈 체크

`local/data/week13-14_issues_complete.csv`의 29개 항목을 빠뜨리지 않기 위한 기준표입니다. 아래 항목은 세부 구현 체크리스트와 중복될 수 있습니다.

### 공통

| 항목 | 완료 | 비고 |
|------|:----:|------|
| 공통 - 핵심 역량 목표 수립 | □ | `goal.md` |
| 공통 - 핵심 역량 목표 달성률 평가 | □ | `goal-achievement.md` |
| 공통 - AI 활용 원칙 수립 | □ | `ai-principles.md` |
| 공통 - 이번 주 할 일(업무) 검토 | □ | 공지, 과제, 발표 일정 확인 |
| 공통 - WIL(What I Learned) 작성 | □ | `wil.md` |
| 공통 - 팀 협업 룰/팁 정리 및 합의 | □ | `team-collaboration.md` |

### 학습

| 항목 | 완료 | 비고 |
|------|:----:|------|
| 학습 - 뉴런 | □ | 입력, 가중치, 편향, 출력 |
| 학습 - 퍼셉트론 | □ | 선형 결합과 임계값 |
| 학습 - 다중 퍼셉트론 | □ | 은닉층, 비선형성 |
| 학습 - 신경망 | □ | forward/loss/backward/update |
| 학습 - 활성화 함수(Sigmoid & Relu) | □ | MNIST layer 구현과 연결 |
| 학습 - 손실 함수(Softmax & 교차엔트로피 오차) | □ | 분류 loss 구현과 연결 |
| 학습 - 오차역전파법 | □ | gradient 흐름 설명 |
| 학습 - 최적화(SGD & Adam) | □ | update 식과 상태값 |
| 학습 - Dropout | □ | train/eval 차이 |
| 학습 - LLM - 트랜스포머 구조 | □ | GPT 전체 구조 |
| 학습 - LLM - GPT 구조 | □ | block, lm_head, loss |
| 학습 - LLM - 토큰나이징 | □ | 텍스트를 token id로 변환 |
| 학습 - LLM - 임베딩 | □ | token/position embedding |
| 학습 - LLM - 바이트 페어 인코딩(BPE) | □ | merge, vocab, encode/decode |
| 학습 - LLM - Self-Attention | □ | Q/K/V와 attention weight |
| 학습 - LLM - 코잘 어텐션(Causal Attention) | □ | 미래 token mask |
| 학습 - LLM - 멀티헤드 어텐션 | □ | head 분리와 concat |
| 학습 - LLM - 활성화 함수(GELU) | □ | FeedForward와 연결 |
| 학습 - LLM - 숏컷 연결 | □ | residual connection |
| 학습 - LLM - 디코딩 전략(온도 스케일링 & 탑-k 샘플링) | □ | generation 결과 기록 |
| 학습 - LLM - 미세조정 | □ | 감성 분류 fine-tuning |

### 과제

| 항목 | 완료 | 비고 |
|------|:----:|------|
| 과제 - 신경망을 이용한 필기체 숫자 인식 | □ | MNIST 구현, 테스트, REPORT |
| 과제 - LLM 구현 | □ | mini GPT 구현, 테스트, REPORT |

---

## 주차 시작 (1주차 금요일, 2026-05-22)

| 항목 | 완료 | 비고 |
|------|:----:|------|
| 핵심 역량 목표 수립 (goal.md) | □ | |
| AI 활용 원칙 수립 (ai-principles.md) | □ | |
| 이번 주 할 일(업무) 검토 | □ | AI로 진화하기, MNIST, mini GPT 공지 확인 |
| 팀 협업 룰/팁 정리 및 합의 (team-collaboration.md) | □ | |
| 팀 GitHub Projects 주소 제출 | □ | 첫날 금요일 자정까지 |
| MNIST 팀 repository 준비 | □ | `mnist-lab` 기반 |
| mini GPT 팀 repository 준비 | □ | `gpt-lab` 기반 |
| Python 3.11 환경 확인 | □ | Colab 또는 로컬 Conda |
| 데이터/checkpoint/token 미커밋 원칙 확인 | □ | `.gitignore`, private token 주의 |
| AI 특강 참석 | □ | 1주차 금요일 |

---

## 1주차 진행 - MNIST 신경망

| 과제/항목 | 완료 | 비고 |
|-----------|:----:|------|
| 과제 템플릿 확인 | □ | `krafton-jungle/mnist-lab` |
| Colab 또는 로컬 실행 확인 | □ | `mnist_lab.ipynb`, Python 3.11 |
| MNIST 데이터 로드 확인 | □ | `data/mnist.npz` |
| `ReLU.forward/backward` 구현 | □ | `pytest tests/test_relu.py -v` |
| `Softmax.forward/backward` 구현 | □ | `pytest tests/test_softmax.py -v` |
| `Affine.forward/backward` 구현 | □ | `pytest tests/test_affine.py -v` |
| `cross_entropy_loss` 구현 | □ | `pytest tests/test_cross_entropy_loss.py -v` |
| `SGD.update` 구현 | □ | `pytest tests/test_sgd.py -v` |
| `Adam.update` 구현 | □ | `pytest tests/test_adam.py -v` |
| `NeuralNetwork` 구현 | □ | `pytest tests/test_neural_network.py -v` |
| `BatchNorm.forward/backward` 구현 | □ | `pytest tests/test_batchnorm.py -v` |
| `Dropout.forward/backward` 구현 | □ | `pytest tests/test_dropout.py -v` |
| `train` 구현 | □ | `pytest tests/test_training.py -v` |
| `evaluate` 확인 | □ | `pytest tests/test_evaluate.py -v` |
| 전체 테스트 실행 | □ | `pytest tests/ -v` |
| Forward/Loss/Backward/Update 흐름 정리 | □ | REPORT 또는 발표 메모 |
| 정확도 실험 | □ | 최소 95%, 권장 97% |
| loss curve 기록 | □ | `plot_loss_history` |
| 모델 구조/학습 설정 기록 | □ | hidden layers, optimizer, lr, epochs, batch size |
| 신경망 과제 발표 준비 | □ | 2026-05-28 오전 10시 |
| MNIST REPORT.md 작성 | □ | 제출용 보고서 |

---

## 2주차 진행 - mini GPT LLM

| 과제/항목 | 완료 | 비고 |
|-----------|:----:|------|
| 과제 템플릿 확인 | □ | `krafton-jungle/gpt-lab` |
| Colab GPU 환경 확인 | □ | 런타임 유형 Python 3, GPU |
| NSMC 데이터 준비 | □ | `python download_data.py` |
| 데이터/checkpoint/token 미커밋 확인 | □ | data, checkpoint, PAT 제외 |
| BPE tokenizer 구현 | □ | `pytest tests/test_bpe.py -v` |
| 한글 encode/decode 복원 확인 | □ | UTF-8 byte-level BPE |
| GPTDataset 구현 | □ | input/target shift |
| InputEmbedding 구현 | □ | token + position embedding |
| Dataset/Embedding 테스트 | □ | `pytest tests/test_dataset.py -v` |
| MultiHeadAttention 구현 | □ | causal mask, multi-head shape |
| Attention 테스트 | □ | `pytest tests/test_attention.py -v` |
| LayerNorm/GELU/FeedForward 구현 | □ | `src/model.py` |
| TransformerBlock/GPTModel 구현 | □ | residual connection, lm_head |
| Model 테스트 | □ | `pytest tests/test_model.py -v` |
| 사전 학습 유틸리티 구현 | □ | loss, checkpoint, generation, train loop |
| Train 테스트 | □ | `pytest tests/test_train.py -v` |
| Smoke 설정 실행 | □ | `corpus[:5000]`, vocab 300, context 32 |
| Light 또는 Basic 설정 실행 | □ | 시간/자원에 맞춰 선택 |
| checkpoint 저장/로드 확인 | □ | model/optimizer/epoch/global_step |
| generation 샘플 기록 | □ | temperature/top-k 여부 기록 |
| 감성 분류 fine-tuning 구현 | □ | `src/finetune.py` |
| Finetune 테스트 | □ | `pytest tests/test_finetune.py -v` |
| 전체 테스트 실행 | □ | `pytest tests/ -v` |
| GPT REPORT.md 작성 | □ | 구현 현황, 테스트, 데이터, BPE, 모델, 학습, fine-tuning |
| GPT LLM 발표 준비 | □ | 2026-06-04 오전 10시 |

---

## 회귀 / 실험 기록 체크

| 구분 | 항목 | 완료 | 비고 |
|------|------|:----:|------|
| MNIST | 전체 pytest | □ | `pytest tests/ -v` |
| MNIST | test accuracy | □ | 최소 95%, 권장 97% |
| MNIST | REPORT.md | □ | 구조, 설정, 결과, 회고 |
| GPT | 전체 pytest | □ | `pytest tests/ -v` |
| GPT | BPE vocabulary 저장/로드 | □ | 경로와 corpus 크기 기록 |
| GPT | pretraining loss 기록 | □ | train/val loss |
| GPT | generation sample | □ | 샘플 문장 |
| GPT | fine-tuning 평가 | □ | validation/test accuracy 또는 loss |
| GPT | REPORT.md | □ | 필수 섹션 포함 |

---

## 주차 종료 (2주차 목요일, 2026-06-04)

| 항목 | 완료 | 비고 |
|------|:----:|------|
| GPT LLM 과제 발표 | □ | 목요일 오전 10시 |
| 팀 GitHub repository 주소 제출 | □ | 과제1, 과제2 repository |
| WIL 작성 (wil.md) | □ | |
| WIL 블로그 포럼 등록 | □ | WEEK13-14 태그 |
| 핵심 역량 목표 달성률 평가 (goal-achievement.md) | □ | |
| 13-14주차 동료피드백 | □ | 차주 발제 종료 전 |
| 나만무 리더 지원 및 추천 | □ | 2주차 목요일 오후 2시 |
| 운영진 티타임 참석 | □ | 2주차 목요일 오후 3시 |
| 나만무 리더 선정 및 발표 확인 | □ | 2주차 목요일 오후 6시 |
