# Layered Architecture 학습 문서

## 1. Layered Architecture란?

Layered Architecture는 프로그램을 역할별 계층으로 나누는 설계 방식입니다.

한 파일이나 한 함수 안에서 요청 처리, 권한 확인, DB 조회, 데이터 저장, 응답 생성까지 모두 처리하면 처음에는 빠르게 만들 수 있습니다. 하지만 기능이 늘어나면 코드가 길어지고, 같은 로직이 여러 곳에 반복되며, 수정할 때 어디를 고쳐야 하는지 찾기 어려워집니다.

그래서 현재 프로젝트는 다음처럼 책임을 나눕니다.

```text
클라이언트
  |
  v
router
  |
  v
service
  |
  v
repository
  |
  v
model / database
```

핵심은 간단합니다.

```text
router      = HTTP 요청/응답 처리
service     = 비즈니스 로직, 권한 판단, 트랜잭션 흐름
repository  = DB 조회/저장
model       = DB 테이블 매핑
schema      = 요청/응답 데이터 구조
```

## 2. 왜 계층을 나누나요?

예를 들어 게시글 작성 API를 만든다고 생각해보겠습니다.

게시글 작성에는 여러 일이 필요합니다.

```text
1. HTTP 요청을 받는다.
2. 로그인한 사용자인지 확인한다.
3. 요청 body가 올바른지 확인한다.
4. 게시글을 DB에 저장한다.
5. 태그가 있다면 태그도 연결한다.
6. 실패하면 rollback 한다.
7. 성공하면 응답 데이터를 반환한다.
```

이 일을 router 하나에서 모두 처리하면 다음과 같은 문제가 생깁니다.

```python
@router.post("/posts")
def create_post(request, db, current_user):
    # 요청 처리
    # 권한 확인
    # 게시글 생성
    # 태그 조회
    # 태그 생성
    # post_tag 연결
    # commit
    # 응답 생성
    ...
```

처음에는 괜찮아 보이지만, 나중에 게시글 수정, 검색, 댓글, 좋아요 같은 기능이 추가되면 router가 점점 커집니다.

Layered Architecture에서는 이 일을 나눕니다.

```text
router:
  HTTP 요청을 받고 service를 호출한다.

service:
  게시글을 만들 수 있는지 판단하고 전체 흐름을 조율한다.

repository:
  실제 DB 조회/저장 작업만 수행한다.
```

이렇게 하면 코드의 위치가 명확해집니다.

```text
HTTP 경로가 궁금하다          -> router 확인
권한/정책이 궁금하다          -> service 확인
DB 조회 방식이 궁금하다       -> repository 확인
테이블 구조가 궁금하다        -> model 확인
요청/응답 필드가 궁금하다     -> schema 확인
```

## 3. 현재 프로젝트의 계층

현재 프로젝트는 FastAPI 기반 백엔드입니다. 문서 기준으로 다음 구조를 따릅니다.

```text
backend/app/
  routers/        HTTP API endpoint
  services/       business logic
  repositories/   database query
  models/         SQLAlchemy model
  schemas/        Pydantic schema
  dependencies/   FastAPI dependency
  db/             database session
```

각 계층의 역할은 다음과 같습니다.

| 계층 | 주 역할 | 예시 |
| --- | --- | --- |
| router | HTTP 요청/응답 처리 | `/posts`, `/comments`, `/auth/login` |
| service | 비즈니스 로직, 권한 확인, commit/rollback 흐름 | 게시글 작성 가능 여부, 댓글 작성 가능 여부 |
| repository | DB 조회/저장 | `select(Post)`, `db.add(post)`, `db.flush()` |
| model | DB 테이블 구조를 Python 클래스로 표현 | `Post`, `User`, `Comment`, `Tag` |
| schema | API 요청/응답 데이터 구조 | `PostCreate`, `PostResponse`, `CommentCreate` |
| dependency | 요청 처리 전에 공통으로 필요한 값 준비 | 현재 로그인 사용자, DB session |

