# TypeScript 학습 자료

## 1. 이 문서의 목표

이 문서는 현재 프로젝트의 Next.js 프론트엔드를 작성하기 위해 TypeScript를 이해하는 것을 목표로 한다.

특히 다음 질문에 답할 수 있어야 한다.

- `.ts` 파일은 무엇인가?
- `.tsx` 파일과 `.ts` 파일은 무엇이 다른가?
- `frontend/types` 폴더는 왜 필요한가?
- 백엔드 API 응답을 TypeScript 타입으로 정의하면 어떤 장점이 있는가?
- `type`, `interface`, `export`, `import type`은 어떤 역할을 하는가?

현재 프로젝트의 프론트엔드 구조는 다음 기준을 따른다.

```text
frontend/
  app/
  components/
  lib/api/
  types/
```

이 중 TypeScript를 가장 먼저 체감하게 되는 곳은 `types/`와 `lib/api/`이다.

---

## 2. TypeScript란 무엇인가?

TypeScript는 JavaScript에 타입 시스템을 추가한 언어이다.

간단히 말하면 다음과 같다.

```text
JavaScript
= 실행되는 코드

TypeScript
= JavaScript + 타입 검사
```

예를 들어 JavaScript에서는 다음 코드가 작성 단계에서는 문제가 없어 보일 수 있다.

```js
function double(value) {
  return value * 2;
}

double("hello");
```

실행하면 `"hello" * 2`는 숫자 계산이 불가능하므로 의도하지 않은 결과가 나온다.

TypeScript에서는 매개변수 타입을 명시할 수 있다.

```ts
function double(value: number) {
  return value * 2;
}

double("hello");
```

이 경우 TypeScript는 실행 전에 다음과 같은 문제를 알려준다.

```text
string 타입은 number 타입 자리에 사용할 수 없다.
```

즉, TypeScript의 핵심 목적은 다음이다.

```text
코드를 실행하기 전에 데이터 사용 실수를 미리 발견하는 것
```

---

## 3. TypeScript는 브라우저에서 그대로 실행되는가?

일반적으로 브라우저는 TypeScript를 직접 실행하지 않는다.

TypeScript 코드는 빌드 과정에서 JavaScript로 변환된다.

```text
TypeScript 코드
-> 타입 검사
-> JavaScript로 변환
-> 브라우저 또는 Node.js에서 실행
```

Next.js 프로젝트에서는 이 과정을 Next.js와 TypeScript 도구가 처리한다.

따라서 개발자는 `.ts`, `.tsx` 파일을 작성하지만, 실제 실행 결과는 JavaScript이다.

중요한 점은 다음이다.

```text
TypeScript 타입은 개발 중에 도움을 주는 장치이다.
런타임에서 자동으로 API 응답을 검증해주는 장치는 아니다.
```

예를 들어 다음 타입을 정의했다고 하자.

```ts
type Tag = {
  id: number;
  name: string;
};
```

이 타입은 "우리는 태그 데이터가 이런 모양이라고 기대한다"는 약속이다.

하지만 실제 백엔드가 잘못된 데이터를 보내는 경우까지 TypeScript가 자동으로 막아주지는 않는다.

```json
{
  "id": "wrong",
  "name": 123
}
```

이런 런타임 검증까지 하려면 나중에 `zod` 같은 검증 라이브러리나 직접 검증 로직을 추가해야 한다.

현재 단계에서는 먼저 타입으로 데이터 구조를 명확히 하는 것이 목표이다.

---

## 4. `.ts`와 `.tsx`의 차이

TypeScript 파일 확장자는 보통 두 가지를 사용한다.

```text
.ts
= 일반 TypeScript 파일

.tsx
= JSX를 포함하는 TypeScript 파일
```

JSX는 React 컴포넌트에서 사용하는 HTML처럼 생긴 문법이다.

예를 들어 다음 파일은 JSX를 포함하므로 `.tsx`가 적절하다.

