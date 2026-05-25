# 팀 협업 룰 (Week 13-14)

> 작성 시점: 주차 시작 시
>
> 기준 일정: 2026-05-22 ~ 2026-06-04
>
> 필요 시 언제든 팀 논의를 거쳐 수정할 수 있습니다.

---

## 1. 일정·만남

아래 일반 시간표는 2026-05-26부터 mini GPT 구현 기간에 적용한다. 2026-05-25 MNIST 마감일에는 쿼드 프로그래밍으로 야간까지 구현 및 보고서 작성을 진행했다.

| 항목 | 우리 팀 규칙 |
|------|--------------|
| 출근 시간 | 10:00 |
| 오전 개인 학습 | 10:00-11:50, 당일 구현 범위에 필요한 교재/테스트/shape 흐름 정리 |
| 학습 내용 토의 | 13:00-14:00, 각자 학습한 개념과 구현 리스크 공유 |
| 개인 구현 | 14:00-18:00, 개인 브랜치에서 당일 구현 범위 작업 및 테스트 실행 |
| 구현 결과 비교/병합 | 19:00-22:00, 구현 비교, 병합 후보 결정, main 반영, 병합 후 테스트 |
| 개인 학습 및 복습 | 22:00 이후, 당일 구현 흐름과 다음날 학습 범위 정리 |
| 스크럼 미팅 | 매일 오전 또는 학습 토의 시작 시, 전날 통과/실패 테스트와 오늘 구현 범위 공유 |
| 점심 식사 | 11:50 |
| 저녁 식사 | 17:50 |
| 코어타임 | 구현 결과 비교, 테스트 실패 분석, 구현 설명, PR 리뷰, main 병합 중심으로 운영 |
| 코칭실 사용 | 필요시 예약 |
| AI 특강 | 2026-05-22(금) 10:00 , 이동석 코치님 |
| 부재일 | 2026-05-24(일), 2026-05-31(일) |
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
| 세부 구현 일정 | `implementation-plan.md`의 전체 일정과 프로젝트별 상세 계획을 기준으로 진행 |
| 진행 상황 공유 | 스크럼 + 코어타임 + PR 설명 + 테스트 결과표로 공유 |
| CSV 기준 이슈 관리 | `week13-14_issues_complete.csv`의 공통 6개, 학습 21개, 과제 2개를 Projects 또는 체크리스트 기준 항목으로 사용 |
| 이슈·태스크 관리 | CSV 과제 2개를 MNIST는 layer/test 단위, GPT는 BPE/Dataset/Attention/Model/Train/Finetune 단위로 분리 |
| 테스트 관리 | 현재 구현 중인 테스트 파일을 먼저 실행하고, 단계별 통과 후 전체 `pytest tests/ -v` 실행 |
| 실험 관리 | 정확도, loss, BPE vocab size, corpus 크기, context length, checkpoint 경로를 REPORT에 기록 |
| 데이터 관리 | `data/`, checkpoint, token, 비밀번호, Personal Access Token은 commit하지 않음 |
| 구현 방식 | 2026-05-25까지 MNIST는 쿼드 프로그래밍으로 진행하고, 2026-05-26부터 mini GPT는 개인 브랜치 구현 후 코어타임에 비교/병합 |


## 4. 코드·리뷰

| 항목 | 우리 팀 규칙 |
|------|--------------|
| 브랜치 전략 | 05-25까지 쿼드 프로그래밍 중 합의된 변경은 main에 직접 반영 가능. 05-26부터는 개인 브랜치에서 작업 후 main(팀 통합/최종 제출 브랜치)로 PR하는 것을 원칙으로 함 |
| PR 리뷰 | 관련 테스트, 수정 파일/함수, 입력/출력 shape, 실패 처리, 실험 결과를 함께 설명 |
| PR 머지 기준 | 관련 테스트 통과 + 작성자가 구현 흐름 설명 가능 + 팀 코어타임 비교 후 합의 + 팀원 1명 이상 리뷰 |
| push 기준 | 예외적으로 쿼드 프로그래밍 중 모든 팀원의 동의가 있으면 main으로 직접 push 가능 |
| main branch | 최종 제출 브랜치로 유지 |
| 충돌 해결 | 충돌 파일 담당자가 변경 내용을 설명한 뒤 팀 논의로 해결 |
| 금지 파일 | 데이터 파일, `checkpoint`, `token`, 비밀번호, Colab/GitHub 인증 정보 |


## 5. MNIST 구현 기준

1주차에는 가능하면 3일 이내에 MNIST 과제를 마치고, 2026-05-25까지 정확도 실험과 REPORT 작성 완료. 2026-05-26부터 남은 기간까지는 mini GPT 프로젝트 진행.

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
