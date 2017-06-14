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
    #define HAS_THREADS
    

#include "Headers/Overlay.h"
#include "Headers/IPCAM.h"
#include "Headers/Puzzle.h"
#include "Headers/Calibrate.h"
#include "GPU_Tasks/GPU.h"



#define BSIZE 80
//const int wait_period=100;



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

bool gpulock=false;      //mutex like variable, used when GPU Functions are needed
bool draw_wait=false;    //For Safe GPU Memory Allocation
bool close_threads=false;                 
/*

For Storing Count Threads (Global Variable)

std::future<int> count;
std::promise<int> p_count;


std::future<int> oldb;
std::future<int> newb;
std::promise<int> p_old;
std::promise<int> p_new;
std::mutex m;
*/
void Finger_Track();
void Draw_Everything(cv::Mat&);

/*Assigning A New Window To Be Globally Accessed To All Threads*/




int main(int argc, char* argv[])
{
       bool calibrated;
       
       
       /* ASSIGN THRESHOLD MEMORY MAP TO GPU BEFOREHAND*/
        int *pcount=assign_threshold();
       //int* pcount[5];
                    
       
       std::vector<std::string> message;
       
                         message.push_back(""); //Initialise With Empty String
       
       /*
        Get Puzzle From Hard Drive
        TERMINATE PROGRAM IF PUZZLE IS NOT FOUND
        THE ASSIGN_PUZZLE FUNCTION ASSIGNS 
        THE IMAGE DURING ITS CALL
        
       */ 
       if(Assign_Puzzle(argv[1],BSIZE,BSIZE,640,480,cv::Scalar(0,0,255))==false)
       {
          return 0;
       }
      
       //Get The Feed From IPCAM
       try
       {
       Puzzle_Rearrange();
       IPCAM::startRx(); 
       
       /*
         Make 2 Clones One For Text Overlay And One for Processing
         For Thread Safety:
         Assign Future Data-Types for accesing data only when available
         THEN ALLOCATE BINARY IMAGE FROM GPU CLONES
       */
       
        cv::Mat OvImg;
        
        IPCAM::getRx(OvImg);
        
        cv::Mat ProcImg_Color,ProcImg_Gray;
        /*
        Allocate Memory via GPU and set gpulock to true during this process
        gfilter VARIABLE NAME IS USED
        BECAUSE GPU ALREADY HAS A GLOBAL VARIABLE NAMED gpufilter
        
        */
                gpulock=true;
                
                cv::Mat gpuImg(OvImg.size(),CV_8U,MapImageToCPU());
                cv::Mat gpuImg2(OvImg.size(),CV_8U,MapSubtractor());
                
                cv::Mat gfilter(BSIZE,BSIZE,CV_8U,MapFilter());
                //cv::Mat gpuImg(OvImg.size(),CV_8U,createImageBuffer(OvImg.rows * OvImg.cols));
                gpuImg2.setTo(0);
                
                gpulock=false;
         
                                
        /*
          Assign An Empty Image For Calibration
          Assign Window Earlier Than 
          Declaration Of Threads That Will 
          Access It For Drawing  
          
        */
        cv::Mat prevRect(BSIZE,BSIZE,CV_8U);       
        //prevRect.setTo(cv::Scalar(0));
        
        
        
        cv::namedWindow( "Puzzle" );
        cv::namedWindow( "GPU Image" );
        /* 
           GET THE FINGERS CALIBRATED FROM THE DATA FOR FIVE FINGERS 
           PLUS MAKE THE REQUIRED FILTERS BEFORE INITIALISATION
           ENSURE THE PREVIOUS CALIBRATION IS FULLY WHITE
           TO ENSURE ITS FULLY NOISY
           
        */
        make_Filter();
        make_Filter(gfilter);
          
        prevRect.setTo(0);
        
        for(int i=0;i<5;i++)
        {
                 
                 calibrated=false;
                 std::ostringstream temp;
                 temp<<"Enter Finger Number : "<<i+1;
                 message[0]=temp.str();
                
              
                                  
                 while(calibrated==false)
                 {
                 
                 IPCAM::getRx(OvImg);
                 Write_Text(message,OvImg);
                 /*
                    Keep On Grabbing Images Till The Image is Non-Noisy and good
                    Calibrate Variable Keeps Track of success or failure
                 */
                    
                   /*
                   THIS IS THE CULPRIT THAT RUINED MY LIFE
                   ASSERTION FAILED ERRORS SHOW HERE
                   */
                    int j = Calibrate_Finger(OvImg,
                                            prevRect,
                                            OvImg.rows,
                                            OvImg.cols,
                                            BSIZE,
                                            BSIZE);
                    
                                          
                    if(j>0)
                    {
                        pcount[i]=j;
                        calibrated=true;
                        Draw_Box(OvImg,cv::Scalar(0,255,0),640,480);
                        cv::imshow("Puzzle", OvImg);
                        IPCAM::RefreshRx(2.0);
                        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                        cv::waitKey(1);       
                        
                        /* WHEN WE FIND A MATCH WE DO TWO THINGS
                           FLUSH OUT THE PREVIOUSLY MATCHED 
                           FINGER TEMPLATE
                           ERASE AND MAKE IT ZERO
                           THEN WE REFRESH THE FEED SO THAT IT 
                           SKIPS THE FRAMES THAT IT SAVED
                           SO THAT USER CAN CHANGE FINGERS
                           THIS MINIMISES RISK OF FALSE MATCHING WITH
                           PREVIOUS FINGER
                           
                           WE MAKE THE MAIN FUNCTION SLEEP FOR TWO SECONDS FOR 
                           THE PERSON TO CHANGE HIS FINGERS
                        */   
                           
                        prevRect.setTo(0);
                        
                        /*
                        SKIP FRAMES FOR 2 SECONDS
                        FUNCTION ACCEPTS FLOATING POINT 
                        VALUES
                        */
                        
                        
                    }
                    else
                    {
                         
                         
                         IPCAM::getRx(OvImg);
                         
                         Draw_Box(OvImg,cv::Scalar(0,0,255),640,480);
                         cv::imshow("Puzzle",OvImg);          
                         cv::waitKey(1);      
                    }
                   
                                     
                 }               
        }
        
        
        
        
        /*
          Initialising The Future Variables before being called
          In The While Loop. This should prevent further race 
          conditions.
        
        newb=p_new.get_future();  
        oldb=p_old.get_future();  
        */ 
        
        IPCAM::getRx(OvImg);
        /* 
            WE RECIEVE A FRAME FROM THE IPCAM FIRST TO DESTROY CALIBRATION FRAMES        
            Create A Thread For Drawing the image after the 30 fps limit has been reached 
            CREATE A THREAD FOR FINDING THE LOCATION OF THE FINGER
        
        */
        
        std::thread Draw(Draw_Everything,std::ref(ProcImg_Color));
        std::thread Locate(Finger_Track);
        
        
        Draw.detach();
        Locate.detach();
        /*
           THIS THREAD WILL BE MADE ONLY AFTER 
           
           a) A GPU IMAGE IS ALREADY ASSIGNED
           b) THE PUZZLE IS LOADED IN THE PROGRAM
           c) THE IPCAM FEED IS ALREADY STARTED
           
           THIS THREAD WILL RUN COMPLETELY INDEPENDENT 
           OF THE MAIN THREAD
           AND WILL BE SYNCHRONISED WITH USER-DEFINED 
           MUTEXES I.E BOOLEAN VALUES
        */    
            
        
        do
        {
          
          draw_wait=true;
          
          IPCAM::getRx(OvImg);
          
          ProcImg_Color=OvImg.clone();
          
          draw_wait=false;
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
              
              cv::cvtColor(ProcImg_Color,ProcImg_Gray,cv::COLOR_RGB2GRAY);
              
              //Set THRESHOLDED Image To GPUIMG
              /*cv::adaptiveThreshold(ProcImg_Gray,gpuImg,1,
                                    CV_ADAPTIVE_THRESH_MEAN_C,
                                    CV_THRESH_BINARY,
                                    5,
                                    0);
              */
              cv::threshold(ProcImg_Gray,gpuImg,0, 255, CV_THRESH_BINARY + CV_THRESH_OTSU); 
              
                                      
              //RemoveBackground();
              
              //gpuImg.copyTo(gpuImg2);
              
              cv::imshow("GPU Image",gpuImg);          
              cv::waitKey(1);  
             // std::cout<<"\n I am here \n";
              gpulock=false;
          }
         
          
         
          
          /* First Watch For Any Finger Movement*/
          if (draw_wait==false)
          {
           draw_wait=true;
           
           Blend_Puzzle(OvImg,0.60);
           imshow("Puzzle",OvImg);
           cv::waitKey(1);
           
           draw_wait=false; 
           
          }           
       
        }while( ( IPCAM::status== 1 ) );// && (isPuzzleComplete()==false) );

  }
  catch(...)
  {
      std::cout<<"\n Some Error Occured. \n Have to check Code \n";
      close_threads=true;
  }







   cleanup();
   close_threads=true;

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
               
               IF THE BLOCKS ARE MATCHING THEN WE SHALL 
               MAKE OUR MOVE
               OTHERWISE DO NOTHING
            
            */
            
            int prev_block=-1;
            int drag_block;
            int new_block=-1;
            
            /* RUN IN A LOOP */
            
            while( close_threads == false )
            {
            
                

                if(gpulock==false)
                {
                   gpulock=true;
                   
                   std::cout<<"Block Location : " <<new_block<<std::endl;
                   
                   /* 
                      IF FINGER IS AT DEFAULT VALUE I.E -1
                      THEN ONLY FIND FINGER LOCATION
                      OTHERWISE ASSUME THAT THE FINGER
                      LOCATION IS STORED FROM A PREVIOUS VALUE
                   */   
                  
                    if(prev_block==-1)
                         {
                             prev_block=finger_location();
                             
                         }
                  
                   gpulock=false;            
                   
                   std::this_thread::sleep_for(std::chrono::milliseconds(500));
                   /*
                     AFTER 500ms we AGAIN CHECK IF ITS SAFE 
                     TO ACCESS GPU MEMORY
                   */  
                   if(gpulock==false)
                      {
                            gpulock=true;
                            new_block=finger_location();
                            if(new_block>0)
                            {
                                if(draw_wait==false)
                                {
                                    draw_wait=true;
                                    Draw_Single(new_block,cv::Scalar(255,0,0));
                                    draw_wait=false;
                                
                                }
                            }
                            
                            gpulock=false;                   
                      }
                   
                   if( (prev_block==new_block) && (prev_block>0) )
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
                     
                     if ( (drag==false) && (draw_wait == false) )
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
    }
    
    
 void Draw_Everything(cv::Mat &pic)
 {
     while(close_threads==false)
     {
     std::this_thread::sleep_for(std::chrono::milliseconds(20));
     
     if( draw_wait==false)
     
     {
            draw_wait=true;
               
            //cv::imshow("Puzzle",pic);
            
            
            draw_wait=false;
        }  
     
     }
 }   
        