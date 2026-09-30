// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#pragma once
#include "Canvas.h"
#include "Layout.h"
#include "../companion/Network.h"

namespace pip {
inline const char *networkStatus(NetworkStatus status, bool publisherOnline) {
    if (status == NETWORK_UNCONFIGURED) return "Set up Wi-Fi to bring Pip online";
    if (status == NETWORK_CONNECTING) return "Connecting to Wi-Fi...";
    if (status == NETWORK_ERROR) return "Could not connect. Check settings.";
    return publisherOnline ? "Wi-Fi and computer publisher are online" :
                             "Wi-Fi is connected; waiting for publisher";
}
inline void masked(Canvas &c, int x, int y, const char *text, bool hide, uint16_t color) {
    char visible[65];
    unsigned n = strlen(text);
    if (n >= sizeof(visible)) n = sizeof(visible) - 1;
    for (unsigned i = 0; i < n; ++i) visible[i] = hide ? '*' : text[i];
    visible[n] = 0;
    while (c.width(visible, fontSmall) > 390 && n) visible[--n] = 0;
    c.text(x, y, visible, fontSmall, color);
}
inline void networkField(Canvas &c, const Box &box, const char *label, const char *value,
                         bool selected, bool secret) {
    uint16_t border = selected ? rgb(83, 143, 112) : rgb(207, 216, 202);
    c.round(box.x, box.y, box.w, box.h, 11, border);
    c.round(box.x + 1, box.y + 1, box.w - 2, box.h - 2, 10, WHITE);
    c.text(box.x + 12, box.y - 18, label, fontSmall, MUTED);
    masked(c, box.x + 12, box.y + 13, value, secret, INK);
}
inline void networkSetup(Canvas &c, const NetworkUi &network) {
    c.rect(0, 0, 1024, 600, BG);
    c.text(52, 39, network.publisherPage ? "Connect your computer" : "Connect Pip to Wi-Fi", fontTitle, INK);
    c.text(52, 86, networkStatus(network.status, network.publisherOnline), fontSmall, MUTED);
    c.round(networkClose.x, networkClose.y, networkClose.w, networkClose.h, 12, STAGE);
    c.centered(networkClose.x + networkClose.w / 2, 48, "Back", fontSmall, INK);
    if (!network.publisherPage) {
    networkField(c, networkFields[0], "Nearby Wi-Fi  v", network.ssid[0] ? network.ssid : "Select a network...", false, false);
    networkField(c, networkFields[1], "Wi-Fi password", network.password,
                 network.field == WIFI_PASSWORD, true);
    c.text(72, 208, network.scanning ? "Scanning for nearby networks..." :
           network.scanFailed ? "Scan failed. Tap Refresh to try again." :
           !network.networkCount ? "No networks found. Tap Refresh to scan again." :
           network.secured ? "Select a network, enter its password, then Connect." :
           "This network is open. No password needed.", fontSmall, MUTED);
    c.round(networkRescan.x, networkRescan.y, networkRescan.w, networkRescan.h, 12, STAGE);
    c.centered(148, 278, "Refresh", fontSmall, INK);
    c.round(networkPublisher.x, networkPublisher.y, networkPublisher.w, networkPublisher.h, 12, STAGE);
    c.centered(350, 278, "Computer pairing", fontSmall, INK);
    } else {
    c.text(72, 136, "Wi-Fi connects independently. Pair your Windows publisher here.", fontSmall, MUTED);
    networkField(c, networkFields[2], "Publisher IP address", network.host,
                 network.field == PUBLISHER_HOST, false);
    networkField(c, networkFields[3], "Pairing key", network.key, network.field == PAIRING_KEY, true);
    networkField(c, networkFields[4], "Port", network.port, network.field == PUBLISHER_PORT, false);
    }
    bool enabled = !network.scanning && network.ready() && (!network.publisherPage || network.publisherReady());
    c.round(networkSave.x, networkSave.y, networkSave.w, networkSave.h, 12, enabled ? MINT : STAGE);
    c.centered(networkSave.x + networkSave.w / 2, 278, network.publisherPage ? "Save pairing" : "Connect", fontSmall, enabled ? INK : MUTED);
    c.round(networkForget.x, networkForget.y, networkForget.w, networkForget.h, 12, STAGE);
    c.centered(networkForget.x + networkForget.w / 2, 278, "Forget", fontSmall, MUTED);
    for (int i = 0; i < 26; ++i) {
        Box b = networkKeyBox(i);
        char label[] = {network.keyLabel(i), 0};
        c.round(b.x, b.y, b.w, b.h, 11, WHITE);
        c.centered(b.x + b.w / 2, b.y + 12, label, fontSmall, INK);
    }
    c.round(networkSpace.x, networkSpace.y, networkSpace.w, networkSpace.h, 11, WHITE);
    c.centered(networkSpace.x + networkSpace.w / 2, 516, "space", fontSmall, INK);
    c.round(networkBack.x, networkBack.y, networkBack.w, networkBack.h, 11, STAGE);
    c.centered(networkBack.x + networkBack.w / 2, 516, "delete", fontSmall, INK);
    c.round(networkDot.x, networkDot.y, networkDot.w, networkDot.h, 11, WHITE);
    c.centered(networkDot.x + networkDot.w / 2, 516, ".", fontBody, INK);
    c.round(networkDash.x, networkDash.y, networkDash.w, networkDash.h, 11, WHITE);
    c.centered(networkDash.x + networkDash.w / 2, 516, "-", fontBody, INK);
    c.round(networkKeyboard.x, networkKeyboard.y, networkKeyboard.w, networkKeyboard.h, 11, STAGE);
    c.centered(networkKeyboard.x + networkKeyboard.w / 2, 516, network.keyboardName(), fontSmall, INK);
    c.text(76, 573, network.publisherPage ? "Use the IPv4 address and pairing key from your Windows publisher." :
           "2.4 GHz networks only. Back returns to Pip; Connect saves your network.",
           fontSmall, MUTED);
    if (network.dropdown) {
        c.round(72, 170, 928, 378, 16, STAGE);
        if (network.scanning || !network.networkCount) {
            c.text(96, 198, network.scanning ? "Scanning..." : network.scanFailed ?
                   "Scan failed. Go Back and tap Refresh." : "No networks found. Go Back and tap Refresh.", fontBody, INK);
        } else {
            for (int row = 0; row < 5; ++row) {
                int index = network.networkPage * 5 + row;
                if (index >= network.networkCount) break;
                const NetworkUi::AccessPoint &ap = network.networks[index];
                Box b = networkRow(row);
                c.round(b.x, b.y, b.w, b.h, 10, WHITE);
                masked(c, b.x + 12, b.y + 16, ap.name, false, INK);
                c.text(660, b.y + 16, ap.signal > -60 ? "Strong" : ap.signal > -75 ? "Good" : "Weak", fontSmall, MUTED);
                c.text(790, b.y + 16, ap.secured ? "Password" : "Open", fontSmall, MUTED);
            }
            c.round(networkPrev.x, networkPrev.y, networkPrev.w, networkPrev.h, 10, WHITE);
            c.centered(162, 498, "Previous", fontSmall, INK);
            c.round(networkNext.x, networkNext.y, networkNext.w, networkNext.h, 10, WHITE);
            c.centered(318, 498, "Next", fontSmall, INK);
        }
    }
}
} // namespace pip
