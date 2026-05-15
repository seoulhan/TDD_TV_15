// 1. 이 매크로가 있으면 라이브러리가 알아서 GTest용 main을 생성합니다.
#define APPROVALS_GOOGLETEST
#include <ApprovalTests.hpp>

#include "TVController.h"
#include "Tuner.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

// 2. 명시적으로 네임스페이스 지정
using namespace ApprovalTests;

class StubTuner : public Tuner {
public:
  std::string seekCH() override { return "5"; }
  void setCH(const std::string &ch) override {}
  std::string getCurrentCH() override { return "1"; }
};

TEST(TVControllerApprovalTest, GoldenMaster_InitialBehavior) {
  std::stringstream buffer;
  std::streambuf *old = std::cout.rdbuf(buffer.rdbuf());

  StubTuner tuner;
  TVController controller(&tuner);

  controller.pushButton(remoteKey::KEY_1);
  controller.pushButton(remoteKey::KEY_OK);

  std::cout.rdbuf(old);

  Approvals::verify(buffer.str());
}
