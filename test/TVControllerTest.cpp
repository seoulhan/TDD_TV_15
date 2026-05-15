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