# TVController 결함 목록 (Defect List)

| 문서 버전 | 1.0 |
|-----------|-----|
| 최종 갱신 | 2026-05-19 |
| 기준 | `ctest` 실패 로그, `Report\006_tv_controller_debug_report.md`, `docs\test_plan.md` (X-OTH-02) |

**Severity 정의**: Critical(크래시·데이터 손상) · Major(핵심 요구 미준수) · Minor(부가 시나리오) · Info(분석·미검증 후보)

**상태**: Fixed = 수정 반영 후 회귀 Green · Open = 미해결 또는 요구 미확정

---

## DEF-001

| 필드 | 내용 |
|------|------|
| **ID** | DEF-001 |
| **Severity** | Major |
| **Feature** | UpDown |
| **Status** | Fixed |
| **Steps** | 1. `FakeTuner` 시청 가능 채널 `{4,6,14}` 설정<br>2. `KEY_SEARCH`로 검색 완료 (`scannedChannels_` 비어 있지 않음)<br>3. 현재 채널 `"6"` 설정<br>4. `KEY_DOWN` 입력 (`TVControllerTest.UpDownWithSearch_OnListChannel`) |
| **Expected** | README §6: 검색 목록 `{4,6,14}`에서 6 시청 중 DOWN → **4** (목록 내 직전 채널) |
| **Actual** | `"5"` (일반 §5 로직 `cur - 1` 적용) |
| **Root Cause** | `include/TVController.h` `pressDown()` (106–116행): `scannedChannels_.empty()`가 `false`인 `else` 분기에서도 `applyChannel((cur == 0) ? 99 : cur - 1)`만 호출 — §6 목록 탐색(`lower_bound` + wrap) 누락 |
| **Fix Summary** | 검색 목록 존재 시 `std::lower_bound`로 현재 채널 이상 첫 위치를 찾고 `--it`으로 직전 항목 선택; `begin`이면 `end()`로 wrap하여 목록 최댓값 적용. `ctest -R UpDownWithSearch` Green 확인 (32/32) |

**실패 로그 (재현 시)**:

```
test/TVControllerTest.cpp:239: Failure
Expected: "4"
Actual:   "5"
```

---

## DEF-002

| 필드 | 내용 |
|------|------|
| **ID** | DEF-002 |
| **Severity** | Major |
| **Feature** | UpDown |
| **Status** | Fixed |
| **Steps** | 1. DEF-001과 동일 Given (검색 `{4,6,14}`, 현재 `"6"`)<br>2. `KEY_UP` 입력 (동일 테스트 후반) |
| **Expected** | README §6: 6 시청 중 UP → **14** (목록에서 6보다 큰 최소 채널) |
| **Actual** | `"7"` (`(cur + 1) % 100` §5 로직 오적용 시) |
| **Root Cause** | `pressUp()`이 `scannedChannels_` 비어 있지 않을 때 `upper_bound` 분기 없이 `(cur + 1) % 100`만 사용하는 구현(대칭 결함). 현재 저장소는 94–103행에 올바른 분기 존재 |
| **Fix Summary** | `std::upper_bound`로 다음 목록 채널 선택, `end`이면 `scannedChannels_.front()`로 wrap. DEF-001 수정과 쌍으로 §5/§6 분기 일관성 유지 |

---

## DEF-003

| 필드 | 내용 |
|------|------|
| **ID** | DEF-003 |
| **Severity** | Info |
| **Feature** | NumberInput |
| **Status** | Open (요구·테스트 미확정) |
| **Steps** | 1. `4`, `5` 입력 → 채널 `"45"`<br>2. `6` 입력 → 버퍼만 채움 (채널 45 유지)<br>3. `KEY_UP` 또는 `KEY_DOWN` 입력 (`pushButton` 경유) |
| **Expected** | `docs\test_plan.md` X-OTH-02: OTHER와 동일하게 버퍼만 무효화할지 **요구 명세 확인 필요**. README §1은 `KEY_OTHER` 무효화만 명시 |
| **Actual** | `pushButton`이 UP/DOWN을 `pressUp`/`pressDown`으로 직접 라우팅 — `pressOther()` 미호출로 `inputBuffer_`가 유지될 수 있음 (채널은 Up/Down 규칙대로 변경) |
| **Root Cause** | `TVController::pushButton` (58–61행): 숫자 버퍼 클리어는 `pressOther()` 경로에만 존재 |
| **Fix Summary** | 요구 확정 후: (A) UP/DOWN/SEARCH/FAV 진입 시 `inputBuffer_ = -1` 또는 (B) 현 동작을 요구로 문서화하고 회귀 테스트 추가. **현재 자동화 실패 없음** — P1 후보 |

---

## 요약

| ID | Severity | Feature | Status | 검증 테스트 |
|----|----------|---------|--------|-------------|
| DEF-001 | Major | UpDown | Fixed | `UpDownWithSearch_OnListChannel` (DOWN) |
| DEF-002 | Major | UpDown | Fixed | `UpDownWithSearch_OnListChannel` (UP) |
| DEF-003 | Info | NumberInput | Open | 미작성 (X-OTH-02) |

## 참조

- `Report\006_tv_controller_debug_report.md` — DEF-001/002 분석·diff
- `Report\005_tv_controller_tests_report.md` — §6 회귀 테스트 추가
- `docs\test_plan.md` §4.1 X-OTH-02 — DEF-003 출처
- `build/Testing/Temporary/LastTestsFailed.log` — 과거 `30:TVControllerTest.UpDownWithSearch_OnListChannel` 기록
