# malloclab 심화 학습자료

## 주제: CS:APP Malloc Lab를 위한 동적 메모리 할당기 설계, 구현, 최적화

---

## 이 문서의 목표

이 문서는 **CS:APP malloclab**을 단순히 통과하는 수준이 아니라, **설계 선택의 이유를 설명하고 구현 대안을 비교할 수 있는 수준**까지 심화 이해하도록 돕기 위해 작성되었다. 다음을 목표로 한다.

- malloclab의 평가 기준과 숨은 설계 의도를 이해한다.
- implicit free list 기반의 기본 allocator를 정확히 구현할 수 있다.
- explicit free list와 segregated free list로 확장할 수 있다.
- `realloc` 최적화 전략을 비교하고 구현할 수 있다.
- heap checker를 설계하여 allocator 불변식(invariant)을 검증할 수 있다.
- throughput / utilization trade-off를 workload 관점에서 분석할 수 있다.
- 다양한 접근 방식의 장단점을 설명할 수 있다.

이 문서의 초점은 **동적 메모리 할당기의 구현 원리**이며, 단순 API 설명보다 **블록 레이아웃, 자료구조, 정책, 디버깅, 성능 분석**에 집중한다. 즉, 기본 구현을 넘어 allocator의 설계 선택과 trade-off를 따져 보는 심화 학습자료다.

---

# 1. malloclab을 어떻게 봐야 하는가

많은 학생이 malloclab을 다음처럼 오해한다.

> “`malloc`, `free`, `realloc`를 흉내 내는 함수 몇 개를 만드는 과제”

하지만 실제로 malloclab은 그보다 훨씬 깊다. 이 과제는 다음 문제를 한꺼번에 다룬다.

1. **저수준 메모리 레이아웃 설계 문제**
2. **자료구조 선택 문제**
3. **알고리즘 정책 문제**
4. **성능 공학 문제**
5. **디버깅 및 검증 문제**

즉, malloclab의 본질은 다음과 같이 요약할 수 있다.

> 제한된 힙 공간 안에서 요청 크기와 수명이 예측 불가능한 객체들을 빠르게 배치하고 회수하면서, 단편화를 최소화하는 allocator를 설계하고 구현하는 문제

이 관점에서 보면 malloclab은 단순 구현 과제가 아니라, **시스템 소프트웨어 설계 연습**에 가깝다.

---

# 2. malloclab의 인터페이스와 실행 모델

일반적인 malloclab 인터페이스는 다음과 같다.

```c
int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void *ptr);
void *mm_realloc(void *ptr, size_t size);
```

보통 lab 환경에서는 allocator가 직접 OS와 상호작용하지 않고, `memlib`이 제공하는 추상화 위에서 동작한다.

전형적으로 다음과 같은 인터페이스가 등장한다.

다만 실제 malloclab 배포본마다 `memlib` 인터페이스나 평가 드라이버의 세부 규칙은 조금씩 다를 수 있다. 따라서 아래 설명은 전형적인 교육용 malloclab 환경을 기준으로 이해하는 것이 좋다.

```c
void *mem_sbrk(int incr);
void *mem_heap_lo(void);
void *mem_heap_hi(void);
size_t mem_heapsize(void);
```

이 모델의 의미는 명확하다.

- allocator는 힙을 직접 관리한다.
- 필요 시 `mem_sbrk`로 힙을 확장한다.
- 실제 OS 수준의 page allocator나 kernel VM 세부 구현은 숨긴다.
- 학생은 **힙 내부 allocator 설계**에 집중한다.

즉, malloclab은 운영체제 전체가 아니라, **user-space allocator의 핵심 메커니즘**만 분리해서 보여 주는 교육용 환경이다.

---

# 3. 평가 지표를 먼저 이해해야 하는 이유

malloclab 설계는 평가 지표를 이해하지 못하면 방향을 잘못 잡기 쉽다. 보통 핵심 지표는 다음 두 가지다.

## 3.1 Throughput

단위 시간당 얼마나 많은 allocation/deallocation 요청을 처리할 수 있는가.

즉, 구현이 얼마나 빠른가를 본다.

Throughput에 영향을 주는 요소:

- 탐색 비용
- 삽입/삭제 비용
- coalescing 비용
- `realloc` 시 복사 비용
- free list 유지 비용

---

## 3.2 Utilization

주어진 workload에서 프로그램이 실제로 필요로 하는 payload 대비, 힙을 얼마나 효율적으로 사용했는가.

Utilization에 영향을 주는 요소:

- 내부 단편화
- 외부 단편화
- 블록 크기 정렬 정책
- split 기준
- fit policy
- coalescing 정책
- free block 메타데이터 오버헤드
- 프롤로그/에필로그 및 분리 리스트 루트 배열 같은 관리 구조 오버헤드

---

## 3.3 이 둘은 왜 충돌하는가?

매우 빠른 allocator는 대체로 단순한 정책을 사용한다.

예:
- LIFO explicit list
- first fit
- 단순 `realloc`

이 경우 탐색과 조작은 빠르지만 단편화가 커질 수 있다.

반대로 utilization을 극대화하려는 allocator는 더 많은 탐색이나 더 정교한 자료구조를 사용한다.