```tsx
export default function HomePage() {
  return <main>게시글 목록</main>;
}
```

반면 다음 파일은 JSX가 없으므로 `.ts`가 적절하다.

```ts
export type Tag = {
  id: number;
  name: string;
};
```

현재 프로젝트에 적용하면 다음과 같다.

```text
frontend/app/page.tsx
= 화면을 그리는 React 컴포넌트
= JSX가 있으므로 .tsx

frontend/components/posts/PostList.tsx
= 게시글 목록 UI 컴포넌트
= JSX가 있으므로 .tsx

frontend/types/posts.ts
= 게시글 데이터 타입 정의
= JSX가 없으므로 .ts

frontend/lib/api/posts.ts
= 게시글 API 호출 함수
= JSX가 없으므로 .ts
```

정리하면 다음 기준으로 판단한다.

```text
return <div>...</div> 같은 JSX가 있다
-> .tsx

타입, 함수, 상수만 있다
-> .ts
```

---

## 5. 현재 프로젝트에서 `types/` 폴더의 역할

현재 프론트엔드는 FastAPI 백엔드 API를 호출한다.

백엔드는 JSON 데이터를 반환한다.

예를 들어 태그 목록 API는 대략 다음과 같은 데이터를 반환할 수 있다.

```json
[
  {
    "id": 1,
    "name": "fastapi"
  },
  {
    "id": 2,
    "name": "nextjs"
  }
]
```

프론트엔드에서는 이 데이터의 모양을 TypeScript 타입으로 정의한다.

```ts
export type Tag = {
  id: number;
  name: string;
};
```

이 타입을 `frontend/types/tags.ts`에 두면 다른 파일에서 재사용할 수 있다.

```ts
import type { Tag } from "@/types/tags";

function printTag(tag: Tag) {
  console.log(tag.name);
}
```

`types/` 폴더의 역할은 다음과 같다.

```text
백엔드 API와 프론트엔드 코드 사이에서 데이터 구조를 명확히 정의하는 곳
```

즉, `types/`는 화면을 그리는 곳도 아니고 API를 직접 호출하는 곳도 아니다.

```text
app/
= 페이지와 라우팅

components/
= 재사용 UI

lib/api/
= 백엔드 API 호출 함수

types/
= API 요청/응답 데이터 타입
```

---

## 6. 백엔드 schema와 프론트엔드 type의 관계

백엔드에는 `schema`가 있다.

예를 들어 FastAPI/Pydantic에서는 요청과 응답 데이터 구조를 schema로 정의한다.

```text
backend/app/schemas/posts.py
```

프론트엔드에는 `type`이 있다.

```text
frontend/types/posts.ts
```

둘은 같은 역할을 다른 언어에서 수행한다고 이해하면 쉽다.

```text
백엔드 schema
= Python/FastAPI 쪽 데이터 구조 약속

프론트엔드 type
= TypeScript/React 쪽 데이터 구조 약속
```

요청 흐름으로 보면 다음과 같다.

```text
FastAPI schema
-> JSON 응답
-> fetch
-> TypeScript type
-> React 화면
```

예를 들어 백엔드 게시글 응답이 다음과 같다고 하자.

```json
{
  "id": 1,
  "title": "첫 게시글",
  "content": "내용입니다.",
  "is_public": true,
  "author": {
    "id": 1,
    "username": "hyungmin"
  },
  "tags": [
    {
      "id": 1,
      "name": "fastapi"
    }
  ],
  "created_at": "2026-06-12T10:00:00"
}
```

프론트엔드에서는 다음처럼 타입을 정의할 수 있다.

```ts
export type PostAuthor = {
  id: number;
  username: string;
};

export type PostTag = {
  id: number;
  name: string;
};

export type PostDetail = {
  id: number;
  title: string;
  content: string;
  is_public: boolean;
  author: PostAuthor;
  tags: PostTag[];
  created_at: string;
};
```

