# Project 3 VM 테스트 전략

이 문서는 VM 구현을 어떤 순서로 테스트하면 좋은지 정리한다. 테스트는 마지막에 한 번 돌리는 절차가 아니라, 지금 구현한 설계가 맞는지 확인하는 도구다.

## 1. 기본 원칙

VM 테스트는 한 번에 전체를 돌리면 실패 원인을 좁히기 어렵다.

권장 흐름은 다음과 같다.

```text
작은 기능 구현
  -> 가장 가까운 테스트 1~3개 실행
  -> 실패 로그 확인
  -> 같은 묶음 테스트 확장
  -> Project 2 회귀 확인
  -> 다음 기능 구현
```

Project 3는 Project 2 위에 올라가므로, VM 테스트 일부가 통과해도 userprog 회귀가 깨지면 안 된다.

## 2. 빌드 확인

먼저 저장소 루트에서 VM build가 되는지 확인한다.

```bash
cd pintos/vm
make
```

특정 테스트만 실행하려면 다음 형태를 사용한다.

```bash
make check TESTS='tests/vm/pt-grow-stack'
```

여러 테스트를 묶어 실행할 수도 있다.

```bash
make check TESTS='tests/vm/lazy-file tests/vm/lazy-anon'
```

## 3. 1단계: Project 2 기본 회귀

VM 초기 구현 중에도 사용자 프로그램이 기본 실행되는지 확인해야 한다.

추천 테스트:

```text
tests/userprog/args-none
tests/userprog/args-single
tests/userprog/args-multiple
tests/userprog/halt
tests/userprog/exit
```

VM 변경 후에는 user pointer, boundary, fork/exec 회귀도 빠르게 확인해야 한다.

```text
tests/userprog/read-bad-ptr
tests/userprog/write-bad-ptr
tests/userprog/open-bad-ptr
tests/userprog/create-bad-ptr
tests/userprog/read-boundary
tests/userprog/write-boundary
tests/userprog/exec-boundary
tests/userprog/fork-boundary
tests/userprog/exec-once
tests/userprog/exec-read
tests/userprog/fork-once
tests/userprog/fork-read
```

확인 포인트:

- user program이 load되는가?
- argument passing이 여전히 맞는가?
- page fault가 정상 lazy loading으로 처리되는가?
- 종료 메시지와 status가 유지되는가?
- syscall이 user buffer를 page 단위로 안전하게 검증하는가?
- boundary를 걸친 buffer에서 lazy page나 stack growth 후보를 잘못 죽이지 않는가?
- `exec()` 성공 후 이전 address space와 SPT가 정리되는가?
- `fork()`에서 SPT, fd table, register state가 함께 일관되는가?

이 단계가 깨지면 lazy loading, setup_stack, syscall 진입, user pointer validation을 먼저 본다.

## 4. 2단계: Lazy loading

추천 테스트:

```text
tests/vm/lazy-file
tests/vm/lazy-anon
```

확인 포인트:

- `load_segment()`가 page를 즉시 읽지 않고 SPT에 등록하는가?
- page fault 때 `lazy_load_segment()`가 호출되는가?
- 파일 offset, read bytes, zero bytes가 page마다 맞는가?
- aux가 너무 빨리 해제되거나 누수되지 않는가?

실패하면 `.output`에서 page fault 주소와 panic 메시지를 먼저 본다.

## 5. 3단계: Page fault 기본 검증

추천 테스트:

```text
tests/vm/pt-bad-addr
tests/vm/pt-bad-read
tests/vm/pt-write-code
tests/vm/pt-write-code2
```

확인 포인트:

- 잘못된 주소 접근을 복구하려고 하지 않는가?
- kernel address 접근을 거부하는가?
- read-only page에 write하면 process를 종료하는가?
- not-present fault와 protection fault를 구분하는가?

이 단계는 `vm_try_handle_fault()`의 조건문 품질을 확인한다.

## 6. 4단계: Stack growth

추천 테스트:

```text
tests/vm/pt-grow-stack
tests/vm/pt-grow-stk-sc
tests/vm/pt-big-stk-obj
tests/vm/pt-grow-bad
```

확인 포인트:

- fault address를 page boundary로 round down하는가?
- user `rsp` 근처 fault만 stack growth로 인정하는가?
- syscall 경로에서 saved user rsp를 쓰는가?
- 1MB stack limit을 지키는가?
- stack으로 볼 수 없는 주소를 거부하는가?

`pt-grow-bad`가 실패하면 stack growth 조건이 너무 느슨할 가능성이 있다.

## 7. 5단계: Fork와 page 복사

추천 테스트:

```text
tests/vm/page-linear
tests/vm/page-parallel
tests/vm/page-merge-seq
tests/vm/page-merge-par
tests/vm/page-merge-stk
tests/vm/page-merge-mm
```

확인 포인트:

- `supplemental_page_table_copy()`가 page type별로 복사하는가?
- child가 parent와 같은 초기 메모리 내용을 보는가?
- child 수정이 parent를 오염시키지 않는가?
- uninit page aux가 parent/child에서 독립적인가?
- fork 실패 시 부분 생성 자원이 정리되는가?

Project 2의 `fork-*`, `multi-*` 테스트도 함께 회귀 확인해야 한다.

## 8. 6단계: mmap

추천 테스트:

```text
tests/vm/mmap-read
tests/vm/mmap-close
tests/vm/mmap-unmap
tests/vm/mmap-write
tests/vm/mmap-ro
tests/vm/mmap-exit
tests/vm/mmap-shuffle
```

