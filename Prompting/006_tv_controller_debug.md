# 006 TVController Debug Transcript

## User (1)

```text
@test/TVControllerTest.cpp @include/TVController.h @test/FakeTuner.h 

[Persona] 디버깅과 결함 분석에 능한 C++ QA 엔지니어입니다.
[Task] (여기에 ctest 실패 로그를 붙여넣을 것)
    1) EXPECT_EQ 실패의 기대/실제 차이 요약
    2) TVController 내부 버그 위치 특정 (파일명:줄번호, 예: 버퍼 초기화 누락, wrap-around 논리 오류)
    3) 결함 심각도 (Critical/Major/Minor/Info) 분류 및 근거
    4) 최소 변경 수정 방안 (C++17 스타일)
       - 단, Tuner.h 인터페이스 및 테스트 대역의 본질적 목적 훼손 금지
[Format] 수정 diff 제안 + cmake --build build && ctest Green 확인 절차
[Wrap-up] 
 - Report\ 디렉토리 아래에 작업 내용 보고서를 번호를 붙여 생성하고 저장
 - 현재까지의 프롬프트 대화 전체를 Prompting\ 디렉토리 아래에 Export Transcript 해줘. Transcript 파일명은 보고서의 이름에서 _report 접미사를 뺀 것으로 해줘.
```

## Assistant (1)

### 조사

- `test/TVControllerTest.cpp`, `include/TVController.h`, `test/FakeTuner.h` 분석
- `build/Testing/Temporary/LastTestsFailed.log` → 실패: `TVControllerTest.UpDownWithSearch_OnListChannel`
- 사용자가 ctest 로그를 붙여넣지 않아, 결함 재현 후 GTest 출력 확보

### 결함 재현 (pressDown 검색 분기 누락 시)

```
C:\DEV\TDD_TV_15\test\TVControllerTest.cpp:239: Failure
Expected equality of these values:
  "4"
  tuner->getCurrentCH()
    Which is: "5"
```

### 분석 결론

1. **기대/실제:** DOWN 시 `"4"` 기대, `"5"` 실제 (6→5는 §5 로직)
2. **위치:** `TVController.h` `pressDown()` 106–116행 — `scannedChannels_` 비어 있지 않을 때 `lower_bound` + wrap 미적용
3. **심각도:** Major (README §6 미준수)
4. **수정:** `lower_bound` 후 `--it`, `begin`이면 `end()`로 wrap
5. **현재 저장소:** 이미 올바른 구현 반영됨 → 코드 변경 없이 `ctest` 32/32 Green

### 산출물

- `Report/006_tv_controller_debug_report.md`
- `Prompting/006_tv_controller_debug.md` (본 transcript)

### 검증 명령

```powershell
cd c:\DEV\TDD_TV_15\build
cmake --build . --target TVControllerTest
ctest --output-on-failure
```

결과: **32/32 passed**
