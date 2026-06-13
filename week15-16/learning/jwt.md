# JWT 학습 문서

## 1. JWT란

JWT는 JSON Web Token의 줄임말이다.

서버와 클라이언트가 사용자 인증 정보를 주고받기 위해 사용하는 문자열 형식의 토큰이다.

게시판 프로젝트에서는 로그인에 성공했을 때 백엔드가 JWT access token을 발급하고, 이후 인증이 필요한 API 요청에서 클라이언트가 이 토큰을 함께 보낸다.

```text
로그인 성공
-> 백엔드가 JWT 발급
-> 프론트엔드가 토큰 저장
-> 인증이 필요한 요청마다 Authorization 헤더에 토큰 포함
-> 백엔드가 토큰 검증
-> 현재 사용자 확인
```

현재 프로젝트의 인증 방식은 문서 기준으로 다음 형태다.

```text
Authorization: Bearer <access_token>
```

여기서 중요한 점은 JWT가 "로그인 상태를 증명하는 문자열"이라는 것이다.

## 2. JWT를 쓰는 이유

전통적인 로그인 방식에서는 서버가 세션 저장소에 로그인 상태를 저장하고, 브라우저는 세션 ID만 들고 다닌다.

```text
Session 방식

브라우저: session_id
서버: session_id에 해당하는 사용자 정보를 세션 저장소에서 조회
```

JWT 방식에서는 토큰 안에 사용자를 식별할 수 있는 정보와 만료 시간이 들어 있고, 서버는 토큰의 서명을 검증해서 신뢰할 수 있는 토큰인지 판단한다.

```text
JWT 방식

브라우저: access_token
서버: access_token 자체를 검증해서 사용자 확인
```

JWT를 쓰는 이유:

- API 서버와 프론트엔드가 분리된 구조에서 사용하기 쉽다.
- 모바일 앱, 웹 프론트엔드, 외부 클라이언트가 같은 방식으로 인증할 수 있다.
- 토큰 자체에 만료 시간과 사용자 식별 정보를 담을 수 있다.
- 서버가 별도 세션 저장소를 반드시 들고 있지 않아도 기본 인증 흐름을 만들 수 있다.

하지만 JWT가 항상 더 좋은 것은 아니다.

JWT는 한 번 발급되면 만료 전까지 기본적으로 유효하다. 그래서 access token 만료 시간을 짧게 잡고, 필요하면 refresh token 같은 추가 구조를 둔다.

현재 프로젝트는 학습용 기본 게시판이므로 우선 access token 중심으로 구현하고 있다.

## 3. JWT의 구조

JWT는 보통 점(`.`)으로 구분된 세 부분으로 구성된다.

```text
header.payload.signature
```

예시:

```text
xxxxx.yyyyy.zzzzz
```

각 부분의 의미:

```text
header    = 토큰 타입과 서명 알고리즘 정보
payload   = 사용자 식별 정보, 만료 시간 같은 데이터
signature = header와 payload가 변조되지 않았음을 확인하는 서명
```

JWT는 암호화된 문자열이 아니라 Base64URL로 인코딩된 문자열이다.

따라서 payload에 비밀번호, 주민등록번호, secret, API key 같은 민감 정보를 넣으면 안 된다.

## 4. Header

Header에는 토큰 타입과 서명 알고리즘이 들어간다.

예시:

```json
{
  "alg": "HS256",
  "typ": "JWT"
}
```

의미:

```text
alg = 서명에 사용할 알고리즘
typ = 토큰 타입
```

현재 프로젝트에서는 설정값을 통해 JWT 알고리즘을 사용한다.

관련 파일:

```text
backend/app/core/security.py
```

핵심 코드 흐름:

```python
return jwt.encode(
    payload,
    settings.jwt_secret_key,
    algorithm=settings.jwt_algorithm,
)
```

여기서 `settings.jwt_algorithm`이 header의 `alg`와 연결된다.

## 5. Payload

Payload에는 토큰에 담을 데이터가 들어간다.

