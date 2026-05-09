# mmap과 File-backed Page

이 문서는 memory mapped file, `mmap`, `munmap`, file-backed page를 설명한다.

## 1. mmap은 무엇인가

보통 파일은 `read()`와 `write()` 시스템 콜로 다룬다.

`mmap()`은 파일 내용을 process의 virtual memory 영역에 연결해서, 파일을 메모리처럼 접근하게 만드는 기능이다.

```text
file "sample.txt"
  -> mmap(addr=0x10000000)
  -> 사용자 프로그램이 0x10000000을 읽으면 file byte를 읽는 것처럼 동작
```

사용자 입장에서는 메모리 load/store를 했을 뿐인데, 커널은 page fault 시점에 파일 내용을 frame으로 읽어 온다.

## 2. mmap의 함수 형태

GitBook 기준 시스템 콜 형태는 다음과 같다.

```c
void *mmap (void *addr, size_t length, int writable, int fd, off_t offset);
void munmap (void *addr);
```

`mmap()`은 성공하면 매핑 시작 주소를 반환하고, 실패하면 `NULL`을 반환해야 한다.

`munmap()`은 이전에 `mmap()`으로 얻은 매핑을 해제한다.

## 3. mmap이 실패해야 하는 경우

초심자는 먼저 invalid case를 정확히 외우는 것이 좋다.

`mmap()`은 다음 경우 실패해야 한다.

- `addr == NULL`
- `addr`이 page-aligned가 아님
- `length == 0`
- `offset`이 page-aligned가 아님
- file length가 0
- fd가 0 또는 1, 즉 stdin/stdout
- fd가 유효하지 않음
- 매핑하려는 주소 범위가 기존 page와 겹침
- 매핑 범위가 user address 범위를 벗어남

주소 overlap은 stack, 실행 파일 segment, 기존 mmap page를 모두 포함해서 검사해야 한다.

## 4. File-backed page

file-backed page는 내용을 파일에서 가져오는 page다.

실행 파일 lazy loading page도 넓게 보면 파일에서 내용을 읽지만, mmap page는 더 강한 파일 연관성을 가진다. mmap page는 사용자가 내용을 수정하면 `munmap()`이나 process exit 때 파일에 다시 기록해야 할 수 있다.

```text
mmap page fault
  -> file에서 page 내용 읽기

munmap 또는 exit
  -> dirty page라면 file에 write-back
```

## 5. mmap도 lazy하게 동작한다

`mmap()`이 호출되었다고 해서 파일 내용을 즉시 모두 읽을 필요는 없다.

대신 page별로 file-backed page를 SPT에 등록해 둔다.

```text
mmap(addr, length, fd, offset)
  -> file_reopen()으로 독립 file 참조 확보
  -> page마다 VM_FILE page 등록
  -> 실제 file_read는 page fault 때 수행
```

이 구조는 실행 파일 lazy loading과 비슷하다. 차이는 unmap/write-back 책임이 있다는 점이다.

## 6. 마지막 page의 zero bytes

파일 길이가 page size의 배수가 아닐 수 있다.

예를 들어 파일 길이가 5000바이트라면:

```text
첫 page: 4096바이트 파일에서 읽음
둘째 page: 904바이트 파일에서 읽고, 나머지 3192바이트는 0으로 채움
```

파일 끝을 넘어선 zero 영역은 write-back할 때 파일에 쓰면 안 된다. 원래 파일에 없는 영역이기 때문이다.

## 7. `file_reopen()`이 필요한 이유

GitBook은 각 mapping마다 파일에 대한 독립 참조를 얻기 위해 `file_reopen()`을 사용하라고 한다.

이유는 간단하다.

사용자 프로그램이 `mmap()` 후 원래 fd를 `close()`해도 mapping은 유지되어야 한다.

```text
fd = open("a.txt")
map = mmap(addr, len, writable, fd, 0)
close(fd)
addr에서 계속 접근 가능해야 함
```

원래 fd의 file object에만 의존하면 close 이후 mmap page가 파일을 잃어버릴 수 있다.

file reference 소유권은 구현 전에 정해야 한다.

| 정책 | 의미 | close 책임 |
|------|------|------|
| mapping 단위 `file_reopen()` | 하나의 mmap 영역이 독립 file reference 하나를 가진다 | `do_munmap()` 또는 exit cleanup에서 한 번 close |
| page 단위 `file_reopen()` | 각 file-backed page가 독립 file reference를 가진다 | 각 page의 `file_backed_destroy()`에서 close |

둘 중 어느 방식을 선택해도 된다. 다만 한 mapping 안의 모든 page가 같은 reopened file을 공유한다면 page destroy마다 close하면 안 된다. 반대로 page마다 reopen했다면 각 page가 자기 file reference를 닫아야 한다. 초심자는 "누가 file을 소유하고 누가 닫는가"를 구조체 필드 옆에 주석이나 문서로 남기는 편이 안전하다.

## 8. File operation과 lock

`mmap()` page는 page fault, eviction, `munmap()`, process exit에서 파일을 읽거나 쓸 수 있다. 따라서 file-backed page 구현도 Project 2의 file system 동기화 규칙을 따른다.

파일 접근이 발생하는 대표 지점은 다음과 같다.

| 지점 | 파일 작업 |
|------|------|
| `do_mmap()` | `file_reopen()`, file length 확인 |
| `file_backed_swap_in()` | file에서 page 내용 읽기 |
| `file_backed_swap_out()` | dirty page를 file에 write-back |
| `do_munmap()` / exit cleanup | dirty page write-back, mapping file close |

