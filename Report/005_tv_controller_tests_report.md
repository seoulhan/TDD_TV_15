# 005 TVController 단위 테스트 구현 보고서

## 작업 개요

- README 6대 핵심 기능(§1~§6)별로 `TVControllerTest`에 **최소 1~2개** `TEST_F`를 작성·정리했습니다.
- Given-When-Then 주석, `ASSERT_EQ`/`EXPECT_EQ`로 `FakeTuner::getCurrentCH()` 검증, 검색 연동·Up/Down 분기를 반영했습니다.
- `cmake --build build && ctest` **32/32 Green** 확인했습니다.

## 변경 파일

| 경로 | 설명 |
|------|------|
| `test\TVControllerTest.cpp` | 6대 기능별 TEST_F 18개 + 예외 1개 (기존 스텁·중복 제거 후 재구성) |
| `Report\005_tv_controller_tests_report.md` | 본 보고서 |
| `Prompting\005_tv_controller_tests.md` | 프롬프트 대화 transcript |

## README 기능별 TEST_F 매핑

| README | TEST_F | 검증 요약 |
|--------|--------|-----------|
| §1 숫자 입력 | `SingleDigitThenOk_ChangesChannel`, `TwoDigitsInRow_AutoChangesWithoutOk`, `FourDigits_ChangesTwice`, `ThreeDigits_BufferThenOkOrInvalidate`, `ZeroThenSeven_ChangesTo7` | 1+OK, 2자리 즉시, 4자리 연속, 45+6 버퍼/OK/OTHER, 0+7 |
| §2 선호 토글 | `FavoriteAdd_AddsCurrentChannel`, `FavoriteToggle_RemovesWhenPressedAgain`, `FavoriteToggleScenario_KeepsSortedUnique` | 추가·삭제·정렬 |
| §3 다음 선호 | `NextFavorite_MovesToSmallestGreater`, `NextFavorite_WrapsToFirst`, `NextFavorite_EmptyList_NoOp` | upper_bound, wrap, 빈 목록 |
| §4 채널 검색 | `ChannelSearch_CollectsAvailableSorted`, `ChannelSearch_RestoresOriginalChannel` | seekCH 수집·시작 채널 복원 |
| §5 Up/Down (검색 없음) | `UpDownWithoutSearch_IncrementDecrement`, `UpDownWithoutSearch_WrapsAtBoundaries` | 6↔5/7, 99→0, 0→99 |
| §6 Up/Down (검색 있음) | `UpDownWithSearch_OnListChannel`, `UpDownWithSearch_OffListChannelWraps` | {4,6,14} 목록 내·외 wrap |
| 경계 | `InvalidChannelThrows` | `applyChannel` ±1, 100 |

## 구현 메모

- Fixture에 `useChannels()` 헬퍼를 추가해 §6 시나리오용 `FakeTuner({4,6,14})`를 주입합니다.
- `pressDown`은 `lower_bound(cur)` 후 `--it`이므로, 현재 채널이 목록에 있을 때 **자기 자신이 아닌 바로 이전** 채널로 이동합니다 (예: 6→4, 14→6).
- 기존 미완성 `UpDownWithSearchResults` 스텁과 `OtherButtonCancelsBuffer`/`OtherButtonCancelsNumberBuffer` 중복은 `ThreeDigits_BufferThenOkOrInvalidate`로 통합했습니다.

## 테스트 실행 결과

```
ctest --test-dir build
32/32 passed (TVControllerTest 18건 포함)
```

## 참조

- `include\TVController.h`, `test\FakeTuner.h`, `docs\requirements_analysis.md`, `README.md`
