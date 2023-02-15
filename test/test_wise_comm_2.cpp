#include <chrono>
#include <cstdio>
#include <iostream>
#include <thread>
#include "WiseComm.h"

using namespace std;

int main(int argc, char** argv) {
    string ip_addr = "192.168.0.100";
    if (argc > 1) {
        ip_addr = string(argv[1]);
    }

    WiseComm wise;
    wise.connect(ip_addr, 502);

    for (int i = 0; i < 10; ++i) {
        bool DI0 = wise.readDigitalInput0();
        bool DI1 = wise.readDigitalInput1();
        printf("DI: [%d, %d]\n", DI0, DI1);
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    cout << "Vacuum ON" << endl;
    wise.writeRelay0(true);
    wise.writeRelay1(false);
    // wise.writeRelayAll(1);

    for (int i = 0; i < 50; ++i) {
        bool DI0 = wise.readDigitalInput0();
        bool DI1 = wise.readDigitalInput1();
        printf("DI: [%d, %d]\n", DI0, DI1);
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    cout << "Blow-off ON" << endl;
    wise.writeRelay0(false);
    wise.writeRelay1(true);
    // wise.writeRelayAll(2);

    for (int i = 0; i < 30; ++i) {
        bool DI0 = wise.readDigitalInput0();
        bool DI1 = wise.readDigitalInput1();
        printf("DI: [%d, %d]\n", DI0, DI1);
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    cout << "OFF" << endl;
    wise.writeRelay0(false);
    wise.writeRelay1(false);
    // wise.writeRelayAll(0);

    for (int i = 0; i < 30; ++i) {
        bool DI0 = wise.readDigitalInput0();
        bool DI1 = wise.readDigitalInput1();
        printf("DI: [%d, %d]\n", DI0, DI1);
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    return 0;
}