## 4. 요청과 응답의 흐름

게시글 목록 조회를 예로 들면 흐름은 아래와 같습니다.

```text
브라우저 또는 프론트엔드
  |
  | GET /api/v1/posts
  v
posts router
  |
  | service 함수 호출
  v
posts service
  |
  | repository 함수 호출
  v
posts repository
  |
  | SQLAlchemy query 실행
  v
database
```

응답은 반대 방향으로 올라옵니다.

```text
database
  |
  v
repository
  |
  v
service
  |
  v
router
  |
  | JSON response
  v
클라이언트
```

중요한 점은 계층을 건너뛰지 않는 것입니다.

```text
좋은 흐름:
router -> service -> repository -> database

피해야 할 흐름:
router -> database
router -> repository에서 직접 복잡한 정책 처리
repository -> HTTPException 발생
```

## 5. 게시글 작성 예시

게시글 작성 API는 보통 다음 흐름을 가집니다.

```text
POST /api/v1/posts
```

요청 body 예시:

```json
{
  "title": "첫 게시글",
  "content": "Layered Architecture를 공부 중입니다.",
  "is_public": true,
  "tag_names": ["fastapi", "architecture"]
}
```

### 5.1 router의 역할

router는 HTTP 요청을 받습니다.

router는 다음 정도만 담당하는 것이 좋습니다.

```text
- 어떤 URL인지
- 어떤 HTTP method인지
- request body schema가 무엇인지
- 로그인 사용자가 필요한지
- 어떤 service 함수를 호출할지
- 어떤 response schema로 반환할지
```

예시:

```python
@router.post("", response_model=PostDetailResponse, status_code=201)
def create_post(
    request: PostCreate,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    return posts_service.create_new_post(
        db,
        request,
        current_user=current_user,
    )
```

이 코드에서 router는 직접 DB에 접근하지 않습니다.

```python
# router에서 이런 코드는 피합니다.
post = Post(title=request.title, author_id=current_user.id)
db.add(post)
db.commit()
```

왜냐하면 이것은 HTTP 처리보다 DB 저장과 비즈니스 흐름에 가까운 코드이기 때문입니다.

### 5.2 service의 역할

service는 게시글 작성이라는 업무 흐름을 담당합니다.

예시:

```python
def create_new_post(
    db: Session,
    request: PostCreate,
    *,
    current_user: User,
) -> Post:
    try:
        post = posts_repository.create_post(
            db,
            title=request.title,
            content=request.content,
            is_public=request.is_public,
            author_id=current_user.id,
        )

        tags = tags_repository.get_or_create_tags(
            db,
            names=request.tag_names,
        )

        posts_repository.replace_post_tags(
            db,
            post=post,
            tags=tags,
        )

        db.commit()
        db.refresh(post)
        return post

    except Exception:
        db.rollback()
        raise
```

service가 하는 일은 다음과 같습니다.

```text
- 현재 사용자가 게시글을 작성할 수 있는지 판단
- 게시글 저장 repository 호출
- 태그 저장 repository 호출
- 여러 DB 작업을 하나의 transaction으로 묶음
- 성공하면 commit
- 실패하면 rollback
```

여기서 중요한 점은 게시글 생성과 태그 연결이 하나의 작업이라는 점입니다.

게시글은 저장됐는데 태그 연결만 실패하면 데이터가 애매한 상태가 될 수 있습니다. 그래서 service에서 전체 흐름을 하나의 transaction으로 묶습니다.

```text
게시글 생성 성공
태그 생성 성공
태그 연결 실패
=> 전체 rollback
```

### 5.3 repository의 역할

repository는 DB 작업만 담당합니다.

예시:

```python
def create_post(
    db: Session,
    *,
    title: str,
    content: str,
    is_public: bool,
    author_id: int,
) -> Post:
    post = Post(
        title=title,
        content=content,
        is_public=is_public,
        author_id=author_id,
    )
    db.add(post)
    db.flush()
    return post
```

