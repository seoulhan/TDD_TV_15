# 009 TVController 리팩토링 계획 작업 보고서

## 작업 개요

- **역할**: 모던 C++ 리팩토링 코치 관점의 단계별 실행 계획 수립
- **제약**: `Tuner.h` 수정 금지, 각 단계 후 **테스트 Green** 유지
- **기준선**: `ctest` 35/35 Passed (2026-05-19, `build` 디렉터리)
- **참조**: `include/TVController.h`, `docs/code_quality_report.md`, `docs/requirements_analysis.md`

## 생성·갱신 파일

| 경로 | 설명 |
|------|------|
| `Report\009_tv_controller_refactoring_plan_report.md` | 본 보고서 |
| `Prompting\009_tv_controller_refactoring_plan.md` | 프롬프트 대화 transcript |

---

## 현재 구조 요약

| 영역 | 현재 구현 | 리팩토링 포인트 |
|------|-----------|-----------------|
| `pushButton` | 7분기 if-else, 숫자 분기에서 직접 `applyChannel` | 디스패치·숫자 입력 분리 |
| `inputBuffer_` | `int`, `-1` = empty | Enum/optional 또는 소형 FSM |
| 네비게이션 | `pressUp`/`pressDown`에 empty 분기 + `upper_bound`/`lower_bound` | `SortedChannelList` 캡슐화 |
| `applyChannel` | 범위 검사 후 `setCH` | 방어 로직·튜너 예외 전파 정책 |
| 선호 채널 | `isFavorite` + `remove`+`erase` + `sort` | repository 헬퍼 |

---

## 공통 검증 명령 (모든 단계 공통)

```powershell
cd c:\DEV\TDD_TV_15\build
cmake --build .
ctest --output-on-failure
```

**통과 기준**: 35 tests, 0 failed (`TVControllerTest` 18 + `TVControllerApprovalTest` 4 + `TunerTest` 13).

선택(회귀 방지): ApprovalTests golden 파일 변경 없음 확인.

---

## Phase 0 — 준비 (커밋 없음)

- [ ] `git status`로 `Tuner.h` 미변경 확인
- [ ] 기준선 `ctest` Green 기록 (스크린샷 또는 CI 로그)
- [ ] 리팩토링 브랜치 생성 (예: `refactor/tv-controller-incremental`)

**검증**: 위 `cmake --build && ctest` 한 번 실행, 35/35.

---

## Phase 1 — `pushButton` 1차 분해 (디스패치만)

**목표**: 동작 변경 없이 가독성·커밋 단위 확보. 숫자/OK 로직은 아직 인라인 유지 가능.

### Commit 1.1 — `isDigitKey` / `keyToDigit` 복원

- [ ] 주석 처리된 `keyToDigit`를 `static` private 또는 무명 네임스페이스 `detail`로 복원
- [ ] `static_cast<int>(key)` 범위 판별을 `isDigitKey(remoteKey)`로 대체

```cpp
// 예시 (TVController.h private)
static bool isDigitKey(remoteKey key) noexcept;
static int keyToDigit(remoteKey key); // non-digit → -1
```

**검증**: `ctest` — S1 시나리오(14~18, 32, 35) 통과.

### Commit 1.2 — `handleDigitKey` / `handleOkKey` 추출

- [ ] `pushButton` 숫자 분기 → `handleDigitKey(int digit)`
- [ ] `KEY_OK` 분기 → `handleOkKey()`
- [ ] `pushButton`은 분류 + 위임만 남김

**검증**: `FourDigits_ChangesTwice`, `ThreeDigits_BufferThenOkOrInvalidate`, Approval `GoldenMaster_DigitChannelFlow`.

### Commit 1.3 — 비숫자 키 위임 정리

- [ ] `dispatchNonDigitKey(remoteKey key)` — UP/DOWN/SEARCH/FAV/NEXT_FAV/OTHER → 기존 `press*` 호출
- [ ] `pushButton` 최종 형태 목표:

