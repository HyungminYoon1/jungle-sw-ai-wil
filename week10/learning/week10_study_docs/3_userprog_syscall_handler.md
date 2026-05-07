# User Programs: syscall_handler 학습 정리

## 1. syscall_handler는 무엇을 하는 함수인가

`syscall_handler()`는 user program이 kernel에게 요청한 일을 처리하는 진입점이다.

사용자 프로그램은 직접 커널 함수에 접근할 수 없다. 대신 `write()`, `exit()`, `open()` 같은 user library 함수를 호출하고, 이 함수들은 CPU의 `syscall` 명령을 사용해 kernel mode로 진입한다. Pintos는 이 요청을 `userprog/syscall.c`의 `syscall_handler()`로 전달한다.

KAIST 초기 skeleton 기준 코드는 다음처럼 되어 있다.

```c
void
syscall_handler (struct intr_frame *f UNUSED) {
	// TODO: Your implementation goes here.
	printf ("system call!\n");
	thread_exit ();
}
```

이 코드는 syscall이 들어왔다는 메시지만 출력하고 모든 사용자 프로세스를 종료한다. 따라서 userprog 테스트를 통과하려면 이 부분을 실제 syscall dispatcher로 바꿔야 한다.

일반적인 구조는 다음과 같다.

```c
void
syscall_handler (struct intr_frame *f) {
	switch (f->R.rax) {
		case SYS_WRITE:
			/* write syscall 처리 */
			break;
		case SYS_EXIT:
			/* exit syscall 처리 */
			break;
		case SYS_HALT:
			/* halt syscall 처리 */
			break;
		default:
			/* 알 수 없는 syscall이면 프로세스 종료 */
			break;
	}
}
```

## 2. 사용자 printf가 출력되는 흐름

userprog 단계에서 `printf()` 자체를 새로 구현하는 것은 아니다. 사용자 프로그램용 `printf()`와 syscall wrapper는 이미 제공된다.

중요한 것은 `printf()`가 최종적으로 호출하는 `write(1, buffer, size)` syscall을 커널에서 처리하는 것이다.

흐름은 다음과 같다.

```text
사용자 프로그램
  printf(), msg(), write() 호출
    |
    v
pintos/lib/user/syscall.c
  syscall instruction 실행
    |
    v
pintos/userprog/syscall-entry.S
  레지스터 상태를 intr_frame에 저장
    |
    v
pintos/userprog/syscall.c
  syscall_handler(struct intr_frame *f) 호출
    |
    v
커널이 요청 처리 후 반환값을 f->R.rax에 저장
```

사용자 프로그램의 출력이 보이려면 커널이 `SYS_WRITE`를 처리할 수 있어야 한다. 또한 사용자 프로그램은 `main()`이 끝난 뒤 `_start()`에서 `exit(main(argc, argv))`를 호출하므로, `SYS_EXIT` 처리도 필요하다.

## 3. struct intr_frame *f는 무엇인가

`struct intr_frame *f`는 syscall 또는 interrupt가 발생한 순간의 CPU 상태를 저장한 구조체이다.

사용자 프로그램이 `syscall` 명령을 실행하면 CPU는 kernel mode로 진입한다. 이때 Pintos는 사용자 프로그램이 사용하던 레지스터 값, instruction pointer, stack pointer 등을 `struct intr_frame`에 저장한 뒤 `syscall_handler(f)`를 호출한다.

정의는 `pintos/include/threads/interrupt.h`에 있다.

```c
struct intr_frame {
	struct gp_registers R;
	uint16_t es;
	uint16_t __pad1;
	uint32_t __pad2;
	uint16_t ds;
	uint16_t __pad3;
	uint32_t __pad4;
	uint64_t vec_no;
	uint64_t error_code;
	uintptr_t rip;
	uint16_t cs;
	uint16_t __pad5;
	uint32_t __pad6;
	uint64_t eflags;
	uintptr_t rsp;
	uint16_t ss;
	uint16_t __pad7;
	uint32_t __pad8;
} __attribute__((packed));
```

여기서 syscall 구현에 가장 자주 쓰는 부분은 `f->R`이다.

## 4. syscall 번호와 인자는 어디에 들어 있는가

KAIST Pintos x86-64에서는 syscall 번호와 인자가 레지스터를 통해 전달된다.

`pintos/lib/user/syscall.c`의 wrapper는 다음 규칙으로 값을 넣고 `syscall` 명령을 실행한다.

