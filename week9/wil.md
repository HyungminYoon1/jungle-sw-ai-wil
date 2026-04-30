# Week 9 - WIL (What I Learned)

> 작성 시점: 주차 중~말

매주 진행한 작업과 배운 내용을 정리합니다. 단순 일지보다는 **학습 포인트 / 문제 해결 과정 / 개선 아이디어**를 중심으로 작성합니다.

---

## 1. 이번 주 목표
- PintOS `Project 1 - Threads`의 실행 흐름을 따라가며 커널 스레드가 생성되고, 준비 큐에 들어가고, 스케줄러에 의해 실행되는 과정을 이해하기
- `timer_sleep()`의 busy waiting을 제거하고, block 상태의 스레드를 timer interrupt에서 다시 ready 상태로 깨우는 alarm clock 구조 구현하기
- ready 상태의 스레드가 priority 기준으로 선택되도록 `ready_list` 정렬과 preemption 조건을 구현하기
- semaphore waiters도 priority 기준으로 깨우도록 바꿔 `priority-sema`까지 통과시키기
- `priority-condvar`, `priority donation`, `MLFQS`까지 Project 1 Threads의 전체 테스트 범위를 이해하고, 구현 우선순위와 실패 시 확인할 지점을 정리하기
- 팀 repository, GitHub Projects, 개인 branch, PR 리뷰 흐름을 사용해 팀 최종 코드베이스를 관리하기

## 2. 시도한 접근 방식
- 처음에는 KAIST PintOS GitBook의 Threads 프로젝트 문서(`Alarm Clock`, `Priority Scheduling`, `Advanced Scheduler`, FAQ)를 보면서 `thread.c`, `timer.c`, `synch.c`, `interrupt.c`의 역할을 나눠 읽었다. PintOS는 함수 이름만 봐서는 전체 흐름이 잘 잡히지 않아서, 부팅 이후 스레드 초기화, timer interrupt, ready list, semaphore 대기열이 어디서 만나는지부터 확인했다.
- alarm clock은 `timer_sleep()`에서 직접 반복문을 돌며 기다리는 방식이 아니라, 현재 스레드에 깨워야 할 tick을 기록하고 sleep list에 넣은 뒤 `thread_block()`으로 멈추는 방향으로 접근했다. 이후 timer interrupt가 발생할 때마다 현재 tick 이하로 깨워야 할 스레드를 찾아 `thread_unblock()`으로 ready 상태에 올리는 흐름을 만들었다.
- priority scheduling은 ready list를 항상 priority 내림차순으로 유지하는 방식으로 구현했다. `thread_unblock()`과 `thread_yield()`에서 `list_push_back()` 대신 `list_insert_ordered()`를 사용했고, 같은 priority에서는 FIFO 순서를 유지하기 위해 비교 함수가 `>` 기준으로만 true를 반환하도록 했다.
- preemption은 새 스레드가 생성되거나 현재 스레드의 priority가 낮아졌을 때 ready list의 가장 앞 스레드와 비교해 현재 스레드가 CPU를 양보하도록 처리했다. 이 과정에서 ready list가 비어 있는 경우를 먼저 확인하지 않으면 `list_front()` 호출로 kernel panic이 날 수 있다는 점도 확인했다.
- semaphore는 `sema_down()`에서 waiters에 들어가는 스레드를 priority 기준으로 정렬하고, `sema_up()`에서 가장 높은 priority waiter를 깨운 뒤 필요하면 yield하도록 했다. `sema_up()`은 일반 thread context뿐 아니라 interrupt context에서도 호출될 수 있으므로 `thread_yield()`와 `intr_yield_on_return()`을 구분했다.
- 팀 작업은 `main <- dev <- 개인 브랜치` 흐름으로 진행했다. 팀 repository와 GitHub Projects를 사용했고, 브랜치 보호 정책과 PR 흐름을 정리해 실수로 main이나 dev에 직접 반영되는 일을 줄이려 했다.
- AI는 PintOS 핵심 코드를 대신 작성하는 용도보다 코드 구조 파악, GitHub Projects 이슈 분류, 개념 확인, 발표 자료 문장 정리에 보조적으로 활용했다. 최종 코드와 테스트 결과는 직접 확인하는 기준으로 두었다.

## 3. 문제와 해결 과정

### busy waiting을 제거한다는 말이 구체적으로 무엇인지 헷갈렸던 점
- **문제:** 처음에 `timer_sleep()`에서 while loop를 제거해야 busy waiting을 제거한다는 내용은 힌트를 얻었는데 구체적으로 어떻게 코드로 구현해야 할지 막막했다.
- **해결:** 동료로부터 도움을 받았다. 잠들어야 하는 스레드를 `sleep_list`에 넣고 `thread_block()`으로 CPU에서 내리는 방식으로 바꿨다. 각 스레드에는 `wakeup_tick`을 저장했고, `timer_interrupt()`에서 현재 tick 이하의 스레드를 깨워 `ready_list`로 다시 올리도록 했다. 이 구조로 `alarm-single`, `alarm-multiple`, `alarm-simultaneous`, `alarm-zero`, `alarm-negative`를 통과했고, 이후 ready list 우선순위 정렬과 preemption 구현을 함께 적용한 뒤 `alarm-priority`까지 통과했다.