```cpp
void pushButton(remoteKey key) {
  if (isDigitKey(key))
    handleDigitKey(keyToDigit(key));
  else
    dispatchNonDigitKey(key);
}
```

**검증**: 전체 35 tests.

---

## Phase 2 — 입력 버퍼 상태 (Enum 기반 → 선택적 State)

**목표**: `inputBuffer_` + `-1` 센티널을 명시적 상태로 표현. **Tuner.h 미수정.**

### Commit 2.1 — `ChannelInputBuffer` 값 객체 (Enum)

- [ ] `enum class BufferPhase { Empty, OneDigit }` 또는 `std::optional<int>` 래퍼 클래스
- [ ] 멤버 `ChannelInputBuffer inputBuffer_;` (또는 동일 역할 struct)
- [ ] API: `bool empty()`, `void clear()`, `void storeFirst(int d)`, `int value() const`
- [ ] 기존 `inputBuffer_` 직접 접근 제거

**상태 전이 (요구사항 동일)**:

| From | Event | To | Side effect |
|------|-------|-----|-------------|
| Empty | digit | OneDigit(d) | — |
| OneDigit(d) | digit | Empty | `applyChannel(10*d+d2)` |
| OneDigit(d) | OK | Empty | `applyChannel(d)` |
| Any | OTHER/UP/… | Empty | `pressOther` 동작 |

**검증**: S1 전체 + `GoldenMaster_BufferInvalidate`.

### Commit 2.2 (선택) — State 패턴 경량 적용

- Phase 2.1이 충분하면 **생략 가능**. FSM이 커질 때만:
- [ ] `struct IInputState { virtual void onDigit(...); virtual void onOk(...); }`
- [ ] `EmptyInputState`, `OneDigitInputState`
- [ ] `std::unique_ptr<IInputState>` 또는 `std::variant` (C++17)

**검증**: 동일 35 tests. Approval diff 없음.

---

## Phase 3 — 정렬 목록 + STL 알고리즘 캡슐화

**목표**: `upper_bound`/`lower_bound`/`find`/`sort` 불변식을 한곳에 모음.

### Commit 3.1 — `SortedChannelList` 헬퍼 (header-only 또는 동일 파일 하단)

- [ ] `class SortedChannelList` — `std::vector<int>` 래핑
- [ ] `insertUnique(int ch)`, `remove(int ch)`, `contains(int ch)`
- [ ] `int nextAfter(int cur) const` — `upper_bound`, wrap → front
- [ ] `int prevBefore(int cur) const` — `lower_bound` + wrap → back

**검증**: `FavoriteToggle*`, `NextFavorite_*`, `UpDownWithSearch_*`.

### Commit 3.2 — `favorites_` / `scannedChannels_` 타입 교체

- [ ] `std::vector<int> favorites_` → `SortedChannelList favorites_`
- [ ] `scannedChannels_` 동일 (검색 시 `push`+`sort` → `insertUnique` 또는 `addSorted`)
- [ ] `pressUp`/`pressDown`/`pressNextFavorite`/`isFavorite`가 헬퍼 호출만 하도록 축소

**검증**: `ChannelSearch_CollectsAvailableSorted`, S6 업/다운, Approval `GoldenMaster_SearchAndUpDown`.

### Commit 3.3 (선택) — `pressSearch` 종료 조건 개선

- [ ] 중복 검사 `std::find` → `contains` (O(n) 동일, 의도 명확화)
- [ ] (성능) `std::unordered_set` 보조 — **동작 동일 시에만**, FakeTuner 시퀀스 회귀 주의

**검증**: `ChannelSearch_*` 2건 + Approval search golden.

---

## Phase 4 — `applyChannel` 방어 로직 강화

**목표**: 도메인 검증 게이트 단일화. **Tuner는 그대로** — Controller에서 선제 검증.

### Commit 4.1 — 검증·튜너 호출 분리

