#include "FindScissors.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#define OPENCV

using namespace std;
using namespace cv;

FindScissors::FindScissors() {}

FindScissors::~FindScissors() {}

void FindScissors::Init(const YAML::Node& node) {
    //get yaml information
    ScissorsHandlePurple.H_high = node["Scissors"]["handle"]["purple"]["h_high"].as<int>();
    ScissorsHandlePurple.S_high = node["Scissors"]["handle"]["purple"]["s_high"].as<int>();
    ScissorsHandlePurple.V_high = node["Scissors"]["handle"]["purple"]["v_high"].as<int>();
    ScissorsHandlePurple.H_low = node["Scissors"]["handle"]["purple"]["h_low"].as<int>();
    ScissorsHandlePurple.S_low = node["Scissors"]["handle"]["purple"]["s_low"].as<int>();
    ScissorsHandlePurple.V_low = node["Scissors"]["handle"]["purple"]["v_low"].as<int>();

    ScissorsHandleBlue.H_high = node["Scissors"]["handle"]["blue"]["h_high"].as<int>();
    ScissorsHandleBlue.S_high = node["Scissors"]["handle"]["blue"]["s_high"].as<int>();
    ScissorsHandleBlue.V_high = node["Scissors"]["handle"]["blue"]["v_high"].as<int>();
    ScissorsHandleBlue.H_low = node["Scissors"]["handle"]["blue"]["h_low"].as<int>();
    ScissorsHandleBlue.S_low = node["Scissors"]["handle"]["blue"]["s_low"].as<int>();
    ScissorsHandleBlue.V_low = node["Scissors"]["handle"]["blue"]["v_low"].as<int>();

    ScissorsHandleBlack.H_high = node["Scissors"]["handle"]["black"]["h_high"].as<int>();
    ScissorsHandleBlack.S_high = node["Scissors"]["handle"]["black"]["s_high"].as<int>();
    ScissorsHandleBlack.V_high = node["Scissors"]["handle"]["black"]["v_high"].as<int>();
    ScissorsHandleBlack.H_low = node["Scissors"]["handle"]["black"]["h_low"].as<int>();
    ScissorsHandleBlack.S_low = node["Scissors"]["handle"]["black"]["s_low"].as<int>();
    ScissorsHandleBlack.V_low = node["Scissors"]["handle"]["black"]["v_low"].as<int>();

    ScissorsBlade.H_high = node["Scissors"]["blade"]["h_high"].as<int>();
    ScissorsBlade.S_high = node["Scissors"]["blade"]["s_high"].as<int>();
    ScissorsBlade.V_high = node["Scissors"]["blade"]["v_high"].as<int>();
    ScissorsBlade.H_low = node["Scissors"]["blade"]["h_low"].as<int>();
    ScissorsBlade.S_low = node["Scissors"]["blade"]["s_low"].as<int>();
    ScissorsBlade.V_low = node["Scissors"]["blade"]["v_low"].as<int>();

    kernel_type = node["Scissors"]["morphologyEx"]["kernel_type"].as<int>();
    kernel_size = node["Scissors"]["morphologyEx"]["kernel_size"].as<int>();

    handleContourArea_high = node["Scissors"]["handle"]["contourArea_high"].as<int>();
    handleContourArea_low = node["Scissors"]["handle"]["contourArea_low"].as<int>();
    bladeContourArea_high = node["Scissors"]["blade"]["contourArea_high"].as<int>();
    bladeContourArea_low = node["Scissors"]["blade"]["contourArea_low"].as<int>();

    suckOffset = node["Scissors"]["suckOffset"].as<double>();
}

void FindScissors::getImg(SenseData& senseData) {
    senseData.rgb[0].copyTo(realSenseRgb);
    senseData.xyz[0].copyTo(realsenseXyz);
}

