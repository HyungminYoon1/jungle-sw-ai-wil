# FastAPI Dependency 학습 문서

## 1. Dependency란

Dependency는 한국어로 "의존성"이라고 번역한다.

FastAPI에서 dependency는 API 함수가 실행되기 전에 필요한 값을 준비해주는 함수 또는 객체를 의미한다.

예를 들어 어떤 API가 실행되려면 다음 값들이 필요할 수 있다.

```text
DB session
현재 로그인한 사용자
Authorization 헤더의 token
페이지 번호 검증 결과
공통 권한 확인 결과
```

이런 값을 router 함수 안에서 매번 직접 준비하면 코드가 반복된다.

FastAPI dependency를 사용하면 공통 준비 작업을 함수로 분리하고, router에서는 필요한 값을 선언만 할 수 있다.

## 2. Dependency를 한 문장으로 설명하기

FastAPI dependency는 요청 처리에 필요한 공통 준비물을 router 함수에 자동으로 넣어주는 구조다.

더 쉽게 말하면:

```text
router가 직접 준비하지 않아도 되는 값을
FastAPI가 대신 준비해서 함수 인자로 넣어주는 기능
```

예:

```python
def get_me(current_user: User = Depends(get_current_user)):
    return current_user
```

이 코드에서 router 함수는 `current_user`를 직접 만들지 않는다.

FastAPI가 `get_current_user` dependency를 먼저 실행하고, 그 결과를 `current_user` 인자로 넣어준다.

## 3. 왜 필요한가

Dependency가 필요한 이유는 크게 네 가지다.

```text
1. 중복 코드를 줄인다.
2. router를 얇게 유지한다.
3. 인증, DB session 같은 공통 작업을 재사용한다.
4. 계층별 책임을 분리한다.
```

현재 프로젝트의 아키텍처 기준은 다음과 같다.

```text
router
-> service
-> repository
-> database
```

router는 HTTP 요청과 응답을 처리하는 곳이다.

router 안에 JWT 검증, DB 조회, 권한 판단 같은 코드가 계속 들어가면 router가 너무 많은 책임을 갖게 된다.

Dependency를 사용하면 router는 이렇게 단순해진다.

```text
router: current_user가 필요하다고 선언
dependency: current_user를 준비
service: token 검증과 사용자 확인
repository: DB 조회
```

## 4. Depends()란

`Depends()`는 FastAPI에게 "이 값을 직접 받지 말고, 다른 함수를 실행해서 만들어 주세요"라고 알려주는 도구다.

예:

```python
from fastapi import Depends


def read_items(db: Session = Depends(get_db)):
    ...
```

의미:

```text
read_items()를 실행하기 전에
get_db()를 먼저 실행해서
그 결과를 db 인자에 넣어 주세요.
```

일반적인 Python 함수 호출과 비교하면 차이가 보인다.

직접 호출:

```python
db = get_db()
read_items(db)
```

FastAPI dependency:

```python
def read_items(db: Session = Depends(get_db)):
    ...
```

FastAPI가 요청 처리 과정에서 dependency 실행 순서를 관리한다.

## 5. Dependency 실행 흐름

예를 들어 router가 다음처럼 작성되어 있다고 하자.

```python
@router.get("/me")
def get_me(current_user: User = Depends(get_current_user)):
    return current_user
```

요청이 들어오면 흐름은 다음과 같다.

```text
1. 클라이언트가 GET /me 요청
2. FastAPI가 get_me()를 실행하려고 함
3. get_me()에 current_user dependency가 있는 것을 확인
4. get_current_user()를 먼저 실행
5. get_current_user()의 반환값을 current_user에 넣음
6. get_me() 실행
7. 응답 반환
```

즉, dependency는 router 함수보다 먼저 실행된다.

dependency에서 예외가 발생하면 router 함수까지 가지 않고 바로 오류 응답이 반환된다.

예:

```text
토큰 없음
-> get_current_user()에서 인증 실패
-> /me router 함수는 실행되지 않음
-> 401 Unauthorized 응답
```

