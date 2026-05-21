# Week 11-12 - WIL (What I Learned)

> 작성 시점: 주차 마무리 시점

PintOS Project 3 - Virtual Memory를 2주 동안 진행했다. 이번 주차는 supplemental page table, lazy loading, page fault, stack growth, `mmap()`/`munmap()`, swap을 구현하는 것이 중심이었다.

---

## 1. 이번 주 목표

이번 주 목표는 PintOS Project 3의 Virtual Memory 필수 범위를 팀 최종 결과물로 완성하는 것이었다.

구체적으로는 SPT를 만들고, page fault가 발생했을 때 lazy loading이나 stack growth로 복구할 수 있게 하며, `mmap()`/`munmap()`, eviction, swap in/out까지 연결하는 것이 목표였다. Project 2에서 구현한 `fork`, fd table, process exit 흐름도 VM 위에서 다시 깨지지 않아야 했다.

팀 최종 결과물 기준으로 VM 필수 테스트 46개, User Programs 회귀 63개, Filesys 회귀 13개, Threads alarm/priority 회귀 18개가 `PASS`로 확인되었다. Extra인 `cow-simple`은 실패 상태로 남았고, 필수 구현과는 분리해서 기록했다.


## 2. 시도한 접근 방식

10주차에는 user program 구현 과정에서 AI가 만든 skeleton에 의존한 부분이 많았다.

2026-05-10까지는 각자 학습하였다. 실제 구현은 2026-05-11부터 시작했고, 11일과 12일에는 4명이 같이 보면서 SPT와 VM core를 맞췄다. 이후에는 2명씩 나누어 lazy loading, page fault, fork, mmap, swap을 이어서 봤다.

구현 순서는 대략 아래 흐름이었다.

| 순서 | 초점 | 본 내용 |
|------|------|---------|
| 1 | SPT | page를 `va` 기준으로 찾고 넣고 지우는 흐름 |
| 2 | lazy loading | 실행 파일을 바로 읽지 않고 fault 시점에 읽는 흐름 |
| 3 | page fault | SPT lookup, writable 검사, stack growth 조건 |
| 4 | fork | 부모 SPT와 page 상태를 자식에게 복사하는 흐름 |
| 5 | mmap / munmap | file-backed page, dirty write-back, cleanup |
| 6 | eviction / swap | frame 부족 시 victim을 고르고 swap으로 내보내는 흐름 |

전체 시스템 기능들이 서로 맞물려 있어 중간까지는 테스트가 안정적으로 통과하기까지 시간이 걸렸다.


## 3. 문제와 해결 과정

처음에 SPT와 PML4의 차이를 헷갈렸다. PML4는 실제 주소 변환에 쓰이는 page table이고, SPT는 아직 frame이 붙지 않은 page나 swap-out된 page까지 기억하는 자료구조였다. 잘 모를 때에는 page fault가 나면 단순히 mapping을 새로 붙이면 된다고 생각했는데, 실제로는 SPT에 남아 있는 정보를 보고 "이 page가 lazy page인지, stack growth 대상인지, swap에서 복구해야 하는지"를 판단해야 했다.

이 부분을 이해한 후에 `spt_find_page()`와 `vm_claim_page()`의 역할이 조금 분리되어 보였다. SPT에서 page를 찾고, frame을 얻고, PML4에 mapping을 붙이고, 그 다음 page type에 맞게 `swap_in()`을 호출하는 식으로 흐름이 이어졌다. 단순해보이지만, 처음 코드를 볼 때는 `page`, `frame`, `pml4`, `aux`가 계속 같이 나와서 누가 진짜 소유자인지 헷갈렸다.

두 번째로 어려웠던 부분은 page fault 조건이었다. write fault가 났다고 해서 무조건 잘못된 접근은 아니었다. 아직 로드되지 않은 writable page에 처음 쓰는 경우도 있기 때문이다. 그래서 SPT에서 page를 찾은 뒤 `page->writable`을 확인해야 했고, SPT에 없는 경우에만 stack growth 후보인지 봐야 했다. syscall 중에 user buffer를 확인하다가 stack growth가 필요한 경우도 있어서 user `rsp`를 저장해 두는 이유도 이때 이해했다.

`fork()`는 Project 2 때도 어려웠는데, VM이 붙으면서 더 복잡해졌다. 부모의 register만 복사하면 되는 것이 아니라 부모의 SPT 상태도 자식에게 맞게 복사해야 했다. 특히 page 상태에 따라 처리 방식이 달랐다. 아직 fault되지 않은 anon page, 이미 frame이 붙은 anon page, swap-out된 anon page, mmap file-backed page를 같은 방식으로 다룰 수 없었다.

가장 기억에 남는 부분은 swap-out된 anon page의 fork 복제였다. 부모 page가 swap disk에 나가 있는 상태에서 자식에게 같은 내용을 줘야 하는데, 부모를 그냥 swap-in하면 부모가 갖고 있던 swap slot 상태가 바뀔 수 있었다. 그래서 부모 swap slot은 유지하고, 자식 frame에는 그 내용을 읽어서 복사하는 방식으로 구현했다. 이 부분을 보면서 swap slot을 "읽는 것"과 "소비하는 것"을 구분해야 한다는 점을 배웠다.

