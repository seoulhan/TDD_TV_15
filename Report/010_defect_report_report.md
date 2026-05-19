# 010 결함 관리 보고서(Defect Report) 작업 보고서

## 작업 개요

- QA 리드 엔지니어 관점에서 **결함 분류 체계**, **결함 보고서 템플릿**, **품질 메트릭 수집 계획**을 통합한 결함 관리 문서를 작성했습니다.
- 산출물은 `docs\defect_report.md`에 Markdown으로 저장했습니다.
- Wrap-up으로 본 보고서와 대화 transcript를 `Report\`, `Prompting\`에 번호 **010**으로 추가했습니다.

## 생성·갱신 파일

| 경로 | 설명 |
|------|------|
| `docs\defect_report.md` | 결함 관리 보고서 본문 (신규) |
| `Report\010_defect_report_report.md` | 작업 보고서 (본 파일) |
| `Prompting\010_defect_report.md` | 프롬프트 대화 transcript |

## 참조한 소스

| 소스 | 용도 |
|------|------|
| `docs\defect_list.md` | DEF-001~003, Severity·Feature 매핑, §1.3 매트릭스 수치 |
| `docs\requirements_analysis.md` | Feature 정의, Test Double 전략, 시나리오 ID |
| `docs\test_plan.md` | P0/P1, lcov 목표, X-OTH-02, TEST_F 매트릭스 |
| `Report\007_defect_list_report.md` | 결함 목록 작업 이력·검증(32/32) |
| `Report\004_test_plan_report.md` | 커버리지·Test Double 계획 |
| `gen_lcov2.bat` | QM-02 lcov 파이프라인 |

## 문서에 반영한 내용

### 1) 결함 분류 체계

- Severity 4단계(Critical/Major/Minor/Info) 정의 및 SLA 권장
- Feature 4분류(숫자입력, 선호채널, 검색, Up/Down)와 README § 매핑
- **Severity × Feature 매트릭스**: 현재 DEF-001/002(Major·UpDown·Fixed), DEF-003(Info·NumberInput·Open) 반영
- Feature별 Major/Minor 등록 가이드

### 2) 결함 보고서 템플릿

- 재현 / 기대 / 실제 / 원인 / 수정 / 검증 섹션
- Status, 발견 단계, Test Double, 관련 TEST_F 메타 필드
- DEF-001 요약 예시 표

### 3) 품질 메트릭 수집 계획

- **QM-01** 테스트 통과율 (`ctest`, 100% 게이트)
- **QM-02~03** lcov line/branch (≥90% / ≥85%, `gen_lcov2.bat`)
- **QM-04** 단계별(T1~T5) 결함 발견율 및 현재 스냅샷
- **QM-07** Fake/Mock/Stub별 최초 발견 비율·격리 효과 분석
- Mermaid 의사결정 트리, 스프린트·PR 보고 주기

## 기존 문서와의 관계

| 문서 | 역할 |
|------|------|
| `defect_report.md` | **프로세스·분류·메트릭** (본 작업) |
| `defect_list.md` | **개별 결함 인스턴스** (DEF-xxx 상세) |

## 검증

- 문서 내용은 `defect_list.md` v1.0 및 `ctest` 32/32 Green 이력(`007` 보고서)과 정합성 확인.
- 별도 빌드 변경 없음 (문서만 추가).

## 후속 권장

- `gen_lcov2.bat`에 `"*/include/*"` extract 추가 후 QM-02 베이스라인 수치 기록
- S4-3 Mock 테스트 추가 시 QM-04 T3 발견율 추적
- DEF-003 요구 확정 후 매트릭스 §1.3 NumberInput 셀 Severity 갱신
