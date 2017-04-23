#ifndef HAS_IOSTREAM
    #include<iostream>
    #define HAS_IOSTREAM
#endif

#ifndef HAS_OPENCV
    #include<opencv2/core/core.hpp>
    #include<opencv2/imgproc/imgproc.hpp>
    #include<opencv2/highgui/highgui.hpp>
    #define HAS_OPENCV
#endif

#ifndef HAS_THREADS
    #include<vector>
    #include<thread>
    #include<mutex>
    #include<chrono>
    #define HAS_THREADS
#endif

#ifndef CALLED_OVERLAY
    #include "../Text Overlay/Overlay.h"
    #define CALLED_OVERLAY
#endif    
using namespace std;

//---------------------------------------------------------- 
typedef std::chrono::high_resolution_clock T;

//------------------------------------------------------------------ 

/*I forego the concept of Class Because Its Difficult To Maintain Classes
    For The GPU Work Of Optical Flow
*/
namespace SplitImage
    
    {
        void WriteText(int,cv::Mat,string);
        void OptFlow(int,cv::Mat);
        //void Stitch_Image(int,cv::Mat,cv::Mat);
        bool finished=false;
        string message="";
        
       
        
      
       
        void assign_mutex();
        void assign_threads();
        void swap_free();
        void doWork();
  -     void timer_start();
        
        enum img_block 
        {
            TOP_LEFT = 1 ,
            TOP_RIGHT = 2 ,
            BOTTOM_RIGHT = 3,
            BOTTOM_LEFT = 4
        };
        //---------------------------------------------------------------------------------------
        void ResolveImage(img_block,cv::Mat);
        //------------------------------------------------------------------------------------------
        cv::Mat resolve_blocks(cv::Mat &image,img_block segment)
        {
            int l=(image.cols/2);
            int b=(image.rows/2);
            
            switch(segment)
            
            {
                case TOP_LEFT:      return cv::Mat(image,cv::Rect(0,0,l,b));
                                    break;
                
                case TOP_RIGHT:     return cv::Mat(image,cv::Rect(l,0,l,b));
                                    break;
                
                case BOTTOM_LEFT:   return cv::Mat(image,cv::Rect(0,b,l,b));
                                    break;
                
                case BOTTOM_RIGHT:  return cv::Mat(image,cv::Rect(l,b,l,b));
                                    break;
            
                default:            return image;
                                    break;
            }
        
        
        
        }
        
        
        /*class img_Mutex
        {
           public: int get_id();
                   bool is_locked();                 
                   void assign_id(int); 
                   cv::Mat GetImage(); 
                   void Lock();
                   void Unlock(); 
                   img_Mutex()
                    
          private: int lock_status;
                   cv::Mat* image_location;
                   int ID;      
                   
        };
        
        img_Mutex::img_Mutex()
        {
        */
        /*Assign No Lock While MUTEX is initialised*/
        //lock_status=0;
        //}
        
        /*Function Definitions for Img_Mutex */
        
        
       /* int img_Mutex::get_id()
        {
            return ID;
        }
        
        cv::Mat img_Mutex::GetImage()
        {
        
            return (*image_location);
        }

        void img_Mutex::Lock()
        {
        lock_status=1;        
        }   
        void img_Mutex::Unlock()
        {
        lock_status=0;        
        }        
        */
        
        /*Modified Original Plan
        
          Assign Values To Each Item 
        
          And Contain Status Flags
        
          Assign Flow Clockwise
        */
        
        std::mutex MTL,MTR,MBR,MBL;
        std::vector<cv::Mat> Images;     
        
        std::vector<std::mutex> *img_mutex;
        std::vector<bool> mutex_locks;
        
        
        void assign_mutex()
        {
            img_mutex->push_back(MTL);
            img_mutex->push_back(MTR);
            img_mutex->push_back(MBR);
            img_mutex->push_back(MBL);
             
            for(int i=0;i<4;i++)
            {
                    bool status=false;
                    mutex_locks.push_back(status);
            
            }     
          
     
        }
        
        void ResolveImage(cv::Mat &image)
        {
                     Images.push_back(resolve_blocks(image,TOP_LEFT));                    
                     Images.push_back(resolve_blocks(image,TOP_RIGHT));   
                     Images.push_back(resolve_blocks(image,BOTTOM_RIGHT));
                     Images.push_back(resolve_blocks(image,BOTTOM_LEFT));
        
        }
            
        
        
        
        void Split(cv::Mat &image)
        {
                
          ResolveImage(image);          
          assign_mutex();                       
          doWork();
        }
        
        void swap_free()
        {
        /*Check For Linear Freeness*/
             
+        
        
        
        } 
        
        /* The Lengthy Thread Management Function */
        void doWork()
        {
                std::thread IMG_Timer(timer_start());
                
                                
                int count=0;
                //Cyclic Function Call
                do 
                {
            for(int i=0,k1,k2,k3,k4;i<4;i++,k1=(i%2),k2=((i+1)%2),k3=((i+2)%2),k4=((i+3)%2))
            {
                    try
                    {
                        if(finished!=true)
                        {   
                            
                            
                            
                            std::thread t1(OptFlow,k1,Images[k1]);
                            std::thread t2(WriteText,k2,Images[k2],message);
                            std::thread t3(WriteText,k3,Images[k3],message);
                            std::thread t4(WriteText,k4,Images[k4],message);
                            
                    
                             t2.join(); 
                             t3.join();
                             t1.join(); 
                             t4.join();
                             else
                             {
                                end
                             }
                          }
                    count++;
                    }
                    catch(...)
                    {
                    swap_free();
                    continue;                   
                    }
            }
           
                      
                
                
                }while(count<4);
        
        finished=true;
            
        }
        
        void WriteText(int ID,cv::Mat &temp,string s)
        {
            mutex *m=img_mutex[ID];
            std::lock_guard<std::mutex> l(*m);
            temp = Write_Text(s,temp);
        }
          
        void OptFlow(int ID,cv::Mat &temp)
        {
            mutex *m=img_mutex[ID];
            std::lock_guard<std::mutex> l(*m);
        
        }
        
        void timer_start()
        {
            std::this_thread::sleep_for(chrono::milliseconds(15));
            finished=true;       
        }        
            
       
    }








