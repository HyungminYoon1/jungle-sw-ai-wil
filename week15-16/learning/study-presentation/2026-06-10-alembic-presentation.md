# Alembic 팀 발표 교육 자료

## 1. 발표 목적

이 자료는 Alembic을 처음 접하는 팀원이 다음 내용을 이해하도록 돕기 위한 발표 자료다.

```text
1. Alembic이 왜 필요한지 이해한다.
2. SQLAlchemy 모델과 실제 DB 테이블의 차이를 이해한다.
3. migration, revision, upgrade, downgrade, head의 의미를 이해한다.
4. 현재 프로젝트에서 Alembic을 어떻게 사용하는지 이해한다.
5. 모델 변경 후 어떤 순서로 DB에 반영해야 하는지 설명할 수 있다.
```

발표 대상:

```text
FastAPI, SQLAlchemy, PostgreSQL을 처음 함께 써보는 팀원
DB migration 개념이 익숙하지 않은 백엔드 입문자
```

권장 발표 시간:

```text
20분 설명
10분 데모
5분 질문
```

## 2. 발표 전체 흐름

발표는 아래 순서로 진행하면 이해하기 쉽다.

```text
1. SQLAlchemy가 무엇인지 먼저 짚기
2. Alembic을 한 문장으로 정의하기
3. SQLAlchemy 모델과 실제 DB의 차이 설명
4. migration이 필요한 이유 설명
5. Alembic 핵심 용어 설명
6. 현재 프로젝트에서 Alembic 파일들이 하는 일 설명
7. 실제 작업 흐름 설명
8. 실습 데모
9. 자주 하는 실수 정리
```

발표자는 처음부터 명령어를 외우게 하려고 하기보다, 먼저 "왜 이 도구가 필요한지"를 납득시키는 것이 좋다.

### SQLAlchemy란

SQLAlchemy는 Python 코드로 데이터베이스를 다룰 수 있게 해주는 라이브러리다.

일반적으로 데이터베이스에 명령을 내리려면 SQL을 작성한다.

예:

```sql
SELECT id, email, username
FROM users
WHERE id = 1;
```

SQLAlchemy를 사용하면 이런 데이터베이스 작업을 Python 코드와 객체 중심으로 표현할 수 있다.

현재 프로젝트에서는 SQLAlchemy ORM을 사용한다.

ORM은 Object Relational Mapping의 줄임말이다.

쉽게 말하면:

```text
Python 클래스 <-> DB 테이블
Python 객체   <-> DB 행(row)
클래스 속성    <-> DB 컬럼
```

예를 들어 `users` 테이블은 Python 코드에서 `User` 클래스로 표현할 수 있다.

```python
class User(Base):
    __tablename__ = "users"

    id = ...
    email = ...
    username = ...
```

이런 클래스를 SQLAlchemy 모델이라고 부른다.

중요한 점은 SQLAlchemy 모델이 실제 DB 테이블 그 자체는 아니라는 것이다.

```text
SQLAlchemy 모델 = Python 코드에 작성한 DB 구조 설계
PostgreSQL 테이블 = DB 안에 실제로 존재하는 구조
```

Alembic은 이 SQLAlchemy 모델 정보를 읽어서 실제 PostgreSQL 구조를 만들거나 변경하는 migration을 관리한다.

## 3. Alembic 한 문장 정의

Alembic은 SQLAlchemy 프로젝트에서 데이터베이스 구조 변경 이력을 관리하는 도구다.

더 쉽게 말하면:

```text
DB 테이블 구조가 언제, 어떻게 바뀌었는지를 기록하고
그 기록을 실제 DB에 적용하거나 되돌릴 수 있게 해주는 도구
```

여기서 DB 구조란 다음을 말한다.

```text
테이블
컬럼
컬럼 타입
nullable 여부
unique 제약
index
foreign key
server default
```

## 4. 초심자용 비유

Alembic을 설명할 때는 건물 비유가 효과적이다.

```text
SQLAlchemy 모델 = 건물 설계도
Alembic migration = 공사 작업 기록
PostgreSQL schema = 실제 지어진 건물
```

각 역할:

| 개념 | 비유 | 설명 |
|---|---|---|
| SQLAlchemy model | 설계도 | Python 코드로 원하는 테이블 구조를 표현한다. |
| Alembic migration | 공사 기록 | 실제 DB에 어떤 변경을 적용할지 기록한다. |
| PostgreSQL schema | 실제 건물 | DB 안에 실제로 존재하는 테이블 구조다. |

핵심 메시지:

```text
설계도를 고쳤다고 실제 건물이 자동으로 바뀌지는 않는다.
SQLAlchemy 모델을 고쳤다고 PostgreSQL 테이블이 자동으로 바뀌지는 않는다.
```

그래서 Alembic migration이 필요하다.

