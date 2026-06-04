# Week 13-14 - WIL (What I Learned)

> 작성 시점: 주차 마무리 시점
>
> 기간: 2026-05-22 ~ 2026-06-04
>
> 주제: AI로 진화하기, MNIST 신경망 구현, mini GPT LLM 구현

---

## 1. 이번 주 목표

이번 주차의 목표는 신경망과 LLM의 핵심 구성 요소를 직접 구현하면서, 모델이 입력을 받아 손실을 계산하고 학습되는 흐름을 코드 수준에서 설명할 수 있게 되는 것이었다.

첫 번째 과제에서는 NumPy만 사용해 MNIST 손글씨 숫자 분류 신경망을 구현했다. `Forward -> Loss -> Backward -> Optimizer Update` 흐름을 각 layer와 optimizer에 연결하고, 테스트 정확도 95% 이상, 가능하면 97% 이상을 달성하는 것을 목표로 했다.

두 번째 과제에서는 PyTorch만 사용해 mini GPT를 구현했다. UTF-8 byte-level BPE tokenizer, GPTDataset, InputEmbedding, Causal Multi-Head Attention, TransformerBlock, GPTModel, pretraining loop, sentiment fine-tuning까지 이어지는 전체 흐름을 직접 연결하는 것이 목표였다.

---

## 2. 시도한 접근 방식

MNIST는 작은 단위 테스트를 먼저 통과시키는 방식으로 진행했다. `ReLU`, `Softmax`, `Affine`, `cross_entropy_loss`처럼 의존성이 낮은 구성 요소부터 구현하고, 이후 `SGD`, `Adam`, `NeuralNetwork`, `BatchNorm`, `Dropout`, `train`, `evaluate` 순서로 확장했다. 전체 구현이 끝난 뒤에는 실제 MNIST 데이터로 20 epoch 학습을 실행해 test accuracy 98.52%를 기록했다.

mini GPT는 `BPE -> Dataset/Embedding -> Attention -> Model -> Train -> Finetune` 순서로 코드를 구현하였다. 모듈 구현 간에는 각 단계의 shape와 입출력 조건을 먼저 확인했다. 기본 구현 테스트 28개를 통과한 뒤, `A0_basic` 기준선을 세우고 이에 대해 사전 학습과 감성 분류 fine-tuning 실험, 그리고 추가적인 파라미터 탐색을 실행했다.

실험은 Smoke, Light, Basic 기준을 나누어 진행했다. Basic baseline인 `A0_basic`은 `vocab_size=3000`, `context_length=128`, `train_char_limit=1500000`, `num_epochs=2` 설정으로 실행했고, best validation loss 6.7148을 기록했다. 이후 D fine-tuning에서는 D0 checkpoint를 최종 선택해 test loss 0.3939, test accuracy 0.8188을 기록했다.

AI는 코드 생성보다 학습 보조와 검증 보조로 사용했다. shape, gradient, causal mask, BPE encode/decode 복원 조건처럼 직접 설명해야 하는 부분은 테스트와 보고서 근거를 우선했고, AI 답변은 개념 정리와 실패 원인 후보를 좁히는 데 사용했다.

---

## 3. 문제와 해결 과정

MNIST에서 가장 중요한 부분은 forward와 backward가 같은 중간 값을 공유해야 한다는 점이었다. `Affine`은 입력 `x`, weight `W`, bias `b`의 shape가 맞아야 하고, backward에서는 `dx`, `dW`, `db`가 각각 원래 입력과 파라미터 shape로 돌아와야 한다. Softmax와 Cross Entropy는 따로 생각하면 복잡하지만, 학습 루프에서는 정답 클래스 위치에서 1을 빼고 batch size로 나누는 방식으로 출력층 gradient를 만들 수 있었다.

BatchNorm과 Dropout은 train/eval mode를 분리하는 것이 핵심이었다. BatchNorm은 학습 중 batch mean과 variance를 사용하고, 평가 중에는 running mean과 running variance를 사용해야 했다. Dropout은 학습 중에만 mask를 만들고, 평가 중에는 고정 비율로 scale해야 했다. 이 차이를 구현하고 테스트로 확인하면서 정확도 98.52%까지 도달했다.

