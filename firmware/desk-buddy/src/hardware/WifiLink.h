// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#pragma once
#include <stdint.h>
#include "../companion/Network.h"
#include "../companion/Media.h"

namespace desk_buddy {
class WifiLink {
  public:
    void begin(pip::NetworkUi &network);
    bool update(pip::NetworkUi &network, pip::MediaLink &mediaLink, pip::Media &media,
                char *command, uint32_t now);
  private:
    pip::NetworkUi saved_;
    uint32_t scanStarted_ = 0;
    uint32_t lastAttempt_ = 0, lastPoll_ = 0;
    void connect(pip::NetworkUi &network, uint32_t now);
    bool poll(pip::NetworkUi &network, pip::MediaLink &mediaLink, pip::Media &media,
              uint32_t now);
    void command(const pip::NetworkUi &network, char *command);
};
} // namespace desk_buddy
