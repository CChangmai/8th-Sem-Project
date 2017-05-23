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

#include<math.h>

cv::Mat filter(100,100,CV_8UC1);
const int threshold=(int)(0.6*10000);//60% of space 

void make_Filter()
{
    int i,j;


      for(i=0;i<100;i++)
      {
        /*Since 0,0 lies on the top left corner
          We want 0,0 from the bottom left corner
          So we have to subtract the coordinate from 100 to get the actual distance
          
          The Other Arc Is Found When we move to a negative x-axis
          So we substitute that value for x + offset, which is 100
        */
        
        j=100-(int)sqrt(10000-(i*i));
        
        filter.at<uchar>(i,j)     =     1;    
        filter.at<uchar>(100-i,j) =     1;
  
      }


}

int Calibrate_Finger(cv::Mat &img,cv::Mat &prev,
                             int height,int width,int bheight,int bwidth)
{
    int tempx= (int)((width-bwidth)/2); 
    int tempy=(int)((height-bheight)/2);
    
    int pixel_density=0;
    int cross_match=0;
    
    cv::Rect trect(tempx,tempy,bwidth,bheight);
    cv::Mat temp_img1,temp_img2;
    
    /*
      Convert RGB To Gray And use Adaptive Thresholding
    */
    cv::cvtColor(img(trect),temp_img1,cv::COLOR_RGB2GRAY);
    cv::adaptiveThreshold(temp_img1,temp_img1,1,
                          CV_ADAPTIVE_THRESH_MEAN_C,
                          CV_THRESH_BINARY,
                          5,
                          0);
    
    /*First Apply Background Subtraction 
      Store The Pixel Density
      Then Bit-Wise 'AND' the Images to extract filter
      Then count the threshold
      The Old Image is lost
      
      prev variable passed as Parameter
    */
    
    cv::bitwise_and(temp_img1,prev,temp_img2);
    pixel_density=cv::countNonZero(temp_img2);
    
    /*Now Apply Filter Matching*/
    cv::bitwise_and(temp_img2,filter,temp_img2);
    cross_match=cv::countNonZero(temp_img2); 
    
    /*
      I'm assuming 5 points should intersect 
      in the curve if the finger 
      is placed correctly
    
    */
    
    
    if( (pixel_density<threshold) && (cross_match<5))
    {
    
       std::cout<<"Calibration Successful";
       return pixel_density;
    }
    else
    {
       std::cout<<"Please make sure Background is Static";
       return 0;
    }
}         