여기서 `created_at`은 날짜처럼 보이지만 API 응답에서는 보통 문자열이다.

따라서 TypeScript 타입은 다음처럼 잡는 것이 자연스럽다.

```ts
created_at: string;
```

화면에서 날짜 형식으로 보여주고 싶다면 나중에 변환한다.

```ts
const date = new Date(post.created_at);
```

---

## 7. 기본 타입

TypeScript에서 자주 쓰는 기본 타입은 다음과 같다.

```ts
const title: string = "게시글 제목";
const viewCount: number = 10;
const isPublic: boolean = true;
const deletedAt: null = null;
```

배열은 다음처럼 표현한다.

```ts
const tags: string[] = ["fastapi", "nextjs"];
const ids: number[] = [1, 2, 3];
```

객체는 다음처럼 표현한다.

```ts
const user: {
  id: number;
  username: string;
} = {
  id: 1,
  username: "hyungmin",
};
```

하지만 객체 타입을 매번 직접 쓰면 길어진다.

그래서 보통 `type`으로 이름을 붙인다.

```ts
type User = {
  id: number;
  username: string;
};

const user: User = {
  id: 1,
  username: "hyungmin",
};
```

---

## 8. `type`이란 무엇인가?

`type`은 데이터의 모양에 이름을 붙이는 문법이다.

```ts
type LoginRequest = {
  login_id: string;
  password: string;
};
```

이 타입은 다음 의미를 가진다.

```text
LoginRequest는 login_id와 password를 가진 객체이다.
둘 다 string이어야 한다.
```

함수에 적용하면 더 명확하다.

```ts
function login(payload: LoginRequest) {
  console.log(payload.login_id);
}
```

이제 다음 코드는 올바르다.

```ts
login({
  login_id: "testuser",
  password: "password123",
});
```

하지만 다음 코드는 TypeScript가 오류를 알려준다.

```ts
login({
  email: "test@example.com",
  password: "password123",
});
```

이유는 `LoginRequest` 타입에 `email`이 없고, 필요한 `login_id`가 없기 때문이다.

---

## 9. `interface`란 무엇인가?

`interface`도 객체의 모양을 정의할 때 사용한다.

```ts
interface User {
  id: number;
  username: string;
}
```

`type`으로도 거의 같은 표현이 가능하다.

```ts
type User = {
  id: number;
  username: string;
};
```

초보 단계에서는 다음 기준으로 사용하면 충분하다.

```text
프로젝트에서 단순 데이터 구조를 정의한다
-> type을 사용

클래스 구현 계약이나 확장 가능한 객체 구조를 다룬다
-> interface를 고려
```

현재 프로젝트의 `types/` 폴더에서는 API 요청/응답 데이터 구조를 정의하는 일이 많다.

따라서 `type`을 우선 사용해도 충분하다.

---

## 10. 선택 속성: `?`

속성이 항상 존재하지 않을 수 있다면 `?`를 붙인다.

```ts
type User = {
  id: number;
  username: string;
  bio?: string;
};
```

이 타입은 다음 의미이다.

```text
id는 반드시 있어야 한다.
username은 반드시 있어야 한다.
bio는 있을 수도 있고 없을 수도 있다.
```

따라서 다음 둘 다 가능하다.

```ts
const user1: User = {
  id: 1,
  username: "hyungmin",
};

const user2: User = {
  id: 2,
  username: "min",
  bio: "안녕하세요.",
};
```

게시글 수정 요청에서도 선택 속성이 자주 사용된다.

```ts
type UpdatePostRequest = {
  title?: string;
  content?: string;
  is_public?: boolean;
  tag_names?: string[];
};
```

이 타입은 "수정하고 싶은 값만 보내도 된다"는 의미이다.

---

## 11. 유니온 타입: `|`

여러 값 중 하나를 허용할 때 `|`를 사용한다.

```ts
type Visibility = "public" | "private";
```

