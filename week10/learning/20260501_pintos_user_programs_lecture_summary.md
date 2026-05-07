# 2026-05-01 PintOS User Programs 강의 정리

이 문서는 이동석 코치님의 PintOS User Programs 강의 필기를 바탕으로, KAIST PintOS 원문 문서의 개념을 함께 보강한 복습용 정리입니다. 목적은 강의에서 나온 키워드를 구현 순서와 운영체제 개념으로 다시 연결하는 것입니다.

## 1. 이번 주의 핵심 방향

Project 2는 `User Programs`입니다. Project 1에서는 테스트 코드가 커널 안에 함께 컴파일되어 커널 내부 기능을 직접 검증했습니다. 반면 Project 2에서는 테스트 프로그램이 별도의 사용자 프로그램으로 컴파일되고, PintOS 파일 시스템 안에 복사된 뒤 실행됩니다.

따라서 이번 주에는 단순히 커널 함수 하나를 고치는 것이 아니라, 커널이 사용자 프로그램을 다음 흐름으로 실행할 수 있게 만들어야 합니다.

```text
사용자 프로그램 실행 파일 준비
-> PintOS 파일 시스템에 복사
-> 커널이 실행 파일을 열고 ELF로 해석
-> 사용자 가상 메모리에 code/data/stack 배치
-> argument passing
-> user mode로 진입
-> 사용자 프로그램이 syscall 호출
-> kernel mode에서 요청 처리
-> 종료 상태와 자원 정리
```

코치님이 강조한 첫 번째 우선순위는 `argument passing`입니다. 이 부분을 최대한 빨리 끝내야 이후 `printf()`, `exit`, `write`, `wait` 같은 흐름을 제대로 확인할 수 있습니다. 다만 단순히 테스트만 맞추는 것이 아니라, 왜 스택에 인자를 그렇게 배치하는지 이해하면서 해결해야 합니다.

## 2. Project 1과 Project 2의 차이

Project 1 Threads에서는 테스트 코드가 커널 이미지 안에 들어 있었습니다. 즉, 테스트가 커널 내부에서 직접 실행되었습니다.

Project 2 User Programs에서는 구조가 달라집니다.

| 구분 | Project 1 Threads | Project 2 User Programs |
|---|---|---|
| 테스트 실행 위치 | 커널 내부 | 사용자 프로그램 |
| 테스트 코드 형태 | 커널에 함께 컴파일 | 별도 실행 파일 |
| 핵심 관심사 | 스케줄링, 동기화 | 실행 파일 로드, syscall, 주소 검증 |
| 실패 위험 | 커널 자료구조 손상 | 사용자 프로그램 오류가 커널까지 무너지는 문제 |

원문 introduction에서도 Project 2는 Project 1 위에 누적되는 과제라고 설명합니다. Project 1 코드가 완벽하지 않더라도 Project 2를 진행할 수는 있지만, 누적형 프로젝트이므로 Project 1 테스트가 계속 깨지지 않도록 유지하는 것이 원칙입니다.

## 3. Program, Process, Thread

강의에서 program, process, thread의 차이가 언급되었습니다. PintOS에서는 한 process가 하나의 thread만 가지므로 둘이 거의 비슷하게 느껴질 수 있지만, 개념은 구분하는 것이 좋습니다.

| 개념 | 의미 |
|---|---|
| program | 디스크에 있는 실행 파일 또는 코드 자체 |
| process | program이 실행 중인 상태 |
| thread | CPU가 실제로 따라가는 실행 흐름 |

예를 들어 `args-single`이라는 실행 파일은 program입니다. 이 파일을 실행해서 메모리, 스택, 파일 디스크립터, 종료 상태를 가진 실행 단위가 되면 process입니다. 그 process 안에서 CPU가 실제로 실행하는 흐름이 thread입니다.

일반 운영체제에서는 하나의 process 안에 여러 thread가 있을 수 있습니다. 하지만 PintOS Project 2에서는 멀티스레드 사용자 프로세스를 지원하지 않습니다. 즉, 사용자 process 하나가 thread 하나를 가진다고 보면 됩니다.