## 5. SQLAlchemy 모델만으로는 부족한 이유

예를 들어 Python 코드에 다음 모델을 작성했다고 하자.

```python
class User(Base):
    __tablename__ = "users"

    id = ...
    email = ...
    username = ...
```

이 코드는 Python 입장에서 "users 테이블은 이런 구조였으면 좋겠다"는 선언이다.

하지만 PostgreSQL 안에 실제 테이블을 만들려면 SQL이 실행되어야 한다.

```sql
CREATE TABLE users (...);
```

모델과 DB 사이에는 차이가 있다.

```text
SQLAlchemy model = Python 코드에 있는 구조
PostgreSQL schema = 실제 DB 안에 있는 구조
```

Alembic은 이 둘 사이를 연결한다.

```text
SQLAlchemy model
-> Alembic migration
-> PostgreSQL schema
```

## 6. Migration이란

Migration은 DB 구조 변경 작업 하나를 뜻한다.

예:

```text
users 테이블 생성
posts 테이블 생성
users 테이블에 display_name 컬럼 추가
posts.title에 index 추가
comments 테이블 삭제
```

Migration 파일은 보통 Python 파일이다.

현재 프로젝트에서는 migration 파일이 아래 폴더에 저장된다.

```text
backend/alembic/versions/
```

예시:

```text
backend/alembic/versions/a57838f6f01a_create_initial_tables.py
```

이 파일은 단순한 임시 파일이 아니라 DB 변경 이력이다.

따라서 Git에 커밋해야 한다.

## 7. Revision이란

Revision은 migration 파일의 고유 ID다.

예:

```text
a57838f6f01a
```

Alembic은 revision ID를 사용해서 DB에 어떤 migration이 적용되었는지 추적한다.

PostgreSQL에는 Alembic이 관리하는 테이블이 하나 생긴다.

```text
alembic_version
```

이 테이블에는 현재 DB가 어느 revision까지 적용되었는지 기록된다.

발표할 때는 이렇게 설명하면 좋다.

```text
alembic_version은 DB의 현재 버전 번호를 적어두는 체크포인트다.
```

## 8. Upgrade와 Downgrade

Migration 파일에는 보통 두 함수가 있다.

```python
def upgrade() -> None:
    ...

def downgrade() -> None:
    ...
```

`upgrade`는 DB 구조를 앞으로 변경한다.

예:

```text
테이블 생성
컬럼 추가
index 추가
foreign key 추가
```

`downgrade`는 DB 구조 변경을 되돌린다.

예:

```text
테이블 삭제
컬럼 제거
index 제거
foreign key 제거
```

쉽게 말하면:

```text
upgrade = 앞으로 가기
downgrade = 뒤로 되돌리기
```

명령어 예:

```powershell
alembic upgrade head
alembic downgrade -1
```

주의:

```text
downgrade는 데이터 손실을 만들 수 있다.
운영 DB에서는 신중하게 사용해야 한다.
```

## 9. Head란

`head`는 Alembic migration 이력에서 가장 최신 revision을 뜻한다.

```powershell
alembic upgrade head
```

이 명령의 의미:

```text
아직 DB에 적용되지 않은 migration들을 최신 revision까지 모두 적용한다.
```

초심자에게는 이렇게 설명하면 된다.

```text
head는 migration 이력에서 가장 끝에 있는 revision이다.
보통 프로젝트가 기대하는 최신 DB 구조를 가리킨다.
```

## 10. Autogenerate란

Alembic은 SQLAlchemy 모델과 현재 DB 상태를 비교해서 migration 파일을 자동 생성할 수 있다.

명령어:

```powershell
alembic revision --autogenerate -m "create initial tables"
```

하지만 autogenerate는 "초안 생성"에 가깝다.

자동 생성된 migration은 반드시 사람이 검토해야 한다.

검토할 내용:

```text
생성되어야 할 테이블이 맞는가
추가되어야 할 컬럼이 맞는가
nullable 설정이 맞는가
unique 설정이 맞는가
index 설정이 맞는가
foreign key 설정이 맞는가
server_default가 필요한 곳에 있는가
downgrade가 적절한가
```

발표할 때 강조할 문장:

```text
autogenerate는 편리하지만 정답 보장 도구는 아니다.
```

## 11. 현재 프로젝트에서 Alembic의 위치

현재 프로젝트의 큰 구조:

```text
FastAPI Backend
-> SQLAlchemy model
-> Alembic migration
-> PostgreSQL
```

백엔드 계층 구조와 연결하면 다음과 같다.

```text
router
-> service
-> repository
-> SQLAlchemy model
-> Alembic migration
-> PostgreSQL schema
```

정확히 말하면 Alembic은 요청 처리 흐름 안에서 매번 실행되는 도구는 아니다.

Alembic은 개발자가 DB 구조를 만들거나 변경할 때 사용하는 개발 및 배포 도구다.

