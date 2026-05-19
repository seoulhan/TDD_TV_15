#include "FakeTuner.h"
#include "TVController.h"
#include <gtest/gtest.h>

class TVControllerTest : public ::testing::Test {
protected:
  std::unique_ptr<FakeTuner> tuner;
  std::unique_ptr<TVController> ctrl;

  void SetUp() override {
    tuner = std::make_unique<FakeTuner>(std::vector<int>{1, 4, 12, 56});
    ctrl = std::make_unique<TVController>(tuner.get());
  }

  void useChannels(const std::vector<int> &channels) {
    tuner = std::make_unique<FakeTuner>(channels);
    ctrl = std::make_unique<TVController>(tuner.get());
  }
};

// ---------------------------------------------------------------------------
// README §1 — 숫자 버튼으로 채널 변경
// ---------------------------------------------------------------------------

TEST_F(TVControllerTest, SingleDigitThenOk_ChangesChannel) {
  // Given: 초기 채널(튜너 기본 1), 숫자 버퍼 비어 있음
  // When: '4' 입력 후 확인
  ctrl->pushButton(remoteKey::KEY_4);
  ctrl->pushButton(remoteKey::KEY_OK);
  // Then: 4번 채널로 전환
  ASSERT_EQ("4", tuner->getCurrentCH());
}

TEST_F(TVControllerTest, TwoDigitsInRow_AutoChangesWithoutOk) {
  // Given: 버퍼 비어 있음
  // When: '1' 후 '2' 연속 입력
  ctrl->pushButton(remoteKey::KEY_1);
  ctrl->pushButton(remoteKey::KEY_2);
  // Then: 즉시 12번 채널
  ASSERT_EQ("12", tuner->getCurrentCH());
}

TEST_F(TVControllerTest, FourDigits_ChangesTwice) {
  // Given: 버퍼 비어 있음
  // When: '1','2','3','4' 연속 입력
  ctrl->pushButton(remoteKey::KEY_1);
  ctrl->pushButton(remoteKey::KEY_2);
  ctrl->pushButton(remoteKey::KEY_3);
  ctrl->pushButton(remoteKey::KEY_4);
  // Then: 12 → 34 순으로 전환
  ASSERT_EQ("34", tuner->getCurrentCH());
}

TEST_F(TVControllerTest, ThreeDigits_BufferThenOkOrInvalidate) {
  // Given: 버퍼 비어 있음
  // When: '4','5' → 45번, '6'은 버퍼만 채움
  ctrl->pushButton(remoteKey::KEY_4);
  ctrl->pushButton(remoteKey::KEY_5);
  ASSERT_EQ("45", tuner->getCurrentCH());

  ctrl->pushButton(remoteKey::KEY_6);
  ASSERT_EQ("45", tuner->getCurrentCH());

  // When: 확인 → 6번
  ctrl->pushButton(remoteKey::KEY_OK);
  ASSERT_EQ("6", tuner->getCurrentCH());

  // Given: 다시 45번, 버퍼에 6
  ctrl->pushButton(remoteKey::KEY_4);
  ctrl->pushButton(remoteKey::KEY_5);
  ctrl->pushButton(remoteKey::KEY_6);
  // When: 기타 버튼 → 무효화
  ctrl->pushButton(remoteKey::KEY_OTHER);
  // Then: 45 유지, OK로도 6번 아님
  ASSERT_EQ("45", tuner->getCurrentCH());
  ctrl->pushButton(remoteKey::KEY_OK);
  EXPECT_EQ("45", tuner->getCurrentCH());
}

TEST_F(TVControllerTest, ZeroThenSeven_ChangesTo7) {
  // Given: 버퍼 비어 있음
  // When: '0' 후 '7'
  ctrl->pushButton(remoteKey::KEY_0);
  ctrl->pushButton(remoteKey::KEY_7);
  // Then: 07 → 7번
  ASSERT_EQ("7", tuner->getCurrentCH());
}

