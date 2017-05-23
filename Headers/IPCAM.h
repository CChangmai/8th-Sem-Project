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

namespace IPCAM
{
    cv::VideoCapture IP_Feed;
    bool status;
    
    cv::Mat IP_Image;
        
    void startRx()
    {   
       
       status=0;
       std::string url;
       
       while(status==0)
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
                 std::cout<<"\n\tDisplaying Image in "<<width<<"x"<<height<<"pixels\n";
                
                 status=1;
                
             }
                 else
                 {
                    std::cout<<"Could Not Find IP Camera Feed For The Given URL\nPlease Try Again\n"; 
                    status=0;   
                 }
       }    
    }
    void getRx()
    {
        if (status==1)
        {
         IP_Feed.read(IP_Image);
        }
    
    
    }
    
    void stopRx()
    {
      IP_Feed.release();
    
    }
    

}



    
    
    




