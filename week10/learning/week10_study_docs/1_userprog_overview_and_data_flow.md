# User Programs: 큰 그림과 데이터 흐름

## 1. User Programs 프로젝트는 무엇을 하는 단계인가

Threads 단계에서는 커널 안에서 실행되는 thread, scheduler, synchronization을 다뤘다.

User Programs 단계에서는 한 단계 더 나아가, 커널이 사용자 프로그램을 실행할 수 있게 만든다.

여기서 사용자 프로그램은 Pintos 커널 밖에서 독립적으로 실행되는 것처럼 보이는 C 프로그램이다. 예를 들어 `pintos/tests/userprog` 아래의 `args-*`, `halt`, `exit`, `write-*` 테스트 프로그램들이 사용자 프로그램이다.

하지만 Pintos에는 Linux나 Windows처럼 완성된 운영체제 환경이 없다. 그래서 커널이 직접 다음 일을 해줘야 한다.

- 실행 파일을 파일 시스템에서 찾는다.
- ELF 실행 파일을 메모리에 적재한다.
- user stack을 만든다.
- `argc`, `argv`를 사용자 프로그램에 전달한다.
- user mode로 진입시킨다.
- 사용자 프로그램이 system call을 요청하면 kernel mode에서 처리한다.
- 프로그램 종료 상태를 부모 process가 확인할 수 있게 관리한다.

즉 User Programs 프로젝트의 핵심은 다음 한 문장으로 정리할 수 있다.

> 커널이 사용자 프로그램을 안전하게 시작시키고, 실행 중 필요한 kernel 기능을 system call로 제공하게 만드는 단계다.

## 2. user program이란 무엇인가

user program은 커널 코드가 아니라 user mode에서 실행되는 프로그램이다.

일반 C 프로그램처럼 `main()`을 가진다.

```c
int
main (int argc, char *argv[]) {
	/* user program code */
}
```

하지만 Pintos에서는 사용자가 직접 이 프로그램을 실행하는 것이 아니다. 커널이 실행 파일을 읽고, 메모리에 올리고, CPU 상태를 맞춘 뒤 user mode로 넘겨준다.

이때 커널이 준비를 잘못하면 user program은 `main()`에 들어가기도 전에 page fault를 낼 수 있다.

## 3. kernel mode와 user mode

운영체제는 보통 kernel mode와 user mode를 구분한다.

kernel mode는 운영체제 커널이 실행되는 모드다. 하드웨어, 메모리, 파일 시스템 같은 중요한 자원에 직접 접근할 수 있다.

user mode는 일반 프로그램이 실행되는 모드다. 중요한 자원에 직접 접근할 수 없고, 필요하면 system call을 통해 커널에 요청해야 한다.

이 구분이 필요한 이유는 안전성 때문이다. 사용자 프로그램이 커널 메모리를 마음대로 읽거나 쓰면 운영체제 전체가 망가질 수 있다.

Pintos User Programs에서 구현해야 하는 많은 기능은 이 경계를 안전하게 넘나드는 일과 관련된다.

```text
user program
  -> system call
  -> kernel
  -> validate user pointer
  -> perform requested work
  -> return to user program
```

## 4. user program 실행의 큰 흐름

사용자 프로그램 실행은 대략 다음 흐름으로 진행된다.

```text
커널 부팅
  -> run action 해석
  -> process_create_initd()
  -> thread_create()
  -> initd()
  -> process_exec()
  -> load()
  -> setup_stack()
  -> do_iret()
  -> _start()
  -> main()
```

각 단계의 의미는 다음과 같다.

| 단계 | 핵심 역할 |
|------|------|
| run action 해석 | `pintos -q run 'args-single onearg'` 같은 실행 요청을 해석한다. |
| `process_create_initd()` | 최초 user process를 만들기 위한 준비를 한다. |
| `thread_create()` | user process를 실행할 kernel thread를 만든다. |
| `initd()` | 새 thread에서 `process_exec()`를 호출한다. |
| `process_exec()` | command line을 바탕으로 실행 파일 적재를 시작한다. |
| `load()` | ELF 실행 파일을 열고 user memory에 segment를 적재한다. |
| `setup_stack()` | user stack page를 만들고 초기 stack pointer를 잡는다. |
| `do_iret()` | 준비된 CPU 상태로 user mode에 진입한다. |
| `_start()` | user library wrapper로, `main(argc, argv)`를 호출한다. |
| `main()` | 실제 테스트 프로그램 또는 사용자 프로그램 코드가 실행된다. |

## 5. command line은 어떻게 이동하는가

예를 들어 다음 명령어가 들어왔다고 하자.

```text
args-multiple some arguments for you!
```

처음에는 이 전체가 하나의 command line 문자열이다.

```text
"args-multiple some arguments for you!"
```

하지만 이 문자열은 두 가지 목적으로 나뉘어 쓰인다.

