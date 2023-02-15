#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#define OPENCV

#include "realsense.h"
#include "yolo_v2_class.hpp"

using namespace std;
using namespace cv;
using namespace realsense;

void DrawBox(Mat img, String name, bbox_t obj);

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "parameter:<path of yaml>" << endl;
        return -1;
    }

    // realsense
    RealSense rs;
    Config config;
    Mat realsenseRgb;
    Mat depth_image;
    Mat draw;

    // yolo
    Detector detect("./yolov4.cfg", "./yolov4_last.weights");

    std::vector<bbox_t> objList;

    // realsenseInit
    config.device_type = DeviceType::CAMERA_ID;
    config.camera_id = 0;

    StreamConfig color_config;
    color_config.stream = RS2_STREAM_COLOR;
    color_config.width = 1280;
    color_config.height = 720;

    StreamConfig depth_config;
    depth_config.stream = RS2_STREAM_DEPTH;
    depth_config.width = 640;
    depth_config.height = 480;

    config.stream_configs.push_back(color_config);
    config.stream_configs.push_back(depth_config);

    cout << "connect RealSense" << endl;
    rs.connect(config);

    if (rs.depth_supports(RS2_OPTION_MIN_DISTANCE))
    rs.set_depth_option(RS2_OPTION_MIN_DISTANCE, 100);
    rs.set_align_stream(RS2_STREAM_COLOR);
    rs.set_colorizer_option(RS2_OPTION_COLOR_SCHEME, 0);
    rs.enable_hole_filling_filter(true);
    rs.set_hole_filling_filter_option(RS2_OPTION_HOLES_FILL, 1);

    // run
    while (1) {
        rs.update();
        rs.retrieve_color_image(realsenseRgb);
        rs.retrieve_depth_image(depth_image);

        draw = depth_image.clone();
        objList = detect.detect(depth_image, 0.8);

        for (size_t i = 0; i < objList.size(); i++) {
            if (objList[i].obj_id == 0) {
                DrawBox(draw, "Hole", objList[i]);
            }
        }

        imshow("realsenseRgb", realsenseRgb);
        imshow("draw", draw);

        if (waitKey(1) == 27) break;
    }

    return 0;
}

void DrawBox(Mat img, String name, bbox_t obj) {
    putText(img, name + " : " + to_string((int)(obj.prob * 100)) + "%",
            Point(obj.x, obj.y), 0, 0.5, Scalar(255, 0, 0), 2);
    rectangle(img, Point(obj.x, obj.y), Point(obj.x + obj.w, obj.y + obj.h),
              Scalar(255, 0, 0), 1);
}
