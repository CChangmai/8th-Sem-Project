#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include<cstdlib>

using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
    VideoCapture cap; //
    string s;  
    cout<<"Enter IP : ";
    cin>>s; 
    //Testing
    s="http://192.168.1.2:3125/videofeed?dummy=param.mjpg";
    cap.open(s);
    if (!cap.isOpened())  // if not success, exit program
    {
        //cout << "Cannot open the video cam" << endl;
        return -1;
    }

   double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
   double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

    //cout << "Frame size : " << dWidth << " x " << dHeight << endl;

    namedWindow("MyVideo",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"
    //system("mjpg_streamer -i \"input_file.so -f /tmp -n livefeed.jpg\" -o \"output_http.so -w /usr/local/www -p 1600\"");
    
     
    
    while (1)
    {
       Mat frame;
       Mat contours;
       bool bSuccess = cap.read(frame); // read a new frame from video

         if (!bSuccess) //if not success, break loop
        {
             cout << "Cannot read a frame from video stream" << endl;
             break;
        }

       
        

        Canny(frame,contours,500,1000,5,true);
        imshow("MyVideo", frame); //show the frame in "MyVideo" window
        //To Write Back To The Phone Now
        //imwrite("/tmp/livefeed.jpg",contours);
        
        cout<<contours<<endl;
        
        //Release The Memory
        frame.release();
        contours.release();       
        
        
        if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
       {
            cout << "esc key is pressed by user" << endl;
            break;
       }
    }
    return 0;
}