## 6. DB session dependency

현재 프로젝트에는 이미 DB session dependency가 있다.

파일:

```text
backend/app/db/session.py
```

함수:

```python
def get_db() -> Generator[Session, None, None]:
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()
```

이 함수의 역할:

```text
1. 요청 하나에 사용할 DB session을 만든다.
2. router 또는 service에서 사용할 수 있게 제공한다.
3. 요청 처리가 끝나면 DB session을 닫는다.
```

`yield`를 사용하는 이유는 요청 처리 전후 작업을 나눌 수 있기 때문이다.

```text
yield 전  = DB session 생성
yield     = API 처리 중 DB session 사용
yield 후  = DB session 정리
```

개념적으로는 다음과 같다.

```text
요청 시작
-> get_db()가 DB session 생성
-> API 함수에서 DB session 사용
-> 요청 종료
-> get_db()가 DB session close
```

## 7. 인증 dependency가 필요한 이유

현재 프로젝트에는 인증이 필요한 API가 여러 개 있다.

예:

```text
GET /auth/me
DELETE /auth/me
POST /posts
PATCH /posts/{post_id}
DELETE /posts/{post_id}
POST /posts/{post_id}/comments
DELETE /posts/{post_id}/comments/{comment_id}
```

이 API들은 모두 현재 로그인한 사용자가 필요하다.

만약 dependency를 사용하지 않으면 각 router마다 다음 코드를 반복해야 한다.

```text
Authorization 헤더 읽기
Bearer token 꺼내기
JWT decode
payload의 sub 확인
DB에서 사용자 조회
탈퇴 사용자 여부 확인
인증 실패 시 401 반환
```

이렇게 하면 문제가 생긴다.

```text
router 코드가 길어진다.
인증 로직이 여러 파일에 흩어진다.
수정할 때 여러 router를 모두 고쳐야 한다.
실수로 어떤 API에서 인증 검증을 빼먹을 수 있다.
```

그래서 인증 dependency를 만든다.

```text
인증이 필요한 router는 current_user만 받는다.
token 추출과 사용자 확인은 dependency/service가 맡는다.
```

## 8. 현재 프로젝트의 인증 dependency 목표

현재 프로젝트의 목표 흐름:

```text
Authorization: Bearer <token>
-> OAuth2PasswordBearer가 token 추출
-> get_db()가 DB session 제공
-> get_current_user() dependency 실행
-> auth service의 get_current_user_from_token(db, token) 호출
-> User 반환
-> router에서 current_user로 사용
```

파일 위치:

```text
backend/app/dependencies/auth.py
```

이 파일의 역할:

```text
인증이 필요한 router에서 공통으로 사용할 current_user dependency를 둔다.
```

## 9. OAuth2PasswordBearer란

`OAuth2PasswordBearer`는 FastAPI가 제공하는 보안 dependency 도구다.

주요 역할은 다음이다.

```text
Authorization 헤더에서 Bearer token을 꺼낸다.
```

요청 예:

```http
GET /api/v1/auth/me HTTP/1.1
Authorization: Bearer fake_access_token
```

`OAuth2PasswordBearer`는 위 요청에서 `fake_access_token` 부분을 추출한다.

사용 예:

```python
oauth2_scheme = OAuth2PasswordBearer(tokenUrl="/api/v1/auth/login")
```

여기서 `tokenUrl`은 Swagger UI 문서에서 "토큰을 받는 URL"을 알려주는 값이다.

중요한 점:

```text
tokenUrl은 실제 로그인 함수를 실행하는 코드가 아니다.
Swagger UI가 인증 버튼을 구성할 때 참고하는 정보다.
```

실제 로그인 처리는 auth router와 auth service에서 한다.

## 10. 인증 dependency 전체 예시

현재 프로젝트에서 작성할 인증 dependency는 다음 형태다.

