#include "FindHole.h"

using namespace std;
using namespace cv;

FindHole::FindHole() {}

void FindHole::getSenseData(SenseData* senseData) {
    realSenseRgb = senseData->rgb[0];
    realSenseDepth = senseData->depth[0];
    realSenseXyz = senseData->xyz[0];
  }

void FindHole::run(PlanData* planData, const YAML::Node& node){
    cout << "-----FindHole Run-----" << endl;
    int iResult;
    findBoundingRect();
    cutBoundingRect();
    iResult = findScissorsHome(node);
    if(iResult == 0)
    {
        findHandleUpDown(planData);
        saveHoleTarget(planData);
        saveHoleDepth(planData);
    }
    else
    cout << "Didn't find any hole"<< endl;
  }

void FindHole::findBoundingRect(){
    threshold(realSenseRgb, img_edge, 120, 255, cv::THRESH_BINARY);
    cvtColor(img_edge, img_edge, cv::COLOR_BGR2GRAY);

    cout << "First Find Contours" << endl;
    cv::findContours(
        img_edge,
        contours,
        hierarchy,
        cv::RETR_CCOMP,
        cv::CHAIN_APPROX_SIMPLE
      );

    sortIdx.resize(contours.size());
    areas.resize(contours.size());
    for( int n = 0; n < (int)contours.size(); n++ ) {
        sortIdx[n] = n;
        areas[n] = contourArea(contours[n], false);
      }

    // sort contours so that the largest contours go first
    std::sort( sortIdx.begin(), sortIdx.end(), AreaCmp(areas));
    idx = sortIdx[0];                                       //find box
    //cv::cvtColor( depth_image, img_color, cv::COLOR_GRAY2BGR );
    // find boundingRect
    boundRect.resize(1);
    boundRect[0] = boundingRect(contours[idx]);
  }

void FindHole::cutBoundingRect(){
    cout << "C U T" << endl;
    Rect rect(boundRect[0].tl().x, boundRect[0].tl().y, boundRect[0].br().x-boundRect[0].tl().x, boundRect[0].br().y-boundRect[0].tl().y);
    cropImage = Mat(realSenseXyz, rect);
  /*imshow("cut", cropImage);
    waitKey();*/
  }

int FindHole::findScissorsHome(const YAML::Node& node){
    cout << "-----Find Scissors Home-----" << endl;
    split(cropImage,xyz);
    xyz[2].copyTo(xyz_z);

    maxTest =xyz[2].at<float>(0, 0);
    minTest =xyz[2].at<float>(0, 0);

    for(number_i = 0; number_i < xyz[2].rows; number_i++) {
        for(number_j = 0; number_j < xyz[2].cols; number_j++) {
            if(maxTest < xyz[2].at<float>(number_i, number_j))
            maxTest = xyz[2].at<float>(number_i, number_j);
            if(minTest > xyz[2].at<float>(number_i, number_j) && xyz[2].at<float>(number_i, number_j) != 0){
                minTest = xyz[2].at<float>(number_i, number_j);
              }
          }
      }
    // normalized
    for(number_i = 0; number_i < xyz_z.rows; number_i++) {
        for(number_j = 0; number_j < xyz_z.cols; number_j++) {
            xyz_z.at<float>(number_i, number_j) = (int)((xyz_z.at<float>(number_i, number_j) - minTest) * 255 / (maxTest-minTest));
          //cout << "zTest[" << number_i << "][" << number_j << "]:" << zTest.at<float>(number_i, number_j);
          }
      }
    xyz_z.convertTo(xyz_z, CV_8U);
    imshow("xyz_z", xyz_z);

    cv::threshold(xyz_z, cropImage_edge, node["Hole"]["threshold"].as<int>(), 255, cv::THRESH_BINARY);

    cout << "Second Find Contours" << endl;

    cropImage_boundRect.clear();
    id.clear();

    cv::findContours(
    cropImage_edge,
    cropImage_contours,
    cropImage_hierarchy,
    cv::RETR_CCOMP,
    cv::CHAIN_APPROX_SIMPLE
  );

    cropImage_sortIdx.resize(cropImage_contours.size());
    cropImage_areas.resize(cropImage_contours.size());
    for( int n = 0; n < (int)cropImage_contours.size(); n++ ) {
        cropImage_sortIdx[n] = n;
        cropImage_areas[n] = contourArea(cropImage_contours[n], false);
      }

    // sort contours so that the largest contours go first
    cout << "sort" << endl;
    std::sort( cropImage_sortIdx.begin(), cropImage_sortIdx.end(), AreaCmp(cropImage_areas));

    for( int n = 0; n < (int)cropImage_sortIdx.size(); n++ ) {
        cropImage_idx = cropImage_sortIdx[n];
        //cv::cvtColor( img_color_copy, cropImage_img_color, cv::COLOR_GRAY2BGR);
        /*cv::drawContours(
        img_depth, cropImage_contours, cropImage_idx,
        cv::Scalar(0,0,255), 2, 1, cropImage_hierarchy,
        0 // Try different values of max_level, and see what happens
      );*/
        //cout << "Contour #" << cropImage_idx << ": area=" << cropImage_areas[cropImage_idx] <<
        //", nvertices=" << cropImage_contours[cropImage_idx].size() << endl;

        if(cropImage_areas[cropImage_idx] > 20000 && cropImage_areas[cropImage_idx] < 30000)
        {
            id.push_back(cropImage_idx);
            shape_areas.push_back(cropImage_areas[cropImage_idx]);
        }
      }

    if(id.size() > 0)
    {
        //find boundingRect
        cropImage_boundRect.resize(id.size());
        for(int i = 0; i < (int)id.size(); i++)
        {
            cropImage_boundRect[i] = boundingRect( cropImage_contours[id[i]] );
        }
        //Draw
        img_depth = realSenseDepth.clone();
        img_Rgb = realSenseRgb.clone();
        //shift contours
        for(size_t i = 0;i < cropImage_contours.size();i++){
            for(size_t j = 0;j < cropImage_contours[i].size();j++){
                cropImage_contours[i][j] = cropImage_contours[i][j]+ Point(boundRect[0].tl().x,boundRect[0].tl().y);
              }
          }
        for(int i = 0; i < (int)id.size(); i++)
        {
            drawContours(
                img_Rgb, cropImage_contours, id[i],
                cv::Scalar(0,0,255), 2, 1, cropImage_hierarchy,
                0                                  // Try different values of max_level, and see what happens
              );
            rectangle(img_depth, cropImage_boundRect[i].tl()+ Point(boundRect[0].tl().x,boundRect[0].tl().y), cropImage_boundRect[i].br()+ Point(boundRect[0].tl().x,boundRect[0].tl().y), Scalar(250,
                100, 200), 2 );
            cout << "Point: "<< cropImage_boundRect[i].tl()+ Point(boundRect[0].tl().x,boundRect[0].tl().y) << cropImage_boundRect[i].br()+ Point(boundRect[0].tl().x,boundRect[0].tl().y) << endl;
        }
        //imshow("depth image", realSenseDepth);
        imshow("scissors home_RGB", img_Rgb);
        imshow("scissors home_depth", img_depth);
        //waitKey();

        return 0;
    }
    else
    return -1;
  }

