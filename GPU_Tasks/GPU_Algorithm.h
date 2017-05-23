//#include<cuda>
#ifndef HAS_IOSTREAM
    #ifdef __cplusplus
                #include<iostream>
                #define HAS_IOSTREAM
    #endif    
#endif


#include "../Globals/SplitImage.h"
#ifndef HAS_OPENCV
    #include<opencv2/core/core.hpp>
    #include<opencv2/imgproc/imgproc.hpp>
    #define HAS_OPENCV
#endif    

#ifndef USES_GPU
    #include "opencv2/cudaimgproc.hpp"
    #include "opencv2/stream_accessor.hpp"
    #include "opencv2/cudaarithm.hpp"
    #include "opencv2/devmem.hpp"
    #define USES_GPU
#endif    


int prev_pos[2]={0,0};
int new_pos[2];
/*It's Difficult to Implement Classes in C
So C++ is better
Because Of Public And Private Visibility Scope

The Basic Method We Want To Use To Reduce Load is 
To Convert The RGB Image To HSV Image
Load It Asynchronously into Memory
Manipulate it In the GPU
And Get results From GPU Processing
I Have To Write The Same Algorithm To Run With Or Without GPU
Hopefully I would be able to write my own CUDA Kernel For Memory Management
*/

enum Angles
{
    deg0=1,
    deg45=2,
    deg90=3,
    deg135=4,
    deg180=5,
    deg225=6,
    deg270=7,
    deg315=8,
    deg360=9

}Angle;

__device__int degToCordinates(Angle* a 


__global__void TraceBoundary(PtrSz<char>*,PtrSz<char>*)
{




}

__device__ MoveAround(int* BlockID,int* X,int*Y)
          {
            //Use Recursive Statement


          } 


void GetChange(cv::Mat image,int rows,int cols)
    {
    
        /* I can't transfer variables, but I can Transfer Images Instead */
        cv::cuda::Stream s1; //For RGB to HSV Conversion
        cv::cuda::Stream s2; //For Manipulating H-plane of HSV Image
       
        /*Transfer Image To Page-Locked GPU Memory
        cv::cuda::CudaMem temp_image(cv::Size(cols,rows),CV_8U);
        cv:::Mat.copyTo(image,temp_image);
        
        cv::Mat temp=temp_image;
        */
        cv::cuda::GpuMat irgb;
        CV_ASSERT(irgb.channels==1);
        
        /*Mouse Cursor Pointers*/
        cv::Mat Point(2,1,CV_16U);       
        cv::cuda::GpuMat NewPt(2,1,CV_16U);   
        
        irgb.upload(image,s1); //Non-Blocking Call, Can Run CPU Processing
        
        
        cuda::cvtColor(irgb,ihsv,COLOR_RGB2HSV);
        
    


    }   

int Mat2Point(cv::Mat &temp)
{




}




 



