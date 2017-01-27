#include<iostream>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>


using namespace cv;
using namespace std;

int main()
{ 
   
    string url;
    cout<<"Enter URL Of The IP Camera : " ;
    cin>>url;
    cout<<"\n Entered URL Is : "<<url<<endl;
    VideoCapture IP_Feed;
    bool frame_status=0;

    //Read Image From URL
    IP_Feed.open(url);
    
    
    //Test If IPCAM is open

    if (IP_Feed.isOpened())
    {
            /*Set Frame Rate to 60 FPS
              Because Less Than That Gives Stuttering            
            */
            IP_Feed.set(CV_CAP_PROP_FPS,120);
            double width=IP_Feed.get(CV_CAP_PROP_FRAME_WIDTH);            
            double height=IP_Feed.get(CV_CAP_PROP_FRAME_HEIGHT);
            cout<<"\n\tDisplaying Image in "<<width<<"x"<<height<<"pixels";

            /*Make A Resizable Window According To The Feed*/
            
            namedWindow("Live Feed",CV_WINDOW_AUTOSIZE);            
            resizeWindow("Live Feed",width,height);

    while(1)
            {

                /* The Lengthy Processing Part*/


                
                Mat image;
                frame_status=IP_Feed.read(image);
                if (frame_status)
                {
                    imshow("Live Feed",image);
                    /*
                    Optional Key to End The Stream
                                
                    */
                     waitKey(1);
                }

                else
                {
                    cout<<"Error. Cannot Read Any Frame From The Live Feed";
                    waitKey(0);
                }
                
            }

    }    
    else
    {
        cout<<"Could Not Find IP Camera Feed For The Given URL";    
    }




    return 0;
}







