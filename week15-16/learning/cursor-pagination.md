# Cursor Pagination 학습 문서

## 1. Cursor Pagination이란?

Cursor Pagination은 페이지 번호 대신 "마지막으로 본 데이터의 위치"를 기준으로 다음 데이터를 가져오는 방식입니다.

일반적인 페이지 방식은 다음처럼 요청합니다.

```text
GET /posts?page=1&size=10
GET /posts?page=2&size=10
```

Cursor Pagination은 다음처럼 요청합니다.

```text
GET /posts?size=10
GET /posts?size=10&cursor=마지막으로_본_게시글_정보
```

핵심 차이는 다음입니다.

```text
Page Pagination
  "2페이지 주세요"

Cursor Pagination
  "내가 마지막으로 본 글 다음부터 10개 주세요"
```

## 2. Page Pagination의 한계

현재 프로젝트의 게시글 목록은 page 기반으로 동작합니다.

```text
GET /posts?page=1&size=10
GET /posts?page=2&size=10
```

백엔드는 보통 다음처럼 계산합니다.

```text
page=1, size=10 -> 0번째부터 10개
page=2, size=10 -> 10번째부터 10개
page=3, size=10 -> 20번째부터 10개
```

이 방식은 이해하기 쉽고, 전체 페이지 수를 보여주기 좋습니다.

하지만 사용자가 1페이지를 보고 있는 동안 다른 사용자가 새 글을 여러 개 작성하면 문제가 생길 수 있습니다.

예를 들어 처음 목록이 이렇다고 하겠습니다.

```text
1페이지: A B C D E F G H I J
2페이지: K L M N O P Q R S T
```

사용자가 1페이지를 보고 있는 사이에 새 글 X, Y, Z가 앞에 추가되면 목록은 이렇게 바뀝니다.

```text
1페이지: X Y Z A B C D E F G
2페이지: H I J K L M N O P Q
3페이지: R S T ...
```

이때 사용자가 2페이지로 이동하면 다음 문제가 생깁니다.

```text
H I J를 다시 보게 될 수 있음
일부 글을 건너뛰게 될 수 있음
목록 순서가 사용자 입장에서 흔들려 보일 수 있음
```

즉 page 기반 방식은 데이터가 자주 추가되거나 삭제되는 화면에서 중복과 누락이 생길 수 있습니다.

## 3. Cursor Pagination의 핵심 개념

Cursor Pagination은 "몇 페이지인가"가 아니라 "어디까지 봤는가"를 기준으로 합니다.

예를 들어 게시글을 최신순으로 정렬한다고 하겠습니다.

```text
정렬 기준:
created_at DESC
id DESC
```

처음 요청은 cursor 없이 보냅니다.

```text
GET /posts?size=10
```

응답은 다음처럼 내려올 수 있습니다.

```json
{
  "items": [
    { "id": 30, "title": "최신 글" },
    { "id": 29, "title": "두 번째 글" },
    { "id": 28, "title": "세 번째 글" }
  ],
  "next_cursor": "2026-06-13T10:00:00|28",
  "has_next": true
}
```

여기서 `next_cursor`는 이런 뜻입니다.

```text
"이번 응답에서 마지막으로 본 게시글은 id 28번이고 created_at은 2026-06-13T10:00:00이다."
```

다음 목록을 가져올 때는 이 cursor를 다시 보냅니다.

```text
GET /posts?size=10&cursor=2026-06-13T10:00:00|28
```

백엔드는 이 cursor를 보고 다음 조건으로 조회합니다.

```text
"created_at이 더 오래됐거나,
 created_at이 같다면 id가 더 작은 게시글을 가져와라"
```

## 4. 왜 created_at과 id를 함께 쓰나요?

게시글은 보통 최신순으로 정렬합니다.

```sql
ORDER BY created_at DESC
```

그런데 같은 시각에 여러 게시글이 만들어질 수 있습니다.

```text
id=10, created_at=2026-06-13 10:00:00
id=11, created_at=2026-06-13 10:00:00
id=12, created_at=2026-06-13 10:00:00
```

이때 `created_at`만 cursor로 쓰면 어느 글까지 봤는지 정확히 나누기 어렵습니다.

그래서 보통 다음처럼 두 값을 함께 씁니다.

```text
created_at + id
```

정렬도 두 기준을 함께 사용합니다.

```sql
ORDER BY created_at DESC, id DESC
```

커서 조건도 두 기준을 함께 사용합니다.

```sql
WHERE
  created_at < :cursor_created_at
  OR (
    created_at = :cursor_created_at
    AND id < :cursor_id
  )
```

이렇게 하면 같은 시각에 생성된 글도 안정적으로 다음 목록을 가져올 수 있습니다.

## 5. SQL 예시

처음 목록 조회는 다음과 비슷합니다.

```sql
SELECT *
FROM posts
WHERE deleted_at IS NULL
ORDER BY created_at DESC, id DESC
LIMIT 11;
```

여기서 `size`가 10이라면 11개를 가져오는 이유가 있습니다.

```text
10개만 화면에 보여준다.
11번째가 있으면 다음 페이지가 있다는 뜻이다.
```

즉 다음처럼 판단합니다.

```text
조회 결과가 11개라면:
  has_next = true
  items는 앞의 10개만 사용

조회 결과가 10개 이하라면:
  has_next = false
```

다음 목록 조회는 cursor 조건이 추가됩니다.

