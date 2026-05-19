#define APPROVALS_GOOGLETEST
#include <ApprovalTests.hpp>

#include "RecordingTuner.h"
#include "TVController.h"
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

using namespace ApprovalTests;

namespace {

std::string formatInts(const std::vector<int> &values) {
  std::ostringstream os;
  os << '[';
  for (size_t i = 0; i < values.size(); ++i) {
    if (i > 0)
      os << ',';
    os << values[i];
  }
  os << ']';
  return os.str();
}

// std::cout 로그 + Tuner 호출 trace + 컨트롤러 관측 상태를 하나의 Golden Master 텍스트로 합침
std::string buildGoldenMaster(const std::string &scenario,
                              const std::string &coutLog,
                              RecordingTuner &tuner,
                              const TVController &controller) {
  std::ostringstream os;
  os << "scenario: " << scenario << '\n';
  os << "--- cout ---\n";
  if (coutLog.empty())
    os << "(none)\n";
  else
    os << coutLog;
  os << "--- state ---\n";
  os << "currentCH: " << tuner.getCurrentCH() << '\n';
  os << "favorites: " << formatInts(controller.getFavoriteChannels()) << '\n';
  os << "scanned: " << formatInts(controller.getScannedChannels()) << '\n';
  os << "--- tuner trace ---\n";
  os << tuner.trace();
  return os.str();
}

class CoutCapture {
  std::stringstream buffer_;
  std::streambuf *saved_ = nullptr;

public:
  CoutCapture() {
    saved_ = std::cout.rdbuf();
    std::cout.rdbuf(buffer_.rdbuf());
  }

  ~CoutCapture() { std::cout.rdbuf(saved_); }

  std::string str() const { return buffer_.str(); }
};

void pushSequence(TVController &controller,
                  const std::vector<remoteKey> &keys) {
  for (remoteKey key : keys)
    controller.pushButton(key);
}

} // namespace

TEST(TVControllerApprovalTest, GoldenMaster_DigitChannelFlow) {
  CoutCapture coutCapture;
  RecordingTuner tuner;
  TVController controller(&tuner);

  pushSequence(controller,
               {remoteKey::KEY_1, remoteKey::KEY_OK, remoteKey::KEY_1,
                remoteKey::KEY_2, remoteKey::KEY_3, remoteKey::KEY_4,
                remoteKey::KEY_4, remoteKey::KEY_5, remoteKey::KEY_6,
                remoteKey::KEY_OK, remoteKey::KEY_0, remoteKey::KEY_7});

  Approvals::verify(buildGoldenMaster("DigitChannelFlow", coutCapture.str(),
                                      tuner, controller));
}

TEST(TVControllerApprovalTest, GoldenMaster_FavoriteAndNextFavorite) {
  CoutCapture coutCapture;
  RecordingTuner tuner({1, 4, 12, 56});
  TVController controller(&tuner);

  tuner.setCH("12");
  pushSequence(controller, {remoteKey::KEY_FAV_ADD});
  tuner.setCH("6");
  pushSequence(controller, {remoteKey::KEY_NEXT_FAV});
  tuner.setCH("56");
  pushSequence(controller, {remoteKey::KEY_FAV_ADD, remoteKey::KEY_NEXT_FAV});

  Approvals::verify(buildGoldenMaster("FavoriteAndNextFavorite",
                                      coutCapture.str(), tuner, controller));
}

TEST(TVControllerApprovalTest, GoldenMaster_SearchAndUpDown) {
  CoutCapture coutCapture;
  RecordingTuner tuner({4, 6, 14});
  TVController controller(&tuner);

  tuner.setCH("6");
  pushSequence(controller, {remoteKey::KEY_SEARCH});
  pushSequence(controller, {remoteKey::KEY_UP, remoteKey::KEY_DOWN});
  tuner.setCH("15");
  pushSequence(controller, {remoteKey::KEY_UP, remoteKey::KEY_DOWN});

  Approvals::verify(buildGoldenMaster("SearchAndUpDown", coutCapture.str(),
                                      tuner, controller));
}

TEST(TVControllerApprovalTest, GoldenMaster_BufferInvalidate) {
  CoutCapture coutCapture;
  RecordingTuner tuner;
  TVController controller(&tuner);

  pushSequence(controller,
               {remoteKey::KEY_4, remoteKey::KEY_5, remoteKey::KEY_6,
                remoteKey::KEY_OTHER, remoteKey::KEY_OK});

  Approvals::verify(buildGoldenMaster("BufferInvalidate", coutCapture.str(),
                                      tuner, controller));
}
