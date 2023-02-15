#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "WiseComm.h"

using namespace std;

int main(int argc, char** argv) {
    string ip_addr = argv[1];

    WiseComm wise;
    wise.connect(ip_addr, 502);

    fd_set rfds;

    while (true) {
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        timeval tv{.tv_sec = 0, .tv_usec = 10000};
        int ret = select(1, &rfds, NULL, NULL, &tv);

        if (ret) {
            string str;
            getline(cin, str);

            if (str == "q") {
                break;
            }
            try {
                int num = stoi(str);
                wise.writeRelay0((num & 0x01) > 0);
                wise.writeRelay1((num & 0x02) > 0);
            } catch (...) {
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

    cout << "Hello world" << endl;

    return 0;
}