## 4. 왜 파일 시스템이 필요한가

Project 1에서는 테스트 코드가 커널 안에 들어 있었기 때문에 별도의 파일 시스템이 중요하지 않았습니다. Project 2에서는 사용자 프로그램이 별도 실행 파일로 존재합니다. 이 실행 파일을 PintOS가 읽으려면 파일 시스템이 필요합니다.

전체 흐름은 다음과 같습니다.

```text
테스트 C 코드
-> 컴파일된 사용자 실행 파일
-> PintOS 파일 시스템에 복사
-> PintOS 커널이 파일 시스템에서 실행 파일을 읽음
-> ELF loader가 메모리에 배치
```

대표 명령은 다음 형태입니다. 디스크 파일을 직접 만들어 쓰는 경우에는 먼저 파일 시스템 디스크를 만들고, 그 디스크를 `--fs-disk`로 지정합니다.

```sh
pintos-mkdisk filesys.dsk 10
pintos --fs-disk filesys.dsk -p tests/userprog/args-single:args-single -- -q -f run 'args-single onearg'
```

임시 파일 시스템을 쓰는 테스트 실행에서는 다음처럼 `--filesys-size=10` 형태도 사용할 수 있습니다.

```sh
pintos --filesys-size=10 -p tests/userprog/args-single:args-single -- -q -f run 'args-single onearg'
```

여기서 앞쪽의 `-p tests/userprog/args-single:args-single`는 사용자 프로그램을 PintOS 파일 시스템에 넣는 단계이고, 뒤쪽의 `run 'args-single onearg'`는 PintOS 안에서 그 프로그램을 실행하는 단계입니다. `-f`는 파일 시스템 포맷, `-q`는 실행 후 종료를 의미합니다.

주의할 점은 이번 주의 핵심이 파일 시스템 자체 구현은 아니라는 것입니다. 원문도 `filesys` 디렉터리의 파일 시스템 코드를 수정하지 않는 것을 권장합니다. 다만 파일 시스템의 제한은 이해해야 합니다.

- 내부 동기화가 없습니다.
- 동시에 접근하면 서로 간섭할 수 있습니다.
- 파일 크기는 생성 시점에 고정됩니다.
- 하위 디렉터리가 없습니다.
- 파일 이름 길이에 제한이 있습니다.

따라서 파일 관련 syscall을 구현할 때는 파일 시스템 코드를 critical section으로 보고 lock으로 보호해야 합니다. 또한 `process_exec()`도 실행 파일을 여는 과정에서 파일 시스템에 접근하므로, 같은 동기화 원칙을 적용해야 합니다.

## 5. ELF와 실행 파일 로드

PintOS의 user program loader는 x86-64 ELF 실행 파일을 로드하도록 제공되어 있습니다. ELF는 Linux, Solaris 등 여러 운영체제에서 실행 파일, object file, shared library에 사용하는 파일 형식입니다.

ELF는 실행 파일 안에 다음과 같은 정보를 담고 있는 형식이라고 이해하면 됩니다.

- 실행할 코드가 어디에 있는가
- 초기화된 데이터는 어디에 있는가
- BSS 같은 초기값 0인 영역은 어떻게 배치할 것인가
- 사용자 프로그램의 시작점은 어디인가

PintOS의 `load()`는 이 ELF 파일을 읽어 사용자 가상 메모리에 배치합니다. 실행 파일을 단순히 여는 것과, 실행 가능한 형태로 메모리에 올리는 것은 다릅니다.

## 6. 사용자 프로그램 실행 흐름

강의에서 나온 핵심 호출 흐름은 다음과 같습니다. 이 흐름은 `run` 명령으로 처음 사용자 프로그램을 실행할 때의 초기 사용자 프로세스 기준입니다. 이후 `fork`나 `exec` syscall을 구현하면 세부 흐름은 달라질 수 있습니다.

