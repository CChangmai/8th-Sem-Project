#ifndef HAS_IOSTREAM
    #include<iostream>
    #define HAS_IOSTREAM
#endif

    /* To Stop Multiple Instances of Header Files Being Called */
    #define HAS_OPENCV
    #include<opencv2/core/core.hpp>
    #include<opencv2/highgui/highgui.hpp>
    #include<opencv2/imgproc/imgproc.hpp>
    


    #include<chrono>
    #include<vector>
    #include<thread>
    #include<future>
    #define HAS_THREADS
    
#include "GPU_Tasks/GPU_Funcs.h"
#include "Headers/Overlay.h"
#include "Headers/IPCAM.h"
#include "Headers/Puzzle.h"
#include "Headers/Calibrate.h"




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

typedef std::chrono::high_resolution_clock Timer;
bool drag=false; //For Indicating Drag And Drop

bool gpulock=false;      //mutex like variable, used when needed
bool draw_wait=false;    //For Safe GPU Memory Allocation
                 
//For Storing Count Threads (Global Variable
std::future<int> count;
std::promise<int> p_count;


std::future<int> oldb;
std::future<int> newb;
std::promise<int> p_old;
std::promise<int> p_new;
mutex m;

void Finger_Track();
int  Find_Fingers(cv::Mat);
void Draw_Everything(cv::Mat);

/*Assigning A New Window To Be Globally Accessed To All Threads*/
cv::namedWindow Output("Puzzle Application",cv::CV_WINDOW_AUTOSIZE);   




int main(int argc, char* argv[])
{
       bool calibrated=true;
       
       
       /* ASSIGN THRESHOLD MEMORY MAP TO GPU BEFOREHAND*/
       int *pcount=assign_threshold();
       
              
       
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
         THEN ALLOCATE BINARY IMAGE FROM GPU CLONES
       */
       
        cv::Mat ProcImg(640,480,CV_8UC1),OvImg=IPCAM::IP_Image.clone();
        
        /*
        Allocate Memory via GPU and set gpulock to true during this process
        */
                gpulock=true
                cv::Mat gpuImg(cv::Size(640,480),CV_8UC1,MapImageToCPU())
                gpulock=false;
                
        /*Assign An Empty Image For Calibration*/
        cv::Mat prevRect(BSIZE,BSIZE,cv::CV_8UC1);       
        prevRect.setTo(cv::Scalar(0));
        
        
        /* GET THE FINGERS CALIBRATED FROM THE DATA FOR FIVE FINGERS */
        
        for(i=0;i<5;i++)
        {
                 
                 calibrate=false;
                 std::ostringstream temp;
                 temp<<"Enter Finger Number : "<<i+1;
                 message[0]=temp.str();
                 IPCAM::getRx();
                 
                 while(calibrate==false)
                 {
                 /*
                    Keep On Grabbing Images Till The Image is Non-Noisy and good
                    Calibrate Variable Keeps Track of success or failure
                 */
                    
                    
                    int j= Calibrate_Finger(OvImg,
                                            prevRect,
                                            OvImg.rows,
                                            OvImg.cols);
                    if(j>0)
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
        
        
        
        
        /*
          Initialising The Future Variables before being called
          In The While Loop. This should prevent further race 
          conditions.
         */ 
        newb=p_new.get_future();  
        oldb=p_old.get_future();  
        
        
        /* Create A Thread For Drawing the image anyway*/
        
        
        
        
        
        
        /*
           For By-Passing mutex concept 
           
           std::future<int> finger=std::async(std::launch::async,
                                             Find_Finger,
                                             std::ref(ProcImg)); 
        */
        while(IPCAM::status==1)
        {
          
          IPCAM::getRx();
          
          /*
          Create two Images
          ProcImg - Image Copy For Processing The Algorithm
          OvImg - For Drawing Overlay To the User
          
          CHECK IF GPU EVALUATION IS ALREADY PERFORMED
          
          IF STATUS IS TRUE THEN GPU IS STILL EVALUATING
          IF FALSE THEN LOAD THE NEW GPU IMAGE
          
          */
          
          if (gpulock==false)
          {
              gpulock=true;
              cv::cvtColor(IPCAM::IP_Image,ProcImg,CV_COLOR_RGB2GRAY);
              
              /*Set THRESHOLDED Image To GPUIMG*/
              cv::adaptiveThreshold(ProcImg,gpuImg,1,
                                    CV_ADAPTIVE_THRESH_MEAN_C,
                                    CV_THRESH_BINARY,
                                    5,
                                    0);
    
              gpulock=false;
          }
          OvImg=IPCAM::IP_Image.clone();
          
         
          
          /* First Watch For Any Finger Movement*/
          if (draw_wait==false)
          {
           Blend_Puzzle(OvImg);
           imshow("Puzzle Application",OvImg);
          }           
       
        }












   return 0;
}

void Finger_Track()
        {
        
            
            /* STEPS I'M PRESUMING
               
               STEP I   -   FIND FINGER STATIC FOR SOME MILLISECONDS (500 ms)
               STEP II  -   IF FINGER STATIC, RECORD THE BLOCK
                            SET DRAG BOOLEAN TO TRUE
               STEP III -   WAIT FOR NEXT STATIC FINGER DETECTION
               STEP IV  -   SET BOOLEAN TO FALSE , INDICATING DRAG COMPLETE
               STEP V   -   SWAP THE PUZZLE PIECES ACCORDINGLY
               
               STEP VI  -   RETURN TO DRAWING PUZZLE PIECES
               
               WAIT FOR 500ms FOR THE FIRST TRACE 
               IF TRUE THEN SET BLOCK POSITION AND DRAG FLAG
               THEN CHECK
               
               TWO VARIABLES ARE PRESENT
               PREV_BLOCK
               NEW_BLOCK
            
            */
            
            int prev_block=0;
            int drag_block;
            int new_block=0;
            while(1)
            {
                if(gpulock==false)
                {
                   gpulock=true;
                  
                     if(prev_block==0)
                         {
                             prev_block=finger_location();
                         }
                  
                   gpulock=false;            
                   
                   std::this_thread::sleep_for(chrono::milliseconds(500));
                   
                   /*
                     AFTER 500ms we AGAIN CHECK IF ITS SAFE 
                     TO ACCESS GPU MEMORY
                   */  
                   if(gpulock==false)
                   {
                    gpulock=true;
                    new_block=finger_location();
                    gpulock=false;                   
                   }
                   
                   if(prev_block==new_block)
                   {
                     /*
                        COMPLEMENT THE PREVIOUS DRAG STATE
                        IF DRAG WAS TRUE EARLIER THEN COMPLEMENT OF IT 
                        IS FALSE
                        THIS MEANS THAT IT WAS PREVIOUSLY IN DRAGGED STATE
                        AND THE USER HAS STOPPED MOVING JUST NOW
                        SO SWAP THE TILES
                        
                        IF THE PREVIOUS STATE WAS FALSE
                        A NEW DRAG OPERATION HAS STARTED   
                        RECORD THE POSITION FROM WHERE 
                        DRAG HAS STARTED
                      */                   
                     drag=!drag;
                     
                     if(drag==false)
                     {
                       draw_wait=true;
                       Swap_Pieces(drag_block,new_block,BSIZE);
                       draw_wait=false;
                     }
                     else
                     {
                        drag_block=new_block;
                        
                     
                     }                
                
                   }
                 }
        }     

        
