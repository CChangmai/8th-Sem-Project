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

cv::Mat element = getStructuringElement( cv::MORPH_ELLIPSE,
                                         cv::Size( 5, 5 ),
                                         cv::Point( 2, 2 ) );
const int threshold=(int)(0.10*(BSIZE*BSIZE));//60% of space 


/*FOR HISTOGRAM*/

unsigned char prev_Cr=0,new_Cr=0,new_Cb=0,prev_Cb=0;
int* max_YCrCb = new int[3];
float Cr_ratio,Cb_ratio;

int* histogram=new int[256];

bool inRange(unsigned char value,unsigned char center,unsigned char deviation)
{
        
return ( ( value > ( center-deviation ) ) && (value < (center + deviation) ) ) ;


}

bool inRange(int val,int c,int dev)
{
        
return ( ( val > ( c-dev ) ) && (val < (c + dev) ) ) ;


}


/*
I'm Making two overloads to this function
One That Makes A Normal Filter 
And One That Makes A Copy for the GPU
*/
unsigned char max_histogram(cv::Mat &img,int ch)
{

      unsigned char position;
      int i,j,max=0;
      
      for(i=0;i<256;i++)
      {
              histogram[i]= 0;   
      }
           
                
      for(i=0;i<BSIZE;i++)
      {
            for(j=0;j<BSIZE;j++)
            {
              unsigned char val = img.at<uchar>(i,j);
              histogram[val]= histogram[val] + 1;   
            }
      }
     
     
     for(i=0;i<256;i++)
     {
         if( histogram[i] > max)
         { 
            max = histogram[i]; 
            position = (unsigned char)i;
         }
     
     
     }

    
  max_YCrCb[ch-1] = histogram[position];

return position;

}
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
        
        filter.at<uchar>( (BSIZE - (0.25 * BSIZE) - j ) , ( BSIZE / 2 ) - i + 15 )   =  255;    
        
        //NEGATIVE HALF + OFFSET
        
        filter.at<uchar>( (BSIZE - (0.25 * BSIZE) - j ) , ( BSIZE / 2 ) + i - 15 ) =     255;
  
      }
      
      
             
        
      dilate(filter,filter,element);  
      //-----------------------------FOR DEBUGGING PURPOSES------------------
      cv::namedWindow("Filter");
      cv::namedWindow("Trace");
      cv::namedWindow("Thresholding");
      cv::namedWindow("Crossmatch");
      cv::imshow("Filter",filter);
      cv::waitKey(0);
      cv::destroyWindow("Filter");
    

}

void make_Filter(cv::Mat &pic)
{
    int i,j;
    pic.setTo(0);

      for(i=0;i<BSIZE/2;i++)
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
        
        pic.at<uchar>( (BSIZE - (0.25 * BSIZE) - j ) , ( BSIZE / 2 ) - i + 15 )   =  255;    
        
        //NEGATIVE HALF + OFFSET
        
        pic.at<uchar>( (BSIZE - (0.25 * BSIZE) - j ) , ( BSIZE / 2 ) + i - 15 ) =     255;
  
      }
      
      dilate(pic,pic,element); 

}



