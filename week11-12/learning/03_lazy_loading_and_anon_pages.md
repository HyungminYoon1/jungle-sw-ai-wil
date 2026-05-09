# Lazy Loading과 Anonymous Page

이 문서는 실행 파일을 처음부터 모두 메모리에 올리지 않고, 실제 접근할 때 로드하는 lazy loading과 anonymous page를 설명한다.

## 1. Lazy loading이란 무엇인가

Project 2에서는 실행 파일의 segment를 load 시점에 바로 메모리에 읽어 올 수 있었다. Project 3에서는 이 방식을 바꾼다.

Lazy loading은 실제로 접근하기 전까지 page 내용을 메모리에 올리지 않는 방식이다.

```text
기존 eager loading
  load 시점에 파일 내용을 모두 읽음

lazy loading
  load 시점에는 "나중에 읽을 정보"만 SPT에 등록
  실제 접근 시 page fault 발생
  page fault handler가 그때 파일을 읽음
```

이렇게 하면 당장 필요하지 않은 page에 물리 frame을 쓰지 않아도 된다.

## 2. `VM_UNINIT` page

lazy loading을 위해 Pintos는 `VM_UNINIT` page를 사용한다.

`VM_UNINIT` page는 아직 anonymous page나 file-backed page로 완전히 초기화되지 않은 page다.

```text
load_segment()
  -> VM_UNINIT page 생성
  -> SPT에 등록

첫 page fault
  -> uninit_initialize()
  -> 실제 page type initializer 호출
  -> lazy_load_segment() 실행
  -> 설정된 실제 page type으로 동작
```

초심자는 `VM_UNINIT`을 "아직 실제 내용은 없고, 나중에 어떻게 만들지 설명서만 가진 page"로 이해하면 된다.

여기서 특히 헷갈리기 쉬운 점은 "파일에서 읽어 온다"와 "file-backed page다"가 항상 같은 뜻은 아니라는 것이다.

| page 종류 | 처음 데이터 출처 | 보통 page type | 나중에 eviction될 때 |
|------|------|------|------|
| 실행 파일 ELF segment lazy page | 실행 파일 | `VM_ANON` | anonymous page처럼 swap disk 사용 |
| `mmap()`으로 만든 page | mmap 대상 파일 | `VM_FILE` | backing file에 dirty page만 write-back |

즉 `load_segment()`에서 등록한 실행 파일 page는 page fault 때 파일을 읽지만, GitBook의 anonymous page 범위에서 다루는 것이 일반적이다. 반대로 `mmap()`으로 만든 page가 file-backed page의 대표 사례다.

## 3. `vm_alloc_page_with_initializer()`

lazy page를 등록할 때 핵심 함수가 `vm_alloc_page_with_initializer()`다.

```c
bool vm_alloc_page_with_initializer (enum vm_type type, void *va,
    bool writable, vm_initializer *init, void *aux);
```

이 함수가 해야 하는 일은 크게 다음과 같다.

```text
1. va가 이미 SPT에 있는지 확인
2. struct page 할당
3. type에 맞는 initializer 선택
4. uninit_new()로 uninit page 구성
5. SPT에 page 삽입
```

여기서 `init`과 `aux`가 중요하다.

- `init`: page fault 때 실제 데이터를 채울 함수
- `aux`: 그 함수가 사용할 보조 정보

실행 파일 lazy loading에서는 `init`으로 보통 `lazy_load_segment`가 들어간다.
이때 `type`은 보통 `VM_ANON`이고, `lazy_load_segment`가 실행 파일에서 필요한 byte를 읽어 frame을 채운다. 파일에서 읽는다는 이유만으로 이 page를 `VM_FILE`로 분류하면 `mmap()` page와 cleanup/write-back 정책이 섞이기 쉽다.

## 4. `load_segment()`는 무엇을 바꾸는가

`userprog/process.c`의 `load_segment()`는 ELF segment를 user memory에 배치하는 함수다.

Project 3에서는 파일 내용을 바로 읽는 대신, page마다 lazy load 정보를 만들어 SPT에 등록해야 한다.

각 page마다 계산할 값은 다음과 같다.

| 값 | 의미 |
|------|------|
| `upage` | user virtual page 주소 |
| `ofs` | 파일에서 읽기 시작할 offset |
| `page_read_bytes` | 이 page에서 파일로부터 읽을 byte 수 |
| `page_zero_bytes` | 나머지를 0으로 채울 byte 수 |
| `writable` | 쓰기 가능한 segment인지 |

