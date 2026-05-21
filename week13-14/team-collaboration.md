# 팀 협업 룰 (Week 13-14)

> 작성 시점: 주차 시작 시
>
> 기준 일정: 2026-05-22 ~ 2026-06-04
>
> 필요 시 언제든 팀 논의를 거쳐 수정할 수 있습니다.

---

## 1. 일정·만남

| 항목 | 우리 팀 규칙 |
|------|--------------|
| 출근 시간 | 팀 합의 후 확정 |
| 스크럼 미팅 | 매일 오전, 전날 통과/실패 테스트와 오늘 구현 범위 공유 |
| 점심 식사 | 팀 합의 후 확정 |
| 저녁 식사 | 팀 합의 후 확정 |
| 코어타임 | 테스트 실패 분석, 구현 설명, PR 리뷰 중심으로 운영 |
| 코칭실 사용 | BPE, attention, backward, 학습 안정화처럼 막힌 주제는 코칭실 예약 후 공동 점검 |
| AI 특강 | 2026-05-22(금), 이동석 코치님 |
| 신경망 과제 발표 | 2026-05-28(목) 오전 10시 |
| GPT LLM 과제 발표 | 2026-06-04(목) 오전 10시 |
| 최종 제출 | 2026-06-04(목) 정오까지 WIL, 과제1/2 repository 주소 제출 |
| 차주 발제/나만무 리더 지원 및 추천 | 2026-06-04(목) 오후 2시 |
| 운영진 티타임 | 2026-06-04(목) 오후 3시 |
| 나만무 리더 선정 및 발표 | 2026-06-04(목) 오후 6시 |


## 2. 소통 방식

| 항목 | 우리 팀 규칙 |
|------|--------------|
| 연락 채널 | 슬랙, 카톡 |
| 응답 기준 | 당일 확인 원칙, 막힌 테스트와 Colab 런타임 문제는 바로 공유 |
| 긴급 연락 | 전화통화 |
| 테스트 실패 공유 | 실패 테스트명, 실행 명령, 기대 결과, 실제 결과, 입력 shape, 의심 함수, 최근 수정 commit, 다음 가설을 함께 공유 |
| 실험 결과 공유 | 모델 구조, 데이터 크기, epoch, learning rate, loss/accuracy, 실행 환경, 실행 시간을 함께 공유 |


## 3. 작업·진행 관리

| 항목 | 우리 팀 규칙 |
|------|--------------|
| GitHub Projects | MNIST TODO, mini GPT TODO, 테스트, REPORT, 발표 자료, 실험 기록 관리 |
| 진행 상황 공유 | 스크럼 + 코어타임 + PR 설명 + 테스트 결과표로 공유 |
| CSV 기준 이슈 관리 | `week13-14_issues_complete.csv`의 공통 6개, 학습 21개, 과제 2개를 Projects 또는 체크리스트 기준 항목으로 사용 |
| 이슈·태스크 관리 | CSV 과제 2개를 MNIST는 layer/test 단위, GPT는 BPE/Dataset/Attention/Model/Train/Finetune 단위로 분리 |
| 테스트 관리 | 현재 구현 중인 테스트 파일을 먼저 실행하고, 단계별 통과 후 전체 `pytest tests/ -v` 실행 |
| 실험 관리 | 정확도, loss, BPE vocab size, corpus 크기, context length, checkpoint 경로를 REPORT에 기록 |
| 데이터 관리 | `data/`, checkpoint, token, 비밀번호, Personal Access Token은 commit하지 않음 |


## 4. 코드·리뷰

| 항목 | 우리 팀 규칙 |
|------|--------------|
| 브랜치 전략 | 개인 branch에서 작업 후 PR로 팀 통합 branch 또는 main/master에 병합 |
| PR 리뷰 | 관련 테스트, 수정 파일/함수, 입력/출력 shape, 실패 처리, 실험 결과를 함께 설명 |
| 머지 기준 | 관련 테스트 통과 + 작성자가 구현 흐름 설명 가능 + 팀원 1명 이상 리뷰 |
| main/master branch | 최종 제출 branch로 유지, 직접 push 금지 |
| 충돌 해결 | 충돌 파일 담당자가 설명 후 팀 논의로 해결 |
| 금지 파일 | 데이터 파일, checkpoint, token, 비밀번호, Colab/GitHub 인증 정보 |


## 5. MNIST 구현 기준

1주차에는 가능하면 3일 이내에 MNIST 과제를 마치고, 남은 기간에는 정확도 실험과 REPORT, mini GPT 준비를 병행한다.

| 단계 | 기준 |
|------|------|
| 작은 테스트 우선 | `test_relu.py`부터 현재 구현 대상 테스트만 먼저 실행 |
| shape 설명 | forward output shape와 backward gradient shape를 PR에 적음 |
| train/eval 구분 | BatchNorm과 Dropout은 train/test 동작 차이를 반드시 확인 |
| 학습 루프 | Forward -> Loss -> Backward -> Update 순서 유지 |
| 정확도 실험 | 최소 95%, 권장 97% 이상 목표 |
| REPORT | 모델 구조, 학습 설정, 실험 환경, 결과, 회고 기록 |


## 6. mini GPT 구현 기준

2주차에는 TODO와 테스트 파일 순서대로 구현한다. 처음부터 전체 `pytest tests/ -v`만 실행하지 않는다.

| 단계 | 기준 |
|------|------|
| BPE | UTF-8 byte-level encode/decode 복원 확인, vocab 저장/로드 기록 |
| Dataset/Embedding | input과 target 한 칸 shift, token/position embedding shape 확인 |
| Attention | Q/K/V shape, causal mask, attention weight shape 확인 |
| Model | LayerNorm, GELU, FeedForward, residual connection, loss 계산 확인 |
| Train | train/eval mode, `torch.no_grad()`, device 이동, checkpoint 저장/로드 확인 |
| Finetune | padding 제외 마지막 token hidden state, classifier output shape 확인 |
| REPORT | 구현 현황, 테스트 결과, 데이터, BPE, 모델 구조, 학습/fine-tuning 결과 기록 |


## 7. AI 사용 룰

- AI에게 전체 코드를 한 번에 요청하지 않는다.
- 실패한 테스트 하나와 현재 구현 중인 함수 하나를 기준으로 질문한다.
- AI 답변을 반영하기 전 입력 shape, 출력 shape, device 이동, train/eval mode, 테스트 통과 여부를 확인한다.
- 설명할 수 없는 코드는 PR에 올리지 않는다.
- AI 대화에 token, 비밀번호, Personal Access Token, 데이터 원본 파일 내용 전체를 붙이지 않는다.


## 8. 완료 기준

- MNIST 전체 테스트를 통과하고 정확도와 loss curve를 기록한다.
- mini GPT 전체 테스트를 통과하고 BPE, 학습, 생성, fine-tuning 결과를 기록한다.
- CSV 기준 공통 6개, 학습 21개, 과제 2개 항목의 진행 상태를 체크한다.
- 팀원이 각자 맡은 구현의 입력/출력 shape와 실패 조건을 설명할 수 있다.
- REPORT.md는 구현 현황, 테스트 통과 현황, 실험 환경, 결과, 회고를 포함한다.
- WIL에는 단순 결과보다 배운 개념, 막힌 지점, 수정 근거, 다음 리스크를 남긴다.