```text
부모 thread:
run_task()
-> tid = process_create_initd(task)
   -> thread_create(..., initd, fn_copy)
-> process_wait(tid)

자식 thread:
kernel_thread()
-> initd(fn_copy)
-> process_exec(fn_copy)
-> load()
-> do_iret()
-> user mode 진입
```

각 단계의 의미는 다음처럼 이해하면 됩니다.

| 단계 | 의미 |
|---|---|
| `run_task()` | `run 'PROG [ARG...]'` 명령을 처리하고 사용자 프로그램 실행 시작 |
| `process_create_initd()` | 초기 사용자 프로세스를 만들기 시작 |
| `thread_create(...)` | 사용자 프로세스를 실행할 커널 thread를 만들고 ready list에 올림 |
| `kernel_thread()` | 새 thread가 스케줄된 뒤 실제 thread 함수인 `initd(fn_copy)` 호출 |
| `initd()` | 새 thread에서 사용자 프로그램 실행 준비 |
| `process_exec()` | 실행할 파일 이름과 인자를 바탕으로 실행 이미지 구성 |
| `load()` | ELF 파일을 읽어 사용자 메모리에 배치 |
| `do_iret()` | user mode로 진입 |
| `process_wait(tid)` | 부모 thread가 초기 사용자 프로세스 종료를 기다리도록 의도된 호출 |

여기서 주의할 점은 `thread_create()`가 `initd()`를 그 자리에서 바로 실행하는 함수가 아니라는 것입니다. 실제 코드에서는 새 thread의 시작 지점을 `kernel_thread`로 설정하고, 첫 번째 인자 레지스터에는 실행할 함수 `initd`, 두 번째 인자 레지스터에는 `fn_copy`를 넣습니다. 이후 스케줄러가 새 thread를 실행하면 `kernel_thread()`가 `initd(fn_copy)`를 호출합니다.

실제 `run_task()` 코드는 `process_wait(process_create_initd(task))`처럼 한 줄로 쓰여 있습니다. 하지만 C에서는 함수 인자가 먼저 계산되므로 `process_create_initd(task)`가 먼저 실행되고, 그 반환값인 초기 thread id가 `process_wait()`에 전달됩니다. 또한 새 thread는 `thread_create()` 이후 언제든 스케줄될 수 있으므로, 부모와 자식 사이의 순서가 필요하다면 별도 동기화가 필요합니다.

`process_create_initd()`가 `file_name`을 새 page에 복사하는 이유도 중요합니다. 원본 명령줄 문자열을 그대로 넘기면 호출자와 새 thread가 같은 문자열을 동시에 다루는 race가 생길 수 있습니다. 그래서 `fn_copy`를 만들고, 그 복사본을 새 thread의 인자로 넘깁니다.

단, skeleton 코드의 `process_wait()`는 아직 실제 대기를 구현하지 않고 `-1`을 반환합니다. 따라서 코드의 호출 형태는 이미 `process_wait(process_create_initd(task))`이지만, 초기 프로세스가 끝날 때까지 기다리는 동작은 직접 구현해야 합니다.

`process_exec()`는 사용자 프로그램으로 진입하기 전에 `struct intr_frame`을 준비합니다. 여기에는 user mode에서 사용할 segment selector, flag, instruction pointer, stack pointer 같은 값이 들어갑니다. 그다음 기존 실행 context를 정리하고 `load(file_name, &_if)`를 호출합니다.

`load()`는 단순히 파일을 여는 함수가 아닙니다. 실제로는 다음 작업을 수행합니다.

- 새 page table인 `pml4`를 만들고 활성화합니다.
- 실행 파일을 `filesys_open(file_name)`으로 엽니다.
- ELF header를 읽고 유효한 실행 파일인지 검사합니다.
- program header를 순회하면서 load 가능한 segment를 사용자 가상 메모리에 올립니다.
- 사용자 stack을 만들고 초기 stack pointer를 설정합니다.
- `if_->rip = ehdr.e_entry`로 사용자 프로그램의 시작 주소를 설정합니다.

