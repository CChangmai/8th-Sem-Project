/*
C++ Program To Read An Image From
A Specified URL And Display And Image
*/

#include<iostream>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

int main()
{
while(1)
{
    string a;      
    cout<<"Enter Image URL : ";
    cin>>a;
    cout<<a<<endl;
    Mat image=imread(a,CV_LOAD_IMAGE_COLOR);
    if (!image.data)
        {
    cout<<"Could Not Get The Image\n";

        }
    else
    {   namedWindow("Display Window",WINDOW_NORMAL);
        imshow("Display Window", image);
        resizeWindow("Display Window", 800 , 600);
        cv::waitKey(0);
    }
}

return 0;
}