void FindHole::findHandleUpDown(PlanData* planData){
    cout << "-----Find Handle UpDown-----" << endl;
    save.resize(cropImage_boundRect.size());
    save.clear();
    // get rect and threshold done pic
    cropImage_clone = cropImage_edge.clone();
    for(size_t i = 0 ; i < cropImage_boundRect.size(); i++)
    {
        //cut half
        Rect rect_up(cropImage_boundRect[i].tl().x, cropImage_boundRect[i].tl().y, cropImage_boundRect[i].br().x-cropImage_boundRect[i].tl().x,
            0.5*(cropImage_boundRect[i].br().y-cropImage_boundRect[i].tl().y));
        cropcropImage_up = Mat(cropImage_clone, rect_up);

        Rect rect_down(cropImage_boundRect[i].tl().x, 0.5*(cropImage_boundRect[i].br().y+cropImage_boundRect[i].tl().y), cropImage_boundRect[i].br().x-cropImage_boundRect[i].tl().x,
            0.5*(cropImage_boundRect[i].br().y-cropImage_boundRect[i].tl().y));
        cropcropImage_down = Mat(cropImage_clone, rect_down);

        //Third Find Contours UP
        cout << "Third Find Contours" << endl;
        cv::findContours(
            cropcropImage_up,
            contours_up,
            hierarchy_up,
            cv::RETR_CCOMP,
            cv::CHAIN_APPROX_SIMPLE
          );
        sortIdx_up.resize(contours_up.size());
        areas_up.resize(contours_up.size());
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
        sortIdx_down.resize(contours_down.size());
        areas_down.resize(contours_down.size());
        for( int n = 0; n < (int)contours_down.size(); n++ ) {
            sortIdx_down[n] = n;
            areas_down[n] = contourArea(contours_down[n], false);
          }
        std::sort( sortIdx_down.begin(), sortIdx_down.end(), AreaCmp(areas_down));

        int idx_up = sortIdx_up[0];
        int idx_down = sortIdx_down[0];

        //cout << "Scissors handle area!"<< areas_up[idx_up] <<","<< areas_down[idx_down]<< endl;
        if(areas_up[idx_up] < areas_down[idx_down])
        {
            cout << i << "Scissors handle is down!" << endl;
            save.push_back(0);
        }
        else
        {
            cout << i << "Scissors handle is up!" << endl;
            save.push_back(1);
        }
        contours_up.clear();
        hierarchy_up.clear();
        sortIdx_up.clear();
        areas_up.clear();
        contours_down.clear();
        hierarchy_down.clear();
        sortIdx_down.clear();
        areas_down.clear();
    }
    planData->holeSituation.assign(save.begin(),save.end());
  }

void FindHole::saveHoleTarget(PlanData* planData){
    cout << "-----save HoleTarget-----"<< endl;
    Point temp;
    Point2f realtemp;
    target.clear();
    for(size_t i = 0; i < id.size(); i++)
    {
        temp.x = 0.5 *(cropImage_boundRect[i].tl().x+ boundRect[0].tl().x+ cropImage_boundRect[i].br().x+ boundRect[0].tl().x);
        temp.y = 0.5 *(cropImage_boundRect[i].tl().y+ boundRect[0].tl().y+ cropImage_boundRect[i].br().y+ boundRect[0].tl().y);
        realtemp = Point2f{realSenseXyz.at<Vec3f>(temp.y, temp.x)[0],realSenseXyz.at<Vec3f>(temp.y, temp.x)[1]};
        target.push_back(realtemp);
        if(i == 0)
        circle(cropImage, temp, 5, Scalar(250, 100, 200), -1);
    }
    planData->holeTarget.assign(target.begin(),target.end());
  }

void FindHole::saveHoleDepth(PlanData* planData){
    cout << "-----save HoleDepth-----" << endl;
    double temp;
    depth.clear();
    for(size_t i = 0; i < target.size(); i++)
    {
        temp = realSenseXyz.at<Vec3f>(target[i].y, target[i].x)[2];
        depth.push_back(temp);
    }
    planData->holeDepth.assign(depth.begin(),depth.end());
  //cout << "holeDepth:" << planData->holeDepth[0] << endl;
  }

FindHole::~FindHole() {}