```python
from fastapi import Depends
from fastapi.security import OAuth2PasswordBearer
from sqlalchemy.orm import Session

from app.db.session import get_db
from app.models.user import User
from app.services.auth import get_current_user_from_token


oauth2_scheme = OAuth2PasswordBearer(tokenUrl="/api/v1/auth/login")


def get_current_user(
    db: Session = Depends(get_db),
    token: str = Depends(oauth2_scheme),
) -> User:
    return get_current_user_from_token(db, token)
```

이 코드는 짧지만 중요한 연결 지점이다.

각 줄의 의미를 나눠서 보면 이해하기 쉽다.

## 11. import 문 설명

```python
from fastapi import Depends
```

`Depends`는 FastAPI dependency를 선언할 때 사용한다.

```python
from fastapi.security import OAuth2PasswordBearer
```

`OAuth2PasswordBearer`는 Authorization 헤더에서 Bearer token을 꺼내기 위해 사용한다.

```python
from sqlalchemy.orm import Session
```

`Session`은 SQLAlchemy DB session 타입이다.

타입 힌트로 사용하면 `db`가 어떤 값인지 읽기 쉽다.

```python
from app.db.session import get_db
```

`get_db`는 요청마다 DB session을 제공하는 dependency다.

```python
from app.models.user import User
```

`User`는 현재 로그인한 사용자 객체의 타입을 표시하기 위해 사용한다.

```python
from app.services.auth import get_current_user_from_token
```

JWT 검증과 사용자 조회는 service에 이미 구현되어 있다.

Dependency는 이 service 함수를 호출해서 현재 사용자를 얻는다.

## 12. oauth2_scheme 설명

```python
oauth2_scheme = OAuth2PasswordBearer(tokenUrl="/api/v1/auth/login")
```

이 변수는 token을 꺼내는 dependency다.

router 요청에 다음 헤더가 있다고 하자.

```text
Authorization: Bearer fake_access_token
```

`oauth2_scheme`은 `fake_access_token` 문자열을 반환한다.

만약 Authorization 헤더가 없거나 Bearer 형식이 아니면 FastAPI가 인증 실패 응답을 만든다.

이때 router 함수는 실행되지 않는다.

## 13. get_current_user 설명

```python
def get_current_user(
    db: Session = Depends(get_db),
    token: str = Depends(oauth2_scheme),
) -> User:
    return get_current_user_from_token(db, token)
```

이 함수는 인증된 현재 사용자를 반환하는 dependency다.

인자 1:

```python
db: Session = Depends(get_db)
```

의미:

```text
get_db()를 실행해서 DB session을 받고,
그 값을 db에 넣어라.
```

인자 2:

```python
token: str = Depends(oauth2_scheme)
```

의미:

```text
Authorization: Bearer <token>에서 token을 꺼내고,
그 값을 token에 넣어라.
```

반환:

```python
return get_current_user_from_token(db, token)
```

의미:

```text
DB session과 token을 auth service에 넘긴다.
service가 JWT를 검증하고 현재 사용자를 찾아서 반환한다.
```

## 14. 왜 dependency에서 직접 JWT를 decode하지 않는가

현재 프로젝트의 설계 원칙은 다음이다.

```text
router에 비즈니스 로직을 넣지 않는다.
router에서 직접 DB에 접근하지 않는다.
service에서 권한 판단과 주요 흐름을 처리한다.
repository는 DB 접근만 담당한다.
```

Dependency도 router와 가까운 HTTP 계층에 있다.

그래서 dependency는 너무 많은 일을 하지 않는 것이 좋다.

인증 dependency의 적절한 책임:

```text
Authorization 헤더에서 token 받기
DB session 받기
service 함수 호출하기
```

service의 책임:

```text
JWT decode
payload의 sub 검증
사용자 ID 변환
사용자 조회
인증 실패 판단
```

repository의 책임:

```text
users 테이블에서 사용자 조회
```

이렇게 나누면 코드 위치가 명확해진다.

## 15. router에서 사용하는 방법

인증 dependency를 만들면 router에서 다음처럼 사용할 수 있다.