이 정보는 `lazy_load_segment()`가 나중에 필요로 한다. 따라서 `aux` 구조체에 담아 넘기는 방식이 흔하다.

## 5. `lazy_load_segment()`의 역할

`lazy_load_segment()`는 page fault가 발생해서 실제 frame이 생긴 뒤 호출된다.

해야 할 일은 다음과 같다.

```text
1. aux에서 file, offset, read_bytes, zero_bytes 확인
2. file_read_at(file, page frame kva, read_bytes, offset)
   또는 file_seek(file, offset) 후 file_read(file, page frame kva, read_bytes)
4. 나머지 zero_bytes 영역을 0으로 채움
5. aux가 동적 할당되었다면 해제
6. 성공 여부 반환
```

초심자가 주의할 점은 `lazy_load_segment()`가 load 시점이 아니라 page fault 시점에 실행된다는 것이다.

가능하면 `file_read_at()`처럼 offset을 인자로 받는 함수를 우선 고려하는 편이 이해하기 쉽다. `file_seek()` 후 `file_read()`를 쓰면 file object의 현재 offset을 바꾸므로, 같은 file object를 여러 page나 여러 thread가 공유할 때 lock 정책이 더 중요해진다.

## 6. Aux lifetime

`aux`는 lazy loading에서 자주 터지는 지점이다.

`aux`에는 나중에 쓸 정보가 들어 있으므로, `load_segment()`의 지역 변수 주소를 넘기면 안 된다. `load_segment()`가 끝나면 지역 변수는 사라지기 때문이다.

안전한 방식은 page마다 별도의 aux 구조체를 동적 할당하고, page가 initialize될 때 해제하는 것이다.

다만 page fault가 한 번도 발생하지 않은 채 process가 종료될 수도 있다. 이 경우 `VM_UNINIT` page가 SPT에 남아 있다가 destroy된다. 따라서 `uninit_destroy()`에서도 aux 누수를 막아야 한다.

## 7. Lazy loading file lifetime

`aux`에 `struct file *`를 넣을 때는 file lifetime을 반드시 정해야 한다. lazy loading은 load 시점이 아니라 page fault 시점에 파일을 읽기 때문에, `load()`가 끝난 뒤 file이 닫히면 나중에 `lazy_load_segment()`가 깨진 file pointer를 사용할 수 있다.

대표적인 정책은 두 가지다.

| 정책 | 설명 | 해제 책임 |
|------|------|------|
| page마다 `file_reopen()` | aux가 독립 file reference를 가진다 | lazy load 성공 시 또는 uninit destroy 시 close |
| executable file을 process가 보관 | 실행 파일 file을 process lifetime 동안 열어 둔다 | `process_exec()` 교체 또는 `process_exit()`에서 close |

초심자에게는 실행 파일 file을 process가 보관하는 방식이 흐름을 따라가기 쉽다. `load_segment()`에서 여러 page의 aux가 같은 실행 파일을 참조하더라도, process가 그 file을 page fault 시점까지 닫지 않는다는 규칙이 분명하기 때문이다. 다만 팀 코드가 이미 page별 `file_reopen()`으로 설계되어 있다면 그 정책을 유지해도 된다.

어느 쪽을 쓰든 중요한 것은 aux 안의 file pointer가 page fault 시점까지 유효해야 한다는 점이다. 또한 실행 중인 executable에는 `file_deny_write()`가 걸려 있을 수 있으므로, 누가 file을 닫고 write deny를 해제하는지도 함께 정해야 한다.

## 8. File read와 filesys lock

`lazy_load_segment()`는 파일에서 데이터를 읽는다. Pintos의 기본 file system 코드는 여러 thread가 동시에 접근해도 안전하다고 가정하면 안 되므로, `file_seek()`, `file_read()`, `file_read_at()` 같은 파일 접근은 팀의 file system lock 정책과 맞춰야 한다.

초심자는 다음 기준으로 생각하면 된다.

- file system lock은 실제 file operation을 감싸는 데 사용한다.
- `file_seek()`/`file_read()` 조합은 file offset을 바꾸므로 같은 file object 공유 여부를 확인한다.
- `file_read_at()`은 명시적 offset을 사용하므로 lazy page마다 다른 offset을 읽는 흐름을 설명하기 쉽다.
- aux 해제, page/frame 연결, SPT 조작까지 모두 같은 lock으로 길게 묶지는 않는다.
- eviction이나 mmap write-back도 파일에 접근하므로 lock 순서가 꼬이지 않게 팀 전체에서 하나의 규칙을 유지한다.