```text
API 요청 처리 시점:
router -> service -> repository -> database

DB 구조 변경 시점:
SQLAlchemy model -> Alembic migration -> database schema
```

## 12. 현재 프로젝트의 Alembic 관련 파일

현재 프로젝트의 Alembic 관련 핵심 파일:

```text
backend/alembic.ini
backend/alembic/env.py
backend/alembic/script.py.mako
backend/alembic/versions/
backend/app/db/base.py
backend/app/models/
```

### 12.1 `backend/alembic.ini`

Alembic 기본 설정 파일이다.

주의할 점:

```text
실제 DB URL이나 비밀번호를 alembic.ini에 직접 쓰면 안 된다.
```

현재 프로젝트에서는 `.env`에서 설정을 읽고, `env.py`에서 Alembic 설정에 주입하는 방향이다.

발표에서는 실제 `.env` 값이나 실제 DB URL을 보여주지 않는다.

### 12.2 `backend/alembic/env.py`

Alembic이 실행될 때 사용하는 Python 설정 파일이다.

중요한 역할:

```text
1. DB 연결 설정을 준비한다.
2. SQLAlchemy Base.metadata를 Alembic에 알려준다.
3. 모델 파일들을 import해서 metadata에 테이블 정보를 등록한다.
```

핵심 개념:

```text
Base.metadata가 있어야 Alembic이 모델에 정의된 테이블 구조를 알 수 있다.
```

### 12.3 `backend/alembic/script.py.mako`

새 migration 파일을 만들 때 사용하는 템플릿이다.

보통 직접 수정하지 않는다.

### 12.4 `backend/alembic/versions/`

실제 migration 파일들이 저장되는 폴더다.

여기에 들어 있는 파일들은 DB 변경 이력이므로 Git에 커밋한다.

### 12.5 `backend/app/models/`

SQLAlchemy 모델들이 들어 있는 폴더다.

현재 프로젝트의 주요 모델:

```text
User
Post
Comment
Tag
PostTag
```

Alembic autogenerate는 이 모델 정보를 기준으로 migration 초안을 만든다.

## 13. Alembic의 기술적 동작 원리

Alembic은 단순히 "명령어를 실행하면 DB가 바뀐다" 수준으로 동작하지 않는다.

내부적으로는 프로젝트 설정, SQLAlchemy 모델 정보, 실제 DB 상태, migration 파일 이력을 함께 사용한다.

큰 흐름은 다음과 같다.

```text
SQLAlchemy model
-> Base.metadata
-> Alembic env.py
-> autogenerate 또는 migration 실행
-> versions 폴더의 migration 파일
-> PostgreSQL schema
-> alembic_version 갱신
```

### 13.1 `alembic revision --autogenerate` 동작 흐름

`alembic revision --autogenerate`는 migration 파일 초안을 만드는 명령이다.

내부 흐름은 대략 다음과 같다.

```text
1. Alembic이 alembic.ini를 읽는다.
2. alembic.ini에 지정된 script location을 찾는다.
3. backend/alembic/env.py를 실행한다.
4. env.py가 DB 연결 설정을 준비한다.
5. env.py가 app.models를 import한다.
6. SQLAlchemy 모델들이 Base.metadata에 등록된다.
7. env.py가 target_metadata = Base.metadata를 Alembic에 전달한다.
8. Alembic이 실제 DB schema를 조회한다.
9. Base.metadata와 실제 DB schema를 비교한다.
10. 차이가 있으면 versions 폴더에 migration 파일 초안을 만든다.
```

핵심은 `Base.metadata`다.

```text
Base.metadata = SQLAlchemy 모델들이 모여 있는 테이블 설계 정보
```

예를 들어 `User`, `Post`, `Comment`, `Tag`, `PostTag` 모델이 모두 import되어야 Alembic이 전체 테이블 구조를 알 수 있다.

그래서 `env.py`에서 모델 import가 빠지면 autogenerate가 테이블 변경을 제대로 감지하지 못할 수 있다.

### 13.2 `alembic upgrade head` 동작 흐름

`alembic upgrade head`는 아직 DB에 적용되지 않은 migration을 최신 revision까지 실행하는 명령이다.

내부 흐름은 대략 다음과 같다.

```text
1. Alembic이 alembic.ini를 읽는다.
2. backend/alembic/env.py를 실행한다.
3. DB 연결을 준비한다.
4. backend/alembic/versions 폴더의 migration 파일들을 읽는다.
5. DB의 alembic_version 테이블을 조회한다.
6. 현재 DB가 어느 revision까지 적용되었는지 확인한다.
7. 현재 revision부터 head까지 필요한 migration 목록을 계산한다.
8. down_revision 순서에 맞춰 migration의 upgrade()를 실행한다.
9. 성공하면 alembic_version 값을 최신 revision으로 갱신한다.
```

