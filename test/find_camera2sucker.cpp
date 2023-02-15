#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <opencv2/core.hpp>
#include "yaml-cpp/yaml.h"
#include "HiwinSDK.h"
#include "SPAData.h"
#include "SenseCamera.h"

using namespace std;
using namespace cv;
using namespace realsense;

void onMouse(int event, int x, int y, int flags, void* ptr);

struct xyz {
    double x = 0;
    double y = 0;
    double z = 0;
  };

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "parameter:<path of yaml>" << endl;
        return -1;
      }

    YAML::Node node = YAML::LoadFile(argv[1]);

    HiwinSDK hiwin;
    RealSense rs;

    xyz realxyz;
    xyz camera2sucker;

    Mat realsenseRgb,realsenseXyz;
    Point previousPoint = Point(0,0);
    Point zero = Point(0,0);
    Point draw;
    char key;
    bool keep = 1;
    double input = 1;
    double regulatePos[6] = {0};
    vector<double> armMoveValue;
    armMoveValue.clear();
    armMoveValue.resize(6);

    //arm init
    cout << "-----Arm init-----" << endl;

    double basePos[6] = {0, 0, 0, 0, 0, 0};
    double toolPos[6] = {0, 0, 0, 0, 0, 0};
    double homePos[6] = {0, 0, 0, 0, -90, 0};
    //double goPos[6] = {150, 0, 0, 0, 0, 0};

    hiwin.connect("192.168.0.1", 502);

    //hiwin.errorReset();
    hiwin.setServoState(1);                                 //servo state 1=on
    hiwin.setOperationMode(0);

    hiwin.setActiveBaseNum(3);
    hiwin.setCurrentBaseCoord(basePos);
    hiwin.setActiveToolNum(3);
    hiwin.setCurrentToolCoord(toolPos);
    hiwin.setOperationMode(1);                              // 1=run
    hiwin.setAcceleration(node["arm"]["acceleration"].as<int>());
    hiwin.setFeedRate(node["arm"]["feedrate"].as<int>());
    hiwin.setPTPSpeed(node["arm"]["PTPspeed"].as<int>());
    sleep(0.1);

    hiwin.movePtpAbs(homePos, HiwinSDK::CoordType::Joint);  //arm go home
    hiwin.waitForIdle();

    /*hiwin.movePtpRel(goPos, HiwinSDK::CoordType::Coord);
    hiwin.waitForIdle();*/

    //arm init end

    //camera init
    cout << "-----Sense init-----" << endl;

    Config config;
    /*config.device_type = DeviceType::CAMERA_ID;
    config.camera_id = 0;*/
    config.device_type = DeviceType::SERIAL_ID;
    config.serial_id = "f0090172";

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

    rs.set_color_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, 1);

    if (rs.depth_supports(RS2_OPTION_MIN_DISTANCE))
    rs.set_depth_option(RS2_OPTION_MIN_DISTANCE, 100);

    /*if (rs.depth_supports(RS2_OPTION_MAX_DISTANCE))
    rs.set_depth_option(RS2_OPTION_MAX_DISTANCE, 400);*/

    rs.set_align_stream(RS2_STREAM_COLOR);

    rs.enable_hole_filling_filter(true);
    rs.set_hole_filling_filter_option(RS2_OPTION_HOLES_FILL, 1);
    //camera init end

    cout << "press mouse on picture make arm goto the point" << endl;
    cout << "press Esc can quit" << endl;

    while(1) {
        rs.update();
        rs.retrieve_color_image(realsenseRgb);
        imshow("realsenseRgb",realsenseRgb);
        setMouseCallback("realsenseRgb", onMouse, &previousPoint);
        if(waitKey(1) == 27) {
            keep = 0;
            break;
          }

        if(previousPoint != zero) {
            rs.retrieve_xyz_measure(realsenseXyz);
            realxyz.x = realsenseXyz.at<Vec3f>(previousPoint.y, previousPoint.x)[0];
            realxyz.y = realsenseXyz.at<Vec3f>(previousPoint.y, previousPoint.x)[1];
            realxyz.z = realsenseXyz.at<Vec3f>(previousPoint.y, previousPoint.x)[2];
            draw = Point(previousPoint.x,previousPoint.y);
            cout << "realxyz: x[ " << realxyz.x << " ], y[ "  << realxyz.y << " ], z[ "  << realxyz.z << " ]" <<endl;

            circle(realsenseRgb, draw, 5, Scalar(0, 0, 255), -1);
            imshow("realsenseRgb",realsenseRgb);

            cout << "press q then arm will move" << endl;
            cout << "press any key without q can retry" << endl;

            key = waitKey(0);

            if(key == 'q')
                break;
            else
                previousPoint = Point(0,0);
          }
      }

    camera2sucker.x = node["realsense"]["move2suckerX"].as<double>();
    camera2sucker.y = node["realsense"]["move2suckerY"].as<double>();
    camera2sucker.z = node["realsense"]["move2suckerZ"].as<double>();

    if(keep == 1) {
        armMoveValue = {realxyz.x*1000 + camera2sucker.x, realxyz.y*-1000 + camera2sucker.y, -realxyz.z*1000 + camera2sucker.z + 30, 0, 0, 0};
        //armMoveValue = {realxyz.x*1000, realxyz.y*-1000, -realxyz.z*1000 + camera2sucker.z + 30, 0, 0, 0};

        for(size_t i = 0; i < armMoveValue.size(); i++)
            cout <<  "armMoveValue[" << i << "]:" <<  armMoveValue[i] << endl;

        hiwin.move(armMoveValue, HiwinSDK::CtrlType::PTP, HiwinSDK::MoveType::Relative, HiwinSDK::CoordType::Coord);
        hiwin.waitForIdle();

        armMoveValue = {0, 0, -30, 0, 0, 0};

        for(size_t i = 0; i < armMoveValue.size(); i++)
            cout <<  "armMoveValue[" << i << "]:" <<  armMoveValue[i] << endl;

        hiwin.move(armMoveValue, HiwinSDK::CtrlType::PTP, HiwinSDK::MoveType::Relative, HiwinSDK::CoordType::Coord);
        hiwin.waitForIdle();

        while(1) {
            rs.update();
            rs.retrieve_color_image(realsenseRgb);
            imshow("realsenseRgb",realsenseRgb);

            armMoveValue = {0, 0, 0, 0, 0, 0};
            cout << "input:" << input << endl;
            key = waitKey(0);

            if (key == 'q')
                break;

            else if (key == 'd') {
                armMoveValue = {input, 0, 0, 0, 0, 0};
                regulatePos[0] = regulatePos[0] + input;
              }

            else if (key == 'a') {
                armMoveValue = {-input, 0, 0, 0, 0, 0};
                regulatePos[0] = regulatePos[0] - input;
              }

            else if (key == 'w') {
                armMoveValue = {0, input, 0, 0, 0, 0};
                regulatePos[1] = regulatePos[1] + input;
              }

            else if (key == 's') {
                armMoveValue = {0, -input, 0, 0, 0, 0};
                regulatePos[1] = regulatePos[1] - input;
              }

            else if (key == 'r') {
                armMoveValue = {0, 0, input, 0, 0, 0};
                regulatePos[2] = regulatePos[2] + input;
              }

            else if (key == 'f') {
                armMoveValue = {0, 0, -input, 0, 0, 0};
                regulatePos[2] = regulatePos[2] - input;
              }

            else if (key == 'j') {
                input = input * 2;
              }

            else if (key == 'k') {
                input = input / 2;
              }

            else
                cout << "error key" << endl;

            cout << "x:" << regulatePos[0] << ", y:" << regulatePos[1] << ", z:" << regulatePos[2] << endl;
            cout << "press q to end" << endl;
            hiwin.move(armMoveValue, HiwinSDK::CtrlType::PTP, HiwinSDK::MoveType::Relative, HiwinSDK::CoordType::Coord);
            hiwin.waitForIdle();
          }

        node["realsense"]["move2suckerX"] = node["realsense"]["move2suckerX"].as<double>() + regulatePos[0];
        node["realsense"]["move2suckerY"] = node["realsense"]["move2suckerY"].as<double>() + regulatePos[1];
        node["realsense"]["move2suckerZ"] = node["realsense"]["move2suckerZ"].as<double>() + regulatePos[2];

        ofstream fout(argv[1]);
        fout << node;
        fout.close();
        keep = 0;
      }

    //arm go home
    hiwin.movePtpAbs(homePos, HiwinSDK::CoordType::Joint);
    hiwin.waitForIdle();

    hiwin.setOperationMode(0);
    hiwin.setServoState(0);

    sleep(0.1);

    hiwin.disconnect();

    return 0;
  }

void onMouse(int event, int x, int y, int flags, void* ptr) {
    Point* p = (Point*)ptr;

    if (event == EVENT_LBUTTONDOWN) *p = Point(x, y);
  }