즉 lazy loading은 VM 기능이지만, 실제 파일을 읽는 순간에는 Project 2의 file system 동기화 규칙을 다시 따른다.

## 9. Anonymous page란 무엇인가

anonymous page는 특정 파일을 backing store로 갖지 않는 page다.

대표 예시는 stack page와 실행 파일 lazy page다.

```text
stack
heap 비슷하게 쓰이는 메모리
zero-filled page
```

anonymous page는 파일에서 다시 읽어 오면 되는 page가 아니다. swap out되면 swap disk에 내용을 저장해야 한다.

## 10. `anon_initializer()`

anonymous page로 초기화할 때는 `anon_initializer()`가 호출된다.

이 함수에서는 보통 다음 작업을 한다.

```text
1. page->operations = &anon_ops
2. page->anon 필드 초기화
3. swap 관련 상태 초기화
```

처음에는 swap을 완성하지 않았더라도, 나중에 swap slot 위치 같은 상태가 `struct anon_page`에 필요할 수 있다는 점을 염두에 두어야 한다.

## 11. `setup_stack()`과 첫 stack page

GitBook은 첫 번째 stack page는 lazy하게 둘 필요가 없다고 설명한다.

사용자 프로그램을 시작하려면 argument passing을 위해 stack에 값을 써야 한다. 따라서 첫 stack page는 load 시점에 바로 할당하고 claim해도 된다.

흐름은 대략 다음과 같다.

```text
setup_stack()
  -> USER_STACK - PGSIZE 위치에 anonymous page 할당
  -> vm_claim_page()로 즉시 frame 연결
  -> rsp를 USER_STACK으로 설정
  -> argument passing 수행
```

이후 stack이 더 아래 주소로 자라야 할 때는 stack growth가 처리한다.

## 12. Lazy loading 이후 page fault 흐름

실행 파일의 lazy page에 처음 접근하면 다음 일이 일어난다.

```text
1. 사용자 프로그램이 주소 X에 접근
2. 아직 pml4 mapping이 없어 page fault 발생
3. vm_try_handle_fault()가 SPT에서 page X를 찾음
4. vm_do_claim_page()가 frame 할당
5. uninit_initialize()가 실제 initializer 호출
6. lazy_load_segment()가 파일 내용을 frame에 읽음
7. page table mapping이 완성됨
8. 사용자 프로그램이 재개됨
```

이 흐름이 `lazy-file`, `lazy-anon` 같은 테스트의 핵심이다.

## 13. 흔한 실수

### 모든 page가 같은 aux를 공유하는 경우

루프에서 aux 하나를 재사용하면 마지막 page 정보만 남거나, 여러 page가 같은 offset을 읽는 문제가 생긴다. page마다 독립적인 aux가 필요하다.

### File offset을 잘못 저장하는 경우

각 page는 파일의 서로 다른 offset을 읽어야 한다. `ofs`를 page마다 증가시키는 흐름을 확인해야 한다.

### Zero fill을 빼먹는 경우

파일에서 읽지 않는 나머지 영역은 0으로 채워야 한다. 그렇지 않으면 사용자 프로그램이 쓰레기 값을 읽는다.

### Aux를 너무 빨리 해제하는 경우

`load_segment()`에서 SPT에 등록하자마자 aux를 해제하면 page fault 때 사용할 정보가 사라진다.

### Aux를 전혀 해제하지 않는 경우

fault가 난 page와 fault가 나지 않은 page 모두에서 aux 해제 경로가 필요하다.

### Aux의 file pointer lifetime을 정하지 않는 경우

`load()`가 끝난 뒤 닫힌 file pointer를 aux가 들고 있으면 첫 page fault 때 파일 읽기가 실패하거나 잘못된 파일 상태를 보게 된다.

### File read에서 lock 정책을 빼먹는 경우

여러 process가 동시에 executable이나 mmap file을 읽고 쓰면 file offset과 file system 내부 상태가 꼬일 수 있다.

## 14. 확인 질문

1. lazy loading은 eager loading과 무엇이 다른가?
2. `VM_UNINIT` page는 왜 필요한가?
3. `aux`에는 어떤 정보가 들어가야 하는가?
4. `lazy_load_segment()`는 언제 실행되는가?
5. anonymous page는 file-backed page와 swap out 방식이 왜 다른가?
6. aux 안의 file pointer는 page fault 시점까지 어떻게 유효하게 유지할 것인가?
7. `lazy_load_segment()`에서 file read를 할 때 lock 정책은 어떻게 적용할 것인가?
