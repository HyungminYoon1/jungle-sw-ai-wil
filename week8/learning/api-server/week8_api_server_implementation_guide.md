# Week 8 API 서버 실전 구현 초안

## 주제: thread pool 기반 API 서버와 7주차 SQL 처리기를 연결해 최소 구현으로 완성하려면 무엇을 먼저 결정해야 하는가

## 이 문서의 목표

이 문서는 8주차 수요 코딩회 구현을 바로 시작할 때 참고하는 `실전 초안`이다.

개념 설명은 별도 학습 문서에서 충분히 다루고, 여기서는 아래 질문에 집중한다.

- 최소 구현 범위를 어떻게 정할 것인가
- 어떤 구조로 파일과 책임을 나눌 것인가
- 어떤 순서로 구현하면 덜 위험한가
- 실제로 어디에서 문제가 많이 생기는가
- README와 발표에서는 무엇을 보여줘야 하는가

이 문서는 공식 API 명세가 아니라, 공지와 7주차 엔진 문서를 기준으로 잡은 구현 초안이다.

# 1. 이번 주 구현 목표를 한 문장으로 요약하면

`외부 클라이언트가 보낸 요청을 thread pool 기반 API 서버가 받아, 7주차 SQL 처리기를 내부 DB 엔진으로 호출하고, 결과를 안전하게 응답으로 돌려주는 구조를 완성하는 것`

# 2. 시작하기 전에 반드시 인정해야 하는 전제

7주차 SQL 처리기는 좋은 출발점이지만, 이번 주 API 서버에 무수정 그대로 붙이기에는 제약이 있다.

- CSV 기반 저장소
- append-only 전제
- `INSERT`, `SELECT`, `WHERE id = ...` 중심
- `UPDATE`, `DELETE`, 고급 질의 제외
- 동시성 제어는 원래 제외 범위

따라서 이번 주 구현에서는 아래 원칙이 중요하다.

- 엔진은 최대한 재사용한다.
- 엔진이 원래 보장하지 않던 동시성은 API 서버 쪽에서 보수적으로 보호한다.
- API 범위는 엔진이 안정적으로 처리 가능한 범위로 제한한다.

# 3. 최소 구현 범위는 어디까지가 좋은가

## 3.1 추천 최소 범위

- `GET /health`
- `POST /query`
- worker thread + 작업 큐
- 내부 DB 엔진 adapter
- 테이블 단위 lock 기반 엔진 보호
- 기본 단위 테스트와 API 테스트

## 3.2 추천 SQL 지원 범위

- `INSERT`
- `SELECT *`
- `SELECT columns`
- `SELECT ... WHERE id = <number>`

이 범위는 사실상 single-table query 중심이며, 이번 문서의 table-level lock 설명도 이 전제에서 가장 자연스럽다.

## 3.3 처음부터 과하게 욕심내면 위험한 범위

- 복수 REST 엔드포인트 세분화
- `UPDATE`, `DELETE`
- 복잡한 `WHERE`
- 트랜잭션
- MVCC, snapshot 같은 고급 동시성 모델
- 같은 테이블 안에서의 read/write lock 세분화

이번 주는 `기능을 많이 넣는 것`보다 `설명 가능한 구조`가 더 중요하다.

# 4. 가장 설명하기 쉬운 API 형태는 무엇인가

공식 API 명세가 없는 상태라면 가장 설명하기 쉬운 형태는 `단일 SQL 실행 API`다.

예시:

- `GET /health`
- `POST /query`

`POST /query`는 공지의 공식 명세가 아니라, 7주차 SQL 처리기를 가장 단순하게 감싸기 위한 최소 구현 제안이다.

요청 예시:

```json
{
  "sql": "SELECT * FROM users WHERE id = 1;"
}
```

응답 예시:

```json
{
  "ok": true,
  "data": "...",
  "error": null
}
```

장점:

- 7주차 SQL 처리기와 자연스럽게 연결된다.
- 구현 난도가 낮다.
- README 설명이 쉽다.

단점:

- SQL 문자열 의존성이 강하다.
- 일반적인 서비스 API처럼 보이지 않는다.

하지만 이번 과제의 목적을 생각하면 충분히 합리적인 선택이다.

# 5. 권장 모듈 구조 초안

```text
server/
  main.c
  server.c
  worker_pool.c
  task_queue.c

http/
  request.c
  response.c
  router.c

api/
  health_handler.c
  query_handler.c

service/
  db_service.c

engine/
  sql_engine_adapter.c
  engine_lock.c

week7_sql_engine/
  parser / executor / storage / index
```

핵심은 `week7_sql_engine`을 직접 뜯어고치기보다 그 바깥에 `adapter`와 `engine_lock`을 두는 것이다.