첫 번째 목적은 실행 파일 이름이다.

```text
load()에 필요한 이름 = "args-multiple"
```

두 번째 목적은 사용자 프로그램에 전달할 인자 목록이다.

```text
argv[0] = "args-multiple"
argv[1] = "some"
argv[2] = "arguments"
argv[3] = "for"
argv[4] = "you!"
argv[5] = NULL
```

따라서 `process_exec()`와 `load()` 근처에서 command line을 다룰 때는 두 요구를 모두 만족해야 한다.

- 실행 파일을 열 때는 첫 번째 토큰만 사용한다.
- user stack에는 모든 토큰을 올린다.

현재 이 repo의 구현은 `load()`가 전체 command line을 받은 뒤 내부에서 토큰화하고, `filesys_open(tmp_argv[0])`처럼 첫 번째 토큰으로 실행 파일을 연다. 구현 위치가 어디든 핵심은 "파일 시스템에서 열 이름은 첫 번째 토큰, 사용자 스택에는 전체 인자"라는 점이다.

## 6. 왜 command line 원본을 조심해야 하나

Pintos에서 command line을 토큰화할 때 `strtok_r()`를 사용할 수 있다.

하지만 `strtok_r()`는 문자열을 읽기만 하는 함수가 아니다. 공백 위치에 `\0`을 넣으면서 원본 문자열을 직접 바꾼다.

예를 들어:

```text
"args-multiple some arguments"
```

를 토큰화하면 내부적으로는 다음처럼 잘릴 수 있다.

```text
"args-multiple\0some\0arguments"
```

그래서 실행 파일 이름 분리용 문자열과, 나중에 stack에 올릴 argument parsing용 문자열을 어떻게 보존할지 생각해야 한다.

원본을 너무 일찍 파괴하면 다음 문제가 생길 수 있다.

- `load()`에는 첫 번째 토큰을 넘겼지만, 전체 인자 목록을 잃어버린다.
- 한 번 잘린 문자열은 원래 command line 형태로 다시 사용할 수 없다.
- 해제된 kernel page 안의 문자열 주소를 user stack의 `argv`로 잘못 사용할 수 있다.

`strtok_r()`로 공백을 `\0`으로 바꾸는 것 자체는 정상적인 토큰화 방식이다. 문제는 같은 문자열을 실행 파일 이름 분리와 전체 인자 재파싱에 동시에 쓰려고 할 때 생긴다. 두 목적이 모두 필요하다면 복사본을 나누어 관리하는 편이 안전하다.

## 7. user stack에는 무엇이 들어가는가

argument passing이 끝나면 user stack에는 크게 세 종류의 데이터가 들어간다.

첫째, 실제 문자열이다.

```text
"args-multiple\0"
"some\0"
"arguments\0"
"for\0"
"you!\0"
```

둘째, 문자열 주소를 담은 `argv[]` 배열이다.

```text
argv[0] -> "args-multiple"
argv[1] -> "some"
argv[2] -> "arguments"
argv[3] -> "for"
argv[4] -> "you!"
argv[5] -> NULL
```

셋째, fake return address다.

```text
fake return address = 0
```

`argv[5] = NULL`은 인자 배열의 끝 표시다. fake return address와는 다른 목적이다.

## 8. intr_frame은 왜 중요한가

Pintos는 user mode로 들어가기 전에 `struct intr_frame`에 CPU 상태를 준비한다.

`intr_frame`에는 user program이 시작할 때 필요한 register 값들이 들어간다.

argument passing에서 특히 중요한 값은 다음이다.

```text
RDI = argc
RSI = argv 배열의 시작 주소
RSP = 최종 user stack pointer
RIP = ELF header의 entry point
```

예를 들어:

```text
args-multiple some arguments for you!
```

라면:

```text
RDI = 5
RSI = &argv[0]
RSP = fake return address 위치
RIP = ELF header의 entry point
```

Pintos user C program에서는 이 entry point가 보통 `lib/user/entry.c`의 `_start()`를 가리킨다. `do_iret()`은 이 `intr_frame`을 바탕으로 CPU 상태를 복원하고 user mode로 넘어간다.

## 9. syscall은 언제 등장하는가

`syscall.c`는 사용자 프로그램을 시작시키는 코드가 아니다.

사용자 프로그램이 이미 실행된 뒤, 커널 기능이 필요할 때 system call을 호출한다. 그때 커널로 들어와 처리하는 곳이 `syscall.c`다.

예를 들어 user program이 출력하려고 하면 내부적으로 `write` system call을 호출한다.

```text
user program
  -> write(fd, buffer, size)
  -> syscall instruction
  -> syscall_entry.S
  -> syscall_handler()
  -> kernel handles SYS_WRITE
  -> return to user program
```

