#include "TVController.h"
#include "FakeTuner.h" // 방금 만든 FakeTuner 포함
#include <gtest/gtest.h>

class TVControllerTest : public ::testing::Test {
protected:
  std::unique_ptr<FakeTuner> tuner;
  std::unique_ptr<TVController> ctrl;

  void SetUp() override {
    // 시청 가능한 채널 목록: 1, 4, 12, 56
    tuner = std::make_unique<FakeTuner>(std::vector<int>{1, 4, 12, 56});
    ctrl = std::make_unique<TVController>(tuner.get());
  }
};

// S1-1: 한 자리 입력 + 확인 -> 채널 변경
TEST_F(TVControllerTest, PressNumber4ThenConfirm) {
  ctrl->pushButton(remoteKey::KEY_4);
  ctrl->pushButton(remoteKey::KEY_OK);

  EXPECT_EQ("4", tuner->getCurrentCH());
}

TEST_F(TVControllerTest, Press1Then2_AutoChange) {
  ctrl->pushButton(remoteKey::KEY_1);
  ctrl->pushButton(remoteKey::KEY_2); // 두 번째 입력 시 즉시 setCH 호출 기대

  EXPECT_EQ("12", tuner->getCurrentCH());
}

// S2-1: 시청 중인 채널을 선호 채널에 추가
TEST_F(TVControllerTest, FavoriteAdd_NewChannel) {
  // 1. 튜너를 12번 채널로 맞춤 (FakeTuner의 상태 활용)
  tuner->setCH("12");

  // 2. 선호 채널 버튼 누름
  ctrl->pressFavorite();

  // 3. 결과 확인 (목록에 12가 들어있어야 함)
  const auto &favs = ctrl->getFavoriteChannels();
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 12));
}

// S2-2: 이미 있는 채널이면 삭제 (Toggle)
TEST_F(TVControllerTest, FavoriteToggle_Remove) {
  tuner->setCH("12");
  ctrl->pressFavorite(); // 추가
  ctrl->pressFavorite(); // 다시 눌러서 삭제

  const auto &favs = ctrl->getFavoriteChannels();
  EXPECT_EQ(favs.end(), std::find(favs.begin(), favs.end(), 12));
}

// S2-3: 복합 시나리오 (정렬 및 중복 삭제 확인)
TEST_F(TVControllerTest, FavoriteToggleScenario) {
  // 12 추가 -> 8 추가 -> 37 추가 -> 8 삭제 -> 6 추가
  for (int ch : {12, 8, 37, 8, 6}) {
    tuner->setCH(std::to_string(ch));
    ctrl->pressFavorite();
  }

  const auto &favs = ctrl->getFavoriteChannels();

  // 기대 결과: {6, 12, 37} - 정렬되어 있어야 함
  ASSERT_EQ(3u, favs.size());
  EXPECT_EQ(6, favs[0]);
  EXPECT_EQ(12, favs[1]);
  EXPECT_EQ(37, favs[2]);
}
