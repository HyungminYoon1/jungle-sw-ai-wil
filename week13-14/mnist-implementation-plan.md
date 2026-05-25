# Week 13-14 MNIST 구현 계획

> 기간: 2026-05-22(금) ~ 2026-05-25(월)
>
> 목표: NumPy 기반 MNIST 신경망 구현, 테스트 통과, 정확도 실험, REPORT 작성
>
> 전체 일정 인덱스: [Week 13-14 구현 계획](./implementation-plan.md)

---

## 1. MNIST 일정

MNIST 기간은 짧으므로 2026-05-22에는 교재 1장부터 4.2장까지 학습한 내용을 기준으로 과제 구조와 핵심 개념을 정리한다. 2026-05-23에는 4장 나머지, 5장, 6장의 핵심을 학습하면서 기본 layer 구현을 병행한다.

| 날짜 | 목표 | 세부 작업 | 확인할 테스트/산출물 |
|------|------|-----------|----------------------|
| 05-22(금) | 과제 구조 파악 및 교재 1~4.2장 학습 | `README.md`, `REPORT.md`, `src/`, `tests/` 구조 확인. 교재 1장~4.2장 기준으로 shape와 행렬곱, 뉴런/퍼셉트론, `Affine -> ReLU -> Softmax`, loss 개념 정리 | 구현 순서 메모, 4.2장까지 학습 기록, 막힌 개념 정리 |
| 05-23(토) | 4장 나머지/5~6장 학습 + 기본 구현 병행 | 교재 4장 나머지, 5장 오차역전파법, 6장 optimizer/BatchNorm/Dropout 핵심 학습. `ReLU`, `Softmax`, `Affine`, `cross_entropy_loss` 우선 구현. 시간이 남으면 `SGD`, `Adam`까지 구현 | `test_relu.py`, `test_softmax.py`, `test_affine.py`, `test_cross_entropy_loss.py`, 가능하면 `test_sgd.py`, `test_adam.py` |
| 05-24(일) | 부재 | 작업 계획 없음. 전날 진행 상태와 다음 시작 위치를 남겨둔다 | 진행 로그 |
| 05-25(월) | 남은 기본 구현 보완 + 네트워크/학습 구현 및 REPORT 작성 | 05-23에 미완료된 `SGD`, `Adam`을 먼저 보완. 이후 `NeuralNetwork`, `BatchNorm`, `Dropout`, `train`, `evaluate` 구현. 전체 테스트 실행, 정확도 실험, loss curve와 학습 설정 기록, REPORT 작성 | 미완료 기본 테스트, `test_neural_network.py`, `test_batchnorm.py`, `test_dropout.py`, `test_training.py`, `test_evaluate.py`, 전체 `pytest tests/ -v`, MNIST `REPORT.md` |

---

## 2. MNIST 구현 모듈 순서

구현 코드는 교재를 참고할 수 있으므로, 실제 일정은 테스트 실행과 실패 원인 추적 시간을 넉넉히 확보하는 방향으로 잡는다. 의존성이 낮은 기본 연산 모듈을 먼저 통과시키고, 이후 optimizer, 네트워크 구성, 정규화/드롭아웃, 학습 루프 순서로 진행한다.

| 순서 | 구현 모듈 | 핵심 확인 내용 | 확인 테스트 | 배정 |
|------|-----------|----------------|-------------|------|
| 1 | `ReLU.forward/backward` | 양수 통과, 음수 차단, backward mask 처리 | `pytest tests/test_relu.py -v` | 토요일 |
| 2 | `Softmax.forward/backward` | 확률 분포 변환, batch 단위 안정적 계산 | `pytest tests/test_softmax.py -v` | 토요일 |
| 3 | `Affine.forward/backward` | 행렬곱, bias 더하기, `dW/db/dx` shape 확인 | `pytest tests/test_affine.py -v` | 토요일 |
| 4 | `cross_entropy_loss` | 정답 label 기준 loss 계산, numerical stability 확인 | `pytest tests/test_cross_entropy_loss.py -v` | 토요일 |
| 5 | `SGD.update` | learning rate 기반 parameter update | `pytest tests/test_sgd.py -v` | 월요일 |
| 6 | `Adam.update` | momentum, variance, bias correction, optimizer state 확인 | `pytest tests/test_adam.py -v` | 월요일 |
| 7 | `NeuralNetwork` | layer 순서 구성, forward/loss/backward/update 연결 | `pytest tests/test_neural_network.py -v` | 월요일 |
| 8 | `BatchNorm.forward/backward` | train/eval 분기, running mean/var, gradient 계산 | `pytest tests/test_batchnorm.py -v` | 월요일 |
| 9 | `Dropout.forward/backward` | train/eval 동작 분리, mask 적용 | `pytest tests/test_dropout.py -v` | 월요일 |
| 10 | `train` | epoch/batch loop, loss 기록, optimizer update | `pytest tests/test_training.py -v` | 월요일 |
| 11 | `evaluate` | 추론 모드 정확도 계산 | `pytest tests/test_evaluate.py -v` | 월요일 |
| 12 | 전체 테스트/실험/REPORT | 전체 회귀 확인, 정확도/loss curve 기록 | `pytest tests/ -v` | 월요일 |

