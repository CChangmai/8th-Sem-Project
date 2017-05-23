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

using namespace std;




/*void Write_Text(std::vector <string> &temp)
{
        
    
    for(int i=0; i<temp.size();i++,SplitImage &pic)
    {
                        //This Function Assumes We Are Sending Write_Text To A Sub-Part Of The Actual Image
                        //Therrefore The Relative Image will have co-ordinate 0,0
                        
                        putText(pic.Image(),temp[i].c_str(), cv::Point(0,0), 
                                            cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 1, CV_AA);

                        
    }   

}
*/
cv::Mat Write_Text(std::vector <string> &temp,cv::Mat &pic,int )
{
    cv::Mat npic=pic;
    int scale_factor= (pic.rows/10);
    int start_pt=scale_factor*temp.size();
    
    for(int i=0; i<temp.size();i++)
    {
                        putText(npic,temp[i].c_str(), cv::Point(0,(pic.rows-start_pt+(i*20))), 
                                            cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 1, CV_AA);

                        
    }   

    return npic;                                             
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
                  
                    cv::circle(frame,
                               cv::Point(midw,midh),
                               1,
                               Color,
                               0.25,
                               8,
                               0);

}


void Blend_Puzzle(cv::Mat &pic,cv::Mat &Puzzle,float transparency)
{
    float opacity=1-transparency;
    addWeighted(Puzzle,opacity,pic,transparency,pic);
}