따라서 argument passing이 되지 않아 user program이 `main()`에 들어가지 못하면, `SYS_WRITE`를 구현해도 그 코드까지 도달하지 못할 수 있다.

이것이 User Programs 초반에 argument passing을 먼저 이해해야 하는 이유다.

## 10. 파일별 큰 역할

User Programs에서 자주 보게 될 파일의 역할은 다음과 같다.

| 파일 | 큰 역할 |
|------|------|
| `pintos/userprog/process.c` | user process 생성, 실행 파일 적재, user stack 준비, wait/exec/fork 흐름 |
| `pintos/userprog/syscall.c` | user program이 요청한 system call 처리 |
| `pintos/userprog/exception.c` | page fault 같은 예외 처리 |
| `pintos/include/threads/thread.h` | thread/process 상태 저장 구조체 |
| `pintos/threads/thread.c` | thread 생성, 스케줄링, 종료 흐름 |
| `pintos/filesys/filesys.c` | 실행 파일과 일반 파일을 여는 file system 인터페이스 |
| `pintos/filesys/file.c` | 열린 파일에 대한 read/write/seek/close 인터페이스 |

초반 argument passing 구현의 중심은 `process.c`다. `syscall.c`는 그 다음 단계에서 중요해진다.

Project 2에서는 `filesys`와 `file` 내부 구현을 바꾸기보다, 제공된 인터페이스를 `userprog` 계층에서 안전하게 호출하는 것이 기본 방향이다. 파일 시스템 동시성 문제도 먼저 `userprog` 계층의 lock으로 보호하는 방식으로 접근한다.

## 11. 예시로 전체 흐름 따라가기

명령어가 다음과 같다고 하자.

```text
args-multiple some arguments for you!
```

처음 커널이 받는 문자열은 전체 command line이다.

```text
"args-multiple some arguments for you!"
```

`load()`에 필요한 실행 파일 이름은 첫 번째 토큰이다.

```text
"args-multiple"
```

user stack에 올려야 할 토큰은 전체 토큰이다.

```text
argc = 5

argv[0] = "args-multiple"
argv[1] = "some"
argv[2] = "arguments"
argv[3] = "for"
argv[4] = "you!"
argv[5] = NULL
```

user mode로 넘어가기 직전 register는 개념적으로 다음과 같다.

```text
RDI = 5
RSI = &argv[0]
RSP = fake return address 위치
```

그 뒤 `_start(argc, argv)`가 실행되고, `_start()`는 다시 `main(argc, argv)`를 호출한다.

```text
_start(5, argv)
  -> main(5, argv)
```

## 12. KAIST 문서 기준으로 구현 순서 보기

구현 순서는 팀의 편의가 아니라 KAIST Pintos 문서와 skeleton code 흐름을 먼저 기준으로 잡아야 한다.

KAIST Project 2 문서는 User Programs의 큰 배경을 설명한 뒤, argument passing을 먼저 다루고 system call을 그 다음에 다룬다. 이는 사용자 프로그램이 system call을 호출하려면 먼저 실행 파일이 적재되고, `_start(argc, argv)`와 `main(argc, argv)`까지 정상적으로 진입해야 하기 때문이다.

큰 기준은 다음 순서로 이해하면 된다.

1. user program 실행 경로를 이해한다.
2. `process_exec()`에서 command line을 토큰화하고 argument passing을 구현한다.
3. system call handler에서 syscall 번호와 인자를 읽는 구조를 만든다.
4. `SYS_HALT`, `SYS_EXIT`, `SYS_WRITE(fd=1)` 같은 기본 syscall을 구현한다.
5. user pointer validation을 강화한다.
6. file descriptor table과 file syscall을 구현한다.
7. wait, exec, fork 같은 process lifecycle을 정식으로 구현한다.

KAIST system call 문서는 initial process가 끝날 때까지 Pintos가 종료되지 않도록 `process_wait()`를 올바르게 구현해야 한다고 설명한다.

## 13. 학습 점검 질문

아래 질문에 답할 수 있으면 User Programs의 큰 그림을 어느 정도 잡은 것이다.

1. User Programs 단계는 Threads 단계와 무엇이 다른가?
2. user program은 왜 kernel mode가 아니라 user mode에서 실행되어야 하는가?
3. `process_exec()`와 `load()`는 각각 어떤 역할을 하는가?
4. 실행 파일을 open할 때 왜 전체 command line이 아니라 첫 번째 토큰만 사용해야 하는가?
5. user stack에는 왜 문자열뿐 아니라 `argv[]` 배열도 만들어야 하는가?
6. `RDI`, `RSI`, `RSP`는 argument passing에서 각각 무엇을 담는가?
7. `syscall.c`는 user program 시작 전에 쓰이는가, 시작 후 요청 처리에 쓰이는가?
8. argument passing이 없으면 `SYS_WRITE`를 구현해도 테스트 출력이 안 보일 수 있는 이유는 무엇인가?
