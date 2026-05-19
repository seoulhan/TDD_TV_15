# 007 Defect List Transcript

## User (1)

```text
[Persona] QA 리드입니다.
[Context] 현재까지 발견된 테스트 실패/결함들을 문서화합니다.
[Task] 발견된 결함을 다음 형식으로 defect_list.md에 정리해줘.
    항목: [ID] [Severity] [Feature] [Steps] [Expected] [Actual] [Root Cause] [Fix Summary]
    (Feature 예시: NumberInput, FavoriteToggle, NextFavorite, UpDown)
[Format] Markdown. docs\ 디렉토리 아래 defect_list.md로 저장
[Wrap-up] 
 - Report\ 디렉토리 아래에 작업 내용 보고서를 번호를 붙여 생성하고 저장
 - 현재까지의 프롬프트 대화 전체를 Prompting\ 디렉토리 아래에 Export Transcript 해줘. Transcript 파일명은 보고서의 이름에서 _report 접미사를 뺀 것으로 해줘.
```

## Assistant (1)

### 조사

- `Report\006_tv_controller_debug_report.md` — ctest #30 `UpDownWithSearch_OnListChannel` 실패, `pressDown` §6 분기 누락
- `build\Testing\Temporary\LastTestsFailed.log` — 과거 실패 기록
- `test\TVControllerTest.cpp`, `include\TVController.h` — 기대/실제 및 수정 위치
- `docs\test_plan.md` X-OTH-02 — 버퍼+UP/DOWN 후보 결함
- `ctest --output-on-failure` — 현재 32/32 Green

### 산출

1. **`docs\defect_list.md`**
   - **DEF-001** (Major, UpDown, Fixed): 검색 후 DOWN 시 6→5 대신 6→4 기대
   - **DEF-002** (Major, UpDown, Fixed): 검색 후 UP 대칭 결함(7 대신 14)
   - **DEF-003** (Info, NumberInput, Open): X-OTH-02 버퍼·UP/DOWN 상호작용 — 요구 미확정, 테스트 미실패

2. **`Report\007_defect_list_report.md`** — 작업 보고서

3. **`Prompting\007_defect_list.md`** — 본 transcript

### 검증

```powershell
cd c:\DEV\TDD_TV_15\build
ctest --output-on-failure
```

결과: **32/32 passed**
