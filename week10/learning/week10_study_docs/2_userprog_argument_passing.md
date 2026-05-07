# User Programs: Argument Passing 학습 정리

## 1. argument passing이 무엇인가

argument passing은 커널이 사용자 프로그램을 시작하기 전에 `argc`, `argv`를 준비해 주는 작업이다.

사용자 프로그램은 보통 다음 함수에서 시작한다고 생각하기 쉽다.

```c
int main (int argc, char *argv[])
```

하지만 Pintos user program은 실제로 `main()`에서 바로 시작하지 않는다. user library의 `_start()` wrapper에서 시작한다.

```c
void
_start (int argc, char *argv[]) {
	exit (main (argc, argv));
}
```

커널은 진짜로 `_start(argc, argv)`를 `call`하지 않는다. 대신 `_start(argc, argv)`가 호출된 것처럼 CPU 레지스터와 user stack을 미리 꾸며 둔다. 이 작업이 argument passing이다.

## 2. 명령어가 들어오면 무엇이 되어야 하나

예를 들어 다음 명령어가 들어왔다고 하자.

```text
echo hello new world happy 2026years
```

이 문자열은 공백 기준으로 6개의 토큰으로 나뉜다.

```text
echo
hello
new
world
happy
2026years
```

사용자 프로그램에서는 다음처럼 보여야 한다.

```text
argc = 6

argv[0] = "echo"
argv[1] = "hello"
argv[2] = "new"
argv[3] = "world"
argv[4] = "happy"
argv[5] = "2026years"
argv[6] = NULL
```

여기서 `argc`는 실제 문자열 인자의 개수다. 마지막 `argv[6] = NULL`은 인자가 하나 더 있다는 뜻이 아니라, `argv` 배열이 끝났다는 표시다.

## 3. RDI, RSI, RSP는 무엇인가

`RDI`, `RSI`, `RSP`는 CPU 안에 있는 저장 공간인 레지스터다.

레지스터는 메모리보다 훨씬 작고 빠른 저장 공간이다. 함수 호출, 주소 계산, 연산 결과 저장 등에 사용된다.

Pintos KAIST는 x86-64 환경이다. x86-64 함수 호출 규약에서는 함수의 첫 번째 인자와 두 번째 인자를 주로 레지스터로 전달한다.

```text
RDI = 첫 번째 함수 인자
RSI = 두 번째 함수 인자
RSP = 현재 stack pointer
```

`_start(argc, argv)` 관점에서 보면 다음과 같다.

```text
RDI = argc
RSI = argv
RSP = user stack의 현재 위치
```

즉 커널은 사용자 프로그램을 시작하기 전에 `intr_frame` 안의 값을 다음처럼 준비해야 한다.

```text
if_->R.rdi = argc
if_->R.rsi = argv 배열의 시작 주소
if_->rsp   = 최종 user stack pointer
```

중요한 점은 `RSI`가 문자열 자체를 담는 것이 아니라는 점이다. `RSI`는 `argv` 배열의 시작 주소를 담는다.

```text
RSI
 |
 v
argv[0] ----> "echo"
argv[1] ----> "hello"
argv[2] ----> "new"
argv[3] ----> "world"
argv[4] ----> "happy"
argv[5] ----> "2026years"
argv[6] ----> NULL
```

`RSP`는 stack pointer다. user stack은 높은 주소에서 낮은 주소 방향으로 자란다. 데이터를 stack에 넣을 때마다 `RSP`는 더 낮은 주소로 이동한다.

## 4. 왜 argument passing이 없으면 page fault가 나는가

argument passing이 없으면 `argc`, `argv`가 올바르게 설정되지 않는다.

Pintos 테스트 프로그램은 시작하자마자 다음과 같은 코드를 실행할 수 있다.

```c
test_name = argv[0];
```

이때 `argv`가 `NULL`이거나 잘못된 주소이면 사용자 프로그램이 잘못된 메모리를 읽게 된다. 그 결과 page fault가 발생한다.

그래서 `SYS_WRITE`나 `SYS_EXIT`를 작성해도, argument passing이 없으면 사용자 프로그램이 syscall까지 도달하지 못할 수 있다.

## 5. 실행 파일 이름과 argv[0]의 관계

다음 명령어를 보자.

```text
args-single onearg
```

사용자 프로그램에서는 다음처럼 보여야 한다.

