# User Programs: User Memory Validation

## 1. 왜 사용자 포인터를 검증해야 하나

User program은 kernel mode가 아니라 user mode에서 실행된다. 따라서 user program이 syscall로 넘기는 포인터는 커널이 신뢰하면 안 된다.

예를 들어 사용자 프로그램은 다음처럼 잘못된 주소를 syscall 인자로 넘길 수 있다.

```c
write (1, (void *) 0x123, 10);
open ((char *) 0xffffffff80000000);
```

커널이 이런 주소를 바로 `putbuf()`, `filesys_open()`, `memcpy()`에 넘기면 커널 page fault나 panic으로 이어질 수 있다. Project 2의 목표는 잘못된 사용자 프로그램 하나 때문에 OS 전체가 죽지 않게 만드는 것이다.

정책은 단순하다.

```text
잘못된 user pointer 접근
  -> kernel panic 금지
  -> 해당 user process를 exit(-1)로 종료
```

## 2. 검증해야 하는 조건

사용자 주소 검증은 최소한 아래 조건을 확인해야 한다.

| 조건 | 의미 |
|------|------|
| `uaddr != NULL` | NULL 포인터는 거부한다. |
| `is_user_vaddr(uaddr)` | 커널 주소 영역이면 거부한다. |
| `pml4_get_page(cur->pml4, uaddr) != NULL` | 현재 프로세스 page table에 매핑되어 있어야 한다. |
| boundary 확인 | 버퍼가 여러 page에 걸치면 모든 page를 확인한다. |

현재 dev 기준 helper는 [syscall.c](/workspaces/week09-team-07-pintos-threads/pintos/userprog/syscall.c:50)에 있다.

```c
static bool user_addr_mapped (const void *uaddr);
void user_check_ptr (const void *uaddr);
void user_check_read (const void *uaddr, size_t size);
void user_check_write (void *uaddr, size_t size);
void user_check_string (const char *uaddr);
char *user_strdup (const char *uaddr);
```

## 3. 단일 포인터 검증

단일 주소 하나를 읽거나 쓸 수 있는지 확인할 때는 `user_check_ptr()`를 사용한다.

개념적으로는 다음 흐름이다.

```text
uaddr == NULL?
  -> exit(-1)
kernel address?
  -> exit(-1)
page table에 매핑 없음?
  -> exit(-1)
정상
```

`pml4_get_page()`는 user virtual address가 현재 process의 page table에서 실제 physical page에 매핑되어 있는지 확인하는 핵심 함수다.

## 4. 버퍼 검증

`read(fd, buffer, size)`와 `write(fd, buffer, size)`는 단일 포인터가 아니라 연속된 메모리 구간을 사용한다.

예를 들어 `buffer`가 page 끝 2바이트 전에 있고 `size`가 10이면, 실제 접근은 다음 page까지 넘어간다.

```text
page A 끝부분: 2 bytes
page B 시작부분: 8 bytes
```

따라서 시작 주소만 검사하면 부족하다. `buffer`부터 `buffer + size - 1`까지 포함하는 모든 page를 검사해야 한다.

```c
user_check_read (buffer, size);   /* user memory를 kernel이 읽을 때 */
user_check_write (buffer, size);  /* user memory에 kernel이 쓸 때 */
```

이름 기준은 커널 관점이다.

| syscall | user buffer 의미 | 사용할 helper |
|------|------|------|
| `write(fd, buffer, size)` | 커널이 user buffer를 읽어서 파일/stdout에 쓴다 | `user_check_read()` |
| `read(fd, buffer, size)` | 커널이 user buffer에 데이터를 써 준다 | `user_check_write()` |

## 5. 문자열 검증

`open(file)`, `create(file)`, `remove(file)`, `exec(cmd_line)`은 C 문자열을 인자로 받는다.

C 문자열은 길이가 따로 넘어오지 않고 `'\0'`을 만날 때 끝난다. 그래서 문자열 검증은 다음처럼 진행해야 한다.

```text
현재 문자 주소가 유효한가?
  -> 아니면 exit(-1)
현재 문자가 '\0'인가?
  -> 맞으면 성공
다음 문자로 이동
```

문자열이 page boundary를 넘어갈 수 있으므로 문자마다 또는 page마다 안전하게 확인해야 한다.

`user_strdup()`은 user string을 kernel page에 복사할 때 유용하다. `exec()`처럼 원본 user memory가 나중에 바뀌면 안 되는 syscall에서는 user string을 kernel memory로 복사해 두는 편이 안전하다.

## 6. syscall별 적용 위치

| syscall | 검증 대상 |
|------|------|
| `halt()` | 없음 |
| `exit(status)` | 없음 |
| `write(fd, buffer, size)` | `buffer` read 검증 |
| `read(fd, buffer, size)` | `buffer` write 검증 |
| `create(file, size)` | `file` string 검증 |
| `open(file)` | `file` string 검증 |
| `remove(file)` | `file` string 검증 |
| `exec(cmd_line)` | `cmd_line` string 검증 |
| `fork(thread_name)` | `thread_name` string 검증 |

## 7. 주의할 점

검증 후에도 race 가능성은 남는다. user program이 검증 직후 자기 memory mapping을 바꾸는 복잡한 상황은 Project 2 범위에서 제한적으로 다루지만, 기본 원칙은 커널이 user pointer를 사용할 때마다 안전성을 먼저 확인하는 것이다.

또한 `user_check_write()`는 현재 `user_check_read()`와 같은 매핑 검증을 한다. Project 2에서는 writable bit까지 엄격히 확인하지 않아도 대부분 테스트를 통과할 수 있지만, 개념적으로는 쓰기 가능한 user page인지도 고려 대상이다.

## 8. 확인 질문

1. `write()`에서 `user_check_write()`가 아니라 `user_check_read()`를 써야 하는 이유는 무엇인가?
2. `buffer` 시작 주소만 검증하면 boundary 테스트에서 왜 실패할 수 있는가?
3. `open(NULL)`은 어떤 경로로 `exit(-1)`이 되어야 하는가?
4. user pointer 검증 실패 시 `thread_exit()`만 호출하는 것과 `process_exit_with_status(-1)`을 호출하는 것의 차이는 무엇인가?
