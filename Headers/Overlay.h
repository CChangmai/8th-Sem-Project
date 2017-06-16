#ifndef HAS_IOSTREAM
    #include<iostream>
    #include<vector>
    #define HAS_IOSTREAM
#endif
    
#ifndef HAS_OPENCV
    #include<opencv2/core/core.hpp>
    #include<opencv2/highgui/highgui.hpp>
    #include<opencv2/imgproc/imgproc.hpp>
    #define HAS_OPENCV
#endif    



void Write_Text(std::vector <std::string> &temp,cv::Mat &pic)
{
    int scale_factor= (pic.rows/10);
    int start_pt=scale_factor*temp.size();
    
    for(int i=0; i<temp.size();i++)
    {
                        putText(pic,temp[i].c_str(), cv::Point(0,(pic.rows-start_pt+(i*20))), 
                                            cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255,100,50), 1, CV_AA);

                        
    }   

                                                
}


void Draw_Box(cv::Mat &pic,cv::Scalar Color,int width,int height)
{
                
                     int midw= width/2;
                     int midh= height/2;
                
                     cv::rectangle(pic,
                               cv::Point(midw-50,midh-50),
                               cv::Point(midw+50,midh+50),
                               Color,
                               8,
                               0.25,
                               0);
                  
                    cv::circle(pic,
                               cv::Point(midw,midh),
                               1,
                               Color,
                               0.25,
                               8,
                               0);

}






