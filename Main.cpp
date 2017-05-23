#ifndef HAS_IOSTREAM
    #include<iostream>
    #define HAS_IOSTREAM
#endif

    #include<opencv2/core/core.hpp>
    #include<opencv2/highgui/highgui.hpp>
    #include<opencv2/imgproc/imgproc.hpp>
    
    /* To Stop Multiple Instances of Header Files Being Called */
    #define INCLUDE_OPENCV

#include<chrono>
    #include<vector>
    #include<thread>
    #include<future>
    #define HAS_THREADS
    

#include "Overlay/Overlay.h"
#include "Globals/IPCAM.h"
#include "Globals/Puzzle.h"
#include "Globals/Calibrate.h"


const int BSIZE=100;
const int wait_period=100;

/* PARAMETERS For Detection 

       Frame Count :   Number of Frames which is considred to be static frame
       Block-ID    :   The Value Returned By The GPU After Processing The Data
       Wait_Time   :   The Value Of Time Considred For The Hand to be considered
                       stationary
       Prev_Block  :   Image Of Previous Block to be Subtacted
       Timer       :   To reset The Count For A Certain Threshold  
       Filter      :   The Custom Filtering Element Produced     
       
*/

#define FPS 30

using namespace std;


typedef std::chrono::high_resolution_clock Timer;
bool drag=false; //For Indicating Drag And Drop
bool lock=false;
//For Storing Count Threads (Global Variable
std::future<int> count;
std::promise<int> p_count;


std::future<int> oldb;
std::future<int> newb;
std::promise<int> p_old;
std::promise<int> p_new;
mutex m;

void Timer_Track();
int Find_Fingers(cv::Mat);
void Draw_Everything(cv::Mat);

/*Assigning A New Window To Be Globally Accessed To All Threads*/
cv::namedWindow("Puzzle Application",cv::CV_WINDOW_AUTOSIZE);   




int main(int argc, char* argv[])
{
       bool calibrated=true;
       
       
       int *pcount=(int*)MapVariable(5*sizeof(int));
       int *block=(int*)MapVariable(sizeof(int));
       std::vector<std::string> message;
       
                         message.push_back(""); //Initialise With Empty String
       
       //Get Puzzle From Hard Drive
       Assign_Puzzle(argv[1],BSIZE,BSIZE,640,480);
       
       //Get The Feed From IPCAM
       IPCAM::startrx(); 
       
       IPCAM::getRx();
       
       
       /*
         Make 2 Clones One For Text Overlay And One for Processing
         For Thread Safety:
         Assign Future Data-Types for accesing data only when available
         
       */
       
        cv::Mat ProcImg,OvImg=IPCAM::IP_Image.clone();
        
        /*Assign An Empty Image For Calibration*/
        cv::Mat prevRect(BSIZE,BSIZE,cv::CV_8UC1);       
        prevRect.setTo(cv::Scalar(0));
        
        for(i=0;i<5;i++)
        {
                 
                 calibrate=false;
                 std::ostringstream temp;
                 temp<<"Enter Finger Number : "<<i+1;
                 message[0]=temp.str();
                 IPCAM::getRx();
                 
                 while(calibrate==false)
                 {
                 /*Keep On Grabbing Images Till The Image is Non-Noisy and good*/
                    
                    
                    int j= Calibrate_Finger(OvImg,
                                            prevRect,
                                            OvImg.rows,
                                            OvImg.cols);
                    if(j<10)
                    {
                        pcount[i]=j;
                        calibrate=true;
                    }
                    else
                    {
                         IPCAM::getRx();
                         OvImg=IPCAM::IP_Image.clone();                   
                    }                    
                 }               
        }
        
        
        OvImg.release();
        
        /*
          Initialising The Future Variables before being called
          In The While Loop. This should prevent further race 
          conditions.
         */ 
        newb=p_new.get_future();  
        oldb=p_old.get_future();  
        
        
        /* Create A Thread For Drawing the image anyway*/
         
       
        while(IPCAM::status==1)
        {
          
                 if(lock==false)
                  {
                    lock=true;      
                    IPCAM::getRx();
                    lock=false;
                  }
          ProcImg=IPCAM::IP_Image.clone();
          OvImg=IPCAM::IP_Image.clone();
          
          std::future<int> finger=std::async(std::launch::async,
                                             Find_Fingers,
                                             std::ref(ProcImg));
          
          
          Draw_Everything(OvImg);
                     
       
        }












   return 0;
}

void Timer_Track()
        {
            std::this_thread::sleep_for(chrono::milliseconds(100));
            p_count.set(0);
        }     
void Draw_Everything(cv::Mat &pic)
        {
            
        
        }
void Draw_Plain()
{



}            