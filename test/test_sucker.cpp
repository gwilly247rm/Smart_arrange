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

    fd_set rfds;

    wise.writeRelay0(true);
    wise.writeRelay1(false);
    cout << "sucker on" << endl;

    while (true) {
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        timeval tv{.tv_sec = 0, .tv_usec = 100000};
        int ret = select(1, &rfds, NULL, NULL, &tv);

        if (ret) {
            string str;
            getline(cin, str);

            if (str == "q") {//quit
                break;
              }
            else if (str == "s") {//suck
                wise.writeRelay0(true);
                wise.writeRelay1(false);
              }
            else if (str == "b") {//blow
                wise.writeRelay0(false);
                wise.writeRelay1(true);
              }
            else if (str == "t") {//time stop
                wise.writeRelay0(false);
                wise.writeRelay1(false);
              }

          } else {
              bool di0 = wise.readDigitalInput0();
              bool di1 = wise.readDigitalInput1();
              bool do0 = wise.readRelay0();
              bool do1 = wise.readRelay1();

              cout << endl;
              cout << "DI: [" << di0 << ", " << di1 << "]";
              cout << ", DO: [" << do0 << ", " << do1 << "]";
              cout << endl;
            }
      }

    wise.writeRelay0(false);
    wise.writeRelay1(false);

    cout << "sucker off" << endl;

    return 0;
  }
