#include "Process.h"
#include <ctime>
#include <unistd.h>

using namespace std;
using namespace cv;

Process::Process(const std::string arm_ip, int port, const std::string wise_ip, const std::string yaml_path) {
    armIp = arm_ip;
    armPort = port;
    wiseIp = wise_ip;
    yamlPath = yaml_path;
  }

Process::~Process() {
    //arm
    cout << "-----Arm destruct-----" << endl;
    double toolPos[6] = {0, 0, 0, 0, 0, 0};
    double homePos[6] = {0, 0, 0, 0, -90, 0};

    //hiwin.errorReset();

    hiwin.movePtpAbs(homePos, HiwinSDK::CoordType::Joint);  //arm go home
    hiwin.waitForIdle();

    hiwin.setOperationMode(0);
    hiwin.setCurrentToolCoord(toolPos);
    hiwin.setServoState(0);

    usleep(100);

    hiwin.disconnect();
    //arm end

    //sucker
    cout << "-----Sucker destruct-----" << endl;
    wise.writeRelay0(false);
    wise.writeRelay1(false);
    cout << "sucker off" << endl;
  //sucker end
  }

int Process::ProcessInit() {
    cout << "-----Process init-----" << endl;

    Node = YAML::LoadFile(yamlPath);

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

    hiwin.connect(armIp, armPort);

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

    //sucker
    cout << "-----Sucker init-----" << endl;
    wise.connect(wiseIp, 502);

    wise.writeRelay0(false);
    wise.writeRelay1(false);
    cout << "sucker off" << endl;
    //sucker end

    //findscissors
    findScissors.Init(Node);

    // process
    camera2sucker.x = Node["realsense"]["move2suckerX"].as<double>();
    camera2sucker.y = Node["realsense"]["move2suckerY"].as<double>();
    camera2sucker.z = Node["realsense"]["move2suckerZ"].as<double>();
    camera2suckerFoamDepth = Node["realsense"]["move2suckerZFoam"].as<double>();

    return iResult;
  }