## 5.1 각 모듈의 책임

- `server`: 소켓 열기, listen, accept, worker 시작
- `worker_pool`: worker thread 생성과 종료
- `task_queue`: 작업 enqueue/dequeue
- `http`: 요청 파싱과 응답 생성
- `api`: 경로별 handler
- `service`: API 요구를 엔진 호출 형태로 변환
- `engine adapter`: 7주차 엔진 호출, 결과 변환, 락 적용

# 6. 구현 순서는 어떻게 잡는 것이 좋은가

## 6.1 1단계: 최소 서버 + health check

먼저 thread pool 없이도 좋으니 요청을 받고 `200 OK`를 돌려주는 최소 서버를 만든다.

확인할 것:

- bind/listen/accept가 되는가
- 요청을 받고 응답이 나가는가
- 서버가 쉽게 죽지 않는가

## 6.2 2단계: `POST /query` 요청 파싱

다음으로 요청 body에서 SQL 문자열을 읽는 흐름을 만든다.

확인할 것:

- 메서드 확인
- body 읽기
- 빈 SQL, 잘못된 JSON 처리

## 6.3 3단계: 7주차 엔진 단독 연결

아직 thread pool 없이, 서버 요청 하나가 7주차 엔진을 호출해 결과를 돌려주는 흐름을 만든다.

확인할 것:

- SQL 문자열이 실제 엔진 실행 함수로 전달되는가
- 결과가 응답으로 변환되는가
- 엔진 오류가 응답으로 전달되는가

## 6.4 4단계: table-level lock 추가

여기서부터 공유 상태 보호를 붙인다.

이때 `target table` 해석은 가능한 한 raw SQL/body 기준의 stateless pre-parse 수준으로 제한하는 편이 안전하다.
즉 lock을 잡기 전에 7주차 엔진의 전역 registry나 공유 메타정보를 먼저 건드리는 구조는 피하는 것이 좋다.

확인할 것:

- 요청에서 대상 테이블을 식별하는가
- 해당 테이블 lock을 잡고 엔진을 호출하는가
- 같은 테이블의 뒤 요청이 앞선 write를 앞지르지 않는가
- 다른 테이블 요청은 병렬로 통과할 수 있는가

주의:

- table-level lock만 있다고 세션 순서 보장이 자동으로 생기는 것은 아니다.
- 같은 클라이언트의 `INSERT -> SELECT` 기대를 지키려면, 앞선 요청 완료를 기준으로 다음 요청을 보내는 흐름이나 요청 순서를 무시하지 않는 처리 흐름이 함께 맞아야 한다.

## 6.5 5단계: task queue + worker pool 추가

이제 accept loop와 실제 처리 thread를 분리한다.

확인할 것:

- queue push/pop이 안전한가
- worker가 요청을 정상 처리하는가
- 종료 처리와 close 누락이 없는가

## 6.6 6단계: 테스트와 README 정리

마지막에 테스트와 문서를 붙인다.

# 7. DB 엔진 adapter가 왜 반드시 필요한가

7주차 SQL 처리기를 API 서버에서 바로 부르면 처음에는 빨라 보일 수 있다.
하지만 오래 가지 않으면 아래 문제가 생긴다.

- HTTP 파싱 코드와 엔진 호출 코드가 섞인다.
- 에러를 어디서 어떤 형식으로 바꿔야 하는지 애매해진다.
- 테스트에서 엔진 호출만 분리하기 어렵다.

adapter는 아래 책임을 모으는 층이다.

- 입력 검증 이후 엔진 호출
- 대상 테이블 해석
- 엔진 결과를 API 응답 구조로 변환
- 공통 에러 처리
- table-level lock 적용

즉 `service -> adapter -> week7 engine` 구조를 유지하는 것이 좋다.

# 8. 가장 단순하면서도 현실적인 엔진 보호 전략

이번 주 구현에서 권장하는 기본 전략은 `테이블 단위 lock`이다.

```text
worker thread
  -> parse request
  -> validate
  -> resolve target table
  -> lock(table_lock[table_name])
  -> execute week7 engine
  -> copy result to local buffer/struct
  -> unlock(table_lock[table_name])
  -> format response
```

이 방식의 장점:

- 같은 테이블의 정합성을 보수적으로 지키기 쉽다.
- 다른 테이블 요청은 병렬로 진행할 수 있다.
- 한 테이블의 긴 작업이 모든 요청을 막는 구조를 피할 수 있다.

이 방식의 단점:

- 같은 테이블의 긴 write가 있으면 같은 테이블의 뒤 read/write는 기다려야 한다.
- 테이블 이름 해석, lock manager, 예외 경로 정리가 전역 mutex보다 복잡하다.

