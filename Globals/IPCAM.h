#ifndef HAS_OPENCV
    #include<opencv2/core/core.hpp>
    #include<opencv2/imgproc/imgproc.hpp>
    #include<opencv2/highgui/highgui.hpp>
    #define HAS_OPENCV
#endif

#ifndef HAS_IOSTREAM
    #include<iostream>
    #define HAS_IOSTREAM
#endif

using namespace std;

VideoCapture IP_Feed;
namespace IPCAM
{
    void startRx()
    {   
       string url;
       cout<<"Enter URL Of The IP Camera : " ;
       cin>>url;
       cout<<"\n Entered URL Is : "<<url<<endl; 
           if (IP_Feed.isOpened())
            {
                /*Set Frame Rate to more than  60 FPS
                  Because Less Than That Gives Stuttering            
                */
                IP_Feed.set(CV_CAP_PROP_FPS,120);
                double width=IP_Feed.get(CV_CAP_PROP_FRAME_WIDTH);            
                double height=IP_Feed.get(CV_CAP_PROP_FRAME_HEIGHT);
                cout<<"\n\tDisplaying Image in "<<width<<"x"<<height<<"pixels";
        }
        else
        {
               cout<<"Could Not Find IP Camera Feed For The Given URL";    
        }
    
    }
    

}



    
    
    

}


