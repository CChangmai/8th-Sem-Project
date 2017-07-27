#ifndef HAS_IOSTREAM
    #include<iostream>
    #define HAS_IOSTREAM
#endif

    /* To Stop Multiple Instances of Header Files Being Called */
    #define HAS_OPENCV 
    #include<opencv2/core/core.hpp>
    #include<opencv2/highgui/highgui.hpp>
    #include<opencv2/imgproc/imgproc.hpp>
    #include "opencv2/videoio.hpp"


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
#define WIDTH 640
#define HEIGHT 480
#define HAS_SIZE
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

typedef std::chrono::steady_clock Timer;
typedef std::chrono::steady_clock::time_point TIME;
bool drag=false; //For Indicating Drag And Drop

bool gpulock=false;      //mutex like variable, used when GPU Functions are needed
bool draw_wait=false;    //For Safe GPU Memory Allocation
bool close_threads=false;                 
/*

For Storing Count Threads (Global Variable)

*/
void Finger_Track();
void Draw_Everything(cv::Mat&);

const cv::Rect sector((WIDTH-BSIZE)/2,(HEIGHT-BSIZE)/2,BSIZE,BSIZE);

cv::Mat element2 = getStructuringElement(cv::MORPH_RECT,
                                         cv::Size( 3, 3 ),
                                         cv::Point( 1 , 1 ) );
/*Assigning A New Window To Be Globally Accessed To All Threads*/




