# Week 10 - WIL (What I Learned)

> 작성 시점: 주차 마무리 시점

PintOS Project 2 - User Programs를 진행하며 사용자 프로그램 실행, system call, process lifecycle, file descriptor, user/kernel boundary를 구현하고 학습했다.

---

## 1. 이번 주 목표

이번 주 목표는 PintOS에서 사용자 프로그램이 커널 위에서 안전하게 실행되도록 Project 2의 핵심 기능을 구현하는 것이었다.

구체적으로는 `argument passing`, `halt/exit`, system call dispatcher, user pointer validation, file descriptor table, file syscall, `fork/exec/wait`, 실행 파일 write deny를 구현하고 테스트로 검증하는 것을 목표로 삼았다. 최종적으로 `pintos/userprog/build/tests/userprog/**/*.result` 기준 64개 테스트를 `PASS`하였다.(`no-vm/multi-oom.result` 포함)


## 2. 시도한 접근 방식

초반 3일 동안은 본인을 포함한 팀원 3명이 함께 `argument passing` 코드를 페어 프로그래밍으로 작성했다. 처음에는 코치님의 조언에 따라 user program이 처음 실행될 때 stack과 register가 어떻게 준비되어야 하는지를 팀 전체가 함께 이해하고 이를 코드로 구현하였다.

이후에는 업무를 A, B, C 세 파트로 나누었다.

| 파트 | 담당 범위 |
|------|-----------|
| A | fd table, file syscall, file system lock |
| B | `exec`, `fork`, `wait`, `rox` |
| C | user memory validation, bad pointer, boundary, robustness |

본인은 B 파트를 맡았다. `exec`에서는 현재 process image를 새 실행 파일로 교체하는 흐름, `fork`에서는 부모의 실행 문맥과 address space를 자식에게 복제하는 흐름, `wait`에서는 부모가 자식의 종료 상태를 회수하는 흐름, `rox`에서는 실행 중인 executable file에 대한 write를 막는 흐름을 구현했다.


## 3. 문제와 해결 과정

가장 어려웠던 부분은 `fork/exec/wait`가 단일 함수 구현이 아니라 process lifecycle 전체와 연결된다는 점이었다. `wait()`는 자식을 찾고 기다리는 함수처럼 보이지만, 실제로는 자식 생성 시점부터 `child_status`를 만들고 부모와 자식이 같은 상태를 공유해야 했다. 자식이 먼저 종료되는 경우, 부모가 먼저 wait하는 경우, 부모가 wait하지 않고 종료하는 경우를 모두 처리해야 했기 때문에 단순한 반환값 처리로는 해결되지 않았다.

이를 위해 `child_status`에 `tid`, `exit_status`, `waited`, `load_success`, reference count, `load_sema`, `exit_sema`를 두고 부모와 자식이 같은 구조체를 공유하도록 했다. `fork`에서는 부모의 `intr_frame`을 복사하되 자식의 `rax`만 0으로 바꾸어 부모와 자식의 반환값을 다르게 만들었다. `exec`에서는 사용자 문자열을 커널 영역으로 복사한 뒤 `process_exec()`에 넘겨 주소 공간 교체 중 사용자 포인터가 사라지는 문제를 피했다. `rox`에서는 `load()` 성공 후 실행 파일을 닫지 않고 thread에 보관하며 `file_deny_write()`를 적용했고, process 종료 시 `file_allow_write()`와 `file_close()`로 정리했다.

다만 이 과정에서 코드 구현에 어려움이 커서 AI의 도움을 받아 코드를 작성했다. 이후 AI가 작성한 코드를 읽고 표면적으로 흐름을 훑기는 했지만, 테스트가 통과한다는 사실에 기대어 깊이 있는 이해까지 도달하지 못했다. 특히 `child_status`의 reference count가 어느 시점에 줄어드는지, `fork_aux`가 왜 필요한지, fd 복제가 `fork-read`와 `fork-close`에 어떤 영향을 주는지, `exec`와 rox의 파일 lifecycle이 어떻게 맞물리는지를 스스로 설명하는 데 부족함이 있었다.

