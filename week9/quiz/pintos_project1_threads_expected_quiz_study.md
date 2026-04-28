# PintOS Project 1 Threads 예상 퀴즈 학습

## 범위

- Alarm Clock
- Priority Scheduling
- Priority Donation
- Advanced Scheduler, MLFQS
- Fixed-Point Arithmetic

---

## 1. 핵심 개념 문제

### Q1. `timer_sleep()`의 기존 구현에서 문제가 되는 busy waiting이란 무엇인가?

**답변:**  
스레드를 block하지 않고, 조건이 만족될 때까지 CPU를 계속 사용하면서 반복 확인하는 방식이다. PintOS의 기존 `timer_sleep()`는 시간이 지났는지 계속 확인하며 `thread_yield()`를 반복 호출하므로 busy waiting에 해당한다.

---

### Q2. `timer_sleep(int64_t ticks)`는 스레드를 정확히 `ticks` 후에 깨워야 하는가?

**답변:**  
정확히 `ticks` 후에 깨울 필요는 없다. 최소 `ticks` 타이머 틱 이상 지난 뒤 ready queue에 넣으면 된다. 시스템이 idle하지 않다면 실제 실행은 더 늦어질 수 있다.

---

### Q3. PintOS에서 타이머 인터럽트는 기본적으로 초당 몇 번 발생하는가?

**답변:**  
기본값은 초당 100번이다. 즉 `TIMER_FREQ`의 기본값은 `100`이다.

---

### Q4. `thread_yield()`를 반복 호출하는 루프가 왜 좋은 구현이 아닌가?

**답변:**  
CPU를 낭비하기 때문이다. 스레드가 실제로 기다려야 하는 상황이라면 ready 상태로 계속 CPU를 양보하는 것이 아니라 blocked 상태가 되어야 한다.

---

### Q5. PintOS에서 인터럽트를 비활성화해서 보호해야 하는 대표적인 경우는 언제인가?

**답변:**  
커널 스레드와 인터럽트 핸들러가 같은 데이터를 공유할 때이다. 예를 들어 alarm clock에서 timer interrupt handler가 잠든 스레드 목록을 확인하고 깨우는 경우가 있다.

---

### Q6. 인터럽트 핸들러와 공유하는 자료구조를 보호할 때 락 대신 인터럽트 비활성화를 쓰는 이유는?

**답변:**  
인터럽트 핸들러는 sleep 또는 block될 수 없기 때문에 락을 획득할 수 없다. 따라서 커널 스레드 쪽에서 인터럽트를 잠깐 꺼서 인터럽트 핸들러의 간섭을 막아야 한다.

---

### Q7. PintOS의 우선순위 범위는 어떻게 되는가?

**답변:**  
`PRI_MIN = 0`, `PRI_DEFAULT = 31`, `PRI_MAX = 63`이다. 숫자가 클수록 높은 우선순위이다.

---

### Q8. ready list에 현재 실행 중인 스레드보다 높은 우선순위의 스레드가 들어오면 어떻게 해야 하는가?

**답변:**  
현재 실행 중인 스레드는 즉시 CPU를 양보해야 한다. 다음 타이머 인터럽트까지 기다리면 안 된다.

---

### Q9. 락, 세마포어, 조건 변수에서 대기 중인 스레드를 깨울 때 어떤 스레드를 먼저 깨워야 하는가?

**답변:**  
가장 높은 우선순위의 대기 스레드를 먼저 깨워야 한다.

---

### Q10. priority inversion이 무엇인지 H, M, L 스레드 예시로 설명하라.

**답변:**  
높은 우선순위 스레드 H가 낮은 우선순위 스레드 L이 가진 락을 기다리고 있다고 하자. 이때 중간 우선순위 스레드 M이 ready 상태이면, M이 계속 CPU를 가져가고 L은 실행되지 못한다. 결과적으로 H도 락을 얻지 못해 실행되지 못한다. 이렇게 낮은 우선순위 스레드 때문에 높은 우선순위 스레드가 간접적으로 막히는 현상이 priority inversion이다.

---

### Q11. priority donation은 어떤 문제를 해결하기 위한 기법인가?

**답변:**  
priority inversion을 완화하기 위한 기법이다. 높은 우선순위 스레드가 낮은 우선순위 스레드가 가진 락을 기다릴 때, 높은 우선순위를 락 보유자에게 임시로 기부하여 락 보유자가 먼저 실행되고 락을 빨리 해제하게 한다.

---

### Q12. priority donation은 락, 세마포어, 조건 변수 모두에 구현해야 하는가?

