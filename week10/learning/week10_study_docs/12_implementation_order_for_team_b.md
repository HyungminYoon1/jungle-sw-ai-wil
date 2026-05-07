# User Programs: 팀원 B 구현 순서

## 1. 팀원 B의 책임 범위

구현 계획 기준 팀원 B는 fork/exec/wait와 rox를 담당한다.

담당 기능은 다음 축으로 나뉜다.

```text
parent-child 상태 관리
process_wait()
exec syscall과 load 동기화
fork syscall과 주소 공간 복사
실행 파일 write deny
```

이 영역은 A의 fd table, C의 user pointer validation과 강하게 연결된다. 따라서 독립 구현보다 공통 interface를 먼저 확인하고 작은 테스트 단위로 붙이는 방식이 안전하다.

## 2. 구현 전 최소 학습 순서

길게 공부한 뒤 한 번에 구현하지 않는다. 아래 주제를 30~60분 단위로 보고 바로 작은 구현으로 확인한다.

1. `struct child_status` 필드 의미
2. `process_exit_with_status()`와 `process_exit()` 종료 흐름
3. semaphore `sema_down`, `sema_up`
4. `process_wait()`의 직접 자식 검증과 wait 중복 방지
5. syscall `exec()`와 `process_exec()` 차이
6. fork에서 `intr_frame` 복사와 child return 0
7. page table 복사와 fd table 복제
8. executable file deny write

## 3. 1차 목표: exec-once, exec-missing

첫 구현 목표는 `exec-once`와 `exec-missing`이 좋다.

KAIST Pintos의 `exec()`는 child를 만들지 않고 현재 process image를 교체한다. 따라서 parent-child 구조를 완성하기 전에 syscall dispatcher, user string 검증, `process_exec()` 실패 처리 흐름을 먼저 확인할 수 있다.

필요 구현:

- `SYS_EXEC` dispatcher 추가
- user cmd_line 검증 및 kernel copy
- `process_exec(kernel_copy)` 호출
- 성공 시 반환하지 않음
- 실패 시 `process_exit_with_status(-1)`

예상 수정 파일:

```text
pintos/userprog/syscall.c
pintos/userprog/process.c
```

## 4. 2차 목표: fork-once

`wait-simple`은 내부에서 `fork()`를 사용하므로, wait만 단독으로 구현해서는 접근하기 어렵다. 따라서 최소 fork 흐름을 먼저 만든다.

필요 구현:

- `SYS_FORK` dispatcher 추가
- user thread_name 검증
- parent `intr_frame`을 child에게 전달
- child에서 `RAX = 0`
- parent에서는 child tid 반환
- child_status 생성 및 parent의 children list 연결
- fork clone 성공/실패를 parent에게 알리는 동기화

테스트:

```text
fork-once
```

## 5. 3차 목표: wait-simple

fork 기반 child 생성이 가능해지면 `process_wait()`를 정식 구현한다.

필요 구현:

- process_exit()에서 child_info에 exit_status 저장
- process_exit()에서 exit_sema up
- process_wait()에서 직접 child 검색
- waited 중복 검사
- sema_down(exit_sema)
- status 반환 및 child_status release

테스트:

```text
wait-simple
```

## 6. 4차 목표: wait-twice, wait-bad-pid, wait-killed

기본 wait가 되면 예외 조건을 강화한다.

체크할 정책:

- children list에 없는 tid는 -1
- 이미 `waited == true`면 -1
- child가 page fault나 invalid syscall로 죽으면 -1
- child가 먼저 죽어도 parent가 status 회수 가능

테스트:

```text
wait-twice
wait-bad-pid
wait-killed
```

## 7. 5차 목표: rox

exec/load 흐름이 안정되면 rox를 붙인다.

필요 구현:

- `load()` 성공 시 executable file을 `thread_current()->exec_file`에 보관
- `file_deny_write(exec_file)` 호출
- `process_exit()`에서 `file_allow_write()` 후 close
- `process_exec()`에서 old exec_file 정리 여부 확인

테스트:

```text
rox-simple
rox-child
rox-multichild
```

## 8. 6차 목표: fork 확장

기본 fork와 wait가 되면 fork의 완성도를 높인다.

필요 구현:

- parent page table 복사
- fd table 복제
- 실패 시 partial cleanup

테스트:

```text
fork-once
fork-multiple
fork-recursive
fork-read
fork-close
fork-boundary
```

## 9. 팀원 A/C와 맞출 인터페이스

### A: fd table

사용할 함수:

```c
int process_add_file (struct file *file);
struct file *process_get_file (int fd);
bool process_close_file (int fd);
void process_close_all_files (void);
bool process_duplicate_fds (struct thread *dst, struct thread *src);
```

fork에서는 `process_duplicate_fds()`를 사용한다.

### C: user pointer validation

사용할 함수:

```c
void user_check_string (const char *uaddr);
char *user_strdup (const char *uaddr);
```

`exec()`와 `fork()`의 문자열 인자 검증에 사용한다.

## 10. 구현 중 자주 하는 실수

- `process_wait()`에서 모든 tid를 허용한다.
- `waited`를 늦게 설정해서 wait race가 생긴다.
- child가 exit한 뒤 `child_status`를 너무 빨리 free한다.
- parent가 child load 완료 전에 `exec()` 성공을 반환한다.
- child의 fork 반환값 `RAX`를 0으로 바꾸지 않는다.
- page table 복사 실패 시 이미 만든 page/fd를 정리하지 않는다.
- rox에서 `file_deny_write()`만 하고 `file_allow_write()`를 빼먹는다.

## 11. 오늘 바로 볼 코드

먼저 아래 파일을 읽고 손으로 흐름을 그린다.

```text
pintos/include/userprog/process.h
pintos/include/threads/thread.h
pintos/userprog/process.c
pintos/userprog/syscall.c
pintos/threads/thread.c
pintos/threads/synch.c
```

특히 다음 함수에 집중한다.

```text
process_create_initd()
process_exec()
process_wait()
process_exit()
process_fork()
__do_fork()
syscall_handler()
sema_down()
sema_up()
```

## 12. 확인 질문

1. `wait-simple`을 통과하려면 parent와 child가 공유해야 하는 최소 상태는 무엇인가?
2. `exec-missing`에서 parent가 -1을 받으려면 어떤 semaphore가 필요한가?
3. fork child가 0을 반환하게 하려면 어느 시점에 어떤 값을 바꿔야 하는가?
4. rox 구현에서 `exec_file`은 언제 열고 언제 닫아야 하는가?