// ---------------------------------------------------------------------------
// README §2 — 선호 채널 추가/삭제 (Toggle)
// ---------------------------------------------------------------------------

TEST_F(TVControllerTest, FavoriteAdd_AddsCurrentChannel) {
  // Given: 12번 시청 중, 선호 목록 비어 있음
  tuner->setCH("12");
  // When: 선호 채널 추가
  ctrl->pressFavorite();
  // Then: 12가 목록에 포함
  const auto &favs = ctrl->getFavoriteChannels();
  ASSERT_NE(favs.end(), std::find(favs.begin(), favs.end(), 12));
}

TEST_F(TVControllerTest, FavoriteToggle_RemovesWhenPressedAgain) {
  // Given: 12번이 선호 채널에 등록됨
  tuner->setCH("12");
  ctrl->pressFavorite();
  // When: 동일 채널에서 다시 추가 버튼
  ctrl->pressFavorite();
  // Then: 12가 목록에서 제거
  const auto &favs = ctrl->getFavoriteChannels();
  EXPECT_EQ(favs.end(), std::find(favs.begin(), favs.end(), 12));
}

TEST_F(TVControllerTest, FavoriteToggleScenario_KeepsSortedUnique) {
  // Given/When: 12→8→37 추가, 8 토글 삭제, 6 추가
  for (int ch : {12, 8, 37, 8, 6}) {
    tuner->setCH(std::to_string(ch));
    ctrl->pressFavorite();
  }
  // Then: {6, 12, 37} 오름차순
  const auto &favs = ctrl->getFavoriteChannels();
  ASSERT_EQ(3u, favs.size());
  EXPECT_EQ(6, favs[0]);
  EXPECT_EQ(12, favs[1]);
  EXPECT_EQ(37, favs[2]);
}

// ---------------------------------------------------------------------------
// README §3 — 다음 선호 채널 (upper_bound, wrap-around)
// ---------------------------------------------------------------------------

TEST_F(TVControllerTest, NextFavorite_MovesToSmallestGreater) {
  // Given: 선호 {1,4,12,56}, 현재 6번(목록에 없음)
  for (int ch : {1, 4, 12, 56}) {
    tuner->setCH(std::to_string(ch));
    ctrl->pressFavorite();
  }
  tuner->setCH("6");
  // When: 다음 선호 채널
  ctrl->pressNextFavorite();
  // Then: 6보다 큰 최소값 12
  ASSERT_EQ("12", tuner->getCurrentCH());
}

TEST_F(TVControllerTest, NextFavorite_WrapsToFirst) {
  // Given: 선호 {1,56}, 현재 56번
  tuner->setCH("1");
  ctrl->pressFavorite();
  tuner->setCH("56");
  ctrl->pressFavorite();
  tuner->setCH("56");
  // When: 다음 선호 채널
  ctrl->pressNextFavorite();
  // Then: wrap → 1번
  ASSERT_EQ("1", tuner->getCurrentCH());
}

TEST_F(TVControllerTest, NextFavorite_EmptyList_NoOp) {
  // Given: 선호 목록 비어 있음, 현재 5번
  tuner->setCH("5");
  // When: 다음 선호 채널
  ctrl->pressNextFavorite();
  // Then: 채널 변경 없음
  EXPECT_EQ("5", tuner->getCurrentCH());
}

// ---------------------------------------------------------------------------
// README §4 — 채널 검색 (FakeTuner seekCH 연동)
// ---------------------------------------------------------------------------

TEST_F(TVControllerTest, ChannelSearch_CollectsAvailableSorted) {
  // Given: 시청 가능 {1,4,12,56}, 현재 12번
  tuner->setCH("12");
  // When: 채널 검색
  ctrl->pushButton(remoteKey::KEY_SEARCH);
  // Then: 검색 결과가 정렬된 available 목록
  const auto &scanned = ctrl->getScannedChannels();
  ASSERT_EQ(4u, scanned.size());
  EXPECT_EQ(1, scanned[0]);
  EXPECT_EQ(4, scanned[1]);
  EXPECT_EQ(12, scanned[2]);
  EXPECT_EQ(56, scanned[3]);
}

