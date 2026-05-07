# User Programs: exec()와 현재 Process Image 교체

## 1. exec()의 의미

KAIST Pintos의 `exec(cmd_line)`은 새 child process를 만드는 syscall이 아니다. 현재 process의 user image를 `cmd_line`이 가리키는 executable로 교체한다.

```c
exec ("child-simple arg1");
```

성공하면 이전 프로그램으로 돌아오지 않는다. 실패하면 현재 process가 `exit(-1)`로 종료되어야 한다.

```text
load 성공 -> 새 프로그램으로 user mode 진입, exec()는 반환하지 않음
load 실패 -> 현재 process exit(-1)
```

## 2. process_exec()와 syscall exec() 구분

이름이 비슷해서 헷갈리기 쉽다.

| 함수 | 의미 |
|------|------|
| `process_exec(void *f_name)` | 현재 thread의 user image를 새 실행 파일로 교체 |
| `SYS_EXEC` 처리 | user pointer를 검증하고 kernel copy를 만든 뒤 `process_exec()`를 호출 |

즉 syscall `exec()`는 `process_exec()`의 syscall wrapper에 가깝다. child 생성은 `exec()`가 아니라 `fork()`의 책임이다.

## 3. exec에서 load 실패 처리가 중요한 이유

`process_exec()`는 내부에서 기존 user address space를 정리하고 새 executable을 load한다. 이때 load가 실패하면 이전 user image로 돌아갈 수 없다고 보는 것이 자연스럽다.

따라서 syscall `exec()`는 성공 시 반환하지 않고, 실패 시 현재 process를 `-1` 상태로 종료해야 한다.

```text
SYS_EXEC
  -> user cmd_line 검증
  -> kernel page에 cmd_line 복사
  -> process_exec(kernel_copy)
      -> 성공하면 do_iret(), 반환 안 함
      -> 실패하면 -1 반환
  -> process_exit_with_status(-1)
```

## 4. fork에서 load/clone 동기화가 필요한 경우

`child_status`의 `load_sema`, `load_success` 같은 필드는 exec 자체보다 parent-child 생성 흐름에서 더 자주 필요하다.

예를 들어 `fork()`는 parent가 child의 resource clone 성공 여부를 알기 전까지 반환하면 안 된다. 이때 child가 page table, fd table, 기타 자원 복제에 성공했는지 parent에게 알려야 한다.

또한 팀 설계에 따라 `fork()` 후 child가 바로 `exec()`하는 흐름을 테스트할 때, child의 종료 상태와 load 실패 상태를 parent가 wait으로 회수할 수 있어야 한다.

## 5. child_status의 동기화 필드

`struct child_status`에는 parent-child 동기화를 위한 필드가 있다.

```c
bool load_done;
bool load_success;
struct semaphore load_sema;
```

권장 의미는 다음과 같다.

| 필드 | 의미 |
|------|------|
| `load_done` | child의 load 또는 clone 시도가 끝났는가 |
| `load_success` | load 또는 clone이 성공했는가 |
| `load_sema` | parent가 child 준비 완료까지 기다릴 때 사용 |

이 필드들의 정확한 의미는 팀 구현에서 하나로 정해야 한다. 이름은 `load_*`이지만 `fork()` clone 성공 동기화에도 재사용할 수 있다.

## 6. command line 복사

`exec(cmd_line)`에서 `cmd_line`은 user pointer다. 따라서 먼저 검증하고 kernel memory로 복사해야 한다.

```text
user_check_string(cmd_line)
kernel_copy = user_strdup(cmd_line)
```

이 복사본을 `process_exec()`에 넘겨야 한다. user memory 주소를 그대로 넘기면 `process_exec()` 중 page table이 교체되거나 원본 memory가 사라질 때 위험하다.

## 7. process_exec() 실패 처리

`process_exec()`는 `load()` 실패 시 `-1`을 반환한다.

syscall `exec()` 처리 쪽에서는 이 실패를 `exit(-1)`로 바꿔야 한다.

```text
if process_exec(kernel_copy) == -1:
  process_exit_with_status(-1)
```

현재 skeleton의 `initd()`는 최초 user process 실행 실패 시 `PANIC()`을 호출한다. 일반 syscall `exec()` 실패는 kernel panic이 아니라 현재 process 종료로 처리해야 한다.

## 8. exec 후 fd는 유지된다

Unix 계열의 일반적인 의미처럼 `exec()`는 현재 process image를 바꾸지만 열린 fd는 유지된다.

Pintos Project 2에서도 `exec` 후 fd 유지가 관련 테스트와 연결된다. 따라서 `process_exec()`가 page table을 갈아엎더라도 fd table을 정리하면 안 된다.

정리 대상:

```text
교체함: user address space, code/data/stack
유지함: tid, fd table, parent-child 관계
```

## 9. 관련 테스트

| 테스트 | 확인하는 것 |
|------|------|
| `exec-once` | 현재 process image가 새 executable로 바뀌는가 |
| `exec-arg` | exec command line argument 전달 |
| `exec-missing` | 없는 파일 exec 시 현재 process가 -1로 종료되는가 |
| `exec-read` | exec 중 file read 관련 안정성 |
| `exec-bad-ptr` | cmd_line pointer 검증 |
| `exec-boundary` | cmd_line이 page boundary에 걸친 경우 |

## 10. 확인 질문

1. `process_exec()`와 syscall `exec()`는 왜 같은 함수가 아닌가?
2. KAIST Pintos에서 `exec()`가 성공하면 왜 호출자 코드로 돌아오지 않는가?
3. `exec()` 인자로 넘어온 user string을 kernel page에 복사해야 하는 이유는 무엇인가?
4. exec 후 fd table을 유지해야 하는 이유는 무엇인가?