**답변:**  
아니다. priority donation은 락에 대해서만 구현하면 된다. 다만 우선순위 스케줄링 자체는 세마포어와 조건 변수에도 적용되어야 하므로, 대기자를 깨울 때는 가장 높은 우선순위 스레드를 먼저 깨워야 한다.

---

### Q13. 중첩 priority donation이 필요한 상황을 예로 들어라.

**답변:**  
H가 M이 가진 락을 기다리고, M은 다시 L이 가진 락을 기다리는 상황이다. 이때 H의 우선순위가 M에게 기부되고, 다시 L에게도 전파되어야 한다.

---

### Q14. 여러 스레드가 하나의 스레드에게 우선순위를 기부하면 effective priority는 어떻게 결정되는가?

**답변:**  
기본 우선순위와 모든 기부 우선순위 중 최댓값이 effective priority가 된다. priority donation은 더하는 방식이 아니다.

---

### Q15. `thread_set_priority()`는 priority donation을 받고 있는 스레드에서 어떤 의미를 가지는가?

**답변:**  
현재 스레드의 기본 우선순위를 바꾼다. 하지만 donation이 남아 있으면 실제 effective priority는 기부받은 값이 더 높을 수 있다. donation이 제거되면 새로 설정한 기본 우선순위가 다시 반영된다.

---

### Q16. `thread_get_priority()`는 기본 우선순위와 기부받은 우선순위 중 무엇을 반환해야 하는가?

**답변:**  
effective priority를 반환해야 한다. 즉 donation이 있다면 기본 우선순위보다 높은 기부 우선순위를 반환할 수 있다.

---

### Q17. 가장 높은 우선순위의 스레드가 `thread_yield()`를 호출했는데 같은 우선순위 스레드가 없다면 어떻게 되는가?

**답변:**  
계속 실행될 수 있다. 더 높은 우선순위나 같은 우선순위의 다른 ready 스레드가 없다면 다시 선택될 수 있기 때문이다.

---

### Q18. `-mlfqs` 옵션이 켜졌을 때 priority donation은 동작해야 하는가?

**답변:**  
아니다. MLFQS에서는 priority donation을 사용하지 않는다.

---

### Q19. `-mlfqs` 옵션이 켜졌을 때 `thread_create()`의 priority 인자는 어떻게 처리되는가?

**답변:**  
무시된다. MLFQS에서는 스케줄러가 공식에 따라 스레드 우선순위를 동적으로 계산한다.

---

### Q20. MLFQS에서 priority 계산 공식은 무엇인가?

**답변:**  

```text
priority = PRI_MAX - (recent_cpu / 4) - (nice * 2)
```

계산 결과는 `PRI_MIN`과 `PRI_MAX` 사이로 보정해야 한다.

---

### Q21. MLFQS에서 `recent_cpu`는 무엇을 의미하는가?

**답변:**  
스레드가 최근에 CPU를 얼마나 사용했는지를 나타내는 추정값이다. 최근에 CPU를 많이 사용한 스레드는 우선순위가 낮아지는 방향으로 계산된다.

---

### Q22. idle thread의 `recent_cpu`도 매 tick 증가하는가?

**답변:**  
아니다. 타이머 인터럽트마다 실행 중인 스레드의 `recent_cpu`를 증가시키지만, idle thread는 제외한다.

---

### Q23. `load_avg` 계산 공식은 무엇인가?

**답변:**  

```text
load_avg = (59 / 60) * load_avg + (1 / 60) * ready_threads
```

`load_avg`는 시스템 전체의 부하 평균이다.

---

### Q24. `ready_threads`에 idle thread를 포함하는가?

**답변:**  
포함하지 않는다. `ready_threads`는 실행 중이거나 실행 준비가 된 스레드 수이지만 idle thread는 제외한다.

---

### Q25. PintOS 커널에서 부동소수점 연산을 쓰지 않는 이유는 무엇인가?

**답변:**  
커널에서 부동소수점 연산을 지원하면 커널 구현이 복잡해지고 느려질 수 있기 때문이다. 그래서 `recent_cpu`와 `load_avg` 같은 실수 값은 정수 기반 고정소수점 방식으로 표현한다.

---

## 2. 계산형 예상 문제

### Q1. `PRI_MAX = 63`, `recent_cpu = 20`, `nice = 0`일 때 priority는?

**풀이:**  

```text
priority = 63 - (20 / 4) - (0 * 2)
         = 63 - 5
         = 58
```

**답:** `58`

---

### Q2. `recent_cpu = 16`, `nice = 5`일 때 priority는?

**풀이:**  

```text
priority = 63 - (16 / 4) - (5 * 2)
         = 63 - 4 - 10
         = 49
```

**답:** `49`

---

