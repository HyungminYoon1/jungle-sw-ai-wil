# Week 8 - WIL (What I Learned)

> 📅 작성 시점: 주차 중~말

매주 진행한 작업과 배운 내용을 정리합니다. 단순 일지보다는 **학습 포인트 / 문제 해결 과정 / 개선 아이디어**를 중심으로 작성합니다.

---

## 1. 이번 주 목표
- `BSD socket`, `IP`, `TCP`, `HTTP`, `file descriptor`, `DNS`를 각각 따로 외우는 것이 아니라, 클라이언트 요청이 서버에 도착하고 응답이 돌아가는 전체 흐름 안에서 설명할 수 있게 만들기
- socket 기반 echo client/server를 통해 `socket -> bind/listen/accept -> read/write -> close` 흐름을 직접 확인하기
- CS:APP 11장의 tiny 웹서버를 구현하면서 정적 콘텐츠, CGI 동적 콘텐츠, 요청 라인, 요청 헤더, HTTP 응답 생성 흐름을 이해하기
- proxy 서버 과제에서 클라이언트와 원 서버 사이의 중계 흐름을 구현하고, 병렬 처리와 캐시가 왜 필요한지 확인하기
- 수요 코딩회에서는 7주차 SQL 처리기와 B+ Tree 인덱스를 내부 DB 엔진으로 재사용하고, 그 바깥에 HTTP API 서버와 병렬 요청 처리 구조를 붙이기
- 구현 결과는 README와 테스트 결과를 기준으로 설명할 수 있게 정리하고, 특히 thread pool, bounded queue, table-level lock, engine adapter의 역할을 말로 설명할 수 있게 만들기

## 2. 시도한 접근 방식
- 먼저 네트워크 키워드를 요청 흐름 중심으로 정리했다. `DNS`는 이름을 주소로 바꾸는 단계, `IP/port`는 목적지를 찾는 정보, `TCP`는 연결 기반 전송, `socket/file descriptor`는 프로그램이 네트워크 입출력을 다루는 창구이며, `HTTP`는 요청과 응답의 형식으로 나눠 이해했다. 첨언하자면 socket은 통신 endpoint이고, 프로세스는 이를 file descriptor로 다룬다.
- echo client/server를 먼저 보면서 웹서버 이전의 가장 단순한 요청/응답 흐름을 확인했다. 이 단계에서는 HTTP를 의식하기보다 서버가 리스닝 소켓으로 기다리고, `accept` 이후 연결 소켓으로 데이터를 읽고 쓰는 구조를 잡는 데 집중했다.
- tiny 웹서버는 요청 한 건이 `doit` 함수 안에서 어떻게 처리되는지를 따라갔다. 요청 라인을 읽고, 헤더를 소비하고, URI를 파일 경로와 CGI 인자로 나누고, 정적 파일 또는 동적 CGI 실행으로 분기하는 흐름을 함수 단위로 정리했다.
- 숙제 문제는 본선 tiny 파일을 크게 흔들지 않도록 문제별 실험 파일로 분리해서 풀었다. HTTP 버전 확인, MPG 파일 타입 추가, `mmap` 대신 `malloc/Rio_readn/Rio_writen` 사용, `HEAD`와 `POST` 처리, `SIGPIPE` 대응처럼 요구사항별로 어떤 동작 차이가 생기는지 확인했다.
- proxy 서버는 처음에 요청 전달 흐름을 학습하고 구현했다. 클라이언트 요청에서 method, URI, version을 읽고, absolute URI를 host/path/port로 나눈 뒤, 원 서버로 보낼 HTTP/1.0 요청 헤더를 다시 구성하고, 원 서버 응답을 클라이언트로 전달하는 방식으로 접근했다.
- 이후 proxy에 스레드 기반 병렬 처리와 URI 기반 캐시를 붙였다. 각 연결을 detached thread가 처리하게 만들고, 캐시는 전역 LRU 리스트와 mutex로 보호해서 여러 요청이 동시에 접근해도 응답 복사와 저장 과정이 깨지지 않게 했다.
- 수요 코딩회 API 서버는 기존 SQL 처리기를 새로 만들지 않고, 7주차 결과물을 내부 엔진으로 감싸는 방향으로 설계했다. API 서버 계층, HTTP 계층, handler 계층, service 계층, engine adapter 계층을 분리하고, 기존 SQL/실행/storage/index 계층은 가능한 한 유지하는 방식으로 접근했다.
- 병렬 SQL 처리는 무조건 모든 요청을 동시에 실행하는 방식이 아니라, 같은 물리 테이블은 직렬화하고 다른 테이블은 병렬 처리하는 정책으로 정리했다. 현재 엔진에는 transaction이나 MVCC가 없으므로, 같은 테이블의 `INSERT`, `SELECT`, lazy rebuild, index registry 접근은 보수적으로 보호하는 편이 더 안전하다고 판단했다.