mini GPT에서는 BPE와 Attention이 가장 큰 학습 지점이었다. 한글은 UTF-8에서 한 글자가 여러 byte로 표현되므로, decode에서 byte를 중간에 문자로 바꾸면 깨질 수 있다. BPE merge token을 원래 byte까지 펼친 뒤 마지막에 UTF-8 decode를 해야 encode/decode 복원이 유지된다.

Attention에서는 Q/K/V shape와 causal mask가 중요했다. 입력 `(B, T, C)`를 head 단위로 나누어 `(B, n_heads, T, head_dim)`으로 바꾸고, attention score는 `(B, n_heads, T, T)`가 된다. causal mask는 현재 token이 미래 token을 보지 못하게 하므로, next-token prediction의 조건을 코드로 강제하는 장치였다.

실험 단계에서는 모든 후보를 Basic 규모로 확인하기 어려웠다. 그래서 Smoke와 Light는 동작 확인과 빠른 후보 선별용으로 사용하고, 제출 기준은 `A0_basic`과 D fine-tuning 최종 결과를 중심으로 정리했다. B/C 실험 결과는 유망 후보로 기록하되, Basic 기준 재확인이 필요한 항목으로 남겼다.

---

## 4. 새로 배운 점

- 신경망 구현에서는 정확도보다 `Forward -> Loss -> Backward -> Update` 순서를 설명할 수 있는지가 더 중요했다.
- ReLU, Softmax, Affine, Cross Entropy는 단독 함수가 아니라 학습 루프 안에서 gradient 흐름으로 연결된다.
- BatchNorm과 Dropout은 train/eval mode가 다르며, 이 분리를 놓치면 테스트와 실제 평가 결과가 어긋난다.
- byte-level BPE는 처음 보는 한글, 영어, 숫자, 문장부호를 모두 byte 단위로 처리할 수 있지만, decode 복원 조건을 정확히 지켜야 한다.
- GPTDataset의 input/target shift는 next-token prediction 문제를 데이터셋 수준에서 만드는 과정이다.
- 임베딩은 token id를 모델이 처리할 수 있는 벡터 표현으로 바꾸는 과정이다. GPT에서는 token embedding에 position embedding을 더해, 같은 token이라도 문장 안의 위치 정보를 함께 반영한다.
- Causal Attention은 단순한 mask가 아니라 GPT가 미래 token을 보지 못하게 만드는 핵심 제약이다.
- TransformerBlock은 attention, feed-forward, residual connection, LayerNorm이 shape를 유지하면서 반복되는 구조다.
- checkpoint, metric JSONL, tokenizer 저장 경로를 남겨야 실험 결과를 다시 추적할 수 있다.
- 사전 학습 성능과 downstream fine-tuning 성능은 따로 보지 말고, checkpoint 선택 기준과 함께 연결해서 기록해야 한다.

---

## 5. 다음 주 계획

다음 주차에는 AI 응용 기술을 활용한 게시판 구현을 진행한다. 이번 주에 정리한 모델 학습 흐름과 AI 활용 원칙을 바탕으로, 기능 구현 전에 요구사항과 데이터 흐름을 먼저 나누어 볼 계획이다.

특히 mini GPT 실험에서 배운 데이터 전처리, 토큰/입력 관리, 모델 출력 해석, 평가 기준 기록 방식을 다음 과제에도 적용하고 싶다. 구현 결과만 남기기보다, 어떤 입력이 어떤 처리 단계를 거쳐 어떤 출력으로 이어지는지 문서와 테스트로 함께 남기는 것을 목표로 한다.

---

## 참고 링크

- [MNIST 학습 및 구현 레포]: https://github.com/Jungle-12-303/week13-team-05-mnist-lab
- [mini GPT 학습 및 구현 레포]: https://github.com/Jungle-12-303/week14-team-05-gpt-lab

