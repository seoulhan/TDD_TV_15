# TDD_TV 요구사항 분석 (C++ 구현 관점)

> **출처**: [README.md](../README.md)  
> **대상 모듈**: `TVController` (의존: `Tuner` 인터페이스, `remoteKey` enum)  
> **관점**: 시니어 C++ QA 엔지니어 — 비즈니스 규칙 명세, 상태·경계값, Test Double, GTest 시나리오

---

## 1. 리모컨 기능별 비즈니스 규칙

| 기능 | 입력 (`remoteKey`) | 전제 조건 | 동작 규칙 | `Tuner` 호출 | 관측 가능 결과 |
|------|-------------------|-----------|-----------|--------------|----------------|
| **숫자 버퍼 + 확인** | `KEY_0`~`KEY_9`, `KEY_OK` | 채널 유효 범위 0~99 | 첫 숫자는 `inputBuffer_`에만 저장. `KEY_OK` 시 버퍼 값으로 `setCH` 후 버퍼 `-1`로 초기화. 예: `1` → `OK` → 채널 1 | `setCH` | 현재 채널 = 버퍼 값 |
| **숫자 2자리 즉시 전환** | 숫자 2회 연속 | 첫 숫자 후 두 번째 숫자 입력 시 `ch = buf×10 + digit`, 즉시 `applyChannel(ch)`, 버퍼 `-1` | `setCH` | 예: `1` → `2` → 채널 12 (확인 불필요) |
| **숫자 4자리 연속 (2자리×2)** | 숫자 4회 | 각 2자리 완성마다 즉시 전환 | `setCH` (2회) | 예: `1`,`2`,`3`,`4` → 12 → 34 |
| **숫자 3자리 패턴 (45 + 6)** | `4`,`5`,`6` 후 분기 | `4`,`5` → 45 즉시 전환. `6`은 버퍼에만 보관 | — | 채널 45 유지, 버퍼=6 |
| **버퍼 확정 (6 + OK)** | `6` + `KEY_OK` | 버퍼에 한 자리 있음 | `applyChannel(6)` | 채널 6 |
| **버퍼 무효화** | `KEY_OTHER` 등 비숫자·비확인 키 (`pressOther`) | 버퍼에 미확정 숫자 있음 | `inputBuffer_ = -1`, 채널 변경 없음 | 이전 채널 유지 |
| **선호 채널 토글** | `KEY_FAV_ADD` | 현재 시청 채널 = `getCurrentCH()` | 목록에 없으면 추가 후 **오름차순 정렬**; 있으면 해당 값 **삭제** | `getCurrentCH` (읽기) | `favorites_` 변경 |
| **다음 선호 채널** | `KEY_NEXT_FAV` | `favorites_` 비어 있지 않음 | `upper_bound(cur)`로 **현재보다 큰 값 중 최소** 선택; 없으면 **목록 최소값**(wrap) | `getCurrentCH`, `setCH` | 예: {1,4,12,56}, cur=6 → 12; cur=56 → 1 |
| **채널 검색** | `KEY_SEARCH` | `Tuner::seekCH()`가 시청 가능 채널을 순환 반환 | 시작 채널 저장 → `setCH("0")` → `seekCH()` 반복, 중복·순환 종료 시까지 수집, **매 수집 후 정렬**, 종료 후 **시작 채널 복원** | `getCurrentCH`, `setCH`, `seekCH` | `scannedChannels_` 갱신, 시청 채널 불변 |
| **업 (검색 목록 없음)** | `KEY_UP` | `scannedChannels_.empty()` | `(cur + 1) % 100` | `getCurrentCH`, `setCH` | 99 → 0 wrap |
| **다운 (검색 목록 없음)** | `KEY_DOWN` | `scannedChannels_.empty()` | `cur == 0` → 99, else `cur - 1` | `getCurrentCH`, `setCH` | 0 → 99 wrap |
| **업 (검색 목록 있음)** | `KEY_UP` | `scannedChannels_` 정렬됨 | `upper_bound(cur)` → 다음 큰 값; 없으면 **목록 front** (wrap) | `getCurrentCH`, `setCH` | 예: {4,6,14}, cur=6 → 14; cur=15 → 4 |
| **다운 (검색 목록 있음)** | `KEY_DOWN` | `scannedChannels_` 정렬됨 | `lower_bound(cur)` 이전 원소; `begin`이면 **목록 back** (wrap) | `getCurrentCH`, `setCH` | 예: {4,6,14}, cur=6 → 4; cur=15 → 14 |

