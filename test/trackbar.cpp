#include <chrono>
#include <iomanip>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "Camera.h"
#include "FindScissors2.h"

using namespace std;

int main(int argc, char* argv[]) {
        int camIndex = 0;
        Camera::BackgroundMode backGroundMode = Camera::BackgroundMode::GRAB;
        // Check argument
        if (argc > 2) {
            backGroundMode = (Camera::BackgroundMode)stod(argv[2]);
        }
        Camera cam;
	find_scissors obj;
        if (argc > 1) {
            string argv1(argv[1]);
            if (argv1.size() > 1) {
                cam.open(argv1, "cam", 640, 480, backGroundMode);
            } else {
                camIndex = stod(argv1);
                cam.open(camIndex, "cam", 640, 480, backGroundMode);
            }
        } else {
            cam.open(camIndex, "cam", 640, 480, backGroundMode);
        }

	obj.create_trackbar();
	cv::Mat hsv;
        char kbin;
        bool run = true;
        string fileName = "";

        while (run) {
            cam.read();

   	    cv::cvtColor(cam.image, hsv, cv::COLOR_BGR2HSV);
obj.find_hsv_mask(hsv,obj.hsv_value.H_low,obj.hsv_value.S_low,obj.hsv_value.V_low,obj.hsv_value.H_high,obj.hsv_value.S_high,obj.hsv_value.V_high,cam.image);
	    obj.show_hsv_mask(find_scissors::Color_mask::Black,cam.image);
	    //obj.find_hsv_mask(hsv,152,131,0,255,255,152,cam.image);
	    //obj.show_hsv_mask(find_scissors::Color_mask::Red,cam.image);
	    //obj.find_hsv_mask(hsv,111,86,26,146,255,91,cam.image);
	    //obj.show_hsv_mask(find_scissors::Color_mask::Blue,cam.image);

	    //(0,0,0),(35,35,35) black
    	    //(152,131,0),(255,255,152)red
    	    //(111,86,26),(146,255,91) blue
	    cam.show();
            kbin = cv::waitKey(1);
            switch (kbin) {
                case 27:
                    run = false;
                    break;
                case 'c':
                    fileName = obj.getFileName();
                    cv::imwrite(fileName, cam.image);
                    cout << "Write: " << fileName << endl;
                    break;
            }
        }

    return 0;
}

