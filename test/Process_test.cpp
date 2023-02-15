#include <iostream>
#include "Process.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "parameter:<path of yaml>" << endl;
        return -1;
      }

    //YAML::Node config = YAML::LoadFile(argv[1]);

    Process process("192.168.0.1", 502, "192.168.0.100", argv[1]);//armIp,armIpPort,wiseIp

    process.ProcessInit();
    process.ProcessRun();



   return 0;
}
