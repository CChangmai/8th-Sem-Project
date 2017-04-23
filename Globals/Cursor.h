#ifndef HAS_IOSTREAM
    #include<iostream>
    #define HAS_IOSTREAM
#endif


#ifdef __cplusplus
extern "C"
{
#endif

#include<stdio.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<unistd.h>
#include<ctype.h>
#include <X11/extensions/XTest.h>
/*We Need A Smooth Accelerating Mouse Cursor So That It Moves Accordingly */

int reaction_time=100000; //In Microseconds, Can Be Changed
float max_velocity=5; //5 pixels per second threshold set
/*int prev_x=0;
int prev_y=0;
*/

/* 

    Human Mind Detects Stuttering In Less Than 60 FPS 
    So, we need to set the movement speed accordingly
*/

const float threshold = (1/60) ;


    void MovePointer(int x,int y)//,int acceleration)
    {
            Display *primary = XOpenDisplay(NULL);
           /* Window root_window;
            root_window = XRootWindow(primary, 0);
          
            if(primary==NULL)
            {
                printf("Cannot Open The Display Or Move The Mouse Pointer");
            }
            else
            {
             */       /*                  
                   XSelectInput(primary, root_window, KeyReleaseMask);
                   XWarpPointer(primary,root_window,root_window, prev_x,prev_y,952,570,x, y);
                   prev_x=x; prev_y=y;
                   */
                   XTestFakeMotionEvent (primary, 0, x, y, CurrentTime);
                   XCloseDisplay(primary);
                           
           // }
     }    











#ifdef __cplusplus
}
#endif
 