### 숫자 입력 상태 전이 (요약)

```text
[버퍼=-1] --(digit)--> [버퍼=d] --(digit)--> apply(10*d+d2), 버퍼=-1
[버퍼=d] --(OK)--> apply(d), 버퍼=-1
[버퍼=d] --(OTHER/UP/DOWN/...)--> 버퍼=-1 (채널 유지)
```

**특수 케이스**: `0`,`7` 연속 입력 → 두 번째 자리에서 `07` → `applyChannel(7)` (선행 0은 두 자리 조합의 일부).

---

## 2. 상태 관리 주의점

| 상태 변수 | 타입 | 초기값 | 초기화·갱신 시점 | 유지 규칙 |
|-----------|------|--------|------------------|-----------|
| `inputBuffer_` | `int` | **`-1`** (생성 시 멤버 초기화) | `-1`: 비어 있음. 첫 숫자 입력 시 0~9 저장. 두 번째 숫자 또는 `KEY_OK` 처리 후 **반드시 `-1`**. `pressOther()` 및 숫자·확인·업다운·검색·선호 외 키 분기에서 무효화 | 미확정 입력만 보관; 2자리 완성 시 버퍼는 즉시 비움 |
| `favorites_` | `vector<int>` | `{}` | `pressFavorite()`마다 add/remove 후 **`std::sort`** | 중복 없음(토글). 채널 번호 오름차순 — `upper_bound` / 다음 선호 채널에 필수 |
| `scannedChannels_` | `vector<int>` | `{}` | `pressSearch()` 시작 시 **`clear()`**, 수집 중 **중복 없이 push**, push마다 **정렬** | 업/다운(검색 모드)은 `upper_bound` / `lower_bound` 전제 → **항상 정렬 유지** |
| `Tuner` 현재 채널 | 외부 | Fake: 첫 available 또는 0 | `applyChannel` → `setCH`. 검색 종료 시 **시작 채널 문자열로 복원** | Controller는 채널 정수 검증 후 위임 |

### 구현·테스트 시 체크리스트

1. **버퍼 `-1`과 `0` 구분**: 버퍼가 “비어 있음”은 `-1`만 사용. 채널 0 입력은 `KEY_0` 한 번 + `KEY_OK` 또는 `KEY_x` 두 번째 자리 조합으로 표현.
2. **검색 vs 선호 목록**: 서로 독립. 검색 결과가 있어도 선호 채널 로직은 `favorites_`만 참조. 업/다운은 `scannedChannels_.empty()`로 모드 분기.
3. **검색 종료 조건**: `seekCH()` 반환값이 이미 `scannedChannels_`에 있으면 루프 종료(순환·중복 방지). Fake/Mock 모두 이 종료 조건을 만족하는 시퀀스를 준비해야 함.
4. **문자열 채널 API**: `Tuner`는 `std::string` 사용. Controller 내부는 `stoi` 후 정수 검증 — 테스트 시 `"12"` vs `12` 혼동 주의.
5. **생성 직후**: 별도 `reset()` 없음. `TVController` 생성 = 버퍼 `-1`, 두 목록 empty.

---

## 3. 예외·경계값 조건

