# 007 결함 목록(Defect List) 작업 보고서

## 작업 개요

- QA 리드 관점에서 **현재까지 발견된 테스트 실패·결함**을 표준 필드 형식으로 정리했습니다.
- 산출물은 `docs\defect_list.md`에 Markdown으로 저장했습니다.
- Wrap-up으로 본 보고서와 대화 transcript를 `Report\`, `Prompting\`에 번호 **007**로 추가했습니다.

## 생성·갱신 파일

| 경로 | 설명 |
|------|------|
| `docs\defect_list.md` | 결함 목록 본문 (신규) |
| `Report\007_defect_list_report.md` | 작업 보고서 (본 파일) |
| `Prompting\007_defect_list.md` | 프롬프트 대화 transcript |

## 참조한 소스

| 소스 | 용도 |
|------|------|
| `Report\006_tv_controller_debug_report.md` | ctest #30 실패 분석, `pressDown`/`pressUp` 근본 원인·수정안 |
| `build\Testing\Temporary\LastTestsFailed.log` | 실패 테스트 ID 기록 |
| `test\TVControllerTest.cpp` | `UpDownWithSearch_OnListChannel` 기대/실제 |
| `include\TVController.h` | 결함 위치(94–116행) 및 현재 수정 상태 |
| `docs\test_plan.md` §4.1 X-OTH-02 | 버퍼+UP/DOWN 후보 결함(DEF-003) |
| `Report\004_test_plan_report.md`, `Report\005_tv_controller_tests_report.md` | 테스트 갭·Green 이력 |

## 등록 결함 요약

| ID | Severity | Feature | Status | 출처 |
|----|----------|---------|--------|------|
| DEF-001 | Major | UpDown | Fixed | ctest 실패 (`KEY_DOWN`, 6→5 오류) |
| DEF-002 | Major | UpDown | Fixed | 동일 시나리오 UP 대칭 결함(재현 조건부) |
| DEF-003 | Info | NumberInput | Open | 테스트 계획 분석(X-OTH-02), 자동 실패 없음 |

## 검증

```powershell
cd c:\DEV\TDD_TV_15\build
ctest --output-on-failure
```

**결과 (2026-05-19):** 32/32 passed. DEF-001/002는 현재 `TVController.h` 기준 수정 반영됨.

## 후속 권장

- DEF-003: README·요구 분석과 X-OTH-02 기대치 정렬 후 P1 테스트 추가 또는 Won't Fix 문서화
- 신규 결함 발견 시 본 문서에 ID 순번(DEF-004…)으로 항목 추가
- PR/스프린트 게이트에 `docs\defect_list.md` Open 항목 0건(또는 승인된 Info) 정책 검토
