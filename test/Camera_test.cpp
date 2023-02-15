#include <stdlib.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "SPAData.h"
#include "SenseCamera.h"

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
    cout << "-----Sense init-----" << endl;
    SenseCamera camera;
    SenseData senseData;
    int iResult;
    Mat realSenseRgb;
    Mat realSenseDepth;
    Mat realSenseDepthGray;
    vector<cv::Mat>bgr;

    iResult = camera.cameraInit(&senseData);
    if (iResult != 0) cout << "Sense Init failed" << endl;

    while (1) {
        camera.cameraRun();
        camera.getImgs(&senseData);
        //camera.showImgs();

        realSenseRgb = senseData.rgb[0].clone();
        realSenseDepth = senseData.depth[0].clone();

        split(realSenseDepth,bgr);
        imshow("b", bgr[0]);
        imshow("g", bgr[1]);
        imshow("r", bgr[2]);

        cvtColor(realSenseDepth, realSenseDepthGray, cv::COLOR_BGR2GRAY);

        imshow("realSenseRgb", realSenseRgb);
        imshow("realSenseDepth", realSenseDepth);
        imshow("realSenseDepth", realSenseDepthGray);

        if (waitKey(1) == 27) break;
    }

    return 0;
}