예:
- best fit에 가까운 정책
- segregated class 세분화
- aggressive coalescing

이 경우 공간 효율은 좋아질 수 있지만 throughput이 떨어질 수 있다.

malloclab은 바로 이 **trade-off를 체감하게 만드는 과제**다.

---

# 4. 힙을 어떻게 모델링할 것인가

malloc allocator는 힙을 연속된 블록들의 나열로 본다.

```text
+--------+--------+--------+--------+--------+
| block  | block  | block  | block  | block  |
+--------+--------+--------+--------+--------+
```

각 블록은 크게 두 상태 중 하나다.

- allocated block
- free block

문제는 단순해 보이지만 실제 핵심은 다음 질문들이다.

- 블록의 경계를 어떻게 알 것인가?
- 이전 블록과 다음 블록을 어떻게 찾을 것인가?
- free block을 어떻게 빠르게 찾을 것인가?
- free된 블록은 어떻게 재사용 가능한 상태로 유지할 것인가?

이 문제를 해결하기 위해 메타데이터가 필요하다.

---

# 5. 블록 레이아웃 설계

malloclab의 가장 기본적인 블록 레이아웃은 다음과 같다.

```text
+---------+-------------------+---------+
| header  | payload           | footer  |
+---------+-------------------+---------+
```

## 5.1 Header가 담는 정보

대개 다음 두 가지가 핵심이다.

- 블록 크기
- allocation bit

추가 최적화가 들어가면 다음도 포함될 수 있다.

- prev_alloc bit
- 작은 class 인덱스 힌트
- 디버깅용 마커

하지만 기본 malloclab에서는 크기와 할당 비트가 핵심이다.

---

## 5.2 Footer가 필요한 이유

Footer는 이전 블록의 크기를 뒤에서 읽을 수 있게 해 준다. 즉, **boundary tag**를 구성한다.

현재 블록에서 이전 블록과 병합하려면 이전 블록의 시작 주소를 알아야 한다. footer가 있으면 다음이 가능하다.

1. 현재 블록 시작 위치에서 바로 이전 워드를 읽는다.
2. 그것이 이전 블록 footer다.
3. 이전 블록 크기를 알 수 있다.
4. 이전 블록 시작 주소를 역산할 수 있다.

즉, footer는 backward traversal을 가능하게 한다.

---

## 5.3 Footer의 비용

모든 블록에 footer를 두면 공간 오버헤드가 증가한다.

특히 작은 블록이 많은 workload에서는 footer의 상대 비용이 크다.

그래서 고급 allocator들은 종종 다음 최적화를 한다.

- allocated block에는 footer를 생략
- free block에만 footer 유지
- header에 `prev_alloc` bit를 추가

이 최적화는 malloclab에서도 충분히 시도할 수 있지만, 구현 복잡도가 증가한다. 또한 malloclab에서는 이런 최적화가 항상 trace 점수 향상으로 이어지는 것도 아니다. 공간 효율 이득은 있을 수 있지만, 상태 갱신과 디버깅 비용이 커져 전체 완성도에 불리할 수 있다. 기본 구현 단계에서는 모든 블록에 footer를 두는 설계가 가장 안정적이다.

---

# 6. 정렬과 메타데이터 packing

## 6.1 정렬이 필요한 이유

반환 포인터는 대개 8바이트 또는 16바이트 정렬을 만족해야 한다. 이는 ABI, 하드웨어 접근 효율, 데이터 타입 요구사항 때문이며, misaligned access를 피하는 목적도 있다.

따라서 요청한 size를 그대로 쓰지 않고, allocator는 정렬 단위로 반올림한다.

예:

```text
requested size = 13
aligned payload = 16
block size = 16 + overhead
```

---

## 6.2 크기와 alloc bit를 한 워드에 같이 저장하는 이유

정렬된 블록 크기의 하위 비트는 항상 0이다. 예를 들어 8-byte alignment라면 하위 3비트는 0이다.

이 비트를 재활용하여 alloc bit를 저장할 수 있다.

예:

```c
header = size | alloc;
```

이 방식의 장점:

- 메타데이터 공간 절약
- 읽기/쓰기 단순화
- 비트 연산으로 크기와 상태를 쉽게 추출 가능

대표적인 예로 다음 매크로들이 사용된다.

다만 실제 코드 골격에서는 `ALIGNMENT`, `WSIZE`, `DSIZE` 정의에 따라 마스킹 방식이 조금씩 달라질 수 있다. 어떤 구현은 `~0x7` 또는 `~0xF`를 직접 사용하고, 어떤 구현은 정렬 단위를 별도 상수로 둔다.

```c
#define PACK(size, alloc) ((size) | (alloc))
#define GET(p)            (*(size_t *)(p))
#define PUT(p, val)       (*(size_t *)(p) = (val))
#define GET_SIZE(p)       (GET(p) & ~(DSIZE - 1))
#define GET_ALLOC(p)      (GET(p) & 0x1)
```

이 매크로들은 단순해 보여도, malloclab 전체 구현의 핵심이다.

---

# 7. 최소 블록 크기(minimum block size)

최소 블록 크기를 정확히 잡지 못하면 구현이 쉽게 깨진다.

## 7.1 implicit list 기준

최소한 필요한 것:

- header
- footer
- 정렬된 최소 payload 또는 빈 payload

예를 들어 64비트 환경에서 header/footer가 각각 8바이트라면 최소 블록은 적어도 16바이트 이상이 된다. 다만 실제 malloclab 코드에서는 `ALIGNMENT`, `WSIZE`, `DSIZE`, footer 사용 여부에 따라 최소 블록 크기가 더 커질 수 있으므로, 구현 정의를 기준으로 계산해야 한다.

---

## 7.2 explicit list 기준

free block 안에는 payload 대신 free list 포인터가 저장된다.

즉, free block에는 최소한 다음이 필요하다.

- header
- pred pointer
- succ pointer
- footer

그러므로 explicit list의 최소 블록 크기는 implicit list보다 더 크다.

특히 64비트 환경에서는 `pred`와 `succ`가 각각 8바이트이므로, explicit free list의 최소 free block 크기는 빠르게 커진다. 즉, explicit list는 탐색 속도를 개선하는 대신 공간 오버헤드를 더 많이 지불할 수 있다.

이 사실은 split 가능 여부 판단에도 직접 영향을 준다.

---

# 8. 프롤로그와 에필로그

힙 구현에서 경계 조건은 코드 복잡성을 급격히 올린다. malloclab에서는 이를 단순화하기 위해 **프롤로그 블록**과 **에필로그 헤더**를 둔다.

## 8.1 프롤로그 블록

작은 allocated block처럼 동작하는 가짜 블록이다.

목적:

- 첫 블록 병합 시 예외 처리 제거
- `PREV_BLKP` 계산을 안전하게 만듦

---

## 8.2 에필로그 헤더

크기 0, allocated 상태의 특수 헤더다.

목적:

- 힙 끝을 쉽게 감지
- 마지막 블록 다음 접근 시 예외 처리 단순화

이 두 센티널은 간단해 보여도 구현 안전성에 매우 중요하다.

---

# 9. implicit free list: 기본 해법

가장 먼저 구현할 allocator는 보통 implicit free list다.

## 9.1 핵심 아이디어

free block만 따로 연결하지 않고, 힙의 모든 블록을 순회하면서 free block을 찾는다.

다음 블록 주소는 현재 블록의 size로 계산한다.

```c
next_bp = bp + GET_SIZE(HDRP(bp));
```

---

## 9.2 장점

- 레이아웃이 단순하다
- 코드가 직관적이다
- malloclab의 기본 불변식을 이해하기 좋다

---

## 9.3 단점

- free block을 찾기 위해 전체 힙을 선형 탐색할 수 있다
- 힙이 커질수록 throughput이 나빠진다
- allocated block이 많을수록 비효율적이다

그럼에도 불구하고 implicit list는 교육적으로 매우 중요하다. explicit/segregated allocator의 모든 개선은 implicit allocator의 병목을 줄이는 방향으로 이해할 수 있기 때문이다.

---

# 10. fit policy 비교

free block을 찾을 때 어떤 정책을 쓸 것인가?

## 10.1 First fit

처음으로 조건을 만족하는 free block을 사용한다.

### 장점
- 빠르고 구현이 단순하다
- 보통 baseline으로 좋다

### 단점
- 힙 앞부분에 작은 조각이 많이 쌓일 수 있다
- 장기적으로 외부 단편화가 악화될 수 있다

---

## 10.2 Next fit

직전 탐색이 끝난 지점부터 다시 탐색한다.

### 장점
- 경우에 따라 탐색 비용 감소
- 힙 앞부분 편중을 줄일 수 있음

### 단점
- locality가 나빠질 수 있다
- 성능 향상이 일관적이지 않다
- 디버깅 시 상태 의존성이 커질 수 있다

malloclab에서는 next fit이 무조건 유리하지 않다. trace의 locality와 힙 또는 free list 순회 패턴에 따라 오히려 first fit보다 성능이 더 불안정해질 수 있다.

---

## 10.3 Best fit

맞는 block 중 가장 작은 것을 선택한다.

### 장점
- split 후 남는 공간을 줄일 가능성이 있다
- utilization 향상 기대

### 단점
- 탐색 비용이 크다
- 전체 탐색이 필요할 수 있다
- 실제 workload에서는 기대만큼 좋아지지 않을 수 있다

best fit은 이론적으로 매력적이지만, malloclab의 throughput 점수를 고려하면 자료구조 개선 없이는 종종 불리하다.

---

## 10.4 실전적 정리

- implicit list: first fit이 전형적인 baseline
- explicit list: first fit 또는 size-ordered 전략을 많이 사용
- segregated list: class 내부 first fit 또는 제한적 best fit이 trace에 따라 유리할 수 있음

즉, fit policy는 free list 구조와 결합해서 생각해야 한다.

---

# 11. place: 블록 배치와 splitting

적절한 free block을 찾았으면 실제로 배치해야 한다. 이때 핵심은 split 여부다.

## 11.1 왜 splitting이 필요한가?

큰 free block 전체를 작은 요청에 통째로 주면 내부 단편화가 커진다.

예:

```text
free 128에 요청 24를 그대로 배치 -> 100바이트 이상 낭비 가능
```

따라서 보통 다음처럼 분할한다.

```text
[ free 128 ]
-> [ alloc 32 ][ free 96 ]
```

