# 001 Cursor Rules 작업 보고서

## 작업 개요
- TDD_TV C++17 프로젝트 루트에 Cursor AI용 규칙 파일 `.cursorrules`를 생성했습니다.
- 요청된 기술 스택, 절대 규칙, 테스트 규칙, 리팩토링 규칙을 프로젝트 지침으로 정리했습니다.
- 작업 대화 transcript를 `Prompting\001_cursorrules.md`로 함께 저장했습니다.

## 생성 파일
- `.cursorrules`
- `Report\001_cursorrules_report.md`
- `Prompting\001_cursorrules.md`

## 반영한 주요 규칙
- `include/Tuner.h`는 외부 제공 인터페이스이므로 수정 금지.
- 채널 범위는 `0`부터 `99`까지로 제한.
- 실제 `Tuner` 대신 Fake 또는 Mock을 사용해 의존성을 격리.
- Given-When-Then 스타일과 `TEST_F` 기반 테스트 권장.
- 채널 `0`, `99`, 빈 목록, 세 자리 입력, Wrap-around 경계값 테스트 포함.
- 테스트 Green 상태에서만 리팩토링 진행.
- `std::cout` 출력 로직은 비즈니스 로직과 분리.

## 검증
- 파일 생성 작업만 수행했으며 빌드나 테스트 실행은 필요하지 않아 수행하지 않았습니다.