int Calibrate_Finger(cv::Mat &img,cv::Mat &R,cv::Mat &G,cv::Mat &B,cv::Mat &prev,
                     std::vector<cv::Mat> &RGB,
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
    
    
    
    cv::Mat temp_color,temp_img(bwidth,bheight,CV_8U),temp_img1(bwidth,bheight,CV_8U),temp_img2(bwidth,bheight,CV_8U);
    
    /*
      Convert RGB To Gray And use Adaptive Thresholding
      THIS IS THE STEP THAT IS KILLING ME
      SOME ROI ERRORS
    */
    split(img(trect),RGB);
  
    equalizeHist(R,R);
    equalizeHist(G,G);
    merge(RGB,temp_color);
    
    for(int i=0;i<BSIZE;i++)
        {
            for(int j=0;j<BSIZE;j++)
            {
                if ( inRange((int)R.at<uchar>(j,i),200,20) &&
                     inRange((int)G.at<uchar>(j,i),200,20) )
                     {
                        pixel_density++;
                     }
            
            }
        
        
        }
    
    cv::imshow("GPU Image",temp_color);
    equalizeHist(B,B);
    
    merge(RGB,temp_color);
    cv::cvtColor(temp_color,temp_img,cv::COLOR_BGR2GRAY);
    
    
    /*
    
     THE CONSTANT FOR GAUSSIAN MEAN IS FOUND TO BE -5
     WORKS BEST WITH INDOOR LIGHTING CONDITION WITH DECENT AMOUNT OF LIGHT
     
    */
   
    
    /*cv::adaptiveThreshold(temp_img,temp_img1,255,
                          CV_ADAPTIVE_THRESH_MEAN_C,
                          CV_THRESH_BINARY,
                          5,
                          0);
                          
    */
    
    cv::threshold(temp_img,temp_img1,0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
    
   // --------------------------FOR DEBUGGING PURPOSES--------------------------
    
      
   
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
    //pixel_density=cv::countNonZero(temp_img2);
    
    /*Now Apply Edge Detection*/
    Canny(temp_img1,temp_img2,1,1,3);
    
    cv::imshow("Trace",prev);
    cv::imshow("Thresholding",temp_img2);
    cv::waitKey(1);
    cv::bitwise_and(temp_img2,filter,temp_img2);
   
   
    cross_match=cv::countNonZero(temp_img2); 
    cv::imshow("Crossmatch",temp_img2);
    cv::waitKey(1);
    /*
      I'm assuming 100+ points should intersect 
      in the curve if the finger 
      is placed correctly
    
    */
    
    
    if( inRange( pixel_density, threshold , 100 ) && inRange( cross_match , 50 , 5 ) )
    {
    
        pixel_density=cv::countNonZero(temp_img1);

       //std::cout<<"\n Calibration Successful : "<<pixel_density<<"\n";
       return pixel_density;
    }
    else
    {
       std::cout<<"Please make sure Background is Static : "<<pixel_density<<"\nAnd Cross Match Is : "<<cross_match<<"\n";
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
                   /*
                        if( inRange(density,localdensity[k] , 100 )  && inRange(cross_match,60,30) )
                            { 
                    
                             b=count; 
                    
                            }
                    */
                   } 
                   
                   count++; 
                
                  }
            
             }
            
            
     return b;       
            
}


bool calibrate_YCrCb(cv::Mat &Y,cv::Mat &Cr, cv::Mat &Cb)
{
    
  unsigned char lumos = max_histogram(Y,3);
  new_Cr = max_histogram(Cr,1);
  new_Cb = max_histogram(Cb,2);
  
  float offset=0.0;
  
  cv::Mat temp(BSIZE,BSIZE,CV_8U);
  int boundaries;
  
  
  Laplacian(Y,temp,CV_8U);
  
  boundaries=cv::countNonZero(temp);
  
  cv::imshow("Trace",temp);
  
  bitwise_and(temp,filter,temp);
  
  int cross_match=cv::countNonZero(temp);
  
  cv::imshow("Thresholding",Y);
  cv::imshow("GPU Image" , temp);
   
  int deviation = boundaries - cross_match;
  
  
    int devCr = new_Cr-prev_Cr;
    int devCb = new_Cb-prev_Cb;
    
    /*
        ASSIGN AN ABSOLUTE RANGE FOR SUCH CHANGES
        IF EITHER Cr OR Cb VARIES TOO MUCH
        THE IMAGE IS VERY NOISY
        
        IF IMAGE IS STATIC THEN
        CHECK WHETHER THE VALUE IS 
        WITHIN VALID SKIN RANGE
    */
    std::cout<<"\n"<<devCr<<":"<<devCb<<":"<<cross_match<<"\n";   
    std::cout<<"\n New Cr : "<<(int)new_Cr<<" New Cb : "<<(int)new_Cb<<" : "<<" New Y : "<<(int)lumos<<"\n";  
    std::cout<<"\n Max Cr : "<<max_YCrCb[0]<<" Max Cb : "<<max_YCrCb[1]<<" : "<<" Max Y : "<<max_YCrCb[2]<<"\n";  
    
    if( (devCr==0) || (devCb==0) )
     {
     
        /* CHECK IF ITS A FINGER */
        if(inRange(new_Cr,153U,19U) && inRange(new_Cb,107U,25U) && 
           inRange(cross_match,230,10) && (deviation < 2100 ))
        {
        
            Cr_ratio = ((float)new_Cr/ (float)lumos);
            Cb_ratio = ((float)new_Cb/ (float)lumos);
            
            offset = Cb_ratio - (int)Cb_ratio;          
            
            Cb_ratio =  ( (int)Cb_ratio * 1000 ) + offset + (float)lumos;
                            
                     
            std::cout<<"Cr Ratio : "<<Cr_ratio<<" Cb Ratio : "<<Cb_ratio<<"\n";
            
            return true;        
        
        }
        
    
     
     }  
     
  
  
  prev_Cr=new_Cr;
  prev_Cb=new_Cb;


return false;


}

