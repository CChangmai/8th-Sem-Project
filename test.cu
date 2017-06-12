#include<iostream>
//#include "GPU_Tasks/GPU.h"

unsigned char* Test1()
{
    static unsigned char x=201;
    unsigned char *ptr=&x;
    std::cout<<"\n My address : "<<(void*)ptr<<std::endl;
    return ptr;
}

int main()
{
    /*Assign CPU Memory and check for leaks*/
   // unsigned char* dummypointer;
    //unsigned char* p = createImageBuffer(10,&dummypointer);
    
    unsigned char* p = Test1();
    *p=25;
    
    
     std::cout<<"Why Is The Address Changing To "<<(void*)p<<"\n";
     std::cout<<"Why Is The Value Changing To : "<<*p<<"\n";
            
return 0;
}