- [ ] `static void validateChannel(int ch)` — `<0 || >99` → `invalid_argument`
- [ ] `applyChannel` = validate + `tuner->setCH(std::to_string(ch))`
- [ ] `nullptr` tuner 방어: 문서화된 정책(테스트는 항상 유효 포인터) — debug assert 또는 early throw `std::logic_error` (기존 테스트에 영향 없도록 **릴리스에서는 no-op assert만** 권장)

### Commit 4.2 — 튜너 예외 전파 정책 (선택, 테스트 추가 시)

- [ ] `FakeTuner`/`Tuner`가 던지는 `invalid_argument`를 그대로 전파 vs 래핑 — **기존 `InvalidChannelThrows` 유지**
- [ ] `stoi(tuner->getCurrentCH())` 실패 방어: `try/catch` → `logic_error` 또는 기본값 정책 (**새 테스트 없으면 기존 Fake 경로만 안전하게**)

**검증**: `InvalidChannelThrows`, TunerTest 13건, 변경 없으면 TVControllerTest 전체.

---

## Phase 5 — `pushButton` 고급 디스패치 (선택, OCP)

**우선순위 낮음** — Phase 1~4 완료 후.

- [ ] `std::unordered_map<remoteKey, void (TVController::*)()>` 또는 Command 팩토리
- [ ] 숫자 키는 맵에 넣지 않고 `isDigitKey` 유지 (10개 enum 중복 방지)

**검증**: 35 tests + Approval 4.

---

## 커밋 단위 요약 (권장 순서)

| # | 커밋 메시지 (예) | 범위 |
|---|------------------|------|
| 1 | `refactor: add isDigitKey and keyToDigit helpers` | Phase 1.1 |
| 2 | `refactor: extract handleDigitKey and handleOkKey` | Phase 1.2 |
| 3 | `refactor: extract dispatchNonDigitKey from pushButton` | Phase 1.3 |
| 4 | `refactor: introduce ChannelInputBuffer value object` | Phase 2.1 |
| 5 | `refactor: add SortedChannelList for favorites and scan` | Phase 3.1–3.2 |
| 6 | `refactor: strengthen applyChannel validation` | Phase 4.1 |
| 7 | `refactor: optional input state pattern` | Phase 2.2 (선택) |
| 8 | `refactor: optional key dispatch map` | Phase 5 (선택) |

---

## 리스크·주의사항

1. **한 커밋에 Phase 2+3 동시 적용 금지** — 회귀 시 원인 분리 어려움.
2. **`pressOther` 호출 경로** — `dispatchNonDigitKey`의 default 분기가 `inputBuffer_.clear()`와 동일해야 S1-4c/d 유지.
3. **ApprovalTests** — 리팩토링 후에도 `.approved.txt` diff 없어야 함 (출력 문자열 동일).
4. **`Tuner.h`** — 시그니처·가상 함수 변경 없음. `TVController.h`만 수정.
5. **public API** — `pressFavorite` 등 public 메서드는 테스트가 직접 호출; 시그니처 유지 또는 deprecate는 별 스프린트.

---

## 완료 정의 (Definition of Done)

- [ ] `pushButton` 15줄 이하 수준의 얇은 조율자
- [ ] 입력 버퍼: Enum/`optional` 또는 State로 `-1` 센티널 제거
- [ ] `upper_bound`/`lower_bound`가 `SortedChannelList`에만 존재
- [ ] `applyChannel`이 유일한 채널 범위 검증 게이트
- [ ] `ctest` 35/35 Green, Approval golden unchanged
- [ ] `Tuner.h` git diff empty

---

## 다음 액션 (구현 착수 시)

1. Phase 0 검증 후 Commit 1.1부터 TDD 없이 **Extract Method** (기존 테스트가 안전망).
2. 각 커밋마다 `cmake --build . ; ctest --output-on-failure`.
3. Phase 4.2는 **Red 테스트 추가 후** 진행 여부 결정 (`stoi` 실패 경로는 FakeTuner에 없음).

---

*본 문서는 계획서이며, 소스 코드 리팩토링 구현은 포함하지 않습니다.*