---

## 11.2 split 조건

남는 조각이 최소 블록 크기 이상이면 split한다. 아니면 전체 블록을 할당한다.

즉:

```text
if (csize - asize >= min_block_size)
    split
else
    consume whole block
```

---

## 11.3 앞쪽 배치 vs 뒤쪽 배치

많은 구현은 block 앞쪽에 allocated 부분을 두고 뒤에 remainder free block을 둔다.

```text
[ alloc ][ free ]
```

하지만 상황에 따라 뒤쪽 배치도 가능하다.

### 앞쪽 배치 장점
- 구현 단순
- 반환 포인터가 기존 bp 그대로 유지

### 뒤쪽 배치 장점
- 특정 `realloc` 시나리오에서 유리할 수 있음

malloclab에서는 일반적으로 앞쪽 배치가 구현과 디버깅 면에서 더 안전한 편이다.

---

# 12. free와 coalescing

free는 단순히 alloc bit를 0으로 바꾸는 것이 아니다. 진짜 핵심은 coalescing이다.

## 12.1 왜 coalescing이 필요한가?

free가 반복되면 힙이 조각난다. 인접 free block을 합치지 않으면 큰 요청이 들어왔을 때 충분한 총 free 공간이 있어도 할당이 실패하거나 힙을 불필요하게 확장하게 된다.

---

## 12.2 4-case coalescing

현재 free된 block `bp`를 기준으로 이전/다음 block의 상태를 조사한다.

### Case 1: prev alloc, next alloc
현재 블록만 free 유지

### Case 2: prev alloc, next free
현재 + 다음 블록 병합

### Case 3: prev free, next alloc
이전 + 현재 블록 병합

### Case 4: prev free, next free
이전 + 현재 + 다음 모두 병합

이 네 경우를 코드로 정확히 구현하는 것이 핵심이다.

---

## 12.3 Immediate vs Deferred coalescing

### Immediate coalescing
free 시 즉시 병합

장점:
- 구현 단순
- 외부 단편화 완화
- heap checker invariant 유지 쉬움

단점:
- free와 malloc이 교차하는 workload에서 split/coalesce 반복 가능

### Deferred coalescing
나중에 필요할 때 병합

장점:
- 특정 workload에서 불필요한 병합 감소

단점:
- 구현 복잡
- free list 상태가 더 복잡해짐
- checker와 디버깅 난이도 상승

malloclab의 기본 출발점으로는 immediate coalescing이 가장 다루기 쉬운 선택인 경우가 많다.

---

# 13. 힙 확장 전략

fit 실패 시 allocator는 힙을 확장해야 한다.

## 13.1 기본 흐름

1. `mem_sbrk`로 chunk 요청
2. 새 영역을 free block으로 초기화
3. 새 에필로그 헤더 배치
4. 이전 마지막 block과 병합

---

## 13.2 Chunk size는 어떻게 정할까?

너무 작게 확장하면:
- `mem_sbrk` 호출이 자주 발생
- 오버헤드 증가

너무 크게 확장하면:
- utilization 악화 가능
- 불필요한 힙 증가

실전에서는 적당한 `CHUNKSIZE`를 사용하고, 큰 요청에 대해서는 `max(asize, CHUNKSIZE)`처럼 처리하는 경우가 많다.

예:

```c
extendsize = MAX(asize, CHUNKSIZE);
```

이 정책은 작은 요청 반복에 대한 안정성과 큰 요청 처리의 유연성을 동시에 노린다.

---

# 14. mm_init 설계

`mm_init`은 단순 초기화 함수가 아니다. 전체 allocator 불변식을 세우는 함수다.

일반적 초기화 절차:

1. alignment padding 확보
2. prologue header/footer 생성
3. epilogue header 생성
4. free list 루트 초기화
5. initial heap extension 수행

이 단계에서 실수하면 이후 모든 포인터 산술이 잘못된다. malloclab 디버깅에서 가장 먼저 점검해야 하는 부분 중 하나다.

---

# 15. explicit free list: 병목 제거 1단계

implicit list의 핵심 병목은 “전체 힙 선형 탐색”이다. explicit free list는 이 병목을 줄인다.

## 15.1 기본 아이디어

free block의 payload 부분을 포인터 저장에 사용한다.

```text
+---------+----------+----------+---------+
| header  | pred ptr | succ ptr | footer  |
+---------+----------+----------+---------+
```

이렇게 하면 free block만 연결된 리스트를 유지할 수 있다.

---

## 15.2 장점

- 탐색 대상이 free block으로 제한된다
- allocated block이 많아도 탐색 비용이 덜 증가한다
- throughput이 크게 개선될 수 있다

---

## 15.3 단점

- 최소 block size 증가
- insert/remove 구현이 필요
- coalescing과 free list 유지가 결합되어 복잡해짐

즉, explicit list는 성능을 위해 구현 복잡성을 지불하는 설계다.

---

# 16. explicit free list에서의 세부 설계 선택

## 16.1 Doubly linked vs Singly linked

### Singly linked
장점:
- 포인터 하나만 사용
- free block 오버헤드 감소

단점:
- 임의 노드 삭제가 어려움
- predecessor 탐색 필요