| ID | 조건 | 기대 동작 | 비고 |
|----|------|-----------|------|
| E1 | `applyChannel(ch)`, `ch < 0` 또는 `ch > 99` | `std::invalid_argument` ("Invalid ch" 등) | 두 자리 입력 `9`,`9` → 99는 유효; `9`,`10` 불가(두 번째 키는 0~9만) |
| E2 | `applyChannel(100)` 등 버퍼 조합 결과 > 99 | 예외 (예: `1`,`0`,`0` 패턴은 키 시퀀스상 두 자리씩만 완성되므로 실질적으로 `10` 등) | **명시 테스트**: `applyChannel(100)`, `applyChannel(-1)` |
| E3 | `favorites_.empty()` 상태에서 `KEY_NEXT_FAV` | **no-op** (튜너 채널 변경 없음) | README 미명시; 빈 목록 안전 동작 권장 |
| E4 | 현재 채널이 선호 목록에 **정확히 포함**된 경우 다음 선호 | `upper_bound(cur)` → **자기 자신 제외**, strictly greater | cur=12, 목록에 12 있음 → 12보다 큰 최소 |
| E5 | Wrap — 일반 업/다운 | cur=99, UP → 0; cur=0, DOWN → 99 | `% 100` / 조건 분기 |
| E6 | Wrap — 검색 목록 업/다운 | cur가 목록에 없거나 최대/최소 초과 시 **순환** (README: cur=15, {4,6,14} → UP=4, DOWN=14) | 정렬된 `scannedChannels_` 필수 |
| E7 | Wrap — 다음 선호 | cur ≥ max(favorites) → `favorites_.front()` | {1,4,12,56}, cur=56 → 1 |
| E8 | `FakeTuner::setCH` / 실제 Tuner | 문자열 `"-1"`, `"100"` 등 | Tuner 측 `invalid_argument` 가능; Controller는 int 검증 후 호출 |
| E9 | 채널 검색 중 `seekCH` 빈 목록·단일 채널 | 무한 루프 방지 — **중복 감지로 종료** | Fake는 available 목록 순환 설계 |

---

## 4. Test Double 전략

| 구분 | FakeTuner (상태 기반) | MockTuner (GMock, 행위 기반) |
|------|------------------------|------------------------------|
| **역할** | 시청 가능 채널 집합을 가진 **가짜 튜너** — `setCH`/`seekCH`/`getCurrentCH`가 내부 `current_`·`available_`로 일관 동작 | **호출 순서·인자·횟수** 검증용 스파이 |
| **적합 기능** | • 숫자 입력 → 최종 채널 (`getCurrentCH`)<br>• 선호 토글·정렬·다음 선호 (상태 준비: `setCH` 후 버튼)<br>• 채널 검색 수집·복원 (실제 `seekCH` 순환)<br>• 검색 후 업/다운 (목록 {1,4,12,56} 등 Fixture 설계)<br>• Wrap-around 시나리오 E5~E7 | • `applyChannel`이 `setCH`를 **정확한 문자열**로 호출하는지<br>• `pressSearch()`의 `setCH("0")` → N×`seekCH()` → `setCH(startCH)` **시퀀스**<br>• 검색 루프 종료 조건(중복 `seekCH` 반환) **계약 테스트**<br>• Tuner 경계: 유효/무효 `setCH` (README 제공 `TunerTest.cpp` 패턴) |
| **검증 스타일** | **결과 상태**: `EXPECT_EQ("12", tuner->getCurrentCH())`, `getFavoriteChannels()`, `getScannedChannels()` | **기대 호출**: `EXPECT_CALL(mock, setCH("7")).Times(1);` `InSequence`로 검색 흐름 |
| **유지보수** | 채널 집합만 바꾸면 시나리오 재사용 (Approval·통합에 유리) | 시퀀스 변경 시 Expectation 전면 수정 |
| **비추천** | “`setCH`가 3번 불렸는지”만 필요한 단위 (과한 상태 설계) | 복잡한 20단계 리모컨 시나리오 전체를 Mock만으로 구성 |
| **프로젝트 참고** | `test/FakeTuner.h`, `TVControllerTest` Fixture | `test/TunerTest.cpp` (`MockTuner`) |

### 권장 조합

- **TVControllerTest**: 기본은 **FakeTuner** + 결과/assert on collections.  
- **검색 알고리즘 계약**·**Tuner 연동 최소 단위**: **MockTuner** + `InSequence`.  
- **Golden/Approval**: **StubTuner** (고정 반환) — 회귀 스냅샷용, 비즈니스 규칙 검증에는 부족.

---

## 5. Google Test 테스트 시나리오 목록

번호는 구현·추가 테스트 추적용. 기존 `TVControllerTest.cpp` 주석(S1-x)과 매핑.

### 5.1 숫자 버튼·버퍼 (README §1)