테스트를 볼 때는 이름을 한 묶음으로 외우기보다, 무엇을 검증하는지로 나누는 편이 좋다.

invalid argument와 overlap:

```text
tests/vm/mmap-bad-fd
tests/vm/mmap-bad-fd2
tests/vm/mmap-bad-fd3
tests/vm/mmap-bad-off
tests/vm/mmap-kernel
tests/vm/mmap-misalign
tests/vm/mmap-null
tests/vm/mmap-over-code
tests/vm/mmap-over-data
tests/vm/mmap-over-stk
tests/vm/mmap-overlap
tests/vm/mmap-zero
tests/vm/mmap-zero-len
```

offset 처리:

```text
tests/vm/mmap-bad-off
tests/vm/mmap-off
```

`mmap-bad-off`는 page-aligned가 아닌 offset 실패를 보고, `mmap-off`는 page-aligned인 0x1000 offset 매핑이 성공해야 함을 본다. 둘을 모두 invalid로 묶으면 의도를 착각하기 쉽다.

lifetime과 file reference:

```text
tests/vm/mmap-close
tests/vm/mmap-remove
tests/vm/mmap-exit
tests/vm/mmap-inherit
```

- `mmap-close`: fd를 close해도 mapping은 유지되어야 한다.
- `mmap-remove`: 파일을 remove해도 이미 만든 mapping은 유지되어야 한다.
- `mmap-exit`: process exit 때 implicit unmap과 dirty write-back이 되어야 한다.
- `mmap-inherit`: fork 후 exec된 child에게 parent의 old mmap mapping이 보이면 안 된다.

dirty/write-back:

```text
tests/vm/mmap-write
tests/vm/mmap-clean
tests/vm/mmap-ro
```

`mmap-clean`은 invalid case가 아니라 clean page를 munmap할 때 파일에 다시 쓰지 않는지 확인하는 테스트다.

여러 mapping과 큰 범위:

```text
tests/vm/mmap-twice
tests/vm/mmap-shuffle
tests/vm/mmap-unmap
```

확인 포인트:

- `addr`, `length`, `fd`, `offset`, overlap 검증이 맞는가?
- fd close 후에도 mapping이 유지되는가?
- dirty page만 write-back하는가?
- 마지막 page의 zero 영역을 파일에 쓰지 않는가?
- process exit 시 implicit unmap이 되는가?

## 9. 7단계: Eviction과 swap

추천 테스트:

```text
tests/vm/page-shuffle
tests/vm/swap-file
tests/vm/swap-anon
tests/vm/swap-iter
tests/vm/swap-fork
```

확인 포인트:

- frame table에 user frame이 모두 등록되는가?
- victim selection이 무한 루프에 빠지지 않는가?
- eviction 후 pml4 mapping을 제거하는가?
- anonymous page는 dirty 여부와 무관하게 eviction 시 현재 내용을 swap disk에 저장하는가?
- swap in 후 slot을 해제하는가?
- file-backed page는 dirty일 때만 write-back하는가?

swap 테스트는 앞 단계가 불안정하면 실패 원인이 섞인다. lazy, stack, mmap이 먼저 안정되어야 한다.

## 10. 실패 로그 보는 순서

Pintos 테스트 결과는 보통 build directory 아래에 생긴다.

```text
pintos/vm/build/tests/vm/<test>.result
pintos/vm/build/tests/vm/<test>.output
pintos/vm/build/tests/vm/<test>.errors
```

확인 순서:

1. `.result`에서 pass/fail 확인
2. `.output`에서 사용자 프로그램 출력 확인
3. `.errors`에서 kernel panic, page fault, timeout 확인
4. 최근 수정한 함수와 fault 지점을 연결

## 11. 증상별 의심 지점

| 증상 | 의심 지점 |
|------|------|
| 같은 page fault가 반복됨 | pml4 mapping 실패, `swap_in()` 실패 |
| SPT lookup 실패 | `pg_round_down()` 누락, insert 누락 |
| `pt-write-code` 실패 | writable 검증 누락 |
| `pt-grow-bad` 실패 | stack growth 조건이 너무 느슨함 |
| `mmap-close` 실패 | `file_reopen()` 또는 file lifetime 문제 |
| `mmap-write` 실패 | dirty write-back 누락 |
| `swap-anon` 실패 | swap slot bitmap, sector read/write |
| `swap-fork` 실패 | swapped page 복사 정책 |
| timeout | lock deadlock, eviction victim loop |
| Project 2 args 실패 | setup_stack, lazy load, user pointer validation |

## 12. PR 전 체크리스트

- 어떤 테스트를 통과했는가?
- 어떤 테스트가 아직 실패하는가?
- page/frame/swap/file 중 새로 소유한 자원이 있는가?
- 실패 경로에서 그 자원을 해제하는가?
- page fault를 복구하는 조건과 종료하는 조건이 명확한가?
- Project 2 회귀 테스트를 어느 범위까지 확인했는가?
- mmap이나 fork에서 parent/child/file lifetime이 꼬이지 않는가?

## 13. 확인 질문

1. 전체 VM 테스트보다 lazy 테스트를 먼저 돌리는 이유는 무엇인가?
2. `pt-grow-bad` 실패와 `pt-grow-stack` 실패는 각각 어떤 방향의 버그를 의심해야 하는가?
3. mmap 테스트 전에 lazy loading이 안정되어야 하는 이유는 무엇인가?
4. swap 테스트가 실패했을 때 frame table과 swap table 중 무엇을 먼저 볼지 어떻게 판단하는가?
5. VM 테스트가 통과해도 Project 2 회귀를 봐야 하는 이유는 무엇인가?