repository는 commit하지 않는 것이 좋습니다.

```python
# repository에서 피하는 코드
db.commit()
```

이유는 service가 여러 repository 작업을 하나의 transaction으로 묶어야 하기 때문입니다.

```text
service:
  create_post()
  get_or_create_tags()
  replace_post_tags()
  commit()
```

만약 `create_post()` 안에서 이미 commit을 해버리면, 뒤의 태그 연결이 실패했을 때 게시글 생성만 DB에 남을 수 있습니다.

## 6. 댓글 작성 예시

댓글 작성 API는 다음과 같은 흐름입니다.

```text
POST /api/v1/posts/{post_id}/comments
```

요청 body 예시:

```json
{
  "content": "좋은 글입니다."
}
```

### 6.1 router

router는 `post_id`, 요청 body, 현재 사용자, DB session을 받아 service로 넘깁니다.

```python
@router.post("/posts/{post_id}/comments", status_code=201)
def create_comment(
    post_id: int,
    request: CommentCreate,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    return comments_service.create_new_comment(
        db,
        post_id,
        request,
        current_user=current_user,
    )
```

router는 다음을 직접 판단하지 않습니다.

```text
- 해당 게시글이 존재하는지
- 비공개 게시글에 댓글을 달 수 있는지
- 댓글 작성 권한이 있는지
- DB에 어떻게 저장할지
```

이 판단은 service가 담당합니다.

### 6.2 service

댓글 작성 service는 먼저 댓글을 달 게시글이 유효한지 확인합니다.

```python
def create_new_comment(
    db: Session,
    post_id: int,
    request: CommentCreate,
    *,
    current_user: User,
) -> Comment:
    post = posts_service.get_post(
        db,
        post_id,
        current_user=current_user,
    )

    comment = comments_repository.create_comment(
        db,
        post_id=post.id,
        author_id=current_user.id,
        content=request.content,
    )

    db.commit()
    db.refresh(comment)
    return comment
```

여기서 `get_post()`를 호출하는 이유는 단순히 게시글 id만 확인하기 위해서가 아닙니다.

```text
get_post()에서 확인할 수 있는 것:
- 게시글이 존재하는가?
- 삭제된 게시글은 아닌가?
- 비공개 게시글이라면 현재 사용자가 볼 수 있는가?
```

즉 댓글 service는 댓글 저장 전에 "이 게시글에 댓글을 작성해도 되는가?"를 확인합니다.

### 6.3 repository

댓글 repository는 DB 저장만 담당합니다.

```python
def create_comment(
    db: Session,
    *,
    post_id: int,
    author_id: int,
    content: str,
) -> Comment:
    comment = Comment(
        post_id=post_id,
        author_id=author_id,
        content=content,
    )
    db.add(comment)
    db.flush()
    return comment
```

repository는 "비공개 게시글에 댓글을 달 수 있는가?" 같은 정책을 판단하지 않습니다.

그 판단은 service의 책임입니다.

## 7. 인증 dependency 예시

현재 프로젝트에는 `backend/app/dependencies/auth.py` 같은 파일이 있습니다.

dependency는 FastAPI에서 요청 처리 전에 공통으로 필요한 값을 준비하는 역할을 합니다.

예를 들어 로그인한 사용자가 필요한 API는 다음처럼 작성할 수 있습니다.

```python
@router.get("/me")
def get_me(
    current_user: User = Depends(get_current_user),
):
    return current_user
```

`get_current_user`는 대략 다음 흐름을 가집니다.

```text
Authorization header 확인
  |
  v
Bearer token 추출
  |
  v
JWT 검증
  |
  v
사용자 조회
  |
  v
current_user 반환
```

dependency는 router와 가까운 위치에서 동작하지만, 비즈니스 로직을 모두 담는 곳은 아닙니다.

dependency는 주로 이런 값을 준비합니다.

```text
- DB session
- 현재 로그인 사용자
- 로그인했을 수도 있고 안 했을 수도 있는 사용자
- 공통 header 또는 query 값
```

