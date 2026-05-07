# User Programs: Project 2 테스트 전략

## 1. 테스트는 구현 순서대로 좁게 돌린다

Project 2 테스트는 한 번에 전체를 돌리면 실패 원인을 찾기 어렵다. 기능 의존성이 강하기 때문이다.

권장 원칙:

```text
작은 기능 하나 구현
  -> 관련 테스트 1~3개 실행
  -> 실패 원인 확인
  -> 같은 묶음 테스트 확장
  -> 회귀 테스트
```

## 2. 기본 확인

먼저 userprog build가 되는지 확인한다.

```bash
cd /workspaces/week09-team-07-pintos-threads/pintos/userprog
make
```

특정 테스트만 실행:

```bash
make check TESTS='tests/userprog/args-none'
```

여러 테스트 실행:

```bash
make check TESTS='tests/userprog/args-none tests/userprog/args-single'
```

## 3. 권장 테스트 순서

### 1단계: argument passing

```text
args-none
args-single
args-multiple
args-many
args-dbl-space
```

확인 포인트:

- `argc` 값
- `argv[i]` 문자열
- `argv[argc] == NULL`
- `argv` 8바이트 정렬

### 2단계: 기본 syscall

```text
halt
exit
```

확인 포인트:

- `SYS_HALT`가 `power_off()`를 호출하는가
- `SYS_EXIT`가 exit status를 저장하는가
- 종료 메시지 형식이 맞는가

### 3단계: user pointer validation

```text
create-null
create-bad-ptr
create-bound
open-null
open-bad-ptr
open-boundary
read-bad-ptr
read-boundary
write-bad-ptr
write-boundary
bad-read
bad-write
bad-jump
```

확인 포인트:

- 커널 panic이 나지 않는가
- 해당 process만 `exit(-1)` 하는가
- boundary를 걸친 주소를 처리하는가

### 4단계: file syscall

```text
create-normal
open-normal
close-normal
read-normal
write-normal
```

이후 bad fd, zero size, stdin/stdout 테스트로 넓힌다.

### 5단계: exec

```text
exec-once
exec-arg
exec-missing
```

확인 포인트:

- exec 성공 시 호출자 코드로 돌아오지 않는가
- exec load 실패가 현재 process의 `exit(-1)`로 처리되는가
- exec 인자 문자열이 새 user stack에 전달되는가

### 6단계: fork 기본

```text
fork-once
```

확인 포인트:

- parent의 fork 반환값이 child tid인가
- child의 fork 반환값이 0인가
- child가 parent의 user context를 이어받는가

### 7단계: wait

```text
wait-simple
wait-twice
wait-killed
wait-bad-pid
```

확인 포인트:

- wait이 직접 자식만 허용하는가
- 같은 child를 두 번 wait하면 두 번째는 `-1`인가
- exit status가 정확히 회수되는가

### 8단계: fork/multi/rox

```text
fork-multiple
fork-recursive
multi-recurse
multi-child-fd
rox-simple
rox-child
rox-multichild
```

확인 포인트:

- child의 fork 반환값이 0인가
- parent와 child address space가 독립적인가
- fd table 복제가 올바른가
- executable write deny가 유지되는가

## 4. 실패 로그 읽기

각 테스트는 보통 build directory 아래에 결과 파일을 남긴다.

```text
pintos/userprog/build/tests/userprog/<test>.output
pintos/userprog/build/tests/userprog/<test>.errors
pintos/userprog/build/tests/userprog/<test>.result
```

확인 순서:

1. `.result`에서 pass/fail 확인
2. `.output`에서 실제 출력 확인
3. `.errors`에서 kernel panic, page fault, timeout 확인

## 5. 실패 유형별 의심 지점

| 증상 | 의심 지점 |
|------|------|
| `argc`, `argv` 출력이 다름 | argument passing stack layout |
| 출력이 전혀 없음 | `SYS_WRITE`, `process_wait()` 임시 흐름 |
| 종료 메시지 status가 틀림 | `SYS_EXIT`, `process_exit_with_status()` |
| kernel panic | user pointer validation 누락 |
| timeout | wait/semaphore deadlock |
| wait가 항상 -1 | parent-child list 또는 waited 처리 |
| exec-missing 실패 | load completion synchronization |
| fork child 반환값 이상 | child intr_frame `RAX` 설정 |
| rox 실패 | `exec_file`, `file_deny_write()` |

## 6. PR 전 체크리스트

- 관련 테스트 이름을 PR 설명에 적었는가
- 실패했던 테스트와 원인을 기록했는가
- 새 helper의 소유 계층이 명확한가
- user pointer 검증 실패 시 lock이 남지 않는가
- process 종료 경로에서 fd/child/exec_file 정리가 되는가
- 임시 구현이 남아 있으면 대체 계획을 적었는가

## 7. 확인 질문

1. 전체 `make check`보다 작은 TESTS 묶음을 먼저 돌리는 이유는 무엇인가?
2. timeout과 page fault는 디버깅 접근이 어떻게 다른가?
3. `exec-missing` 실패는 왜 wait 구현 문제가 아니라 load synchronization 문제일 수 있는가?
4. rox 테스트를 돌리기 전 어떤 필드가 thread에 있어야 하는가?