```text
argc = 2
argv[0] = "args-single"
argv[1] = "onearg"
argv[2] = NULL
```

여기서 실행 파일 이름은 첫 번째 토큰인 `args-single`이다.

`load()`나 `filesys_open()`에서 사용할 이름도 `args-single`이어야 한다.

반대로 user stack에는 `args-single`과 `onearg`를 모두 올려야 한다. `argv[0]`에는 실행 파일 이름도 포함되기 때문이다.

정리하면 다음과 같다.

```text
실행 파일 open에 사용할 이름 = 첫 번째 토큰
user stack에 올릴 argv       = 모든 토큰
```

## 6. 전체 command line으로 load하면 안 되는 이유

다음 문자열 전체를 그대로 실행 파일 이름으로 사용한다고 가정하자.

```text
args-single onearg
```

그러면 Pintos는 파일 시스템에서 다음 이름의 실행 파일을 찾으려고 한다.

```text
args-single onearg
```

하지만 실제 파일 이름은 다음이다.

```text
args-single
```

따라서 command line을 먼저 토큰화하고, `load()`에는 첫 번째 토큰만 넘겨야 한다.

다만 현재 repo 구현처럼 `load()`가 전체 command line을 받은 뒤 내부에서 토큰화하고 `filesys_open(tmp_argv[0])`를 호출하는 구조도 가능하다. 중요한 것은 `filesys_open()`에 전체 command line이 아니라 첫 번째 토큰만 들어가야 한다는 점이다.

## 7. command line 토큰화

토큰화는 하나의 문자열을 여러 단어로 나누는 작업이다.

```text
"args-single onearg"
```

는 다음처럼 나뉜다.

```text
argv[0] = "args-single"
argv[1] = "onearg"
argc = 2
```

여러 공백은 하나의 구분자로 취급해야 한다.

```text
args-dbl-space two  spaces!
```

는 다음과 같아야 한다.

```text
argv[0] = "args-dbl-space"
argv[1] = "two"
argv[2] = "spaces!"
argc = 3
```

Pintos에는 `strtok_r()`가 제공되어 있다. `strtok_r()`는 문자열을 직접 잘라내므로, 원본 command line을 계속 보존해야 한다면 복사본을 만들어 사용해야 한다.

## 8. user stack에 문자열을 복사한다는 뜻

사용자 프로그램은 커널 메모리에 있는 문자열을 직접 사용할 수 없다.

따라서 `"args-single"`이나 `"onearg"` 같은 문자열은 사용자 주소 공간의 stack에 복사해야 한다.

초기 stack pointer는 `USER_STACK`이다. stack은 아래 방향으로 자라므로 데이터를 넣을 때마다 stack pointer를 감소시킨다.

예를 들어:

```text
처음:
rsp = USER_STACK

"onearg\0" 복사:
rsp -= 7

"args-single\0" 복사:
rsp -= 12
```

문자열을 복사한 뒤에는 그 문자열이 놓인 user virtual address를 기록해 둔다. 나중에 `argv[]` 배열을 만들 때 이 주소들이 필요하다.

## 9. argv 배열도 user stack에 만들어야 한다

문자열만 stack에 복사하면 충분하지 않다.

`main()`은 다음 형태를 기대한다.

```c
int main (int argc, char *argv[])
```

`argv`는 문자열 하나가 아니라 문자열 포인터들의 배열이다.

따라서 user stack 안에는 다음 구조가 있어야 한다.

```text
argv[0] -> "args-single"
argv[1] -> "onearg"
argv[2] -> NULL
```

`argv[argc] == NULL`은 C 프로그램에서 흔히 기대하는 관례이며, Pintos 테스트도 이를 확인한다.

## 10. 문자열 영역과 포인터 배열 영역

argument passing에서 stack에는 크게 두 종류의 데이터가 들어간다.

첫 번째는 실제 문자열이다.

```text
"args-single\0"
"onearg\0"
```

두 번째는 그 문자열들을 가리키는 주소 배열이다.

```text
argv[0] = "args-single"이 저장된 주소
argv[1] = "onearg"이 저장된 주소
argv[2] = NULL
```

즉 `argv[0]` 자체가 문자열을 담는 것이 아니다. `argv[0]`은 문자열이 저장된 위치를 가리키는 포인터다.

## 11. 왜 8바이트 정렬이 필요한가

x86-64에서 포인터 크기는 8바이트다.

