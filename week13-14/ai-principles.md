# AI 활용 원칙 (Week 13-14)

## 과제별 AI 사용 범위

| 과제 | AI 사용 | 필수 조건 |
|------|---------|-----------|
| **MNIST 신경망 구현** | 학습·검토 보조 중심 | forward, loss, backward, optimizer update 흐름과 각 tensor shape를 직접 설명할 수 있어야 함 |
| **mini GPT LLM 구현** | 학습·검토 보조 중심 | BPE, Dataset, Embedding, Attention, GPT block, train/fine-tuning 흐름을 직접 설명할 수 있어야 함 |
| **CSV 학습 이슈 21개 정리** | 허용 | AI 요약을 그대로 쓰지 않고 과제 구현, 테스트, 발표 메모 중 하나와 연결해 검증 |
| **테스트 실패 분석** | 허용 | 실패한 테스트 하나와 현재 구현 함수 하나를 기준으로 원인 후보를 좁히는 용도로 사용 |
| **실험/REPORT/WIL 정리** | 허용 | 실험 수치와 구현 현황을 과장하지 않고, 직접 확인한 결과만 기록 |
| **제한 환경 또는 개인 이해 검증 전 코드 제출** | 미허용 | 설명할 수 없는 코드를 제출하거나 PR에 올리지 않음 |

## 우리 팀 선택 (AI로 진화하기)

| 수준 | 허용 | 선택 |
|------|------|------|
| 일부 사용 | 공식 문서 이해, 용어 정리, 오류 메시지 해석 | □ |
| 학습 보조 | 위 + 테스트 실패 원인 분석, shape/gradient/attention 흐름 검증 | ☑ |
| 코드 생성 | 위 + 핵심 구현 코드 생성 | □ |

이번 주에는 AI를 코드 생성기처럼 사용하지 않는다. 특히 신경망 backward, BPE merge, attention mask, train loop처럼 과제의 핵심 학습 대상은 이해 없이 붙여 넣지 않는다.

## 핵심 원칙

- AI가 설명한 내용은 과제 문서, 교재 흐름, 테스트 결과와 대조한다.
- CSV의 학습 이슈 21개는 AI 요약만으로 완료 처리하지 않고, 구현 코드·테스트·REPORT·발표 메모 중 하나에 연결해 검증한다.
- MNIST 구현에서는 입력 shape, 출력 shape, gradient shape, parameter update 순서를 직접 설명할 수 있어야 한다.
- mini GPT 구현에서는 BPE encode/decode 복원, input/target shift, Q/K/V shape, causal mask, loss 계산, train/eval mode를 직접 설명할 수 있어야 한다.
- AI에게 전체 파일 구현을 한 번에 요청하지 않는다. 실패한 테스트 하나와 현재 구현 중인 함수 하나만 기준으로 질문한다.
- AI가 제안한 코드는 테스트 통과 여부와 별개로 왜 맞는지 설명한 뒤에만 반영한다.
- 데이터 파일, checkpoint, token, 비밀번호, Personal Access Token은 AI 대화나 로그에 노출하지 않는다.
- Colab/GitHub 인증 정보를 확인할 때는 존재 여부만 확인하고 값을 출력하지 않는다.

## 허용하는 것

- 신경망 forward/backward 계산 흐름 설명 요청
- CSV 학습 이슈별 핵심 개념을 과제 구현과 연결해 정리하는 질문
- ReLU, Softmax, Cross Entropy, BatchNorm, Dropout의 shape와 gradient 검증 질문
- `pytest` 실패 메시지를 기반으로 의심 함수와 입력 조건 정리
- BPE tokenizer의 merge, save/load, encode/decode 복원 조건 설명 요청
- MultiHeadAttention의 Q/K/V shape, causal mask, output projection 흐름 검증
- train/eval mode, `torch.no_grad()`, device 이동, checkpoint 저장 항목 검토
- REPORT.md, 발표 자료, WIL 문장 정리 보조

## 허용하지 않는 것

- MNIST 또는 mini GPT 핵심 구현 코드를 이해 없이 AI로 생성해 그대로 붙여 넣기
- 전체 `src/` 파일이나 전체 TODO를 한 번에 AI에게 구현하게 하기
- 테스트 실패 원인을 확인하지 않고 AI가 제안한 수정만 적용하기
- shape, gradient, mask, loss 계산을 설명할 수 없는 코드를 PR에 올리기
- Hugging Face `transformers`, 외부 tokenizer, pretrained model처럼 과제 금지 도구 사용을 AI에게 우회 요청하기
- 데이터, checkpoint, token, 비밀번호, Personal Access Token 값을 출력하거나 공유하기

## 예외 상황

- Python 3.11, Colab GPU, pytest 실행, GitHub branch/PR 같은 환경 문제는 AI 도움을 받을 수 있다.
- 문서나 발표 자료를 정리할 때는 AI 도움을 받을 수 있지만, 구현 성과와 실험 수치를 과장하지 않는다.
- 마감 직전이라도 핵심 구현 코드를 설명하지 못하면 병합하지 않는다.
