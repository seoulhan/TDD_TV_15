# 006 TVController ctest 결함 분석 보고서

## 1. 실패 로그 요약 (`LastTestsFailed.log` / ctest #30)

**실패 테스트:** `TVControllerTest.UpDownWithSearch_OnListChannel`  
**위치:** `test/TVControllerTest.cpp:239` (`ASSERT_EQ`)

```
Expected equality of these values:
  "4"
  tuner->getCurrentCH()
    Which is: "5"
```

| 단계 | 기대 | 실제 | 차이 |
|------|------|------|------|
| Given: 검색 완료 `{4,6,14}`, 현재 `6` | — | `6` | — |
| When: `KEY_DOWN` | `"4"` (목록 내 이전 채널) | `"5"` | **+1** (일반 채널 −1 로직이 적용됨) |
| When: `KEY_UP` (재현 시) | `"14"` | `"7"` | **+1** (`(cur+1)%100` 오적용 시) |

두 번째 실패(UP)는 `pressUp()`이 검색 목록 분기 없이 `(cur+1)%100`만 사용할 때 재현됩니다.

---

## 2. TVController 내부 결함 위치

| 항목 | 내용 |
|------|------|
| **파일** | `include/TVController.h` |
| **함수** | `pressDown()` (주 결함), 필요 시 `pressUp()` 대칭 점검 |
| **줄번호** | **106–116** (`pressDown`), **94–103** (`pressUp`) |
| **유형** | README §6 분기 누락 — `scannedChannels_`가 비어 있지 않을 때도 §5(일반 ±1) 로직 사용 |
| **근거** | `scannedChannels_.empty()`가 `false`인데 `cur - 1` → 6→5. README §6: 목록 `{4,6,14}`에서 6 시청 중 DOWN → **4** |

### 잘못된 패턴 (결함)

```cpp
void pressDown() {
  int cur = std::stoi(tuner->getCurrentCH());
  if (scannedChannels_.empty()) {
    applyChannel((cur == 0) ? 99 : cur - 1);
  } else {
    applyChannel((cur == 0) ? 99 : cur - 1);  // BUG: 검색 모드 미구현
  }
}
```

### 올바른 패턴 (현재 저장소 기준)

`lower_bound(cur)`로 현재 채널 이상 첫 위치를 찾은 뒤 `--it`으로 **목록 내 직전** 채널 선택, `begin`이면 wrap하여 목록 최댓값.

---

## 3. 결함 심각도

| 등급 | **Major** |
|------|-----------|
| **근거** | README §6 핵심 시나리오(검색 후 채널 업/다운) 미준수. 사용자가 검색한 채널 목록을 무시하고 0–99 연속 채널로 이동함. §5와 §6 동작이 혼동되어 회귀 테스트 `UpDownWithSearch_*` 전부 실패 가능. |
| **비고** | 크래시·데이터 손상은 없음(Critical 아님). 선호/숫자 버퍼 등 다른 기능은 독립. |

---

## 4. 최소 변경 수정 방안 (C++17)

`Tuner.h` / `FakeTuner.h` / 테스트 의도는 변경하지 않습니다. `TVController::pressDown`만 검색 목록 분기를 보완합니다.

```diff
--- a/include/TVController.h
+++ b/include/TVController.h
@@ -107,7 +107,12 @@ public:
     if (scannedChannels_.empty()) {
       applyChannel((cur == 0) ? 99 : cur - 1);
     } else {
-      applyChannel((cur == 0) ? 99 : cur - 1);
+      auto it = std::lower_bound(scannedChannels_.begin(),
+                                 scannedChannels_.end(), cur);
+      if (it == scannedChannels_.begin())
+        it = scannedChannels_.end();
+      applyChannel(*(--it));
     }
   }
```

`pressUp()`은 이미 `upper_bound` + wrap(`front`)로 §6를 만족합니다. 동일 클래스 내 대칭 유지만 확인하면 됩니다.

---

## 5. Green 확인 절차

```powershell
cd c:\DEV\TDD_TV_15\build
cmake --build . --target TVControllerTest
ctest --output-on-failure -R "TVControllerTest.UpDownWithSearch"
ctest --output-on-failure
```

**검증 결과 (2026-05-19):** 위 수정이 반영된 `include/TVController.h` 기준 **32/32 passed**.

---

## 6. 참고

- 실패는 `build/Testing/Temporary/LastTestsFailed.log`에 `30:TVControllerTest.UpDownWithSearch_OnListChannel`로 기록됨.
- 워크스페이스 HEAD의 `pressDown`은 이미 올바른 구현이며, 결함은 **검색 목록 분기 누락/오적용** 시 재현됩니다.
- `FakeTuner::seekCH` 및 검색 수집 로직은 본 실패와 무관함(검색·wrap-off-list 테스트는 Green).

## 산출물

| 경로 | 설명 |
|------|------|
| `Report\006_tv_controller_debug_report.md` | 본 보고서 |
| `Prompting\006_tv_controller_debug.md` | 대화 transcript |