```sql
SELECT *
FROM posts
WHERE deleted_at IS NULL
  AND (
    created_at < :cursor_created_at
    OR (
      created_at = :cursor_created_at
      AND id < :cursor_id
    )
  )
ORDER BY created_at DESC, id DESC
LIMIT 11;
```

## 6. API 설계 예시

현재 page 기반 API는 다음과 같습니다.

```text
GET /api/v1/posts?page=1&size=10&q=fastapi&tag=react
```

Cursor Pagination으로 바꾸면 다음과 같이 설계할 수 있습니다.

```text
GET /api/v1/posts?size=10&q=fastapi&tag=react
GET /api/v1/posts?size=10&q=fastapi&tag=react&cursor=2026-06-13T10:00:00|28
```

응답 구조 예시는 다음과 같습니다.

```json
{
  "items": [
    {
      "id": 28,
      "title": "FastAPI 페이징 정리",
      "created_at": "2026-06-13T10:00:00"
    }
  ],
  "size": 10,
  "next_cursor": "2026-06-13T10:00:00|28",
  "has_next": true
}
```

page 기반 응답과 비교하면 `page`, `total` 대신 `next_cursor`, `has_next`가 중요해집니다.

```text
Page Pagination:
  page
  size
  total

Cursor Pagination:
  size
  next_cursor
  has_next
```

## 7. 프론트엔드 흐름 예시

Page Pagination에서는 다음처럼 페이지 번호를 링크에 넣습니다.

```text
/?page=2
/?page=3
```

Cursor Pagination에서는 다음 cursor를 링크나 상태에 보관합니다.

```text
/?cursor=2026-06-13T10:00:00|28
```

또는 무한 스크롤에서는 React state에 보관할 수 있습니다.

```ts
const [items, setItems] = useState<PostListItem[]>([]);
const [nextCursor, setNextCursor] = useState<string | null>(null);

async function loadMore() {
    const response = await getPosts({
        size: 10,
        cursor: nextCursor,
    });

    setItems([...items, ...response.items]);
    setNextCursor(response.next_cursor);
}
```

이 방식은 "다음 페이지로 이동"보다 "더 보기" 버튼이나 무한 스크롤에 잘 맞습니다.

## 8. 현재 프로젝트에 적용한다면?

현재 프로젝트의 목록 응답 타입은 다음과 같습니다.

```ts
export type PostListResponse = {
    items: PostListItem[];
    page: number;
    size: number;
    total: number;
};
```

Cursor Pagination으로 바꾸면 예를 들어 다음처럼 바뀔 수 있습니다.

```ts
export type CursorPostListResponse = {
    items: PostListItem[];
    size: number;
    next_cursor: string | null;
    has_next: boolean;
};
```

API client의 요청 타입도 바뀝니다.

```ts
type GetPostsParams = {
    size?: number;
    q?: string;
    tag?: string;
    cursor?: string;
};
```

백엔드 repository에서는 `offset` 대신 cursor 조건을 사용합니다.

```python
query = query.order_by(Post.created_at.desc(), Post.id.desc())

if cursor_created_at and cursor_id:
    query = query.filter(
        or_(
            Post.created_at < cursor_created_at,
            and_(
                Post.created_at == cursor_created_at,
                Post.id < cursor_id,
            ),
        )
    )

posts = query.limit(size + 1).all()
```

## 9. 장점

Cursor Pagination의 장점은 다음과 같습니다.

```text
데이터가 새로 추가되어도 중복/누락이 줄어든다.
큰 offset을 건너뛰지 않아 성능에 유리하다.
무한 스크롤이나 더 보기 UI에 적합하다.
실시간성이 있는 목록에 안정적이다.
```

특히 게시글, 댓글, 알림, 채팅, 피드처럼 계속 새 데이터가 추가되는 화면에서 유용합니다.

## 10. 단점

Cursor Pagination에도 단점이 있습니다.

```text
1페이지, 2페이지, 3페이지처럼 임의 페이지 이동이 어렵다.
전체 페이지 수를 보여주기 어렵다.
cursor 인코딩/디코딩 처리가 필요하다.
정렬 기준이 명확하고 안정적이어야 한다.
```

예를 들어 "10페이지로 바로 이동" 같은 기능은 page 기반 방식이 더 쉽습니다.

## 11. 언제 어떤 방식을 쓰나요?

간단히 정리하면 다음과 같습니다.

```text
관리자 목록, 검색 결과, 게시판:
  page 기반 pagination도 충분히 적절하다.

피드, 댓글, 알림, 채팅, 무한 스크롤:
  cursor pagination이 더 적절하다.
```

현재 프로젝트는 학습용 게시판이므로 page 기반 pagination으로 충분합니다.

다만 게시글이 매우 자주 올라오는 서비스나 무한 스크롤을 구현한다면 cursor pagination을 고려하는 것이 좋습니다.

## 12. 학습 체크리스트

아래 질문에 답할 수 있으면 Cursor Pagination의 핵심을 이해한 것입니다.

```text
Page Pagination은 어떤 기준으로 다음 데이터를 가져오나요?
Cursor Pagination은 어떤 기준으로 다음 데이터를 가져오나요?
왜 created_at만 쓰지 않고 id도 함께 쓰나요?
왜 size보다 1개 더 조회하나요?
Cursor Pagination에서 total 페이지 수를 보여주기 어려운 이유는 무엇인가요?
현재 프로젝트에 적용하려면 어떤 타입과 API가 바뀌어야 하나요?
```