1. **S1-1** 한 자리 + 확인: `4` → `OK` → 채널 4  
2. **S1-2** 두 자리 즉시: `1` → `2` → 채널 12 (`OK` 없음)  
3. **S1-3** 네 자리 연속: `1`,`2`,`3`,`4` → 12, 이후 34  
4. **S1-4a** `4`,`5`,`6` → 45, 버퍼 6 유지  
5. **S1-4b** 버퍼 6 상태에서 `OK` → 채널 6  
6. **S1-4c** 버퍼 6 상태에서 `KEY_OTHER` → 채널 45 유지, 버퍼 무효  
7. **S1-4d** 무효화 후 `OK` → 채널 변화 없음  
8. **S1-5** `0`,`7` → 채널 7  
9. **S1-6** `1` → `OK` → 채널 1 (README 첫 예시)

### 5.2 선호 채널 (README §2)

10. **S2-1** 시청 중 채널 선호 추가 (비선호 → 목록 포함)  
11. **S2-2** 동일 채널 재입력 시 선호에서 삭제 (토글)  
12. **S2-3** 복수 추가·삭제 후 정렬 유지 (예: 12,8,37,8제거,6 → {6,12,37})

### 5.3 다음 선호 채널 (README §3)

13. **S3-1** {1,4,12,56}, cur=6 → 다음 → 12  
14. **S3-2** cur=4 → 다음 → 12 (중간값)  
15. **S3-3** cur=56 → 다음 → 1 (wrap)  
16. **S3-4** `favorites_` empty → `KEY_NEXT_FAV` no-op (경계 E3)

### 5.4 채널 검색 (README §4)

17. **S4-1** 검색 후 `scannedChannels_` = Fake available 정렬 목록  
18. **S4-2** 검색 전후 `getCurrentCH()` 동일 (시작 채널 복원)  
19. **S4-3** (Mock) `setCH("0")` → seek 반복 → `setCH(원래채널)` 호출 순서

### 5.5 업/다운 — 검색 없음 (README §5)

20. **S5-1** cur=6, UP → 7, DOWN → 5  
21. **S5-2** cur=99, UP → 0  
22. **S5-3** cur=0, DOWN → 99  

### 5.6 업/다운 — 검색 있음 (README §6)

23. **S6-1** scanned={4,6,14}, cur=6, UP→14, DOWN→4  
24. **S6-2** scanned={4,6,14}, cur=15, UP→4, DOWN→14 (목록 외 현재값 wrap)  
25. **S6-3** scanned 단일 원소 시 UP/DOWN wrap 동작  

### 5.7 예외·경계 (§3 문서)

26. **E-1** `applyChannel(-1)`, `applyChannel(100)` → `std::invalid_argument`  
27. **E-2** (선택) 두 자리 조합 99 경계: `9`,`9` → 99 성공  

### 5.8 Tuner 계약 (`TunerTest.cpp`, Controller 외)

28. **T-1** 초기 `getCurrentCH` 0~99  
29. **T-2** 유효 `setCH` 파라미터화 ("0","4",…,"99")  
30. **T-3** 무효 `setCH` 예외 ("-12","100",…)  
31. **T-4** `seekCH` 10회 반복·반환 범위  

### 5.9 미완·보강 권장 (코드베이스 갭)

32. **S6-x** `UpDownWithSearchResults` — Fixture에 scanned {4,6,14} 주입 후 S6-1~2 완성  
33. **S4-x** 검색 후 UP/DOWN이 scanned 모드로 동작하는지 S5와 교차 확인  
34. **Approval** Golden Master — Stub 한계 인지, Fake 기반 시나리오 확장 검토  

---

## 부록: C++ 구현 매핑

| README 항목 | `TVController` API | 키 진입점 |
|-------------|-------------------|-----------|
| §1 숫자 | `pushButton` 숫자/`KEY_OK`/`pressOther` | `inputBuffer_`, `applyChannel` |
| §2 선호 | `pressFavorite` | `favorites_` |
| §3 다음 선호 | `pressNextFavorite` | `upper_bound` on `favorites_` |
| §4 검색 | `pressSearch` | `scannedChannels_` |
| §5·§6 업/다운 | `pressUp`, `pressDown` | empty `scannedChannels_` 분기 |

**의존성 주입**: `TVController(Tuner*)` — 테스트에서 `FakeTuner` / `MockTuner` / `StubTuner` 교체.

---

*문서 버전: README 및 `include/TVController.h`, `test/TVControllerTest.cpp` 기준 (2026-05-19)*