마지막의 `do_iret(&_if)`는 일반적인 함수 호출처럼 다시 돌아오는 단계가 아닙니다. `intr_frame`에 들어 있는 값을 CPU 레지스터로 복원한 뒤 `iretq` 명령으로 kernel mode에서 user mode로 전환합니다. 이 시점부터 CPU는 ELF entry point에서 사용자 프로그램 코드를 실행합니다.

중요한 점은 `load()` 전에 명령줄을 어떻게 다룰지 결정해야 한다는 것입니다. 예를 들어 `args-single onearg`에서 실제 실행 파일 이름은 `args-single`이고, `onearg`는 사용자 프로그램의 인자입니다. 파일을 열 때 전체 문자열을 파일 이름으로 보면 실패합니다.

따라서 `process_exec()`에서 명령줄을 파싱하고, 실행 파일 이름과 인자를 분리해야 합니다. 그리고 argument passing 규칙에 맞춰 사용자 스택에 인자를 배치해야 합니다.

## 7. Argument Passing

이번 주 초반에 가장 먼저 이해해야 할 부분입니다. 현재 인자가 없는 프로그램은 실행되는데 인자가 있는 프로그램이 실패한다면, 대개 argument passing이 제대로 구현되지 않았기 때문입니다.

사용자 프로그램은 보통 다음 형태로 시작합니다.

```c
int main(int argc, char *argv[]);
```

하지만 실제 진입점은 `main()`이 아니라 사용자 라이브러리의 `_start()`입니다.

```c
void
_start (int argc, char *argv[]) {
    exit (main (argc, argv));
}
```

커널은 `_start()`가 정상적으로 `main(argc, argv)`를 호출할 수 있도록, 사용자 스택과 레지스터를 준비해야 합니다.

예를 들어 다음 명령을 보겠습니다.

```text
/bin/ls -l foo bar
```

이 명령은 네 개의 단어로 나뉩니다.

```text
argv[0] = "/bin/ls"
argv[1] = "-l"
argv[2] = "foo"
argv[3] = "bar"
argv[4] = NULL
argc = 4
```

사용자 스택의 개념적 모양은 다음과 같습니다.

```text
높은 주소
+-----------------------------+
| "bar\0"                     |  문자열들
| "foo\0"                     |
| "-l\0"                      |
| "/bin/ls\0"                 |
+-----------------------------+
| padding                     |  8바이트 정렬
+-----------------------------+
| NULL                        |  argv[4]
| &"bar\0"                    |  argv[3]
| &"foo\0"                    |  argv[2]
| &"-l\0"                     |  argv[1]
| &"/bin/ls\0"                |  argv[0]
+-----------------------------+
| fake return address = 0     |
+-----------------------------+ <- 초기 stack pointer
낮은 주소

RDI = argc = 4
RSI = argv 배열의 시작 주소
```

핵심은 문자열 자체와 문자열을 가리키는 포인터 배열이 모두 필요하다는 점입니다. `argv[1]`에는 `"-l"`이라는 문자열 자체가 들어 있는 것이 아니라, `"-l"`이 놓인 사용자 스택 주소가 들어 있습니다.

디버깅할 때는 `hex_dump()`로 스택에 값이 의도대로 들어갔는지 확인할 수 있습니다. 다만 최종 제출에는 불필요한 출력이 남으면 안 됩니다.

## 8. Kernel Mode와 User Mode

사용자 프로그램에게 모든 권한을 주면 안 됩니다. 사용자 프로그램이 커널 메모리를 마음대로 쓰거나, 장치를 직접 제어하거나, 다른 프로세스의 메모리를 건드릴 수 있으면 운영체제 전체가 무너질 수 있습니다.

그래서 CPU와 OS는 kernel mode와 user mode를 나눕니다.

| 구분 | 가능한 일 |
|---|---|
| kernel mode | 모든 메모리 접근, 장치 제어, 프로세스 관리 |
| user mode | 제한된 사용자 메모리 접근, 직접 장치 제어 불가 |

PintOS에서 중요한 전환은 두 가지입니다.

