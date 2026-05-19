# 008 Approval Golden Master Transcript

## User (1)

```text
@test/ApprovalTest.cpp @include/TVController.h 

[Persona] 회귀 테스트(Approval/Golden Master) 설계 전문가입니다.
[Context] C++17, Google Test, ApprovalTests.cpp, CMake
[Task] std::cout 로그나 외부 의존성(Tuner) 호출 순서를 기반으로 Golden Master를 설계/구현해줘.
    1) ApprovalTests.cpp를 활용한 expected 출력 파일(.approved.txt) 생성/보관 전략
    2) TVController의 pushButton 연속 호출에 따른 상태 변화를 캡처하는 테스트 구현
    3) CMake/ctest에 통합하는 방법
    4) 리팩토링 중 로그가 제거될 경우, Approval Test를 상태 검증으로 대체하거나 폐기하는 전략 제시
[Format] 테스트 코드 + CMake 수정안 + 실행 방법
[Wrap-up] 
 - Report\ 디렉토리 아래에 작업 내용 보고서를 번호를 붙여 생성하고 저장
 - 현재까지의 프롬프트 대화 전체를 Prompting\ 디렉토리 아래에 Export Transcript 해줘. Transcript 파일명은 보고서의 이름에서 _report 접미사를 뺀 것으로 해줘.
```

## Assistant (1)

```text
분석:
- TVController.h에는 std::cout 없음 → Golden Master는 Tuner 호출 trace + getters 상태 중심.
- 기존 ApprovalTest는 빈 cout만 검증하던 스텁 수준.

구현:
1. test/RecordingTuner.h — FakeTuner 위임 + setCH/seekCH/getCurrentCH 호출 로그
2. test/ApprovalTest.cpp — buildGoldenMaster(scenario, cout, tuner, controller) 포맷
   - GoldenMaster_DigitChannelFlow (§1)
   - GoldenMaster_FavoriteAndNextFavorite (§2·§3)
   - GoldenMaster_SearchAndUpDown (§4·§6)
   - GoldenMaster_BufferInvalidate (§1 무효화)
3. CMakeLists.txt — TVControllerApprovalTest 타깃 분리, gtest_discover_tests 등록
4. test/*.approved.txt 4개 생성 (AutoApprove 1회 후 커밋 대상)

실행:
  cmake -S . -B build && cmake --build build
  ctest --test-dir build                    # 35/35 Green
  ctest --test-dir build -R TVControllerApproval

리팩토링 전략 (요약):
- cout 제거만 → Approval 유지 (none)
- 로그 전용 Approval → 폐기 또는 state/trace로 대체
- P0는 TVControllerTest, P2는 Approval 이중 방어

Wrap-up:
- Report/008_approval_golden_master_report.md
- Prompting/008_approval_golden_master.md (본 transcript)
```