---

## 3. 토요일 기본 모듈 완료 기록

토요일에는 약 4시간 동안 의존성이 낮은 기본 모듈 4개를 구현하고 각 단위 테스트를 확인했다.

| 완료 모듈 | 확인 테스트 |
|-----------|-------------|
| `ReLU.forward/backward` | `pytest tests/test_relu.py -v` |
| `Softmax.forward/backward` | `pytest tests/test_softmax.py -v` |
| `Affine.forward/backward` | `pytest tests/test_affine.py -v` |
| `cross_entropy_loss` | `pytest tests/test_cross_entropy_loss.py -v` |

---

## 4. 월요일 오전 Chapter 6 리뷰

월요일 오전에는 오후 구현 전에 Chapter 6의 optimizer, BatchNorm, Dropout 개념과 테스트 리스크를 먼저 맞춘다.

| 시간 | 리뷰 내용 |
|------|-----------|
| 10:00-10:30 | `SGD`, `Adam` update 식, optimizer state, bias correction 확인 |
| 10:30-11:20 | `BatchNorm.forward/backward` 입력/출력 shape, gradient shape, running mean/var, train/eval 차이 확인 |
| 11:20-11:50 | `Dropout.forward/backward` mask 적용, train/eval 차이 확인 |
| 11:50-12:00 | 오후 구현 순서와 담당 테스트 확인 |

---

## 5. 월요일 오후-저녁 구현 계획

월요일 구현 시간은 14:00-18:00, 19:00-23:00로 나누어 사용한다. 전체 테스트는 마지막에만 의존하지 않고, 주요 묶음 단위로 나누어 확인한다.

| 시간 | 작업 |
|------|------|
| 14:00-14:30 | 토요일 구현 범위 테스트 재확인 및 실패 보완 |
| 14:30-15:30 | `SGD.update`, `Adam.update` 구현 및 `test_sgd.py`, `test_adam.py` 실행 |
| 15:30-16:40 | `NeuralNetwork` 구현 및 `test_neural_network.py` 실행 |
| 16:40-17:00 | 기본 layer/loss/optimizer/network 묶음 회귀 테스트 |
| 17:00-18:00 | `BatchNorm.forward/backward` 구현 및 `test_batchnorm.py` 실행 |
| 19:00-20:00 | `BatchNorm` 실패 보완 또는 `Dropout.forward/backward` 구현 및 `test_dropout.py` 실행 |
| 20:00-21:00 | `train`, `evaluate` 구현 및 `test_training.py`, `test_evaluate.py` 실행 |
| 21:00-22:00 | 전체 `pytest tests/ -v` 실행 및 실패 보완 |
| 22:00-22:40 | 정확도 실험, loss curve 기록 |
| 22:40-23:00 | MNIST `REPORT.md`에 구현 현황, 테스트 결과, 실험 결과 정리 |

---

## 6. MNIST 완료 기준

- 관련 테스트를 단계별로 통과한다.
- 전체 `pytest tests/ -v`를 실행한다.
- test accuracy 최소 95%, 가능하면 97% 이상을 목표로 한다.
- `REPORT.md`에 모델 구조, 학습 설정, 실험 환경, 정확도, loss curve, 회고를 기록한다.
- `Forward -> Loss -> Backward -> Update` 흐름을 설명할 수 있어야 한다.