dependency 덕분에 router마다 같은 코드를 반복하지 않아도 됩니다.

```python
# dependency가 없으면 매 API마다 이런 작업을 반복해야 합니다.
authorization = request.headers.get("Authorization")
token = authorization.replace("Bearer ", "")
current_user = verify_token_and_get_user(token)
```

## 8. schema와 model의 차이

초심자가 많이 헷갈리는 부분입니다.

```text
schema = API에서 주고받는 데이터 모양
model  = DB 테이블의 구조
```

### 8.1 schema 예시

```python
class PostCreate(BaseModel):
    title: str
    content: str
    is_public: bool = True
    tag_names: list[str] = []
```

이 schema는 클라이언트가 게시글을 만들 때 보내는 데이터 구조입니다.

```json
{
  "title": "제목",
  "content": "내용",
  "is_public": true,
  "tag_names": ["fastapi"]
}
```

### 8.2 model 예시

```python
class Post(Base):
    __tablename__ = "posts"

    id = Column(Integer, primary_key=True)
    title = Column(String, nullable=False)
    content = Column(Text, nullable=False)
    author_id = Column(Integer, ForeignKey("users.id"), nullable=False)
    is_public = Column(Boolean, nullable=False)
```

이 model은 DB의 `posts` 테이블 구조를 Python 클래스로 표현합니다.

### 8.3 왜 schema와 model을 분리하나요?

API에서 받는 데이터와 DB에 저장되는 데이터가 항상 같지는 않기 때문입니다.

예를 들어 게시글 작성 요청에는 `author_id`가 없습니다.

```json
{
  "title": "제목",
  "content": "내용"
}
```

하지만 DB에는 `author_id`가 필요합니다.

```text
posts table
- id
- title
- content
- author_id
- created_at
- updated_at
```

`author_id`는 클라이언트가 보내는 값이 아니라 로그인한 사용자 정보에서 가져와야 합니다.

그래서 schema와 model을 분리합니다.

```text
schema:
  클라이언트가 보내거나 받는 데이터

model:
  DB에 실제로 저장되는 데이터
```

## 9. 각 계층에서 하면 좋은 일과 피해야 할 일

### 9.1 router

router에서 하면 좋은 일:

```text
- URL과 HTTP method 정의
- request schema 받기
- response schema 지정
- Depends로 DB session, current_user 받기
- service 함수 호출
```

router에서 피해야 할 일:

```text
- 직접 SQLAlchemy query 작성
- 직접 db.add(), db.commit() 호출
- 복잡한 권한 판단
- 여러 DB 작업을 직접 조율
```

나쁜 예:

```python
@router.post("/posts")
def create_post(request: PostCreate, db: Session = Depends(get_db)):
    post = Post(title=request.title, content=request.content)
    db.add(post)
    db.commit()
    return post
```

좋은 예:

```python
@router.post("/posts")
def create_post(
    request: PostCreate,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    return posts_service.create_new_post(
        db,
        request,
        current_user=current_user,
    )
```

### 9.2 service

service에서 하면 좋은 일:

```text
- 권한 확인
- 존재 여부 확인
- 여러 repository 함수 호출
- transaction 흐름 관리
- commit/rollback 처리
- 비즈니스 규칙 적용
```

service에서 피해야 할 일:

```text
- HTTP 요청/응답 세부 처리
- response status code 중심의 코드 작성
- DB query를 과도하게 직접 작성
```

service는 애플리케이션의 업무 규칙을 담는 계층입니다.

예:

```text
- 게시글 작성자는 자신의 게시글을 수정할 수 있다.
- 비공개 게시글은 작성자만 볼 수 있다.
- 삭제된 댓글은 일반 목록에 나오지 않는다.
```

### 9.3 repository

repository에서 하면 좋은 일:

```text
- select query 작성
- insert/update/delete 처리
- db.add()
- db.flush()
- DB 조회 조건 구성
```

repository에서 피해야 할 일:

```text
- commit/rollback
- HTTPException 발생
- 현재 사용자의 권한 판단
- request/response schema 조립
```

repository는 DB와 가까운 계층입니다.

예:

```python
def get_post_by_id(db: Session, post_id: int) -> Post | None:
    return db.scalar(
        select(Post).where(Post.id == post_id)
    )
```

이 함수는 게시글을 조회할 뿐입니다.

```text
이 게시글을 현재 사용자가 볼 수 있는가?
```

이 질문은 repository가 아니라 service에서 판단하는 것이 좋습니다.

### 9.4 model

model에서 하면 좋은 일:

```text
- 테이블 이름 정의
- 컬럼 정의
- ForeignKey 정의
- DB 제약조건 정의
```

model에서 피해야 할 일:

```text
- HTTP 요청 처리
- API 응답 형태 결정
- 복잡한 업무 흐름 작성
```

### 9.5 schema

schema에서 하면 좋은 일:

```text
- request body 구조 정의
- response body 구조 정의
- 필드 타입 정의
- 기본적인 데이터 검증
```

schema에서 피해야 할 일:

```text
- DB 조회
- 권한 판단
- commit/rollback
```

## 10. commit은 왜 service에서 하나요?

이 프로젝트에서는 repository가 `commit()`을 하지 않고, 보통 `flush()`까지만 합니다.

이유는 service가 전체 작업 단위를 알고 있기 때문입니다.

예를 들어 게시글 작성은 실제로 여러 DB 작업으로 구성될 수 있습니다.

```text
1. posts 테이블에 게시글 추가
2. tags 테이블에서 태그 조회
3. 없던 태그는 tags 테이블에 추가
4. post_tags 테이블에 게시글-태그 연결 추가
```

이 작업은 모두 성공하거나 모두 실패해야 합니다.

```text
모두 성공 -> commit
하나라도 실패 -> rollback
```

만약 repository마다 commit을 해버리면 이런 문제가 생길 수 있습니다.

```text
create_post()에서 commit 완료
get_or_create_tags()에서 commit 완료
replace_post_tags()에서 오류 발생

결과:
게시글은 DB에 남았지만 태그 연결은 실패
```

그래서 repository는 DB 작업을 수행하되, 최종 확정은 service가 담당하는 것이 좋습니다.

```text
repository:
  db.add()
  db.flush()

service:
  db.commit()
  db.rollback()
```

`flush()`는 DB에 SQL을 보내서 id 같은 값을 확보할 수 있게 하지만, transaction을 최종 확정하지는 않습니다.

```text
flush:
  아직 되돌릴 수 있음

commit:
  transaction 확정
```

## 11. MVC와 Layered Architecture의 차이

MVC는 보통 다음 세 가지로 설명합니다.

```text
Model       = 데이터와 핵심 로직
View        = 화면
Controller  = 요청 처리
```

웹 API 백엔드에서는 View가 HTML 화면이 아니라 JSON 응답이거나 프론트엔드 앱일 수 있습니다.

현재 프로젝트는 API 서버이므로 전통적인 MVC보다 다음처럼 더 세분화된 Layered Architecture를 사용합니다.

```text
Controller에 가까운 것:
  router

Model을 더 세분화한 것:
  schema
  service
  repository
  model

View에 가까운 것:
  frontend 또는 API response
```

MVC와 현재 구조를 비교하면 다음과 같습니다.

| MVC | 현재 프로젝트 |
| --- | --- |
| Controller | router |
| Model | service + repository + model + schema |
| View | frontend 또는 JSON response |

즉 Layered Architecture는 MVC와 완전히 반대되는 개념이라기보다, 백엔드 API에서 책임을 더 명확하게 나눈 구조라고 이해하면 됩니다.

## 12. 전체 예시: 게시글 상세 조회

게시글 상세 조회는 다음 API라고 가정합니다.

```text
GET /api/v1/posts/{post_id}
```

### 12.1 요청 흐름