예시:

```json
{
  "sub": "1",
  "exp": 1781000000
}
```

현재 프로젝트에서 중요한 payload 값:

```text
sub = subject, 보통 사용자 ID를 넣는다.
exp = expiration time, 토큰 만료 시간이다.
```

현재 프로젝트의 access token 생성 함수는 사용자 ID를 `sub`에 넣는다.

관련 파일:

```text
backend/app/core/security.py
```

핵심 코드:

```python
payload: dict[str, Any] = {
    "sub": str(subject),
    "exp": expire,
}
```

`subject`로 `user.id`가 들어오면 payload에는 다음과 비슷한 의미의 값이 들어간다.

```json
{
  "sub": "1",
  "exp": "만료 시간"
}
```

주의할 점:

```text
payload는 누구나 디코딩해서 볼 수 있다.
```

그래서 payload에는 아래 값을 넣지 않는다.

```text
비밀번호
비밀번호 해시
JWT secret
DB URL
API key
개인정보성 민감 데이터
```

## 6. Signature

Signature는 토큰이 변조되지 않았는지 확인하기 위한 서명이다.

서버는 header와 payload를 secret key로 서명해서 signature를 만든다.

개념적으로는 다음과 비슷하다.

```text
signature = sign(
    base64url(header) + "." + base64url(payload),
    secret_key
)
```

클라이언트가 토큰을 보냈을 때 서버는 같은 secret key로 서명을 다시 검증한다.

검증 결과:

```text
서명이 맞다   -> 서버가 발급한 토큰으로 볼 수 있다.
서명이 틀리다 -> 중간에 변조되었거나 잘못된 토큰이다.
```

이 때문에 `JWT_SECRET_KEY` 같은 secret 값은 절대 외부에 노출되면 안 된다.

현재 프로젝트 규칙상 `.env` 내용이나 secret 값은 문서, 로그, 출력에 포함하지 않는다.

## 7. JWT는 암호화가 아니다

JWT에서 가장 많이 헷갈리는 점은 "토큰처럼 생겼으니 안전하게 숨겨져 있겠지"라고 생각하는 것이다.

JWT의 header와 payload는 쉽게 디코딩할 수 있다.

즉, JWT는 기본적으로:

```text
숨기는 기술이 아니라
변조 여부를 검증하는 기술
```

이다.

비유하면 다음과 같다.

```text
payload = 봉투 안의 내용이 아니라 투명 파일에 적힌 내용
signature = 그 내용이 서버가 작성한 원본인지 확인하는 도장
```

그래서 JWT payload에는 공개되어도 괜찮은 최소 정보만 넣어야 한다.

현재 프로젝트에서는 사용자 ID와 만료 시간만 넣는 방향이 적절하다.

## 8. 로그인 시 JWT 발급 흐름

현재 프로젝트에서 로그인 흐름은 다음과 같다.

```text
POST /api/v1/auth/login
-> router
-> auth service의 login()
-> user repository의 get_user_by_login_id()
-> 비밀번호 검증
-> create_access_token(user.id)
-> access token 응답
```

관련 파일:

```text
backend/app/services/auth.py
backend/app/core/security.py
backend/app/repositories/users.py
backend/app/schemas/auth.py
```

현재 service의 핵심 흐름:

```python
user = get_user_by_login_id(db, request.login_id)

if user is None:
    raise HTTPException(...)

is_valid_password = verify_password(
    request.password,
    user.password_hash,
)

if not is_valid_password:
    raise HTTPException(...)

access_token = create_access_token(user.id)
return access_token
```

이 흐름에서 repository는 DB 조회만 한다.

비밀번호 검증과 토큰 발급은 service가 담당한다.

이 구조는 프로젝트의 아키텍처 문서와 맞다.

```text
router -> service -> repository -> database
```

## 9. 인증이 필요한 요청 흐름

로그인 후 클라이언트는 인증이 필요한 API를 호출할 때 Authorization 헤더를 보낸다.

