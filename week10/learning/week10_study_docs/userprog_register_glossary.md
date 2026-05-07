# Pintos User Programs 레지스터 용어집

이 문서는 Pintos Project 2의 `argument passing`, `syscall`, `intr_frame` 흐름을 읽을 때 자주 나오는 레지스터와 관련 개념을 정리한 학습용 문서다.

## 큰 그림

Pintos는 사용자 프로그램을 실행하기 직전에 `struct intr_frame` 안에 레지스터 값을 채운다. 이후 `do_iret()`이 이 값을 실제 CPU 레지스터로 복원하면서 사용자 모드로 진입한다.

argument passing에서는 `_start(argc, argv)`가 이미 호출된 것처럼 아래 값을 만들어야 한다.

```text
RDI = argc
RSI = argv 배열의 사용자 가상 주소
RSP = fake return address가 놓인 사용자 스택 주소
RIP = 사용자 프로그램의 시작 주소
```

## 주요 레지스터

### RDI

`RDI`는 x86-64 호출 규약에서 첫 번째 정수 또는 포인터 인자를 담는 레지스터다.

Pintos argument passing에서는 `_start(int argc, char *argv[])`의 첫 번째 인자인 `argc`를 담는다.

```c
if_->R.rdi = argc;
```

### RSI

`RSI`는 x86-64 호출 규약에서 두 번째 정수 또는 포인터 인자를 담는 레지스터다.

Pintos argument passing에서는 `_start(int argc, char *argv[])`의 두 번째 인자인 `argv`를 담는다. 여기서 중요한 점은 `RSI`에 커널 지역 배열 주소를 넣으면 안 된다는 것이다. 사용자 프로그램이 접근할 수 있는 사용자 스택 안의 `argv[0]` 위치를 넣어야 한다.

```c
if_->R.rsi = if_->rsp;
```

이 코드는 `argv` 포인터 배열을 사용자 스택에 모두 push한 직후에 실행해야 한다. 그 시점의 `if_->rsp`가 사용자 스택 안의 `argv[0]` 주소이기 때문이다.

### RSP

`RSP`는 현재 스택의 맨 위를 가리키는 스택 포인터다.

x86-64에서 스택은 높은 주소에서 낮은 주소 방향으로 자란다. 따라서 값을 push할 때는 보통 먼저 `rsp`를 줄이고, 줄어든 위치에 값을 복사한다.

```c
if_->rsp -= size;
memcpy ((void *) if_->rsp, data, size);
```

argument passing이 끝난 뒤 `RSP`는 fake return address가 들어 있는 사용자 스택 주소를 가리켜야 한다.

### RIP

`RIP`는 다음에 실행할 명령어의 주소를 담는 instruction pointer다.

Pintos의 `load()`는 ELF 실행 파일을 읽은 뒤 entry point를 `RIP`에 넣는다.

```c
if_->rip = ehdr.e_entry;
```

이 값으로 인해 사용자 모드 진입 후 프로그램의 시작 코드가 실행된다.

### RAX

`RAX`는 일반 함수 호출에서 반환값을 담는 레지스터다.

Pintos syscall 흐름에서는 syscall 번호와 반환값에도 관여한다. 사용자 프로그램이 syscall을 호출하면 syscall 번호가 `RAX`에 들어오고, 커널은 처리 결과를 다시 `RAX`에 넣어 사용자 프로그램에 돌려준다.

```c
switch (f->R.rax) {
	case SYS_WRITE:
		f->R.rax = written_size;
		break;
}
```

### RDX, RCX/R10, R8, R9

일반 x86-64 함수 호출 규약에서 세 번째부터 여섯 번째 인자를 담는 레지스터는 다음과 같다.

```text
1번째 인자: RDI
2번째 인자: RSI
3번째 인자: RDX
4번째 인자: RCX
5번째 인자: R8
6번째 인자: R9
```

하지만 Pintos의 user syscall wrapper는 `syscall` 명령을 사용할 때 네 번째 인자를 `RCX`가 아니라 `R10`에 넣는다. `syscall` 명령이 `RCX`를 return address 저장에 사용하기 때문이다.

Pintos syscall handler에서 인자를 읽을 때는 다음 규칙을 따른다.

```text
syscall 번호: RAX
1번째 인자: RDI
2번째 인자: RSI
3번째 인자: RDX
4번째 인자: R10
5번째 인자: R8
6번째 인자: R9
```

