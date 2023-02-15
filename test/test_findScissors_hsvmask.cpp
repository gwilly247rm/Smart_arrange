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
void onMouse(int event, int x, int y, int flags, void* ptr);

int main(int argc, char** argv) {
    //set up
    SenseCamera camera;
    SenseData senseData;
    int iResult = 0;
    int item = 0;
    int tempItem = 0;
    Point previousPoint = Point(0,0);
    int kernel_type;
    int kernel_size;
    Mat realSenseRgb;
    Mat realSenseHSV;
    Mat realSenseBinary;
    Mat realSenseBinaryConvolution;
    Mat kernel;
    HSV ScissorsHandlePurple;
    HSV ScissorsHandleBlue;
    HSV ScissorsHandleBlack;
    HSV ScissorsBlade;
    HSV temp = {0};
    
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
    
    temp = ScissorsBlade;
    // Create Window
    namedWindow("realSenseRgb", WINDOW_AUTOSIZE);
    namedWindow("realSenseBinary", WINDOW_AUTOSIZE);
    namedWindow("realSenseBinaryConvolution", WINDOW_AUTOSIZE);
    createTrackbar("H_high", "realSenseBinary", &temp.H_high, 180, trackbar);
    createTrackbar("S_high", "realSenseBinary", &temp.S_high, 255, trackbar);
    createTrackbar("V_high", "realSenseBinary", &temp.V_high, 255, trackbar);
    createTrackbar("H_low", "realSenseBinary", &temp.H_low, 180, trackbar);
    createTrackbar("S_low", "realSenseBinary", &temp.S_low, 255, trackbar);
    createTrackbar("V_low", "realSenseBinary", &temp.V_low, 255, trackbar);
    createTrackbar("item", "realSenseBinary", &item, 3, trackbar);
    createTrackbar("kernel_type", "realSenseBinaryConvolution", &kernel_type, 2, trackbar);//0:MORPH_RECT; 1:MORPH_CROSS; 2:MORPH_ELLIPSE;
    createTrackbar("kernel_size", "realSenseBinaryConvolution", &kernel_size, 50, trackbar);
    setMouseCallback("realSenseRgb", onMouse, &previousPoint);
    
    while (1) {
        if (argc == 2) {
            //get camera image
            camera.cameraRun();
            camera.getImgs(&senseData);
            realSenseRgb = senseData.rgb[0].clone();
        }
        
        //convert image from rgb to binary and do convolution
        cvtColor(realSenseRgb, realSenseHSV, COLOR_BGR2HSV);
        inRange(realSenseHSV, Scalar(temp.H_low, temp.S_low, temp.V_low), Scalar(temp.H_high, temp.S_high, temp.V_high), realSenseBinary);
        kernel = getStructuringElement(kernel_type, Size(kernel_size+1, kernel_size+1));
        morphologyEx(realSenseBinary, realSenseBinaryConvolution, CV_MOP_OPEN, kernel);
        
        imshow("realSenseRgb", realSenseRgb);
        imshow("realSenseBinary",realSenseBinary);
        imshow("realSenseBinaryConvolution",realSenseBinaryConvolution);
        
        //set HSV value
        if(item == 0)
            cout << "blade" << endl;
        else if(item == 1)
            cout << "handle purple" << endl;
        else if(item == 2)
            cout << "handle blue" << endl;
        else if(item == 3)
            cout << "handle black" << endl;
        
        if (waitKey(1) == 27) {
            if(tempItem == 0)
                ScissorsBlade = temp;
            else if(tempItem == 1)
                ScissorsHandlePurple = temp;
            else if(tempItem == 2)
                ScissorsHandleBlue = temp;
            else if(tempItem == 3)
                ScissorsHandleBlack = temp;
            
            break;
        }
        //get value of h from camera image
        if(previousPoint != Point{0,0}) {
            temp.H_high = realSenseHSV.at<Vec3b>(previousPoint.y, previousPoint.x)[0];
            temp.S_high = realSenseHSV.at<Vec3b>(previousPoint.y, previousPoint.x)[1];
            temp.V_high = realSenseHSV.at<Vec3b>(previousPoint.y, previousPoint.x)[2];
            setTrackbarPos("H_high", "realSenseBinary", temp.H_high);
            setTrackbarPos("S_high", "realSenseBinary", temp.S_high);
            setTrackbarPos("V_high", "realSenseBinary", temp.V_high);
            previousPoint = Point{0,0};
        }
        
        //save hsv value and change item
        if(tempItem != item) {
            if(tempItem == 0)
                ScissorsBlade = temp;
            else if(tempItem == 1)
                ScissorsHandlePurple = temp;
            else if(tempItem == 2)
                ScissorsHandleBlue = temp;
            else if(tempItem == 3)
                ScissorsHandleBlack = temp;
            
            tempItem = item;
            
            if(item == 0)
                temp = ScissorsBlade;
            else if(item == 1)
                temp = ScissorsHandlePurple;
            else if(item == 2) 
                temp = ScissorsHandleBlue;
            else if(item == 3)
                temp = ScissorsHandleBlack;
            
            setTrackbarPos("H_high", "realSenseBinary", temp.H_high);
            setTrackbarPos("S_high", "realSenseBinary", temp.S_high);
            setTrackbarPos("V_high", "realSenseBinary", temp.V_high);
            setTrackbarPos("H_low", "realSenseBinary", temp.H_low);
            setTrackbarPos("S_low", "realSenseBinary", temp.S_low);
            setTrackbarPos("V_low", "realSenseBinary", temp.V_low);
        }
    }
    //save hsv value in yaml
    node["Scissors"]["handle"]["purple"]["h_high"] =  ScissorsHandlePurple.H_high;
    node["Scissors"]["handle"]["purple"]["s_high"] = ScissorsHandlePurple.S_high;
    node["Scissors"]["handle"]["purple"]["v_high"] = ScissorsHandlePurple.V_high;
    node["Scissors"]["handle"]["purple"]["h_low"] = ScissorsHandlePurple.H_low;
    node["Scissors"]["handle"]["purple"]["s_low"] = ScissorsHandlePurple.S_low;
    node["Scissors"]["handle"]["purple"]["v_low"] = ScissorsHandlePurple.V_low;
    
    node["Scissors"]["handle"]["blue"]["h_high"] = ScissorsHandleBlue.H_high;
    node["Scissors"]["handle"]["blue"]["s_high"] = ScissorsHandleBlue.S_high;
    node["Scissors"]["handle"]["blue"]["v_high"] = ScissorsHandleBlue.V_high;
    node["Scissors"]["handle"]["blue"]["h_low"] = ScissorsHandleBlue.H_low; 
    node["Scissors"]["handle"]["blue"]["s_low"] = ScissorsHandleBlue.S_low; 
    node["Scissors"]["handle"]["blue"]["v_low"] = ScissorsHandleBlue.V_low; 
    
    node["Scissors"]["handle"]["black"]["h_high"] = ScissorsHandleBlack.H_high;
    node["Scissors"]["handle"]["black"]["s_high"] = ScissorsHandleBlack.S_high;
    node["Scissors"]["handle"]["black"]["v_high"] = ScissorsHandleBlack.V_high;
    node["Scissors"]["handle"]["black"]["h_low"] = ScissorsHandleBlack.H_low;
    node["Scissors"]["handle"]["black"]["s_low"] = ScissorsHandleBlack.S_low;
    node["Scissors"]["handle"]["black"]["v_low"] = ScissorsHandleBlack.V_low;
    
    node["Scissors"]["blade"]["h_high"] = ScissorsBlade.H_high;
    node["Scissors"]["blade"]["s_high"] = ScissorsBlade.S_high;
    node["Scissors"]["blade"]["v_high"] = ScissorsBlade.V_high;
    node["Scissors"]["blade"]["h_low"] = ScissorsBlade.H_low;
    node["Scissors"]["blade"]["s_low"] = ScissorsBlade.S_low;
    node["Scissors"]["blade"]["v_low"] = ScissorsBlade.V_low;
    
    ofstream fout(argv[1]);
    fout << node;
    fout.close();
    
    return 0;
}

void onMouse(int event, int x, int y, int flags, void* ptr) {
    Point* p = (Point*)ptr;
    
    if (event == EVENT_LBUTTONDOWN) *p = Point(x, y);
}