즉 이 방식은 `다른 테이블끼리는 병렬`, `같은 테이블은 정합성을 위해 보수적으로 직렬화`에 가깝다.

`전역 mutex`는 여전히 fallback이 될 수 있다.
디버깅 단계에서 문제 범위를 줄이거나, table-level lock 구현 전 임시 보호 장치로는 유용하다.
하지만 최종 설명용 기본 전략으로는 공지의 `병렬 SQL 처리` 요구에 더 가까운 `테이블 단위 lock`이 낫다.

# 9. 실제로 많이 터지는 포인트

## 9.1 전역 registry race

7주차 인덱스 registry가 전역 상태라면 여러 worker가 동시에 접근할 때 race가 날 수 있다.

대응:

- registry 접근을 테이블 단위 lock 경계 안으로 넣기
- registry를 직접 건드리는 코드를 adapter/engine 경계 안쪽으로 모으기
- 디버깅 단계에서는 전역 mutex fallback으로 문제 범위를 좁히기

추가 주의:

- `table name -> mutex`를 관리하는 lock manager 자체도 공유 상태일 수 있다.
- lock 객체를 lazy하게 만들거나 전역 맵에서 조회한다면, 그 초기화/조회 경계도 별도 보호가 필요할 수 있다.

## 9.2 next id 중복

동시에 여러 `INSERT`가 들어오면 같은 id가 중복될 수 있다.

대응:

- id 생성, CSV append, index update를 하나의 critical section으로 묶기

## 9.3 파일 기록과 인덱스 갱신 불일치

CSV는 기록됐는데 인덱스 반영이 실패하거나, 인덱스는 반영됐는데 파일 기록이 실패할 수 있다.

대응:

- 쓰기 순서를 명확히 정하기
- 실패 시 어떤 상태를 허용할지 정하기
- 최소한 재시작 후 rebuild 가능한 구조인지 확인하기

## 9.4 lazy rebuild 중 동시 조회

인덱스를 rebuild하는 동안 다른 요청이 들어오면 중간 상태를 읽을 수 있다.

대응:

- rebuild도 같은 테이블 lock 경계 안에서 수행하기

## 9.5 lock을 너무 오래 잡음

엔진 결과를 문자열로 포매팅하거나 응답을 JSON으로 바꾸는 동안까지 lock을 잡고 있으면 병목이 커진다.

대응:

- 엔진 호출과 결과 복사까지만 lock 안에서 처리하기
- 응답 직렬화는 lock 밖에서 수행하기

## 9.6 같은 테이블의 대량 INSERT가 뒤 요청을 오래 막음

한 클라이언트가 같은 테이블에 대량 `INSERT`를 수행하면, 그 뒤에 온 같은 테이블의 `SELECT`나 추가 `INSERT`는 오래 기다릴 수 있다.

대응:

- 같은 테이블에 대해서는 순서와 정합성을 우선한다고 README에 분명히 적기
- lock 밖에서 할 수 있는 파싱, 검증, 응답 포매팅은 최대한 먼저 또는 나중으로 빼기
- 필요하면 요청 크기 제한이나 배치 처리 정책을 두기
  배치 처리는 병목을 줄일 수 있지만, 한 요청이 한 번에 원자적으로 반영된다는 기대나 가시성 의미를 약하게 만들 수 있으므로 trade-off를 문서에 명시해야 한다.
- 대신 다른 테이블 요청은 병렬로 통과되도록 table-level lock을 유지하기

## 9.7 에러 경로에서 unlock 누락

실패 케이스가 많아질수록 `return` 경로에서 unlock 누락이 자주 생긴다.

대응:

- 단일 exit 패턴
- cleanup label 사용
- adapter 내부에서 공통 패턴 강제

## 9.8 HTTP 요청 파싱이 약함

body 길이, 개행, 잘못된 JSON, 빈 본문 때문에 서버가 쉽게 깨질 수 있다.

대응:

- 처음에는 아주 단순한 입력 형식만 받기
- 잘못된 입력은 명확한 에러 응답으로 돌려주기

## 9.9 클라이언트 연결 close 누락

worker thread에서 예외 경로가 생기면 소켓 close 누락이 자주 생긴다.

대응:

- 요청 처리 함수 안에서 정리 경로를 하나로 만들기

# 10. HTTP 응답 규칙은 어떻게 잡는가

이 부분도 너무 복잡하게 가면 구현이 느려진다.

추천 규칙:

- `GET /health` 성공: `200 OK`
- 정상 처리된 API 요청: `200 OK`
- 잘못된 요청 형식: `400 Bad Request`
- 지원하지 않는 메서드: `405 Method Not Allowed`
- SQL 문법 오류, 지원하지 않는 SQL, 잘못된 파라미터: `400 Bad Request`
- 파일 I/O 실패, 내부 상태 불일치, 예상하지 못한 엔진 오류: `500 Internal Server Error`
- 없는 경로: `404 Not Found`

