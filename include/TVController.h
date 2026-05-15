/**
 * Copyright 2020 by Samsung Electronics, Inc.,
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung.
 */

#ifndef TV_CONTROLLER_H
#define TV_CONTROLLER_H

#include "Tuner.h"
#include "remoteKey.h"
#include <iostream>
#include <string>


class TVController {
private:
  Tuner *tuner;
  std::string processingCH;

  void setTunerCh() {
    std::cout << "현재 설정하는 채널 : " << processingCH << std::endl;
    // tuner->setCH(processingCH);
  }

public:
  explicit TVController(Tuner *tuner) : tuner(tuner), processingCH("") {}

  void pushButton(remoteKey key) {
    switch (key) {
    case remoteKey::KEY_1:
      processingCH += to_string(key);
      break;
    case remoteKey::KEY_OK:
      setTunerCh();
      break;
    }
  }
};

#endif // TV_CONTROLLER_H
