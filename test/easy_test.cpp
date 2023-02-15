#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
 
using namespace std;
using namespace cv;
 
int main()
{
	/*Mat img = Mat::zeros(Size(800, 600), CV_8UC3);
	
	img.setTo(255);                     
 
	Point p(20, 20);
	circle(img, p, 4, Scalar(255, 0, 255), -1);

	imshow("Img", img);
	waitKey();*/
    
    vector<int> a(3,1);
    vector<int> b(5,2);
    vector<int> c;
    
    c.insert(c.end(),a.begin(),a.end());
    c.insert(c.end(),b.begin(),b.end());
    
    for(size_t i = 0; i < c.size(); i++)
        cout << c[i] << "/";
        
    cout << endl;
    
	return 0;
}
