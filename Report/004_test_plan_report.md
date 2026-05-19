# 004 테스트 계획서 작업 보고서

## 작업 개요

- 시니어 QA 리드 관점에서 `TVController` 단위 테스트 계획서를 작성했습니다.
- 산출물은 `docs\test_plan.md`에 Markdown 형식으로 저장했습니다.
- Wrap-up으로 본 보고서와 대화 transcript를 `Report\`, `Prompting\`에 번호 **004**로 추가했습니다.

## 생성·갱신 파일

| 경로 | 설명 |
|------|------|
| `docs\test_plan.md` | 테스트 계획서 본문 (신규) |
| `Report\004_test_plan_report.md` | 작업 보고서 (본 파일) |
| `Prompting\004_test_plan.md` | 프롬프트 대화 transcript |

## 참조한 소스

- `include\TVController.h` — API·상태 변수·분기 구조
- `README.md` — 기능 요구 §1~§6
- `docs\requirements_analysis.md` — 시나리오 ID(S1~S6, E-x), Test Double·경계값 정리
- `test\TVControllerTest.cpp` — 기존 TEST_F·구현/미구현 갭
- `test\FakeTuner.h`, `test\TunerTest.cpp` — Fake vs Mock 패턴
- `CMakeLists.txt`, `gen_lcov2.bat` — gcov/lcov 측정 파이프라인

## 문서에 반영한 내용

### 1) TEST_F 범위·우선순위

- `TVControllerTest` Fixture 기준 P0/P1/P2 분류
- S1~S6, E-x 시나리오별 권장 TEST_F 이름·구현 상태(✅/❌/⚠️) 매트릭스
- 스프린트별 실행 순서 권장

### 2) 경계값 케이스

- 채널 0/99 및 wrap (B-CH-04/05)
- 빈 선호 채널에서 다음 선호 no-op (B-FAV-01)
- 검색 목록 없을 때 Up/Down (B-SCN-01, S5)
- 검색 목록 있을 때 {4,6,14}·목록 외 cur=15 (B-SCN-10~13)

### 3) 예외·특이 케이스

- 연속 숫자 + OTHER 무효화 (S1-4, 기존 테스트 매핑)
- `0`→`7` 두 자리 조합 (X-DIG-01)
- `applyChannel` 예외, 버퍼+UP/DOWN 상호작용 주의(X-OTH-02)

### 4) Test Double 계획

- FakeTuner: 상태·통합 시나리오
- MockTuner: 검색 호출 시퀀스·계약
- 혼용 금지 가이드 및 선택적 `TVControllerSearchMockTest.cpp`

### 5) 커버리지

- `TVController` line **≥ 90%**, branch **≥ 85%** 목표
- `gen_lcov2.bat` + ctest 파이프라인, `*/include/*` extract 보강 권장
- 미커버 구간 ↔ 테스트 ID 연계 개선 루프

## 현재 테스트 갭 요약

| 우선순위 | 미구현·스켈레톤 |
|----------|-----------------|
| P0 | S1-3, S1-4b, S1-5, S5-1~3, S6-1~2, `UpDownWithSearchResults` |
| P1 | S1-6, S3-2, S3-4, S4-2~3(Mock), S6-3, E-2 |

## 검증

- 문서 작성 작업만 수행했으며 CMake 빌드·ctest·lcov 실행은 수행하지 않았습니다.

## 후속 권장 작업

- P0 TEST_F 구현 후 `gen_lcov2.bat` extract에 `*/include/*` 추가
- `docs\test_plan.md` §8 Exit Criteria 체크리스트로 PR 게이트 운영
