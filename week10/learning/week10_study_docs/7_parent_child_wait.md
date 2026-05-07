# User Programs: Parent-Child 관계와 wait()

## 1. wait()가 해결해야 하는 문제

부모 process는 자식 process가 어떻게 종료되었는지 알아야 한다.

```c
pid_t pid = fork ("child-simple");
if (pid == 0)
  exec ("child-simple");
else
  wait (pid);
```

KAIST Pintos에서 `exec()`는 새 child를 만드는 함수가 아니라 현재 process image를 교체하는 syscall이다. 따라서 부모가 기다릴 child는 보통 `fork()`로 만들어진다.

`wait()`는 단순히 잠깐 기다리는 함수가 아니다. 다음 조건을 만족해야 한다.

- 직접 자식 process만 기다릴 수 있다.
- 같은 자식을 두 번 wait하면 두 번째는 `-1`을 반환한다.
- 자식이 아직 실행 중이면 종료될 때까지 block한다.
- 자식이 이미 종료되었어도 exit status를 회수할 수 있다.
- 자식이 커널에 의해 죽으면 `-1`을 회수한다.

## 2. 왜 struct child_status가 필요한가

자식 thread가 종료되면 `struct thread`는 언젠가 파괴된다. 그런데 부모가 그 후에 `wait()`를 호출할 수 있다.

따라서 부모와 자식이 공유하는 별도 상태가 필요하다.

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

이 구조체는 parent의 `children` list에 들어가고, child도 자기 `child_info` 포인터로 참조한다.

## 3. wait의 기본 흐름

`process_wait(child_tid)`는 대략 다음 순서로 구현한다.

```text
현재 process의 children list에서 child_tid 검색
  -> 없으면 -1
이미 waited == true?
  -> 맞으면 -1
waited = true
child가 종료될 때까지 exit_sema down
exit_status 읽기
children list에서 제거
child_status reference release
exit_status 반환
```

자식이 이미 종료된 상태라면 `exit_sema`가 이미 올라가 있어야 하므로 부모는 바로 통과할 수 있다.

## 4. 자식 종료 흐름

자식 process가 종료될 때는 자신의 `child_info`에 종료 상태를 저장하고 부모를 깨워야 한다.

```text
child process_exit()
  -> child_info->exit_status = current->exit_status
  -> sema_up(&child_info->exit_sema)
  -> child_status_release(child_info)
```

부모가 먼저 죽은 경우에도 child가 나중에 종료될 수 있다. 그래서 reference count가 필요하다.

## 5. ref_cnt 개념

`child_status`는 부모와 자식이 동시에 참조한다.

```text
parent reference
child reference
```

초기값은 보통 2로 둘 수 있다.

- 부모가 wait를 완료하거나 parent exit에서 children list를 정리하면 parent reference를 내려놓는다.
- 자식이 exit하면 child reference를 내려놓는다.
- `ref_cnt == 0`이 되면 `free(cs)`한다.

이 구조를 쓰면 다음 상황을 모두 처리할 수 있다.

| 상황 | 처리 |
|------|------|
| 부모가 먼저 wait | 자식 종료까지 block |
| 자식이 먼저 exit | 상태 저장 후 parent wait까지 유지 |
| 부모가 먼저 exit | child reference만 남고, child exit 때 free |
| wait 두 번 호출 | `waited`로 두 번째 호출 거부 |

## 6. process_wait() 임시 구현 제거

현재 [process_wait()](/workspaces/week09-team-07-pintos-threads/pintos/userprog/process.c:399)는 임시 sleep loop다.

정식 구현에서는 이 sleep loop를 제거하고 `child_status` 기반 동기화로 바꿔야 한다.

임시 구현은 user program 실행 관찰용일 뿐이다. `wait-*`, `exec-*`, `multi-*` 테스트를 통과하려면 정식 wait가 필요하다.

## 7. wait 관련 테스트

| 테스트 | 확인하는 것 |
|------|------|
| `wait-simple` | 정상 자식 종료 status 회수 |
| `wait-twice` | 같은 자식 두 번 wait 금지 |
| `wait-killed` | 예외로 죽은 자식은 -1 |
| `wait-bad-pid` | 자식이 아닌 pid wait 금지 |
| `multi-recurse` | 여러 세대 process의 wait/exit 안정성 |

## 8. 확인 질문

1. child exit status를 parent list에 남겨야 하는 이유는 무엇인가?
2. `waited` flag가 없으면 어떤 테스트가 실패하는가?
3. 부모가 먼저 죽고 자식이 나중에 죽을 때 `child_status`는 누가 free해야 하는가?
4. `exit_sema`는 누가 down하고 누가 up해야 하는가?