즉 `upgrade head`는 무작정 모든 migration을 다시 실행하지 않는다.

현재 DB의 revision을 보고 아직 적용되지 않은 migration만 실행한다.

### 13.3 revision과 down_revision

각 migration 파일에는 보통 이런 값이 있다.

```python
revision = "bbbb"
down_revision = "aaaa"
```

의미:

```text
revision = 이 migration 파일의 ID
down_revision = 이 migration 바로 전에 와야 하는 revision의 ID
```

Alembic은 이 값을 이용해 migration 순서를 만든다.

예:

```text
revision aaaa
-> revision bbbb
-> revision cccc
```

이 구조 덕분에 Alembic은 어떤 migration을 먼저 실행해야 하는지 알 수 있다.

단순한 선형 migration 이력에서는 `down_revision`이 보통 이전 migration ID 하나다.

다만 첫 migration의 `down_revision`은 `None`일 수 있고, 팀원이 동시에 migration을 만들어 이력이 갈라지면 head가 여러 개 생길 수도 있다.

초심자 단계에서는 먼저 아래처럼 이해하면 충분하다.

```text
revision = 현재 migration의 버전 ID
down_revision = 이 migration 직전에 적용되어야 하는 버전 ID
```

### 13.4 alembic_version 테이블

Alembic은 DB 안에 `alembic_version` 테이블을 만든다.

이 테이블에는 현재 DB에 적용된 revision ID가 저장된다.

예:

```text
alembic_version
----------------
version_num
bbbb
```

의미:

```text
현재 이 DB는 bbbb revision까지 적용되어 있다.
```

프로젝트 코드의 최신 migration이 `cccc`라면 Alembic은 다음처럼 판단한다.

```text
DB 현재 revision = bbbb
프로젝트 head = cccc
적용해야 할 migration = bbbb 이후부터 cccc까지 아직 적용되지 않은 migration
```

그래서 `alembic upgrade head`를 실행하면 아직 적용되지 않은 migration들의 `upgrade()`가 순서대로 실행된다.

예를 들어 이력이 아래와 같다면:

```text
aaaa -> bbbb -> cccc -> dddd
```

현재 DB가 `bbbb`까지 적용된 상태에서 `head`가 `dddd`라면 Alembic은 다음을 실행한다.

```text
cccc의 upgrade()
dddd의 upgrade()
```

### 13.5 autogenerate가 비교하는 것

Autogenerate는 크게 두 가지를 비교한다.

```text
1. SQLAlchemy 모델이 나타내는 구조
2. 실제 DB에 존재하는 구조
```

예:

```text
SQLAlchemy model에는 users.bio 컬럼이 있다.
실제 DB의 users 테이블에는 bio 컬럼이 없다.
```

그러면 Alembic은 migration 초안에 다음과 비슷한 코드를 만든다.

```python
def upgrade() -> None:
    op.add_column(
        "users",
        sa.Column("bio", sa.Text(), nullable=True),
    )
```

반대로 모델에서 컬럼을 제거하면 `op.drop_column()` 같은 코드가 생성될 수 있다.

그래서 autogenerate 결과는 반드시 검토해야 한다.

컬럼 삭제나 타입 변경은 데이터 손실을 만들 수 있기 때문이다.

### 13.6 Alembic이 자동으로 해결하지 않는 것

Alembic은 변경 이력을 관리하고 적용 순서를 계산해준다.

하지만 아래 문제를 자동으로 안전하게 해결해주지는 않는다.

```text
기존 데이터와 새 제약조건의 충돌
컬럼 이름 변경 의도 파악
데이터 손실이 있는 컬럼 삭제
복잡한 데이터 이전
팀원이 동시에 만든 migration branch 충돌
```

예를 들어 컬럼 이름을 `username`에서 `login_id`로 바꿨다고 하자.

Alembic autogenerate는 이것을 "이름 변경"으로 이해하지 못하고 아래처럼 볼 수 있다.

```text
username 컬럼 삭제
login_id 컬럼 추가
```

이 경우 기존 `username` 데이터가 사라질 수 있으므로 사람이 migration 파일을 수정해야 한다.

발표할 때 핵심 문장은 다음이다.

```text
Alembic은 DB schema 변경의 버전 관리 도구다.
하지만 안전한 migration 설계는 개발자가 책임져야 한다.
```

## 14. DB 생성과 테이블 생성은 다르다

초심자가 자주 헷갈리는 부분이다.

```text
DB 생성 = PostgreSQL에 데이터베이스 자체를 만든다.
테이블 생성 = 이미 존재하는 DB 안에 users, posts 같은 테이블을 만든다.
```

현재 프로젝트 기준:

```text
DB 생성: backend/scripts/create_database.py
테이블 생성/변경: alembic upgrade head
```

