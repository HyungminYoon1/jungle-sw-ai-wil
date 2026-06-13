# Alembic 학습 문서

## 1. Alembic이란

Alembic은 SQLAlchemy 프로젝트에서 데이터베이스 스키마 변경 이력을 관리하는 도구다.

여기서 스키마는 테이블, 컬럼, 인덱스, 외래 키 같은 데이터베이스 구조를 의미한다.

예를 들어 SQLAlchemy 모델에 `User` 클래스를 작성했다고 해서 PostgreSQL에 `users` 테이블이 자동으로 생기지는 않는다.

```text
SQLAlchemy model
-> Alembic migration
-> PostgreSQL schema
```

Alembic은 Python 코드로 작성된 migration 파일을 실행해서 실제 DB 구조를 변경한다.

## 2. SQLAlchemy 모델과 Alembic의 차이

SQLAlchemy 모델은 Python 코드로 DB 구조를 표현한 것이다.

예:

```python
class User(Base):
    __tablename__ = "users"

    id = ...
    email = ...
    username = ...
```

이 코드는 "users 테이블은 이런 구조여야 한다"는 선언이다.

하지만 실제 PostgreSQL에 테이블을 만들려면 SQL이 필요하다.

```sql
CREATE TABLE users (...);
```

Alembic은 이 변경을 migration 파일로 관리한다.

```text
모델 = 원하는 DB 구조를 Python 코드로 표현
migration = 그 구조를 실제 DB에 적용하는 변경 이력
```

## 3. Migration이란

Migration은 DB 구조 변경 작업 하나를 의미한다.

예:

```text
users 테이블 생성
posts 테이블 생성
comments 테이블 생성
tags 테이블 생성
post_tags 테이블 생성
```

현재 프로젝트의 첫 migration 파일은 다음 역할을 한다.

```text
users, posts, comments, tags, post_tags 테이블 생성
```

파일 위치:

```text
backend/alembic/versions/a57838f6f01a_create_initial_tables.py
```

## 4. Revision이란

Revision은 migration 파일의 고유 ID다.

예:

```text
a57838f6f01a
```

Alembic은 이 ID를 사용해서 어떤 migration이 DB에 적용되었는지 추적한다.

DB에는 Alembic이 관리하는 테이블이 하나 생긴다.

```text
alembic_version
```

이 테이블에는 현재 적용된 revision ID가 저장된다.

## 5. Upgrade와 Downgrade

Migration 파일에는 보통 두 함수가 있다.

```python
def upgrade() -> None:
    ...

def downgrade() -> None:
    ...
```

`upgrade`는 DB를 앞으로 변경한다.

예:

```text
테이블 생성
컬럼 추가
인덱스 추가
외래 키 추가
```

`downgrade`는 DB 변경을 되돌린다.

예:

```text
테이블 삭제
컬럼 삭제
인덱스 삭제
외래 키 삭제
```

## 6. Head란

`head`는 Alembic migration 이력에서 가장 최신 revision을 의미한다.

```powershell
alembic upgrade head
```

이 명령은 아직 적용되지 않은 migration들을 최신 revision까지 적용한다.

## 7. 현재 프로젝트의 Alembic 파일 역할

### `backend/alembic.ini`

Alembic 기본 설정 파일이다.

현재 프로젝트에서는 실제 DB URL을 이 파일에 직접 쓰지 않는다.

이유:

```text
DB URL에는 사용자 이름, 비밀번호 같은 민감 정보가 포함될 수 있다.
```

실제 DB URL은 `.env`에서 읽고, `env.py`에서 Alembic 설정에 주입한다.

### `backend/alembic/env.py`

Alembic이 실행될 때 사용하는 Python 설정 파일이다.

현재 프로젝트에서 중요한 역할:

```text
1. .env에서 DATABASE_URL 읽기
2. SQLAlchemy Base.metadata 연결
3. 모델들을 import해서 Alembic이 테이블 구조를 알 수 있게 하기
```

핵심 코드:

```python
from app.core.config import settings
from app.db.base import Base
import app.models

config.set_main_option("sqlalchemy.url", settings.database_url)
target_metadata = Base.metadata
```

`target_metadata = Base.metadata`가 있어야 Alembic autogenerate가 모델 구조를 읽을 수 있다.

`import app.models`가 있어야 `User`, `Post`, `Comment`, `Tag`, `PostTag` 모델이 실제로 import되어 `Base.metadata`에 등록된다.

### `backend/alembic/script.py.mako`

새 migration 파일을 만들 때 사용하는 템플릿이다.

예:

```powershell
alembic revision --autogenerate -m "create initial tables"
```

이 명령을 실행하면 Alembic은 `script.py.mako`를 바탕으로 `versions/` 아래에 새 migration 파일을 만든다.