이 타입은 다음 의미이다.

```text
Visibility는 "public" 또는 "private"만 가능하다.
```

사용 예시는 다음과 같다.

```ts
const visibility: Visibility = "public";
```

다음 값은 허용되지 않는다.

```ts
const visibility: Visibility = "hidden";
```

API 상태를 표현할 때도 사용할 수 있다.

```ts
type ApiStatus = "idle" | "loading" | "success" | "error";
```

---

## 12. 배열 타입

배열 타입은 두 가지 방식으로 표현할 수 있다.

```ts
type TagList = Tag[];
```

또는 다음처럼 쓸 수도 있다.

```ts
type TagList = Array<Tag>;
```

현재 프로젝트에서는 `Tag[]`처럼 짧은 표현을 사용하면 충분하다.

```ts
type PostListItem = {
  id: number;
  title: string;
  tags: Tag[];
};
```

---

## 13. `export`와 `import type`

다른 파일에서 타입을 사용하려면 `export`해야 한다.

```ts
// frontend/types/tags.ts
export type Tag = {
  id: number;
  name: string;
};
```

다른 파일에서는 `import type`으로 가져온다.

```ts
// frontend/lib/api/tags.ts
import type { Tag } from "@/types/tags";
```

`import type`은 "이 import는 실행 코드가 아니라 타입 정보만 가져온다"는 뜻이다.

타입만 가져올 때는 `import type`을 사용하는 습관이 좋다.

```text
실제 함수나 값 가져오기
-> import

타입만 가져오기
-> import type
```

예시:

```ts
import { apiClient } from "@/lib/api/client";
import type { Tag } from "@/types/tags";
```

여기서 `apiClient`는 실제 실행되는 함수이다.

반면 `Tag`는 개발 중 타입 검사에만 사용된다.

---

## 14. 제네릭: `<T>`

제네릭은 "타입을 나중에 전달받는 문법"이다.

현재 프로젝트의 API client에서 특히 유용하다.

예를 들어 공통 API 함수가 있다고 하자.

```ts
export async function apiClient<T>(
  path: string,
  options?: RequestInit,
): Promise<T> {
  const response = await fetch(path, options);
  return response.json() as Promise<T>;
}
```

여기서 `<T>`는 호출하는 쪽에서 응답 타입을 지정하겠다는 뜻이다.

태그 목록 API는 `Tag[]`를 반환한다고 지정할 수 있다.

```ts
import { apiClient } from "@/lib/api/client";
import type { Tag } from "@/types/tags";

export function getTags() {
  return apiClient<Tag[]>("/api/v1/tags");
}
```

로그인 API는 `LoginResponse`를 반환한다고 지정할 수 있다.

```ts
import { apiClient } from "@/lib/api/client";
import type { LoginResponse } from "@/types/auth";

export function login() {
  return apiClient<LoginResponse>("/api/v1/auth/login");
}
```

정리하면 다음과 같다.

```text
apiClient<T>
= 이 API가 반환할 데이터 타입을 T 자리에 넣어서 사용한다.
```

---

## 15. API 요청 타입과 응답 타입을 나누는 이유

요청 데이터와 응답 데이터는 모양이 다를 수 있다.

회원가입 요청은 다음과 같을 수 있다.

```json
{
  "email": "test@example.com",
  "username": "testuser",
  "password": "password123"
}
```

하지만 회원가입 응답은 비밀번호를 포함하면 안 된다.

```json
{
  "id": 1,
  "email": "test@example.com",
  "username": "testuser"
}
```

따라서 타입도 나누는 것이 좋다.

```ts
export type SignupRequest = {
  email: string;
  username: string;
  password: string;
};

export type UserResponse = {
  id: number;
  email: string;
  username: string;
};
```

이렇게 나누면 다음 장점이 있다.

