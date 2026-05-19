# 001 Cursor Rules Transcript

## User

```text
[Persona] 당신은 임베디드/컨트롤러 모듈의 TDD 및 리팩토링을 돕는 시니어 C++ 엔지니어입니다.
[Context] TDD_TV C++17 프로젝트에서 Cursor AI가 항상 따라야 하는 규칙을
    프로젝트 루트의 .cursorrules로 작성하려고 합니다.
[Task] 아래 요구를 만족하는 .cursorrules 내용을 작성하고 파일을 생성해줘.
    - 기술 스택: C++17 + CMake + Google Test + GMock + ApprovalTests (+ gcov/lcov)
    - 절대 규칙: 외부 제공 인터페이스인 Tuner.h 수정 금지, 채널 범위는 0~99로 제한, 실제 Tuner 대신 반드시 Fake 또는 Mock을 사용하여 의존성 격리
    - 테스트 규칙: Given-When-Then, TEST_F, 경계값(채널 0, 99, 빈 목록, 세 자리 입력, Wrap-around) 포함
    - 리팩토링 규칙: 테스트 Green 상태에서만 진행, std::cout 로직은 비즈니스 로직과 분리
[Format] .cursorrules 파일에 그대로 붙여넣을 수 있는 텍스트
[Wrap-up] 
 - Report\ 디렉토리 아래에 작업 내용 보고서를 번호를 붙여 생성하고 저장
 - 현재까지의 프롬프트 대화 전체를 Prompting\ 디렉토리 아래에 Export Transcript 해줘. Transcript 파일명은 보고서의 이름에서 _report 접미사를 뺀 것으로 해줘.
```

## Assistant

```text
요청에 따라 프로젝트 루트에 .cursorrules 파일을 생성하고, Report\001_cursorrules_report.md 보고서와 Prompting\001_cursorrules.md transcript를 저장했습니다.
```
