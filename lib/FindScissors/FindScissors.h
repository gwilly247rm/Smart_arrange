#ifndef __FINDSCISSORS_H__
#define __FINDSCISSORS_H__

#include <iostream>
#include <opencv2/core.hpp>
#include <yaml-cpp/yaml.h>
#include "SPAData.h"

class FindScissors {
    public:
        FindScissors();
        ~FindScissors();
        void Init(const YAML::Node& Node);
        void getImg(SenseData& senseData);
        void process(PlanData& planData);

        //test
        std::vector<std::vector<std::vector<cv::Point>>> returnContours();
        std::vector<std::vector<std::vector<cv::Point>>> contoursCopy;
        std::vector<std::vector<cv::Rect>> returnRect();
        std::vector<std::vector<cv::Rect>> rectCopy;

    private:
        typedef struct HSV {
            int H_high;
            int S_high;
            int V_high;
            int H_low;
            int S_low;
            int V_low;
        }HSV;

        typedef struct Scissor {
            cv::Rect blade;
            cv::Rect handle;
        }Scissor;

        cv::Mat fileterBackground(cv::Mat img, HSV hsv);
        std::vector<cv::Rect> findScissorsPos(cv::Mat Img, int item);
        void calSuckPos();

        //parameter
        int kernel_type;
        int kernel_size;
        int bladeContourArea_high;
        int bladeContourArea_low;
        int handleContourArea_high;
        int handleContourArea_low;
        int contourArea_high;
        int contourArea_low;
        double suckOffset;
        cv::Mat realSenseRgb;
        cv::Mat realsenseXyz;
        cv::Mat realSenseRgbDraw;
        HSV ScissorsHandlePurple;
        HSV ScissorsHandleBlue;
        HSV ScissorsHandleBlack;
        HSV ScissorsBlade;

        //use for class
        std::vector<cv::Rect> Blade;
        std::vector<cv::Rect> HandlePurple;
        std::vector<cv::Rect> HandleBlue;
        std::vector<cv::Rect> HandleBlack;
        std::vector<cv::Rect> Handle;
        std::vector<Scissor> scissor;
        std::vector<cv::Point> suckPoint;
        std::vector<bool> sccissorSituation;//0 mean handle down,1 mean handle up
};

#endif
