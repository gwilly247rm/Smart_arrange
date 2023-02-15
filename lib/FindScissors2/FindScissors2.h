#ifndef FindScissors2_H
# define FindScissors2_H

# include <iostream>
# include <opencv2/opencv.hpp>
# include <string>

using namespace std;

struct HSV_PARAMETER {
    int H_low;
    int S_low;
    int V_low;
    int H_high;
    int S_high;
    int V_high;
  };

class find_scissors {
public:
    enum Color_mask{ Black, Red, Blue};
    HSV_PARAMETER hsv_value;

    string getFileName();
    void find_hsv_mask(cv::Mat,int, int, int, int, int, int,cv::Mat);
    void connected_components(int, void*);
    void show_hsv_mask(int Color_mask,cv::Mat image);
    void create_trackbar();

protected:

    cv::Mat result_black,result_red,result_blue;
    cv::Mat img;
    int threshval = 160;

  };

#endif