```text
클라이언트
  |
  | GET /api/v1/posts/1
  v
router
  |
  | get_post(db, post_id, current_user)
  v
service
  |
  | get_post_by_id(db, post_id)
  v
repository
  |
  | SELECT * FROM posts WHERE id = 1
  v
database
```

### 12.2 service에서 판단할 수 있는 것

```text
게시글이 없다
  -> 404 Not Found

게시글은 있지만 삭제됐다
  -> 404 Not Found

비공개 게시글이고 작성자가 아니다
  -> 404 Not Found 또는 403 Forbidden

볼 수 있는 게시글이다
  -> 게시글 반환
```

이 판단은 단순 DB 조회가 아니라 정책입니다.

따라서 repository보다 service에 두는 것이 적절합니다.

### 12.3 repository는 단순 조회

```python
def get_post_by_id(
    db: Session,
    post_id: int,
) -> Post | None:
    return db.scalar(
        select(Post).where(Post.id == post_id)
    )
```

repository는 "id가 1인 게시글을 가져온다"까지만 담당합니다.

## 13. 전체 예시: 게시글 수정

게시글 수정은 작성자만 가능해야 합니다.

이 요구사항은 비즈니스 규칙입니다.

```text
작성자 본인만 게시글을 수정할 수 있다.
```

따라서 service에서 처리하는 것이 좋습니다.

```python
def update_existing_post(
    db: Session,
    post_id: int,
    request: PostUpdate,
    *,
    current_user: User,
) -> Post:
    post = posts_repository.get_post_by_id(db, post_id)

    if post is None:
        raise NotFoundError("게시글을 찾을 수 없습니다.")

    if post.author_id != current_user.id:
        raise PermissionDeniedError("게시글을 수정할 권한이 없습니다.")

    posts_repository.update_post(
        post,
        title=request.title,
        content=request.content,
        is_public=request.is_public,
    )

    db.commit()
    db.refresh(post)
    return post
```

repository는 실제 값 변경만 담당합니다.

```python
def update_post(
    post: Post,
    *,
    title: str | None = None,
    content: str | None = None,
    is_public: bool | None = None,
) -> Post:
    if title is not None:
        post.title = title
    if content is not None:
        post.content = content
    if is_public is not None:
        post.is_public = is_public
    return post
```

## 14. 자주 헷갈리는 질문

### Q1. service와 repository 둘 다 함수인데 차이가 뭔가요?

service는 "무엇을 해야 하는가"와 "해도 되는가"를 판단합니다.

repository는 "DB에서 어떻게 가져오거나 저장할 것인가"를 담당합니다.

```text
service:
  이 사용자가 이 게시글을 수정할 수 있는가?

repository:
  posts 테이블에서 id가 1인 게시글을 가져온다.
```

### Q2. router에서 repository를 바로 호출하면 안 되나요?

작은 조회 API에서는 가능해 보일 수 있습니다.

하지만 프로젝트 규칙상 router는 service를 거치는 것이 좋습니다.

이유는 다음과 같습니다.

```text
- 권한 판단 위치가 흩어지는 것을 막기 위해
- transaction 처리 위치를 일관되게 유지하기 위해
- 나중에 로직이 추가되어도 router를 크게 바꾸지 않기 위해
```

### Q3. repository에서 HTTPException을 발생시키면 안 되나요?

피하는 것이 좋습니다.

repository는 HTTP를 모르는 계층이어야 합니다.

repository는 다음처럼 값을 반환하는 것이 좋습니다.

```python
post = posts_repository.get_post_by_id(db, post_id)
```

그리고 service에서 판단합니다.

```python
if post is None:
    raise HTTPException(status_code=404, detail="Post not found")
```

더 엄격한 구조에서는 service도 HTTPException 대신 도메인 예외를 발생시키고, router 또는 예외 handler에서 HTTP 응답으로 바꾸기도 합니다. 현재 학습 프로젝트에서는 복잡도를 낮추기 위해 service에서 HTTPException을 사용하는 방식도 가능합니다.