초심자는 "VM lock"과 "filesys lock"을 구분해야 한다. frame table이나 SPT를 보호하는 lock과 file system을 보호하는 lock을 같은 의미로 쓰면 안 된다. 팀 구현에서 lock 순서를 정하고, 파일 read/write 구간만 짧게 filesys lock으로 감싸는 방향이 이해하기 쉽다.

## 9. `do_mmap()`의 기본 흐름

`vm/file.c`의 `do_mmap()`은 syscall handler에서 검증한 값을 받아 실제 매핑을 만드는 함수로 볼 수 있다.

기본 흐름은 다음과 같다.

```text
1. 인자 검증
2. file_reopen()으로 mapping용 file 확보
3. length를 page 단위로 나눔
4. 각 page의 read_bytes, zero_bytes 계산
5. overlap 검사
6. VM_FILE page를 SPT에 등록
7. 성공하면 addr 반환
```

구현 방식에 따라 매핑 단위를 추적하는 구조체를 둘 수도 있고, 각 file page에 필요한 정보를 넣을 수도 있다.

중요한 것은 `munmap(addr)`가 "이 주소에서 시작한 mapping 범위"를 알 수 있어야 한다는 점이다.

mapping 단위 file reference를 쓴다면 `munmap(addr)`가 그 file reference를 한 번만 닫아야 한다. page 단위 file reference를 쓴다면 각 page destroy가 자기 file을 닫아도 된다.

또 하나 중요한 점은 실패 경로다. 여러 page를 등록하던 중 중간 page에서 overlap이나 할당 실패가 발생하면, 이미 등록한 page와 reopened file reference를 정리해야 한다.

## 10. `do_munmap()`의 기본 흐름

`do_munmap()`은 매핑된 page들을 제거하고, 필요한 경우 파일에 write-back한다.

기본 흐름은 다음과 같다.

```text
1. addr이 mmap 시작 주소인지 확인
2. mapping 범위의 page들을 순회
3. 각 page가 dirty인지 확인
4. dirty이면 파일 offset에 맞춰 write-back
5. pml4 mapping 제거
6. SPT에서 page 제거
7. page destroy
8. mapping용 file close
```

dirty하지 않은 page는 파일에 다시 쓰면 안 된다. 불필요한 write-back은 테스트에서 실패할 수 있고, 의미상으로도 맞지 않다.

## 11. Dirty bit

dirty bit는 page가 write된 적 있는지 알려준다.

Pintos에서는 `pml4_is_dirty()`를 사용할 수 있다.

```text
dirty page
  -> file_write_at() 필요

clean page
  -> write-back 불필요
```

주의할 점은 page가 이미 swap out되었거나 pml4 mapping이 지워진 상태일 수 있다는 것이다. 구현의 page 상태와 dirty 확인 시점을 잘 맞춰야 한다.

## 12. `file_backed_swap_in/out`

file-backed page operations는 대략 다음을 한다.

### swap in

```text
file_backed_swap_in(page, kva)
  -> page에 저장된 file, offset, read_bytes 확인
  -> file_read_at() 또는 seek/read
  -> 나머지 zero fill
```

### swap out

```text
file_backed_swap_out(page)
  -> dirty 여부 확인
  -> dirty하면 file에 write-back
  -> write-back했다면 dirty bit 정리
  -> pml4 mapping 제거
  -> frame과 page 연결 해제
```

anonymous page와 달리 file-backed page는 일반적으로 swap disk가 아니라 backing file을 사용한다. write-back 후 dirty bit를 그대로 두면 다음 eviction이나 destroy에서 같은 내용을 다시 썼다고 판단할 수 있으므로, 구현의 dirty 상태를 일관되게 정리해야 한다.

## 13. 흔한 실수

### Overlap 검사를 대충 하는 경우

첫 page만 검사하고 뒤 page를 검사하지 않으면 기존 mapping과 겹치는 영역이 생길 수 있다.

### fd close와 mmap lifetime을 묶는 경우

원래 fd가 닫혀도 mmap은 살아 있어야 한다. mapping용 file reference를 따로 가져야 한다.

### mmap 생성 실패 시 partial cleanup을 하지 않는 경우

여러 page 중 일부만 SPT에 들어간 상태에서 실패하면, 이미 만든 page와 file reference를 되돌려야 한다.

### File operation에 lock을 적용하지 않는 경우

여러 process가 동시에 같은 file system 코드에 접근하면서 file offset이나 내부 상태가 꼬일 수 있다.

### Dirty page만 write-back해야 한다는 조건을 놓치는 경우

clean page까지 쓰면 파일 내용이나 파일 시스템 상태가 예상과 달라질 수 있다.

### 마지막 page의 zero 영역을 파일에 쓰는 경우

파일 길이를 넘어선 부분은 메모리에서는 0이어도 disk에 기록하면 안 된다.

### `munmap()`에서 SPT entry를 제거하지 않는 경우

unmap 후에도 같은 주소 접근이 계속 성공하거나, exit cleanup에서 중복 해제가 발생할 수 있다.

## 14. 확인 질문

1. `mmap()`은 왜 파일 내용을 즉시 모두 읽지 않아도 되는가?
2. `file_reopen()`이 필요한 이유는 무엇인가?
3. mmap page의 마지막 zero 영역은 write-back해야 하는가?
4. dirty page와 clean page는 `munmap()`에서 어떻게 다르게 처리되는가?
5. `addr` overlap 검사는 어떤 기존 영역들과 비교해야 하는가?
6. mmap page 등록 중 일부만 성공한 뒤 실패하면 어떤 자원을 되돌려야 하는가?
7. file-backed page에서 filesys lock은 어떤 작업을 감싸야 하는가?
