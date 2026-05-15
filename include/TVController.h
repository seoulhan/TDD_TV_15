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
  int inputBuffer_ = -1;             // -1: 버퍼 비어있음
  std::vector<int> favorites_;       // 선호 채널 목록
  std::vector<int> scannedChannels_; // 채널 검색 결과 저장

  bool isFavorite(int ch) const {
    return std::find(favorites_.begin(), favorites_.end(), ch) !=
           favorites_.end();
  }

public:
  explicit TVController(Tuner *tuner) : tuner(tuner) {}

  const std::vector<int> &getScannedChannels() const {
    return scannedChannels_;
  }

  void applyChannel(int ch) {
    if (ch < 0 || ch > 99) {
      throw std::invalid_argument("Invalid ch"); // PDF 요구사항: 예외 발생
    }
    tuner->setCH(std::to_string(ch));
  }

  //   int keyToDigit(remoteKey key) {
  //     int val = static_cast<int>(key);
  //     if (val >= 0 && val <= 9)
  //       return val;
  //     return -1;
  //   }

  void pushButton(remoteKey key) {
    int val = static_cast<int>(key);
    if (val >= 0 && val <= 9) { // 숫자 버튼
      if (inputBuffer_ == -1) {
        inputBuffer_ = val;
      } else {
        int ch = inputBuffer_ * 10 + val;
        inputBuffer_ = -1;
        applyChannel(ch);
      }
    } else if (key == remoteKey::KEY_OK) {
      if (inputBuffer_ != -1) {
        applyChannel(inputBuffer_);
        inputBuffer_ = -1;
      }
    } else if (key == remoteKey::KEY_UP) {
      pressUp();
    } else if (key == remoteKey::KEY_DOWN) {
      pressDown();
    } else if (key == remoteKey::KEY_SEARCH) {
      pressSearch();
    } else if (key == remoteKey::KEY_FAV_ADD) {
      pressFavorite();
    } else if (key == remoteKey::KEY_NEXT_FAV) {
      pressNextFavorite();
    } else {
      pressOther();
    } // S1-4 무효화 로직
  }

  void pressSearch() {
    scannedChannels_.clear();
    std::string startCH = tuner->getCurrentCH();

    tuner->setCH("0");

    while (true) {
      int found = std::stoi(tuner->seekCH());
      // 이미 찾은 채널이거나 시작 채널로 돌아왔으면 종료 (실제 Tuner 사양에
      // 따라 조정)
      if (std::find(scannedChannels_.begin(), scannedChannels_.end(), found) !=
          scannedChannels_.end())
        break;
      scannedChannels_.push_back(found);
      std::sort(scannedChannels_.begin(), scannedChannels_.end());
    }

    tuner->setCH(startCH);
  }

  // [기능 5, 6] 업/다운 버튼 동작
  void pressUp() {
    int cur = std::stoi(tuner->getCurrentCH());
    if (scannedChannels_.empty()) {
      applyChannel((cur + 1) % 100);
    } else {
      auto it = std::upper_bound(scannedChannels_.begin(),
                                 scannedChannels_.end(), cur);
      applyChannel(it == scannedChannels_.end() ? scannedChannels_.front()
                                                : *it);
    }
  }

  void pressDown() {
    int cur = std::stoi(tuner->getCurrentCH());
    if (scannedChannels_.empty()) {
      applyChannel((cur == 0) ? 99 : cur - 1);
    } else {
      auto it = std::lower_bound(scannedChannels_.begin(),
                                 scannedChannels_.end(), cur);
      if (it == scannedChannels_.begin())
        it = scannedChannels_.end();
      applyChannel(*(--it));
    }
  }

  const std::vector<int> &getFavoriteChannels() const { return favorites_; }

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

  void pressNextFavorite() {
    if (favorites_.empty())
      return;

    int cur = std::stoi(tuner->getCurrentCH());

    auto it = std::upper_bound(favorites_.begin(), favorites_.end(), cur);
    applyChannel(it == favorites_.end() ? favorites_.front() : *it);
  }

  void pressOther() { inputBuffer_ = -1; }
};

#endif