예를 들어 `write(fd, buffer, size)` syscall을 처리할 때는 보통 아래처럼 읽는다.

```c
int fd = f->R.rdi;
const void *buffer = (const void *) f->R.rsi;
unsigned size = f->R.rdx;
```

### RFLAGS 또는 EFLAGS

`RFLAGS`는 CPU 상태 플래그를 담는 레지스터다. Pintos 코드에서는 `eflags`라는 이름으로 보인다.

사용자 프로그램을 시작할 때 보통 interrupt enable flag와 must-be-set flag를 설정한다.

```c
if_->eflags = FLAG_IF | FLAG_MBS;
```

### CS, SS, DS, ES

이 레지스터들은 segment selector를 담는다. Pintos에서는 사용자 코드와 사용자 데이터 segment를 지정하기 위해 설정한다.

```c
if_->cs = SEL_UCSEG;
if_->ds = if_->es = if_->ss = SEL_UDSEG;
```

사용자 프로그램이 커널 모드가 아니라 사용자 모드에서 실행되도록 만드는 데 필요한 값이다.

## struct intr_frame

`struct intr_frame`은 사용자 모드로 진입하거나 syscall/interrupt로 커널에 들어올 때 레지스터 상태를 저장하는 구조체다.

관련 정의는 `pintos/include/threads/interrupt.h`에 있다.

```c
struct intr_frame {
	struct gp_registers R;
	uintptr_t rip;
	uint64_t eflags;
	uintptr_t rsp;
	...
};
```

`if_->R.rdi`, `if_->R.rsi`처럼 접근하는 값은 범용 레지스터이고, `if_->rsp`, `if_->rip`은 interrupt frame의 별도 필드다.

## 사용자 스택과 커널 스택

argument passing에서 가장 흔한 실수는 커널 스택에 있는 지역 변수 주소를 사용자 프로그램에 넘기는 것이다.

예를 들어 아래 코드는 위험하다.

```c
char *argv[MAX_ARGV + 1];
if_->R.rsi = argv;
```

`argv` 배열 자체는 커널 스택에 있다. 사용자 프로그램은 이 주소에 접근할 수 없으므로 page fault가 날 수 있다.

올바른 방식은 다음과 같다.

1. 문자열들을 사용자 스택에 복사한다.
2. 각 문자열의 사용자 스택 주소를 커널의 임시 `argv[i]`에 저장한다.
3. `argv[i]` 포인터 값들을 다시 사용자 스택에 복사한다.
4. 사용자 스택 안의 `argv[0]` 주소를 `RSI`에 넣는다.

## fake return address

사용자 프로그램은 실제로 커널의 `call` 명령으로 호출되는 것이 아니지만, `_start(argc, argv)`가 일반 함수처럼 시작된 상태를 만들어야 한다.

그래서 스택 맨 아래쪽에 가짜 반환 주소를 하나 넣는다.

```c
void *fake_ret = NULL;
if_->rsp -= sizeof fake_ret;
memcpy ((void *) if_->rsp, &fake_ret, sizeof fake_ret);
```

fake return address는 `RSI`에 넣는 값이 아니다. `RSI`는 `argv`의 사용자 스택 주소이고, `RSP`가 fake return address 위치를 가리킨다.

## argument passing 예시

명령이 아래와 같다고 하자.

```text
args one two
```

사용자 프로그램 시작 직전에는 개념적으로 아래와 같은 상태가 되어야 한다.

```text
높은 주소

"two\0"
"one\0"
"args\0"
padding
argv[3] = NULL
argv[2] = "two\0"의 사용자 주소
argv[1] = "one\0"의 사용자 주소
argv[0] = "args\0"의 사용자 주소   <- RSI
fake return address = NULL          <- RSP

낮은 주소
```

레지스터 값은 다음과 같다.

```text
RDI = 3
RSI = 사용자 스택 안의 argv[0] 주소
RSP = fake return address 주소
```

## 8바이트 정렬

문자열들을 스택에 복사한 뒤에는 `argv` 포인터들을 push하기 전에 `RSP`를 8바이트 경계에 맞춘다.

스택은 낮은 주소 방향으로 자라므로 현재 주소보다 낮은 8의 배수 주소로 내린다.

```c
uintptr_t padding = if_->rsp % 8;
if_->rsp -= padding;
```

예를 들어 `if_->rsp % 8 == 5`라면 5바이트를 더 내려서 8의 배수 주소에 맞춘다.

