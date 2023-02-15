#include <iostream>
#include "Process.h"

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "parameter:<path of yaml>" << endl;
        return -1;
    }
    
    YAML::Node config = YAML::LoadFile(argv[1]);
    
    
    /*ofstream fout(argv[1]);
    fout << config;
    fout.close();
    
    config = YAML::LoadFile(argv[1]);*/
    
    return 0;
}
