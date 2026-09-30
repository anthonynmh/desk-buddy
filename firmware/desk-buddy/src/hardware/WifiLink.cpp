// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include "WifiLink.h"

namespace desk_buddy {
namespace {
void load(Preferences &prefs, const char *key, char *value, size_t size) {
    prefs.getString(key, value, size);
    value[size - 1] = 0;
}
void store(const pip::NetworkUi &network) {
    Preferences prefs;
    if (!prefs.begin("desk-buddy", false)) return;
    prefs.putString("ssid", network.ssid);
    prefs.putString("pass", network.password);
    prefs.putString("host", network.host);
    prefs.putString("key", network.key);
    prefs.putString("port", network.port);
    prefs.end();
}
bool receive(HTTPClient &http, pip::MediaLink &link, pip::Media &media, uint32_t now) {
    String response = http.getString();
    if (response.length() >= sizeof(link.line)) return false;
    bool accepted = false;
    for (unsigned i = 0; i < response.length(); ++i)
        accepted = link.feed(response[i], media, now) || accepted;
    return accepted;
}
bool openRequest(HTTPClient &http, WiFiClient &client, const pip::NetworkUi &network, const char *path) {
    uint16_t port = pip::NetworkUi::portNumber(network.port);
    if (!port || !http.begin(client, network.host, port, path)) return false;
    http.setConnectTimeout(1200);
    http.setTimeout(1500);
    http.addHeader("X-Desk-Buddy-Key", network.key);
    http.addHeader("Connection", "close");
    return true;
}
} // namespace

void WifiLink::begin(pip::NetworkUi &network) {
    WiFi.mode(WIFI_STA);
    Preferences prefs;
    if (!prefs.begin("desk-buddy", true)) return;
    load(prefs, "ssid", network.ssid, sizeof(network.ssid));
    load(prefs, "pass", network.password, sizeof(network.password));
    load(prefs, "host", network.host, sizeof(network.host));
    load(prefs, "key", network.key, sizeof(network.key));
    load(prefs, "port", network.port, sizeof(network.port));
    prefs.end();
    if (!network.port[0]) strcpy(network.port, "8791");
    network.secured = network.password[0] != 0;
    saved_ = network;
    if (network.ready()) connect(network, millis());
}

void WifiLink::connect(pip::NetworkUi &network, uint32_t now) {
    network.publisherOnline = false;
    network.status = pip::NETWORK_CONNECTING;
    network.changedAt = now;
    lastAttempt_ = now;
    WiFi.disconnect(false, false);
    WiFi.mode(WIFI_STA);
    WiFi.begin(saved_.ssid, saved_.password);
}

bool WifiLink::poll(pip::NetworkUi &network, pip::MediaLink &mediaLink, pip::Media &media,
                    uint32_t now) {
    WiFiClient client;
    HTTPClient http;
    if (!openRequest(http, client, saved_, "/v1/state")) return false;
    int code = http.GET();
    bool accepted = code == HTTP_CODE_OK && receive(http, mediaLink, media, now);
    http.end();
    network.publisherOnline = accepted;
    if (accepted) network.lastPublisherAt = now;
    return accepted;
}

void WifiLink::command(const pip::NetworkUi &network, char *value) {
    if (!value[0]) return;
    WiFiClient client;
    HTTPClient http;
    if (openRequest(http, client, network, "/v1/command")) {
        http.addHeader("Content-Type", "text/plain");
        http.POST((uint8_t *)value, strlen(value));
        http.end();
    }
    value[0] = 0; // A command is token-bound; never retry it after a failed request.
}

bool WifiLink::update(pip::NetworkUi &network, pip::MediaLink &mediaLink, pip::Media &media,
                      char *commandValue, uint32_t now) {
    bool changed = false;
    if (network.forgetRequested) {
        Preferences prefs;
        if (prefs.begin("desk-buddy", false)) { prefs.clear(); prefs.end(); }
        network = pip::NetworkUi();
        network.open = true;
        saved_ = pip::NetworkUi();
        media = pip::Media(); commandValue[0] = 0;
        WiFi.scanDelete();
        WiFi.disconnect(true, false);
        return true;
    }
    if (network.scanRequested && !network.scanning && network.status != pip::NETWORK_CONNECTING) {
        network.scanRequested = false;
        network.scanFailed = false;
        network.networkCount = network.networkPage = 0;
        WiFi.mode(WIFI_STA);
        WiFi.scanDelete();
        network.scanning = true;
        scanStarted_ = now;
        if (WiFi.scanNetworks(true) == WIFI_SCAN_FAILED) {
            network.scanning = false; network.scanFailed = true;
        }
        changed = true;
    }
    if (network.scanning) {
        int count = WiFi.scanComplete();
        if (count != WIFI_SCAN_RUNNING || uint32_t(now - scanStarted_) > 15000) {
            for (int i = 0; i < count && i < 256; ++i)
                network.addNetwork(WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
            network.scanning = false;
            network.scanFailed = count < 0;
            WiFi.scanDelete();
            changed = true;
        }
        return changed;
    }
    if (network.saveRequested) {
        network.saveRequested = false;
        store(network);
        bool wifiChanged = strcmp(saved_.ssid, network.ssid) || strcmp(saved_.password, network.password);
        saved_ = network;
        if (wifiChanged || WiFi.status() != WL_CONNECTED) connect(network, now);
        changed = true;
    }
    if (!saved_.ready()) return changed;
    if (WiFi.status() != WL_CONNECTED) {
        network.publisherOnline = false;
        commandValue[0] = 0;
        if (network.status == pip::NETWORK_ONLINE) {
            network.status = pip::NETWORK_ERROR; changed = true;
        }
        if (network.status == pip::NETWORK_CONNECTING && uint32_t(now - network.changedAt) > 15000) {
            network.status = pip::NETWORK_ERROR;
            changed = true;
        }
        if (!network.open && uint32_t(now - lastAttempt_) > 30000) { connect(network, now); changed = true; }
        return changed;
    }
    if (network.status != pip::NETWORK_ONLINE) { network.status = pip::NETWORK_ONLINE; changed = true; }
    if (network.open || !saved_.publisherReady()) return changed;
    command(saved_, commandValue);
    if (uint32_t(now - lastPoll_) >= 1000) {
        lastPoll_ = now;
        bool before = network.publisherOnline;
        bool accepted = poll(network, mediaLink, media, now);
        if (accepted || before != network.publisherOnline) changed = true;
    }
    return changed;
}
} // namespace desk_buddy