| 전환 | 의미 |
|---|---|
| `iretq` | kernel mode에서 user mode로 내려감 |
| `syscall` | user mode에서 kernel mode로 들어옴 |

사용자 프로그램이 직접 화면, 키보드, 파일 시스템 같은 공유 자원을 만지면 안 됩니다. 대신 system call을 통해 커널에게 요청해야 합니다.

## 9. `printf()`와 `write`

강의에서 `printf()`가 언급되었습니다. 사용자 프로그램의 `printf()`는 단순히 커널의 `printf()`를 호출하는 것이 아닙니다. 사용자 프로그램은 커널 함수를 직접 호출할 수 없습니다.

사용자 프로그램의 출력 흐름은 다음처럼 이해하면 됩니다.

```text
user printf()
-> 사용자 라이브러리 내부에서 write syscall 호출
-> syscall handler 진입
-> write(fd=1, buffer, size) 처리
-> 콘솔 출력
```

fd 1은 standard output입니다. 따라서 `write(1, buffer, size)`를 구현해야 사용자 프로그램의 `printf()` 출력도 정상적으로 보일 수 있습니다.

현재 skeleton 코드의 `syscall_handler()`는 아직 syscall 번호를 해석하지 않습니다. 기본 상태에서는 `"system call!"`을 출력하고 `thread_exit()`으로 종료합니다. 따라서 사용자 `printf()`가 제대로 동작하려면 먼저 syscall 번호를 `rax`에서 읽고, `SYS_WRITE`일 때 `write(fd=1, buffer, size)`를 처리하도록 만들어야 합니다.

x86-64 PintOS에서 syscall 번호는 `rax`에 들어오고, syscall 인자는 `rdi`, `rsi`, `rdx`, `r10`, `r8`, `r9` 순서로 전달됩니다. 따라서 `write(fd, buffer, size)`를 처리할 때는 `fd`, `buffer`, `size`가 각각 `rdi`, `rsi`, `rdx`에 들어온다고 이해하면 됩니다.

커널 내부에서 쓰는 `printf()`와 사용자 프로그램의 `printf()`는 구분해야 합니다. 커널의 `printf()`는 커널 코드에서 바로 사용할 수 있지만, 사용자 프로그램의 `printf()`는 system call 구현에 의존합니다.

## 10. Virtual Memory와 Pointer Validation

A 프로그램의 주소 `0x100`과 B 프로그램의 주소 `0x100`은 같은 숫자처럼 보일 수 있습니다. 하지만 각 process는 독립적인 사용자 가상 메모리를 가지므로 실제로는 서로 다른 메모리를 가리킬 수 있습니다.

PintOS의 주소 공간은 크게 두 영역으로 나뉩니다.

```text
낮은 주소                                               높은 주소
0                         KERN_BASE
+-------------------------+-----------------------------+
| user virtual memory     | kernel virtual memory       |
| 프로세스마다 달라짐     | 모든 프로세스에서 공통     |
+-------------------------+-----------------------------+
```

사용자 프로그램이 syscall을 호출하면서 포인터를 넘길 수 있습니다. 예를 들어 `write(fd, buffer, size)`의 `buffer`는 사용자 메모리 주소입니다. 커널은 이 주소를 그대로 믿으면 안 됩니다.

검증해야 할 대표 사례는 다음과 같습니다.

- null pointer
- kernel address
- 매핑되지 않은 user address
- 시작 주소는 유효하지만 buffer 중간에 invalid page가 있는 경우

syscall 번호와 정수 인자는 레지스터로 전달되므로, 그 값을 읽는 것 자체가 사용자 메모리 접근은 아닙니다. 하지만 `buffer`, `cmd_line`, 파일 이름 문자열처럼 포인터 인자가 가리키는 데이터는 사용자 가상 메모리에 있으므로 반드시 검증해야 합니다.

특히 문자열 포인터는 시작 주소 하나만 유효하다고 충분하지 않습니다. `open(file)`의 파일 이름이나 `exec(cmd_line)`의 명령줄은 null 문자 `\0`을 만날 때까지 계속 읽어야 하므로, 문자열이 여러 바이트나 여러 페이지에 걸쳐 있을 수 있습니다. 따라서 문자열 전체를 읽는 과정에서도 커널 주소, unmapped address, page boundary를 조심해야 합니다.

