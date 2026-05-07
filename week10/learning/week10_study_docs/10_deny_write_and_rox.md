# User Programs: deny_write와 rox 테스트

## 1. rox란 무엇인가

rox는 "read-only executable" 계열 테스트를 가리킨다.

핵심 요구는 다음과 같다.

```text
실행 중인 executable file은 write로 수정되면 안 된다.
```

사용자 프로그램이 자기 실행 파일을 열고 write하려 하거나, 다른 process가 실행 중인 파일을 write하려 할 때 커널이 막아야 한다.

## 2. 왜 실행 파일 쓰기를 막아야 하나

실행 파일은 `load()` 과정에서 code/data segment로 user memory에 적재된다.

만약 실행 중인 executable file을 다른 process가 write로 바꿀 수 있다면 다음 문제가 생긴다.

- 이미 적재된 code와 disk file 내용이 달라진다.
- lazy loading이 있는 이후 프로젝트에서는 아직 읽지 않은 page가 변조될 수 있다.
- 실행 중 프로그램의 안정성과 보안이 깨진다.

Project 2에서는 테스트 요구사항으로 실행 중인 파일에 대한 write deny를 구현한다.

## 3. file_deny_write()

Pintos file layer에는 write deny 관련 함수가 있다.

```c
file_deny_write (file);
file_allow_write (file);
```

실행 파일을 성공적으로 연 뒤, 그 file object를 process가 종료될 때까지 보관하고 write deny 상태로 둔다.

현재 dev 기준 `struct thread`에는 [thread.h](/workspaces/week09-team-07-pintos-threads/pintos/include/threads/thread.h:127)에 `exec_file` 필드가 있다.

```c
struct file *exec_file;
```

## 4. load()에서 처리할 일

`load()`는 실행 파일을 연다.

```c
file = filesys_open (tmp_argv[0]);
```

ELF load가 성공하면 이 file을 바로 닫지 말고 현재 thread의 `exec_file`에 저장하고 `file_deny_write()`를 호출해야 한다.

개념 흐름:

```text
file = filesys_open(executable)
ELF header/segment load 성공
file_deny_write(file)
thread_current()->exec_file = file
load 성공 반환
```

주의할 점은 실패 경로다. load 실패 시에는 file을 닫아야 한다. 성공 시에는 process 종료까지 보관해야 한다.

## 5. process_exit()에서 처리할 일

process가 종료될 때는 실행 파일 write deny를 해제하고 close해야 한다.

현재 [process_exit()](/workspaces/week09-team-07-pintos-threads/pintos/userprog/process.c:416)에 정리 코드가 준비되어 있다.

```c
if (curr->exec_file != NULL) {
	file_allow_write (curr->exec_file);
	file_close (curr->exec_file);
	curr->exec_file = NULL;
}
```

따라서 rox 구현에서 중요한 작업은 `load()` 성공 시 `exec_file`에 올바르게 저장하는 것이다.

## 6. exec와 deny_write

`exec()`는 현재 process image를 새 실행 파일로 교체한다.

이미 실행 중이던 old `exec_file`이 있다면 새 파일을 load하기 전에 정리해야 한다. 그렇지 않으면 이전 실행 파일의 write deny가 계속 남을 수 있다.

개념적으로:

```text
process_exec()
  -> old address space cleanup
  -> old exec_file allow_write + close
  -> load(new file)
  -> new exec_file deny_write
```

다만 이 순서는 실패 처리와도 연결된다. 새 load가 실패하면 process 자체가 종료되므로 old image 복구는 하지 않는다.

## 7. 관련 테스트

| 테스트 | 확인하는 것 |
|------|------|
| `rox-simple` | 자기 실행 파일 write 금지 |
| `rox-child` | child 실행 파일 write 금지 |
| `rox-multichild` | 여러 child 실행 중 write deny 유지 |

## 8. 확인 질문

1. `load()` 성공 후 executable file을 바로 `file_close()`하면 왜 rox를 구현할 수 없는가?
2. `file_deny_write()`는 어느 시점에 호출해야 하는가?
3. process 종료 시 `file_allow_write()`를 호출하지 않으면 어떤 문제가 남는가?
4. `exec()`로 새 프로그램을 실행할 때 이전 `exec_file`은 어떻게 처리해야 하는가?
