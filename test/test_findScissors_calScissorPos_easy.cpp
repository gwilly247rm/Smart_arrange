#include "FindScissors.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include "yaml-cpp/yaml.h"
#include "SPAData.h"
#include "SenseCamera.h"
#define OPENCV

using namespace std;
using namespace cv;

typedef struct Scissor {
    Rect blade;
    Rect handle;
}Scissor;

int main(int argc, char** argv) {
    //set up
    SenseCamera camera;
    SenseData senseData;
    PlanData planData;
    FindScissors findScissors;
    int iResult;
    /*int suckOffset;
    double hypotenuse;
    double suckOffsetX;
    double suckOffsetY;*/
    Mat realSenseRgb;
    Mat realSenseRgbDraw;
    std::vector<std::vector<cv::Rect>> rectCopy;
    std::vector<cv::Rect> Blade;
    std::vector<cv::Rect> Handle;
    vector<Scissor> scissor;
    Scissor temp;
    Point suckPoint;

    rectCopy.clear();
    Blade.clear();
    Handle.clear();
    scissor.clear();

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

    YAML::Node node = YAML::LoadFile(argv[1]);
    //suckOffset = node["Scissors"]["suckOffset"].as<int>();

    findScissors.Init(node);

    if (argc == 2) {
        while (1) {
            //get camera image
            camera.cameraRun();
            camera.getImgs(&senseData);
            realSenseRgb = senseData.rgb[0].clone();

            findScissors.getImg(senseData);
            findScissors.process(planData);

            if (waitKey(10) == 27)
                break;
        }
    }

    else if (argc == 3) {
        senseData.rgb.push_back(imread(argv[2]));
        findScissors.getImg(senseData);
        findScissors.process(planData);
    }
/*
    rectCopy = findScissors.returnRect();

    Blade = rectCopy[0];
    Handle.insert(Handle.end(),rectCopy[1].begin(),rectCopy[1].end());
    Handle.insert(Handle.end(),rectCopy[2].begin(),rectCopy[2].end());
    Handle.insert(Handle.end(),rectCopy[3].begin(),rectCopy[3].end());

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
                    checkBalade[i] = 0;
                    checkHandle[j] = 0;
                  }
              }
          }
      }

    realSenseRgbDraw = senseData.rgb[0].clone();

    if(scissor.size() > 0) {
        cout << "find " << scissor.size() << " scissors" <<endl;
        for(size_t i = 0; i < scissor.size(); i++) {
            rectangle(realSenseRgbDraw, scissor[i].blade, Scalar(255, 0, 255));
            rectangle(realSenseRgbDraw, scissor[i].handle, Scalar(255, 0, 255));
          }

        hypotenuse = sqrt(pow(((scissor[0].handle.x + scissor[0].handle.width/2) - (scissor[0].blade.x + scissor[0].blade.width/2)), 2) +
            pow((scissor[0].handle.y + scissor[0].handle.height/2) - (scissor[0].blade.y + scissor[0].blade.height/2), 2));

        suckOffsetX = (double)suckOffset*abs((scissor[0].blade.x + scissor[0].blade.width/2) - (scissor[0].handle.x + scissor[0].handle.width/2)) / hypotenuse;
        suckOffsetY = (double)suckOffset*abs((scissor[0].blade.y + scissor[0].blade.height/2) - (scissor[0].handle.y + scissor[0].handle.height/2)) / hypotenuse;

        if(scissor[0].handle.y > scissor[0].blade.y)
            suckPoint = Point{scissor[0].handle.x + scissor[0].handle.width/2 - (int)suckOffsetX,scissor[0].handle.y - (int)suckOffsetY};

        else if(scissor[0].handle.y < scissor[0].blade.y)
            suckPoint = Point{scissor[0].handle.x + scissor[0].handle.width/2 + (int)suckOffsetX,scissor[0].handle.y + scissor[0].handle.height + (int)suckOffset};

        circle(realSenseRgbDraw, suckPoint, 5, Scalar(255, 0, 255), -1);
      }

    else
        cout << "don't find any scissor" << endl;

    imshow("realSenseRgbDraw",realSenseRgbDraw);
    waitKey(0);
*/
    return 0;
}




