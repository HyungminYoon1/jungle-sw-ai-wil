# Stack Growth

이 문서는 Project 3의 stack growth를 설명한다. stack growth는 page fault 중에서도 "정상적인 stack 접근"으로 볼 수 있는 경우에 새 stack page를 만들어 주는 기능이다.

## 1. Project 2의 stack 한계

Project 2에서는 보통 stack page 하나를 만들어 `USER_STACK` 아래에 배치했다.

```text
USER_STACK
  |
  v
+------------------+
| one stack page   |
+------------------+
```

하지만 실제 프로그램은 함수 호출, 지역 변수, 인자 전달 등으로 stack을 계속 사용한다. 큰 지역 배열을 만들거나 깊은 재귀를 하면 stack이 한 page보다 커질 수 있다.

Project 3에서는 stack이 더 필요할 때 page fault를 이용해 stack을 늘린다.

## 2. Stack은 어느 방향으로 자라는가

x86-64에서 stack은 높은 주소에서 낮은 주소 방향으로 자란다.

```text
높은 주소
USER_STACK
  |
  | push, call, local variables
  v
낮은 주소
```

따라서 stack growth는 보통 `USER_STACK`보다 아래쪽 주소에 새 page를 추가하는 일이다.

## 3. 모든 아래쪽 fault를 stack growth로 보면 안 된다

사용자 프로그램이 아무 주소나 접근했는데 커널이 stack이라고 믿고 page를 만들어 주면 안 된다.

예를 들어 `0x1234` 같은 낮은 주소에 쓰기를 시도했을 때 stack page를 만들어 주면 잘못된 포인터 버그를 정상 접근처럼 숨기게 된다.

그래서 stack growth에는 heuristic, 즉 판단 기준이 필요하다.

## 4. Stack 접근으로 볼 수 있는 기준

일반적으로 다음 조건들을 함께 본다.

- fault address가 user virtual address인지
- fault address가 `USER_STACK`보다 아래인지
- fault address가 stack 최대 크기 제한 안에 있는지
- fault address가 현재 user `rsp` 근처인지
- fault가 not-present fault인지

KAIST Pintos에서는 stack 최대 크기를 1MB로 제한해야 한다.

```text
USER_STACK - 1MB <= fault_addr < USER_STACK
```

## 5. 왜 `rsp` 근처인지 보아야 하는가

stack은 현재 stack pointer 근처에서 자라는 것이 정상이다.

예를 들어 현재 `rsp`가 `0x7fffffffe000` 근처인데 갑자기 `0x400000`에 접근했다면, 이것은 stack growth라기보다 잘못된 포인터일 가능성이 높다.

PUSH 명령은 stack pointer를 조정하기 전에 접근 권한을 검사할 수 있으므로, `rsp - 8` 근처에서 fault가 날 수 있다. 그래서 fault address가 정확히 `rsp` 이상이어야 한다고 너무 엄격하게 잡으면 정상 push를 막을 수 있다.

흔히 다음과 비슷한 기준을 둔다.

```text
fault_addr >= rsp - 8
```

이 조건은 `rsp - 8`보다 크거나 같은 주소를 허용하므로, `rsp`보다 위쪽 주소에서 난 fault도 통과할 수 있다. GitBook FAQ의 "stack pointer 위쪽 fault"는 이 지점을 말한다. 중요한 것은 "무조건 아래쪽만 허용"이 아니라, 현재 user `rsp` 주변의 stack 접근으로 볼 수 있는지 판단하는 것이다.

팀 구현에 따라 syscall 인자 접근 등도 고려해 기준을 조정해야 한다.

## 6. User `rsp`를 어디서 얻는가

사용자 모드에서 page fault가 발생하면 `page_fault()`에 전달된 `struct intr_frame`의 `rsp`를 사용할 수 있다.

하지만 커널 모드에서 사용자 포인터를 검증하다 page fault가 나는 경우에는 `intr_frame->rsp`가 사용자 stack pointer가 아닐 수 있다. 프로세서는 user mode에서 kernel mode로 넘어올 때만 user `rsp`를 저장하기 때문이다.

