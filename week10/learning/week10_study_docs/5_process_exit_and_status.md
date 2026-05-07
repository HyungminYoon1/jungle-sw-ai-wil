# User Programs: Process Exit와 Exit Status

## 1. exit status란 무엇인가

사용자 프로그램은 종료될 때 정수 상태값을 남긴다.

```c
exit (0);   /* 정상 종료 */
exit (-1);  /* 비정상 종료 */
```

이 값은 두 군데에서 의미가 있다.

- 커널이 종료 메시지를 출력할 때 사용한다.
- 부모 process가 `wait(child_tid)`로 자식의 종료 결과를 회수할 때 사용한다.

Pintos 테스트는 보통 다음 형태의 출력을 기대한다.

```text
program-name: exit(0)
program-name: exit(-1)
```

## 2. 종료 흐름

사용자 프로그램이 종료되는 대표 경로는 두 가지다.

첫째, 사용자 코드가 직접 `exit(status)` syscall을 호출한다.

```text
user exit(status)
  -> syscall_handler()
  -> process_exit_with_status(status)
  -> thread_exit()
  -> process_exit()
```

둘째, `main()`이 return하면 `_start()` wrapper가 return 값을 `exit()`로 넘긴다.

```c
void
_start (int argc, char *argv[]) {
	exit (main (argc, argv));
}
```

그래서 `SYS_EXIT` 구현은 기본 테스트에서도 중요하다.

## 3. process_exit_with_status()

현재 dev 기준으로 [process.c](/workspaces/week09-team-07-pintos-threads/pintos/userprog/process.c:199)에 helper가 있다.

```c
void
process_exit_with_status (int status) {
	thread_current ()->exit_status = status;
	thread_exit ();
	NOT_REACHED ();
}
```

역할은 명확하다.

```text
현재 thread의 exit_status 저장
  -> thread_exit() 호출
  -> thread_exit() 내부에서 process_exit() 호출
```

사용자 포인터 검증 실패, 알 수 없는 syscall, bad memory 접근 같은 경우에는 보통 `process_exit_with_status(-1)`로 종료시킨다.

## 4. process_exit()

[process_exit()](/workspaces/week09-team-07-pintos-threads/pintos/userprog/process.c:413)는 실제 process 자원 정리를 담당한다.

현재 구현은 아래 일을 한다.

- 종료 메시지 출력
- 열린 file descriptor 전체 close
- 실행 중인 executable file write deny 해제 및 close
- page table 정리

개념적으로는 다음 순서다.

```text
process_exit()
  -> "name: exit(status)" 출력
  -> fd table 정리
  -> executable file 정리
  -> page table 파괴
```

## 5. 왜 thread_exit()만 호출하면 부족한가

`thread_exit()`는 thread 생명주기를 끝내는 함수다. 하지만 user process 관점에서는 종료 상태를 먼저 저장해야 한다.

나쁜 예:

```c
thread_exit ();
```

이렇게만 호출하면 현재 process가 왜 종료되었는지 부모에게 전달할 `exit_status`가 없거나 기본값으로 남을 수 있다.

좋은 예:

```c
process_exit_with_status (-1);
```

이 helper를 쓰면 상태 저장과 종료가 한 흐름으로 묶인다.

## 6. wait과 exit status

`wait(child_tid)`가 구현되면 exit status는 부모가 회수해야 하는 값이 된다.

중요한 점은 자식 thread가 종료되어도 부모가 아직 `wait()`하지 않았을 수 있다는 것이다. 따라서 exit status는 자식 thread 구조체 안에만 두면 안 된다. thread 구조체는 종료 후 파괴될 수 있기 때문이다.

그래서 parent-child 공유 상태 구조체가 필요하다.

현재 [process.h](/workspaces/week09-team-07-pintos-threads/pintos/include/userprog/process.h:8)에 `struct child_status`가 있다.

```c
struct child_status {
	tid_t tid;
	int exit_status;
	bool waited;
	bool load_done;
	bool load_success;
	int ref_cnt;
	struct semaphore load_sema;
	struct semaphore exit_sema;
	struct list_elem elem;
};
```

`exit_status`는 최종적으로 이 구조체에 저장되어야 부모가 회수할 수 있다.

## 7. 자원 정리 책임

process 종료 시 정리할 자원은 점점 늘어난다.

| 자원 | 정리 위치 |
|------|------|
| page table | `process_cleanup()` |
| fd table | `process_exit()` |
| executable file | `process_exit()` |
| child status | parent/child reference count 정책에 따라 release |
| parent의 children list | wait 또는 parent exit 시 정리 |

Project 2에서 흔한 버그는 종료 경로 하나만 처리하고 예외 종료 경로를 놓치는 것이다. `exit(-1)`, page fault, invalid syscall도 같은 자원 정리 경로로 들어오게 만드는 것이 안전하다.

## 8. 확인 질문

1. `main()`이 `return 7`을 하면 최종적으로 어떤 syscall이 호출되는가?
2. user pointer 검증 실패 시 왜 `exit_status = -1`을 저장해야 하는가?
3. 자식의 exit status를 `struct thread`에만 저장하면 왜 `wait()`에서 문제가 생길 수 있는가?
4. `process_exit()`에서 page table을 먼저 파괴하면 fd 정리에 어떤 문제가 생길 수 있는가?
