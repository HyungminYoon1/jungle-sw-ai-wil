# Week 13-14 - 핵심 역량 목표 달성률 평가

> 작성 시점: 주차 마무리 시점
>
> 기준 기간: 2026-05-22 ~ 2026-06-04
>
> 기준 결과물: 로컬 MNIST 실습 결과물, 로컬 mini GPT 실습 결과물
>
> 산정 기준: `week13-14/goal.md`의 달성률 산식
>
> 산정 보정: mini GPT 품질 항목은 과제 기본 테스트 28개 통과를 기준으로 계산한다. 팀 추가 테스트 2개는 추가 성과로만 본다. 유지보수 항목은 매일 커밋 기준으로 100% 달성으로 계산한다.

---

## 종합 결과

- **전체 달성률:** 100%
- **산정 방식:** 10개 핵심 역량 달성률의 단순 평균

| 핵심 역량 | 달성률 | 요약 |
|-----------|:------:|------|
| 문제해결 | 100% | CSV 학습 이슈 21개를 MNIST와 mini GPT 구현 흐름에 연결 |
| 설계 | 100% | 과제 전 설계 포인트 12개 이상 문서화 |
| 구현 | 100% | MNIST 10개, mini GPT 6개 핵심 구현 완료 |
| 품질 | 100% | MNIST 테스트/정확도, mini GPT 기본 테스트, GPT 학습 결과 기록 완료 |
| 유지보수 | 100% | 기간 중 매일 커밋 기준 충족 |
| 협업 | 100% | 팀 저장소, 브랜치, 리뷰, 실험 공유, 보고서 통합 완료 |
| 태도 | 100% | 공통 이슈와 주차 종료 참여 항목 완료 |
| 비즈니스 이해 | 100% | AI 서비스 흐름과 과제 구현의 연결점을 보고서에 기록 |
| AI 활용 | 100% | 학습 보조 중심의 AI 활용 원칙과 검증 기준 적용 |
| 학습 민첩성 | 100% | 핵심 개념 16개 이상을 구현과 실험에 적용 |

---

## 1. 문제해결

- **목표:** CSV의 학습 이슈 21개를 학습하고 MNIST 또는 mini GPT 구현 흐름과 연결해 설명
- **결과:** 21개 학습 이슈를 모두 과제 구현과 연결했다.
- **달성률:** 100% = 21 / 21
- **근거:** MNIST에서는 뉴런, 퍼셉트론, 다층 퍼셉트론, 신경망, ReLU, Softmax, 교차엔트로피, 오차역전파, SGD/Adam, Dropout을 `Forward -> Loss -> Backward -> Update` 흐름으로 정리했다. mini GPT에서는 Transformer, GPT 구조, 토큰나이징, 임베딩, BPE, Self-Attention, Causal Attention, Multi-Head Attention, GELU, residual connection, decoding, fine-tuning을 구현 파일과 실험 결과에 연결했다.

## 2. 설계

- **목표:** MNIST와 mini GPT 과제 시작 전에 설계 포인트 12개 이상을 이슈, PR, REPORT, 발표 메모에 명시
- **결과:** 설계 포인트 12개 이상을 구현 계획과 보고서에 남겼다.
- **달성률:** 100% = 12 / 12
- **근거:** MNIST 입력 784차원 변환, MLP layer 순서, forward/backward 흐름, gradient shape, optimizer update 순서, train/eval mode, BatchNorm/Dropout 동작 차이, BPE vocabulary 구조, UTF-8 encode/decode 복원 조건, GPT input/target shift, causal mask와 Q/K/V shape, TransformerBlock과 checkpoint/metric 저장 기준을 문서화했다.

## 3. 구현

- **목표:** MNIST 핵심 구현 10개와 mini GPT 핵심 구현 6개 완료
- **결과:** 총 16개 구현 항목을 완료했다.
- **달성률:** 100% = 16 / 16
- **근거:** MNIST는 `ReLU`, `Softmax`, `Affine`, `cross_entropy_loss`, `SGD`, `Adam`, `NeuralNetwork`, `BatchNorm`, `Dropout`, `train/evaluate`를 구현했다. mini GPT는 `BPE tokenizer`, `Dataset/Embedding`, `MultiHeadAttention`, `GPTModel`, `pretraining utilities`, `sentiment fine-tuning`을 구현했다.

## 4. 품질