void Process::ProcessRun() {
    Point2f finalPutPoint;
    double finalPutDepth;
    bool finalPutSituation;
    int min;
    double error;
    double errorTemp;
    char key;

    while(1)
    {
        keep = 1;

        while(1) {
            CameraProcess();
            imshow("realSenseRgb",senseData.rgb[0]);
            key = waitKey(1);
            if (key == 27) {
                break;
              }
            else if (key == 'q') {
                break;
                keep = 0;
              }
          }
        destroyAllWindows();
        if(keep == 1){
            // find scissor and hole
            // find hole process
            planData.action.arm.coordType = HiwinSDK::CoordType::Coord;
            planData.action.arm.moveType = HiwinSDK::MoveType::Relative;
            planData.action.arm.ctrlType = HiwinSDK::CtrlType::PTP;
            planData.action.arm.value = {150, 0, 0, 0, 0, 0};
            ArmProcess();

            finalPutPoint = Point2f{0, 0};

            while(1) {
                planData.holeTarget.clear();
                planData.holeDepth.clear();
                planData.holeSituation.clear();
                min = 0;

                CameraProcess();
                FindHoleProcess();

                if(planData.holeTarget.size() != 0) {
                    error = sqrt((planData.holeTarget[min].x * planData.holeTarget[min].x) + (planData.holeTarget[min].y * planData.holeTarget[min].y));
                    for (size_t i = 1; i < planData.holeTarget.size(); i++) {
                        errorTemp = sqrt((planData.holeTarget[i].x * planData.holeTarget[i].x) + (planData.holeTarget[i].y * planData.holeTarget[i].y));
                        if(errorTemp <= error) {
                            error = errorTemp;
                            min = i;
                          }
                      }

                    for (size_t i = 0; i < planData.holeTarget.size(); i++)
                    cout << "holeTarget[" << i << "]:" <<  planData.holeTarget[i] << endl;

                    cout << "error" << error << endl;

                    //waitKey(0);

                    if(error > 0.001) {
                        finalPutPoint = finalPutPoint + planData.holeTarget[min];
                        planData.action.arm.coordType = HiwinSDK::CoordType::Coord;
                        planData.action.arm.moveType = HiwinSDK::MoveType::Relative;
                        planData.action.arm.ctrlType = HiwinSDK::CtrlType::PTP;
                        planData.action.arm.value = {planData.holeTarget[min].x*1000, planData.holeTarget[min].y*-1000, 0, 0, 0, 0};
                        ArmProcess();
                      }
                    else {
                        finalPutDepth = planData.holeDepth[min];
                        finalPutSituation = planData.holeSituation[min];
                        break;
                      }
                    /*finalPutPoint = finalPutPoint + planData.holeTarget[min];
                    finalPutDepth = planData.holeDepth[min];
                    finalPutSituation = planData.holeSituation[min];
                        break;*/

                  }
                else
                    cout << "Process don't find any Hole" << endl;

                if(waitKey(1) == 'q') {
                    keep = 0;
                        break;
                  }
              }

            if (keep == 1) {
                // find scissor process
                planData.action.arm.coordType = HiwinSDK::CoordType::Joint;
                planData.action.arm.moveType = HiwinSDK::MoveType::Absolute;
                planData.action.arm.ctrlType = HiwinSDK::CtrlType::PTP;
                planData.action.arm.value = {0, 0, 0, 0, -90, 0};
                ArmProcess();

                planData.action.arm.coordType = HiwinSDK::CoordType::Coord;
                planData.action.arm.moveType = HiwinSDK::MoveType::Relative;
                planData.action.arm.ctrlType = HiwinSDK::CtrlType::PTP;
                planData.action.arm.value = {-150, 0, 0, 0, 0, 0};
                ArmProcess();

                while(1) {
                    planData.sccissorDepth.clear();
                    planData.sccissorTarget.clear();
                    planData.sccissorSituation.clear();
                    min = 0;

                    CameraProcess();
                    imshow("realSenseRgb",senseData.rgb[0]);
                    FindScissorsProcess();

                    if(planData.sccissorTarget.size() != 0) {
                        error = sqrt((planData.sccissorTarget[min].x * planData.sccissorTarget[min].x) + (planData.sccissorTarget[min].y * planData.sccissorTarget[min].y));
                        for (size_t i = 1; i < planData.sccissorTarget.size(); i++) {
                            errorTemp = sqrt((planData.sccissorTarget[i].x * planData.sccissorTarget[i].x) + (planData.sccissorTarget[i].y * planData.sccissorTarget[i].y));
                            if(errorTemp <= error) {
                                error = errorTemp;
                                min = i;
                              }
                          }

                        cout << "error" << error << endl;

                        if(error > 0.001) {
                            planData.action.arm.coordType = HiwinSDK::CoordType::Coord;
                            planData.action.arm.moveType = HiwinSDK::MoveType::Relative;
                            planData.action.arm.ctrlType = HiwinSDK::CtrlType::PTP;
                            planData.action.arm.value = {planData.sccissorTarget[min].x*1000, planData.sccissorTarget[min].y*-1000, 0, 0, 0, 0};
                            ArmProcess();
                          }
                        else
                            break;
                      }
                    else
                        cout << "Process don't find any Scissor" << endl;

                    if(waitKey(1) == 'q') {
                        keep = 0;
                            break;
                      }
                  }

                if (keep == 1) {
                    // start to put scissor into scissor's home
                    planData.action.arm.coordType = HiwinSDK::CoordType::Coord;
                    planData.action.arm.moveType = HiwinSDK::MoveType::Relative;
                    planData.action.arm.ctrlType = HiwinSDK::CtrlType::PTP;
                    planData.action.arm.value = {camera2sucker.x, camera2sucker.y, planData.sccissorDepth[min]*-1000 + camera2sucker.z + 30,
                        0, 0, planData.sccissorSituation[min]*180.0};
                    planData.action.sucker.suck = 1;
                    ArmProcess();

                    planData.action.arm.coordType = HiwinSDK::CoordType::Coord;
                    planData.action.arm.moveType = HiwinSDK::MoveType::Relative;
                    planData.action.arm.ctrlType = HiwinSDK::CtrlType::PTP;
                    planData.action.arm.value = {0, 0, -30, 0, 0, 0, 0};
                    ArmProcess();
                    SuckerProcess();

                    planData.action.arm.coordType = HiwinSDK::CoordType::Joint;
                    planData.action.arm.moveType = HiwinSDK::MoveType::Absolute;
                    planData.action.arm.ctrlType = HiwinSDK::CtrlType::PTP;
                    planData.action.arm.value = {0, 0, 0, 0, -90, 0};
                    ArmProcess();

                    if (keep == 1) {
                        planData.action.arm.coordType = HiwinSDK::CoordType::Coord;
                        planData.action.arm.moveType = HiwinSDK::MoveType::Relative;
                        planData.action.arm.ctrlType = HiwinSDK::CtrlType::PTP;
                        planData.action.arm.value = {
                            finalPutPoint.x*1000 + camera2sucker.x + 150,
                            finalPutPoint.y*-1000 + camera2sucker.y,
                            finalPutDepth*-1000 + camera2suckerFoamDepth + 30,
                            0, 0, finalPutSituation*180.0
                          };
                        planData.action.sucker.suck = 0;
                        ArmProcess();

                        planData.action.arm.coordType = HiwinSDK::CoordType::Coord;
                        planData.action.arm.moveType = HiwinSDK::MoveType::Relative;
                        planData.action.arm.ctrlType = HiwinSDK::CtrlType::PTP;
                        planData.action.arm.value = {0, 0, -30, 0, 0, 0, 0};
                        ArmProcess();
                        SuckerProcess();
                      }
                  }
              }

            /*EasyProcess();

            if(keep == 1) {
            ArmProcess();
            SuckerProcess();
          }*/

            planData.action.arm.coordType = HiwinSDK::CoordType::Joint;
            planData.action.arm.moveType = HiwinSDK::MoveType::Absolute;
            planData.action.arm.ctrlType = HiwinSDK::CtrlType::PTP;
            planData.action.arm.value = {0, 0, 0, 0, -90, 0};
            ArmProcess();

            cout << "press q to end program" << endl;
            cout << "press any key without esc to continue" << endl;
            if (waitKey(0) == 'q')
                break;
            else
                destroyAllWindows();
          }
    }
    destroyAllWindows();
  }