### ready list만 고치면 priority scheduling이 끝난다고 생각했던 점
- **문제:** 처음에는 ready list만 priority 순서로 정렬하면 priority scheduling이 될 것이라고 생각했다. 하지만 `priority-preempt`, `priority-change`, `priority-fifo`, `priority-sema`를 확인하면서 스레드가 ready 상태가 되는 경로가 여러 곳이라는 점을 알게 되었다. `thread_create()` 이후 새 스레드가 현재 스레드보다 priority가 높으면 즉시 yield해야 했고, `thread_set_priority()`로 현재 스레드의 priority를 낮춘 뒤에도 더 높은 ready thread가 있으면 CPU를 양보해야 했다. 또한 ready list가 비어 있는데 `list_front()`를 호출해 kernel panic이 발생하는 문제도 있었다.
- **해결:** `thread_unblock()`과 `thread_yield()`에서 `ready_list`를 priority 순서로 정렬 삽입하도록 바꿨고, 같은 priority에서는 FIFO가 유지되도록 비교 함수를 `>` 기준으로 작성했다. `thread_create()`와 `thread_set_priority()`에는 선점 판단을 추가했고, `list_front()`를 호출하기 전에는 `list_empty()`를 먼저 확인하도록 수정했다. 이 과정을 통해 `priority-preempt`, `priority-change`, `priority-fifo`를 통과했다.

### 같은 priority에서 FIFO 순서가 깨질 수 있었던 점
- **문제:** priority가 높은 스레드를 앞에 넣으려고 비교 함수를 단순히 `>=` 기준으로 작성하면, 같은 priority를 가진 새 스레드가 기존 스레드보다 앞에 들어갈 수 있다. 이 경우 priority는 맞지만 FIFO 순서는 깨진다.
- **해결:** 비교 함수는 `a->priority > b->priority`일 때만 true를 반환하도록 했다. 같은 priority에서는 새 원소를 기존 원소 뒤쪽에 두어 기존 순서를 보존했다. 

### semaphore waiters도 priority 기준으로 처리해야 했던 점
- **문제:** ready list는 priority 정렬을 적용했지만, semaphore waiters가 FIFO로 남아 있으면 `priority-sema`에서 높은 priority waiter가 먼저 깨어난다는 보장이 없었다. 또한 처음에는 `list_pop_front()`로 waiter를 꺼낸 뒤 다시 `list_front()`를 보려고 해서, 실제로 깨운 thread를 잃어버릴 위험이 있었다.
- **해결:** `sema_down()`에서 waiters에 현재 스레드를 넣을 때 priority 순서로 삽입했다. `sema_up()`에서는 가장 앞의 waiter를 꺼내 `woken_t` 변수에 저장한 뒤 `thread_unblock()`하고, `woken_t->priority`와 현재 thread priority를 비교해 yield 필요 여부를 판단했다. 이때 `sema_up()`은 interrupt handler 안에서도 호출될 수 있으므로 일반 thread context에서는 `thread_yield()`, interrupt context에서는 `intr_yield_on_return()`을 사용하도록 구분했다.

### condition variable과 donation을 완료하지 못한 점
- **문제:** `priority-condvar`는 condition variable의 waiters가 단순 FIFO 순서로 깨워져 실패했다. 또한 lock holder에게 높은 priority를 빌려주는 priority donation 구조가 없어서 `priority-donate-*` 테스트들이 실패했다.
- **해결하지 못한 범위:** 이번 주 코드에는 `lock_acquire()` 전에 donation을 전파하거나, `lock_release()` 후 해당 lock 관련 donation만 회수하는 로직을 구현하지 못했다. `struct thread`에 base priority와 waiting lock 정보를 추가하고, lock holder에게 들어온 donation을 추적하는 구조도 설계해야 한다.

### MLFQS를 손대지 못한 점
- **문제:** MLFQS는 `nice`, `recent_cpu`, `load_avg`, fixed-point arithmetic, 전체 thread 순회, 갱신 주기를 모두 맞춰야 하는 작업이었다. 하지만 alarm과 기본 priority scheduling을 잡는 데 시간을 많이 사용해 MLFQS 구현까지 진행하지 못했다.
- **해결하지 못한 범위:** 현재 `thread_set_nice()`, `thread_get_nice()`, `thread_get_load_avg()`, `thread_get_recent_cpu()`는 구현되지 않은 상태로 남아 있다. MLFQS는 손을 대지 못하였고, `mlfqs-*` 9개 테스트는 미완료 범위로 남았다.