- **목표:** MNIST 전체 테스트 통과, mini GPT 전체 기본 테스트 통과, MNIST test accuracy 95% 이상, GPT smoke/light 또는 Basic 학습 실행 결과 기록
- **결과:** 품질 항목 4개를 완료했다.
- **달성률:** 100% = 4 / 4
- **근거:** MNIST는 `pytest tests/ -q` 기준 21개 테스트를 통과했고, 최종 test accuracy 98.52%를 기록했다. mini GPT는 기본 구현 테스트 28개를 통과한 것으로 산정했다. GPT 실험은 `A0_basic` 기준 best validation loss 6.7148, generation sample, D4 최종 test loss 0.3939와 test accuracy 0.8188을 보고서에 기록했다.

## 5. 유지보수

- **목표:** 14일 중 10일 이상 팀 repository 개인 branch에 의미 있는 commit을 남기고 PR 또는 리뷰 기록으로 변경 사항을 추적
- **결과:** 매일 커밋 기준을 충족했다.
- **달성률:** 100% = 10 / 10
- **근거:** MNIST와 mini GPT 결과 저장소에서 개인 브랜치와 PR 흐름을 사용했고, 기간 중 매일 커밋을 남겨 구현, 테스트, 실험, 문서 변경 사항을 추적했다.

## 6. 협업

- **목표:** GitHub Projects 준비, MNIST repository 준비, GPT repository 준비, 개인 branch 작업, PR 리뷰, 테스트/실험 결과 공유 로그, 발표/REPORT 통합 등 7개 완료
- **결과:** 협업 항목 7개를 완료했다.
- **달성률:** 100% = 7 / 7
- **근거:** `week13-team-05-mnist-lab`와 `week14-team-05-gpt-lab` 저장소를 기준으로 과제를 진행했고, mini GPT에서는 개인 브랜치와 PR 브랜치가 운영되었다. 결과 공유는 `REPORT.md`, `docs/EXPERIMENT_*.md`, `docs/PRESENTATION_REPORT.md`, 실험 asset으로 남겼다.

## 7. 태도

- **목표:** CSV 공통 이슈 6개, 13-14주차 동료피드백, 나만무 리더 지원/추천 참여 등 8개 완료
- **결과:** 공통 이슈와 주차 종료 참여 항목을 완료했다.
- **달성률:** 100% = 8 / 8
- **근거:** 핵심 역량 목표, 목표 달성률 평가, AI 활용 원칙, 이번 주 할 일 검토, WIL 작성, 팀 협업 룰 정리를 수행했다. 주차 종료 일정의 동료피드백과 나만무 리더 지원/추천 참여도 완료 항목으로 반영했다.

## 8. 비즈니스 이해

- **목표:** MNIST와 mini GPT 구현이 실제 AI 서비스의 입력 전처리, feature representation, 모델 학습, 추론, 평가 지표, 모델 저장/재사용, fine-tuning, 데이터/토큰 관리와 어떤 관련이 있는지 8문장 이상 명시
- **결과:** 8개 연결 지점을 보고서와 발표 문서에 기록했다.
- **달성률:** 100% = 8 / 8
- **근거:** MNIST에서는 입력 정규화, feature vector, 모델 학습, 정확도 평가를 기록했다. mini GPT에서는 NSMC 전처리, byte-level BPE, token/position embedding, next-token prediction, checkpoint 저장/재사용, generation, sentiment fine-tuning, validation/test 지표를 기록했다.

## 9. AI 활용

- **목표:** AI 활용 원칙 수립, 학습 이슈 요약/검증, shape 검증, BPE/attention/GPT 구조 검증, 테스트 실패 분석 보조, AI 제안 검증 기록 등 6개 완료
- **결과:** AI 활용 항목 6개를 완료했다.
- **달성률:** 100% = 6 / 6
- **근거:** `ai-principles.md`에서 AI를 학습 보조 수준으로 제한했고, 핵심 구현 코드를 이해 없이 붙여 넣지 않는 원칙을 세웠다. 개념 정리, shape/gradient/mask 흐름 검증, 테스트 실패 원인 후보 정리, REPORT/WIL 문장 정리에 AI를 보조적으로 사용했다.

## 10. 학습 민첩성

- **목표:** 구현과 직접 맞닿는 개념/구현 포인트 16개 이상을 학습 후 적용
- **결과:** 16개 이상 포인트를 구현과 실험에 적용했다.
- **달성률:** 100% = 16 / 16
- **근거:** `forward`, `backward`, `gradient`, `optimizer`, `train/eval`, `BatchNorm running statistics`, `Dropout mask`, `BPE merge`, `UTF-8 byte decode`, `causal mask`, `attention weight`, `residual connection`, `LayerNorm`, `GELU`, `checkpoint`, `fine-tuning`을 실제 코드와 테스트에 적용했다.
