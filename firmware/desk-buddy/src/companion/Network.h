// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#pragma once
#include <stdint.h>
#include <string.h>

namespace pip {
enum NetworkField { WIFI_NAME, WIFI_PASSWORD, PUBLISHER_HOST, PAIRING_KEY, PUBLISHER_PORT };
enum NetworkStatus { NETWORK_UNCONFIGURED, NETWORK_CONNECTING, NETWORK_ONLINE, NETWORK_ERROR };
struct NetworkUi {
    bool open = false, saveRequested = false, forgetRequested = false;
    bool lowerCase = false, symbols = false;
    NetworkField field = WIFI_NAME;
    NetworkStatus status = NETWORK_UNCONFIGURED;
    bool publisherOnline = false;
    uint32_t changedAt = 0, lastPublisherAt = 0;
    char ssid[33] = {}, password[65] = {}, host[64] = {}, key[33] = {}, port[6] = "8791";
    char *value() {
        switch (field) {
        case WIFI_NAME: return ssid;
        case WIFI_PASSWORD: return password;
        case PUBLISHER_HOST: return host;
        case PAIRING_KEY: return key;
        default: return port;
        }
    }
    unsigned capacity() const {
        switch (field) {
        case WIFI_NAME: return sizeof(ssid);
        case WIFI_PASSWORD: return sizeof(password);
        case PUBLISHER_HOST: return sizeof(host);
        case PAIRING_KEY: return sizeof(key);
        default: return sizeof(port);
        }
    }
    void append(char ch) {
        char *text = value();
        unsigned length = strlen(text);
        if (ch >= 32 && ch <= 126 && length + 1 < capacity()) {
            text[length] = ch;
            text[length + 1] = 0;
        }
    }
    void erase() {
        char *text = value();
        unsigned length = strlen(text);
        if (length) text[length - 1] = 0;
    }
    void select(NetworkField next) { field = next; }
    char keyLabel(int index) const {
        static const char *upper = "QWERTYUIOPASDFGHJKLZXCVBNM";
        static const char *lower = "qwertyuiopasdfghjklzxcvbnm";
        static const char *symbol = "1234567890!@#$%^&*()[]{}_/";
        return symbols ? symbol[index] : (lowerCase ? lower[index] : upper[index]);
    }
    void nextKeyboard() {
        if (symbols) { symbols = false; lowerCase = true; }
        else if (lowerCase) lowerCase = false;
        else symbols = true;
    }
    const char *keyboardName() const { return symbols ? "123" : (lowerCase ? "abc" : "ABC"); }
    bool ready() const { return ssid[0] && host[0] && key[0] && port[0]; }
    static uint16_t portNumber(const char *text) {
        uint32_t value = 0;
        if (!*text) return 0;
        for (; *text; ++text) {
            if (*text < '0' || *text > '9') return 0;
            value = value * 10 + unsigned(*text - '0');
            if (value > 65535) return 0;
        }
        return uint16_t(value);
    }
};
} // namespace pip