또 하나 아쉬웠던 점은 업무 분장 전에 공통적으로 적용될 모듈을 GPT에게 구현하도록 지시한 부분이다. 그 결과 실제로 필요한 최소 공통 모듈보다 많은 helper와 interface가 생성되었고, 이후 A/B/C 파트 구현은 사실상 GPT가 만든 핵심 스켈레톤 위에서 진행되었다. 속도 면에서는 도움이 되었지만, 팀이 공통 모듈의 책임 범위와 필요성을 충분히 합의하고 검증하기 전에 구조가 먼저 생겨 버렸다. 다음 주에는 공통 모듈을 만들기 전에 팀이 먼저 "어떤 공통 함수가 꼭 필요한지", "그 함수는 누가 만들고 누가 사용할지", "어디까지 처리하고 어디서부터 각자 구현할지"를 정한 뒤 AI를 설계 검토와 누락 위험 확인에 더 제한적으로 사용해야겠다고 느꼈다.

이번 주의 가장 큰 반성은 "테스트 통과"와 "설명 가능한 이해"가 같지 않다는 점이다. AI를 활용하더라도 구현 전 설계 가정, 구현 후 코드 흐름, 실패 시나리오, 자원 정리 책임을 직접 말로 설명하지 못하면 내 코드라고 보기 어렵다.


## 4. 새로 배운 점

첫째, user program 실행은 단순히 실행 파일을 여는 일이 아니라 ELF segment를 적재하고, user stack을 만들고, `_start(argc, argv)`가 호출될 수 있도록 `RDI`, `RSI`, `RSP`를 맞추는 일이다. `argument passing`을 페어 프로그래밍으로 같이 구현하면서 stack에 문자열, 주소 배열, 정렬 padding, fake return address가 어떤 순서로 들어가는지 확인했다.

둘째, system call은 user mode와 kernel mode 사이의 경계다. 사용자 프로그램이 넘긴 포인터를 그대로 믿으면 커널이 잘못된 user address 때문에 page fault를 내거나 panic이 날 수 있다. 그래서 syscall마다 문자열, read buffer, write buffer를 구분해 검증해야 한다.

셋째, `fork`, `exec`, `wait`는 각각 독립 기능이 아니라 process lifecycle을 이루는 한 묶음이다. `fork`는 실행 문맥과 자원을 복제하고, `exec`는 현재 실행 이미지를 교체하며, `wait`는 부모가 자식의 종료 상태를 한 번만 회수하게 한다.

넷째, `rox-*` 테스트는 실행 중인 파일을 수정하지 못하게 하는 운영체제의 안정성 요구를 검증한다. Project 2에서는 즉시 모든 파일 시스템 보안 모델을 구현하는 것은 아니지만, 실행 파일에 대한 write deny는 이후 lazy loading과 virtual memory에서도 중요한 전제가 된다.

다섯째, AI는 구현 속도를 높여줄 수 있지만 이해를 대신하지 않는다. AI가 만든 코드가 맞아 보여도 reference count, semaphore, file close 위치처럼 작은 lifecycle 실수는 테스트 통과 여부와 별개로 반드시 직접 추적해야 한다.


## 5. 다음 주 계획

11~12주차는 PintOS Project 3 - Virtual Memory를 진행한다. 공지와 `week11-12_issues_complete.csv` 기준으로 핵심 학습 주제는 Virtual Memory, Page Table, Swap in/out, Page Replacement이며, 구현 범위는 lazy loading, supplemental page table, page fault handling, stack growth, mmap, swap 계열 테스트까지 확장된다.

우선 KAIST GitBook Project 3 Virtual Memory와 FAQ, Appendix의 Memory Allocation, Virtual Address, Page Table을 먼저 읽고, 이번 주에 부족했던 "코드가 왜 그렇게 동작하는지 설명하는 습관"을 보완할 계획이다. 구현에 들어가기 전에는 SPT가 어떤 정보를 소유하는지, page fault handler가 어떤 조건에서 lazy load와 stack growth를 구분하는지, mmap page와 anonymous page의 lifecycle이 어떻게 다른지를 먼저 문서화한다.

AI를 사용할 때도 방식을 바꾼다. 바로 코드를 받기보다 먼저 설계 검토, 실패 로그 해석, 코드 리뷰 체크리스트 작성에 사용하고, 코드 제안을 받더라도 병합 전에는 내가 직접 함수 단위 흐름과 자원 정리 시점을 설명할 수 있는지 확인한다.

Virtual Memory를 일찍 끝낼 수 있다면 Project 1~3 내용을 복습하고 포트폴리오와 블로그에 정리하며, 팀원들과 모의 면접 또는 Project 4 진행 여부를 논의한다.


---

### 참고 링크
- KAIST PintOS Assignment: https://casys-kaist.github.io/pintos-kaist/
- [9주차 PintOS threads 협업 레포] https://github.com/Jungle-12-303/week09-team-07-pintos-threads