## 3. 문제와 해결 과정

### 네트워크 키워드가 따로 노는 것처럼 느껴졌던 점
- **문제:** 처음에는 `BSD socket`, `IP`, `TCP`, `HTTP`, `DNS`, `file descriptor`가 명확하게 다가오지 않았고 어느 순서로 학습해야 할지 고민했다. TCP와 HTTP의 차이, socket과 file descriptor의 관계를 명확히 설명하지 못하였고 tiny나 proxy 코드를 읽을 때 해석이 어렵게 느껴졌다.
- **해결:** 브라우저 요청 흐름 하나를 기준으로 다시 정리했다. 도메인을 DNS로 IP로 바꾸고, IP와 port로 TCP 연결을 맺고, 그 연결 위에서 HTTP 메시지를 주고받으며, C 프로그램은 그 연결을 file descriptor 형태로 읽고 쓴다고 보니 개념들이 한 흐름으로 연결되었다. 이후 echo, tiny, proxy 코드를 하나씩 구현하고 코드 리뷰를 하며 "지금은 연결을 여는 단계인지, HTTP 메시지를 해석하는 단계인지, 응답을 쓰는 단계인지"를 구분할 수 있었다.

### tiny 웹서버에서 정적 콘텐츠와 동적 콘텐츠 분기가 헷갈렸던 점
- **문제:** 처음 tiny 코드를 볼 때는 URI를 파일명으로 바꾸고, `cgi-bin` 포함 여부에 따라 정적/동적 콘텐츠로 나누는 방식 뒤에 파일 권한, CGI 인자, `fork/execve`, stdout 리다이렉션까지 함께 이해해야 했다.
- **해결:** `parse_uri`, `serve_static`, `serve_dynamic`의 책임을 분리해서 봤다. `serve_static`은 파일을 읽어 HTTP 응답 body로 보내는 흐름이고, `serve_dynamic`은 CGI 프로그램을 실행한 뒤 그 출력이 클라이언트로 가도록 연결하는 흐름으로 정리했다. 이렇게 분할하고 나니 `cgi-bin/adder`의 동작이 더 분명하게 이해됐다.

### proxy 서버는 서버이면서 클라이언트이기도 하다는 점
- **문제:** proxy는 tiny처럼 요청을 받아 응답하는 서버라고만 생각하면 구조가 잘 맞지 않았다. 실제로는 브라우저 입장에서는 서버지만, 원 서버 입장에서는 클라이언트처럼 다시 연결을 만들고 요청을 보내야 했다.
- **해결:** proxy 요청 한 건을 두 개의 연결로 나눠 봤다. 첫 번째는 브라우저와 proxy 사이의 client connection이고, 두 번째는 proxy와 원 서버 사이의 server connection이다. 이 관점으로 보니 절대 URI 파싱, Host 헤더 재구성, `Connection: close`, `Proxy-Connection: close`, User-Agent 지정이 왜 필요한지 이해할 수 있었다.

### proxy 병렬 처리에서 공유 캐시를 안전하게 다뤄야 했던 점
- **문제:** 연결마다 thread를 만들면 여러 요청을 동시에 처리할 수 있지만, 전역 캐시를 여러 thread가 동시에 읽고 쓰면 LRU 리스트나 캐시 크기가 깨질 수 있었다. 특히 캐시 hit 시 LRU 위치를 갱신하고, cache miss 후 새 응답을 저장하면서 오래된 블록을 제거하는 작업은 모두 공유 상태를 변경했다.
- **해결:** 캐시 탐색, LRU 갱신, 저장, eviction(기존 캐시 데이터 삭제)을 하나의 mutex로 보호했다. 다만 캐시 hit에서 클라이언트로 응답을 보내는 동안까지 락을 잡으면 병목이 커지므로, 캐시 내부 데이터를 복사한 뒤 락을 풀고 복사본을 응답에 사용하도록 했다. 이 과정에서 동시성 제어는 "무조건 오래 잠그기"가 아니라 "공유 상태가 깨지는 구간만 정확히 잠그기"가 중요하다는 점을 느꼈다.

