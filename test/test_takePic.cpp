#include <iostream>
#include <opencv2/opencv.hpp>
#include "realsense.h"
#include <string>

using namespace std;
using namespace cv;
using namespace realsense;

string getFileName();

int main(int argc, char** argv) {

    RealSense rs;
    Config config;

    Mat realsenseRgb;
    Mat depth_image;

    string fileName = "";
    char kbin;
    bool run = true;



    // RealSense init
    cout << "RealSenseInit" << endl;
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

    while (run) {

        rs.update();
        rs.retrieve_color_image(realsenseRgb);
        rs.retrieve_depth_image(depth_image);

        imshow("pictureRgb", realsenseRgb);
        imshow("pictureDepth", depth_image);

            kbin = cv::waitKey(1);
            switch (kbin) {
                case 27:
                    run = false;
                    break;
                case 'c':
                    fileName = getFileName();
                cv::imwrite("./pic/" + fileName, depth_image);
                    //cv::imwrite("./pic/depth_" + fileName, depth_image_ocv);
                    cout << "Write: " << fileName << endl;
                    //cout << "Write: depth_" << fileName << endl;
                    break;
            }
    }
    return 0;
}

string getFileName() {
    auto now = chrono::system_clock::now();
    time_t time = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&time), "%y%m%d_%H%M%S");
    uint64_t microSec =
        chrono::duration_cast<chrono::microseconds>(now.time_since_epoch())
            .count();
    int micro = microSec % 1000000;
    ss << "_" << setfill('0') << std::setw(6) << micro;

    return "Pic_" + ss.str() + ".jpg";
}
