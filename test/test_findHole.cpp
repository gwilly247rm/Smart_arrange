#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "realsense.h"
#include "yaml-cpp/yaml.h"

using namespace std;
using namespace cv;
using namespace realsense;

struct AreaCmp {
  AreaCmp(const vector<float>& _areas) : areas(&_areas) {}
  bool operator()(int a, int b) const { return (*areas)[a] > (*areas)[b]; }
  const vector<float>* areas;
};

static void threshold_on_trackbar(int threshold_ratio, void*) {}

//original find box and cut depth image to find contours
int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "parameter:<path of yaml>" << endl;
        return -1;
      }

    YAML::Node node = YAML::LoadFile(argv[1]);

    RealSense rs;
    Config config;

    Mat realsenseRgb;
    Mat depth_image;
    Mat realsenseXyz;
    Mat img_edge, img_Rgb;
    Mat cropImage, cropImage_edge, img_depth;
    Mat xyz_z;

    int threshold_ratio = node["Hole"]["threshold"].as<int>();

    double maxTest;
    double minTest;
    int number_i;
    int number_j;

    vector< vector< cv::Point > > contours;
    vector< cv::Vec4i > hierarchy;

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

    StreamConfig infrared_config;
    infrared_config.stream = RS2_STREAM_INFRARED;
    infrared_config.width = 640;
    infrared_config.height = 480;

    config.stream_configs.push_back(color_config);
    config.stream_configs.push_back(depth_config);
    config.stream_configs.push_back(infrared_config);

    cout << "connect RealSense" << endl;
    rs.connect(config);

    if (rs.depth_supports(RS2_OPTION_MIN_DISTANCE))
    rs.set_depth_option(RS2_OPTION_MIN_DISTANCE, 400);
    rs.set_align_stream(RS2_STREAM_COLOR);
    rs.set_colorizer_option(RS2_OPTION_COLOR_SCHEME, 0);
    rs.enable_hole_filling_filter(true);
    rs.set_hole_filling_filter_option(RS2_OPTION_HOLES_FILL, 1);

    while (run) {

        rs.update();
        rs.retrieve_color_image(realsenseRgb);
        rs.retrieve_depth_image(depth_image);
        rs.retrieve_xyz_measure(realsenseXyz);

        threshold(realsenseRgb, img_edge, 120, 255, cv::THRESH_BINARY);
        cvtColor(img_edge, img_edge, cv::COLOR_BGR2GRAY);

        cout << "First Find Contours" << endl;
        cv::findContours(
            img_edge,
            contours,
            hierarchy,
            cv::RETR_CCOMP,
            cv::CHAIN_APPROX_SIMPLE
          );

        vector<int> sortIdx(contours.size());
        vector<float> areas(contours.size());
        for( int n = 0; n < (int)contours.size(); n++ ) {
            sortIdx[n] = n;
            areas[n] = contourArea(contours[n], false);
        }

        // sort contours so that the largest contours go first
        std::sort( sortIdx.begin(), sortIdx.end(), AreaCmp(areas));
        int idx = sortIdx[0]; //find box
        //cv::cvtColor( depth_image, img_color, cv::COLOR_GRAY2BGR );
        img_Rgb = realsenseRgb.clone();
        // find boundingRect
        vector<Rect> boundRect(1);
        boundRect[0] = boundingRect(contours[idx]);
        rectangle(img_Rgb, boundRect[0].tl(), boundRect[0].br(), Scalar(200, 200, 200), 2);
        //cout << "Point: " << boundRect[0].tl() << boundRect[0].br() << endl;
        cv::drawContours(
            img_Rgb, contours, idx,
            cv::Scalar(0,0,255), 2, 8, hierarchy,
                         0 // Try different values of max_level, and see what happens
        );

        //cut
        Rect rect(boundRect[0].tl().x, boundRect[0].tl().y, boundRect[0].br().x-boundRect[0].tl().x, boundRect[0].br().y-boundRect[0].tl().y);
        cropImage = Mat(realsenseXyz, rect);
        /*imshow("cut", cropImage);
        waitKey();*/

        //get z in cropImage
        vector<cv::Mat>xyz;
        split(cropImage,xyz);
        xyz[2].copyTo(xyz_z);

        maxTest =xyz[2].at<float>(0, 0);
        minTest =xyz[2].at<float>(0, 0);

        for(number_i = 0; number_i < xyz[2].rows; number_i++) {
            for(number_j = 0; number_j < xyz[2].cols; number_j++) {
                if(maxTest < xyz[2].at<float>(number_i, number_j))
                maxTest = xyz[2].at<float>(number_i, number_j);
                if(minTest > xyz[2].at<float>(number_i, number_j) && xyz[2].at<float>(number_i, number_j) != 0){
                    minTest = xyz[2].at<float>(number_i, number_j);
                  }
              }
          }
        // normalized
        for(number_i = 0; number_i < xyz_z.rows; number_i++) {
            for(number_j = 0; number_j < xyz_z.cols; number_j++) {
                xyz_z.at<float>(number_i, number_j) = (int)((xyz_z.at<float>(number_i, number_j) - minTest) * 255 / (maxTest-minTest));
              //cout << "zTest[" << number_i << "][" << number_j << "]:" << zTest.at<float>(number_i, number_j);
              }
          }
        xyz_z.convertTo(xyz_z, CV_8U);
        imshow("xyz_z", xyz_z);

        //findContours
        namedWindow("threshold for z", WINDOW_AUTOSIZE);  // Create Window
        createTrackbar("ratio", "threshold for z", &threshold_ratio, 255, threshold_on_trackbar);
        cv::threshold(xyz_z, cropImage_edge, threshold_ratio, 255, cv::THRESH_BINARY);

        imshow("pictureRgb", realsenseRgb);
        //imshow("cropImage", cropImage);
        imshow("threshold for z", cropImage_edge);
        if (waitKey(1) == 27) break;
    }

    node["Hole"]["threshold"] = threshold_ratio;
    ofstream fout(argv[1]);
    fout << node;
    fout.close();

    return 0;
}
