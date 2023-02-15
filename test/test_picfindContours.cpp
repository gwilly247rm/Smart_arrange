#include <iostream>
#include <string>

#include "realsense.h"

using namespace std;
using namespace cv;

struct AreaCmp {
    AreaCmp(const vector<float>& _areas) : areas(&_areas) {}
    bool operator()(int a, int b) const { return (*areas)[a] > (*areas)[b]; }
    const vector<float>* areas;
};

//static void canny_on_trackbar(int canny_ratio, void*) {}
static void threshold_on_trackbar(int threshold_ratio, void*) {}
static void threshold2_on_trackbar(int threshold2_ratio, void*) {}

int main(int argc, char* argv[]) {
    
    Mat img, img_edge, img_color;
    Mat cropImage, cropImage_img, cropImage_edge, cropImage_img_color;
    
    //int canny_ratio;
    int threshold_ratio;
    int threshold2_ratio;
    
    vector<int> id;
    vector<float> shape_areas;
    
    // load image or show help if no image was provided
    //
    if( argc != 2 || (img = cv::imread(argv[1],cv::IMREAD_GRAYSCALE)).empty() ) {
        cout << "\nERROR: You need 2 parameters, you had " << argc << "\n" << endl;
        cout << "\nExample 14_2: Drawing Contours\nCall is:\n" << argv[0] << " <image>\n\n"
        << "Example:\n" << argv[0] << " ../box.png\n" << endl;
        return -1;
    }
    //test canny
    
    //cvtColor(img, img_edge, cv::COLOR_BGR2GRAY);
    /*while(1){
    cout << "canny" << endl;
    namedWindow("Canny", WINDOW_AUTOSIZE);  // Create Window
    createTrackbar("ratio", "Canny", &canny_ratio, 20, canny_on_trackbar);
    Canny(img, img_edge, 50,canny_ratio * 50);
    imshow("Canny", img_edge);
    if (waitKey(1) == 27) break;
    cout << "after canny" << endl;
    }*/
    while(1){
    namedWindow("T", WINDOW_AUTOSIZE);  // Create Window
    createTrackbar("ratio", "T", &threshold_ratio, 255, threshold_on_trackbar);
    cv::threshold(img, img_edge, threshold_ratio, 255, cv::THRESH_BINARY);//128,255
    //adaptiveThreshold(img,img_edge,threshold_ratio,ADAPTIVE_THRESH_GAUSSIAN_C ,cv::THRESH_BINARY,15,0);
    cv::imshow("Image after threshold", img_edge);
    if (waitKey(1) == 27) break;
    }
    vector< vector< cv::Point > > contours;
    vector< cv::Vec4i > hierarchy;
    
    cv::findContours(
        img_edge,
        contours,
        hierarchy,
        cv::RETR_CCOMP,
        cv::CHAIN_APPROX_SIMPLE
    );
    cout << "\n\nHit any key to draw the next contour, ESC to quit\n\n";
    cout << "Total Contours Detected: " << contours.size() << endl;
    /*vector<Rect> boundRect( contours.size() );
    vector<vector<Point> > contours_poly( contours.size() );
     for( size_t i = 0; i < contours.size(); i++ )
    {
        boundRect[i] = boundingRect( contours_poly[i] );
    }
    cv::cvtColor( img, img_color, cv::COLOR_GRAY2BGR );
    Mat img_color_copy = img_color.clone();
    for( size_t i = 0; i< contours.size(); i++ )
    {
        cv::drawContours(
        img_color, contours, i,
        cv::Scalar(0,0,255), 2, 8, hierarchy,
                     0 // Try different values of max_level, and see what happens
    );
        rectangle(img_color, boundRect[i].tl(), boundRect[i].br(), Scalar(250, 100, 200), 2 );
    }*/

    vector<int> sortIdx(contours.size());
    vector<float> areas(contours.size());
    for( int n = 0; n < (int)contours.size(); n++ ) {
        sortIdx[n] = n;
        areas[n] = contourArea(contours[n], false);
    }
    
    // sort contours so that the largest contours go first
    //
    std::sort( sortIdx.begin(), sortIdx.end(), AreaCmp(areas ));
    int idx = sortIdx[0]; //the largest contour
    cv::cvtColor( img, img_color, cv::COLOR_GRAY2BGR );
    Mat img_color_copy = img_color.clone();
    // find boundingRect
    vector<Rect> boundRect(1);
    boundRect[0] = boundingRect(contours[idx]);
    rectangle(img_color, boundRect[0].tl(), boundRect[0].br(), Scalar(200, 200, 200), 2);
    cout << "Point: " << boundRect[0].tl() << boundRect[0].br() << endl;
    cv::drawContours(
        img_color, contours, idx,
        cv::Scalar(0,0,255), 2, 8, hierarchy,
                     0 // Try different values of max_level, and see what happens
    );
    imshow(argv[0], img_color);
    waitKey();
    
    //cut
    Rect rect(boundRect[0].tl().x, boundRect[0].tl().y, boundRect[0].br().x-boundRect[0].tl().x, boundRect[0].br().y-boundRect[0].tl().y);
    cropImage = Mat(img_color_copy, rect);
    imshow("cut", cropImage);
    waitKey();
    
    //findContours
    while(1){
    namedWindow("Image after threshold", WINDOW_AUTOSIZE);  // Create Window
    createTrackbar("ratio", "Image after threshold", &threshold2_ratio, 200, threshold2_on_trackbar);
    cv::threshold(cropImage, cropImage_edge, threshold2_ratio, 255, cv::THRESH_BINARY);
    cv::imshow("Image after threshold", cropImage_edge);
    if (waitKey(1) == 27) break;
    }
    cvtColor(cropImage_edge, cropImage_edge, cv::COLOR_BGR2GRAY);
    
    vector< vector< cv::Point > > cropImage_contours;
    vector< cv::Vec4i > cropImage_hierarchy;
    
    cout << "findContours" << endl;
    cv::findContours(
        cropImage_edge,
        cropImage_contours,
        cropImage_hierarchy,
        cv::RETR_CCOMP,
        cv::CHAIN_APPROX_SIMPLE
    );  
    
    vector<int> cropImage_sortIdx(cropImage_contours.size());
    vector<float> cropImage_areas(cropImage_contours.size());
    for( int n = 0; n < (int)cropImage_contours.size(); n++ ) {
        cropImage_sortIdx[n] = n;
        cropImage_areas[n] = contourArea(cropImage_contours[n], false);
    }
    
    // sort contours so that the largest contours go first
    cout << "sort" << endl;
    std::sort( cropImage_sortIdx.begin(), cropImage_sortIdx.end(), AreaCmp(cropImage_areas ));
    
    for( int n = 0; n < (int)cropImage_sortIdx.size(); n++ ) {
        int cropImage_idx = cropImage_sortIdx[n];
        //cv::cvtColor( img_color_copy, cropImage_img_color, cv::COLOR_GRAY2BGR);
        cv::drawContours(
            cropImage, cropImage_contours, cropImage_idx,
            cv::Scalar(0,0,255), 2, 1, cropImage_hierarchy,
                         0 // Try different values of max_level, and see what happens
        );
        cout << "Contour #" << cropImage_idx << ": area=" << cropImage_areas[cropImage_idx] <<
        ", nvertices=" << cropImage_contours[cropImage_idx].size() << endl;
        
        if(cropImage_areas[cropImage_idx] > 20000 && cropImage_areas[cropImage_idx] < 60000)
        {
            id.push_back(cropImage_idx);
            shape_areas.push_back(cropImage_areas[cropImage_idx]);
        }
        /*imshow("Scissors Home", cropImage);
        int k;
            if ((k = cv::waitKey() & 255) == 27)
                break;*/
    }
    
    //find boundingRect
    vector<Rect> cropImage_boundRect(id.size());
    for(int i = 0; i < id.size(); i++)
    {
        cropImage_boundRect[i] = boundingRect( cropImage_contours[id[i]] );
    }
    //Draw
    for(int i = 0; i < id.size(); i++)
    {
        drawContours(
            cropImage, cropImage_contours, id[i],
            cv::Scalar(0,0,255), 2, 1, cropImage_hierarchy,
                     0 // Try different values of max_level, and see what happens
        );
        rectangle(cropImage, cropImage_boundRect[i].tl(), cropImage_boundRect[i].br(), Scalar(250, 100, 200), 2 );
        cout << "Point: "<< cropImage_boundRect[i].tl() << cropImage_boundRect[i].br() << endl;
    }
    
    imshow("Scissors Home", cropImage);
    waitKey();
    
    return 0;
}