void FindScissors::process(PlanData& planData) {
    Mat realSenseBinaryBlade;
    Mat realSenseBinaryHandlePurple;
    Mat realSenseBinaryHandleBlue;
    Mat realSenseBinaryHandleBlack;
    vector<vector<Rect>> rectDraw;

    Blade.clear();
    HandlePurple.clear();
    HandleBlue.clear();
    HandleBlack.clear();
    Handle.clear();
    scissor.clear();
    rectDraw.clear();
    suckPoint.clear();
    sccissorSituation.clear();
    planData.sccissorDepth.clear();
    planData.sccissorTarget.clear();
    planData.sccissorSituation.clear();

    //test
    contoursCopy.clear();
    rectCopy.clear();

    realSenseRgbDraw = realSenseRgb.clone();

    cout << "-----FindScissors fileterBackground-----" << endl;
    realSenseBinaryBlade = fileterBackground(realSenseRgb, ScissorsBlade);
    realSenseBinaryHandlePurple = fileterBackground(realSenseRgb, ScissorsHandlePurple);
    realSenseBinaryHandleBlue = fileterBackground(realSenseRgb, ScissorsHandleBlue);
    realSenseBinaryHandleBlack = fileterBackground(realSenseRgb, ScissorsHandleBlack);
    cout << "-----FindScissors findScissorsPos-----" << endl;
    Blade = findScissorsPos (realSenseBinaryBlade, 0);
    HandlePurple = findScissorsPos (realSenseBinaryHandlePurple, 1);
    HandleBlue = findScissorsPos (realSenseBinaryHandleBlue, 1);
    HandleBlack = findScissorsPos (realSenseBinaryHandleBlack, 1);

    Handle.insert(Handle.end(),HandlePurple.begin(),HandlePurple.end());
    Handle.insert(Handle.end(),HandleBlue.begin(),HandleBlue.end());
    Handle.insert(Handle.end(),HandleBlack.begin(),HandleBlack.end());

    //draw on img
    rectDraw.push_back(Blade);
    rectDraw.push_back(HandlePurple);
    rectDraw.push_back(HandleBlue);
    rectDraw.push_back(HandleBlack);

    /*for(size_t i = 0; i < rectDraw.size(); i++)
        for(size_t j = 0; j < rectDraw[i].size(); j++)
            rectangle(realSenseRgbDraw, rectDraw[i][j], Scalar(255, 0, 255));*/

    //test
    rectCopy = rectDraw;

    if (Handle.size() > 0 || Blade.size() > 0) {
        calSuckPos();
        for(size_t i = 0; i < suckPoint.size(); i++) {
            planData.sccissorTarget.push_back(Point2f{realsenseXyz.at<Vec3f>(suckPoint[i].y, suckPoint[i].x)[0],realsenseXyz.at<Vec3f>(suckPoint[i].y, suckPoint[i].x)[1]});
            planData.sccissorDepth.push_back(realsenseXyz.at<Vec3f>(suckPoint[i].y, suckPoint[i].x)[2]);
            planData.sccissorSituation.push_back(sccissorSituation[i]);
          }
      }
    else
        cout << "don't find any Handle or Blade" << endl;

    imshow("realSenseRgbDraw",realSenseRgbDraw);
    waitKey(1);
}
//test
vector<vector<vector<Point>>> FindScissors::returnContours()
{
    return contoursCopy;
}
vector<vector<Rect>> FindScissors::returnRect()
{
    return rectCopy;
}

Mat FindScissors::fileterBackground(Mat img, HSV hsv) {
    Mat imgHSV;
    Mat imgBinary;
    Mat imgBinaryConvolution;
    Mat kernel;

    //convert image from rgb to binary and do convolution
    cvtColor(img, imgHSV, COLOR_BGR2HSV);
    inRange(imgHSV, Scalar(hsv.H_low, hsv.S_low, hsv.V_low), Scalar(hsv.H_high, hsv.S_high, hsv.V_high), imgBinary);

    kernel = getStructuringElement(kernel_type, Size(kernel_size, kernel_size));
    morphologyEx(imgBinary, imgBinaryConvolution, CV_MOP_OPEN, kernel);

    return imgBinaryConvolution;
}

vector<Rect> FindScissors::findScissorsPos(Mat img, int item) {
    vector<vector<Point>> contours;
    vector<vector<Point>> contoursSelect;
    vector<Vec4i> hierarchy;
    vector<Rect> boundRect;

    contours.clear();
    contoursSelect.clear();
    hierarchy.clear();
    boundRect.clear();

    findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    //test
    contoursCopy.push_back(contours);

    if(item == 0) {
        contourArea_high = bladeContourArea_high;
        contourArea_low = bladeContourArea_low;
    }
    else if(item == 1) {
        contourArea_high = handleContourArea_high;
        contourArea_low = handleContourArea_low;
    }

    //select the contour area
    for (size_t i = 0; i < contours.size(); i++) {
        if(contourArea(contours[i]) > contourArea_low && contourArea(contours[i]) < contourArea_high) {
            contoursSelect.push_back(contours[i]);
        }
    }
    //draw
    /*for (size_t i = 0; i < contoursSelect.size(); i++) {
        for (size_t j = 0; j < contoursSelect.size(); j++) {
            circle(realSenseRgbDraw, Point(contoursSelect[i][j].x,contoursSelect[i][j].y), 1, Scalar(255, 0, 255), -1);
        }
    }*/

    //get boundRect
    boundRect.resize(contoursSelect.size());
    for (size_t i = 0; i < contoursSelect.size(); i++) {
        boundRect[i] = boundingRect(contoursSelect[i]);//get boundRect
        //rectangle(realSenseRgbDraw, boundRect[i], Scalar(0, 255, 0)); //draw
    }

    return boundRect;
}

