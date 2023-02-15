#ifndef __WISECOMM_H__
#define __WISECOMM_H__

#include <memory>
#include <string>
//
#include <modbus.h>
//
#include "WiseModbusException.h"

class WiseComm {
   public:
    WiseComm();
    ~WiseComm();

    void connect(const std::string& ip, int port = 502);

    bool isConnected();

    bool readDigitalInput0();

    bool readDigitalInput1();

    int readDigitalInputAll();

    void writeRelay0(bool value);

    void writeRelay1(bool value);

    void writeRelayAll(int value);

    bool readRelay0();

    bool readRelay1();

    int readRelayAll();

   private:
    std::shared_ptr<modbus_t> ctx_;
    bool connected;
    std::string ip;
    int port;

    int maxRetry;

    void reconnect();

    void reconnectIfNeeded();

};

#endif
