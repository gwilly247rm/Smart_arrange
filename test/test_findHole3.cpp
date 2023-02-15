#include <iostream>
#include <opencv2/opencv.hpp>
#include "realsense.h"

using namespace std;
using namespace cv;
using namespace realsense;

struct AreaCmp {
  AreaCmp(const vector<float>& _areas) : areas(&_areas) {}
  bool operator()(int a, int b) const { return (*areas)[a] > (*areas)[b]; }
  const vector<float>* areas;
};

//original find box and cut depth image to find contours
int main(int argc, char** argv) {

    RealSense rs;
    Config config;

    Mat realsenseRgb;
    Mat depth_image;
    Mat realsenseXyz;
    Mat img_edge, img_Rgb;
    Mat cropImage, cropImage_edge, img_depth;

    Mat cropcropImage_up,cropcropImage_down;

    vector< vector< cv::Point > > contours;
    vector< cv::Vec4i > hierarchy;
    vector<int> id;
    vector<float> shape_areas;

    vector< vector< cv::Point > > contours_up;
    vector< cv::Vec4i > hierarchy_up;
    vector< vector< cv::Point > > contours_down;
    vector< cv::Vec4i > hierarchy_down;

    bool run = true;

    // RealSense init
    cout << "RealSenseInit" << endl;
    config.device_type = DeviceType::CAMERA_ID;
    config.camera_id = 0;

    StreamConfig color_config;
    color_config.stream = RS2_STREAM_COLOR;
    color_config.width = 1280;
    color_config.height = 720;

    StreamConfig depth_config;
    depth_config.stream = RS2_STREAM_DEPTH;
    depth_config.width = 640;
    depth_config.height = 480;

    StreamConfig infrared_config;
    infrared_config.stream = RS2_STREAM_INFRARED;
    infrared_config.width = 640;
    infrared_config.height = 480;

    config.stream_configs.push_back(color_config);
    config.stream_configs.push_back(depth_config);
    config.stream_configs.push_back(infrared_config);

    cout << "connect RealSense" << endl;
    rs.connect(config);

    if (rs.depth_supports(RS2_OPTION_MIN_DISTANCE))
    rs.set_depth_option(RS2_OPTION_MIN_DISTANCE, 400);
    rs.set_align_stream(RS2_STREAM_COLOR);
    rs.set_colorizer_option(RS2_OPTION_COLOR_SCHEME, 0);
    rs.enable_hole_filling_filter(true);
    rs.set_hole_filling_filter_option(RS2_OPTION_HOLES_FILL, 1);

    while (run) {

        rs.update();
        rs.retrieve_color_image(realsenseRgb);
        rs.retrieve_depth_image(depth_image);
        rs.retrieve_xyz_measure(realsenseXyz);

        threshold(realsenseRgb, img_edge, 120, 255, cv::THRESH_BINARY);
        cvtColor(img_edge, img_edge, cv::COLOR_BGR2GRAY);
        imshow("realsenseRgb", realsenseRgb);
        imshow("img_edge", img_edge);
        waitKey();

        cout << "First Find Contours" << endl;
        cv::findContours(
            img_edge,
            contours,
            hierarchy,
            cv::RETR_CCOMP,
            cv::CHAIN_APPROX_SIMPLE
          );

        vector<int> sortIdx(contours.size());
        vector<float> areas(contours.size());
        for( int n = 0; n < (int)contours.size(); n++ ) {
            sortIdx[n] = n;
            areas[n] = contourArea(contours[n], false);
        }

        // sort contours so that the largest contours go first
        std::sort( sortIdx.begin(), sortIdx.end(), AreaCmp(areas));
        int idx = sortIdx[0]; //find box
        //cv::cvtColor( depth_image, img_color, cv::COLOR_GRAY2BGR );
        img_Rgb = realsenseRgb.clone();
        // find boundingRect
        vector<Rect> boundRect(1);
        boundRect[0] = boundingRect(contours[idx]);
        rectangle(img_Rgb, boundRect[0].tl(), boundRect[0].br(), Scalar(200, 200, 200), 2);
        //cout << "Point: " << boundRect[0].tl() << boundRect[0].br() << endl;
        cv::drawContours(
            img_Rgb, contours, idx,
            cv::Scalar(0,0,255), 2, 8, hierarchy,
                         0 // Try different values of max_level, and see what happens
        );
        /*imshow(argv[0], img_Rgb);
        waitKey();*/

        //cut
        Rect rect(boundRect[0].tl().x, boundRect[0].tl().y, boundRect[0].br().x-boundRect[0].tl().x, boundRect[0].br().y-boundRect[0].tl().y);
        cropImage = Mat(depth_image, rect);
        imshow("cut", cropImage);
        waitKey();

        //new
        vector<cv::Mat>xyzTest;
        cv::Mat xyzTestCrop;
        cv::Mat zTest;
        double maxTest;
        double minTest;
        int number_i;
        int number_j;

        xyzTestCrop = Mat(realsenseXyz, rect);
        split(xyzTestCrop,xyzTest);
        xyzTest[2].copyTo(zTest);

        maxTest =xyzTest[2].at<float>(0, 0);
        minTest =xyzTest[2].at<float>(0, 0);

        for(number_i = 0; number_i < xyzTest[2].rows; number_i++) {
            for(number_j = 0; number_j < xyzTest[2].cols; number_j++) {
                if(maxTest < xyzTest[2].at<float>(number_i, number_j))
                maxTest = xyzTest[2].at<float>(number_i, number_j);
                if(minTest > xyzTest[2].at<float>(number_i, number_j) && xyzTest[2].at<float>(number_i, number_j) != 0){
                    minTest = xyzTest[2].at<float>(number_i, number_j);
                  }
              }
          }

        for(number_i = 0; number_i < zTest.rows; number_i++) {
            for(number_j = 0; number_j < zTest.cols; number_j++) {
                zTest.at<float>(number_i, number_j) = (int)((zTest.at<float>(number_i, number_j) - minTest) * 255 / (maxTest-minTest));
                //cout << "zTest[" << number_i << "][" << number_j << "]:" << zTest.at<float>(number_i, number_j);
              }
          }
        zTest.convertTo(zTest, CV_8U);
        imshow("zTest", zTest);

        cv::threshold(zTest, cropImage_edge, 125, 255, cv::THRESH_BINARY);//160
        //new end

        imshow("cropImage_edge", cropImage_edge);
        waitKey();
        //cvtColor(cropImage_edge, cropImage_edge, cv::COLOR_BGR2GRAY);

        vector< vector< cv::Point > > cropImage_contours;
        vector< cv::Vec4i > cropImage_hierarchy;

        cout << "Second Find Contours" << endl;
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
        std::sort( cropImage_sortIdx.begin(), cropImage_sortIdx.end(), AreaCmp(cropImage_areas));

        for( int n = 0; n < (int)cropImage_sortIdx.size(); n++ ) {
            int cropImage_idx = cropImage_sortIdx[n];
            //cv::cvtColor( img_color_copy, cropImage_img_color, cv::COLOR_GRAY2BGR);
            img_depth = depth_image.clone();
            cv::drawContours(
                img_depth, cropImage_contours, cropImage_idx,
                cv::Scalar(0,0,255), 2, 1, cropImage_hierarchy,
                             0 // Try different values of max_level, and see what happens
            );
            cout << "Contour #" << cropImage_idx << ": area=" << cropImage_areas[cropImage_idx] <<
            ", nvertices=" << cropImage_contours[cropImage_idx].size() << endl;

            if(cropImage_areas[cropImage_idx] > 20000 && cropImage_areas[cropImage_idx] < 30000)
            {
                id.push_back(cropImage_idx);
                shape_areas.push_back(cropImage_areas[cropImage_idx]);
            }
            else
                cout << "Didn't find any hole"<< endl;
        }

        //find boundingRect
        vector<Rect> cropImage_boundRect(id.size());
        for(int i = 0; i < (int)id.size(); i++)
        {
            cropImage_boundRect[i] = boundingRect( cropImage_contours[id[i]] );
        }

        vector<int> save(cropImage_boundRect.size());
        save.clear();
        cv::Mat cropImage_clone = cropImage_edge.clone();
        //cut half
        for(size_t i = 0 ; i < cropImage_boundRect.size(); i++)
        {
            Rect rect_up(cropImage_boundRect[i].tl().x, cropImage_boundRect[i].tl().y, cropImage_boundRect[i].br().x-cropImage_boundRect[i].tl().x, 0.5*(cropImage_boundRect[i].br().y-cropImage_boundRect[i].tl().y));
            cropcropImage_up = Mat(cropImage_clone, rect_up);

            Rect rect_down(cropImage_boundRect[i].tl().x, 0.5*(cropImage_boundRect[i].br().y+cropImage_boundRect[i].tl().y), cropImage_boundRect[i].br().x-cropImage_boundRect[i].tl().x, 0.5*(cropImage_boundRect[i].br().y-cropImage_boundRect[i].tl().y));
            cropcropImage_down = Mat(cropImage_clone, rect_down);
            /*imshow("cut_up", cropcropImage_up);
            imshow("cut_down", cropcropImage_down);
            waitKey();*/

            //get z in cropImage
            /*vector<cv::Mat>xyz_up;
            vector<cv::Mat>xyz_down;
            split(cropcropImage_up,xyz_up);
            split(cropcropImage_down,xyz_down);
            cv::threshold(xyz_up[2], xyz_up[2], 200, 255, cv::THRESH_BINARY);
            cv::threshold(xyz_down[2], xyz_down[2], 200, 255, cv::THRESH_BINARY);
            imshow("z_up", xyz_up[2]);
            imshow("z_down", xyz_down[2]);
            waitKey();*/

            //Third Find Contours UP
            cout << "Third Find Contours" << endl;
            cv::findContours(
                cropcropImage_up,
                contours_up,
                hierarchy_up,
                cv::RETR_CCOMP,
                cv::CHAIN_APPROX_SIMPLE
            );
            vector<int> sortIdx_up(contours_up.size());
            vector<float> areas_up(contours_up.size());
            for( int n = 0; n < (int)contours_up.size(); n++ ) {
                sortIdx_up[n] = n;
                areas_up[n] = contourArea(contours_up[n], false);
            }
            std::sort( sortIdx_up.begin(), sortIdx_up.end(), AreaCmp(areas_up));
            //Third Find Contours DOWN
            cv::findContours(
                cropcropImage_down,
                contours_down,
                hierarchy_down,
                cv::RETR_CCOMP,
                cv::CHAIN_APPROX_SIMPLE
            );
            vector<int> sortIdx_down(contours_down.size());
            vector<float> areas_down(contours_down.size());
            for( int n = 0; n < (int)contours_down.size(); n++ ) {
                sortIdx_down[n] = n;
                areas_down[n] = contourArea(contours_down[n], false);
            }
            std::sort( sortIdx_down.begin(), sortIdx_down.end(), AreaCmp(areas_down));

            int idx_up = sortIdx_up[0];
            int idx_down = sortIdx_down[0];
            /*drawContours(
                cropcropImage_up, contours_up, idx_up,
                cv::Scalar(0,0,255), 2, 1, hierarchy_up,
                         0 // Try different values of max_level, and see what happens
            );
            drawContours(
                cropcropImage_down, contours_down, idx_down,
                cv::Scalar(0,0,255), 2, 1, hierarchy_down,
                         0 // Try different values of max_level, and see what happens
            );
            imshow("up", cropcropImage_up);
            imshow("down", cropcropImage_down);
            waitKey();*/

            cout << "Scissors handle area!"<< areas_up[idx_up] <<","<< areas_down[idx_down]<< endl;
            if(areas_up[idx_up] > areas_down[idx_down])
            {
                //cout <<"Scissors handle is up!" << endl;
                cout << i << "Scissors handle is down!" << endl;
                save.push_back(0);
            }
            else
            {
                //cout << "Scissors handle is down!" << endl;
                cout << i << "Scissors handle is up!" << endl;
                save.push_back(1);
            }
            //xyz_up.clear();
            //xyz_down.clear();
            contours_up.clear();
            hierarchy_up.clear();
            sortIdx_up.clear();
            areas_up.clear();
            contours_down.clear();
            hierarchy_down.clear();
            sortIdx_down.clear();
            areas_down.clear();
        }
        for(size_t i = 0 ; i < save.size(); i++)
        {
         cout << "1 is handleUp! 0 is handleDown!"<< save[i] << endl;
        }
        //Draw
        for(int i = 0; i < (int)id.size(); i++)
        {
            drawContours(
                cropImage, cropImage_contours, id[i],
                cv::Scalar(0,0,255), 2, 1, cropImage_hierarchy,
                         0 // Try different values of max_level, and see what happens
            );
            rectangle(cropImage, cropImage_boundRect[i].tl(), cropImage_boundRect[i].br(), Scalar(250, 100, 200), 2 );
            cout << "Point: "<< cropImage_boundRect[i].tl() << cropImage_boundRect[i].br() << endl;
        }

        imshow("pictureRgb", realsenseRgb);
        imshow("pictureDepth", img_depth);
        imshow("Scissors Home", depth_image);

        id.clear();
        shape_areas.clear();

        if (waitKey() == 27) break;
    }
    return 0;
}