void FindScissors::calSuckPos() {
    Scissor temp;
    double suckOffsetX;
    double suckOffsetY;

    //set all vector memeber value = 1
    vector<bool> checkBalade(Blade.size(),1);
    vector<bool> checkHandle(Handle.size(),1);

    for(size_t i = 0; i < Blade.size(); i++) {
        for(size_t j = 0; j < Handle.size(); j++) {
            if(Blade[i].x < (Handle[j].x + Handle[j].width) && Blade[i].x > Handle[j].x) {
                if(checkBalade[i] != 0 && checkHandle[j] != 0) {
                    temp.blade =  Blade[i];
                    temp.handle = Handle[j];
                    scissor.push_back(temp);
                    //cout << "scissor[" << i << "]:" << scissor[i].blade << "/" << scissor[i].handle << endl;
                    checkBalade[i] = 0;
                    checkHandle[j] = 0;
                }
            }
        }
    }

    if(scissor.size() > 0) {
        cout << "find " << scissor.size() << " scissors" <<endl;

        for(size_t i = 0; i < scissor.size(); i++) {
            rectangle(realSenseRgbDraw, scissor[i].blade, Scalar(255, 0, 255));
            rectangle(realSenseRgbDraw, scissor[i].handle, Scalar(255, 0, 255));
          }

        suckPoint.resize(scissor.size());
        sccissorSituation.resize(scissor.size());

        for(size_t i = 0; i < scissor.size(); i++) {
            if(scissor[i].handle.y > scissor[i].blade.y) {
                suckOffsetX = suckOffset * ((scissor[i].blade.x + scissor[i].blade.width/2) - (scissor[i].handle.x + scissor[i].handle.width/2));
                suckOffsetY = suckOffset * ((scissor[i].blade.y + scissor[i].blade.height/2) - (scissor[i].handle.y + scissor[i].handle.height/2));

                suckPoint[i] = Point{(scissor[i].handle.x + scissor[i].handle.width/2) + (int)suckOffsetX,
                                     (scissor[i].handle.y + scissor[i].handle.height/2) + (int)suckOffsetY};
                //cout << "Scissor handle down" << endl;
                //suckPoint[i] = Point{scissor[i].handle.x + scissor[i].handle.width/2,scissor[i].handle.y - suckOffset};
                //cout << "suckPoint[" << i << "]:" << suckPoint[i] << endl;
                sccissorSituation[i] = 0;
              }
            else if(scissor[i].handle.y < scissor[i].blade.y) {
                suckOffsetX = suckOffset * ((scissor[i].blade.x + scissor[i].blade.width/2) - (scissor[i].handle.x + scissor[i].handle.width/2));
                suckOffsetY = suckOffset * ((scissor[i].blade.y + scissor[i].blade.height/2) - (scissor[i].handle.y + scissor[i].handle.height/2));

                suckPoint[i] = Point{(scissor[i].handle.x + scissor[i].handle.width/2) + (int)suckOffsetX,
                                     (scissor[i].handle.y + scissor[i].handle.height/2) + (int)suckOffsetY};
                //cout << "Scissor handle up" << endl;
                //suckPoint[i] = Point{scissor[i].handle.x + scissor[i].handle.width/2,scissor[i].handle.y + scissor[i].handle.height + suckOffset};
                //cout << "suckPoint[" << i << "]:" << suckPoint[i] << endl;
                sccissorSituation[i] = 1;
              }
            circle(realSenseRgbDraw, suckPoint[i], 5, Scalar(255, 0, 255), -1);

            line(realSenseRgbDraw, Point{scissor[i].handle.x + scissor[i].handle.width/2,scissor[i].handle.y + scissor[i].handle.height/2},
                Point{scissor[i].blade.x + scissor[i].blade.width/2,scissor[i].blade.y + scissor[i].blade.height/2}, Scalar(255, 0, 0), 2, LINE_AA);
          }

        //circle(realSenseRgbDraw, suckPoint[0], 5, Scalar(255, 0, 255), -1);
      }
    else
        cout << "don't find any Scissor" << endl;
}