## 4. 새로 배운 점
- busy waiting을 제거한다는 것은 단순히 기다리는 코드를 다른 위치로 옮기는 것이 아니라, 기다리는 동안 CPU를 점유하지 않도록 스레드를 blocked 상태로 전환하는 것이다.
- timer interrupt는 단순히 시간을 증가시키는 역할만 하는 것이 아니라, 잠든 스레드를 깨우고 time slice 만료 시 선점을 예약하는 스케줄링의 중요한 진입점이다.
- PintOS의 list element는 하나의 리스트에만 들어갈 수 있으므로, 같은 `elem`을 여러 리스트에서 동시에 쓰면 안 된다. 스레드 상태와 list 소속 관계를 함께 생각해야 한다.
- priority scheduling은 ready list만 정렬한다고 끝나지 않는다. 스레드가 ready 상태가 되는 모든 경로와, 높은 priority 스레드가 등장했을 때 현재 스레드가 양보해야 하는 시점을 함께 봐야 한다.
- 같은 priority에서 FIFO를 유지하는 것은 사소한 구현 세부사항처럼 보이지만 테스트 결과를 바꾸는 정책이다. 비교 함수의 `>`와 `>=` 차이가 실행 순서를 바꿀 수 있다.
- semaphore waiters도 scheduler 정책과 연결된다. 동기화 primitive의 대기열이 FIFO로 남아 있으면 ready list가 priority 정렬되어 있어도 전체 실행 순서는 priority scheduling과 어긋날 수 있다.
- 일반 thread context와 interrupt context는 yield 처리 방식이 다르다. interrupt handler 안에서는 직접 `thread_yield()`를 호출할 수 없고, interrupt가 끝난 뒤 yield되도록 예약해야 한다.
- priority inversion은 개념적으로 이해했지만 이번 주 코드로 해결하지 못했다. 높은 priority thread가 낮은 priority lock holder를 기다릴 때 holder에게 priority를 임시로 빌려주는 donation 구조가 필요하다.
- MLFQS는 단순한 priority 정렬 문제가 아니라 scheduler가 주기적으로 priority를 계산하는 정책이다. fixed-point 계산과 갱신 시점이 테스트 결과에 직접 영향을 줄 수 있다.
- 팀 프로젝트에서는 테스트 통과 여부뿐 아니라 어떤 테스트가 왜 통과했고, 무엇이 아직 실패하는지를 명확히 남기는 것이 중요하다. 이번 주에는 10개 테스트를 통과했고 17개 테스트는 미완료로 남았다.

## 5. 다음 주 계획
- 다음 주는 PintOS `Project 2 - User Programs`를 진행한다. 공지 기준으로 Project 2는 옵션이 아니라 모두 필수 구현 사항이며, 1주 안에 테스트 케이스 통과를 목표로 한다.
- 금요일 오전 10시에는 PintOS 기초 특강 - User Program에 참석하고, 첫날 자정 전까지 팀 GitHub Projects 주소를 제출해야 한다. 화요일에는 PintOS와 C언어 범위 퀴즈가 있고, 목요일 오전 10시에는 주간 공유 발표가 있다.
- 학습 키워드는 `User Program`, `User mode`, `Kernel Mode`, `System Call`, `Virtual Memory Layout`로 잡는다. 특히 ELF 로더, system call 진입 흐름, file descriptor, 유저-커널 주소 검증을 중점적으로 볼 계획이다.
- 구현은 사용자 프로그램 실행 흐름부터 단계적으로 확인한다. 먼저 argument passing 관련 `args-none`, `args-single`, `args-multiple`, `args-many`, `args-dbl-space`를 보고, 이후 `halt`, `exit`처럼 작은 syscall부터 시작한다.
- 파일 관련 syscall은 `create`, `open`, `close`, `read`, `write` 테스트 묶음으로 나눠 진행한다. 정상 케이스뿐 아니라 null pointer, bad pointer, boundary, bad fd, stdin/stdout 같은 예외 조건을 함께 확인할 계획이다.
- 프로세스 관련 구현은 `fork`, `exec`, `wait` 순서로 접근한다. `fork-once`, `fork-multiple`, `exec-once`, `exec-arg`, `wait-simple`처럼 기본 흐름을 먼저 잡고, 이후 bad pointer, missing executable, killed child, recursive fork 같은 실패 조건을 다룬다.
- `rox-simple`, `rox-child`, `rox-multichild`, `multi-recurse`, `multi-child-fd`, `multi-oom`처럼 여러 프로세스와 파일이 엮이는 테스트는 앞선 syscall 구현이 안정된 뒤 회귀 테스트로 확인한다.
- 9주차에서 미완료로 남은 `priority-condvar`, `priority donation`, `MLFQS`도 CSV에 다시 포함되어 있으므로, Project 2 진행 중에도 threads 회귀가 필요한지 확인한다. 다만 10주차의 우선순위는 User Programs 필수 구현과 제출물 완료에 둔다.
- 목요일 정오 전까지 팀 주간 공유 발표 자료와 WIL을 준비해야 하므로, 구현 중 막힌 테스트는 실패 로그, 의심 함수, 수정 가설, 재실행 결과를 바로 기록할 계획이다.

---

### 참고 링크
- KAIST PintOS Assignment: https://casys-kaist.github.io/pintos-kaist/
- [9주차 PintOS threads 협업 레포] https://github.com/Jungle-12-303/week09-team-07-pintos-threads

