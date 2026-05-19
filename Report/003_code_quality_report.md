# 003 TVController 코드 품질 분석 작업 보고서

## 작업 개요

- `include/TVController.h`, `include/remoteKey.h`를 대상으로 SOLID 원칙(SRP/OCP 중심) 및 Code Smell 관점의 정적 분석을 수행했습니다.
- `pushButton` 메서드, 상태 변수(`inputBuffer_`, `favorites_`, `scannedChannels_`), C++17 개선 방향, Testability, 리팩토링 우선순위(1~5)를 Markdown 표 형식으로 정리했습니다.
- 산출물은 `docs\code_quality_report.md`에 저장했습니다.
- Wrap-up으로 본 보고서와 대화 transcript를 `Report\`, `Prompting\`에 003 번호로 추가했습니다.

## 생성·갱신 파일

| 경로 | 설명 |
|------|------|
| `docs\code_quality_report.md` | 코드 품질 분석 본문 (신규) |
| `Report\003_code_quality_report.md` | 작업 보고서 (본 파일) |
| `Prompting\003_code_quality.md` | 프롬프트 대화 transcript |

## 참조한 소스

- `include\TVController.h` — `pushButton`, `applyChannel`, `pressSearch`/`pressUp`/`pressDown`, 선호 채널, 상태 멤버
- `include\remoteKey.h` — `enum class remoteKey`, `to_string` (일부 키만 매핑)
- `include\Tuner.h` — 가상 인터페이스 (DIP/Test Double 경계)
- `test\TVControllerTest.cpp` — FakeTuner + `pushButton`/`press*` 혼합 테스트 패턴
- `Report\002_requirements_analysis_report.md` — 번호 체계·wrap-up 형식 참고

## 문서에 반영한 내용

### 1) 문제점 분석표 (5열)

- SRP/OCP 위반: 입력 디스패치·버퍼·검색·선호·네비게이션의 단일 클래스 결합
- Code Smell: Long Method(`pushButton`), Primitive Obsession(`int` 버퍼, `stoi` 반복)
- 영향·개선 방향(Command, State, Strategy, 값 객체)·우선순위 1~5

### 2) SRP/OCP 상세

- 6가지 변경 이유 표
- `Tuner` 가상화는 OCP에 부분 충족, `remoteKey` 확장은 미충족

### 3) C++17 개선

- Command / 입력 FSM State / `optional`·`variant` / Navigator·Scanner 추출
- STL: `set`, `unordered_set`, 정렬 invariant 캡슐화

### 4) Testability

- FakeTuner: **양호** (상태 기반 시나리오)
- Mock·격리 단위: **미흡** (God Class, public `press*` 이중 API)
- 모듈 분리 후 Fake/Mock 매핑 권장

### 5) 리팩토링 우선순위

1. `pushButton` Command/핸들러 분리  
2. `ChannelInputBuffer` + optional  
3. `Channel` 값 타입  
4. Navigator / Scanner 추출  
5. `Tuner` 소유권 정리  

## 검증

- 문서·보고서·transcript 작성만 수행했으며 CMake 빌드·테스트 실행은 수행하지 않았습니다.
- 코드 변경(리팩토링 구현)은 범위에 포함하지 않았습니다.

## 다음 단계 (권장)

- 우선순위 1: `makeCommand(remoteKey)` + 기존 GTest Green 유지한 Extract Method
- `docs\requirements_analysis.md` 시나리오 S1/S3와 1:1 매핑하며 Red-Green
- `UpDownWithSearchResults` 등 미완 테스트를 Navigator 추출 트리거로 사용