### Q4. dependency는 계층 중 어디에 속하나요?

dependency는 router 주변에서 요청 처리에 필요한 공통 값을 준비하는 도구로 보면 됩니다.

대표 예시는 다음과 같습니다.

```text
- get_db
- get_current_user
- get_optional_current_user
```

dependency는 service나 repository를 완전히 대체하는 계층이 아닙니다.

### Q5. schema는 왜 필요한가요?

schema가 있으면 API 입출력 구조가 명확해집니다.

예를 들어 게시글 작성 요청에서 `title`은 문자열이어야 합니다.

```python
class PostCreate(BaseModel):
    title: str
    content: str
```

만약 클라이언트가 숫자를 보내거나 필수 값을 빼먹으면 FastAPI와 Pydantic이 검증을 도와줍니다.

### Q6. model만 있으면 schema는 없어도 되지 않나요?

분리하는 것이 좋습니다.

DB model을 그대로 API 응답으로 사용하면 다음 문제가 생길 수 있습니다.

```text
- DB 내부 필드가 외부에 노출될 수 있음
- password_hash 같은 민감한 값이 응답에 포함될 위험
- API 응답 구조를 DB 구조와 독립적으로 바꾸기 어려움
```

따라서 model과 schema를 구분하는 것이 안전합니다.

## 15. 현재 프로젝트에서 기억할 기준

현재 프로젝트에서 코드를 작성할 때는 다음 기준을 따르면 됩니다.

```text
1. router는 얇게 유지한다.
2. router는 service를 호출한다.
3. service는 권한과 업무 흐름을 담당한다.
4. service는 commit/rollback을 담당한다.
5. repository는 DB 조회/저장만 담당한다.
6. repository는 commit하지 않는다.
7. model은 DB 테이블 구조를 표현한다.
8. schema는 API 요청/응답 구조를 표현한다.
9. dependency는 요청마다 필요한 공통 값을 준비한다.
```

게시글 기능을 만들 때의 기준:

```text
posts router:
  게시글 API endpoint 정의

posts service:
  공개/비공개 여부 판단
  작성자 권한 확인
  게시글 생성/수정/삭제 흐름 조율

posts repository:
  posts 테이블 조회/저장
  post_tags 연결 처리

post model:
  posts 테이블 컬럼 정의

post schema:
  게시글 생성 요청, 수정 요청, 응답 구조 정의
```

댓글 기능을 만들 때의 기준:

```text
comments router:
  댓글 API endpoint 정의

comments service:
  댓글 작성 가능 여부 확인
  댓글 작성자 권한 확인
  댓글 생성/수정/삭제 흐름 조율

comments repository:
  comments 테이블 조회/저장

comment model:
  comments 테이블 컬럼 정의

comment schema:
  댓글 생성 요청, 수정 요청, 응답 구조 정의
```

## 16. 코드 작성 전 체크리스트

새 기능을 만들 때 다음 질문을 해보면 계층을 정하기 쉽습니다.

```text
이 코드는 URL, method, response_model과 관련 있나요?
  -> router

이 코드는 권한, 정책, 업무 흐름과 관련 있나요?
  -> service

이 코드는 select, insert, update, delete와 관련 있나요?
  -> repository

이 코드는 DB 테이블 컬럼과 관련 있나요?
  -> model

이 코드는 요청 body 또는 응답 body 구조와 관련 있나요?
  -> schema

이 코드는 여러 router에서 공통으로 필요한 요청 준비 작업인가요?
  -> dependency
```

## 17. 한 문장 요약

Layered Architecture의 핵심은 "코드를 역할별로 나누고, 각 계층이 자기 책임만 하게 만드는 것"입니다.

현재 프로젝트에서는 다음 문장을 기억하면 됩니다.

```text
router는 HTTP를 받고,
service는 판단하고,
repository는 DB를 다루고,
model은 테이블을 표현하고,
schema는 API 데이터 모양을 표현한다.
```
