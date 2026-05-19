# 008 Approval Golden Master 작업 보고서

## 작업 개요

- `TVController::pushButton` 연속 호출에 따른 **Tuner API 호출 순서**와 **관측 가능한 상태**(현재 채널, favorites, scanned)를 Golden Master(`.approved.txt`)로 고정하는 회귀 테스트를 설계·구현했습니다.
- `std::cout` 캡처 인프라는 유지하되, 현재 `TVController`에는 로그가 없어 `(none)`으로 기록됩니다. 리팩토링 후 로그가 추가되면 동일 슬롯에서 회귀가 잡힙니다.
- Wrap-up: 본 보고서와 transcript `Prompting\008_approval_golden_master.md`.

## 생성·갱신 파일

| 경로 | 설명 |
|------|------|
| `test\RecordingTuner.h` | FakeTuner + Tuner 호출 trace (신규) |
| `test\ApprovalTest.cpp` | Golden Master 4건 (전면 재작성) |
| `test\ApprovalTest.*.approved.txt` | 기대 스냅샷 4개 (Git 커밋 대상) |
| `CMakeLists.txt` | `TVControllerApprovalTest` 타깃 분리 |
| `Report\008_approval_golden_master_report.md` | 본 보고서 |
| `Prompting\008_approval_golden_master.md` | 프롬프트 대화 transcript |

## 1) `.approved.txt` 생성·보관 전략

### 네이밍·위치

- ApprovalTests.cpp 기본 규칙: `test/ApprovalTest.<TestSuite>.<TestName>.approved.txt`
- 소스(`ApprovalTest.cpp`)와 **동일 디렉터리**에 두어 diff·리뷰가 쉽도록 함.
- `.received.txt`는 실패 시에만 생성 → **커밋하지 않음** (로컬/CI 아티팩트).

### 최초 생성(로컬)

```powershell
cd build
$env:APPROVAL_TESTS_USE_REPORTER = "AutoApprove"
..\test\..\..\build\TVControllerApprovalTest.exe   # 또는 ctest -R Approval
```

- `AutoApprove` / `AutoApproveIfMissing`로 `.received` → `.approved` 승인.
- 의도적 동작 변경 시: diff 확인 후 `.approved.txt`만 수동 갱신.

### CI·팀 운영

- `.approved.txt`는 **버전 관리에 포함** (Golden Master의 단일 진실 공급원).
- CI에서는 Reporter 미설정 → 불일치 시 **실패** (승인은 로컬/PR 리뷰에서만).
- PR에 `.approved.txt` diff가 있으면 “의도된 동작 변경”임을 리뷰어가 확인.

### 스냅샷 포맷(고정)

```
scenario: <이름>
--- cout ---
(none)|<로그>
--- state ---
currentCH: ...
favorites: [...]
scanned: [...]
--- tuner trace ---
setCH(...)
seekCH -> ...
getCurrentCH -> ...
```

## 2) pushButton 시나리오별 테스트

| TEST | 시나리오 | 검증 포인트 |
|------|----------|-------------|
| `GoldenMaster_DigitChannelFlow` | README §1 숫자·OK·연속 입력 | setCH 시퀀스 1→12→34→45→6→7 |
| `GoldenMaster_FavoriteAndNextFavorite` | §2·§3 선호·다음 선호 | favorites `[12,56]`, next wrap |
| `GoldenMaster_SearchAndUpDown` | §4·§6 검색 후 Up/Down | scanned `[4,6,14]`, seekCH 루프·wrap |
| `GoldenMaster_BufferInvalidate` | §1 456+OTHER+OK | 45 유지, 버퍼 무효화 |

- `RecordingTuner`가 `FakeTuner`와 동일한 채널 집합 의미를 유지하면서 **호출 순서**를 기록.
- `pushSequence()`로 리모컨 입력 시퀀스를 한눈에 표현.

## 3) CMake / ctest 통합

### CMake 변경 요약

- `TVControllerTest`: 단위 테스트만 (`gtest_main`).
- `TVControllerApprovalTest`: Approval 전용 (`APPROVALS_GOOGLETEST`가 main 제공).
- `gtest_discover_tests(TVControllerApprovalTest)` → ctest에 4개 테스트 자동 등록.

### 실행 방법

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build                          # 전체 35 tests
ctest --test-dir build -R TVControllerApproval  # Approval 4 tests only
```

- 검증 결과: **35/35 Passed** (Tuner 13 + TVController 18 + Approval 4).

## 4) 리팩토링 시 로그 제거·Approval 대체 전략

| 상황 | 권장 조치 |
|------|-----------|
| `std::cout`만 제거, Tuner/상태 동일 | **Approval 유지** — `--- cout ---`가 `(none)`으로 안정화되면 diff 없음 |
| 로그로만 검증하던 Approval | **폐기** 또는 **상태 블록 강화** 후 재승인 |
| 비즈니스 규칙 변경(의도적) | `.approved.txt` 갱신 + `TVControllerTest` EXPECT_EQ 병행 |
| 내부 구현만 변경(동작 동일) | Golden Master **Green 유지**가 목표 — 실패 시 회귀 |
| Approval이 과민(구현 세부) | trace에서 `getCurrentCH` 등 **과다 기록 줄이기** 또는 P0만 `TEST_F`로 이전 |

### 권장 이중 방어

1. **P0/P1**: `TVControllerTest` + FakeTuner로 규칙·경계값 명시 검증.
2. **P2**: Approval 4건으로 **통합 시나리오** 회귀(호출 순서 + 최종 상태).

로그를 `Presenter`/`Logger` 인터페이스로 분리할 경우: Approval은 Presenter 출력을 검증하고, Controller는 `RecordingTuner` + getter만 검증하도록 역할 분리.

## 검증

- `cmake --build build` 성공.
- `ctest --test-dir build` → 35/35 Passed.

## 후속 권장

- CI에 `ctest -R TVControllerApproval` 단계 추가.
- 새 README 시나리오마다 Approval 1건 추가 vs `TVControllerTest`만 추가 — P2는 Approval, 경계는 TEST_F.