### Doubly linked
장점:
- O(1) 삭제 가능
- coalescing 시 free list 갱신이 편함

단점:
- 포인터 두 개 필요
- 최소 block size 증가

malloclab에서는 doubly linked list가 구현과 디버깅 면에서 더 다루기 쉬운 경우가 많다.

---

## 16.2 LIFO insertion

새로 free된 block을 리스트 맨 앞에 삽입한다.

### 장점
- 삽입 O(1)
- 구현 단순
- throughput 유리

### 단점
- 동일 영역 재사용 편향이 커질 수 있음
- fragmentation 특성이 workload에 따라 나빠질 수 있음

LIFO는 malloclab에서 가장 자주 쓰이는 baseline이다.

---

## 16.3 Address-ordered insertion

주소 순으로 free list를 유지한다.

### 장점
- coalescing과 locality 측면에서 유리할 수 있음
- 디버깅 시 리스트 상태가 더 직관적

### 단점
- 삽입 O(n)
- throughput 감소 가능

utilization을 더 노릴 때 고려할 수 있지만, 기본 점수 최적화에는 불리할 수 있다.

---

## 16.4 Size-ordered insertion

free list를 block size 기준으로 정렬한다.

### 장점
- best-fit 유사 탐색이 쉬움
- fragmentation 개선 가능

### 단점
- 삽입과 삭제 비용 증가
- 구현 복잡

malloclab에서 이 접근은 높은 utilization을 목표로 할 때 의미가 있지만, 코드 안정성과 throughput 비용을 같이 고려해야 한다.

---

# 17. explicit list 구현에서 가장 자주 깨지는 부분

## 17.1 free list remove 누락

split 또는 coalesce 전에 관련 free block을 리스트에서 제거하지 않으면, dangling node나 duplicate node가 발생한다.

---

## 17.2 coalescing 순서 오류

병합 전에 remove해야 하는지, 병합 후 insert해야 하는지 순서를 잘못 잡으면 list inconsistency가 생긴다.

안전한 패턴은 보통 다음과 같다.

1. 병합 대상 free block들 remove
2. size 계산 및 header/footer 갱신
3. 병합된 결과 block insert

---

## 17.3 split 후 remainder 처리 누락

block을 분할했으면 remainder free block을 free list에 삽입해야 한다. 이 부분을 빼먹으면 heap 상에는 free block이 있지만 allocator는 찾지 못하게 된다.

---

## 17.4 pred/succ 포인터 초기화 실수

특히 free block이 allocated block으로 바뀌는 순간, 과거 pred/succ 포인터가 남아 있어 checker를 혼란스럽게 만들 수 있다. 기능적으로는 무관할 수 있어도 디버깅을 위해 명시적 초기화가 도움이 된다.

---

# 18. segregated free list: 병목 제거 2단계

explicit list의 다음 진화는 free block을 크기 class별로 나누는 것이다.

## 18.1 기본 아이디어

하나의 free list 대신 여러 개의 리스트를 둔다.

예:

- class 0: 16 ~ 31
- class 1: 32 ~ 63
- class 2: 64 ~ 127
- class 3: 128 ~ 255
- ...

할당 요청이 들어오면 해당 size class에서 먼저 탐색한다.

---

## 18.2 장점

- 탐색 공간이 줄어든다
- size가 비슷한 block끼리 모이므로 fragmentation 특성이 좋아질 수 있다
- class 내부 제한적 best fit 구현이 가능하다

---

## 18.3 설계 포인트

### class 경계 설정
- power-of-two 기반
- 균등 구간 기반
- 하이브리드 기반

### 클래스 내부 정책
- first fit
- best fit
- bounded search

### large block 처리
- 마지막 클래스에 몰아넣기
- tree 구조로 확장하기

malloclab에서는 power-of-two 기반 분리 리스트가 구현 단순성과 성능의 균형을 맞추기 쉬운 편이다.

---

## 18.4 흔한 전략

실전적으로는 다음 조합이 자주 쓰인다.

- segregated free list
- class 내부 LIFO insertion
- class 내부 first fit 또는 limited best fit
- immediate coalescing

이 조합은 구현 난이도와 점수 사이에서 좋은 균형을 만드는 경우가 많다.

---

# 19. footer 제거 최적화와 prev_alloc bit

고급 allocator 설계에서는 allocated block의 footer를 제거해 공간을 절약할 수 있다.

## 19.1 아이디어

현재 block header에 `prev_alloc` bit를 저장한다.

그러면 현재 block이 이전 block의 상태를 알 수 있으므로, 이전 block이 allocated라면 footer가 필요 없다.

### free block
- header 유지
- footer 유지

### allocated block
- header 유지
- footer 생략 가능

---

## 19.2 장점

- 작은 allocated block에 대한 오버헤드 감소
- utilization 향상 가능

---

## 19.3 단점

- 상태 갱신 로직 복잡
- split/free/coalesce 시 다음 block의 `prev_alloc` bit 관리 필요
- 디버깅 난이도 상승

malloclab에서 충분히 도전할 만하지만, checker 없이 구현하면 매우 쉽게 깨진다.

---

# 20. realloc: allocator의 진짜 실력 차이

많은 학생이 `realloc`을 마지막에 대충 구현하지만, 실제로 `realloc`은 점수와 설계 수준을 크게 좌우한다.

