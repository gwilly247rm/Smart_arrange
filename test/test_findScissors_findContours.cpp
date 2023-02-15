#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include "yaml-cpp/yaml.h"
#include "SPAData.h"
#include "SenseCamera.h"
#define OPENCV

using namespace std;
using namespace cv;

typedef struct HSV {
    int H_high;
    int S_high;
    int V_high;
    int H_low;
    int S_low;
    int V_low;
}HSV;

static void trackbar(int hsv, void*) {}

int main(int argc, char** argv) {
    //set up
    SenseCamera camera;
    SenseData senseData;
    int iResult = 0;
    int item = 0;
    int tempItem = 0;
    int kernel_type;
    int kernel_size;
    int bladeContourArea_high;
    int bladeContourArea_low;
    int handleContourArea_high;
    int handleContourArea_low;
    int contourArea_high;
    int contourArea_low;
    Mat realSenseRgb;
    Mat realSenseRgbDraw;
    Mat realSenseHSV;
    Mat realSenseBinary;
    Mat realSenseBinaryBlade;
    Mat realSenseBinaryHandle;
    Mat realSenseBinaryHandle2;
    Mat realSenseBinaryHandle3;
    Mat temp;
    Mat realSenseBinaryConvolution;
    Mat kernel;
    vector<vector<Point>> contours;
    vector<vector<Point>> contoursSelect;
    vector<Vec4i> hierarchy;
    vector<Rect> boundRect;
    HSV ScissorsHandlePurple;
    HSV ScissorsHandleBlue;
    HSV ScissorsHandleBlack;
    HSV ScissorsBlade;

    if (argc < 2) {
        cout << "parameter:<path of yaml> <path of picture>(select)" << endl;
        return -1;
    }
    else if (argc == 2) {
        cout << "-----Sense init-----" << endl;
        iResult = camera.cameraInit(&senseData);
        if (iResult != 0)
            cout << "Sense Init failed" << endl;
    }
    else if (argc == 3) {
        realSenseRgb = imread(argv[2]);
    }

    YAML::Node node = YAML::LoadFile(argv[1]);

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
    kernel = getStructuringElement(kernel_type, Size(kernel_size, kernel_size));

    handleContourArea_high = node["Scissors"]["handle"]["contourArea_high"].as<int>();
    handleContourArea_low = node["Scissors"]["handle"]["contourArea_low"].as<int>();
    bladeContourArea_high = node["Scissors"]["blade"]["contourArea_high"].as<int>();
    bladeContourArea_low = node["Scissors"]["blade"]["contourArea_low"].as<int>();

    contourArea_high = bladeContourArea_high;
    contourArea_low = bladeContourArea_low;
    // Create Window
    namedWindow("realSenseRgb", WINDOW_AUTOSIZE);
    //namedWindow("realSenseBinary", WINDOW_AUTOSIZE);
    namedWindow("realSenseBinaryConvolution", WINDOW_AUTOSIZE);
    createTrackbar("contourArea_high", "realSenseRgb", &contourArea_high, 100000, trackbar);
    createTrackbar("contourArea_low", "realSenseRgb", &contourArea_low, 100000, trackbar);
    createTrackbar("item", "realSenseRgb", &item, 1, trackbar);

    while (1) {
        if (argc == 2) {
            //get camera image
            camera.cameraRun();
            camera.getImgs(&senseData);
            realSenseRgb = senseData.rgb[0].clone();
        }

        //convert image from rgb to binary and do convolution
        cvtColor(realSenseRgb, realSenseHSV, COLOR_BGR2HSV);
        if (item == 0) {
            inRange(realSenseHSV, Scalar(ScissorsBlade.H_low, ScissorsBlade.S_low, ScissorsBlade.V_low), Scalar(ScissorsBlade.H_high, ScissorsBlade.S_high, ScissorsBlade.V_high), realSenseBinaryBlade);
            realSenseBinary = realSenseBinaryBlade;
        }
        else if (item == 1) {
            inRange(realSenseHSV, Scalar(ScissorsHandlePurple.H_low, ScissorsHandlePurple.S_low, ScissorsHandlePurple.V_low), Scalar(ScissorsHandlePurple.H_high, ScissorsHandlePurple.S_high, ScissorsHandlePurple.V_high), realSenseBinaryHandle);
            inRange(realSenseHSV, Scalar(ScissorsHandleBlue.H_low, ScissorsHandleBlue.S_low, ScissorsHandleBlue.V_low), Scalar(ScissorsHandleBlue.H_high, ScissorsHandleBlue.S_high, ScissorsHandleBlue.V_high), realSenseBinaryHandle2);
            temp = realSenseBinaryHandle | realSenseBinaryHandle2;
            inRange(realSenseHSV, Scalar(ScissorsHandleBlack.H_low, ScissorsHandleBlack.S_low, ScissorsHandleBlack.V_low), Scalar(ScissorsHandleBlack.H_high, ScissorsHandleBlack.S_high, ScissorsHandleBlack.V_high), realSenseBinaryHandle3);
            realSenseBinary = temp | realSenseBinaryHandle3;
        }
        kernel = getStructuringElement(kernel_type, Size(kernel_size+1, kernel_size+1));
        morphologyEx(realSenseBinary, realSenseBinaryConvolution, CV_MOP_OPEN, kernel);

        //select the contour area
        findContours(realSenseBinaryConvolution, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

        for (size_t i = 0; i < contours.size(); i++) {
            if(contourArea(contours[i]) > contourArea_low && contourArea(contours[i]) < contourArea_high) {
                contoursSelect.push_back(contours[i]);
            }
        }

        //draw on img
        realSenseRgbDraw = realSenseRgb.clone();
        /*for (size_t i = 0; i < contoursSelect.size(); i++) {
            for (size_t j = 0; j < contoursSelect.size(); j++) {
                circle(realSenseRgbDraw, Point(contoursSelect[i][j].x,contoursSelect[i][j].y), 1, Scalar(255, 0, 255), -1);
            }
        }*/
        boundRect.resize(contoursSelect.size());
        for (size_t i = 0; i < contoursSelect.size(); i++) {
            boundRect[i] = boundingRect(contoursSelect[i]);//get boundRect
            rectangle(realSenseRgbDraw, boundRect[i], Scalar(0, 255, 0));
        }

        imshow("realSenseRgb", realSenseRgbDraw);
        imshow("realSenseBinaryConvolution",realSenseBinaryConvolution);

        //set ConturArea value
        if(item == 0)
            cout << "blade" << endl;
        else if(item == 1)
            cout << "handle" << endl;

        if (waitKey(1) == 27)
        {
            if(item == 0) {
                bladeContourArea_high = contourArea_high;
                bladeContourArea_low = contourArea_low;
            }
            else if(item == 1) {
                handleContourArea_high = contourArea_high;
                handleContourArea_low = contourArea_low;
            }

            break;
        }

        //save ConturArea value and change item
        if(tempItem != item) {
            if(tempItem == 0) {
                bladeContourArea_high = contourArea_high;
                bladeContourArea_low = contourArea_low;
            }
            else if(tempItem == 1) {
                handleContourArea_high = contourArea_high;
                handleContourArea_low = contourArea_low;
            }
            tempItem = item;
            if(item == 0) {
                contourArea_high = bladeContourArea_high;
                contourArea_low = bladeContourArea_low;
            }
            else if(item == 1) {
                contourArea_high = handleContourArea_high;
                contourArea_low = handleContourArea_low;
            }
            setTrackbarPos("contourArea_high", "realSenseRgb", contourArea_high);
            setTrackbarPos("contourArea_low", "realSenseRgb", contourArea_low);
        }

        contours.clear();
        contoursSelect.clear();
        hierarchy.clear();
        boundRect.clear();
    }

    node["Scissors"]["handle"]["contourArea_high"] = handleContourArea_high;
    node["Scissors"]["handle"]["contourArea_low"] = handleContourArea_low;
    node["Scissors"]["blade"]["contourArea_high"] = bladeContourArea_high;
    node["Scissors"]["blade"]["contourArea_low"] = bladeContourArea_low;

    ofstream fout(argv[1]);
    fout << node;
    fout.close();

    return 0;
}