보통 직접 수정하지 않고 초기 생성 상태 그대로 둔다.

### `backend/alembic/versions/`

실제 migration 파일들이 저장되는 폴더다.

예:

```text
backend/alembic/versions/a57838f6f01a_create_initial_tables.py
```

이 파일들은 DB 스키마 변경 이력이므로 Git에 커밋해야 한다.

## 8. 현재 프로젝트의 Alembic 흐름

현재 프로젝트의 기본 흐름:

```text
1. SQLAlchemy 모델 작성
2. app.models.__init__.py에서 모델 import
3. Alembic env.py가 Base.metadata를 읽음
4. alembic revision --autogenerate 실행
5. migration 파일 생성
6. migration 파일 검토
7. alembic upgrade head 실행
8. PostgreSQL에 테이블 생성
```

## 9. 자주 쓰는 명령어

백엔드 폴더에서 실행한다.

```powershell
cd C:\developer_folder\study-web-hyungmin\backend
```

현재 DB에 적용된 revision 확인:

```powershell
alembic current
```

전체 migration 이력 확인:

```powershell
alembic history
```

모델 기준으로 migration 파일 자동 생성:

```powershell
alembic revision --autogenerate -m "create initial tables"
```

최신 migration까지 DB에 적용:

```powershell
alembic upgrade head
```

마지막 migration 하나 되돌리기:

```powershell
alembic downgrade -1
```

특정 revision으로 되돌리기:

```powershell
alembic downgrade <revision_id>
```

## 10. DB 생성과 테이블 생성의 차이

Alembic은 보통 데이터베이스 자체를 생성하지 않는다.

예:

```sql
CREATE DATABASE study_board;
```

이 작업은 현재 프로젝트에서 별도 스크립트가 담당한다.

```text
backend/scripts/create_database.py
```

Alembic은 이미 존재하는 DB 안에서 테이블과 컬럼을 관리한다.

```text
DB 생성: scripts/create_database.py
테이블 생성: alembic upgrade head
```

## 11. Migration 파일을 커밋해야 하는 이유

Migration 파일은 DB 변경 이력이다.

커밋하지 않으면 다른 환경에서 같은 DB 구조를 재현할 수 없다.

예를 들어 다른 사람이 프로젝트를 받은 뒤:

```powershell
alembic upgrade head
```

를 실행하려면 `versions/` 안의 migration 파일이 필요하다.

따라서 아래 파일들은 커밋 대상이다.

```text
backend/alembic.ini
backend/alembic/env.py
backend/alembic/script.py.mako
backend/alembic/README
backend/alembic/versions/*.py
```

반대로 아래 파일은 커밋하면 안 된다.

```text
backend/.env
```

## 12. 주의할 점

### 12.1 Secret을 Alembic 설정에 직접 쓰지 않기

`alembic.ini`에 실제 DB URL을 쓰면 안 된다.

잘못된 예:

```ini
sqlalchemy.url = postgresql+psycopg://real_user:real_password@localhost:5432/study_board
```

이런 값은 Git에 올라가면 안 된다.

현재 프로젝트는 `.env`에서 읽은 값을 `env.py`에서 주입하는 방식을 사용한다.

### 12.2 자동 생성된 migration은 반드시 검토하기

`--autogenerate`는 편리하지만 완벽하지 않다.

생성된 파일에서 다음을 확인한다.

```text
필요한 테이블이 모두 생성되는가
nullable 설정이 맞는가
unique/index 설정이 맞는가
foreign key가 맞는가
server_default가 필요한 컬럼에 들어갔는가
downgrade가 적절한가
```

### 12.3 모델 변경 후 migration을 새로 만들기

모델을 수정했다고 DB가 자동으로 바뀌지 않는다.

예:

```text
Post 모델에 컬럼 추가
```

그 다음 해야 할 일:

```powershell
alembic revision --autogenerate -m "add column to posts"
alembic upgrade head
```

### 12.4 이미 적용한 migration 파일 수정 주의

아직 다른 환경에 적용하지 않은 초기 migration은 수정할 수 있다.

하지만 이미 공유되었거나 운영 DB에 적용된 migration 파일은 함부로 수정하지 않는다.

그 경우 새 migration을 추가해서 변경한다.

## 13. 현재 프로젝트에서 기억할 기준

현재 프로젝트에서는 다음 기준을 따른다.

```text
SQLAlchemy 모델 작성
-> migration 자동 생성
-> migration 파일 검토
-> alembic upgrade head
-> DB 테이블 확인
-> migration 파일 커밋
```

그리고 항상 아래 원칙을 지킨다.

```text
실제 secret 값은 출력하지 않는다.
.env 파일은 커밋하지 않는다.
alembic.ini에는 실제 DB URL을 쓰지 않는다.
migration 파일은 커밋한다.
```
