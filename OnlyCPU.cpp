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
    

#include "Headers/Overlay.h"
#include "Headers/IPCAM.h"
#include "Headers/Puzzle.h"
#include "Headers/Calibrate.h"

#ifndef HAS_SIZE
    #define WIDTH 640
    #define HEIGHT 480
    #define SIZE 48
    #define BSIZE 80
    #define HAS_SIZE
    #define wait_period 100
#endif


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

bool draw_wait=false; 
bool proc_wait=false;   //For Safe GPU Memory Allocation
bool close_threads=false;
int  pcount[5];

void Finger_Track(cv::Mat&);
void Draw_Everything();

/*Assigning A New Window To Be Globally Accessed To All Threads*/




int main(int argc, char* argv[])
{
       bool calibrated=false;
       
       
       /* ASSIGN THRESHOLD MEMORY MAP TO GPU BEFOREHAND*/
       
       
              
       
       std::vector<std::string> message;
       
       message.push_back(""); //Initialise With Empty String
       
       //Get Puzzle From Hard Drive
       if(Assign_Puzzle(argv[1],BSIZE,BSIZE,WIDTH,HEIGHT,cv::Scalar(0,0,255)) == false)
       {
            
          return -1;
       
       }
       
      
       //Get The Feed From IPCAM
       try
       {
       Puzzle_Rearrange();
     
       IPCAM::startRx(); 
       cv::Mat OvImg(HEIGHT,WIDTH,CV_8U);
             
       IPCAM::getRx(OvImg);
       
       
       /*
         Make 2 Clones One For Text Overlay And One for Processing
         For Thread Safety:
         Assign Future Data-Types for accesing data only when available
         THEN ALLOCATE BINARY IMAGE FROM GPU CLONES
       */
       
        cv::Mat ProcImg,ImgBW(HEIGHT,WIDTH,CV_8UC1);
        /*
        Allocate Memory via GPU and set gpulock to true during this process
        */
     
                
        /*Assign An Empty Image For Calibration*/
        cv::Mat prevRect(BSIZE,BSIZE,CV_8U);       
        prevRect.setTo(0);
         
        make_Filter();
        
        
        cv::namedWindow("Puzzle Application");
        cv::namedWindow("Threshold");
        cv::namedWindow("Fast Data");
        /* GET THE FINGERS CALIBRATED FROM THE DATA FOR FIVE FINGERS */
        
        for(int i=0;i<5;i++)
        {
                 
                 calibrated=false;
                 std::ostringstream temp;
                 temp<<"Enter Finger Number : "<<i+1;
                 message[0]=temp.str();
               
                 IPCAM::getRx(OvImg);
                 
                               
                 while(calibrated==false)
                 {
                
                 /*
                    Keep On Grabbing Images Till The Image is Non-Noisy and good
                    Calibrate Variable Keeps Track of success or failure
                 */
                    IPCAM::getRx(OvImg);
                    Write_Text(message,OvImg);
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
                        
                        message[0]="Finger Calibrated";
                        Write_Text(message,OvImg);
                        Draw_Box(OvImg,cv::Scalar(0,255,0),640,480);
                        cv::imshow("Puzzle Application", OvImg);
                        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                        IPCAM::RefreshRx(2.0);
                        cv::waitKey(1);   
                         
                    }
                    else
                    {
                         IPCAM::getRx(OvImg);
                         Write_Text(message,OvImg);
                         Draw_Box(OvImg,cv::Scalar(0,0,255),WIDTH,HEIGHT);
                         cv::imshow("Puzzle Application",OvImg);  
                         cv::waitKey(1);              
                    }
                   
                                     
                 }               
        }
        
        
        cv::destroyWindow("Crossmatch");
        
       //std::thread Draw(Draw_Everything);
       std::thread Locate(Finger_Track,std::ref(ImgBW));
        
        
       //Draw.detach();
       Locate.detach();
        
        
        
        while(IPCAM::status==1)
        {
          
          
          
          IPCAM::getRx(OvImg);
          
                   
          if (draw_wait==false)
          {
              draw_wait=true;
              cv::cvtColor(OvImg,ProcImg,cv::COLOR_RGB2GRAY);
              cv::threshold(ProcImg,ImgBW,0, 255, CV_THRESH_BINARY_INV + CV_THRESH_OTSU); 
              cv::imshow("Threshold",ImgBW);
              cv::waitKey(1);
              draw_wait=false;
        
          }
          IPCAM::getRx(OvImg);
          /* First Watch For Any Finger Movement*/
          if (draw_wait==false)
          {
           draw_wait=true;
           Blend_Puzzle(OvImg,0.60);
           imshow("Puzzle Application",OvImg);
           cv::waitKey(1);
           draw_wait=false;
          }           
       
        }

  }
  catch(...)
  {
      close_threads=true;
  }








    close_threads=true;

   return 0;
}

void Finger_Track(cv::Mat &img)
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
            
            int prev_block=-1;
            int drag_block;
            int new_block=-1;
            
          
            while(close_threads == false)
            {
            
              
              try
              { 
                if(draw_wait==false)
                {
                               
                   prev_block=Find_BlockCPU(img);
                 
                  
                             
                   
                   std::this_thread::sleep_for(std::chrono::milliseconds(500));
                   
                   /*
                     AFTER 500ms we AGAIN CHECK IF ITS SAFE 
                     TO ACCESS GPU MEMORY
                   */  
                   new_block=Find_BlockCPU(img);
                   
                   std::cout<<"Location : "<<new_block<<"\n";
                   
                   if ( (prev_block==new_block) && (prev_block>0) )
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
                     
                     if ( (drag==false) && (drag_block != new_block) ) 
                     {
                       while( draw_wait == true )
                       {}
                       draw_wait=true;
                       Swap_Pieces(drag_block,new_block,BSIZE);
                       draw_wait=false;
                       
                       
                       
                     }
                     else if ( (drag==false) && ( drag_block=new_block ) ) 
                     {
                       while( draw_wait == true )
                       {}
                       draw_wait=true;
                       Draw_Single(new_block,cv::Scalar(0,0,255));
                       draw_wait=false;
                     }        
                     else
                     {
                        while( draw_wait == true )
                        {}
                        draw_wait=true;
                        drag_block=new_block;
                        Draw_Single(new_block,cv::Scalar(255,0,0));
                        draw_wait=false;
                     
                     }        
                
                   }
                 }
                 
                 
             }     
        
        catch(...)
        {
            draw_wait=false;
        
        }     
        
   }
    
}

void Draw_Everything()
{
    cv::Mat ThreadDraw;
    while(close_threads == false)
    {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    IPCAM::getRx(ThreadDraw);
    Blend_Puzzle(ThreadDraw,0.60);
    imshow("Fast Data",ThreadDraw);
    cv::waitKey(1);
    }
    

}
        