Alembic은 보통 데이터베이스 자체를 생성하는 도구가 아니다.

Alembic은 이미 존재하는 DB 안에서 테이블과 컬럼 구조를 관리한다.

## 15. 현재 프로젝트의 기본 작업 흐름

모델 변경 후 DB에 반영하는 기본 흐름:

```text
1. SQLAlchemy 모델 작성 또는 수정
2. app.models.__init__.py에서 모델 import 확인
3. migration 파일 자동 생성
4. migration 파일 검토
5. migration 적용
6. DB 구조 확인
7. migration 파일 커밋
```

명령어 흐름:

```powershell
cd C:\developer_folder\study-web-hyungmin\backend
alembic revision --autogenerate -m "describe change"
alembic upgrade head
alembic current
```

커밋 대상:

```text
backend/alembic/versions/*.py
모델 변경 파일
관련 코드 변경 파일
```

커밋하면 안 되는 파일:

```text
backend/.env
```

## 16. 발표용 예시 시나리오

초심자에게는 실제 프로젝트와 가까운 예시가 좋다.

예시 요구사항:

```text
사용자 프로필에 bio 컬럼을 추가하고 싶다.
```

작업 흐름:

```text
1. User 모델에 bio 컬럼 추가
2. migration 자동 생성
3. 생성된 migration 파일 확인
4. alembic upgrade head 실행
5. PostgreSQL users 테이블에 bio 컬럼이 생겼는지 확인
```

개념 흐름:

```text
User 모델 수정
-> Alembic이 모델과 DB 차이를 비교
-> "bio 컬럼 추가" migration 생성
-> migration 적용
-> 실제 DB 테이블 변경
```

발표 시 실제 코드를 수정하지 않아도, 이 흐름만 그림으로 설명해도 충분하다.

### 16.1 변경 전 모델 예시

처음 `User` 모델이 아래와 같다고 가정한다.

```python
class User(Base):
    __tablename__ = "users"

    id = ...
    email = ...
    username = ...
```

이 상태의 의미:

```text
Python 코드에는 users 테이블에 id, email, username이 있다고 선언되어 있다.
```

하지만 이 코드는 실제 PostgreSQL 테이블을 직접 바꾸지는 않는다.

### 16.2 변경 후 모델 예시

요구사항이 생겨서 사용자 자기소개를 저장하는 `bio` 컬럼을 추가한다고 하자.

```python
from sqlalchemy import Text
from sqlalchemy.orm import Mapped, mapped_column


class User(Base):
    __tablename__ = "users"

    id = ...
    email = ...
    username = ...
    bio: Mapped[str | None] = mapped_column(Text, nullable=True)
```

이 변경은 Python 모델을 수정한 것이다.

아직 실제 DB에 `bio` 컬럼이 생긴 것은 아니다.

```text
모델 수정 완료
DB 변경은 아직 안 됨
```

### 16.3 migration 생성 명령

모델 변경 후 migration 파일을 생성한다.

```powershell
cd C:\developer_folder\study-web-hyungmin\backend
alembic revision --autogenerate -m "add bio to users"
```

이 명령의 의미:

```text
현재 SQLAlchemy 모델과 실제 DB 구조를 비교해서
차이가 있으면 migration 파일 초안을 만든다.
```

### 16.4 생성될 수 있는 migration 예시

Alembic이 생성하는 migration 파일은 대략 아래와 비슷하다.

```python
from alembic import op
import sqlalchemy as sa


def upgrade() -> None:
    op.add_column(
        "users",
        sa.Column("bio", sa.Text(), nullable=True),
    )


def downgrade() -> None:
    op.drop_column("users", "bio")
```

`upgrade()`의 의미:

```text
users 테이블에 bio 컬럼을 추가한다.
```

`downgrade()`의 의미:

```text
users 테이블에서 bio 컬럼을 제거한다.
```

발표할 때 강조할 점:

```text
autogenerate는 이 파일을 자동으로 만들어주지만,
항상 사람이 upgrade와 downgrade 내용을 직접 확인해야 한다.
```

### 16.5 migration 적용 명령

검토가 끝났다면 실제 DB에 적용한다.

```powershell
alembic upgrade head
```

이 명령이 실행되면 PostgreSQL의 `users` 테이블에 `bio` 컬럼이 추가된다.

흐름을 다시 정리하면:

```text
User 모델에 bio 추가
-> alembic revision --autogenerate
-> migration 파일 검토
-> alembic upgrade head
-> 실제 users 테이블에 bio 컬럼 추가
```

### 16.6 충돌이 생길 수 있는 예시

아래처럼 `nullable=False` 컬럼을 바로 추가하면 문제가 생길 수 있다.

```python
def upgrade() -> None:
    op.add_column(
        "users",
        sa.Column("bio", sa.Text(), nullable=False),
    )
```

