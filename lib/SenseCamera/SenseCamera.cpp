#include "SenseCamera.h"

using namespace std;
using namespace cv;
using namespace realsense;

SenseCamera::SenseCamera() {}

SenseCamera::~SenseCamera() {
    rs.disconnect();
}

int SenseCamera::cameraInit(SenseData* senseData) {
    cout << "-----Sense init-----" << endl;

    Config config;
    config.device_type = DeviceType::CAMERA_ID;
    config.camera_id = 0;
    /*config.device_type = DeviceType::SERIAL_ID;
    config.serial_id = "f0090172";*/

    StreamConfig color_config;
    color_config.stream = RS2_STREAM_COLOR;
    color_config.width = 1280;
    color_config.height = 720;

    StreamConfig depth_config;
    depth_config.stream = RS2_STREAM_DEPTH;
    depth_config.width = 1024;
    depth_config.height = 768;

    StreamConfig infrared_config;
    infrared_config.stream = RS2_STREAM_INFRARED;
    infrared_config.width = 1024;
    infrared_config.height = 768;

    config.stream_configs.push_back(color_config);
    config.stream_configs.push_back(depth_config);
    config.stream_configs.push_back(infrared_config);

    cout << "Open RealSense" << endl;

    rs.connect(config);

    //rs.set_color_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, 1);

    if (rs.depth_supports(RS2_OPTION_MIN_DISTANCE))
        rs.set_depth_option(RS2_OPTION_MIN_DISTANCE, 100);

    /*if (rs.depth_supports(RS2_OPTION_MAX_DISTANCE))
        rs.set_depth_option(RS2_OPTION_MAX_DISTANCE, 400);*/

    rs.set_align_stream(RS2_STREAM_COLOR);

    rs.enable_hole_filling_filter(true);
    rs.set_hole_filling_filter_option(RS2_OPTION_HOLES_FILL, 1);

    return 0;
}

void SenseCamera::cameraRun() {
    cout << "-----Sense run-----" << endl;
    rgb.clear();
    depth.clear();
    xyz.clear();

    Mat realsenseRgb;
    Mat realsenseDepth;
    Mat realsenseXyz;

    rs.update();

    rs.retrieve_color_image(realsenseRgb);
    //rs.retrieve_depth_measure(realsenseDepth);
    rs.retrieve_depth_image(realsenseDepth);
    rs.retrieve_xyz_measure(realsenseXyz);

    rgb.push_back(realsenseRgb);
    depth.push_back(realsenseDepth);
    xyz.push_back(realsenseXyz);
}

void SenseCamera::showImgs() {
    cout << "-----Sense show-----" << endl;
    for (int i = 0; i < 1; i++) {
        string rgb_name = "rgbImg" + to_string(i);
        namedWindow(rgb_name, WINDOW_AUTOSIZE);
        imshow(rgb_name, rgb[i]);

        string depth_name = "depthImg" + to_string(i);
        namedWindow(depth_name, WINDOW_AUTOSIZE);
        imshow(depth_name, depth[i]);
    }
}

void SenseCamera::getImgs(SenseData* senseData) {
    senseData->rgb.clear();
    senseData->depth.clear();
    senseData->xyz.clear();

    for (size_t i = 0; i < 1; i++) {
        senseData->rgb.push_back(rgb[i].clone());
        senseData->depth.push_back(depth[i].clone());
        senseData->xyz.push_back(xyz[i].clone());
    }
}
