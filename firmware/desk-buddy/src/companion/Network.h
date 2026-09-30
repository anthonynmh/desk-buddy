// Original Pip app, MIT license. See licenses/LICENSE-Pip.txt.
#pragma once
#include <stdint.h>
#include <string.h>

namespace pip {
enum NetworkField { WIFI_NAME, WIFI_PASSWORD, PUBLISHER_HOST, PAIRING_KEY, PUBLISHER_PORT };
enum NetworkStatus { NETWORK_UNCONFIGURED, NETWORK_CONNECTING, NETWORK_ONLINE, NETWORK_ERROR };
struct NetworkUi {
    struct AccessPoint { char name[33] = {}; int signal = -100; bool secured = true; };
    AccessPoint networks[12];
    int networkCount = 0, networkPage = 0;
    bool dropdown = false, scanRequested = false, scanning = false, scanFailed = false;
    bool secured = true, publisherPage = false;
    void addNetwork(const char *name, int signal, bool secure) {
        if (!name[0] || strlen(name) > 32) return;
        int index = networkCount;
        for (int i = 0; i < networkCount; ++i)
            if (!strcmp(networks[i].name, name)) { index = i; break; }
        if (index < networkCount && networks[index].signal >= signal) return;
        if (index == networkCount) {
            if (networkCount == 12) {
                if (signal <= networks[11].signal) return;
                index = 11;
            } else ++networkCount;
        }
        strcpy(networks[index].name, name);
        networks[index].signal = signal;
        networks[index].secured = secure;
        while (index > 0 && networks[index].signal > networks[index - 1].signal) {
            AccessPoint swap = networks[index - 1];
            networks[index - 1] = networks[index]; networks[index] = swap; --index;
        }
    }
    void chooseNetwork(int index) {
        if (index < 0 || index >= networkCount || scanning) return;
        if (strcmp(ssid, networks[index].name)) password[0] = 0;
        strcpy(ssid, networks[index].name);
        secured = networks[index].secured;
        if (!secured) password[0] = 0;
        dropdown = false; field = WIFI_PASSWORD;
    }
    bool open = false, saveRequested = false, forgetRequested = false;
    bool lowerCase = false, symbols = false, moreSymbols = false;
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
        if (field == WIFI_NAME) return;
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
        static const char *more = "+=:;,'\"?<>\\|~`(){}[]!@#$%^";
        const char *keys = symbols ? (moreSymbols ? more : symbol) : (lowerCase ? lower : upper);
        return index >= 0 && unsigned(index) < strlen(keys) ? keys[index] : ' ';
    }
    void nextKeyboard() {
        if (symbols && moreSymbols) { symbols = false; moreSymbols = false; lowerCase = false; }
        else if (symbols) { symbols = false; lowerCase = true; }
        else if (lowerCase) { lowerCase = false; symbols = true; moreSymbols = true; }
        else symbols = true;
    }
    const char *keyboardName() const { return symbols ? (moreSymbols ? "#+=" : "123") :
                                                 (lowerCase ? "abc" : "ABC"); }
    bool ready() const { return ssid[0] && (!secured || strlen(password) >= 8); }
    bool publisherReady() const { return host[0] && key[0] && portNumber(port); }
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
