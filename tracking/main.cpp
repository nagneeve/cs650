#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/video.hpp"

using namespace cv;
using namespace std;

Mat frame;
Mat hsv;
Mat hue;

Point point1, point2;

const char* src_window = "Select ROI";

int drag = 0;
bool setup = false;

Rect rect_roi;

// histogram stuff
MatND hist;
float h_range[] = {0, 179};
float s_range[] = {0, 255};
const float* ranges[] = {h_range, s_range};
int channels[] = {0, 1};

// function prototype
void mouseHandler(int event, int x, int y, int flags, void* param);
void getHistogram(Mat roi);

int main(int argc, char** argv)
{
    VideoCapture cap("messi.mp4");

    if( !cap.isOpened() )
    {
        cout << "Could not initialize capturing...\n";
        return -1;
    }
    
    namedWindow(src_window,CV_WINDOW_AUTOSIZE);
    setMouseCallback(src_window,mouseHandler,0);
    
    for(;;){
        cap >> frame;
        cvtColor(frame, hsv, CV_BGR2HSV);

        if(setup){
            MatND backproj;
            calcBackProject(&hsv, 1, channels, hist, backproj, ranges, 1, true);
            imshow("BackProj", backproj);
            int max_iteration = 30;
            double desired_accuracy = 0.1;
            int itrs = meanShift(backproj, rect_roi, TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, max_iteration, desired_accuracy));
            rectangle(frame, rect_roi, Scalar(255,0,0), 2, CV_AA);

        }
        imshow(src_window,frame);

        if(waitKey(0) == 27){
            cout <<"esc key is pressed by user" << endl;
            break;
        }
    }

    return 0;
}

void getHistogram(Mat roi){
    int ch[] = {0, 0};
    int h_bins = 100;    // initial 30
    int s_bins = 100;    // initial 32
    int histSize[] = {h_bins,s_bins};
    
    calcHist(&roi, 1, channels, Mat(), hist, 2, histSize, ranges, true, false);
    normalize(hist,hist,0,255,NORM_MINMAX,-1,Mat());
   
}

void mouseHandler(int event, int x, int y, int flags, void* param)
{
    if (event == CV_EVENT_LBUTTONDOWN && !drag)
    {
        /* left button clicked. ROI selection begins */
        point1 = Point(x, y);
        drag = 1;
        setup = false;
    }

    if (event == CV_EVENT_MOUSEMOVE && drag)
    {
        /* mouse dragged. ROI being selected */
        Mat img1 = frame.clone();
        point2 = Point(x, y);
        rectangle(img1, point1, point2, CV_RGB(255, 0, 0), 1, 8, 0);
        
        imshow(src_window, img1);
    }

    if (event == CV_EVENT_LBUTTONUP && drag)
    {
        Mat img2 = frame.clone();

        point2 = Point(x, y);
        drag = 0;

        cout << "point1  " << point1.x << "  " << point1.y << endl;
        cout << "point2  " << point2.x << "  " << point2.y << endl;
        rect_roi = Rect(point1.x, point1.y, (point2.x-point1.x+1), (point2.y-point1.y+1));
        Mat roi(img2, rect_roi);
        cvtColor(roi, roi, CV_BGR2HSV);
        
        namedWindow("roi",WINDOW_NORMAL);
        imshow("roi",roi);        
        getHistogram(roi);
        setup = true;
    }
}