```text
요청에 필요한 값과 응답으로 받는 값을 구분할 수 있다.
비밀번호 같은 민감한 값이 응답 타입에 섞이는 실수를 줄일 수 있다.
API 함수의 역할이 명확해진다.
```

---

## 16. 현재 프로젝트 타입 예시

### 인증 타입

`frontend/types/auth.ts`에는 인증 관련 타입을 둘 수 있다.

```ts
export type SignupRequest = {
  email: string;
  username: string;
  password: string;
};

export type LoginRequest = {
  login_id: string;
  password: string;
};

export type TokenResponse = {
  access_token: string;
  token_type: string;
};

export type CurrentUser = {
  id: number;
  email: string;
  username: string;
};
```

여기서 `login_id`는 백엔드 API 명세에 맞춘 이름이다.

프론트엔드에서 임의로 `loginId`로 바꾸면 API 요청 body가 달라질 수 있으므로 주의해야 한다.

```ts
// 백엔드가 기대하는 형태
{
  login_id: "testuser",
  password: "password123",
}
```

프론트엔드 내부 변수명은 `loginId`로 쓸 수 있지만, API 요청 body를 만들 때는 백엔드 필드명에 맞춰야 한다.

```ts
const loginId = "testuser";

const body = {
  login_id: loginId,
  password: "password123",
};
```

### 태그 타입

`frontend/types/tags.ts`에는 태그 타입을 둘 수 있다.

```ts
export type Tag = {
  id: number;
  name: string;
};
```

### 댓글 타입

`frontend/types/comments.ts`에는 댓글 타입을 둘 수 있다.

```ts
export type CommentAuthor = {
  id: number;
  username: string;
};

export type Comment = {
  id: number;
  content: string;
  author: CommentAuthor;
  created_at: string;
};

export type CreateCommentRequest = {
  content: string;
};
```

### 게시글 타입

`frontend/types/posts.ts`에는 게시글 타입을 둘 수 있다.

```ts
import type { Comment } from "@/types/comments";
import type { Tag } from "@/types/tags";

export type PostAuthor = {
  id: number;
  username: string;
};

export type PostListItem = {
  id: number;
  title: string;
  is_public: boolean;
  author: PostAuthor;
  tags: Tag[];
  created_at: string;
};

export type PostDetail = PostListItem & {
  content: string;
  comments: Comment[];
};

export type CreatePostRequest = {
  title: string;
  content: string;
  is_public: boolean;
  tag_names: string[];
};

export type UpdatePostRequest = {
  title?: string;
  content?: string;
  is_public?: boolean;
  tag_names?: string[];
};
```

여기서 `PostDetail = PostListItem & { ... }`는 교차 타입이다.

의미는 다음과 같다.

```text
PostDetail은 PostListItem의 필드를 모두 가진다.
추가로 content와 comments도 가진다.
```

---

## 17. `lib/api`에서 타입을 사용하는 방식

`types/`는 데이터 구조만 정의한다.

실제 API 호출은 `lib/api/`에서 한다.

예를 들어 `frontend/lib/api/posts.ts`는 다음 역할을 한다.

```text
게시글 목록 조회
게시글 상세 조회
게시글 생성
게시글 수정
게시글 삭제
```

타입을 적용하면 다음처럼 작성할 수 있다.

```ts
import { apiClient } from "@/lib/api/client";
import type {
  CreatePostRequest,
  PostDetail,
  PostListItem,
  UpdatePostRequest,
} from "@/types/posts";

export function getPosts() {
  return apiClient<PostListItem[]>("/api/v1/posts");
}

export function getPost(postId: number) {
  return apiClient<PostDetail>(`/api/v1/posts/${postId}`);
}

export function createPost(payload: CreatePostRequest, token: string) {
  return apiClient<PostDetail>("/api/v1/posts", {
    method: "POST",
    headers: {
      Authorization: `Bearer ${token}`,
      "Content-Type": "application/json",
    },
    body: JSON.stringify(payload),
  });
}

export function updatePost(
  postId: number,
  payload: UpdatePostRequest,
  token: string,
) {
  return apiClient<PostDetail>(`/api/v1/posts/${postId}`, {
    method: "PATCH",
    headers: {
      Authorization: `Bearer ${token}`,
      "Content-Type": "application/json",
    },
    body: JSON.stringify(payload),
  });
}
```

