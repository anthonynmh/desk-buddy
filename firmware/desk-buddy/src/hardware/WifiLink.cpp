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
    Preferences prefs;
    if (!prefs.begin("desk-buddy", true)) return;
    load(prefs, "ssid", network.ssid, sizeof(network.ssid));
    load(prefs, "pass", network.password, sizeof(network.password));
    load(prefs, "host", network.host, sizeof(network.host));
    load(prefs, "key", network.key, sizeof(network.key));
    load(prefs, "port", network.port, sizeof(network.port));
    prefs.end();
    if (!network.port[0]) strcpy(network.port, "8791");
    WiFi.mode(WIFI_STA);
    if (network.ready() && pip::NetworkUi::portNumber(network.port)) connect(network, millis());
}

void WifiLink::connect(pip::NetworkUi &network, uint32_t now) {
    network.publisherOnline = false;
    network.status = pip::NETWORK_CONNECTING;
    network.changedAt = now;
    lastAttempt_ = now;
    WiFi.disconnect(false, false);
    WiFi.begin(network.ssid, network.password);
}

bool WifiLink::poll(pip::NetworkUi &network, pip::MediaLink &mediaLink, pip::Media &media,
                    uint32_t now) {
    WiFiClient client;
    HTTPClient http;
    if (!openRequest(http, client, network, "/v1/state")) return false;
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
        WiFi.disconnect(true, false);
        return true;
    }
    if (network.saveRequested) {
        network.saveRequested = false;
        store(network);
        connect(network, now);
        changed = true;
    }
    if (!network.ready() || !pip::NetworkUi::portNumber(network.port)) return changed;
    if (WiFi.status() != WL_CONNECTED) {
        if (network.status == pip::NETWORK_CONNECTING && uint32_t(now - network.changedAt) > 15000) {
            network.status = pip::NETWORK_ERROR;
            changed = true;
        }
        if (uint32_t(now - lastAttempt_) > 30000) connect(network, now);
        return changed;
    }
    if (network.status != pip::NETWORK_ONLINE) { network.status = pip::NETWORK_ONLINE; changed = true; }
    command(network, commandValue);
    if (uint32_t(now - lastPoll_) >= 1000) {
        lastPoll_ = now;
        bool before = network.publisherOnline;
        bool accepted = poll(network, mediaLink, media, now);
        if (accepted || before != network.publisherOnline) changed = true;
    }
    return changed;
}
} // namespace desk_buddy
