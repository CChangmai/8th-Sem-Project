#include<iostream>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

//#include "GPU2.h"

using namespace cv;
using namespace std;
int main(int argc,char* argv[])
{
    VideoCapture cap; //
    cap.open(argv[1]);
    if (!cap.isOpened())  // if not success, exit program
    {
        cout<<argv[1];
        cout << "Cannot open the video cam" << endl;
        return -1;
    }

   int dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
   int dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

   
    //cout << "Frame size : " << dWidth << " x " << dHeight << endl;

    namedWindow("MyVideo",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"
    //system("mjpg_streamer -i \"input_file.so -f /tmp -n livefeed.jpg\" -o \"output_http.so -w /usr/local/www -p 1600\"");
    Mat frame;
    Mat contours;
    Mat newRect;
    Mat prevRect; 
    
    while (1)
    {
        bool bSuccess = cap.read(frame); // read a new frame from video

         if (!bSuccess) //if not success, break loop
        {
             cout << "Cannot read a frame from video stream" << endl;
             break;
        }

        //Watch out for any changes
        
       
                     rectangle(frame,
                               Point(midw-50,midh-50),
                               Point(midw+50,midh+50),
                               Scalar(255,0,0),
                               8,
                               0.25,
                               0);
                  

        
        imshow("MyVideo", frame); //show the frame in "MyVideo" window
        //To Write Back To The Phone Now
        //imwrite("/tmp/livefeed.jpg",contours);
        
      
        //Release The Memory
        
           
        
        
        if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
       {
            cout << "esc key is pressed by user" << endl;
            break;
       }
    }
    return 0; 

}