예시:

```http
GET /api/v1/auth/me HTTP/1.1
Authorization: Bearer fake_access_token
```

백엔드에서는 다음 순서로 처리한다.

```text
1. Authorization 헤더에서 Bearer token 추출
2. JWT decode 및 서명 검증
3. payload에서 sub 꺼내기
4. sub를 사용자 ID로 변환
5. DB에서 사용자 조회
6. 사용자가 존재하고 탈퇴 상태가 아니면 current_user로 반환
```

현재 프로젝트에서는 2번부터 6번까지를 이미 service 함수로 작성했다.

관련 파일:

```text
backend/app/services/auth.py
```

함수:

```python
def get_current_user_from_token(db: Session, token: str) -> User:
    ...
```

이 함수의 책임:

```text
토큰 검증
-> payload의 sub 확인
-> 사용자 조회
-> 현재 사용자 반환
```

## 10. Bearer token이란

`Authorization: Bearer <token>`에서 Bearer는 "이 토큰을 가진 사람이 인증된 사용자라고 본다"는 방식이다.

예:

```text
Authorization: Bearer fake_access_token
```

의미:

```text
이 요청은 fake_access_token을 가진 사용자로 처리해 주세요.
```

Bearer 방식에서는 토큰을 가진 사람이 권한을 가진다.

그래서 토큰이 유출되면 다른 사람이 그 토큰으로 요청할 수 있다.

따라서 실제 서비스에서는 다음을 지켜야 한다.

```text
HTTPS 사용
짧은 access token 만료 시간
토큰을 로그에 출력하지 않기
브라우저 저장 위치 신중히 선택
필요하면 refresh token과 재발급 구조 추가
```

## 11. FastAPI에서 OAuth2PasswordBearer의 역할

FastAPI에서는 `OAuth2PasswordBearer`를 사용해 Authorization 헤더에서 Bearer token을 쉽게 꺼낼 수 있다.

다음 작업에서 만들 dependency는 대략 이런 형태가 된다.

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

여기서 각 줄의 의미:

```text
OAuth2PasswordBearer(...)
-> Authorization: Bearer <token>에서 token 부분을 꺼내는 FastAPI 도구

tokenUrl="/api/v1/auth/login"
-> Swagger UI에서 로그인 토큰을 받을 URL을 알려주는 설정

db: Session = Depends(get_db)
-> 요청마다 DB 세션을 dependency로 받는다.

token: str = Depends(oauth2_scheme)
-> Authorization 헤더에서 Bearer token을 꺼낸다.

get_current_user_from_token(db, token)
-> 실제 JWT 검증과 사용자 조회는 service에 맡긴다.
```

이 dependency가 생기면 router에서는 다음처럼 사용할 수 있다.

```python
@router.get("/me")
def get_me(current_user: User = Depends(get_current_user)):
    return current_user
```

이때 router는 JWT decode 방법을 몰라도 된다.

router는 "현재 사용자가 필요하다"는 선언만 한다.

## 12. 현재 프로젝트 코드와 JWT 연결

현재 JWT 관련 흐름은 세 파일을 중심으로 이해하면 된다.

```text
backend/app/core/security.py
backend/app/services/auth.py
backend/app/repositories/users.py
```

### 12.1 `backend/app/core/security.py`

역할:

```text
비밀번호 해시
비밀번호 검증
JWT 생성
JWT decode
```

JWT 관련 함수:

```python
def create_access_token(subject: str | int) -> str:
    ...

def decode_access_token(token: str) -> dict[str, Any]:
    ...
```

`create_access_token()`은 로그인 성공 시 사용한다.

`decode_access_token()`은 인증이 필요한 요청에서 token을 검증할 때 사용한다.

### 12.2 `backend/app/services/auth.py`

역할:

```text
회원가입 로직
로그인 로직
현재 사용자 확인 로직
회원 탈퇴 로직
```

JWT 관련 함수:

```python
def login(db: Session, request: LoginRequest) -> str:
    ...

def get_current_user_from_token(db: Session, token: str) -> User:
    ...
```

`login()`은 JWT를 만든다.

`get_current_user_from_token()`은 JWT를 검증하고 현재 사용자를 찾는다.

### 12.3 `backend/app/repositories/users.py`

역할:

```text
users 테이블 조회와 저장
```

현재 사용자 조회에 쓰이는 함수:

```python
def get_user_by_id(
    db: Session,
    user_id: int,
    *,
    include_deleted: bool = False,
) -> User | None:
    ...
```

기본값이 `include_deleted=False`이므로 탈퇴 처리된 사용자는 조회되지 않는다.

따라서 탈퇴 처리된 사용자가 기존 token을 갖고 있어도 current user 조회에서 실패한다.

## 13. 토큰 만료 시간

JWT payload에는 `exp`가 들어간다.

`exp`는 expiration time, 즉 만료 시간이다.

현재 프로젝트에서는 설정값을 기준으로 만료 시간을 계산한다.

관련 파일:

```text
backend/app/core/security.py
```

핵심 흐름:

```python
expire = datetime.now(timezone.utc) + timedelta(
    minutes=settings.access_token_expire_minutes
)
```

토큰이 만료되면 `decode_access_token()`에서 오류가 발생하고, service는 인증 실패로 처리한다.

이때 클라이언트는 다시 로그인하거나, 나중에 refresh token 구조가 생기면 토큰 재발급을 시도할 수 있다.

현재 프로젝트에서는 우선 단순하게 access token만 사용한다.

## 14. 인증 실패가 나는 경우

현재 구조에서 인증 실패가 나는 대표적인 경우:

```text
Authorization 헤더가 없음
Bearer 형식이 아님
토큰 문자열이 잘못됨
서명이 맞지 않음
토큰이 만료됨
payload에 sub가 없음
sub가 숫자로 변환되지 않음
sub에 해당하는 사용자가 없음
사용자가 탈퇴 처리됨
```

이 경우 API 응답은 보통 401 Unauthorized가 된다.

현재 service에서는 인증 실패 시 다음 헤더를 함께 설정한다.

```python
headers={"WWW-Authenticate": "Bearer"}
```

이 헤더는 클라이언트에게 Bearer 인증이 필요하다는 의미를 전달한다.

## 15. JWT와 Soft Delete

현재 프로젝트는 회원 탈퇴를 hard delete가 아니라 soft delete로 처리한다.

회원 탈퇴 시:

```text
users.deleted_at에 탈퇴 시각 저장
email, username, display_name, password_hash를 재식별하기 어렵게 변경
```

문제 상황:

```text
사용자가 로그인해서 token을 발급받음
-> 회원 탈퇴
-> 기존 token이 아직 만료되지 않음
```

이때 기존 token으로 `/auth/me`를 호출하면 어떻게 될까?

현재 구조에서는 다음 흐름으로 막힌다.

```text
token decode 성공
-> sub에서 user id 추출
-> get_user_by_id(db, user_id)
-> deleted_at is None 조건으로 조회
-> 탈퇴 사용자는 조회되지 않음
-> 인증 실패
```

즉, soft delete 정책과 JWT 인증 흐름이 함께 동작한다.

## 16. JWT에서 조심할 보안 포인트

### 16.1 Payload에 민감 정보를 넣지 않기

JWT payload는 쉽게 읽을 수 있다.

넣으면 안 되는 값:

```text
비밀번호
비밀번호 해시
JWT secret
DB URL
API key
주민등록번호 같은 민감 개인정보
```

현재 프로젝트에서는 `sub`, `exp` 정도만 넣는 것이 적절하다.

### 16.2 Secret 값을 출력하지 않기

JWT secret은 토큰 서명에 쓰인다.

secret이 유출되면 공격자가 유효한 토큰을 직접 만들 수 있다.

따라서 다음을 지킨다.

