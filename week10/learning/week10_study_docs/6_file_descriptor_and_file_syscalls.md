# User Programs: File Descriptor와 File Syscall

## 1. file descriptor란 무엇인가

File descriptor, 줄여서 fd는 사용자 프로그램이 열린 파일을 가리키기 위해 사용하는 작은 정수다.

사용자 프로그램은 kernel 안의 `struct file *`를 직접 볼 수 없다. 대신 커널이 정수 fd를 발급하고, 이후 syscall에서 그 fd를 다시 넘긴다.

```text
open("sample.txt") -> 2
read(2, buffer, 100)
close(2)
```

커널은 각 process별 fd table에서 `2 -> struct file *` 매핑을 찾아 실제 파일 작업을 수행한다.

## 2. fd 0과 fd 1

Pintos user programs에서 특별한 fd가 있다.

| fd | 의미 |
|------|------|
| `0` | stdin |
| `1` | stdout |
| `2` 이상 | 일반 파일 |

`write(1, buffer, size)`는 콘솔 출력으로 처리한다.

`read(0, buffer, size)`는 키보드 입력으로 처리한다.

일반 파일은 `open()` 성공 시 2부터 증가하는 fd를 발급하는 방식이 흔하다.

## 3. fd table의 저장 위치

fd table은 `intr_frame`에 저장하면 안 된다. `intr_frame`은 syscall 한 번의 레지스터 상태만 담기 때문이다.

fd table은 process가 살아 있는 동안 유지되어야 하므로 `struct thread` 같은 process 상태에 둔다.

현재 dev 기준 [thread.h](/workspaces/week09-team-07-pintos-threads/pintos/include/threads/thread.h:123)에 다음 필드가 추가되어 있다.

```c
struct list fd_table;
int next_fd;
```

[process.c](/workspaces/week09-team-07-pintos-threads/pintos/userprog/process.c:32)는 list 기반 fd entry를 사용한다.

```c
struct fd_entry {
	int fd;
	struct file *file;
	struct list_elem elem;
};
```

## 4. fd helper 흐름

현재 공통 helper는 다음 역할을 한다.

| 함수 | 역할 |
|------|------|
| `process_user_init()` | fd table과 children list 초기화 |
| `process_add_file(file)` | 새 fd 발급 후 table에 추가 |
| `process_get_file(fd)` | fd로 file pointer 조회 |
| `process_close_file(fd)` | fd 제거 후 file close |
| `process_close_all_files()` | process 종료 시 모든 fd close |
| `process_duplicate_fds(dst, src)` | fork 시 fd table 복제 |

파일 syscall 담당자는 syscall handler에서 이 helper를 사용하면 된다.

## 5. global filesys lock

Pintos file system은 Project 2 단계에서 여러 process가 동시에 접근할 때 안전하다고 가정하기 어렵다.

그래서 모든 파일 시스템 접근은 하나의 global lock으로 보호하는 방식이 일반적이다.

현재 [syscall.h](/workspaces/week09-team-07-pintos-threads/pintos/include/userprog/syscall.h:6)에 선언되어 있다.

```c
extern struct lock filesys_lock;
```

그리고 [syscall.c](/workspaces/week09-team-07-pintos-threads/pintos/userprog/syscall.c:20)에 정의되어 있다.

```c
struct lock filesys_lock;
```

사용 예:

```c
lock_acquire (&filesys_lock);
file = filesys_open (name);
lock_release (&filesys_lock);
```

lock을 잡은 뒤 중간에 `exit(-1)`로 빠지면 lock이 풀리지 않을 수 있으므로, 검증은 가능하면 lock 획득 전에 끝내는 편이 좋다.

## 6. syscall별 구현 방향

### create(file, initial_size)

- user string 검증
- `filesys_create(file, initial_size)` 호출
- 성공 여부 반환

### open(file)

- user string 검증
- `filesys_open(file)` 호출
- 실패 시 `-1`
- 성공 시 `process_add_file(file)`로 fd 발급

### read(fd, buffer, size)

- `buffer`는 user write buffer 검증
- `fd == 0`이면 keyboard input
- `fd == 1`이면 실패 또는 `-1`
- 일반 fd면 `process_get_file(fd)` 후 `file_read()`

### write(fd, buffer, size)

- `buffer`는 user read buffer 검증
- `fd == 1`이면 `putbuf()`
- `fd == 0`이면 실패 또는 `-1`
- 일반 fd면 `file_write()`

### close(fd)

- fd table에서 제거
- 이미 닫힌 fd나 잘못된 fd면 실패 처리

## 7. fork와 fd 복제

`fork()`는 부모 process의 열린 파일 상태를 자식에게 복사해야 한다.

단순히 `struct file *` 포인터만 복사하면 부모와 자식의 close가 서로 영향을 줄 수 있다. KAIST Pintos에는 `file_duplicate()`가 제공되어 있어 이를 활용한다.

현재 `process_duplicate_fds()`는 각 fd entry마다 `file_duplicate()`를 호출하는 방향으로 준비되어 있다.

## 8. 확인 질문

1. fd table을 `struct intr_frame`에 두면 왜 안 되는가?
2. `write(1, buffer, size)`와 `write(2, buffer, size)`는 커널 내부에서 어떻게 달라지는가?
3. 파일 syscall에서 user pointer 검증을 lock 획득 전에 하는 편이 안전한 이유는 무엇인가?
4. `fork()`에서 fd table을 복제할 때 `file_duplicate()`가 필요한 이유는 무엇인가?