### API 서버에서 기존 SQL 처리기를 어떻게 재사용할지 정하는 문제
- **문제:** 8주차 API 서버는 외부 클라이언트 요청을 받아 SQL을 실행해야 했지만, 7주차 SQL 처리기는 원래 단일 실행 흐름을 전제로 만들어졌다. HTTP handler에서 곧바로 parser, executor, storage, index를 호출하면 계층 책임이 섞이고, 동시성 문제도 어디서 보호해야 하는지 애매해질 수 있었다.
- **해결:** API 서버와 기존 엔진 사이에 service와 engine adapter를 두었다. HTTP 계층은 요청 파싱과 응답 생성에 집중하고, API handler는 endpoint별 검증을 담당하고, service는 API 입력을 엔진 요청으로 바꾸고, engine adapter는 SQL 파싱, 대상 테이블 식별, 락 획득, 기존 executor 호출, 출력 캡처, 오류 변환을 담당하도록 나눴다. 이렇게 하니 기존 SQL 처리기 구조를 크게 훼손하지 않으면서도 API 서버의 책임을 분리할 수 있었다.

### 병렬 SQL 처리와 데이터 정합성 사이의 균형
- **문제:** thread pool을 붙이면 여러 SQL 요청이 동시에 들어올 수 있다. 하지만 기존 엔진은 CSV 파일, B+ Tree, table index registry, 자동 id 같은 공유 상태를 갖고 있어 아무 보호 없이 병렬 실행하면 중복 id, 인덱스 불일치, rebuild race가 생길 수 있었다.
- **해결:** 같은 물리 테이블 요청은 `storage_name` 기준 table-level exclusive lock으로 직렬화하고, 다른 테이블 요청은 병렬로 처리할 수 있게 했다. 전역 index registry는 별도 mutex로 보호하되, registry mutex가 테이블 전체 정합성을 대신하지 않는다는 점도 분리했다. 이 설계를 통해 "병렬 처리"를 모든 요청의 무조건 동시 실행이 아니라, 안전하게 분리 가능한 범위까지 병렬화하는 것으로 이해하게 되었다.

### 테스트 범위가 생각보다 넓어졌던 점
- **문제:** API 서버는 정상 SQL 요청만 처리하면 끝나는 것이 아니었다. 잘못된 JSON, Content-Type 오류, Content-Length 누락, body 크기 초과, header 크기 초과, 지원하지 않는 HTTP 기능, SQL parser 오류, 존재하지 않는 경로, queue full, graceful shutdown까지 확인해야 했다.
- **해결:** 테스트를 `엔진 회귀`, `API 기능`, `HTTP 입력 검증`, `오류 매핑`, `동시성`, `복구성`으로 나눠 봤다. 자동 테스트에서는 정상 응답뿐 아니라 오류 코드와 상태 코드가 의도대로 매핑되는지, 같은 테이블 동시 접근이 직렬화되는지, 다른 테이블 요청은 병렬 처리 가능한지, 서버 재시작 후 인덱스 rebuild가 되는지까지 검증했다. 최종적으로 자동 테스트 387개가 실패 없이 통과한 것을 확인하며, 서버 과제에서는 테스트가 매우 중요하다는 점을 다시 느꼈다.

