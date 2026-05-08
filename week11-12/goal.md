# Week 11-12 - 핵심 역량 목표 설정

> 작성 시점: 주차 시작 시

이번 주차는 PintOS Project 3 - Virtual Memory를 2주 동안 진행합니다.
달성률 산정을 위해 목표는 수치화 가능한 형태로 작성합니다.

---

## 1. 문제해결
- **목표:** Virtual Memory 핵심 주제 12개(`Virtual Memory`, `Page Table`, `Supplemental Page Table`, `Page Fault`, `Lazy Loading`, `Stack Growth`, `Frame Table`, `Page Replacement`, `Swap In/Out`, `mmap/munmap`, `Anonymous/File-backed Page`, `Project 2 Regression`)를 학습하고 Project 3 구현 흐름에 적용
- **달성률 산식:** (학습 후 적용한 핵심 주제 수 / 12) × 100


## 2. 설계
- **목표:** Project 3 구현 전 **SPT entry 구조, page type별 lifecycle, lazy load initializer, page fault 분기 조건, stack growth 조건, frame table 소유권, eviction victim 정책, swap slot 관리, mmap/munmap cleanup, fork/exit VM 자원 처리, 테스트 전략** 등 설계 포인트 **11개 이상**을 이슈·PR·발표 메모에 명시
- **달성률 산식:** (문서화한 설계 포인트 수 / 11) × 100


## 3. 구현
- **목표:** Project 3 핵심 구현 8개(`supplemental page table`, `page fault handling`, `lazy loading`, `stack growth`, `anonymous page/swap`, `file-backed page`, `mmap/munmap`, `fork/exit VM cleanup`) 완료
- **달성률 산식:** (완료한 구현 항목 수 / 8) × 100


## 4. 품질
- **목표:** CSV 기준 필수·회귀 테스트 140개(Project 3 핵심 46개 + Project 2/User Programs 회귀 63개 + Filesys 회귀 13개 + Threads 회귀 18개) 통과, Extra `cow-simple`은 별도 확인
- **달성률 산식:** (통과한 필수·회귀 테스트 수 / 140) × 100


## 5. 유지보수
- **목표:** 2주 기간 중 **14일 중 10일 이상** 팀 repository 개인 branch에 의미 있는 commit을 남기고, PR 또는 리뷰 기록으로 구현·테스트 변경 사항을 추적
- **달성률 산식:** (commit 또는 PR 기록이 있는 일수 / 10) × 100 (100% 초과 시 100%로 계산)


## 6. 협업
- **목표:** 팀 GitHub Projects 준비(1개) + 팀 repository 준비(1개) + 개인 branch 작업(1개) + 코어타임 PR 리뷰(1개) + 테스트 실패 공유 로그(1개) + 주간 공유 발표 자료 통합(1개) = **6개 완료**
- **달성률 산식:** (완료한 협업 항목 수 / 6) × 100


## 7. 태도
- **목표:** 공통 6개(목표 수립, 달성률 평가, AI 원칙, 업무 검토, WIL, 팀 협업 룰) 중 **6개 완료**
- **달성률 산식:** (완료한 공통 과제 수 / 6) × 100


## 8. 비즈니스 이해
- **목표:** Virtual Memory 구현이 운영체제의 **메모리 효율, 지연 적재, 프로세스 격리, page fault 복구, 파일 매핑, swap 기반 자원 확장**과 어떤 관련이 있는지 WIL 또는 발표 자료에 **6문장 이상** 명시
- **달성률 산식:** (명시한 문장 수 / 6) × 100


## 9. AI 활용
- **목표:** AI 활용 원칙 수립(1개) + Project 3 개념 설명 검증(1개) + 테스트 실패 분석 또는 코드 리뷰 보조 활용 기록(1개) + AI 제안 코드/설계의 직접 설명 검증 기록(1개) = **4개 완료**
- **달성률 산식:** (완료한 AI 활용 항목 수 / 4) × 100


## 10. 학습 민첩성
- **목표:** `vm_try_handle_fault`, `struct page`, `supplemental_page_table`, `uninit page`, `anon page`, `file-backed page`, `frame table`, `eviction`, `swap slot`, `mmap`, `munmap`, `stack growth`, `dirty bit`, `fork page copy`를 포함해 **14개 이상 개념/구현 포인트**를 학습 후 적용
- **달성률 산식:** (학습 후 적용 완료한 개념/구현 포인트 수 / 14) × 100


---

### 달성률 산식 참고
- 문제해결: Virtual Memory 핵심 개념과 구현 흐름 단위로 카운트
- 설계: 이슈, PR 설명, 발표 메모에 남긴 설계 포인트 수 기준
- 구현: Project 3 핵심 기능 단위로 카운트
- 품질: `week11-12_issues_complete.csv`의 필수·회귀 테스트 140개 기준, `cow-simple`은 Extra로 별도 기록
- 유지보수: 일 단위 commit 또는 PR 기록 기준
- 협업/태도/AI 활용: 완료 항목 수 기준
- 비즈니스 이해: WIL 또는 발표 자료에 명시한 문장 수 기준