```text
.env 내용을 출력하지 않는다.
secret 값을 문서에 쓰지 않는다.
secret 값을 로그에 남기지 않는다.
```

### 16.3 Access token 만료 시간을 너무 길게 잡지 않기

access token은 유출될 수 있다고 가정해야 한다.

만료 시간이 너무 길면 유출된 토큰이 오래 유효하다.

학습 프로젝트에서는 단순하게 시작하되, 실제 서비스에서는 refresh token 구조도 함께 고려한다.

### 16.4 HTTPS 사용

토큰은 요청 헤더에 실려 이동한다.

운영 환경에서는 HTTPS 없이 토큰을 주고받으면 안 된다.

### 16.5 로그에 Authorization 헤더를 남기지 않기

요청 전체를 그대로 로그에 남기면 Authorization 헤더가 같이 찍힐 수 있다.

운영 코드에서는 Authorization 헤더와 token 값을 마스킹해야 한다.

## 17. 직접 디코딩해보는 실습 개념

JWT는 다음 구조다.

```text
header.payload.signature
```

학습할 때는 실제 secret이나 실제 서비스 토큰을 사용하지 말고, 테스트용 토큰만 사용한다.

확인할 수 있는 것:

```text
header를 디코딩하면 alg, typ가 보인다.
payload를 디코딩하면 sub, exp가 보인다.
signature는 secret 없이 검증할 수 없다.
```

중요한 구분:

```text
디코딩 = 내용을 읽는 것
검증 = 서버가 발급한 유효한 토큰인지 확인하는 것
```

누구나 header와 payload는 디코딩할 수 있다.

하지만 secret이 없으면 올바른 signature를 만들 수 없다.

## 18. 다음 구현 단계

현재까지 이미 작성된 함수:

```text
create_access_token()
decode_access_token()
login()
get_current_user_from_token()
```

다음에 작성할 것은 FastAPI dependency다.

목표 파일:

```text
backend/app/dependencies/auth.py
```

목표 흐름:

```text
Authorization: Bearer <token>
-> OAuth2PasswordBearer가 token 추출
-> get_db()가 DB session 제공
-> get_current_user() dependency 실행
-> get_current_user_from_token(db, token) 호출
-> User 반환
```

그 다음 router에서는 다음 형태로 사용한다.

```python
@router.get("/me", response_model=UserResponse)
def get_me(current_user: User = Depends(get_current_user)):
    return current_user
```

## 19. 학습 체크리스트

JWT를 이해했는지 확인하려면 아래 질문에 답해본다.

```text
JWT는 몇 부분으로 나뉘는가?
header에는 무엇이 들어가는가?
payload에는 무엇을 넣으면 안 되는가?
signature는 어떤 역할을 하는가?
JWT는 암호화인가, 서명인가?
Authorization: Bearer <token>은 어떤 의미인가?
현재 프로젝트에서 user.id는 JWT의 어느 필드에 들어가는가?
토큰이 만료되면 어디에서 오류가 발생하는가?
탈퇴한 사용자가 기존 token으로 요청하면 왜 실패하는가?
router가 직접 JWT를 decode하지 않도록 하는 이유는 무엇인가?
```

이 질문에 답할 수 있으면 다음 작업인 인증 dependency 작성으로 넘어가도 된다.

## 20. 현재 프로젝트에서 기억할 기준

현재 프로젝트에서는 JWT를 다음 기준으로 사용한다.

```text
로그인 성공 시 access token 발급
payload에는 사용자 ID와 만료 시간만 담기
Authorization: Bearer <token>으로 인증
JWT 검증과 사용자 조회는 service에서 처리
router는 current_user dependency만 사용
secret 값은 절대 출력하지 않기
탈퇴 사용자는 current user 조회에서 제외
```

최종 흐름:

```text
POST /auth/login
-> login()
-> create_access_token(user.id)
-> access token 응답

GET /auth/me
-> get_current_user dependency
-> get_current_user_from_token()
-> decode_access_token()
-> get_user_by_id()
-> current_user 반환
```