### Q3. `recent_cpu = 0`, `nice = -5`일 때 priority는?

**풀이:**  

```text
priority = 63 - (0 / 4) - (-5 * 2)
         = 63 + 10
         = 73
```

하지만 priority는 `PRI_MAX = 63`을 넘을 수 없으므로 보정한다.

**답:** `63`

---

### Q4. `load_avg = 0`, `ready_threads = 3`일 때 다음 `load_avg`는?

**풀이:**  

```text
load_avg = (59 / 60) * 0 + (1 / 60) * 3
         = 3 / 60
         = 0.05
```

**답:** `0.05`

---

### Q5. `TIMER_FREQ = 100`일 때 `load_avg`와 모든 스레드의 `recent_cpu`는 몇 tick마다 갱신되는가?

**답변:**  
100 tick마다 갱신된다. 즉 1초마다 갱신된다.

---

### Q6. MLFQS에서 priority는 몇 tick마다 다시 계산되는가?

**답변:**  
모든 스레드의 priority는 매 4번째 tick마다 다시 계산된다.

---

### Q7. 고정소수점에서 `q = 14`라면 scale factor `f`는 얼마인가?

**풀이:**  

```text
f = 1 << 14 = 16384
```

**답:** `16384`

---

### Q8. 17.14 고정소수점에서 정수 `3`을 고정소수점 값으로 바꾸면?

**풀이:**  

```text
3 * 16384 = 49152
```

**답:** `49152`

---

## 3. 시험 직전 암기 포인트

- `timer_sleep()`는 busy waiting 없이 구현해야 한다.
- 잠든 스레드는 blocked 상태가 되어야 한다.
- 타이머 인터럽트 기본값은 100Hz이다.
- `TIMER_FREQ` 값은 바꾸지 않는 것이 원칙이다.
- priority 값은 클수록 높다.
- `PRI_MIN = 0`, `PRI_DEFAULT = 31`, `PRI_MAX = 63`이다.
- 높은 우선순위 스레드가 ready 상태가 되면 즉시 선점해야 한다.
- semaphore, lock, condition variable의 waiters는 우선순위 기준으로 깨워야 한다.
- priority donation은 lock에 대해서만 필요하다.
- donation은 더하는 것이 아니라 최댓값을 반영하는 것이다.
- nested donation을 처리해야 한다.
- `thread_set_priority()`는 기본 우선순위를 바꾼다.
- `thread_get_priority()`는 effective priority를 반환한다.
- MLFQS에서는 priority donation을 사용하지 않는다.
- MLFQS에서는 `thread_create()`의 priority 인자를 무시한다.
- MLFQS priority 공식은 `PRI_MAX - recent_cpu / 4 - nice * 2`이다.
- `recent_cpu`는 최근 CPU 사용량의 추정값이다.
- idle thread의 `recent_cpu`는 증가시키지 않는다.
- `load_avg`는 시스템 전체 부하 평균이다.
- `ready_threads`에는 idle thread를 포함하지 않는다.
- `load_avg`와 모든 스레드의 `recent_cpu`는 1초마다 갱신한다.
- 모든 스레드의 priority는 4 tick마다 갱신한다.
- PintOS 커널에서는 부동소수점 연산 대신 고정소수점 연산을 쓴다.
- 고정소수점 곱셈과 나눗셈에서는 overflow 방지를 위해 64비트 연산을 사용한다.

---

## 4. 자주 헷갈리는 포인트

### Busy waiting vs Blocking

busy waiting은 스레드가 계속 CPU를 사용하면서 기다리는 방식이다. blocking은 스레드를 대기 상태로 바꾸고 CPU를 다른 스레드에게 넘기는 방식이다. `timer_sleep()`는 blocking 방식으로 구현해야 한다.

### Priority Scheduling vs MLFQS

일반 priority scheduling에서는 사용자가 지정한 priority와 priority donation을 고려한다. MLFQS에서는 스케줄러가 `nice`, `recent_cpu`, `load_avg`를 이용해 priority를 직접 계산하며 priority donation은 하지 않는다.

### Base Priority vs Effective Priority

base priority는 스레드가 원래 가진 우선순위이다. effective priority는 donation까지 반영한 실제 스케줄링 우선순위이다.

### `recent_cpu`가 음수가 될 수 있는가?

가능하다. nice 값이 음수인 경우 `recent_cpu`가 음수가 될 수 있으며, 이를 0으로 고정하면 안 된다.

### 인터럽트는 오래 꺼도 되는가?

안 된다. 인터럽트 비활성화 구간은 가능한 짧아야 한다. 너무 오래 끄면 타이머 틱이나 입력 이벤트 처리가 지연될 수 있다.