## 20.1 가장 단순한 구현

```text
malloc new -> memcpy -> free old
```

### 장점
- 구현 단순
- correctness 확보 쉬움

### 단점
- 항상 복사 발생 가능
- throughput과 locality 악화
- 큰 객체에서 비용 큼

---

## 20.2 제자리 확장(in-place growth)

다음 block이 free이고 합치면 충분한 경우, 현재 block을 확장할 수 있다.

### 장점
- 복사 비용 제거
- throughput 개선
- cache locality 유지 가능

### 구현 포인트
- next free block remove
- 합친 size 계산
- split 여부 판단
- metadata 갱신

---

## 20.3 제자리 축소(in-place shrink)

새 size가 충분히 작아졌고 남는 부분이 최소 block size 이상이면 split 가능하다.

예:

```text
old 128 -> new 32
=> [alloc 32][free 96]
```

다만 너무 공격적으로 split하면 작은 remainder들이 많아져 fragmentation이 커질 수 있다. 따라서 shrink split 기준을 보수적으로 잡는 전략도 가능하다.

---

## 20.4 앞/뒤 재배치 전략

일반적인 allocator는 block 시작 주소를 유지하려고 하지만, 더 공격적인 구현은 인접 free block들을 활용하여 데이터 이동량을 줄일 수도 있다. malloclab 수준에서는 보통 다음 순서의 전략이 현실적이다.

1. 현재 block이 이미 충분한지 확인
2. next free block과 결합 가능한지 확인
3. 아니면 malloc-copy-free

여기서 중요한 점은, 이전 블록(prev free)과의 결합은 새 payload 시작 주소가 바뀌므로 일반적인 의미의 in-place `realloc`과는 다르다는 것이다. 따라서 많은 구현이 먼저 현재 블록 유지 또는 다음 블록과의 결합을 우선 고려한다.

---

## 20.5 `realloc`의 corner case

- `ptr == NULL` -> `malloc(size)`
- `size == 0` -> C 표준과 구현에 따라 세부 동작이 다를 수 있으므로, lab 드라이버 요구사항에 맞게 정책을 정하는 것이 좋다. 흔히 `free(ptr)` 후 `NULL`을 반환하는 형태를 사용한다.
- old payload보다 큰 크기를 복사하면 안 됨
- overlap 없는 `memcpy` 조건 확인 필요

---

# 21. 다양한 allocator 접근 방식 비교

이 절에서는 malloclab에서 시도할 수 있는 대표적 접근들을 비교한다.

## 21.1 접근 A: Implicit + First fit + Immediate coalescing

### 특징
- 가장 전통적인 baseline
- 구현 가장 쉬움
- correctness 확보 용이

### 장점
- 빠르게 완성 가능
- 디버깅 용이
- heap invariant 이해에 좋음

### 단점
- throughput 낮음
- 큰 trace에서 병목 심함

### 적합한 상황
- 첫 정답 구현
- checker 구축 전 단계

---

## 21.2 접근 B: Explicit + LIFO + First fit

### 특징
- malloclab에서 매우 흔한 실전적 해법
- free block만 탐색

### 장점
- throughput 크게 개선 가능
- 구현 난이도 수용 가능 수준

### 단점
- fragmentation이 trace에 따라 나빠질 수 있음
- free list 조작 버그 위험 증가

### 적합한 상황
- 점수 개선 1차 목표
- baseline optimized allocator

---

## 21.3 접근 C: Explicit + Address-ordered + First/Best fit

### 특징
- free list를 주소 순 유지
- 병합 판단과 공간 구조 파악이 더 직관적

### 장점
- utilization 개선 가능
- 디버깅 가독성 좋음
- 물리적으로 인접한 블록들을 사람이 추적하기 쉬움

### 단점
- 삽입 비용 증가
- throughput 손해 가능

### 적합한 상황
- utilization을 중시하는 전략
- trace 특성이 fragmentation 민감할 때

---

## 21.4 접근 D: Segregated lists + First fit

### 특징
- class별 분리 리스트
- 탐색 범위 크게 감소

### 장점
- throughput/utilization 균형 좋음
- malloclab 상위권 전략으로 자주 등장

### 단점
- 구현 복잡
- class 설계가 중요

### 적합한 상황
- 고득점 목표
- allocator 구조를 확장할 여유가 있을 때

---

## 21.5 접근 E: Segregated lists + Limited best fit + Realloc optimization

### 특징
- 성능 지향 고급 설계
- 클래스 내부에서 일부 노드만 탐색해 best-like 선택
- in-place realloc 적극 활용

### 장점
- 높은 utilization 가능
- realloc-heavy trace에서 유리

### 단점
- 코드 복잡도 큼
- checker 없으면 디버깅 어려움
- 작은 실수로 correctness 깨짐

### 적합한 상황
- 충분한 시간과 디버깅 도구가 있을 때
- allocator 설계 자체를 학습 목적으로 깊게 해보고 싶을 때

---

# 22. 구현 순서 전략

malloclab에서 흔한 실패 패턴은 처음부터 고급 구조를 넣는 것이다. 좋은 전략은 단계적으로 가는 것이다.

## 권장 순서

### 단계 1
implicit free list로 correctness 확보