void Process::CameraProcess() {
    camera.cameraRun();
    camera.getImgs(&senseData);
  //camera.showImgs();
  }

void Process::FindScissorsProcess() {
    findScissors.getImg(senseData);
    findScissors.process(planData);
  }

void Process::FindHoleProcess()
{
    findhole.getSenseData(&senseData);
    findhole.run(&planData, Node);
}

void Process::SuckerProcess() {
    clock_t start, end;
    bool di0;

    if(planData.action.sucker.suck == 1) {
        wise.writeRelay0(true);
        wise.writeRelay1(false);
        cout << "sucker on" << endl;
      }
    else {
        wise.writeRelay0(false);
        wise.writeRelay1(true);
        cout << "sucker off" << endl;
      }

    start=clock();
    while(1) {
        di0 = wise.readDigitalInput0();
        if(di0 == 0 && planData.action.sucker.suck == 1) {
                break;
          }
        else if(di0 == 1 && planData.action.sucker.suck == 0) {
            wise.writeRelay0(false);
            wise.writeRelay1(false);
                break;
          }
        else {
            end=clock();
            //cout << "time:" << start  << "," << end << "," << CLOCKS_PER_SEC << endl;
            if(end-start > CLOCKS_PER_SEC*2) {
                //if(end-start > 60000) {
                cout << "sucker fail" << endl;
                keep = 0;
                wise.writeRelay0(false);
                wise.writeRelay1(false);
                break;
              }
          }
      }

    actData.checkSuck = wise.readDigitalInput0();
    cout << "Suck situation DI: [" << actData.checkSuck << "]";
  }

void Process::ArmProcess() {
    hiwin.move(planData.action.arm.value, planData.action.arm.ctrlType, planData.action.arm.moveType, planData.action.arm.coordType);
    hiwin.waitForIdle();

    planData.action.arm.coordType = HiwinSDK::CoordType::Coord;
    planData.action.arm.moveType = HiwinSDK::MoveType::Relative;
    planData.action.arm.ctrlType = HiwinSDK::CtrlType::PTP;
    planData.action.arm.value = {0, 0, 0, 0, 0, 0};

    actData.currentPos = hiwin.getCurrentPosition();

    for(size_t i = 0; i < actData.currentPos.size(); i++)
    cout << "Arm pos[" << i << "]:" << actData.currentPos[i] << endl;
  }

void Process::EasyProcess() {
    char key;
    xyz realxyz;
    Mat realsenseRgb;
    Mat realsenseXyz;
    Point previousPoint = Point{0, 0};
    Point zero = Point{0, 0};
    Point draw;

    realsenseXyz = senseData.xyz[0].clone();

    cout << "press q can quit" << endl;
    while(1) {
        keep = 1;
        realsenseRgb = senseData.rgb[0].clone();
        imshow("realsenseRgb",realsenseRgb);
        setMouseCallback("realsenseRgb", onMouse, &previousPoint);

        if(waitKey(1) == 'q') {
            keep = 0;
                break;
          }

        if(previousPoint != zero) {
            realxyz.x = realsenseXyz.at<Vec3f>(previousPoint.y, previousPoint.x)[0];
            realxyz.y = realsenseXyz.at<Vec3f>(previousPoint.y, previousPoint.x)[1];
            realxyz.z = realsenseXyz.at<Vec3f>(previousPoint.y, previousPoint.x)[2];
            draw = Point(previousPoint.x,previousPoint.y);
            cout << "realxyz: x[ " << realxyz.x << " ], y[ "  << realxyz.y << " ], z[ "  << realxyz.z << " ]" <<endl;

            circle(realsenseRgb, draw, 5, Scalar(0, 0, 255), -1);
            imshow("realsenseRgb",realsenseRgb);

            cout << "press s or p then arm will move to suck or put" << endl;
            cout << "press any key without s&p can retry" << endl;

            key = waitKey(0);

            if(key == 's') {
                planData.action.sucker.suck = 1;
                break;
              }
            if(key == 'p') {
                planData.action.sucker.suck = 0;
                break;
              }
            else
            previousPoint = Point(0,0);
          }
      }

    if(keep == 1) {
        planData.action.arm.coordType = HiwinSDK::CoordType::Coord;
        planData.action.arm.moveType = HiwinSDK::MoveType::Relative;
        planData.action.arm.ctrlType = HiwinSDK::CtrlType::PTP;
        planData.action.arm.value = {realxyz.x*1000 + camera2sucker.x, realxyz.y*-1000 + camera2sucker.y, -realxyz.z*1000 + camera2sucker.z, 0, 0, 0};
      }
  }

void Process::onMouse(int event, int x, int y, int flags, void* ptr) {
    Point* p = (Point*)ptr;

    if (event == EVENT_LBUTTONDOWN) *p = Point(x, y);
  }