잘못된 사용자 포인터를 만나면 커널 전체가 죽으면 안 됩니다. 해당 사용자 프로세스만 종료하고, 이미 잡은 lock이나 할당한 메모리는 정리해야 합니다.

## 11. `fork`, `exec`, `wait`, `process_wait()`

강의에서 `fork()`, `exec()`, `wait()`은 어렵다고 강조되었습니다. 이 세 함수는 단순한 syscall이 아니라 process 관계와 자원 정리를 포함합니다.

| syscall | 의미 |
|---|---|
| `fork(thread_name)` | 현재 process를 복제해 자식 process 생성 |
| `exec(cmd_line)` | 현재 process의 실행 이미지를 새 프로그램으로 교체 |
| `wait(pid)` | 자식 process가 종료될 때까지 기다리고 exit status 회수 |

특히 `wait`는 parent-child 관계를 정확히 관리해야 합니다.

- 직계 자식만 기다릴 수 있습니다.
- 같은 자식을 두 번 wait할 수 없습니다.
- 자식이 먼저 죽어도 종료 상태를 보관해야 합니다.
- 부모가 먼저 죽어도 자식 자원이 언젠가 정리되어야 합니다.
- 자식이 예외로 종료되면 `wait(pid)`는 `-1`을 반환해야 합니다.

또 하나 중요한 함수가 `process_wait()`입니다. `process_wait()`의 기본 대기 구조는 초기 사용자 프로세스가 종료될 때까지 PintOS가 먼저 끝나지 않도록 하기 위해 필요할 수 있습니다. `argument passing`과 `printf()`를 어느 정도 구현했는데도 테스트가 계속 이상하게 끝난다면, 이 부분이 아직 제대로 구현되지 않았을 가능성을 확인해야 합니다.

다만 `process_wait()`와 사용자 syscall인 `wait(pid)`는 구분해서 이해해야 합니다. `process_wait()`의 기초 구조는 비교적 앞에서 필요할 수 있지만, `wait(pid)`의 완전한 동작은 `fork`로 만들어진 직계 자식 관계, exit status 보관, 중복 wait 방지까지 함께 구현되어야 검증할 수 있습니다.

현재 skeleton 기준으로 `process_fork()`는 `thread_create(name, PRI_DEFAULT, __do_fork, thread_current())` 형태로 새 thread를 만드는 틀만 제공합니다. 부모의 주소 공간 복제, 파일 디스크립터 복제, fork 성공/실패 동기화, 자식에서 반환값을 0으로 만드는 처리는 구현해야 할 영역입니다. `process_wait()`도 skeleton에서는 아직 실제 대기를 하지 않으므로, 이 섹션은 “현재 코드가 이미 완성한 동작”이 아니라 “최종적으로 만족해야 할 동작”으로 읽어야 합니다.

## 12. 동기화와 세마포어

Project 2에는 동기화 이슈가 많습니다. 특히 가끔 통과하고 가끔 실패하는 테스트가 있다면 기능 자체보다 synchronization 문제가 의심됩니다.

대표적으로 필요한 동기화는 다음과 같습니다.

- 파일 시스템 접근을 보호하는 lock
- 부모가 자식의 load 성공/실패를 기다리는 동기화
- 부모가 자식의 exit status를 기다리는 동기화
- 자식과 부모 중 누가 먼저 종료되더라도 자원을 안전하게 정리하는 구조

세마포어는 어떤 사건이 일어날 때까지 기다리거나, 사건이 끝났다고 알려 줄 때 자주 쓰입니다. 예를 들어 부모가 자식의 load 결과를 기다려야 한다면, 자식이 load를 끝낸 뒤 세마포어를 올리고 부모가 그때 깨어나는 구조를 생각할 수 있습니다.

## 13. 테스트를 어떻게 볼 것인가

