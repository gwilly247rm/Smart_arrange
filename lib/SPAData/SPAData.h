#ifndef __SPADATA_H__
#define __SPADATA_H__

#include <cstdlib>  //needed by HiwinSDK.h
#include <opencv2/core.hpp>
#include <vector>
#include "HiwinSDK.h"

// Sense
struct SenseData {
    std::vector<cv::Mat> rgb;
    std::vector<cv::Mat> depth;
    std::vector<cv::Mat> xyz;
};

// Plan->Act
struct Robot {
    HiwinSDK::CoordType coordType;
    HiwinSDK::MoveType moveType;
    HiwinSDK::CtrlType ctrlType;

    std::vector<double> value;
};

// Plan->Act
struct Sucker {
    bool suck;
};

struct Action {
    Robot arm;
    Sucker sucker;
};

struct PlanData {
    std::vector<cv::Point2f> sccissorTarget;
    std::vector<double> sccissorDepth;
    std::vector<bool> sccissorSituation;//0 mean handle down,1 mean handle up

    std::vector<cv::Point2f> holeTarget;
    std::vector<double> holeDepth;
    std::vector<int> holeSituation;//0 mean handle down,1 mean handle up

    Action action;
};

// Act
struct ActData {
    std::vector<double> currentPos;
    bool checkSuck;
};

#endif
