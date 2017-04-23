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

#include "../Text Overlay/Overlay.h"
#define CALLED_OVERLAY

#ifdef NEEDS_VECTOR
    #include<vector>
#endif

using namespace cv;
using namespace std;

int main()
{ 
   
    string url;
    cout<<"Enter URL Of The IP Camera : " ;
    getline(cin,url);
    cout<<"\n Entered URL Is : "<<url<<endl;
    cout.flush();
    
    VideoCapture IP_Feed;
    bool frame_status=0;

    //Read Image From URL
    IP_Feed.open(url);
    
    
    //Test If IPCAM is open

    if (IP_Feed.isOpened())
    {
            /*Set Frame Rate to 60 FPS
              Because Less Than That Gives Stuttering            
            */
            IP_Feed.set(CV_CAP_PROP_FPS,120);
            double width=IP_Feed.get(CV_CAP_PROP_FRAME_WIDTH);            
            double height=IP_Feed.get(CV_CAP_PROP_FRAME_HEIGHT);
           

            /*Make A Resizable Window According To The Feed*/
            
            
            std::vector <string> m;
            string input;
            char ch='Y';
            do
            {
            
            
            cout<<"Enter The Line You Want To Enter : ";
                               
            getline(cin,input);
          
            cout<<"Entered Input Is "<< input<<endl;
            m.push_back(input);
          
            cout<<"Enter Y or y to continue : ";
          
            cin>>ch;          
            cin.ignore();
            }while((ch=='Y')||(ch=='y'));
            
            
                
            namedWindow("Live Feed",CV_WINDOW_AUTOSIZE);            
            resizeWindow("Live Feed",width,height);            
            cout<<"\n\tDisplaying Image in "<<width<<"x"<<height<<"pixels";

    while(1)
            {

                /* The Lengthy Processing Part*/


                
                Mat image;
                frame_status=IP_Feed.read(image);
                if (frame_status)
                {
                    
                    imshow("Live Feed",Write_Text(m,image));
                    /*
                    Optional Key to End The Stream
                            
                    */
                     waitKey(1);
                }

                else
                {
                    cout<<"Error. Cannot Read Any Frame From The Live Feed";
                    waitKey(0);
                }
                
            }

    }    
    else
    {
        cout<<"Could Not Find IP Camera Feed For The Given URL";    
    }




    return 0;
}







