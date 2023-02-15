#ifndef __SENSE_MODEL_H__
#define __SENSE_MODEL_H__

#include <iostream>
#include <opencv2/core.hpp>

#include "realsense.h"
#include "SPAData.h"

class SenseCamera {
   public:
    SenseCamera();
    ~SenseCamera();

    int cameraInit(SenseData* senseData);
    void cameraRun();
    void showImgs();
    void getImgs(SenseData* senseData);

   private:
    realsense::RealSense rs;

    std::vector<cv::Mat> rgb;
    std::vector<cv::Mat> depth;
    std::vector<cv::Mat> xyz;
};

#endif
