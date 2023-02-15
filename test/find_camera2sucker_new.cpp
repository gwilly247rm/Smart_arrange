#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <cmath>
#include <opencv2/core.hpp>

#include "SenseCamera.h"
#include "FindHole.h"
#include "FindScissors.h"
#include "HiwinSDK.h"
#include "SPAData.h"

using namespace std;
using namespace cv;

struct xyz {
    double x;
    double y;
    double z;
  };

int main(int argc, char** argv) {
    if (argc < 3) {
        cout << "parameter:<path of yaml> <select scissor/hole (0:scissor 1:hole)> <arm go left/right first (-1:left 1:right 0:no move)>(select)" << endl;
        return -1;
      }

    if(atoi(argv[2]) == 0 || atoi(argv[2]) == 1 || atoi(argv[3]) == -1 || atoi(argv[3]) == 1) {
        cout << "parameter correct" << endl;
    }
    else {
        cout << "parameter error" << endl;
        return -1;
      }

    YAML::Node Node = YAML::LoadFile(argv[1]);

    cout << "-----Process init-----" << endl;

    SenseData senseData;
    PlanData planData;
    SenseCamera camera;
    HiwinSDK hiwin;
    FindScissors findScissors;
    FindHole findhole;

    //camera
    int iResult;
    iResult = camera.cameraInit(&senseData);
    if (iResult != 0)
    cout << "Camera Init failed" << endl;
    //camera end

    //arm
    cout << "-----Arm init-----" << endl;

    double basePos[6] = {0, 0, 0, 0, 0, 0};
    double toolPos[6] = {0, 0, 0, 0, 0, 0};
    double homePos[6] = {0, 0, 0, 0, -90, 0};
    double goPos[6] = {0, 0, 0, 0, 0, 0};
    int go = 0;

    hiwin.connect("192.168.0.1", 502);

    //hiwin.errorReset();
    hiwin.setServoState(1);                                 //servo state 1=on
    hiwin.setOperationMode(0);

    hiwin.setActiveBaseNum(3);
    hiwin.setCurrentBaseCoord(basePos);
    hiwin.setActiveToolNum(3);
    hiwin.setCurrentToolCoord(toolPos);
    hiwin.setOperationMode(1);                              // 1=run
    hiwin.setAcceleration(Node["arm"]["acceleration"].as<int>());
    hiwin.setFeedRate(Node["arm"]["feedrate"].as<int>());
    hiwin.setPTPSpeed(Node["arm"]["PTPspeed"].as<int>());
    usleep(100);

    hiwin.movePtpAbs(homePos, HiwinSDK::CoordType::Joint);  //arm go home
    hiwin.waitForIdle();
    //arm end

    //findscissors
    findScissors.Init(Node);

    // process
    bool keep = 1;
    double error;
    double input = 1;
    double regulatePos[6] = {0};
    vector<double> value;
    char key;
    xyz camera2sucker;
    if(atoi(argv[2]) == 0) {
        camera2sucker.x = Node["realsense"]["move2suckerX"].as<double>();
        camera2sucker.y = Node["realsense"]["move2suckerY"].as<double>();
        camera2sucker.z = Node["realsense"]["move2suckerZ"].as<double>();
    }
    else if(atoi(argv[2]) == 1) {
        camera2sucker.x = Node["realsense"]["move2suckerXFoam"].as<double>();
        camera2sucker.y = Node["realsense"]["move2suckerYFoam"].as<double>();
        camera2sucker.z = Node["realsense"]["move2suckerZFoam"].as<double>();
    }
    else {
        cout << "get parameter error" << endl;
        return -1;
    }

    // arm move
    goPos[0] = (go + atoi(argv[3]))*150;
    hiwin.movePtpRel(goPos, HiwinSDK::CoordType::Coord);
    hiwin.waitForIdle();

    while(1) {
        camera.cameraRun();
        camera.getImgs(&senseData);

        imshow("senseData.rgb", senseData.rgb[0]);

        if (waitKey(1) == 27)
            break;
      }

    if(atoi(argv[2]) == 0) {
        while(1) {
            planData.sccissorDepth.clear();
            planData.sccissorTarget.clear();
            planData.sccissorSituation.clear();

            camera.cameraRun();
            camera.getImgs(&senseData);

            findScissors.getImg(senseData);
            findScissors.process(planData);

            if(planData.sccissorTarget.size() != 0) {
                error = sqrt((planData.sccissorTarget[0].x * planData.sccissorTarget[0].x) + (planData.sccissorTarget[0].y * planData.sccissorTarget[0].y));
                cout << "error" << error << endl;

                if(error > 0.001) {
                    value = {planData.sccissorTarget[0].x*1000, planData.sccissorTarget[0].y*-1000, 0, 0, 0, 0};
                    hiwin.move(value, HiwinSDK::CtrlType::PTP, HiwinSDK::MoveType::Relative, HiwinSDK::CoordType::Coord);
                    hiwin.waitForIdle();
                  }
                else
                    break;
              }
            else
                cout << "don't find any Scissor" << endl;

            if (waitKey(1) == 27) {
                keep = 0;
                break;
              }
          }
      }

    else if(atoi(argv[2]) == 1) {
        while(1) {
            planData.holeTarget.clear();
            planData.holeDepth.clear();
            planData.holeSituation.clear();

            camera.cameraRun();
            camera.getImgs(&senseData);

            findhole.getSenseData(&senseData);
            findhole.run(&planData, Node);

            if(planData.holeTarget.size() != 0) {
                cout << "planData.holeTarget[0]:" << planData.holeTarget[0] << endl;
                error = sqrt((planData.holeTarget[0].x * planData.holeTarget[0].x) + (planData.holeTarget[0].y * planData.holeTarget[0].y));
                cout << "error" << error << endl;
                if(error > 0.001) {
                    value = {planData.holeTarget[0].x*1000, planData.holeTarget[0].y*-1000, 0, 0, 0, 0};
                    hiwin.move(value, HiwinSDK::CtrlType::PTP, HiwinSDK::MoveType::Relative, HiwinSDK::CoordType::Coord);
                    hiwin.waitForIdle();
                  }
                else
                    break;
              }
            else
                cout << "don't find any Hole" << endl;

            if (waitKey(0) == 27) {
                keep = 0;
                break;
              }
          }
      }

    if(keep == 1) {
        if(atoi(argv[2]) == 0) {
            value = {camera2sucker.x, camera2sucker.y, planData.sccissorDepth[0]*-1000 + camera2sucker.z + 30, 0, 0, 0};
            hiwin.move(value, HiwinSDK::CtrlType::PTP, HiwinSDK::MoveType::Relative, HiwinSDK::CoordType::Coord);
            hiwin.waitForIdle();

          }
        else if(atoi(argv[2]) == 1) {
            value = {camera2sucker.x, camera2sucker.y, planData.holeDepth[0]*-1000 + camera2sucker.z + 30, 0, 0, 0};
            hiwin.move(value, HiwinSDK::CtrlType::PTP, HiwinSDK::MoveType::Relative, HiwinSDK::CoordType::Coord);
            hiwin.waitForIdle();
          }

        value = {0, 0, -30, 0, 0, 0, 0};
        hiwin.move(value, HiwinSDK::CtrlType::PTP, HiwinSDK::MoveType::Relative, HiwinSDK::CoordType::Coord);
        hiwin.waitForIdle();

        while(1) {
            value = {0, 0, 0, 0, 0, 0};
            cout << "input:" << input << endl;
            key = waitKey(0);

            if (key == 'q')
                break;

            else if (key == 'd') {
                value = {input, 0, 0, 0, 0, 0};
                regulatePos[0] = regulatePos[0] + input;
              }

            else if (key == 'a') {
                value = {-input, 0, 0, 0, 0, 0};
                regulatePos[0] = regulatePos[0] - input;
              }

            else if (key == 'w') {
                value = {0, input, 0, 0, 0, 0};
                regulatePos[1] = regulatePos[1] + input;
              }

            else if (key == 's') {
                value = {0, -input, 0, 0, 0, 0};
                regulatePos[1] = regulatePos[1] - input;
              }

            else if (key == 'r') {
                value = {0, 0, input, 0, 0, 0};
                regulatePos[2] = regulatePos[2] + input;
              }

            else if (key == 'f') {
                value = {0, 0, -input, 0, 0, 0};
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

            hiwin.move(value, HiwinSDK::CtrlType::PTP, HiwinSDK::MoveType::Relative, HiwinSDK::CoordType::Coord);
            hiwin.waitForIdle();
          }

        if(atoi(argv[2]) == 0) {
            Node["realsense"]["move2suckerX"] = Node["realsense"]["move2suckerX"].as<double>() + regulatePos[0];
            Node["realsense"]["move2suckerY"] = Node["realsense"]["move2suckerY"].as<double>() + regulatePos[1];
            Node["realsense"]["move2suckerZ"] = Node["realsense"]["move2suckerZ"].as<double>() + regulatePos[2];
        }
        if(atoi(argv[2]) == 1) {
            Node["realsense"]["move2suckerXFoam"] = Node["realsense"]["move2suckerXFoam"].as<double>() + regulatePos[2];
            Node["realsense"]["move2suckerYFoam"] = Node["realsense"]["move2suckerYFoam"].as<double>() + regulatePos[1];
            Node["realsense"]["move2suckerZFoam"] = Node["realsense"]["move2suckerZFoam"].as<double>() + regulatePos[2];
        }

        ofstream fout(argv[1]);
        fout << Node;
        fout.close();
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