이번 주는 테스트 수가 많고, 테스트 이름만 봐도 요구사항을 어느 정도 추측할 수 있습니다. 기능 단위로 그룹핑해서 처리하는 것이 좋습니다.

| 테스트 묶음 | 의미 |
|---|---|
| `args-*` | argument passing |
| `halt`, `exit` | 기본 syscall |
| `create`, `open`, `close` | 파일 생성과 fd table |
| `read`, `write` | 파일 입출력과 user buffer 검증 |
| `fork`, `exec`, `wait` | process 관계 |
| `bad-*` | 잘못된 포인터와 비정상 입력 |
| `*-boundary` | page boundary에 걸친 인자와 buffer 검증 |
| `multi-*` | 여러 프로세스 실행, 반복 실행, fd 상속과 자원 정리 |
| `multi-oom` | 메모리 부족 상황에서 실패 경로와 자원 정리 |
| `rox-*` | 실행 중인 파일 쓰기 금지 |

Project 2 userprog 테스트 목록에는 `syn-*` 묶음이 직접 포함되어 있지는 않습니다. `syn-*`는 주로 파일 시스템 프로젝트 쪽 테스트에서 더 많이 보입니다. 이번 주에는 `multi-*`, `multi-oom`, `rox-*`, `bad-*`, `*-boundary` 계열을 통해 동시성, 자원 정리, 잘못된 포인터 처리를 확인한다고 보는 편이 더 정확합니다.

out-of-memory 계열 테스트는 특히 어렵습니다. 단순히 메모리가 부족한 상황이 아니라, 실패 경로에서 자원을 제대로 정리하는지까지 봅니다.

## 14. 구현 우선순위

강의 내용을 바탕으로 현실적인 구현 순서를 잡으면 다음과 같습니다.

1. `argument passing`
2. syscall handler 기본 구조, syscall 번호와 인자 읽기
3. `halt`, `exit`
4. user pointer validation 기초
5. `write(fd=1, ...)`로 사용자 `printf()` 확인
6. fd table과 파일 syscall 기초(`create`, `open`, `close`, `remove`, `filesize`, `seek`, `tell`)
7. `read`, `write` 일반화
8. 초기 프로세스를 위한 `process_wait()` 기초 구조
9. `exec`와 load 성공/실패 동기화
10. `fork`와 `wait(pid)` 완성
11. 동시성, edge case, out-of-memory, `rox-*`

정답 순서는 아니지만, `write(fd=1, ...)`도 사용자 `buffer` 포인터를 받으므로 user pointer validation 기초를 먼저 잡는 편이 안전합니다. 또한 `process_wait()`의 기초 구조와 사용자 syscall `wait(pid)` 완성은 분리해서 보는 것이 좋습니다. 전자는 초기 프로세스가 끝날 때까지 PintOS가 기다리게 하는 데 필요하고, 후자는 `fork` 이후 부모-자식 관계와 exit status 전달까지 구현되어야 제대로 검증할 수 있습니다.

## 15. 강의 후 바로 확인할 질문

강의 복습 후에는 다음 질문에 답할 수 있어야 합니다.

- Project 1 테스트와 Project 2 테스트는 실행 방식이 어떻게 다른가?
- program, process, thread는 어떻게 다른가?
- PintOS에서 process 하나와 thread 하나가 거의 대응되는 이유는 무엇인가?
- 사용자 프로그램 실행 파일은 왜 PintOS 파일 시스템에 복사되어야 하는가?
- ELF loader는 무엇을 하는가?
- `process_exec()` 전에 왜 명령줄을 파싱해야 하는가?
- `argc`, `argv`, `RDI`, `RSI`는 어떻게 연결되는가?
- 사용자 프로그램의 `printf()`가 왜 `write` syscall에 의존하는가?
- user pointer validation을 하지 않으면 어떤 문제가 생기는가?
- `process_wait()`이 구현되지 않으면 왜 테스트가 이상하게 끝날 수 있는가?
- 가끔 통과하고 가끔 실패하는 테스트가 왜 동기화 문제일 가능성이 높은가?
