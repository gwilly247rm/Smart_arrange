#include "FindScissors.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include "yaml-cpp/yaml.h"
#include "SPAData.h"
#include "SenseCamera.h"
#define OPENCV

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
    //set up
    SenseCamera camera;
    SenseData senseData;
    PlanData planData;
    FindScissors findScissors;
    int iResult;
    Mat realSenseRgb;
    std::vector<std::vector<std::vector<cv::Point>>> contoursCopy;
    
    if (argc < 2) {
        cout << "parameter:<path of yaml> <path of picture>(select)" << endl;
        return -1;
    }
    else if (argc == 2) {
        cout << "-----Sense init-----" << endl;
        iResult = camera.cameraInit(&senseData);
        if (iResult != 0)
            cout << "Sense Init failed" << endl;
    }
    else if (argc == 3) {
        senseData.rgb.push_back(imread(argv[2]));
    }
    
    YAML::Node node = YAML::LoadFile(argv[1]);
    
    findScissors.Init(node);
    
    while (1) {
        if (argc == 2) {
            //get camera image
            camera.cameraRun();
            camera.getImgs(&senseData);
            realSenseRgb = senseData.rgb[0].clone();
        }

        findScissors.getImg(senseData);
        findScissors.process(planData);
        
        if (waitKey(1) == 27)
            break;
    }
    
    contoursCopy = findScissors.returnContours();
    
    return 0;
}
