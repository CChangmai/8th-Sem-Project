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

#ifndef FPS
    #define FPS 30
#endif

namespace IPCAM
{
    cv::VideoCapture IP_Feed;
    bool status=false;
    std::string url;
    
    // cv::Mat IP_Image;
    
       
    void startRx()
    {   
       
       status=false;
      
       
       while(status==false)
       {
       std::cout<<"Enter URL Of The IP Camera : " ;
       std::cin>>url;
       std::cout<<"\n Entered URL Is : "<<url<<std::endl; 
       IP_Feed.open(url);
       
                if (IP_Feed.isOpened())
                {
                 /*Set Frame Rate to more than  60 FPS
                   Because Less Than That Gives Stuttering            
                 */
                 IP_Feed.set(CV_CAP_PROP_FPS,120);
                 double width=IP_Feed.get(CV_CAP_PROP_FRAME_WIDTH);            
                 double height=IP_Feed.get(CV_CAP_PROP_FRAME_HEIGHT);
                 std::cout<<"\n\tDisplaying Image in "<<width<<"x"<<height<<" pixels\n";
                
                 status=true;
                
             }
                 else
                 {
                    std::cout<<"Could Not Find IP Camera Feed For The Given URL\nPlease Try Again\n"; 
                    status=false;   
                 }
       }    
    }
    void getRx(cv::Mat &img)
    {
        if (status==true)
        {
         IP_Feed.read(img);
        }
    
    
    }
    
    void RefreshRx(float time)
    {
    
       /* WE SET THE PROPERTY OF THE VIDEOCAPTURE TO GET THE NEXT FEED */
       int current_frame = (int)IP_Feed.get(CV_CAP_PROP_POS_FRAMES);
       int frameskip = (int)(time*FPS);
       IP_Feed.set(CV_CAP_PROP_POS_FRAMES,current_frame+frameskip-1);
      
    }
    
    void stopRx()
    {
      IP_Feed.release();
    
    }
    

}



    
    
    