문제 상황:

```text
이미 users 테이블에 기존 row가 있다.
새 bio 컬럼은 NULL을 허용하지 않는다.
하지만 기존 row에는 bio 값이 없다.
```

이 경우 DB는 이렇게 판단할 수 있다.

```text
기존 데이터에 넣을 bio 값이 없는데 nullable=False 제약이 있다.
따라서 migration을 적용할 수 없다.
```

더 안전한 방식은 단계를 나누는 것이다.

```text
1. bio 컬럼을 nullable=True로 추가
2. 기존 사용자 row에 기본 bio 값 채우기
3. 필요하다면 nullable=False 제약 추가
```

예시:

```python
def upgrade() -> None:
    op.add_column(
        "users",
        sa.Column("bio", sa.Text(), nullable=True),
    )

    op.execute("UPDATE users SET bio = '' WHERE bio IS NULL")

    op.alter_column(
        "users",
        "bio",
        nullable=False,
    )
```

이렇게 최종적으로 DB 컬럼을 `nullable=False`로 만들었다면 SQLAlchemy 모델도 그 의도에 맞게 정리해야 한다.

예:

```python
bio: Mapped[str] = mapped_column(Text, nullable=False)
```

반대로 모델이 계속 아래처럼 되어 있으면 모델은 여전히 NULL 허용 구조를 표현하는 것이다.

```python
bio: Mapped[str | None] = mapped_column(Text, nullable=True)
```

이 예시는 민감 정보나 실제 접속 정보를 포함하지 않는다.

핵심은 다음이다.

```text
DB 구조 변경은 기존 데이터와 충돌할 수 있다.
Alembic은 변경 순서를 관리해주지만,
안전한 변경 순서는 개발자가 직접 설계해야 한다.
```

## 17. 데모 순서

데모는 실제 프로젝트에서 아래 순서로 보여주면 된다.

### 17.1 현재 Alembic 상태 확인

백엔드 폴더로 이동:

```powershell
cd C:\developer_folder\study-web-hyungmin\backend
```

현재 DB revision 확인:

```powershell
alembic current
```

전체 migration 이력 확인:

```powershell
alembic history
```

설명 포인트:

```text
history는 프로젝트가 가진 migration 이력이다.
current는 현재 DB가 어디까지 적용되었는지를 보여준다.
```

### 17.2 migration 파일 열어보기

확인할 위치:

```text
backend/alembic/versions/
```

설명할 부분:

```text
revision ID
down_revision
upgrade()
downgrade()
```

`down_revision`은 이전 migration을 가리킨다.

이 값으로 Alembic은 migration 순서를 안다.

### 17.3 migration 적용 명령 설명

최신 상태로 적용:

```powershell
alembic upgrade head
```

마지막 migration 하나 되돌리기:

```powershell
alembic downgrade -1
```

주의:

```text
팀 발표 데모에서는 운영 DB가 아니라 로컬 개발 DB에서만 downgrade를 보여준다.
```

## 18. 발표자가 그대로 읽을 수 있는 설명 스크립트

아래 문장은 발표자가 거의 그대로 읽어도 된다.

### 18.1 시작 멘트

```text
오늘은 Alembic을 설명하겠습니다.
Alembic은 SQLAlchemy를 쓰는 Python 프로젝트에서 DB 테이블 구조 변경 이력을 관리하는 도구입니다.
우리가 User 모델을 작성했다고 해서 PostgreSQL에 users 테이블이 자동으로 생기는 것은 아닙니다.
모델은 Python 코드에 있는 설계도이고, 실제 DB 구조를 바꾸려면 migration이라는 변경 기록을 만들고 적용해야 합니다.
Alembic이 바로 그 migration을 만들고 실행하는 도구입니다.
```

### 18.2 비유 설명

```text
건물로 비유하면 SQLAlchemy 모델은 설계도입니다.
PostgreSQL의 실제 테이블은 완성된 건물입니다.
설계도를 고쳤다고 건물이 자동으로 바뀌지는 않습니다.
중간에 어떤 공사를 할지 기록하고, 그 공사를 실제로 실행해야 합니다.
이 공사 기록이 migration이고, 그 migration을 관리하는 도구가 Alembic입니다.
```

### 18.3 핵심 용어 설명

```text
Migration은 DB 구조 변경 작업 하나입니다.
Revision은 migration 파일의 고유 ID입니다.
Head는 현재 코드 기준 가장 최신 migration입니다.
Upgrade는 DB를 최신 구조로 앞으로 변경하는 것이고, downgrade는 이전 구조로 되돌리는 것입니다.
```

### 18.4 현재 프로젝트 연결 설명

