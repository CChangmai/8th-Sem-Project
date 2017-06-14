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

#ifndef HAS_SIZE
    #define WIDTH 640
    #define HEIGHT 480
    #define SIZE 48
    #define BSIZE 80
    #define HAS_SIZE
#endif



cv::Mat filter(BSIZE,BSIZE,CV_8UC1);
const int threshold=(int)(0.25*(BSIZE*BSIZE));//60% of space 
int cpudensity[SIZE];
int localdensity[5];
int c=0;

bool inRange(int value,int center,int deviation)
{
        
return ( ( value > ( center-deviation ) ) && (value < (center + deviation) ) ) ;


}


/*
I'm Making two overloads to this function
One That Makes A Normal Filter 
And One That Makes A Copy for the GPU
*/


void make_Filter()
{
    int i,j;
    filter.setTo(0);

      for(i=0;i<(BSIZE/2);i++)
      {
        /*Since 0,0 lies on the top left corner
          We want 0,0 from the bottom left corner
          So we have to subtract the coordinate from 100 to get the actual distance
          
          The Other Arc Is Found When we move to a negative x-axis
          So we substitute that value by
          x + offset + (0.5 * finger width) 
          which is assumed to be 15 each side
          The OFFSET is half the block size
        */
        
        j=(int)sqrt( ( (BSIZE*BSIZE)/4) - (i*i) );
        
        //POSITIVE HALF + OFFSET
        
        filter.at<uchar>( (BSIZE - j ) , ( BSIZE / 2 ) - i + 15 )   =  255;    
        
        //NEGATIVE HALF + OFFSET
        
        filter.at<uchar>( (BSIZE - j ) , ( BSIZE / 2 ) + i - 15 ) =     255;
  
      }
      /*-----------------------------FOR DEBUGGING PURPOSES------------------
      cv::namedWindow("Filter");
      cv::namedWindow("Trace");
      cv::namedWindow("Thresholding");
      cv::namedWindow("Crossmatch");
      cv::imshow("Filter",filter);
      cv::waitKey(0);
      cv::destroyWindow("Filter");
     */

}

void make_Filter(cv::Mat &pic)
{
    int i,j;
    pic.setTo(0);

      for(i=0;i<BSIZE;i++)
      {
        /*Since 0,0 lies on the top left corner
          We want 0,0 from the bottom left corner
          So we have to subtract the coordinate from 100 to get the actual distance
          
          The Other Arc Is Found When we move or solve the equation of the circle
          to the negative x-axis
          So we substitute that value for x + offset, which is 100
        */
        
        j=(int)sqrt((BSIZE*BSIZE)-(i*i));
        
        pic.at<uchar>(BSIZE-i,BSIZE-j)     =     255;    
        pic.at<uchar>(BSIZE-i,j) =               255;
  
      }


}



int Calibrate_Finger(cv::Mat &img,cv::Mat &prev,
                             int height,int width,int bheight,int bwidth)
{
    if((width<bheight)||(height<bheight))
    {
    return 0;
    }    
    
    int tempx= (int)((width-bwidth)/2); 
    int tempy=(int)((height-bheight)/2);
    
    int pixel_density=0;
    int cross_match=0;
    
    cv::Rect trect(tempx,tempy,bwidth,bheight);
    
    
    
    cv::Mat temp_img(bwidth,bheight,CV_8U),temp_img1(bwidth,bheight,CV_8U),temp_img2(bwidth,bheight,CV_8U);
    
    /*
      Convert RGB To Gray And use Adaptive Thresholding
      THIS IS THE STEP THAT IS KILLING ME
      SOME ROI ERRORS
    */
    cv::cvtColor(img(trect),temp_img,cv::COLOR_BGR2GRAY);
    
    /*
    
     THE CONSTANT FOR GAUSSIAN MEAN I'VE FOUND TO BE -5
     WORKS BEST WITH INDOOR LIGHTING CONDITION WITH DECENT AMOUNT OF LIGHT
     
    */
   
    
    /*cv::adaptiveThreshold(temp_img,temp_img1,255,
                          CV_ADAPTIVE_THRESH_MEAN_C,
                          CV_THRESH_BINARY,
                          5,
                          0);
                          
    */
    
    cv::threshold(temp_img,temp_img1,0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
    /* 
    --------------------------FOR DEBUGGING PURPOSES--------------------------
    
      
    cv::imshow("Trace",prev);
    cv::imshow("Thresholding",temp_img1);
    cv::waitKey(1);
    /*
      First Apply Background Subtraction 
      Store The Pixel Density
      Then Bit-Wise 'AND' the Images to extract filter
      Then count the threshold
      The Old Image is lost
      
      prev variable passed as Parameter
      WHICH WILL BE UPDATED WITH NEW VALUE
      AFTER FILTER MATCHING
    */
    
    
    cv::bitwise_and(temp_img1,prev,temp_img2);
    
    /* STORE PREVIOUS GRAYSCALE IMAGE TO REFER LATER */

    temp_img1.copyTo(prev);
    pixel_density=cv::countNonZero(temp_img2);
    
    /*Now Apply Filter Matching*/
    cv::bitwise_and(temp_img2,filter,temp_img2);
   
    cross_match=cv::countNonZero(temp_img2); 
    cv::imshow("Crossmatch",temp_img2);
    cv::waitKey(1);
    /*
      I'm assuming 100+ points should intersect 
      in the curve if the finger 
      is placed correctly
    
    */
    
    
    if( inRange( pixel_density, threshold , 100 ) && inRange( cross_match , 60 , 10 ) )
    {
    
        localdensity[c]=pixel_density;
        c++;
       //std::cout<<"\n Calibration Successful : "<<pixel_density<<"\n";
       return pixel_density;
    }
    else
    {
       //std::cout<<"Please make sure Background is Static : "<<pixel_density<<"\nAnd Cross Match Is : "<<cross_match<<"\n";
       return 0;
    }
}    

int Find_BlockCPU(cv::Mat &pic)
{

        int i,j,count=0,density=0,cross_match=0,b=-1;
        
        for(i=0;i<pic.rows;i+=BSIZE)
            {
            
                density=0;
                cross_match=0;
                
                for(j=0;j<pic.cols;j+=BSIZE)
                {
                
                   cv::Rect temp(j,i,BSIZE,BSIZE);
                   cv::Mat temp_img = pic(temp).clone(); 
                   density=cv::countNonZero(temp_img);
                   
                   cv::bitwise_and(temp_img,filter,temp_img);
                   cross_match = cv::countNonZero(temp_img);
                   
                   for(int k=0;k<5;k++)
                   {
                   
                        if( inRange(density,localdensity[k] , 100 )  && inRange(cross_match,60,25) )
                            { 
                    
                             b=count; 
                    
                            }
                    
                   } 
                   
                   count++; 
                
                  }
            
             }
            
            
     return b;       
            
}

           





















     