이렇게 하면 각 함수가 어떤 데이터를 받고 어떤 데이터를 반환하는지 명확해진다.

---

## 18. React 컴포넌트에서 타입을 사용하는 방식

컴포넌트는 `.tsx` 파일에 작성한다.

예를 들어 게시글 목록 아이템 컴포넌트는 다음처럼 작성할 수 있다.

```tsx
import type { PostListItem } from "@/types/posts";

type PostCardProps = {
  post: PostListItem;
};

export function PostCard({ post }: PostCardProps) {
  return (
    <article>
      <h2>{post.title}</h2>
      <p>{post.author.username}</p>
    </article>
  );
}
```

여기서 중요한 점은 `props`에도 타입을 붙인다는 것이다.

```ts
type PostCardProps = {
  post: PostListItem;
};
```

이렇게 하면 `PostCard`를 사용하는 쪽에서 잘못된 데이터를 넘겼을 때 TypeScript가 알려준다.

```tsx
<PostCard post={post} />
```

만약 `post`에 `title`이 없다면 컴파일 단계에서 문제를 발견할 수 있다.

---

## 19. `any`를 조심해야 하는 이유

TypeScript에는 `any`라는 타입이 있다.

```ts
let value: any = "hello";
value = 123;
value = true;
```

`any`는 어떤 값이든 허용한다.

편해 보이지만 TypeScript의 장점을 대부분 잃게 된다.

```ts
function printTitle(post: any) {
  console.log(post.titel);
}
```

위 코드에서 `titel`은 `title`의 오타이지만, `post`가 `any`이면 TypeScript가 잡아주기 어렵다.

타입을 제대로 지정하면 오타를 발견할 수 있다.

```ts
type Post = {
  title: string;
};

function printTitle(post: Post) {
  console.log(post.titel);
}
```

따라서 현재 프로젝트에서는 다음 원칙을 권장한다.

```text
처음부터 any를 쓰지 않는다.
모양을 알 수 있는 데이터는 type으로 정의한다.
API 응답은 types/에 타입을 만든 뒤 사용한다.
```

---

## 20. `unknown`은 언제 쓰는가?

`unknown`은 "아직 타입을 모르는 값"을 의미한다.

```ts
let value: unknown = "hello";
```

`unknown`은 바로 사용할 수 없다.

```ts
console.log(value.toUpperCase());
```

TypeScript는 이 코드를 막는다.

먼저 타입을 확인해야 한다.

```ts
if (typeof value === "string") {
  console.log(value.toUpperCase());
}
```

API 에러 응답처럼 모양이 불확실한 데이터에는 `unknown`이 `any`보다 안전하다.

```text
any
= 아무렇게나 써도 허용

unknown
= 먼저 확인한 뒤 써야 함
```

초보 단계에서는 `unknown`을 자주 쓸 필요는 없지만, `any`보다 안전한 선택지라는 점을 기억하면 된다.

---

## 21. 타입 추론

TypeScript는 모든 곳에 타입을 직접 쓰지 않아도 된다.

다음 코드는 TypeScript가 자동으로 타입을 추론한다.

```ts
const title = "게시글 제목";
const count = 10;
const isPublic = true;
```

TypeScript는 각각 다음처럼 이해한다.

```text
title -> string
count -> number
isPublic -> boolean
```

따라서 단순 변수에는 타입을 매번 쓰지 않아도 된다.

하지만 함수 매개변수, API 응답, 컴포넌트 props에는 타입을 명확히 쓰는 것이 좋다.

