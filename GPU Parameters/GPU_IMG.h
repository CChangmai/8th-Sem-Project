#include<stdio.h>
#include<GPU_Cores.h>
#include<cuda>
#include<iostream>
#include<opencv2/core/core.hpp>
using namespace std;

/*It's Difficult to Implement Classes in C
So C++ is better
Because Of Public And Private Visibility Scope
*/
class GPUImage
{
    public: int *location;
            int rows;
            int cols;
            std::string url="";
            int size;
            int data[];
            int* Pixel(int a,int b);
            //Constructors For Different Things
            GPUImage(int a,int b,int s);
            GPUImage(string);
            void CreateGPUImage(cv::Mat);
            void CreateGPUImage(cv::IplImage);   
                            
    private:
            void calculate_size()
            {
                this.size=double(rows*cols);
            }
            
            
};

GPUImage :: *Pixel(int a,int b)
{
    /* Translate To Coordinates*/
    int temp = *(location);
    


}
void GPUImage :: CreateGPUImage()
{
        




}
/* All The Constructors will be added Here */
GPUImage :: GPUImage(int a,int b,int s)
{
    //I am assuming nobody needs a console message printed
    rows=a;
    cols=b;
    size=s;
    CreateGPUImage
}



GPUImage :: GPUImage(
{}
GPUImage :: GPUImage(
{}
GPUImage :: GPUImage(
{}





