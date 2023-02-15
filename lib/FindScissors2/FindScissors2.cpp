#include <chrono>
#include "FindScissors2.h"

using namespace std;

string find_scissors::getFileName() {
    auto now = chrono::system_clock::now();
    time_t time = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&time), "%y%m%d_%H%M%S");
    uint64_t microSec =
    chrono::duration_cast<chrono::microseconds>(now.time_since_epoch())
    .count();
    int micro = microSec % 1000000;
    ss << "_" << setfill('0') << std::setw(6) << micro;

    return "Pic_" + ss.str() + ".bmp";
  }


void find_scissors::find_hsv_mask(cv::Mat hsv,int H_low,int S_low,int V_low,int H_high,int S_high,int V_high,cv::Mat image){

    cv::Mat hsvMask;
    cv::inRange(hsv, cv::Scalar(H_low, S_low, V_low), cv::Scalar(H_high, S_high, V_high), hsvMask);
    hsvMask.copyTo(img);
  /*
    vector<cv::Point> ptList;
    for (int i = 0; i < hsvMask.rows; i++)
    {
    for (int j = 0; j < hsvMask.cols; j++)
    {
    if (hsvMask.at<uchar>(i, j) > 128)
    {
    ptList.push_back(cv::Point(j, i));
  }
  }
  }

    cv::Rect objRect = boundingRect(ptList);

    cv::Point center(objRect.x + objRect.width / 2, objRect.y + objRect.height / 2);
    cv::rectangle(image, objRect, cv::Scalar(255, 0, 0), 1);
    cv::circle(image, center, 3, cv::Scalar(0, 0, 255), 2);
   */
  }

void find_scissors::show_hsv_mask(int Color_mask,cv::Mat image)
{
    switch(Color_mask){
        case Color_mask::Black:
        result_black=cv::Scalar(0,0,0);
        cv::bitwise_and(image, image, result_black, img);
        cv::imshow("result_black",result_black);
            //connected_components(threshval,0);
            break;
        case Color_mask::Red:
        result_red=cv::Scalar(0,0,0);
        cv::bitwise_and(image, image, result_red, img);
        cv::imshow("result_red",result_red);
            //connected_components(threshval,0);
            break;
        case Color_mask::Blue:
        result_blue=cv::Scalar(0,0,0);
        cv::bitwise_and(image, image, result_blue, img);
        cv::imshow("result_blue",result_blue);
        connected_components(threshval,0);
            break;
      }
}
void find_scissors::connected_components(int, void*)
{
    cv::Mat bw = threshval < 128 ? (img < threshval) : (img > threshval);
    cv::Mat labelImage(img.size(), CV_32S);
    int nLabels = cv::connectedComponents(bw, labelImage, 8);
    int label;
    vector<cv::Vec3b> colors(nLabels);
    vector<int> pixel_number(nLabels);
    colors[0] = cv::Vec3b(0, 0, 0);                         //background
    for(int label = 1; label < nLabels; ++label){
        colors[label] = cv::Vec3b( (label*255/nLabels), (label*255/nLabels), (label*255/nLabels) );
      }
    cv::Mat dst = cv::Mat::zeros(img.size(), CV_8UC3);
    for(int i = 1; i< nLabels; ++i)
    {
        for(int r = 0; r < dst.rows; ++r){
            for(int c = 0; c < dst.cols; ++c){
                label = labelImage.at<int>(r, c);
                if (label==i)
                pixel_number[i]++;
              }
          }
    }
    for(int r = 0; r < dst.rows; ++r){
        for(int c = 0; c < dst.cols; ++c){
            label = labelImage.at<int>(r, c);
            cv::Vec3b &pixel = dst.at<cv::Vec3b>(r, c);
            if(pixel_number[label]>300)
            pixel = colors[label];
          }
      }

    vector<cv::Point> ptList;
    cv::Rect objRect[nLabels];
    for(int label = 1; label< nLabels; ++label)
    {
        if(pixel_number[label]>300)
        {
            for(int r = 0; r < dst.rows; ++r)
            {
                for(int c = 0; c < dst.cols; ++c)
                {
                    if (labelImage.at<uchar>(r, c) > 20)
                    {
                        ptList.push_back(cv::Point(c, r));
                    }
                }
            }
            objRect[label] = boundingRect(ptList);

            cv::Point center(objRect[label].x + objRect[label].width / 2, objRect[label].y + objRect[label].height / 2);
            cv::rectangle(dst, objRect[label], cv::Scalar(255, 0, 0), 1);
            cv::circle(dst, center, 3, cv::Scalar(0, 0, 255), 2);
        }
    }
    imshow( "Connected Components", dst );
}

void find_scissors::create_trackbar()
{
    cv::namedWindow("HSVParameter", 1);
    hsv_value.H_low = 0;
    cv::createTrackbar("H_low", "HSVParameter", &hsv_value.H_low, 255);
    hsv_value.H_high = 255;
    cv::createTrackbar("H_high", "HSVParameter", &hsv_value.H_high, 255);
    hsv_value.S_low = 0;
    cv::createTrackbar("S_low", "HSVParameter", &hsv_value.S_low, 255);
    hsv_value.S_high = 255;
    cv::createTrackbar("S_high", "HSVParameter", &hsv_value.S_high, 255);
    hsv_value.V_low = 0;
    cv::createTrackbar("V_low", "HSVParameter", &hsv_value.V_low, 255);
    hsv_value.V_high = 255;
    cv::createTrackbar("V_high", "HSVParameter", &hsv_value.V_high, 255);

}