중요한 것은 상태코드 개수보다 `일관성`이다.

# 11. 테스트는 무엇을 먼저 확인해야 하는가

공지에서 테스트가 강조되어 있으므로 아래 정도는 반드시 있어야 한다.

## 11.1 단위 테스트

- 작업 큐 enqueue/dequeue
- 요청 검증 함수
- adapter 입력/출력 변환
- engine lock helper

## 11.2 API 기능 테스트

- `GET /health`
- 정상 `POST /query`
- 잘못된 메서드
- 빈 SQL
- 문법 오류 SQL
- `WHERE id = ...` 정상 조회

## 11.3 동시성 테스트

- 같은 테이블에 동시에 여러 read 요청 보내기
- 같은 테이블에 read/write를 섞어서 보내기
- 같은 테이블에 대량 `INSERT` 뒤 `SELECT`를 보내 순서와 정합성이 지켜지는지 확인하기
- 서로 다른 테이블에 대한 요청이 병렬로 처리되는지 확인하기
- 동시에 여러 `INSERT`를 보내도 id 중복이나 데이터 유실이 없는지 확인하기

## 11.4 복구성 확인

- 서버 재시작 후 인덱스 rebuild가 가능한가
- 이전 기록 데이터가 다시 조회되는가

# 12. README에는 무엇을 넣어야 하는가

발표와 README는 이번 주 결과물의 일부다.
따라서 구현이 끝난 뒤 부수적으로 쓰는 것이 아니라, 처음부터 어떤 내용을 보여줄지 생각해야 한다.

추천 항목:

- 프로젝트 목표
- 시스템 구조도
- thread pool 설명
- API 요청/응답 예시
- 내부 DB 엔진이 7주차 SQL 처리기 기반이라는 설명
- 동시성 보호 전략
- 테스트 항목
- 인증/인가 및 접근 제어는 이번 범위에서 제외한 localhost 기반 학습용 최소 구현이라는 설명
- 한계와 추후 개선점

# 13. 발표에서 설명하면 좋은 포인트

발표는 단순히 `동작합니다`가 아니라 `왜 이렇게 설계했는가`를 보여주는 자리다.

특히 아래 포인트가 좋다.

- 왜 SQL 처리기 자체를 새로 만들지 않고 7주차 엔진을 재사용했는가
- 왜 REST를 과하게 세분화하지 않고 단일 SQL 실행 API로 시작했는가
- 왜 thread pool이 필요한가
- 왜 전역 mutex 대신 테이블 단위 lock을 기본 전략으로 선택했는가
- 같은 테이블과 다른 테이블의 동시성 정책을 어떻게 나눴는가
- 어떤 테스트로 정합성을 확인했는가

# 14. 현실적인 구현 로드맵

## 14.1 0단계

- 7주차 엔진 빌드와 단독 실행 다시 확인

## 14.2 1단계

- 최소 health server

## 14.3 2단계

- 최소 `POST /query`

## 14.4 3단계

- week7 engine adapter 연결

## 14.5 4단계

- 테이블 이름 해석 + table-level lock 추가

## 14.6 5단계

- task queue + worker pool

## 14.7 6단계

- 동시성 테스트

## 14.8 7단계

- README 정리와 데모 시나리오 구성

# 15. 최종적으로 팀이 답할 수 있어야 하는 질문

- 이 구조가 이번 주 과제 요구사항과 어떻게 연결되는가
- thread pool은 어디에 있고 어떻게 동작하는가
- 내부 DB 엔진은 어떤 부분을 재사용했는가
- 병렬 요청에서 어떤 위험을 어떻게 막았는가
- 왜 같은 테이블은 보수적으로 잠그고, 다른 테이블은 병렬 허용했는가
- 현재 구조의 한계는 무엇인가

# 16. 마지막 정리

- 이번 주 구현은 `API 서버를 새로 만들고 7주차 SQL 처리기를 내부 엔진으로 감싸는 작업`이다.
- 가장 설득력 있는 시작점은 `단일 SQL 실행 API + thread pool + 테이블 단위 lock` 조합이다.
- 핵심은 기능을 많이 넣는 것이 아니라 `구조적으로 설명 가능한 연결`을 만드는 것이다.
- 특히 전역 상태, index rebuild, next id, CSV append 순서를 반드시 의식하고 구현해야 한다.

이 문서는 실전 초안이므로, 실제 구현 과정에서는 팀이 선택한 API 형식과 파일 구조에 맞춰 조정하면 된다.
