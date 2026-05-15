#ifndef TV_CONTROLLER_H
#define TV_CONTROLLER_H

#include "Tuner.h"
#include "remoteKey.h"
#include <algorithm>
#include <string>
#include <vector>

class TVController {
private:
  Tuner *tuner;
  int inputBuffer_ = -1;       // -1: 버퍼 비어있음
  std::vector<int> favorites_; // 선호 채널 목록

  bool isFavorite(int ch) const {
    return std::find(favorites_.begin(), favorites_.end(), ch) !=
           favorites_.end();
  }

  // PDF 15p : 채널 적용 전 유효성 검사 및 실제 튜너 호출
  void applyChannel(int ch) {
    if (ch >= 0 && ch <= 99) {
      tuner->setCH(std::to_string(ch));
    }
  }

  // Enum 키를 숫자로 변환 (이 부분은 추후 모든 숫자로 확장 가능)
  int keyToDigit(remoteKey key) {
    switch (key) {
    case remoteKey::KEY_1:
      return 1;
    case remoteKey::KEY_2:
      return 2;
    case remoteKey::KEY_3:
      return 3;
    case remoteKey::KEY_4:
      return 4;
    default:
      return -1;
    }
  }

public:
  explicit TVController(Tuner *tuner) : tuner(tuner) {}

  void pressFavorite() {
    int ch = std::stoi(tuner->getCurrentCH());

    if (isFavorite(ch)) {
      // 이미 있으면 삭제
      favorites_.erase(std::remove(favorites_.begin(), favorites_.end(), ch),
                       favorites_.end());
    } else {
      // 없으면 추가 및 정렬
      favorites_.push_back(ch);
      std::sort(favorites_.begin(), favorites_.end());
    }
  }

  const std::vector<int> &getFavoriteChannels() const { return favorites_; }

  void pressNextFavorite() {
    if (favorites_.empty())
      return; // S3-4: 목록 비어있으면 무시

    int cur = std::stoi(tuner->getCurrentCH());

    // 현재 채널보다 큰 첫 번째 요소 찾기 (O(log n))
    auto it = std::upper_bound(favorites_.begin(), favorites_.end(), cur);

    int next;
    if (it != favorites_.end()) {
      next = *it; // 현재보다 큰 채널이 있음
    } else {
      next = favorites_.front(); // 없으면 첫 번째로
    }

    applyChannel(next);
  }

  void pushButton(remoteKey key) {
    if (key == remoteKey::KEY_OK) {
      if (inputBuffer_ != -1) {
        applyChannel(inputBuffer_);
        inputBuffer_ = -1;
      }
      return;
    }

    if (key == remoteKey::KEY_NEXT_FAV) {
      pressNextFavorite();
      return;
    }

    int digit = keyToDigit(key);
    if (digit != -1) {
      if (inputBuffer_ == -1) {
        inputBuffer_ = digit; // 첫 번째 자리 저장
      } else {
        // 두 번째 자리 완성 시 자동 변경 (PDF 15p 힌트 로직)
        int ch = inputBuffer_ * 10 + digit;
        inputBuffer_ = -1;
        applyChannel(ch);
      }
    }
  }
};

#endif