# TDD_TV QA 종합 보고서 (Final)

| 항목 | 내용 |
|------|------|
| **문서 버전** | 1.0 |
| **작성일** | 2026-05-19 |
| **관점** | QA 리드 엔지니어 |
| **범위** | `TVController` TDD·리팩토링·결함 관리·커버리지·Test Double·Cursor AI 활용 |
| **기준 산출물** | [requirements_analysis.md](requirements_analysis.md), [test_plan.md](test_plan.md), [defect_report.md](defect_report.md), [code_quality_report.md](code_quality_report.md), [defect_list.md](defect_list.md) |

---

## Executive Summary

TDD_TV C++ 프로젝트는 README §1~§6 핵심 요구를 **FakeTuner 기반 단위 테스트 18건**과 **Approval Golden Master 4건**으로 고정한 뒤, Up/Down §5·§6 분기 결함(DEF-001/002)을 TDD로 수정하여 **ctest 35/35(100%)** 를 달성했다. `include/TVController.h` 라인 커버리지는 lcov `*/include/*` 추출 시 **100%(81/81)** 로 `test_plan` 목표(≥90%)를 충족한다. 결함은 **Up/Down·모드 분기(Wrap-around)** 에 집중되었고, FakeTuner가 2건의 Major를 조기 검출했다. Mock 검색 시퀀스(S4-3)·일부 P1 경계는 미구현이며, 리팩토링은 계획(009) 단계로 코드 품질 개선은 문서화·테스트 안전망 확보에 머물렀다.

---

## 1. 테스트 완료율·커버리지 (목표 대비)

### 1.1 테스트 통과율 (QM-01)

| 지표 | 목표 | 실측 (2026-05-19) | 판정 |
|------|------|---------------------|------|
| `ctest` 통과율 | P0 **100%** | **35/35 (100%)** | ✅ |
| `TVControllerTest` | README §1~§6 각 1건 이상 | **18/18 Green** | ✅ |
| `TVControllerApprovalTest` | 회귀 스냅샷 | **4/4 Green** | ✅ |
| `TunerTest` | Tuner 계약 | **13/13 Green** | ✅ |

```text
Test project C:/DEV/TDD_TV_15/build
100% tests passed, 0 tests failed out of 35
```

### 1.2 테스트 계획(test_plan §2.2) 시나리오 완료율

`test_plan.md` 작성 시점(초기) 대비 **현재 `TVControllerTest.cpp` 구현 상태**를 재평가했다.

| 구분 | 계획 ID 수(대표) | 구현·Green | 미구현·갭 | 완료율 |
|------|------------------|------------|-----------|--------|
| **P0 (핵심)** | 22 | **22** | 0 | **100%** |
| **P1** | 8 | 5 | S3-2, S4-3(Mock), S6-3, E-2 | **63%** |
| **P2** | 1 | 0 | E-3 | 0% |
| **전체 시나리오 ID** | ~31 | **25** | 6 | **~81%** |

**P0에서 닫힌 주요 갭** (계획서 ❌ → 현재 ✅):

- S1-3 `FourDigits_ChangesTwice`, S1-4b 버퍼+OK, S1-5 `ZeroThenSeven`
- S3-4 `NextFavorite_EmptyList_NoOp`
- S5-1~3 일반 Up/Down·Wrap(99↔0)
- S6-1~2 검색 목록 Up/Down·목록 외 wrap (`useChannels` Fixture)

**잔여 갭 (P1/P2)**:

| ID | 내용 | 비고 |
|----|------|------|
| S1-6 | `1`+OK 단독 시나리오 | `SingleDigitThenOk`와 중복 가능 |
| S3-2 | cur=4 → 다음 선호 12 | `NextFavorite_MovesToSmallestGreater`와 부분 중복 |
| S4-3 | Mock `setCH("0")`→seek→복원 시퀀스 | T3 단계 미구현 |
| S6-3 | scanned 단일 원소 wrap | |
| E-2 | `9`,`9` → 채널 99 | `applyChannel(99)` 직접 또는 숫자 시퀀스 |
| E-3 | 두 자리 조합 100 불가 패턴 | P2 |

### 1.3 lcov 커버리지 (QM-02, QM-03)

| 대상 | 목표 (`test_plan` §6) | 측정 방법 | 실측 | 판정 |
|------|----------------------|-----------|------|------|
| **`TVController.h` Line** | **≥ 90%** | `lcov` capture 후 `*/include/*` extract | **100% (81/81)** | ✅ |
| **`TVController.h` Function** | — | 동일 | **100% (12/12)** | ✅ |
| **Branch** | ≥ 85% | gcov branch | **미수집** (lcov `no data found`) | ⚠️ N/A |
| **`gen_lcov2.bat` 기본 extract** | — | `*/src/*`, `*/test/*` 만 | Line **39.0%** (1971/5056) | ⚠️ **TVController 미포함** |

