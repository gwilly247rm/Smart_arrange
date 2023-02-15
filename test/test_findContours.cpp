#include <iostream>
#include <string>

#include "realsense.h"
#include "yaml-cpp/yaml.h"

using namespace std;
using namespace cv;
using namespace realsense;

struct AreaCmp {
    AreaCmp(const vector<float>& _areas)
        : areas(&_areas)
    {
    }
    bool operator()(int a, int b) const { return (*areas)[a] > (*areas)[b]; }
    const vector<float>* areas;
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        cout << "parameter:<path of yaml>" << endl;
        return -1;
    }

    Mat realSenseRgb;
    Mat img_edge;
    Mat img_color;

    RealSense rs;
    Config config;

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

    while (1) {
        rs.update();
        rs.retrieve_color_image(realSenseRgb);

        cv::threshold(realSenseRgb, img_edge, 80, 200, cv::THRESH_BINARY);
        cv::imshow("Image after threshold", img_edge);
        cv::cvtColor(img_edge, img_edge, cv::COLOR_BGR2GRAY);
        vector<vector<cv::Point>> contours;
        vector<cv::Vec4i> hierarchy;

        cv::findContours(img_edge, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
        cout << "\n\nHit any key to draw the next contour, ESC to quit\n\n";
        cout << "Total Contours Detected: " << contours.size() << endl;

        vector<int> sortIdx(contours.size());
        vector<float> areas(contours.size());
        for (int n = 0; n < (int)contours.size(); n++) {
            sortIdx[n] = n;
            areas[n] = contourArea(contours[n], false);
        }

        // sort contours so that the largest contours go first

        std::sort(sortIdx.begin(), sortIdx.end(), AreaCmp(areas));

        for (int n = 0; n < (int)sortIdx.size(); n++) {
            int idx = sortIdx[n];
            cv::cvtColor(realSenseRgb, img_color, cv::COLOR_BGR2GRAY);
            cv::drawContours(img_color, contours, idx, cv::Scalar(0, 0, 255), 2, 1, hierarchy,
                0 // Try different values of max_level, and see what happens
            );
            cout << "Contour #" << idx << ": area=" << areas[idx] << ", nvertices=" << contours[idx].size() << endl;

            // find boundingRect
            vector<Rect> boundRect(1);
            boundRect[0] = boundingRect(contours[idx]);
            rectangle(realSenseRgb, boundRect[0].tl(), boundRect[0].br(), Scalar(200, 200, 200), 2);
            cout << "Point: " << boundRect[0].tl() << boundRect[0].br() << endl;
            // circle(img_color, Point(205, 200), 0 ,Scalar(200, 100, 200), 5);
            cv::imshow("img", realSenseRgb);
            int k;
            if ((k = cv::waitKey() & 255) == 27)
                break;
        }

        return 0;
    }
}
