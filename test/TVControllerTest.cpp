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

// S1-4: 세 자리 입력 중 기타 버튼 누르면 무효화
TEST_F(TVControllerTest, OtherButtonCancelsBuffer) {
  ctrl->pushButton(remoteKey::KEY_4);
  ctrl->pushButton(remoteKey::KEY_5);     // 45번 채널 변경됨
  ctrl->pushButton(remoteKey::KEY_6);     // 버퍼에 6 들어감
  ctrl->pushButton(remoteKey::KEY_OTHER); // 버퍼 지워짐

  // 6번으로 변경되지 않고 마지막 채널인 45번 유지 확인
  EXPECT_EQ("45", tuner->getCurrentCH());
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

// S3-1 & S3-2: 목록 중 현재보다 큰 채널로 이동
TEST_F(TVControllerTest, NextFavorite_Normal) {
  // 선호 채널 등록: {1, 4, 12, 56} (직접 추가 헬퍼가 없으므로 버튼으로
  // 추가하거나 목록에 직접 삽입)
  for (int ch : {1, 4, 12, 56}) {
    tuner->setCH(std::to_string(ch));
    ctrl->pressFavorite();
  }

  tuner->setCH("6");         // 현재 6번 시청 중
  ctrl->pressNextFavorite(); // 다음 선호 채널 버튼 클릭

  EXPECT_EQ("12", tuner->getCurrentCH()); // 6보다 큰 12로 이동 기대
}

// S3-3: 마지막 채널에서 누르면 첫 번째로 (Wrap-around)
TEST_F(TVControllerTest, NextFavorite_WrapAround) {
  tuner->setCH("1");
  ctrl->pressFavorite();
  tuner->setCH("56");
  ctrl->pressFavorite();

  tuner->setCH("56"); // 현재 마지막 선호 채널인 56번 시청 중
  ctrl->pressNextFavorite();

  EXPECT_EQ("1", tuner->getCurrentCH()); // 첫 번째인 1로 돌아가기 기대
}

// 예외 상황 테스트
TEST_F(TVControllerTest, InvalidChannelThrows) {
  EXPECT_THROW(ctrl->applyChannel(100), std::invalid_argument);
  EXPECT_THROW(ctrl->applyChannel(-1), std::invalid_argument);
}

TEST_F(TVControllerTest, SearchAndRestoreOriginalChannel) {
  tuner->setCH("12"); // 원래 12번 보고 있었음
  ctrl->pushButton(remoteKey::KEY_SEARCH);

  // 검색 결과 확인 (FakeTuner setup에 따른 1, 4, 12, 56)
  const auto &scanned = ctrl->getScannedChannels();
  ASSERT_EQ(4u, scanned.size());
  EXPECT_EQ(1, scanned[0]);

  // 원래 채널로 복원되었는지 확인
  EXPECT_EQ("12", tuner->getCurrentCH());
}

// 3. 업/다운 테스트 (검색 결과가 있을 때) - README 6번 항목
TEST_F(TVControllerTest, UpDownWithSearchResults) {
  ctrl->pushButton(remoteKey::KEY_SEARCH); // 4, 6, 14 등이 있다고 가정할 때
                                           // (Setup 변경 필요)

  // 가상의 검색 결과 주입을 위해 Fixture에서 Tuner 설정을 조정한 후 테스트
  // 15번 시청 중 업 누르면 가장 작은 4번으로 가는지 확인 등
}

// 4. 버퍼 무효화 테스트 - README 1번 항목
TEST_F(TVControllerTest, OtherButtonCancelsNumberBuffer) {
  ctrl->pushButton(remoteKey::KEY_4);
  ctrl->pushButton(remoteKey::KEY_5);     // 45번 이동
  ctrl->pushButton(remoteKey::KEY_6);     // 버퍼에 6
  ctrl->pushButton(remoteKey::KEY_OTHER); // 무효화
  ctrl->pushButton(remoteKey::KEY_OK);    // OK 눌러도 변화 없어야 함

  EXPECT_EQ("45", tuner->getCurrentCH());
}