**측정 파이프라인 (권장·실측에 사용)**

```bat
cmake --build build
ctest --test-dir build --output-on-failure
lcov --capture --directory build --base-directory . --gcov-tool gcov --output-file lcov_full.info
lcov --extract lcov_full.info "*TVController*" "*include/TVController*" --output-file lcov_tv.info
lcov --summary lcov_tv.info
```

**핵심 해석**

1. 헤더 온리(`TVController.h`) 구현은 **컴파일 단위가 테스트 타깃에 포함**되므로, `gen_lcov2.bat`에 `"*/include/*"` extract가 없으면 **프로덕션 로직 커버리지가 0%로 보이는 측정 오류**가 발생한다.
2. 현재 테스트 스위트로 `TVController` **모든 실행 가능 라인이 1회 이상 실행**되었음(100%). 다만 **분기 커버리지**는 도구·플래그 미설정으로 목표 85% 검증은 불가.
3. `FakeTuner.h` 93.8%, `RecordingTuner.h` 88.9% — Test Double 자체도 대부분 실행됨.

### 1.4 Exit Criteria 체크리스트 (`test_plan` §8)

| 기준 | 상태 |
|------|------|
| P0 TEST_F 전부 Green | ✅ |
| README §1~§6 각 최소 1 TEST_F | ✅ |
| 경계 §3 주요 ID 매핑 | ✅ (0/99 wrap, 빈 선호, invalid ch) |
| `TVController` line ≥ 90% | ✅ (100%, include extract) |
| `UpDownWithSearchResults` 스켈레톤 제거 | ✅ (`UpDownWithSearch_*`) |
| X-OTH-02 요구 확정 | ⏳ DEF-003 Open |

---

## 2. 결함 패턴 분석

### 2.1 Feature × Severity 분포

| Feature | Critical | Major | Minor | Info | 합계 |
|---------|----------|-------|-------|------|------|
| **NumberInput** | 0 | 0 | 0 | 1 Open | 1 |
| **Favorite** | 0 | 0 | 0 | 0 | 0 |
| **Search** | 0 | 0 | 0 | 0 | 0 |
| **UpDown** | 0 | 2 Fixed | 0 | 0 | 2 |
| **합계** | 0 | 2 | 0 | 1 | **3** |

### 2.2 논리 오류 유형별 집계

| 결함 유형 | 건수 | 대표 ID | 설명 |
|-----------|------|---------|------|
| **상태·모드 분기 오류** | 2 | DEF-001, 002 | `scannedChannels_.empty()` false인데 §5(±1, `%100`) 적용 |
| **Wrap-around / 목록 탐색** | 2 | (동일) | `upper_bound`/`lower_bound` 미사용 → 6→**5**, 6→**7** (기대 4, 14) |
| **숫자 버퍼·입력 FSM** | 0 (Major) | DEF-003 Info | UP/DOWN 시 버퍼 유지 여부 **요구 미확정** |
| **선호 채널 정렬·토글** | 0 | — | 테스트로 방어됨 |
| **검색 루프·복원** | 0 | — | Fake 기반 S4 Green |

**가장 많은 논리 오류가 발생한 기능: Up/Down (검색 목록 모드, §6)**

- 원인 패턴: **단일 `if (scannedChannels_.empty())` 분기**에서 else 블록이 §5 로직을 복제하지 않고 **동일 코드를 재사용**한 구현 실수(또는 분기 누락).
- Wrap-around는 §5에서는 `% 100`·0↔99, §6에서는 **정렬된 벡터 front/back** — **서로 다른 도메인 규칙**이므로 테스트 없이는 혼동하기 쉬움.

### 2.3 발견 단계별 분포 (QM-04)

| 단계 | Test Double | 신규 DEF | 비율 |
|------|-------------|----------|------|
| T2 Controller 기능 | **FakeTuner** | 2 (Major) | **67%** |
| T5 요구·분석 | N/A | 1 (Info) | **33%** |
| T1 Tuner Mock | MockTuner | 0 | 0% |
| T3 Mock 시퀀스 | — | 0 | 0% |
| T4 Approval | RecordingTuner | 0 | 0% |

### 2.4 결함–테스트 추적

| ID | 검증 테스트 | 회귀 방지 |
|----|-------------|-----------|
| DEF-001 | `UpDownWithSearch_OnListChannel` (DOWN) | ✅ |
| DEF-002 | (동일, UP) | ✅ |
| DEF-003 | 미작성 | 요구 확정 후 추가 필요 |

---

## 3. Test Double·Approval Test의 리팩토링 긍정 효과

### 3.1 FakeTuner (상태 기반)