`mmap()`은 단순히 파일 내용을 메모리에 읽어오는 함수가 아니었다. `close(fd)`를 해도 mapping은 유지되어야 하므로 mmap page는 독립적인 file reference를 가져야 했다. 또 `munmap()`이나 process exit 때 dirty page만 파일에 다시 써야 했고, 마지막 page의 zero 영역까지 쓰면 안 되었다. file-backed page는 fault 전에는 `mmap_aux`, fault 후에는 `file_page`로 상태가 나뉘는 것도 처음에는 꽤 헷갈렸다.

다만 이번에도 모든 코드를 처음부터 혼자 다시 구현할 수 있을 정도로 완전히 익혔다고 보기는 어렵다. 10주차보다는 각 구조체가 왜 필요한지 더 많이 말로 설명해 보려고 했지만, VM의 부분적인 흐름을 대략적으로는 설명할 수 있지만 여전히 전체 흐름이 한 번에 머릿속에 들어오지는 않았다. 특히 eviction과 swap, fork가 같이 걸리는 부분은 테스트 결과와 구현 계획 문서를 보면서 모듈 단위로 통과를 시켰으나 각각의 모듈이 어떻게 유기적으로 동작하는지 설명하는 것은 여전히 어렵다.


## 4. 새로 배운 점

첫째, page fault는 항상 오류가 아니라는 점을 배웠다. SPT에 복구할 정보가 있다면 page fault는 lazy loading이나 swap in을 시작하는 정상적인 경로가 될 수 있다.

둘째, SPT entry는 "현재 메모리에 올라온 page"만 뜻하지 않는다. 아직 로드되지 않은 page나 swap-out된 page도 SPT에 남아 있어야 나중에 복구할 수 있다. 그래서 eviction 때 SPT를 지우면 안 된다는 점이 중요했다.

셋째, VM에서는 자원 lifecycle을 계속 따라가야 한다. page는 누가 만들고 언제 지우는지, frame은 언제 비워지는지, aux는 initialize 후 어디서 해제되는지, swap slot은 언제 반환되는지를 놓치면 테스트 하나는 통과해도 다른 회귀에서 문제가 생길 수 있다.

넷째, `mmap()`은 system call 하나를 추가하는 문제가 아니라 file-backed page의 생명주기를 관리하는 문제였다. address validation, overlap 검사, dirty write-back, file reference 정리가 모두 같이 맞아야 했다.

다섯째, AI는 개념 이해 확인 및 작성 코드 검증 용도로 사용하였다. 우선 가급적 AI의 도움 없이 팀원과 코드를 작성하였고, 작성 중 오랫동안 막히면 힌트를 물어보거나 현재 작성 방향이 맞는지 검증하는 용도로 사용하였다. 또한 문서를 작성하고, 내가 이해한 흐름이 맞는지 확인하거나 실패 원인을 좁히는 데에도 사용하였다. 그래도 여전히 어려운 부분에서는 설명을 읽고 이해했다고 착각하기 쉬웠고, 앞으로도 직접 그림을 그리거나 함수 호출 순서를 적어 보는 과정이 필요하다고 느꼈다.


## 5. 다음 주 계획

13~14주차는 AI 과제로 넘어간다. 공지 기준으로 먼저 NumPy만 사용해 MNIST 필기체 숫자 분류기를 구현하고, 그 다음 PyTorch만 사용해 mini GPT LLM을 구현한다.

MNIST 과제에서는 `ReLU`, `Softmax`, `Affine`, loss, optimizer, BatchNorm, Dropout, training loop를 직접 구현해야 한다. PintOS와 다르게 이번에는 forward와 backward의 shape가 계속 중요할 것 같다. 테스트를 통과시키는 것만 보지 말고, 각 layer에서 입력과 출력, gradient shape가 어떻게 바뀌는지 먼저 정리하면서 진행하려고 한다.

mini GPT 과제에서는 BPE tokenizer, dataset, embedding, causal multi-head attention, transformer block, pretraining, generation, fine-tuning을 구현한다. 특히 BPE는 한국어를 byte 단위로 다뤄야 하고, attention은 미래 token을 보지 못하게 mask를 적용해야 한다. 처음부터 전체 모델을 완성하려 하기보다 `test_bpe.py`, `test_dataset.py`, `test_attention.py`처럼 작은 테스트부터 차례대로 보려고 한다.

이번 PintOS에서 배운 것은 큰 기능도 결국 작은 lifecycle과 shape, 상태 변화로 쪼개서 봐야 한다는 점이었다. 다음 주에는 신경망과 LLM도 같은 방식으로, 코드가 돌아간다는 결과보다 왜 그 계산이 필요한지 설명하는 데 더 신경 쓰려고 한다.


---

### 참고 링크
- [11-12주차 PintOS vm 협업 레포] https://github.com/Jungle-12-303/week11-team-03-pintos-vm