```text
우리 프로젝트에서는 backend/app/models 폴더에 SQLAlchemy 모델이 있습니다.
Alembic은 이 모델 정보를 Base.metadata를 통해 읽습니다.
그리고 backend/alembic/versions 폴더에 migration 파일을 만듭니다.
우리는 그 파일을 검토한 다음 alembic upgrade head 명령으로 PostgreSQL에 실제 테이블을 생성하거나 변경합니다.
```

### 18.5 주의사항 설명

```text
중요한 점은 migration 파일을 반드시 검토해야 한다는 것입니다.
autogenerate는 모델과 DB 차이를 보고 초안을 만들어주지만 항상 완벽하지는 않습니다.
또 alembic.ini나 발표 자료에 실제 DB URL, 비밀번호, secret 값을 적으면 안 됩니다.
환경 변수나 .env의 실제 값은 절대 출력하지 않는 것이 원칙입니다.
```

### 18.6 마무리 멘트

```text
정리하면 Alembic은 DB 구조 변경 이력을 안전하게 관리하기 위한 도구입니다.
SQLAlchemy 모델만으로는 실제 DB가 바뀌지 않습니다.
모델 변경 후에는 migration을 만들고, 검토하고, 적용하고, 커밋해야 합니다.
이 흐름을 지키면 팀원이 같은 DB 구조를 재현할 수 있고, 변경 이력을 추적할 수 있습니다.
```

## 19. 슬라이드 구성 예시

발표 자료를 슬라이드로 만들면 아래 구성이 적당하다.

```text
1. 제목: Alembic이란?
2. 왜 필요한가: 모델과 실제 DB는 다르다
3. 비유: 설계도, 공사 기록, 실제 건물
4. 핵심 흐름: model -> migration -> database
5. 핵심 용어: migration, revision, head
6. upgrade와 downgrade
7. 현재 프로젝트 파일 구조
8. 실제 작업 순서
9. 데모: alembic current, history, versions 파일 보기
10. 주의사항
11. 요약
12. Q&A
```

각 슬라이드의 핵심 문장:

```text
1. Alembic은 DB 구조 변경 이력을 관리하는 도구다.
2. 모델을 고쳤다고 DB가 자동으로 바뀌지 않는다.
3. 모델은 설계도, migration은 공사 기록, DB는 실제 건물이다.
4. 변경은 model -> migration -> database 순서로 반영된다.
5. revision은 migration의 버전 ID다.
6. upgrade는 적용, downgrade는 되돌리기다.
7. versions 폴더의 migration 파일은 커밋해야 한다.
8. autogenerate 후 반드시 사람이 검토해야 한다.
9. .env와 secret은 절대 출력하지 않는다.
```

## 20. 팀원에게 꼭 전달할 핵심 5가지

```text
1. SQLAlchemy 모델은 DB 구조의 선언일 뿐이다.
2. Alembic migration을 적용해야 실제 PostgreSQL 구조가 바뀐다.
3. migration 파일은 DB 변경 이력이므로 Git에 커밋해야 한다.
4. autogenerate 결과는 반드시 검토해야 한다.
5. 실제 DB URL, 비밀번호, secret 값은 migration 설정이나 문서에 직접 쓰면 안 된다.
```

## 21. 자주 하는 실수

### 21.1 모델만 수정하고 migration을 만들지 않음

상황:

```text
User 모델에 컬럼을 추가했는데 DB에는 컬럼이 없다.
```

원인:

```text
모델 수정 후 migration을 만들고 적용하지 않았다.
```

해결:

```powershell
alembic revision --autogenerate -m "add user column"
alembic upgrade head
```

### 21.2 migration 파일을 커밋하지 않음

상황:

```text
내 로컬 DB에서는 잘 되는데 팀원 DB에서는 테이블이 없다.
```

원인:

```text
migration 파일을 Git에 커밋하지 않았다.
```

해결:

```text
backend/alembic/versions/*.py 파일을 커밋한다.
```

### 21.3 autogenerate 결과를 확인하지 않음

상황:

```text
의도하지 않은 컬럼 삭제나 제약 변경이 migration에 들어갔다.
```

원인:

```text
자동 생성된 migration 파일을 검토하지 않았다.
```

해결:

```text
upgrade(), downgrade() 내용을 직접 읽고 의도한 변경만 있는지 확인한다.
```

### 21.4 실제 DB URL을 설정 파일에 직접 작성

상황:

```text
alembic.ini에 실제 DB 접속 정보가 들어갔다.
```

문제:

```text
DB 사용자 이름, 비밀번호가 Git에 올라갈 수 있다.
```

해결:

```text
실제 값은 .env 또는 환경 변수에서 읽는다.
문서와 로그에는 secret 값을 출력하지 않는다.
```

### 21.5 DB 생성과 테이블 생성을 혼동

상황:

```text
alembic upgrade head를 실행했는데 데이터베이스 자체가 없다는 오류가 난다.
```

원인:

```text
Alembic은 보통 DB 자체를 만들지 않는다.
```

해결:

```text
먼저 데이터베이스를 생성한 뒤 alembic upgrade head를 실행한다.
```

## 22. 예상 질문과 답변

### Q1. SQLAlchemy 모델을 만들면 테이블이 자동으로 생기나요?

아니다.

SQLAlchemy 모델은 Python 코드에 있는 테이블 구조 선언이다.

실제 PostgreSQL에 테이블을 만들려면 Alembic migration을 생성하고 적용해야 한다.

### Q2. Alembic migration 파일은 커밋해야 하나요?

해야 한다.

Migration 파일은 DB 구조 변경 이력이다.

팀원이 같은 DB 구조를 만들려면 migration 파일이 필요하다.

### Q3. autogenerate만 믿어도 되나요?

안 된다.

Autogenerate는 초안을 만들어주는 기능이다.

생성된 migration 파일의 `upgrade()`와 `downgrade()`를 반드시 검토해야 한다.

### Q4. `alembic upgrade head`는 무슨 뜻인가요?

아직 적용되지 않은 migration들을 최신 revision까지 DB에 적용하라는 뜻이다.

`head`는 가장 최신 migration을 의미한다.

### Q5. `downgrade`는 언제 쓰나요?

로컬 개발 중 실수한 migration을 되돌리거나, 특정 변경을 롤백해야 할 때 사용한다.

하지만 데이터 손실이 생길 수 있으므로 운영 DB에서는 매우 신중하게 사용해야 한다.

### Q6. `alembic_version` 테이블은 무엇인가요?

Alembic이 DB에 현재 적용된 revision을 기록하는 테이블이다.

개발자가 직접 수정하지 않는다.

### Q7. migration 파일을 이미 적용했는데 수정해도 되나요?

혼자 로컬에서만 적용한 초기 작업이면 상황에 따라 가능하다.

하지만 이미 팀에 공유했거나 다른 환경에 적용된 migration은 수정하지 않는 것이 원칙이다.

그 경우 새 migration을 추가해서 변경한다.

### Q8. Alembic이 데이터도 옮겨주나요?

기본적으로 Alembic은 DB 구조 변경을 관리한다.

필요하면 migration 안에 데이터 변경 SQL을 직접 작성할 수 있지만, 초심자 단계에서는 구조 변경 관리 도구로 이해하는 것이 좋다.

## 23. 발표 중 칠판에 그릴 그림

간단한 그림 하나만 그려도 이해가 빨라진다.

```text
Python 코드

User model
Post model
Comment model
      |
      v
Base.metadata
      |
      v
Alembic autogenerate
      |
      v
migration file
      |
      v
alembic upgrade head
      |
      v
PostgreSQL tables
```

옆에 이렇게 적는다.

```text
모델 작성만으로는 DB가 바뀌지 않는다.
migration 생성과 적용이 필요하다.
```

## 24. 실습 과제

발표 후 팀원에게 줄 수 있는 간단한 과제다.

```text
1. alembic history 명령으로 migration 이력을 확인한다.
2. alembic current 명령으로 현재 DB revision을 확인한다.
3. versions 폴더의 migration 파일을 열어 revision, down_revision, upgrade, downgrade를 찾는다.
4. User 모델과 migration 파일에서 users 테이블 컬럼이 어떻게 연결되는지 비교한다.
5. 왜 migration 파일을 커밋해야 하는지 한 문장으로 정리한다.
```

모범 답안 예:

```text
Migration 파일을 커밋해야 다른 팀원도 같은 DB 구조를 재현할 수 있다.
```

## 25. 발표 마무리 요약

마지막에는 아래 문장으로 정리하면 좋다.

```text
Alembic은 DB 구조 변경 이력을 관리하는 도구다.
SQLAlchemy 모델은 설계도이고, migration은 실제 DB에 적용할 변경 기록이다.
모델을 수정한 뒤에는 migration을 생성하고, 검토하고, upgrade로 적용해야 한다.
migration 파일은 팀 전체가 같은 DB 구조를 재현하기 위해 반드시 커밋해야 한다.
secret 값과 실제 DB 접속 정보는 문서나 설정 파일에 직접 남기지 않는다.
```

## 26. 발표자 체크리스트

발표 전 확인:

```text
로컬 DB가 준비되어 있는가
backend 폴더에서 alembic 명령이 실행되는가
alembic current가 동작하는가
alembic history가 동작하는가
versions 폴더의 migration 파일을 열 수 있는가
.env 내용이나 실제 secret 값을 화면에 띄우지 않는가
```

발표 중 강조:

```text
모델과 실제 DB는 다르다.
migration은 변경 이력이다.
autogenerate는 검토가 필요하다.
versions 폴더는 커밋 대상이다.
secret은 절대 출력하지 않는다.
```