int main(int argc, char* argv[])
{
       bool calibrated;
       
       
       /* ASSIGN THRESHOLD MEMORY MAP TO GPU BEFOREHAND*/
        int *pcount=assign_threshold();
       
                    
      
       
       std::vector<std::string> message;
       message.push_back(""); //Initialise With Empty String
       message.push_back("Please try to Maintain");
       message.push_back("A Plain And Static Background"); 
      
   
       
       
       /*
        Get Puzzle From Hard Drive
        TERMINATE PROGRAM IF PUZZLE IS NOT FOUND
        THE ASSIGN_PUZZLE FUNCTION ASSIGNS 
        THE IMAGE DURING ITS CALL
        
       */ 
       if(Assign_Puzzle(argv[1],BSIZE,BSIZE,640,480,cv::Scalar(0,0,255)) == false)
       {
          return 0;
       }

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
      /*  
       cv::VideoWriter cpu,gpu;
       
       int codec = CV_FOURCC('M','J','P','G');
       int codec2 = CV_FOURCC('M','P','4','2');
       
       cpu.open("cpu.avi",codec,FPS,OvImg.size(),CV_8UC3);
       gpu.open("gpu.avi",codec2,FPS,OvImg.size(),CV_8UC1);
        
        cv::Mat ProcImg_Color,ProcImg_Gray,ProcImg_BW;
       */ 
        /*
        Allocate Memory via GPU and set gpulock to true during this process
        gfilter VARIABLE NAME IS USED
        BECAUSE GPU ALREADY HAS A GLOBAL VARIABLE NAMED gpufilter
        
        */
                gpulock=true;
                
                cv::Mat gpuImg(  OvImg.size(), CV_8UC1, (void*)MapImageToCPU(1)),
                        gpuImg2( OvImg.size(), CV_8UC1, (void*)MapImageToCPU(2)),
                        R_Plane( OvImg.size(), CV_8UC1, (void*)MapImageToCPU(3)),
                        G_Plane( OvImg.size(), CV_8UC1, (void*)MapImageToCPU(4)),
                        B_Plane( OvImg.size(), CV_8UC1, (void*)MapImageToCPU(5)),
                        Y_Plane( OvImg.size(), CV_8UC1, (void*)MapImageToCPU(6)),
                        Cr_Plane(OvImg.size(), CV_8UC1, (void*)MapImageToCPU(7)),
                        Cb_Plane(OvImg.size(), CV_8UC1, (void*)MapImageToCPU(8));
                        
               std::vector<cv::Mat> Channels {B_Plane,G_Plane,R_Plane};
                
               
               cv::Mat Box_R(BSIZE,BSIZE, CV_8UC1, (void*)MapCalibrator(1)),
                       Box_G(BSIZE,BSIZE, CV_8UC1, (void*)MapCalibrator(2)),
                       Box_B(BSIZE,BSIZE, CV_8UC1, (void*)MapCalibrator(3)),
                       Box_Y(BSIZE,BSIZE, CV_8UC1, (void*)MapCalibrator(4)),
                       Box_Cr(BSIZE,BSIZE,CV_8UC1, (void*)MapCalibrator(5)),
                       Box_Cb(BSIZE,BSIZE,CV_8UC1, (void*)MapCalibrator(6));

               std::vector<cv::Mat> Box_RGB {Box_B,Box_G,Box_R};
                    
                //ALLOCATE AND CREATE GPU IMAGE IN MEMORY    
               
                cv::Mat gfilter(BSIZE,BSIZE,CV_8UC1,(void*)MapFilter());
                make_Filter(gfilter);
                             
                           
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
        cv::namedWindow( "GPU Filter");
        
        cv::imshow("GPU Filter",gfilter);
        
        /* 
           GET THE FINGERS CALIBRATED FROM THE DATA FOR FIVE FINGERS 
           PLUS MAKE THE REQUIRED FILTERS BEFORE INITIALISATION
           ENSURE THE PREVIOUS CALIBRATION IS FULLY WHITE
           TO ENSURE ITS FULLY NOISY
           
        */
        make_Filter();
       
               
        float* CrCb_ratios = new float[10];
        unsigned char* CrCb_Values = new unsigned char[10];
          
        prevRect.setTo(0);
        
        for(int i=0,j=0,k=1;i<5;i++,j=2*i,k=(2*i)+1)
        {
                 
                 calibrated=false;
                 std::ostringstream temp;
                 temp<<"Enter Finger Number : "<<i+1;
                 message[0]=temp.str();
                 
                 IPCAM::RefreshRx(5);
                                  
                 while(calibrated==false)
                 {
                 
                 IPCAM::getRx(OvImg);
                 
                 split(OvImg(sector),Box_RGB);
                 
                 ConvertBox();
                 /*
                    Keep On Grabbing Images Till The Image is Non-Noisy and good
                    Calibrate Variable Keeps Track of success or failure
                 */
                    
               
                     
                     bool t = calibrate_YCrCb(Box_Y,Box_Cr,Box_Cb);
                     
                    
                     
                     
                             
                        
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
                    
                        
                   if (t == true)
                    {
                        TIME start= Timer::now();
                        
                        pcount[i] = max_YCrCb[2];
                        
                        
                        CrCb_Values[j] = new_Cr;
                        CrCb_Values[k] = new_Cb;
                        
                        CrCb_ratios[j] = Cr_ratio;
                        CrCb_ratios[k] = Cb_ratio;
                    
                        Draw_Box(OvImg,cv::Scalar(0,255,0),640,480);
                        
                        cv::imshow("Puzzle", OvImg);
                        
                                             
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                       
                       
                        TIME stop = Timer::now();
                        
                        
                        
                        //IPCAM::RefreshRx(std::chrono::duration_cast<std::chrono::seconds>(stop - start).count());
                        
                        calibrated = true;
                         
                        
                    }
                    
                    else
                    {
                         
                         
                         Write_Text(message,OvImg);
                         Draw_Box(OvImg,cv::Scalar(0,0,255),640,480);
                         cv::imshow("Puzzle",OvImg);          
                         //cpu.write(OvImg);
                         cv::waitKey(1);      
                    }
                   
                                     
                 }               
        }
        
        TransferValues(CrCb_Values,pcount,CrCb_ratios);
       
        /* 
            WE RECIEVE A FRAME FROM THE IPCAM FIRST TO DESTROY CALIBRATION FRAMES        
            Create A Thread For Drawing the image after the 30 fps limit has been reached 
            CREATE A THREAD FOR FINDING THE LOCATION OF THE FINGER
        
        */
        
        //std::thread Draw(Draw_Everything,std::ref(ProcImg_Color));
        /*THIS PART WAS MAKING ERROR BECAUSE THERE ARE NO YCRCB VALUES
          MADE BEFORE DETACHING THREAD
          SO ERRONOUS RESULTS OCCUR
        */  
        
        IPCAM::getRx(OvImg);
        split(OvImg,Channels);
        
        Y_Plane.setTo(0);
        Cr_Plane.setTo(0);
        Cb_Plane.setTo(0);
        
        std::thread Locate(Finger_Track);
        
                                
        
        
        //Draw.detach();
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
          
          split(OvImg,Channels);
         
          draw_wait=false;
          /*
          Creatxe two Images
          ProcImg - Image Copy For Processing The Algorithm
          OvImg - For Drawing Overlay To the User
          
          CHECK IF GPU EVALUATION IS ALREADY PERFORMED
          
          IF STATUS IS TRUE THEN GPU IS STILL EVALUATING
          IF FALSE THEN LOAD THE NEW GPU IMAGE
          
          */
          
          
          if(gpulock==false)
          {
              gpulock=true;
              
              
              /*
              equalizeHist(R_Plane,R_Plane);
              equalizeHist(G_Plane,G_Plane);
              equalizeHist(B_Plane,B_Plane);
              
              merge(Channels,ProcImg_Color);
              
              //cv::cvtColor(ProcImg_Color,ProcImg_Gray,cv::COLOR_RGB2GRAY);
              
              //Set THRESHOLDED Image To GPUIMG
              /*cv::adaptiveThreshold(ProcImg_Gray,gpuImg,1,
                                    CV_ADAPTIVE_THRESH_MEAN_C,
                                    CV_THRESH_BINARY,
                                    5,
                                    0);
              
              */
              //cv::threshold(Y_Plane,gpuImg,0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU); 
                       
            //  ProcImg_BW.copyTo(gpuImg);         
                                 
              RemoveBackground();
              
              //morphologyEx(gpuImg,gpuImg,cv::MORPH_CLOSE,element2);
             
              //ProcImg_BW.copyTo(gpuImg2);
              
              cv::imshow("GPU Image",gpuImg);  
              //gpu.write(gpuImg);           
               
             // std::cout<<"\n I am here \n";
              gpulock=false;
          
          }
          
         
          
          /* First Watch For Any Finger Movement*/
          if(draw_wait == false)
          {
           draw_wait=true;
           Blend_Puzzle(OvImg,0.60);
           imshow("Puzzle",OvImg);
           //cpu.write(OvImg);
           cv::waitKey(1);
           
           draw_wait=false; 
          } 
          
          
                     
       
        }while ( (IPCAM::status == 1 ) && (isPuzzleComplete() == false) );

  free(CrCb_Values);
  free(CrCb_ratios);
  free(max_YCrCb);
  free(histogram);
  
  }
  catch(...)
  {
      std::cout<<"\n Some Error Occured. \n Have to check Code \n";
      close_threads=true;
      cleanup();
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
            
               try
               { 

                while(gpulock==true)
                {}
                
                   /*GPU LOCK BECOMES FALSE HERE
                     SET IT TO TRUE AND DO WORK
                   */  
                   gpulock=true;
                   
                   /* 
                      IF FINGER IS AT DEFAULT VALUE I.E -1
                      THEN ONLY FIND FINGER LOCATION
                      OTHERWISE ASSUME THAT THE FINGER
                      LOCATION IS STORED FROM A PREVIOUS VALUE
                   */   
                   prev_block=finger_location();
                   
                   std::cout<<"\n Block Location : " <<prev_block<<std::endl;          
                                    
                   gpulock=false;            
              
                   std::this_thread::sleep_for(std::chrono::milliseconds(500));
                  
                   while(gpulock==true)
                   { }
                   /* LOCK THE GPU AFTER A WAITING TIME */
                  
                            gpulock=true;
                            new_block=finger_location();
                            gpulock=false;      
                  
                   
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
                       //draw_wait=false;
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
               
               catch(...)
               {
                    draw_wait=false;
                    gpulock=false;
               
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
        