| 효과 | 근거 |
|------|------|
| **§5/§6 결과 채널 assert** | `getCurrentCH()`로 DEF-001/002를 **한 번의 ctest**로 재현 |
| **Fixture 재사용** | `useChannels({4,6,14})`로 README §6 데이터만 교체 |
| **리팩토링 안전망** | `pressUp`/`pressDown` 분기 수정 후 18개 TVController 테스트로 즉시 회귀 확인 |
| **격리** | Tuner 하드웨어 없이 Controller **비즈니스 분기**만 검증 (Major 2건 모두 T2에서 발견) |

**한계**: `setCH` 호출 **순서·횟수**는 검증 약함 → 검색 계약은 Mock 또는 Approval trace 보완.

### 3.2 MockTuner (행위 기반, `TunerTest.cpp`)

| 효과 | 근거 |
|------|------|
| **Tuner 경계 조기 차단** | 유효/무효 `setCH` 파라미터화 — Controller까지 결함 전파 방지 |
| **Controller 결함과 분리** | T1에서 0건 — Controller 버그와 Tuner API 버그 구분 |

**갭**: S4-3 검색 시퀀스 Mock 미구현 → 검색 **호출 계약** 결함은 아직 Fake·Approval에 의존.

### 3.3 Approval Test + RecordingTuner

| 효과 | 근거 |
|------|------|
| **Golden Master 회귀** | 시나리오별 `currentCH`, `favorites`, `scanned`, **Tuner trace** 일괄 스냅샷 |
| **리팩토링 시 부수 효과 탐지** | `pushButton` 디스패치·내부 호출 순서 변경 시 `.approved.txt` diff |
| **문서화된 시나리오** | Digit / Favorite / Search+UpDown / BufferInvalidate 4 시나리오 |

**정성 평가**: Extract Class·Command 패턴 등 **구조 리팩토링(009 계획)** 시, Fake만으로는 놓칠 수 있는 **호출 순서 변경**을 Approval이 보완한다. 본 저장소에서는 **리팩토링 구현 전** 단계이나, **테스트 자산으로 이미 가치 확보**.

### 3.4 Test Double 선택 요약

```text
Fake   → P0 기능·상태·Wrap 결과 (일상 TDD)
Mock   → Tuner 계약·(계획) 검색 시퀀스
Approval → 릴리스·대규모 리팩토링 회귀
```

---

## 4. 다음 임베디드/컨트롤러 TDD Best Practice 5가지

### BP-1. 모드 분기는 테스트 이름에 명시한다

`UpDownWithoutSearch_*` vs `UpDownWithSearch_*`처럼 **전제 상태(empty vs non-empty)** 를 TEST_F 이름에 넣는다. §5/§6 혼동형 결함(DEF-001/002)을 설계 단계에서 분리한다.

### BP-2. 헤더 온리 프로덕션 코드는 lcov extract에 `include`를 포함한다

측정 오류 없이 **QM-02 ≥90%** 를 운영하려면 `gen_lcov2.bat`에 `"*/include/*"` 를 필수 추가하고, PR 게이트는 **TVController 단일 파일** summary를 파싱한다.

### BP-3. Fake로 상태·Mock으로 계약·Approval으로 회귀 삼층 구조

| 층 | 도구 | 목적 |
|----|------|------|
| 1 | Fake | 채널·목록·wrap **결과** |
| 2 | Mock | `seekCH`/`setCH` **순서** |
| 3 | Approval | 다단계 시나리오 **스냅샷** |

임베디드에서는 하드웨어 대신 **Fake HAL + Mock DMA 시퀀스**로 동일 패턴 적용 가능.

### BP-4. Wrap-around·경계는 표로 먼저 고정하고 TEST_F 1:1 매핑

0/99, 목록 front/back, `upper_bound` strict greater 등 **경계 ID(B-CH-*, B-SCN-*)** 를 요구 문서에 두고, 미매핑 ID는 결함이 아닌 **테스트 부채**로 추적한다.

### BP-5. 리팩토링은 “Green 유지 + 한 Phase 한 커밋” (009 계획 준수)

`Tuner` 인터페이스 고정, `ChannelInputBuffer`/`SortedChannelList` 등 **Extract Class**는 ctest 35/35·Approval diff 무변을 각 단계 게이트로 삼는다. SRP 분해는 **테스트가 없으면 하지 않는다**.

---

## 5. Cursor AI 활용 효과 (정량·정성)

### 5.1 정량 요약 (프로젝트 산출물 기준)

