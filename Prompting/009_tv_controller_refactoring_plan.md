# 009 TV Controller Refactoring Plan Transcript

## User (1)

```text
@include/TVController.h @docs/code_quality_report.md @docs/requirements_analysis.md 

[Persona] 모던 C++ 리팩토링 코치입니다.
[Context] 제약: Tuner.h 수정 금지, 테스트 Green 상태에서만 진행
[Task] TVController 리팩토링 계획을 단계별로 제안해줘.
    - 거대한 pushButton() 내의 if-else/switch 문 축소 및 함수 추출 (커밋 단위로 쪼개기)
    - 입력 버퍼 상태(숫자 입력 중 vs 대기 중)에 따른 State 패턴 적용 또는 Enum 기반 매핑
    - std::vector 및 STL 알고리즘(upper_bound, lower_bound)을 활용한 로직 캡슐화
    - applyChannel() 내 예외 처리 방어 로직 강화
[Format] 단계별 체크리스트 + 각 단계 검증 방법 (cmake --build && ctest)
[Wrap-up] 
 - Report\ 디렉토리 아래에 작업 내용 보고서를 번호를 붙여 생성하고 저장
 - 현재까지의 프롬프트 대화 전체를 Prompting\ 디렉토리 아래에 Export Transcript 해줘. Transcript 파일명은 보고서의 이름에서 _report 접미사를 뺀 것으로 해줘.
```

## Assistant (1)

```text
분석 대상:
- include/TVController.h — pushButton 7분기, inputBuffer_(-1), favorites_/scannedChannels_, applyChannel
- docs/code_quality_report.md — SRP/OCP, Command/State/Strategy 우선순위
- docs/requirements_analysis.md — S1~S6 상태 전이, E1~E2 예외, Test Double

기준선 확인:
- build 디렉터리에서 ctest 실행 → 35/35 Passed (TVControllerTest 18, Approval 4, TunerTest 13)

제약 준수:
- Tuner.h 미수정
- 각 Phase/Commit 후 cmake --build && ctest Green 유지

산출물 번호: 기존 Report 008 이후 → 009
```

## Assistant (2) — 리팩토링 계획 요약 (본문)

### Phase 0 — 준비

- Tuner.h 미변경 확인, ctest 기준선 기록, refactor 브랜치 생성
- 검증: `cd build; cmake --build .; ctest --output-on-failure`

### Phase 1 — pushButton 분해 (3 commits)

1. **1.1** `isDigitKey` / `keyToDigit` — `static_cast<int>` 제거
2. **1.2** `handleDigitKey`, `handleOkKey` 추출
3. **1.3** `dispatchNonDigitKey` — UP/DOWN/SEARCH/FAV/OTHER 위임

목표 `pushButton`:

```cpp
void pushButton(remoteKey key) {
  if (isDigitKey(key))
    handleDigitKey(keyToDigit(key));
  else
    dispatchNonDigitKey(key);
}
```

### Phase 2 — 입력 버퍼 (Enum → 선택 State)

1. **2.1** `ChannelInputBuffer` + `BufferPhase{Empty, OneDigit}` 또는 `std::optional<int>`
2. **2.2 (선택)** `IInputState` / `EmptyInputState` / `OneDigitInputState`

상태 전이: requirements_analysis §1 숫자 입력 표와 동일 유지.

### Phase 3 — SortedChannelList + STL

1. **3.1** `nextAfter(cur)`, `prevBefore(cur)` — upper_bound/lower_bound wrap
2. **3.2** `favorites_`, `scannedChannels_` 래핑
3. **3.3 (선택)** pressSearch `contains` 명확화

### Phase 4 — applyChannel 방어

1. **4.1** `validateChannel(ch)` 분리, applyChannel 단순화
2. **4.2 (선택)** tuner nullptr / stoi 실패 — 새 테스트 있을 때만

### Phase 5 (선택) — key dispatch map / Command

- Phase 1~4 완료 후 OCP 확장

### 커밋 메시지 예시

1. refactor: add isDigitKey and keyToDigit helpers
2. refactor: extract handleDigitKey and handleOkKey
3. refactor: extract dispatchNonDigitKey from pushButton
4. refactor: introduce ChannelInputBuffer value object
5. refactor: add SortedChannelList for favorites and scan
6. refactor: strengthen applyChannel validation

### 리스크

- Phase 2+3 동시 커밋 금지
- pressOther → inputBuffer clear 경로 유지 (S1-4c/d)
- Approval golden diff 없음
- Tuner.h diff empty

### 검증 (모든 단계)

```powershell
cd c:\DEV\TDD_TV_15\build
cmake --build .
ctest --output-on-failure
```

통과: 35 tests, 0 failed.

## User (2)

```text
[Wrap-up 동일 요청 — Report/Prompting 저장]
```

## Assistant (3)

```text
Wrap-up 처리:
- Report\009_tv_controller_refactoring_plan_report.md — 단계별 체크리스트·검증·커밋 단위·리스크·DoD
- Prompting\009_tv_controller_refactoring_plan.md — 본 transcript

번호: Report 008 (approval golden master) 다음 009 사용.
기준선 ctest: 35/35 Green 확인됨.
```