TEST_F(TVControllerTest, ChannelSearch_RestoresOriginalChannel) {
  // Given: 검색 전 12번 시청
  tuner->setCH("12");
  // When: 채널 검색
  ctrl->pushButton(remoteKey::KEY_SEARCH);
  // Then: 검색 후에도 시청 채널 12 유지
  ASSERT_EQ("12", tuner->getCurrentCH());
}

// ---------------------------------------------------------------------------
// README §5 — 업/다운 (검색 결과 없음)
// ---------------------------------------------------------------------------

TEST_F(TVControllerTest, UpDownWithoutSearch_IncrementDecrement) {
  // Given: 검색 미실행, 현재 6번
  tuner->setCH("6");
  // When: 채널 다운
  ctrl->pushButton(remoteKey::KEY_DOWN);
  // Then: 5번
  ASSERT_EQ("5", tuner->getCurrentCH());

  // When: 채널 업 (6번으로 복귀 후 업)
  tuner->setCH("6");
  ctrl->pushButton(remoteKey::KEY_UP);
  // Then: 7번
  EXPECT_EQ("7", tuner->getCurrentCH());
}

TEST_F(TVControllerTest, UpDownWithoutSearch_WrapsAtBoundaries) {
  // Given: 검색 미실행
  // When/Then: 99 → 업 → 0
  tuner->setCH("99");
  ctrl->pushButton(remoteKey::KEY_UP);
  ASSERT_EQ("0", tuner->getCurrentCH());

  // When/Then: 0 → 다운 → 99
  tuner->setCH("0");
  ctrl->pushButton(remoteKey::KEY_DOWN);
  EXPECT_EQ("99", tuner->getCurrentCH());
}

// ---------------------------------------------------------------------------
// README §6 — 업/다운 (검색 결과 있음)
// ---------------------------------------------------------------------------

TEST_F(TVControllerTest, UpDownWithSearch_OnListChannel) {
  // Given: 시청 가능 {4,6,14}, 검색 완료, 현재 6번
  useChannels({4, 6, 14});
  ctrl->pushButton(remoteKey::KEY_SEARCH);
  tuner->setCH("6");

  // When: 채널 다운
  ctrl->pushButton(remoteKey::KEY_DOWN);
  // Then: 목록에서 이전 값 4 (6 → 4)
  ASSERT_EQ("4", tuner->getCurrentCH());

  // When: 채널 업
  tuner->setCH("6");
  ctrl->pushButton(remoteKey::KEY_UP);
  // Then: 목록에서 다음 큰 값 14 (6 → 14)
  EXPECT_EQ("14", tuner->getCurrentCH());
}

TEST_F(TVControllerTest, UpDownWithSearch_OffListChannelWraps) {
  // Given: 검색 결과 {4,6,14}, 현재 15번(목록 외)
  useChannels({4, 6, 14});
  ctrl->pushButton(remoteKey::KEY_SEARCH);
  tuner->setCH("15");

  // When: 채널 업
  ctrl->pushButton(remoteKey::KEY_UP);
  // Then: wrap → 목록 최소 4
  ASSERT_EQ("4", tuner->getCurrentCH());

  // When: 채널 다운 (15 기준으로 다시 설정 후)
  tuner->setCH("15");
  ctrl->pushButton(remoteKey::KEY_DOWN);
  // Then: wrap → 목록 최대 14
  EXPECT_EQ("14", tuner->getCurrentCH());
}

// ---------------------------------------------------------------------------
// 경계 — applyChannel 예외
// ---------------------------------------------------------------------------

TEST_F(TVControllerTest, InvalidChannelThrows) {
  EXPECT_THROW(ctrl->applyChannel(100), std::invalid_argument);
  EXPECT_THROW(ctrl->applyChannel(-1), std::invalid_argument);
}