### 단계 2
heap checker 구축

### 단계 3
explicit free list로 전환

### 단계 4
realloc 개선

### 단계 5
segregated list로 최적화

### 단계 6
footer 제거나 prev_alloc 같은 미세 최적화 시도

이 순서는 단순히 쉬워서가 아니라, **불변식을 점진적으로 축적하기 가장 좋은 순서**이기 때문이다.

---

# 23. heap checker 설계

allocator는 눈으로만 디버깅하기 어렵다. checker가 없으면 거의 반드시 고생한다.

## 23.1 checker가 검사해야 할 불변식

### 블록 단위 불변식
- 각 block의 size는 정렬 조건을 만족하는가?
- header와 footer가 일치하는가?
- size가 최소 block size 이상인가?

### 힙 구조 불변식
- prologue/epilogue가 올바른가?
- 힙 전체 순회가 가능한가?
- 인접 free block이 병합되지 않은 채 남아 있지 않은가?

### free list 불변식
- free list 안의 모든 block은 실제 free인가?
- 힙 상의 모든 free block이 free list에 포함되는가?
- free list에 중복 노드가 없는가?
- pred/succ 링크가 일관적인가?

---

## 23.2 checker 구현 팁

- 단계별 verbosity 레벨을 두면 좋다.
- block 주소, size, alloc 상태를 출력하는 dump 함수를 만든다.
- `mm_malloc`, `mm_free`, `mm_realloc` 직후마다 조건부로 checker를 호출할 수 있게 한다.
- trace의 특정 지점에서만 활성화 가능하게 하면 디버깅 효율이 올라간다.

---

# 24. 디버깅 전략

malloclab은 “한 번에 완성”이 거의 불가능하다. 좋은 디버깅 전략이 필요하다.

## 24.1 작은 trace부터

큰 trace를 돌리기 전에 다음 시나리오를 손으로 추적 가능한 수준에서 검증한다.

- alloc 하나
- alloc 둘
- free 하나
- split 발생
- 4-case coalesce 각각
- realloc grow/shrink

---

## 24.2 시각화

힙 상태를 아래처럼 출력하면 디버깅이 훨씬 쉬워진다.

```text
[addr=0x... size=32 alloc=1]
[addr=0x... size=64 alloc=0 pred=... succ=...]
[addr=0x... size=16 alloc=1]
```

특히 explicit/segregated allocator에서는 **힙 순회 결과**와 **free list 순회 결과**를 동시에 비교하는 것이 중요하다.

---

## 24.3 assertion 적극 사용

예:

- `assert(GET_SIZE(HDRP(bp)) % DSIZE == 0)`
- `assert(!GET_ALLOC(HDRP(bp)))` before inserting into free list
- `assert(in_heap(bp))`
- `assert(aligned(bp))`

assert는 crash를 빨리 일으켜 주므로, 조용히 corruption이 전파되는 것보다 훨씬 낫다.

---

# 25. 성능 튜닝 포인트

## 25.1 탐색 길이 제한

segregated class 내부에서 무조건 끝까지 탐색하지 않고, 일정 개수만 보고 멈추는 bounded search를 쓸 수 있다.

장점:
- throughput 향상
- best-fit 비용 제한

단점:
- 완전한 best fit보다 utilization 손해 가능

---

## 25.2 클래스 수 조정

segregated list의 클래스 수를 늘리면 fit 품질은 좋아질 수 있지만, 관리 오버헤드가 증가한다. 너무 적으면 각 class가 너무 거칠어져 탐색이 길어질 수 있다.

즉, 클래스 수는 tuning parameter다.

---

## 25.3 split threshold 조정

남는 조각이 최소 크기 이상이라도 너무 작은 remainder는 성능에 해로울 수 있다. 그래서 단순 minimum보다 조금 큰 threshold를 두어 split을 덜 공격적으로 하는 전략도 가능하다.

---

## 25.4 eager vs lazy policy mix

- immediate coalescing 유지
- 하지만 `realloc` shrink 시에는 split을 보수적으로
- large block은 별도 class에서 다루기

이처럼 정책을 workload에 맞게 조합할 수 있다.

---

# 26. 흔한 오개념 정리

## 26.1 “best fit이면 항상 utilization이 최고다”

아니다. 탐색 비용, split 패턴, free list 구조에 따라 실제 결과는 달라진다.

## 26.2 “explicit list면 무조건 빠르다”

대체로 빠르지만, free list 조작 비용과 coalescing 비용, class 선택 비용까지 고려해야 한다.

## 26.3 “footer는 없어야 좋은 allocator다”

항상 그렇지 않다. footer 제거는 공간 이득이 있지만 구현 복잡성과 버그 위험이 증가한다. malloclab에서 correctness를 먼저 확보하는 것이 더 중요하다.

## 26.4 “`realloc`은 malloc-copy-free면 충분하다”

정답은 맞지만 성능이 크게 손해날 수 있다. 특히 realloc-heavy trace에서는 allocator 품질 차이가 많이 드러난다.

---

# 27. 추천 구현 로드맵

## 27.1 최소 정답 로드맵

- implicit list
- first fit
- immediate coalescing
- simple realloc
- 기본 checker

목표:
- correctness 확보
- trace 통과