그래서 syscall 진입 시점의 user `rsp`를 `struct thread`에 저장해 두는 방식이 자주 쓰인다.

```text
syscall_handler(f)
  -> thread_current()->user_rsp = f->rsp

kernel mode page fault
  -> saved user_rsp 사용
```

## 7. `vm_try_handle_fault()`에서 할 일

stack growth 판단은 page fault 처리 중에 이루어진다.

흐름은 다음과 같다.

```text
vm_try_handle_fault(f, addr, user, write, not_present)
  -> addr 기본 검증
  -> SPT에서 page 찾기
  -> page가 있으면 claim
  -> page가 없으면 stack growth 후보인지 검사
  -> 후보면 vm_stack_growth(addr)
  -> 아니면 false
```

즉 SPT에 page가 없는 모든 fault를 stack growth로 보내면 안 된다. stack 기준을 통과해야 한다.

## 8. `vm_stack_growth()`에서 할 일

`vm_stack_growth()`는 fault address가 더 이상 fault를 내지 않도록 page를 만든다.

기본 흐름은 다음과 같다.

```text
1. addr을 pg_round_down(addr)로 page boundary에 맞춤
2. VM_ANON page를 writable로 할당
3. SPT에 삽입
4. vm_claim_page()로 즉시 frame 연결
```

GitBook은 하나 이상의 anonymous page를 할당할 수 있다고 설명한다. 단순 구현에서는 fault가 난 page 하나만 할당해도 많은 테스트를 시작할 수 있다. 다만 큰 stack jump를 처리해야 하는지 테스트 요구를 보며 판단해야 한다.

## 9. Stack marker

`vm_type`에는 보조 marker를 붙일 수 있다. stack page를 구분하고 싶다면 `VM_MARKER_0` 같은 marker를 활용할 수 있다.

예를 들어 anonymous page이면서 stack page임을 표시하면, 나중에 fork나 cleanup에서 의미를 더 명확히 할 수 있다.

단 marker는 설계 선택이다. marker 없이도 주소 범위로 stack을 판단할 수 있다.

## 10. 흔한 실수

### `addr`을 round down하지 않고 page를 만드는 경우

fault address가 `0x7fffffabc123`이면 실제 page 시작은 `0x7fffffabc000`이다. round down하지 않으면 SPT key와 pml4 mapping이 어긋난다.

### NULL 근처 주소를 stack으로 허용하는 경우

잘못된 포인터 접근을 stack growth로 복구해 버릴 수 있다.

### 1MB 제한을 빼먹는 경우

테스트는 비정상적으로 큰 stack growth를 거부하는지 확인할 수 있다.

### 커널 모드 fault에서 잘못된 `rsp`를 쓰는 경우

syscall 처리 중 user pointer 접근으로 fault가 나는 경우를 고려하지 않으면 stack 관련 syscall 테스트가 실패할 수 있다.

### Read-only 또는 present fault를 stack growth로 처리하는 경우

stack growth는 보통 not-present fault를 복구하는 기능이다. 권한 위반을 새 page 할당으로 숨기면 안 된다.

## 11. 테스트 관점

stack growth 관련 테스트는 대략 다음을 본다.

| 테스트 | 확인하는 것 |
|------|------|
| `pt-grow-stack` | 일반적인 stack 확장 |
| `pt-grow-stk-sc` | syscall 경로에서 stack 확장 |
| `pt-big-stk-obj` | 큰 stack object |
| `pt-grow-bad` | stack으로 볼 수 없는 주소 거부 |

실패하면 fault address, saved user rsp, SPT lookup 결과, stack limit 계산을 같이 봐야 한다.

## 12. 확인 질문

1. stack은 높은 주소와 낮은 주소 중 어느 방향으로 자라는가?
2. 모든 SPT miss를 stack growth로 처리하면 왜 위험한가?
3. `rsp - 8` 근처 fault를 고려해야 하는 이유는 무엇인가?
4. kernel mode page fault에서 user `rsp`를 따로 저장해야 할 수 있는 이유는 무엇인가?
5. stack 최대 크기 1MB 제한은 어떤 잘못된 접근을 막는가?
