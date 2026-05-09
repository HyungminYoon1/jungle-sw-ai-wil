# VM 리뷰 질문 모음

이 문서는 팀원이 Project 3 구현 전후로 서로 확인할 수 있는 질문 모음이다. 답을 외우기보다, 코드의 어떤 함수와 구조체를 근거로 설명할 수 있는지가 중요하다.

## 1. 큰 그림

힌트: `깃북/04_project3_virtual_memory/01_introduction`, `pintos/include/vm/vm.h`

1. Project 3에서 page fault가 정상 동작일 수 있는 이유는 무엇인가?
2. page, frame, page table, SPT를 각각 한 문장으로 설명할 수 있는가?
3. 사용자 프로그램은 frame을 직접 볼 수 없는 이유는 무엇인가?
4. Project 2가 불안정하면 Project 3에서 어떤 문제가 생기는가?
5. VM 구현에서 `PAL_USER`를 반드시 써야 하는 이유는 무엇인가?

## 2. 주소와 page

힌트: `pintos/include/threads/vaddr.h`, `깃북/06_appendix/04_virtual_address`

1. `pg_round_down()`은 어떤 상황에서 필요한가?
2. SPT key를 fault address 그대로 쓰면 어떤 문제가 생기는가?
3. user virtual address와 kernel virtual address를 어떻게 구분하는가?
4. `USER_STACK`은 stack의 어느 끝을 의미하는가?
5. page offset은 몇 bit이며, page size와 어떤 관계인가?

## 3. SPT

힌트: `pintos/include/vm/vm.h`, `pintos/vm/vm.c`의 `spt_find_page`, `spt_insert_page`

1. SPT는 page table과 어떤 점이 다른가?
2. SPT entry에는 최소 어떤 정보가 필요하다고 보는가?
3. `spt_find_page()`는 실패하면 무엇을 반환해야 하는가?
4. `spt_insert_page()`가 중복 insert를 거부해야 하는 이유는 무엇인가?
5. process exit에서 SPT가 필요한 이유는 무엇인가?

## 4. Page fault

힌트: `pintos/userprog/exception.c`, `pintos/vm/vm.c`의 `vm_try_handle_fault`

1. `vm_try_handle_fault()`는 어떤 조건에서 false를 반환해야 하는가?
2. not-present fault와 write-protection fault는 어떻게 다르게 보아야 하는가?
3. read-only page에 write하는 접근을 복구하면 왜 안 되는가?
4. SPT에 page가 없지만 stack growth로 처리할 수 있는 경우는 어떤 경우인가?
5. page fault handler가 성공적으로 반환하면 사용자 프로그램은 어떻게 이어서 실행되는가?
6. VM 이후 syscall user pointer validation은 Project 2와 어떤 점이 달라지는가?

## 5. Frame claim

힌트: `pintos/vm/vm.c`의 `vm_get_frame`, `vm_claim_page`, `vm_do_claim_page`

1. `vm_claim_page()`와 `vm_do_claim_page()`는 역할이 어떻게 다른가?
2. frame과 page가 서로 back reference를 갖는 이유는 무엇인가?
3. `pml4_set_page()`가 실패하면 어떤 정리가 필요한가?
4. frame table은 eviction 전에도 필요한가, 후에만 필요한가?
5. frame을 얻은 뒤 page table mapping을 만들지 않으면 어떤 증상이 생기는가?

## 6. Lazy loading

힌트: `pintos/userprog/process.c`의 `load_segment`, `lazy_load_segment`, `pintos/vm/uninit.c`

1. lazy loading은 실행 파일 로딩을 어떻게 바꾸는가?
2. `VM_UNINIT` page는 어떤 상태의 page인가?
3. `load_segment()`에서 page마다 aux를 따로 만들어야 하는 이유는 무엇인가?
4. `lazy_load_segment()`는 언제 호출되는가?
5. fault가 한 번도 나지 않은 uninit page의 aux는 언제 해제되어야 하는가?
6. aux가 들고 있는 `struct file *`는 page fault 시점까지 어떻게 유효하게 유지할 것인가?

## 7. Anonymous page

힌트: `pintos/include/vm/anon.h`, `pintos/vm/anon.c`, `pintos/include/vm/vm.h`의 `VM_ANON`

1. anonymous page는 왜 anonymous라고 부르는가?
2. stack page는 왜 anonymous page로 볼 수 있는가?
3. anonymous page가 swap out될 때 backing store는 무엇인가?
4. `anon_initializer()`는 어떤 page operation을 설정해야 하는가?
5. anonymous page destroy에서 swap slot을 확인해야 하는 이유는 무엇인가?

## 8. Stack growth

힌트: `pintos/vm/vm.c`의 `vm_stack_growth`, `vm_try_handle_fault`, `pintos/userprog/syscall.c`

1. stack은 어느 방향으로 자라는가?
2. fault address가 `rsp` 근처인지 확인하는 이유는 무엇인가?
3. x86-64 PUSH 때문에 `rsp - 8`을 고려하는 이유는 무엇인가?
4. kernel mode page fault에서 user `rsp`를 따로 저장해야 하는 이유는 무엇인가?
5. stack 1MB 제한을 어디 기준으로 계산해야 하는가?