종료 기준:
- `mdriver`의 주요 trace에서 correctness가 안정적으로 유지된다.
- split, coalescing, checker가 함께 동작해 기본 allocator 불변식이 깨지지 않는다.

---

## 27.2 균형형 로드맵

- explicit free list
- LIFO insertion
- first fit
- immediate coalescing
- in-place realloc grow 일부 지원
- 강화된 checker

목표:
- 구현 복잡도 대비 높은 점수
- 디버깅 가능성 확보

종료 기준:
- free list insert/remove/coalesce가 일관되게 유지된다.
- 기본 해법 대비 성능 개선이 확인되면서도 checker로 회귀 검증이 가능하다.

---

## 27.3 고득점 로드맵

- segregated free lists
- class 내부 bounded best fit
- immediate coalescing
- prev_alloc 최적화 가능
- 적극적 realloc 최적화
- 강력한 heap checker + dump 도구

목표:
- throughput/utilization 동시 개선
- allocator 설계 자체를 학습

종료 기준:
- class 정책, `realloc`, `prev_alloc` 최적화가 서로 충돌하지 않고 유지된다.
- dump 도구와 checker를 이용해 고급 최적화 이후에도 버그를 추적할 수 있다.

---

# 28. malloclab을 통해 얻어야 하는 핵심 감각

malloclab을 잘 끝냈다는 것은 단지 코드를 제출했다는 뜻이 아니다. 진짜로 얻어야 하는 것은 다음 감각이다.

## 28.1 메모리는 추상적이지 않다

포인터 산술, 헤더, 푸터, 정렬, 경계 조건이 모두 실제 바이트 수준 레이아웃 문제라는 감각을 얻어야 한다.

## 28.2 자료구조 선택은 성능을 바꾼다

implicit, explicit, segregated의 차이는 단순한 구현 스타일 차이가 아니라, 탐색 비용과 단편화 패턴을 바꾸는 설계 선택이다.

## 28.3 좋은 시스템 코드는 검증 도구와 함께 간다

checker 없는 allocator는 신뢰할 수 없다. 검증은 구현의 일부다. 특히 explicit/segregated allocator에서는 힙 전체를 검사하는 checker와 free list 구조를 검사하는 checker를 분리해 두면 디버깅 효율이 훨씬 좋아진다.

## 28.4 정책은 workload와 분리될 수 없다

어떤 allocator가 “좋다”는 말은 항상 어떤 trace, 어떤 요청 분포, 어떤 성능 목표를 기준으로 한 말이다.

---

# 29. 최종 요약

malloclab의 핵심은 다음 문장으로 요약할 수 있다.

> 동적 메모리 할당기는 힙 위에 세워진 작은 메모리 관리자다.  
> 이 관리자는 블록 레이아웃, free list 자료구조, fit policy, split/coalesce 전략, realloc 최적화, 그리고 검증 도구를 조합해  
> 속도와 공간 효율 사이의 균형을 맞춰야 한다.

따라서 malloclab은 단순히 `malloc`을 흉내 내는 과제가 아니라,

- 저수준 메모리 표현
- 자료구조 설계
- 성능 trade-off
- 디버깅과 불변식 검증

을 동시에 훈련하는 매우 좋은 시스템 프로그래밍 과제다.

---

# 30. 심화 확인 질문

1. implicit free list에서 first fit이 가지는 구조적 병목은 정확히 무엇인가?
2. explicit free list에서 LIFO 삽입은 왜 throughput에 유리한가?
3. address-ordered free list가 fragmentation에 미칠 수 있는 긍정적 영향은 무엇인가?
4. segregated free list의 class 경계를 어떻게 정하면 좋을지, 그 이유와 함께 설명할 수 있는가?
5. allocated block의 footer를 제거하려면 어떤 추가 상태가 필요한가?
   - 힌트: 다음 블록 헤더에 반영되는 `prev_alloc` 비트를 떠올려 보자.
6. `realloc`을 제자리 확장할 때 free list와 metadata는 어떤 순서로 갱신하는 것이 안전한가?
7. heap checker가 없다면 어떤 종류의 버그가 가장 늦게 발견되는가?
8. throughput과 utilization 사이의 trade-off를 실제 trace 관점에서 설명할 수 있는가?

검토 포인트:
- 답을 할 때는 correctness, 탐색 비용, 메타데이터 오버헤드, 단편화, 디버깅 가능성을 함께 묶어서 설명해 본다.
- 특정 정책이 더 낫다고 말할 때는 어떤 trace 또는 workload를 가정했는지 같이 말해 본다.

---

# 31. 실전 과제 제안

이 문서를 읽은 뒤 다음 순서로 직접 실습해보면 좋다.

1. implicit allocator를 직접 구현하고, 4-case coalescing을 손으로 검증한다.
2. heap checker를 만든 뒤 intentional bug를 심어 잡히는지 확인한다.
3. explicit free list로 바꾸고, insert/remove/coalesce 상호작용을 추적한다.
4. segregated list를 도입하고 class 수를 바꿔 성능 차이를 관찰한다.
5. naive `realloc`과 optimized `realloc`의 trace 성능 차이를 비교한다.

이 과정을 거치면 malloclab은 단순 과제가 아니라 allocator 설계 실험실이 된다.