```ts
function formatPostTitle(title: string) {
  return `[게시글] ${title}`;
}
```

---

## 22. 타입은 작게 시작하고 필요할 때 확장한다

처음부터 너무 복잡한 타입을 만들 필요는 없다.

예를 들어 게시글 목록 화면에서 필요한 값이 `id`, `title`, `author`, `created_at`뿐이라면 다음처럼 시작해도 된다.

```ts
export type PostListItem = {
  id: number;
  title: string;
  author: {
    id: number;
    username: string;
  };
  created_at: string;
};
```

나중에 태그가 필요해지면 추가한다.

```ts
export type PostListItem = {
  id: number;
  title: string;
  author: {
    id: number;
    username: string;
  };
  tags: Tag[];
  created_at: string;
};
```

다만 백엔드 API가 이미 명확히 정해져 있다면, API 응답 형태에 맞춰 처음부터 정의하는 것이 더 낫다.

현재 프로젝트는 백엔드 API가 먼저 구현되어 있으므로 프론트엔드 타입은 백엔드 응답에 맞추는 것이 좋다.

---

## 23. TypeScript가 막아주는 실수 예시

### 숫자와 문자열 혼동

```ts
function getPost(postId: number) {
  return `/api/v1/posts/${postId}`;
}

getPost("1");
```

문제:

```text
"1"은 string이고, postId는 number를 기대한다.
```

### 필드명 오타

```ts
type Post = {
  title: string;
};

const post: Post = {
  title: "첫 게시글",
};

console.log(post.titel);
```

문제:

```text
titel이라는 필드는 없다.
title이 맞다.
```

### 필수 필드 누락

```ts
type CreatePostRequest = {
  title: string;
  content: string;
  is_public: boolean;
};

const payload: CreatePostRequest = {
  title: "제목",
};
```

문제:

```text
content와 is_public이 빠져 있다.
```

### 잘못된 배열 요소

```ts
type Tag = {
  id: number;
  name: string;
};

const tags: Tag[] = [
  { id: 1, name: "fastapi" },
  { id: "2", name: "nextjs" },
];
```

문제:

```text
id는 number여야 하는데 string이 들어갔다.
```

---

## 24. 현재 프로젝트에서 추천하는 작성 순서

프론트엔드 API 연결 코드는 다음 순서로 작성하는 것이 좋다.

```text
1. types/에 요청/응답 타입 작성
2. lib/api/client.ts에 공통 fetch 함수 작성
3. lib/api/auth.ts에 인증 API 함수 작성
4. lib/api/posts.ts에 게시글 API 함수 작성
5. 컴포넌트에서 API 함수와 타입 사용
```

예를 들어 게시글 목록 화면을 만든다면 다음 흐름이다.

```text
frontend/types/posts.ts
-> PostListItem 타입 정의

frontend/lib/api/posts.ts
-> getPosts 함수 작성

frontend/app/page.tsx 또는 frontend/app/posts/page.tsx
-> getPosts 호출
-> PostListItem[] 타입으로 화면 렌더링
```

---

## 25. 실습 예제: 태그 타입부터 API 함수까지

### 1단계: 타입 정의

```ts
// frontend/types/tags.ts
export type Tag = {
  id: number;
  name: string;
};
```

### 2단계: API 함수 작성

```ts
// frontend/lib/api/tags.ts
import { apiClient } from "@/lib/api/client";
import type { Tag } from "@/types/tags";

export function getTags() {
  return apiClient<Tag[]>("/api/v1/tags");
}
```

### 3단계: 화면에서 사용

```tsx
// 예시 컴포넌트
import type { Tag } from "@/types/tags";

type TagListProps = {
  tags: Tag[];
};

export function TagList({ tags }: TagListProps) {
  return (
    <ul>
      {tags.map((tag) => (
        <li key={tag.id}>{tag.name}</li>
      ))}
    </ul>
  );
}
```

이 흐름에서 각 파일의 책임은 명확하다.

