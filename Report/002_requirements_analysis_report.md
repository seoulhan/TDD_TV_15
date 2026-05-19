# 002 요구사항 분석 문서 작업 보고서

## 작업 개요

- README.md와 기존 `TVController` / 테스트 코드를 기준으로, 시니어 C++ QA 엔지니어 관점의 요구사항 분석 문서를 작성했습니다.
- 산출물은 `docs\requirements_analysis.md`에 Markdown(표 + 번호 목록) 형식으로 저장했습니다.
- Wrap-up으로 본 보고서와 대화 transcript를 각각 `Report\`, `Prompting\`에 번호 체계(002)로 추가했습니다.

## 생성·갱신 파일

| 경로 | 설명 |
|------|------|
| `docs\requirements_analysis.md` | 요구사항 분석 본문 (신규) |
| `Report\002_requirements_analysis_report.md` | 작업 보고서 (본 파일) |
| `Prompting\002_requirements_analysis.md` | 프롬프트 대화 transcript |

## 참조한 소스

- `README.md` — 리모컨·채널·Tuner·Controller 기능 명세
- `include\TVController.h` — 현재 구현 상태(`inputBuffer_`, `favorites_`, `scannedChannels_`)
- `include\remoteKey.h`, `include\Tuner.h`
- `test\TVControllerTest.cpp` — 기존 S1/S2/S3 시나리오 및 미완 테스트(`UpDownWithSearchResults`)
- `test\FakeTuner.h`, `test\TunerTest.cpp` — Fake vs Mock 사용 패턴

## 문서에 반영한 내용

### 1) 리모컨 기능별 비즈니스 규칙 표

- 숫자 버퍼·2자리 즉시 전환·4자리 연속·45+6 패턴·무효화
- 선호 채널 토글(추가/삭제·정렬)
- 다음 선호 채널(`upper_bound`, wrap)
- 채널 검색(수집·정렬·시작 채널 복원)
- 업/다운(검색 목록 유무에 따른 분기)

### 2) 상태 관리 주의점

- `inputBuffer_ = -1` 초기화 시점 및 `-1` vs 채널 `0` 구분
- `favorites_`, `scannedChannels_` 오름차순 유지와 `upper_bound`/`lower_bound` 전제
- 검색 루프 종료(중복 `seekCH`), `Tuner` 문자열 API(`stoi`) 주의

### 3) 예외·경계값

- 채널 0~99 이탈 시 `std::invalid_argument`
- 빈 선호 목록에서 다음 선호 no-op
- 일반 업/다운·검색 목록 업/다운·다음 선호 각각의 wrap-around

### 4) Test Double 전략

- **FakeTuner**: 결과·통합 시나리오(채널, 목록, 검색, wrap)
- **MockTuner**: `setCH`/`seekCH` 호출 순서·계약(`TunerTest` 패턴)
- **StubTuner**: Approval Golden Master 한계 명시

### 5) Google Test 시나리오

- S1-1 ~ S6-3, E-x, T-x 및 미완 항목까지 **34개** 번호 목록
- `TVControllerTest.cpp` 기존 주석과 매핑

## 검증

- 문서 작성 작업만 수행했으며 CMake 빌드·테스트 실행은 수행하지 않았습니다.
- `docs\` 디렉토리는 본 작업 중 신규 생성되었습니다.

## 후속 권장 작업

- `TVControllerTest::UpDownWithSearchResults` 미완 시나리오(S6-1, S6-2) 구현
- `docs\requirements_analysis.md` §5.9 항목을 기준으로 테스트 커버리지 보강
