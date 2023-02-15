#include "WiseComm.h"

#include <bits/stdint-uintn.h>
#include <sys/time.h>
#include <iostream>
#include <stdexcept>

#include <modbus.h>

#define MODBUS_CHECK(ret, flag)                            \
    if ((ret) == -1) {                                     \
        flag = false;                                      \
        const char* error_msg = modbus_strerror(errno);    \
        throw WiseModbusException("Wise modbus Error: " +  \
                                  std::string(error_msg)); \
    }

#define THROW_WISE_MODBUS_ERROR()                          \
    {                                                      \
        const char* error_msg = modbus_strerror(errno);    \
        throw WiseModbusException("Wise modbus Error: " +  \
                                  std::string(error_msg)); \
    }

#define MODBUS_TRY(func_, reconn_, maxRetry_, flag_)              \
    {                                                             \
        bool done = false;                                        \
        for (int i = 0; i < maxRetry_ && !done; ++i) {            \
            reconn_();                                            \
            int ret = func_;                                      \
            if (ret == -1) {                                      \
                flag_ = false;                                    \
                std::cout << "Wise modbus retrying" << std::endl; \
            } else {                                              \
                done = true;                                      \
                break;                                            \
            }                                                     \
        }                                                         \
        if (!done) {                                              \
            THROW_WISE_MODBUS_ERROR();                            \
        }                                                         \
    }

WiseComm::WiseComm()
    : ctx_(nullptr), connected(false), ip(), port(), maxRetry(10) {}

WiseComm::~WiseComm() {}

void WiseComm::connect(const std::string& ip, int port) {
    this->ip = ip;
    this->port = port;
    ctx_ = std::shared_ptr<modbus_t>(modbus_new_tcp(ip.c_str(), port),
                                     modbus_free);
#if LIBMODBUS_VERSION_CHECK(3, 1, 1)
    modbus_set_response_timeout(ctx_.get(), 3, 0);
#else
    timeval timeout{.tv_sec = 3, .tv_usec = 0};
    modbus_set_response_timeout(ctx_.get(), &timeout);
#endif
    MODBUS_CHECK(modbus_connect(ctx_.get()), connected);
    connected = true;
}

bool WiseComm::isConnected() { return connected; }

bool WiseComm::readDigitalInput0() {
    uint8_t val = 0;

    MODBUS_TRY(modbus_read_bits(ctx_.get(), 0, 1, &val), reconnectIfNeeded,
               maxRetry, connected);

    return val > 0;
}

bool WiseComm::readDigitalInput1() {
    uint8_t val = 0;
    MODBUS_TRY(modbus_read_bits(ctx_.get(), 1, 1, &val), reconnectIfNeeded,
               maxRetry, connected);
    return val > 0;
}

int WiseComm::readDigitalInputAll() {
    uint16_t value = 0;
    MODBUS_TRY(modbus_read_registers(ctx_.get(), 300, 1, &value),
               reconnectIfNeeded, maxRetry, connected);
    return value;
}

void WiseComm::writeRelay0(bool value) {
    MODBUS_TRY(modbus_write_bit(ctx_.get(), 16, value), reconnectIfNeeded,
               maxRetry, connected);
}

void WiseComm::writeRelay1(bool value) {
    MODBUS_TRY(modbus_write_bit(ctx_.get(), 17, value), reconnectIfNeeded,
               maxRetry, connected);
}

void WiseComm::writeRelayAll(int value) {
    uint16_t val = value;
    MODBUS_TRY(modbus_write_register(ctx_.get(), 302, val), reconnectIfNeeded,
               maxRetry, connected);
}

bool WiseComm::readRelay0() {
    uint8_t value = 0;
    MODBUS_TRY(modbus_read_bits(ctx_.get(), 16, 1, &value), reconnectIfNeeded,
               maxRetry, connected);
    return value > 0;
}

bool WiseComm::readRelay1() {
    uint8_t value = 0;
    MODBUS_TRY(modbus_read_bits(ctx_.get(), 17, 1, &value), reconnectIfNeeded,
               maxRetry, connected);
    return value > 0;
}

int WiseComm::readRelayAll() {
    uint16_t value = 0;
    MODBUS_TRY(modbus_read_registers(ctx_.get(), 302, 1, &value),
               reconnectIfNeeded, maxRetry, connected);
    return value;
}

void WiseComm::reconnect() {
    ctx_ = std::shared_ptr<modbus_t>(modbus_new_tcp(ip.c_str(), port),
                                     modbus_free);
#if LIBMODBUS_VERSION_CHECK(3, 1, 1)
    modbus_set_response_timeout(ctx_.get(), 3, 0);
#else
    timeval timeout{.tv_sec = 3, .tv_usec = 0};
    modbus_set_response_timeout(ctx_.get(), &timeout);
#endif
    int ret = modbus_connect(ctx_.get());
    if (ret == -1) {
        connected = false;
    } else {
        connected = true;
    }
}

void WiseComm::reconnectIfNeeded() {
    if (!connected && ip.size() > 0) {
        reconnect();
    }
}