| 활동 | 산출 문서·코드 | Report # | 추정 효과 |
|------|----------------|----------|-----------|
| 요구·시나리오 분해 | `requirements_analysis.md` (34 TEST 시나리오) | 002 | 수동 명세 **1~2일 → 수시간** |
| 테스트 계획 | `test_plan.md` (P0/P1, 경계 ID, lcov) | 004 | TEST_F 매트릭스·우선순위 즉시 합의 |
| TEST_F 구현 | `TVControllerTest.cpp` 18건 | 005 | README §1~§6 **최소 1~2건/기능** 일괄 생성 |
| 결함 분석·수정 | DEF-001/002, `006` debug report | 006 | 실패 로그→원인 줄번호→패치 **1 세션** |
| Approval 인프라 | `RecordingTuner`, 4 Golden | 008 | 보일러플레이트·스냅샷 포맷 자동화 |
| 리팩토링 로드맵 | `009` plan (Phase 0~5) | 009 | SRP/OCP 개선 **단계·검증 명령** 사전 정의 |
| 결함·메트릭 체계 | `defect_report.md`, `defect_list.md` | 007, 010 | Severity×Feature·QM-01~07 |

| 메트릭 | Before (추정) | After | 비고 |
|--------|---------------|-------|------|
| TVController TEST_F | ~7 (초기) | **18** | 005 작업 |
| ctest 총계 | 30 실패 이력 | **35/35** | §6 수정 포함 |
| 문서화 산출 | README only | **docs 6종 + Report 10+** | 추적성 |
| DEF Major Open | 2 | **0** | Fake 회귀 |

### 5.2 정성 요약

| 영역 | Cursor AI 기여 | 한계·주의 |
|------|----------------|-----------|
| **시간 단축** | Given-When-Then 테스트·Fixture 헬퍼·문서 템플릿 반복 생성 | 빌드·lcov는 로컬 검증 필수 |
| **경계값 도출** | B-CH/B-SCN/B-FAV ID, X-OTH-02, 0/7·99/0·목록 외 cur=15 | DEF-003처럼 **요구 미확정**은 사람 판단 필요 |
| **코드 스멜 개선** | `code_quality_report` Command/SRP/Navigator 로드맵 | **실제 분리 리팩토링은 미착수** — 계획만 존재 |
| **결함 격리** | Fake vs Mock 역할 표, DEF↔TEST 매핑 | Mock S4-3 미구현 갭은 AI가 “계획”으로 남김 |
| **지식 전달** | Report/Prompting transcript로 **재현 가능한 프롬프트 이력** | transcript는 대화 요약본, 공식 감사 로그 아님 |

### 5.3 QA 리드 관점 총평

Cursor AI는 **명세→계획→테스트→결함 문서** 파이프라인을 압축했고, 특히 **§6 Up/Down 실패 → 테스트 고정 → 수정** 루프에서 TDD 이득이 크다. 반면 **lcov extract 설정·Branch 커버리지·Mock 계약**은 도구가 제안만 하고 **실행 검증은 QA가 수행**해야 했다. 다음 사이클에서는 BP-2·S4-3 Mock을 CI에 넣어 AI가 생성한 “계획 ❌” 항목을 자동으로 닫는 것이 좋다.

---

## 6. 리팩토링·코드 품질 현황

| 항목 | 상태 |
|------|------|
| `code_quality_report` SRP/OCP 이슈 | **문서화 완료**, 코드 분해 **미적용** |
| `009` 리팩토링 Phase | **계획만** (ChannelInputBuffer, Command 등) |
| 테스트 안전망 | **35 ctest + 4 Approval** — 리팩토링 착수 가능 수준 |
| `Tuner.h` | 변경 금지 정책 유지 |

---

## 7. 권장 후속 조치 (우선순위)

1. **`gen_lcov2.bat`** — `"*/include/*"` extract 추가, CI에서 `TVController.h` line summary 게이트.
2. **P1 갭** — S4-3 `MockTuner` 검색 시퀀스, S6-3 단일 목록 wrap, E-2 채널 99.
3. **DEF-003** — X-OTH-02 요구 확정 후 테스트 1건 + Severity 재분류.
4. **리팩토링 009 Phase 1** — `pushButton` 디스패치 분리, 매 Phase `ctest`+Approval 무변 확인.
5. **Branch coverage** — `--coverage` + lcov branch 활성화 시 QM-03 목표 검증.

---

## 8. 참조

| 자료 | 경로 |
|------|------|
| 구현 | `include/TVController.h`, `test/TVControllerTest.cpp`, `test/FakeTuner.h`, `test/ApprovalTest.cpp` |
| 결함 | `docs/defect_list.md`, `Report/006_tv_controller_debug_report.md` |
| 커버리지 | `gen_lcov2.bat`, `lcov_full.info` (include extract) |
| 작업 이력 | `Report/001`~`010`, `Prompting/*` |

---

*본 보고서는 2026-05-19 로컬 `ctest`·`lcov` 실측 및 저장소 문서·코드 정적 검토를 기준으로 작성되었다.*