`argv`는 `char *` 포인터들의 배열이므로 8바이트 단위로 정렬되는 것이 좋다. Pintos 테스트도 `argv` 주소가 8바이트 정렬되어 있는지 확인한다.

```c
if (((unsigned long long) argv & 7) != 0)
	msg ("argv and stack must be word-aligned...");
```

따라서 문자열들을 stack에 복사한 뒤, 포인터 배열을 push하기 전에 stack pointer를 8바이트 경계로 맞춰야 한다.

이때 생기는 `padding`은 의미 있는 데이터가 아니다. 문자열을 복사한 뒤 다음에 놓을 포인터 배열이 8바이트 경계에서 시작하도록 채워 넣는 정렬용 빈 공간이다.

## 12. fake return address

문서에서는 마지막에 fake return address를 push하라고 설명한다.

사용자 프로그램은 `_start()`에서 시작하지만, 실제로 누군가가 `call _start`를 실행한 것은 아니다. 그래도 일반 함수 호출 프레임처럼 보이게 하기 위해 stack에 가짜 return address를 넣는다.

대개 값은 `0`이다.

```text
fake return address = 0
```

이 값이 실제로 사용될 가능성은 낮다. `_start()`는 `main()`이 반환하면 `exit()`를 호출하기 때문이다.

## 13. 전체 stack 예시

명령이 다음과 같다고 하자.

```text
args-single onearg
```

최종 user stack은 개념적으로 다음과 같다.

```text
높은 주소

USER_STACK
| "onearg\0"        |  실제 문자열
| "args-single\0"   |  실제 문자열
| padding           |  8-byte alignment
| NULL              |  argv[2]
| &"onearg"         |  argv[1]
| &"args-single"    |  argv[0]
| fake return addr  |

낮은 주소
```

그리고 레지스터는 다음과 같아야 한다.

```text
RDI = 2
RSI = &argv[0]
RSP = fake return address 위치
```

다시 강조하면, `RSI`는 `"args-single"`의 주소가 아니라 `argv[0]` 칸의 주소다.

`RSP`가 `argv`를 직접 가리키지 않는 이유는 `_start()`가 일반 함수 호출로 시작된 것처럼 stack 모양을 맞추기 위해서다. `argc`와 `argv`는 stack에서 꺼내는 것이 아니라 각각 `RDI`, `RSI` 레지스터로 전달된다. 따라서 최종 `RSP`는 fake return address가 놓인 위치를 가리키고, `RSI`가 따로 `argv` 배열의 시작 주소를 가리킨다.

## 14. 구현 사고 순서

구현할 때는 다음 순서로 생각하면 된다.

1. command line을 복사본 안에서 토큰화한다.
2. 첫 번째 토큰을 실행 파일명으로 사용한다.
3. `load()`를 성공시켜 user stack page를 만든다.
4. 문자열들을 user stack에 복사한다.
5. 각 문자열의 시작 주소를 따로 기록한다.
6. stack pointer를 8바이트 정렬한다.
7. `argv[argc] == NULL`이 되도록 null sentinel을 push한다.
8. 문자열 주소들을 역순으로 push해서 `argv[]` 배열을 만든다.
9. `argv[0]`의 주소를 저장한다.
10. fake return address를 push한다.
11. `RDI = argc`, `RSI = argv`, `RSP = 최종 stack pointer`를 설정한다.

## 15. 흔한 실수

- `load("args-single onearg")`처럼 전체 command line으로 파일을 열려고 한다.
- 문자열은 stack에 복사했지만 `argv[]` 배열을 만들지 않는다.
- `argv[argc] == NULL`을 빠뜨린다.
- `argv` 주소가 8바이트 정렬되지 않는다.
- `argc`, `argv`를 stack에만 두고 `RDI`, `RSI`를 설정하지 않는다.
- `RSI`에 `argv[0]` 문자열 주소를 넣는다. `RSI`에는 `argv` 배열의 시작 주소가 들어가야 한다.
- `process_exec()` 끝에서 kernel page를 해제한 뒤, 그 안의 문자열 주소를 user program이 쓰게 만든다.

## 16. 핵심 문장

argument passing의 핵심은 다음 한 문장으로 정리할 수 있다.

> 커널이 command line을 파싱한 뒤, 사용자 프로그램이 `_start(argc, argv)`로 호출된 것처럼 user stack과 레지스터를 꾸며줘야 한다.