## 9. mmap

힌트: `pintos/userprog/syscall.c`, `pintos/vm/file.c`의 `do_mmap`, `do_munmap`

1. `mmap()`은 파일 내용을 왜 즉시 모두 읽지 않아도 되는가?
2. `addr == NULL`인 mmap을 실패시켜야 하는 이유는 무엇인가?
3. `file_reopen()`을 사용하는 이유는 무엇인가?
4. `close(fd)` 후에도 mmap이 유지되어야 하는 이유는 무엇인가?
5. `munmap()`에서 dirty page와 clean page는 어떻게 다르게 처리해야 하는가?
6. mmap page를 여러 개 등록하던 중 실패하면 어떤 partial cleanup이 필요한가?

## 10. File-backed page

힌트: `pintos/include/vm/file.h`, `pintos/vm/file.c`, `file_backed_swap_in/out`

1. file-backed page의 swap in은 무엇을 읽어 오는가?
2. file-backed page의 swap out은 언제 파일에 write-back해야 하는가?
3. 마지막 page의 zero bytes를 파일에 쓰면 안 되는 이유는 무엇인가?
4. mmap page와 실행 파일 lazy page는 어떤 점이 비슷하고 어떤 점이 다른가?
5. process exit에서 mmap page를 어떻게 정리해야 하는가?
6. file-backed page를 write-back한 뒤 dirty bit는 어떻게 해야 하는가?

## 11. Eviction

힌트: `pintos/vm/vm.c`의 frame table, `vm_get_victim`, `vm_evict_frame`

1. eviction은 왜 필요한가?
2. victim frame을 고르려면 어떤 자료구조가 필요한가?
3. eviction 후 SPT entry를 지우면 왜 안 되는가?
4. eviction 후 pml4 mapping을 제거하지 않으면 어떤 문제가 생기는가?
5. accessed bit는 clock algorithm에서 어떻게 쓰이는가?

## 12. Swap

힌트: `pintos/vm/anon.c`, `pintos/devices/block.c`, `pintos/include/devices/block.h`

1. swap slot은 언제 할당해야 하는가?
2. swap in이 성공하면 swap slot은 어떻게 해야 하는가?
3. swap table을 bitmap으로 만들기 좋은 이유는 무엇인가?
4. swap disk sector 수와 `PGSIZE`는 어떤 관계가 있는가?
5. swap full이면 Pintos Project 3에서 어떻게 처리해도 되는가?

## 13. Fork

힌트: `pintos/userprog/process.c`의 `__do_fork`, `pintos/vm/vm.c`의 `supplemental_page_table_copy`

1. fork에서 SPT를 복사해야 하는 이유는 무엇인가?
2. uninit page를 복사할 때 aux를 공유하면 어떤 문제가 생기는가?
3. loaded anonymous page는 child에게 어떻게 복사해야 하는가?
4. Copy-on-Write를 하지 않는다면 parent와 child가 frame을 공유하면 안 되는 이유는 무엇인가?
5. swapped page를 가진 process를 fork할 때 어떤 선택지가 있는가?

## 14. Cleanup

힌트: `supplemental_page_table_kill`, `vm_dealloc_page`, 각 page type의 `destroy`

1. `supplemental_page_table_kill()`은 언제 호출되는가?
2. `uninit_destroy()`는 어떤 자원을 해제해야 하는가?
3. `anon_destroy()`는 어떤 자원을 확인해야 하는가?
4. `file_backed_destroy()`는 dirty page를 어떻게 처리해야 하는가?
5. frame, swap slot, file reference 중 하나라도 해제하지 않으면 어떤 테스트에서 문제가 드러날 수 있는가?

## 15. 테스트와 디버깅

힌트: `pintos/vm/build/tests/*/*.output`, `*.result`, `*.errors`

1. 같은 page fault가 반복되면 가장 먼저 무엇을 확인할 것인가?
2. SPT lookup이 실패하면 어떤 값을 출력해 볼 것인가?
3. `pt-grow-bad`가 실패하면 stack growth 조건은 너무 느슨한가, 너무 엄격한가?
4. `mmap-close`가 실패하면 file lifetime에서 무엇을 의심할 것인가?
5. `swap-anon`이 실패하면 swap out과 swap in 중 어느 쪽 로그를 먼저 볼 것인가?
6. file read/write가 관련된 실패에서 filesys lock 누락을 어떻게 의심할 것인가?

## 16. 코드 리뷰 체크

힌트: PR 본문에는 새 구조체 필드, 소유권, 해제 경로, 통과 테스트를 같이 적는다.

아래 질문에 답하지 못하면 PR 설명이나 코드 주석을 보강하는 편이 좋다.

- 이 변경은 어떤 page type의 lifecycle을 바꾸는가?
- 새로 추가한 구조체 필드는 누가 초기화하고 누가 해제하는가?
- 실패 경로에서 frame, page, aux, file, swap slot이 남지 않는가?
- Project 2 user pointer validation과 fork에 영향이 있는가?
- 테스트가 통과한 이유를 코드 흐름으로 설명할 수 있는가?
