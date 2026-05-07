# User Programs: fork()와 Address Space 복사

## 1. fork()의 의미

`fork()`는 현재 process를 거의 그대로 복제해 child process를 만드는 syscall이다.

```c
pid_t pid = fork ("child");
```

부모와 자식은 같은 코드 위치에서 실행을 이어가지만 반환값이 다르다.

```text
parent에서 fork() 반환값 = child tid
child에서 fork() 반환값 = 0
실패 시 parent 반환값 = -1
```

이 반환값 차이를 만들기 위해 `intr_frame` 복사가 필요하다.

## 2. intr_frame 복사

syscall이 들어왔을 때 parent의 user context는 `syscall_handler(struct intr_frame *f)` 인자로 들어온다.

`process_fork(name, f)`는 이 `f`를 child에게 전달해야 한다. child는 parent의 `intr_frame`을 복사하되, child 쪽 반환값인 `RAX`를 0으로 바꿔야 한다.

개념 흐름:

```text
parent syscall_handler()
  -> process_fork(name, f)
  -> child thread 생성
child __do_fork()
  -> parent intr_frame 복사
  -> copied_if.R.rax = 0
  -> do_iret(&copied_if)
parent
  -> child tid 반환
```

현재 skeleton의 `__do_fork()`에는 `parent_if` 전달 TODO가 남아 있다.

## 3. page table 복사

fork 후 parent와 child는 처음에는 같은 user memory 내용을 가져야 한다.

Project 2에서는 VM lazy loading이 없으므로 parent의 page table을 순회하며 실제 page를 복사한다.

`duplicate_pte()`가 담당해야 할 일은 다음과 같다.

```text
parent virtual address va 확인
kernel page나 invalid mapping이면 skip
parent physical page 찾기
child용 user page 새로 할당
내용 memcpy
writable bit 확인
child page table에 va -> newpage 매핑
```

핵심은 parent와 child가 같은 physical page를 공유하면 안 된다는 것이다. 새 page를 할당하고 내용을 복사해야 한다.

## 4. fd table 복제

fork는 열린 파일도 복제해야 한다.

현재 공통 helper:

```c
bool process_duplicate_fds (struct thread *dst, struct thread *src);
```

이 함수는 parent의 fd entry를 순회하고 `file_duplicate()`로 child 쪽 file object를 만든다.

fd table 복제는 A의 file syscall 구현과 연결된다. fork 담당자는 fd helper의 계약을 믿고 호출하되, 실패 시 이미 복사한 fd들을 정리해야 한다.

## 5. parent-child 동기화

fork에서도 parent는 child가 복제에 성공했는지 알아야 한다.

나쁜 흐름:

```text
parent: thread_create(child)
parent: child tid 반환
child: page table 복사 실패
child: exit
```

좋은 흐름:

```text
parent: child 생성
parent: child fork 완료 sema 대기
child: page table/fd 복제
child: 성공 여부 저장
child: sema up
parent: 성공이면 tid, 실패면 -1 반환
```

기존 `child_status`의 load 동기화 필드를 fork 성공 동기화에도 재사용할지, 별도 필드를 둘지는 팀 설계로 맞춰야 한다.

## 6. fork 구현이 어려운 이유

fork는 여러 구현 축을 한 번에 요구한다.

- user pointer 검증
- thread 생성
- parent-child 상태 생성
- `intr_frame` 복사
- page table 복사
- fd table 복제
- 실패 시 partial cleanup
- child return 0 처리

그래서 `fork()`는 `wait()`와 `exec()`의 계약을 먼저 이해한 뒤 구현에 들어가는 편이 안전하다.

## 7. 관련 테스트

| 테스트 | 확인하는 것 |
|------|------|
| `fork-once` | 기본 fork와 child return 0 |
| `fork-multiple` | 여러 child fork/wait |
| `fork-recursive` | 재귀 fork 안정성 |
| `fork-read` | fork 후 memory/file read |
| `fork-close` | parent/child fd close 독립성 |
| `fork-boundary` | fork 인자 boundary 검증 |

## 8. 확인 질문

1. child의 `fork()` 반환값이 0이 되려면 어느 레지스터를 바꿔야 하는가?
2. parent page를 child page table에 그대로 매핑하면 어떤 문제가 생기는가?
3. fd table 복제 실패 시 이미 복제한 fd들은 어떻게 해야 하는가?
4. parent가 child fork 완료를 기다리지 않으면 어떤 테스트에서 불안정해질 수 있는가?