| 위치 | 의미 |
| --- | --- |
| `f->R.rax` | syscall 번호. 커널이 반환값을 저장하는 위치이기도 하다. |
| `f->R.rdi` | 1번째 인자 |
| `f->R.rsi` | 2번째 인자 |
| `f->R.rdx` | 3번째 인자 |
| `f->R.r10` | 4번째 인자 |
| `f->R.r8` | 5번째 인자 |
| `f->R.r9` | 6번째 인자 |

예를 들어 사용자 프로그램이 다음 코드를 실행했다고 하자.

```c
write (1, buffer, size);
```

커널의 `syscall_handler()`에서는 다음처럼 보인다.

```c
f->R.rax == SYS_WRITE
f->R.rdi == 1       /* fd */
f->R.rsi == buffer  /* user buffer address */
f->R.rdx == size
```

즉 `SYS_WRITE`의 file descriptor 값은 `f->R.rdi`에 들어 있다.

## 5. file descriptor는 intr_frame 안에 저장되는가

정확히 말하면 file descriptor table이 `intr_frame` 안에 저장되는 것은 아니다.

`intr_frame`에는 syscall 호출 순간의 레지스터 값만 들어 있다. `write(1, buffer, size)`처럼 fd가 syscall 인자로 전달된 경우, 그 fd 값 하나가 `f->R.rdi`에 들어 있을 뿐이다.

반면 fd table은 프로세스가 살아 있는 동안 유지되어야 하는 상태이다. 따라서 `intr_frame`이 아니라 현재 실행 중인 프로세스 또는 스레드의 상태에 저장해야 한다.

Pintos에서는 보통 `struct thread`에 다음과 같은 필드를 추가해서 관리한다.

```c
struct file *fd_table[MAX_FD];
int next_fd;
```

또는 list 기반으로 fd entry 구조체를 만들어 관리할 수도 있다.

핵심 차이는 다음과 같다.

| 구분 | 저장 위치 | 수명 |
| --- | --- | --- |
| syscall 인자로 전달된 fd 값 | `intr_frame`의 레지스터 필드 | syscall 처리 중 |
| fd table | `struct thread` 등 프로세스 상태 | 프로세스 생존 기간 |

## 6. SYS_WRITE 처리 방향

`fd == 1`은 stdout을 의미한다. 사용자 프로그램이 `write(1, buffer, size)`를 호출하면 커널은 `buffer`에 있는 데이터를 콘솔로 출력해야 한다.

개념 코드는 다음과 같다.

```c
case SYS_WRITE: {
	int fd = f->R.rdi;
	const void *buffer = (const void *) f->R.rsi;
	unsigned size = f->R.rdx;

	if (fd == 1) {
		putbuf (buffer, size);
		f->R.rax = size;
	} else {
		f->R.rax = -1;
	}
	break;
}
```

실제 구현에서는 `buffer`가 유효한 사용자 주소인지 반드시 검사해야 한다. 잘못된 포인터를 그대로 `putbuf()`에 넘기면 커널이 page fault를 내거나 다른 프로세스에 영향을 줄 수 있다.

## 7. system call 단계의 기본 syscall 세트

KAIST 문서 흐름을 기준으로 보면 argument passing을 먼저 이해하고 구현한 뒤 system call 단계로 넘어오는 것이 자연스럽다.

system call 단계에 들어오면 다음 세트를 작게 여는 것이 좋다.

1. `SYS_WRITE`
   - `fd == 1` stdout 출력
   - 사용자 프로그램의 `printf()`와 테스트 `msg()` 출력을 보기 위해 필요

2. `SYS_EXIT`
   - 사용자 프로그램 종료 상태 처리
   - `_start()`가 `main()` 반환값으로 `exit()`를 호출하므로 필요

3. `SYS_HALT`
   - `halt` 테스트 처리
   - `power_off()` 호출

이 세트는 system call 구현의 출발점이다. 다만 `args-*` 테스트 자체는 argument passing이 핵심이므로, system call 구현과 별개로 stack/register 구성이 올바른지 확인해야 한다.

## 8. 구현 순서 추천

KAIST Pintos 원문과 초기 skeleton code 흐름을 기준으로 보면 다음 순서가 기본이다.

1. `process_exec()`에서 argument passing을 구현한다.
2. `syscall_handler()` dispatcher를 작성한다.
3. `SYS_HALT`, `SYS_EXIT`, `SYS_WRITE(fd == 1)`을 작게 구현한다.
4. user pointer validation을 강화한다.
5. file syscall과 fd table을 구현한다.
6. `process_wait()`와 wait, exec, fork를 정식으로 구현한다.

KAIST system call 문서는 initial process가 끝날 때까지 Pintos가 종료되지 않아야 한다고 설명한다. 이를 위해 `process_wait()`를 올바르게 구현해야 한다.
