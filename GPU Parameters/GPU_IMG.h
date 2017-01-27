#include<stdio.h>
#include<GPU_Cores.h>
#include<cuda>
#include<iostream>
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
            int value; //Considering 8-Bit Value
            std::string url="";
            double size;
            //Constructors For Different Things
            GPUImage(int a,int b);
            GPUImage(string),                     
    private:
            void calculate_size()
            {
                this.size=double(rows*cols);
            }
            void CreateGPUImage(GPUImage*);
            {

            }
};