## 4. 새로 배운 점
- socket은 운영체제가 제공하는 입출력 대상이라는 점을 배웠다. 파일처럼 descriptor로 읽고 쓰기 때문에 네트워크 프로그램도 결국 `read`, `write`, `close`의 흐름으로 볼 수 있었다. 엄밀하게 표현하면 socket 자체를 프로세스가 file descriptor로 다룬다.
- TCP와 HTTP를 구분하여 설명할 수 있게 되었다. TCP는 안정적인 연결과 바이트 스트림을 제공하고, HTTP는 그 위에서 요청과 응답의 의미를 정하는 형식이다.
- tiny 웹서버는 작지만 웹서버의 핵심을 거의 모두 담고 있었다. 요청 라인 파싱, 헤더 처리, 정적 파일 전송, CGI 실행, 오류 응답 생성만 따라가도 웹서버가 하는 일을 큰 틀에서 이해할 수 있었다.
- proxy는 중간 전달자이기 때문에 서버와 클라이언트 양쪽 역할을 모두 갖는다. 이 구조를 직접 구현해 보니 프레임워크나 브라우저 뒤에 숨어 있던 HTTP 요청 재구성 과정이 훨씬 구체적으로 보였다.
- 병렬 처리는 단순히 thread를 많이 만드는 문제가 아니었다. 어떤 상태가 공유되는지, 어디까지 잠글지, 락을 잡은 상태에서 오래 걸리는 I/O를 수행해도 되는지까지 함께 생각해야 했다.
- 캐시가 꽉 찼을 때 무엇을 버릴지, 객체 크기를 얼마로 제한할지, hit 시 복사를 어떻게 할지, thread-safe 접근은 어떻게 구현할지 같은 설계를 함께 고려해야 한다는 것을 배웠다.
- API 서버에서 중요한 것은 endpoint를 많이 만드는 것이 아니라, 외부 요청과 내부 엔진 사이의 경계를 명확히 세우는 것이었다. HTTP 파싱, API 검증, service, engine adapter, storage/index의 계층 간 책임을 나누니 구조를 설명하기 쉬워졌다.
- 기존 SQL 처리기는 단일 실행 흐름으로 만들어졌는데 이 코드를 멀티스레드 서버에서 사용하기 위해 동시성 보호 계층이 필요했다.
- 상용 DBMS에서는 동시성 제어를 위해 row-level lock이나 MVCC(Multi-Version Concurrency Control, 다중 버전 동시성 제어)처럼 더 세분화된 동시성 제어를 사용한다는 것을 배웠다. 이번에 작성한 수요코딩회 프로젝트에서는 단일 테이블 중심의 SQL 처리기와 CSV 저장소를 사용하는 상황이고, 학우들에게 설명 가능한 구현을 하기 위해 `table-level lock` 중심으로 최소 구현을 하였다.
- 테스트는 기능 확인뿐 아니라 설계 의도를 고정하는 역할을 했다. 예를 들어 queue full은 `503`, chunked 요청은 `501`, 같은 테이블 동시 요청은 직렬화하는 것과 같이 정책을 테스트로 남기면 구현과 문서가 더 일관되게 유지된다.

## 5. 다음 주 계획
- 다음 주부터는 PintOS `Project 1 - Threads`에 들어간다. 이번 주까지는 사용자 공간의 서버와 네트워크 요청을 다뤘다면, 다음 주부터는 커널 수준 스레드의 생성, 스케줄링, 동기화를 직접 구현하는 방향으로 학습 범위가 바뀐다.
- 첫날에는 팀 GitHub Projects와 팀 repository를 준비하고, 개인별 branch를 나눠 작업할 계획이다. 최종 master branch가 하나의 팀 결과물이 되므로 PR 리뷰와 병합 기준을 더 엄격하게 가져가야 한다.
- 실습 환경은 Ubuntu 22.04 또는 Docker 기반 PintOS 환경으로 맞추고, 테스트 실행 방법을 먼저 확인한다. 환경 문제로 시간을 잃지 않도록 첫날에 빌드, 기본 테스트 실행, 테스트 유틸 사용 가능 여부를 확인할 예정이다.
- 학습 키워드는 `threads`, `synchronization`, `priority scheduling`, `priority inversion`, `MLFQS(Multi Level Queue Scheduling)`를 중심으로 잡는다. 특히 semaphore, lock, condition variable이 race condition을 어떻게 막는지, priority donation이 왜 필요한지, MLFQS가 어떤 기준으로 우선순위를 계산하는지 정리할 계획이다.
- 구현 목표는 `Project 1` 테스트 케이스 통과다. 옵션 구현이 따로 있는 과제가 아니라 Project 1 범위 전체를 구현해야 하므로, alarm clock, priority scheduling, priority donation, synchronization primitive, MLFQS 관련 테스트를 단계별로 통과시키는 방식으로 접근하려고 한다.
- 매일 팀 코어타임 시간에는 개인 branch의 구현 내용을 PR로 올리고, 팀원들과 함께 핵심 로직과 테스트 결과를 확인한 뒤 master에 병합하는 흐름을 만들 계획이다. 서버 과제에서 README와 테스트가 중요했던 것처럼, PintOS에서도 어떤 테스트가 왜 실패하는지와 어떤 수정으로 통과했는지를 기록할 필요가 있다.
- 다음 주 목요일 오전 주간 공유 발표에는 프로젝트 팀 구성, 구현 및 트러블슈팅, 회고가 포함되어야 하므로, 주중에 문제를 해결할 때마다 미리 발표 자료에 넣을 수 있는 근거를 남길 계획이다.

---

### 📎 참고 링크
- [8주차 webproxy-lab 학습] https://github.com/HyungminYoon1/jungle-sw-ai-webproxy-lab-docker
- [SQL 처리기 + API 서버] https://github.com/Jungle-12-303/week8-team5-db-api
