#include<iostream>
//#include "GPU_Tasks/GPU.h"

unsigned char* Test1()
{
    unsigned char x=201;
    unsigned char *ptr=&x;
    std::cout<<"\n My address : "<<(void*)ptr<<std::endl;
    std::cout<<"\n My Value : "<<(int)*ptr<<std::endl;
    return ptr;
}

int main()
{
    /*Assign CPU Memory and check for leaks*/
    //unsigned char* dummypointer;
    //unsigned char* p = createImageBuffer(10,&dummypointer);
    
    unsigned char* p = Test1();
    
    std::cout<<(int)*p<<std::endl;
    
    *p=25;
    
    
     std::cout<<"New VALUE :  "<<(int)*p<<"\n";
    
            
return 0;
}
