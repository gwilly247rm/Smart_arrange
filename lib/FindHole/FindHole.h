#ifndef __FINDHOLE_H__
#define __FINDHOLE_H__

#include "SPAData.h"
#include <opencv2/opencv.hpp>
# include <yaml-cpp/yaml.h>
#include <iostream>

class FindHole {
public:
    FindHole();
    ~FindHole();

    void getSenseData(SenseData* senseData);
    void run(PlanData* planData, const YAML::Node& node);
    void findBoundingRect();
    void cutBoundingRect();
    int findScissorsHome(const YAML::Node& node);
    void findHandleUpDown(PlanData* planData);
    void saveHoleTarget(PlanData* planData);
    void saveHoleDepth(PlanData* planData);

private:
    cv::Mat realSenseRgb;
    cv::Mat realSenseDepth;
    cv::Mat realSenseXyz;
    cv::Mat img_edge;
    cv::Mat img_Rgb;
    cv::Mat cropImage;
    cv::Mat xyz_z;
    cv::Mat cropImage_clone;
    cv::Mat cropImage_edge;
    cv::Mat img_depth;

    cv::Mat cropcropImage_up;
    cv::Mat cropcropImage_down;

    std::vector< std::vector< cv::Point > > contours;
    std::vector< cv::Vec4i > hierarchy;
    std::vector<int> sortIdx;
    std::vector<float> areas;
    std::vector<cv::Rect> boundRect;

    std::vector<cv::Mat>xyz;

    std::vector< std::vector< cv::Point > > cropImage_contours;
    std::vector< cv::Vec4i > cropImage_hierarchy;
    std::vector<int> cropImage_sortIdx;
    std::vector<float> cropImage_areas;
    std::vector<int> id;
    std::vector<float> shape_areas;
    std::vector<cv::Rect> cropImage_boundRect;

    std::vector<cv::Point2f> target;
    std::vector<double> depth;

    std::vector<cv::Mat>xyz_up;
    std::vector<cv::Mat>xyz_down;
    std::vector< std::vector< cv::Point > > contours_up;
    std::vector< cv::Vec4i > hierarchy_up;
    std::vector<int> sortIdx_up;
    std::vector<float> areas_up;
    std::vector< std::vector< cv::Point > > contours_down;
    std::vector< cv::Vec4i > hierarchy_down;
    std::vector<int> sortIdx_down;
    std::vector<float> areas_down;

    std::vector<int> save;

    double maxTest;
    double minTest;
    int number_i;
    int number_j;

    int idx;
    int cropImage_idx;
    int idx_up;
    int idx_down;

    struct AreaCmp {
        AreaCmp(const std::vector<float>& _areas) : areas(&_areas) {}
        bool operator()(int a, int b) const { return (*areas)[a] > (*areas)[b]; }
        const std::vector<float>* areas;
    };

};

#endif