```python
@router.get("/me")
def get_me(current_user: User = Depends(get_current_user)):
    return current_user
```

이 코드를 읽는 방법:

```text
/me API는 현재 로그인한 사용자가 필요하다.
현재 로그인한 사용자는 get_current_user dependency로 얻는다.
```

요청 흐름:

```text
GET /auth/me
-> get_current_user 실행
-> token 추출
-> service에서 token 검증
-> User 반환
-> get_me의 current_user 인자에 User 주입
-> User 응답
```

router는 token이 어떻게 생겼는지, JWT가 어떻게 decode되는지 몰라도 된다.

## 16. dependency를 쓰지 않은 코드와 비교

dependency를 쓰지 않으면 router가 이런 일을 직접 해야 한다.

```python
@router.get("/me")
def get_me(authorization: str, db: Session):
    token = authorization에서 직접 추출
    payload = JWT 직접 decode
    user_id = payload에서 sub 추출
    user = DB에서 직접 조회
    if user is None:
        인증 실패
    return user
```

문제:

```text
router가 너무 많은 일을 한다.
JWT 검증 코드가 여러 router에 반복된다.
DB 접근이 router에 들어갈 수 있다.
아키텍처 원칙이 깨진다.
```

dependency를 사용하면 다음처럼 짧아진다.

```python
@router.get("/me")
def get_me(current_user: User = Depends(get_current_user)):
    return current_user
```

차이:

```text
인증 준비는 dependency/service가 담당
HTTP 응답 처리는 router가 담당
DB 조회는 repository가 담당
```

## 17. `backend/app/dependencies/__init__.py`의 역할

`__init__.py`는 해당 폴더를 Python package로 인식하게 하는 파일이다.

현재 구조:

```text
backend/app/dependencies/
├── __init__.py
└── auth.py
```

이 구조를 두면 다른 파일에서 다음처럼 import할 수 있다.

```python
from app.dependencies.auth import get_current_user
```

`__init__.py` 파일 내용은 비어 있어도 된다.

## 18. dependency와 service의 차이

초심자가 헷갈리기 쉬운 부분이다.

Dependency와 service는 모두 함수로 작성될 수 있지만 책임이 다르다.

| 구분 | 역할 |
|---|---|
| dependency | 요청 처리에 필요한 공통 값을 준비한다. |
| service | 비즈니스 로직, 권한 판단, 트랜잭션 흐름을 처리한다. |

현재 인증 예시:

```text
dependency
-> token과 DB session을 준비한다.

service
-> token이 유효한지 판단한다.
-> 어떤 사용자인지 확인한다.
-> 인증 실패 여부를 결정한다.
```

따라서 `get_current_user()`는 dependency이고, `get_current_user_from_token()`은 service 함수다.

## 19. dependency와 repository의 차이

Repository는 DB에 직접 접근하는 계층이다.

Dependency는 DB 접근을 직접 담당하지 않는다.

현재 인증 흐름:

```text
get_current_user dependency
-> get_current_user_from_token service
-> get_user_by_id repository
-> users table
```

역할:

```text
dependency: token과 db를 service에 넘김
service: user_id를 해석하고 인증 실패 판단
repository: user_id로 DB 조회
```

## 20. 인증 실패 흐름

인증 dependency를 사용하는 API에서 인증 실패가 발생하면 router 함수는 실행되지 않는다.

예:

```text
Authorization 헤더 없음
-> oauth2_scheme에서 실패
-> 401 응답
-> router 실행 안 됨
```

다른 예:

```text
Authorization 헤더 있음
-> token 추출 성공
-> get_current_user_from_token() 호출
-> JWT 만료 또는 변조 확인
-> 401 응답
-> router 실행 안 됨
```

이 구조의 장점:

```text
인증 실패 요청이 비즈니스 로직까지 들어오지 않는다.
router 함수는 인증된 사용자만 받는다고 가정할 수 있다.
```

## 21. 현재 프로젝트에서 기억할 흐름

현재 프로젝트의 인증 관련 파일:

```text
backend/app/db/session.py
backend/app/dependencies/auth.py
backend/app/services/auth.py
backend/app/repositories/users.py
backend/app/models/user.py
```

전체 흐름:

```text
클라이언트 요청
Authorization: Bearer <token>
        |
        v
OAuth2PasswordBearer
token 추출
        |
        v
get_current_user dependency
DB session과 token 준비
        |
        v
get_current_user_from_token service
JWT 검증과 사용자 확인
        |
        v
get_user_by_id repository
DB에서 사용자 조회
        |
        v
router
current_user 사용
```

## 22. 자주 하는 실수

### 22.1 Depends import를 빼먹음

문제:

```python
db: Session = Depends(get_db)
```

를 사용하면서 아래 import가 없으면 오류가 난다.

```python
from fastapi import Depends
```

### 22.2 OAuth2PasswordBearer import를 빼먹음

문제:

```python
oauth2_scheme = OAuth2PasswordBearer(...)
```

를 사용하면서 아래 import가 없으면 오류가 난다.

```python
from fastapi.security import OAuth2PasswordBearer
```

### 22.3 dependency에서 DB session을 직접 만들고 닫음

현재 프로젝트에는 이미 `get_db()`가 있다.

따라서 인증 dependency에서 `SessionLocal()`을 직접 호출하지 않는다.

좋은 방향:

```python
db: Session = Depends(get_db)
```

### 22.4 router에서 JWT를 직접 decode함

router가 JWT decode를 직접 하면 계층 책임이 흐려진다.

좋은 방향:

```text
router -> dependency -> service
```

### 22.5 service 로직을 dependency에 많이 넣음

dependency는 공통 준비 작업을 담당한다.

JWT 검증, 사용자 조회, 인증 실패 판단은 service에 두는 것이 현재 프로젝트 구조에 맞다.

## 23. 앞으로 확장할 수 있는 dependency 예시

인증 dependency 외에도 다음 dependency를 만들 수 있다.

```text
get_optional_current_user
-> 로그인하지 않은 사용자도 접근 가능한 API에서 사용
-> token이 있으면 User 반환, 없으면 None 반환

get_current_active_user
-> 탈퇴 또는 비활성 사용자를 더 엄격히 제외할 때 사용

require_post_author
-> 게시글 작성자인지 확인할 때 사용 가능
```

다만 권한 판단은 service에 두는 것이 현재 프로젝트 원칙에 더 맞다.

따라서 dependency를 너무 크게 만들기보다, 처음에는 `get_current_user`처럼 명확한 기능부터 작게 만든다.

## 24. 학습 체크리스트

아래 질문에 답할 수 있으면 dependency 개념을 이해한 것이다.

```text
FastAPI dependency는 무엇을 준비해주는가?
Depends()는 어떤 의미인가?
dependency는 router보다 먼저 실행되는가?
get_db()는 왜 dependency로 작성하는가?
인증 dependency는 왜 필요한가?
OAuth2PasswordBearer는 어떤 값을 꺼내는가?
get_current_user()와 get_current_user_from_token()의 역할 차이는 무엇인가?
router에서 JWT를 직접 decode하지 않는 이유는 무엇인가?
dependency에서 인증 실패가 나면 router 함수는 실행되는가?
__init__.py는 어떤 역할을 하는가?
```

## 25. 현재 프로젝트에서 기억할 기준

현재 프로젝트에서는 dependency를 다음 기준으로 사용한다.

```text
요청 처리에 필요한 공통 준비 작업을 dependency로 분리한다.
DB session은 get_db() dependency로 받는다.
현재 로그인 사용자는 get_current_user dependency로 받는다.
JWT 검증과 사용자 조회 판단은 service에 맡긴다.
repository는 DB 조회만 담당한다.
router는 HTTP 요청과 응답에 집중한다.
```

최종 인증 흐름:

```text
Authorization: Bearer <token>
-> OAuth2PasswordBearer
-> get_current_user
-> get_current_user_from_token
-> get_user_by_id
-> current_user
```

