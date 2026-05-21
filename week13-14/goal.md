# Week 13-14 - 핵심 역량 목표 설정

> 작성 시점: 주차 시작 시

이번 주차는 2주 동안 "AI로 진화하기" 과제를 진행합니다.
1주차에는 NumPy 기반 MNIST 필기체 숫자 인식 신경망을 구현하고, 2주차에는 PyTorch 기반 mini GPT LLM을 구현합니다.
목표와 체크 기준은 `local/data/week13-14_issues_complete.csv`의 **공통 6개, 학습 21개, 과제 2개** 이슈를 기준으로 맞춥니다.
달성률 산정을 위해 목표는 수치화 가능한 형태로 작성합니다.

---

## 1. 문제해결
- **목표:** CSV의 학습 이슈 21개(`뉴런`, `퍼셉트론`, `다중 퍼셉트론`, `신경망`, `활성화 함수(Sigmoid & Relu)`, `손실 함수(Softmax & 교차엔트로피 오차)`, `오차역전파법`, `최적화(SGD & Adam)`, `Dropout`, `트랜스포머 구조`, `GPT 구조`, `토큰나이징`, `임베딩`, `BPE`, `Self-Attention`, `Causal Attention`, `멀티헤드 어텐션`, `GELU`, `숏컷 연결`, `디코딩 전략`, `미세조정`)를 학습하고 MNIST 또는 mini GPT 구현 흐름과 연결해 설명
- **달성률 산식:** (학습 정리와 구현 연결을 완료한 CSV 학습 이슈 수 / 21) × 100


## 2. 설계
- **목표:** CSV의 과제 이슈 2개(`신경망을 이용한 필기체 숫자 인식`, `LLM 구현`)를 시작하기 전에 **MNIST forward/backward shape, gradient 흐름, optimizer update 순서, train/eval mode, BatchNorm/Dropout 동작 차이, BPE vocabulary 구조, encode/decode 복원 조건, GPT dataset input/target shift, attention mask, GPT block 구조, checkpoint/데이터 관리, 실험 기록 기준** 등 설계 포인트 **12개 이상**을 이슈·PR·REPORT·발표 메모에 명시
- **달성률 산식:** (문서화한 설계 포인트 수 / 12) × 100


## 3. 구현
- **목표:** CSV 과제 이슈 2개를 구현 단위로 쪼개서, MNIST 핵심 구현 10개(`ReLU`, `Softmax`, `Affine`, `cross_entropy_loss`, `SGD`, `Adam`, `NeuralNetwork`, `BatchNorm`, `Dropout`, `train/evaluate`)와 mini GPT 핵심 구현 6개(`BPE tokenizer`, `Dataset/Embedding`, `MultiHeadAttention`, `GPTModel`, `pretraining utilities`, `sentiment fine-tuning`) 완료
- **달성률 산식:** (완료한 구현 항목 수 / 16) × 100


## 4. 품질
- **목표:** MNIST 전체 테스트 `pytest tests/ -v` 통과 + mini GPT 전체 테스트 `pytest tests/ -v` 통과 + MNIST test accuracy 최소 95% 이상, 권장 97% 이상 달성 + GPT smoke/light 학습 실행 결과 기록 = **4개 품질 항목 완료**
- **달성률 산식:** (완료한 품질 항목 수 / 4) × 100


## 5. 유지보수
- **목표:** 2주 기간 중 **14일 중 10일 이상** 팀 repository 개인 branch에 의미 있는 commit을 남기고, PR 또는 리뷰 기록으로 구현·테스트·실험 변경 사항을 추적
- **달성률 산식:** (commit 또는 PR 기록이 있는 일수 / 10) × 100 (100% 초과 시 100%로 계산)


## 6. 협업
- **목표:** 팀 GitHub Projects에 CSV 29개 이슈 기준 작업을 등록하고, 팀 GitHub Projects 준비(1개) + MNIST repository 준비(1개) + GPT repository 준비(1개) + 개인 branch 작업(1개) + PR 리뷰(1개) + 테스트/실험 결과 공유 로그(1개) + 발표/REPORT 통합(1개) = **7개 완료**
- **달성률 산식:** (완료한 협업 항목 수 / 7) × 100


## 7. 태도
- **목표:** CSV의 공통 이슈 6개(`핵심 역량 목표 수립`, `핵심 역량 목표 달성률 평가`, `AI 활용 원칙 수립`, `이번 주 할 일(업무) 검토`, `WIL 작성`, `팀 협업 룰/팁 정리 및 합의`) + 13-14주차 동료피드백 + 나만무 리더 지원/추천 참여 = **8개 완료**
- **달성률 산식:** (완료한 공통/참여 항목 수 / 8) × 100


## 8. 비즈니스 이해
- **목표:** MNIST와 mini GPT 구현이 실제 AI 서비스의 **입력 전처리, feature representation, 모델 학습, 추론, 평가 지표, 모델 저장/재사용, fine-tuning, 데이터/토큰 관리**와 어떤 관련이 있는지 WIL 또는 발표 자료에 **8문장 이상** 명시
- **달성률 산식:** (명시한 문장 수 / 8) × 100


## 9. AI 활용
- **목표:** AI 활용 원칙 수립(1개) + CSV 학습 이슈 요약/검증 기록(1개) + 신경망 개념/shape 검증 기록(1개) + BPE/attention/GPT 구조 검증 기록(1개) + 테스트 실패 분석 보조 기록(1개) + AI 제안 코드/설계 직접 설명 검증 기록(1개) = **6개 완료**
- **달성률 산식:** (완료한 AI 활용 항목 수 / 6) × 100


## 10. 학습 민첩성
- **목표:** CSV 학습 이슈 21개 중 이번 과제 구현과 직접 맞닿는 `forward`, `backward`, `gradient`, `optimizer`, `train/eval`, `BatchNorm running statistics`, `Dropout mask`, `BPE merge`, `UTF-8 byte decode`, `causal mask`, `attention weight`, `residual connection`, `LayerNorm`, `GELU`, `checkpoint`, `fine-tuning`을 포함해 **16개 이상 개념/구현 포인트**를 학습 후 적용
- **달성률 산식:** (학습 후 적용 완료한 개념/구현 포인트 수 / 16) × 100


---

### 달성률 산식 참고
- 문제해결: CSV 학습 이슈 21개를 기준으로 카운트
- 설계: 이슈, PR 설명, REPORT, 발표 메모에 남긴 설계 포인트 수 기준
- 구현: CSV 과제 이슈 2개를 MNIST와 mini GPT TODO/테스트 단위로 쪼개서 카운트
- 품질: 전체 pytest, 정확도, smoke/light 학습 결과 기록 기준
- 유지보수: 일 단위 commit 또는 PR 기록 기준
- 협업/태도/AI 활용: CSV 공통 이슈와 팀 운영 완료 항목 수 기준
- 비즈니스 이해: WIL 또는 발표 자료에 명시한 문장 수 기준