```text
types/tags.ts
= 태그 데이터의 모양

lib/api/tags.ts
= 태그 API 호출 방법

TagList.tsx
= 태그 데이터를 화면에 보여주는 방법
```

---

## 26. 실습 예제: 로그인 요청과 응답

### 타입 정의

```ts
// frontend/types/auth.ts
export type LoginRequest = {
  login_id: string;
  password: string;
};

export type TokenResponse = {
  access_token: string;
  token_type: string;
};
```

### API 함수

```ts
// frontend/lib/api/auth.ts
import { apiClient } from "@/lib/api/client";
import type { LoginRequest, TokenResponse } from "@/types/auth";

export function login(payload: LoginRequest) {
  return apiClient<TokenResponse>("/api/v1/auth/login", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(payload),
  });
}
```

### 사용하는 쪽

```ts
const response = await login({
  login_id: "testuser",
  password: "password123",
});

console.log(response.access_token);
```

TypeScript는 `response`가 `TokenResponse` 타입이라고 이해한다.

따라서 다음 속성은 자동완성된다.

```text
response.access_token
response.token_type
```

반대로 없는 속성을 쓰면 오류를 알려준다.

```ts
console.log(response.token);
```

---

## 27. 자주 헷갈리는 점

### 타입 이름은 런타임에 남아 있는가?

남아 있지 않는다.

```ts
type Tag = {
  id: number;
  name: string;
};
```

이 `Tag`는 개발 중 타입 검사에 사용되고, JavaScript로 변환된 뒤에는 사라진다.

### 타입을 정의하면 API 응답이 자동으로 바뀌는가?

아니다.

TypeScript 타입은 프론트엔드 쪽 약속이다.

백엔드 응답 자체를 바꾸려면 FastAPI schema와 router/service 로직을 수정해야 한다.

### `type` 파일에도 함수가 들어갈 수 있는가?

문법상 가능하지만 현재 프로젝트에서는 권장하지 않는다.

현재 프로젝트에서는 책임을 분리한다.

```text
types/
= 타입 정의

lib/api/
= API 호출 함수

components/
= UI 컴포넌트
```

### `created_at`은 Date 타입인가?

API 응답으로 받은 시점에는 보통 `string`이다.

```ts
created_at: string;
```

화면에서 날짜 계산이나 포맷이 필요할 때 `Date`로 변환한다.

```ts
const createdAt = new Date(post.created_at);
```

---

## 28. 현재 프로젝트 기준 권장 규칙

현재 프로젝트에서는 다음 규칙을 따르면 된다.

```text
1. JSX가 있는 파일은 .tsx를 사용한다.
2. JSX가 없는 타입/함수 파일은 .ts를 사용한다.
3. API 요청/응답 타입은 frontend/types에 둔다.
4. API 호출 함수는 frontend/lib/api에 둔다.
5. 타입만 import할 때는 import type을 사용한다.
6. any는 가능한 사용하지 않는다.
7. 백엔드 API 필드명과 프론트엔드 타입 필드명을 일치시킨다.
8. 날짜 문자열은 우선 string으로 받는다.
9. 요청 타입과 응답 타입은 필요하면 분리한다.
10. 화면 컴포넌트 props에도 타입을 붙인다.
```

---

## 29. 한 문장 요약

TypeScript는 JavaScript 코드에 타입이라는 안전장치를 추가해서, 백엔드 API 데이터와 프론트엔드 화면 코드 사이의 실수를 개발 단계에서 줄여주는 도구이다.

현재 프로젝트에서 `frontend/types/*.ts` 파일은 다음 역할을 한다.

```text
백엔드 API가 주고받는 데이터의 모양을 프론트엔드에서 명확히 기록하는 파일
```

따라서 `types` 파일을 먼저 잘 작성해두면, 이후 `lib/api`, `components`, `app` 코드를 더 안전하고 일관되게 작성할 수 있다.
