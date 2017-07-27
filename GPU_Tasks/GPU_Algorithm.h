//All The Functions That Can Be Called By The CPU is contained

int finger_location();

unsigned char* CreateFilterAddress();

void RemoveBackground();

unsigned char* MapImageToCPU(int);

unsigned char* MapCalibrator(int);

int* assign_threshold();

int* MapValues(int);

void* GetGPUAddress(int*);

void CleanImage();

unsigned char* MapFilter();

void calculateHistogram();

void checkSafe(cudaError_t,std::string,int);

void ConvertBox(); 

void TransferValues(int*,int*,float*);

/*May Change it Later.
 Assuming 6 Variables
 Copy it only once in memory for processing
 Using simple Malloc Function
*/ 

#define isize 307200
#define N 48
#define DELTA 128

#ifndef HAS_SIZE
    #define BSIZE 80
    #define HEIGHT 480
    #define WIDTH 640
    #define HAS_SIZE
#endif    

/*Store Pixel Density in CUDA Memory And GPU Memory for Analysis */
int *pdensity,*cpu_maxval,*gpu_maxval,*gputhresh;

__constant__ float f_ratios[10];
__constant__ int f_values[10];
__shared__ int fast_density[N];
//I will get The Thread ID From The Called Function
/* 

    Swan Dive Into Zero-Copy MEMORY World 
    ALSO CALLED HOST MAPPED MEMORY
    Keeping My Fingers Crossed
    VARIABLE NAMES STARTING WITH g OR gpu stores the gpu location of mapped memory
    cpuptr stores the cpu location of mapped memory
    
    I THINK ALL OF THESE VARIABLES ARE HOST CODE
*/
unsigned char *gpuptr,*gpucount,*gpufilter,*gpusubptr,*gpu_R,*gpu_G,*gpu_B,*gpu_Y,*gpu_Cr,*gpu_Cb; //FOR ACCESS BY BOTH CPU AND GPU
unsigned char *calibrate_R,*calibrate_G,*calibrate_B,*calibrate_Y,*calibrate_Cr,*calibrate_Cb;


typedef struct
{
   unsigned char data[BSIZE * BSIZE]; //80x80 image
}Blocks;

__device__ int checkarc(Blocks*,unsigned int,unsigned char*);

__device__ bool checkRange(int,int,int);

__device__ bool checkRange(unsigned char,unsigned char,unsigned char);

__device__ bool checkRange(float,float,float,float);

__device__ float GetPart(float,int);

__global__ void find_maxvalue(int*,int*,int *,Blocks*,unsigned char*);

__global__ void BGSubtract(unsigned char*,unsigned char*,unsigned char*,unsigned char*);

__global__ void splitblocks(unsigned char*, Blocks*);

__global__ void PrintK(int*);

__global__ void Make_Histogram();

__global__ void write_density(int*,Blocks*);

__global__ void Find_YCrCb(unsigned char*,unsigned char*,unsigned char*,unsigned char*,unsigned char*,unsigned char*,int);
/*
    Assign Blocks in CUDA Memory for fast execution
    Blocks Is User-Defined Datatype defined above
    UNSURE WHETHER ITS HOST MEMORY OR DEVICE MEMORY
*/

Blocks *small_images;
/*
Use This If U need To Find The Cursor Location
int *loc_x,*loc_y; 
*/
 


void cleanup()
{
    //cudaFreeHost(cpuptr); Not Doing This due to some problems in main function
    cudaFree(small_images);    
    cudaFree(pdensity);
}


void checkSafe(cudaError_t err,std::string message,int line)
{

    if(err != cudaSuccess)
    {
        std::cout<<"Error At Function : "<<message<<"@ line "<<line<<"\n";
        std::cout<<cudaGetErrorString(err)<<std::endl;
    }


}


void TransferValues(int* ptr,int* density,float *ptr2)
{
    cudaMemcpyToSymbol(f_values,(void*)ptr,10 * sizeof(int),0,cudaMemcpyHostToDevice);
    cudaMemcpyToSymbol(f_ratios,(void*)ptr2,10 * sizeof(float),0,cudaMemcpyHostToDevice);
    PrintK <<<1,1>>> (NULL);
    
    for(int i=0;i<5;i++)
    { std::cout<<density[i]<<" "; }
}

__host__ unsigned char* MapImageToCPU(int ch=1)
{
    //Add Finger Count Here As Well
    unsigned char* ptr;
                   
                      
    checkSafe(cudaSetDeviceFlags(cudaDeviceMapHost),
              "Assigning Device Flags @ MapImageToCPU",
              __LINE__); 
    
    /*Assign Both CPU image and GPU Count*/
    
    checkSafe(cudaHostAlloc((void**)&ptr, HEIGHT * WIDTH * sizeof(unsigned char) ,cudaHostAllocMapped),
                            "Allocating CPU Main Image",
                            __LINE__);
        
    /*Assign The GPU Pointer To The Location Made by HostAlloc
                        In CPU Memory */
      
    switch(ch)
    {
        case 1 : 
              checkSafe(cudaHostGetDevicePointer((void**)&gpuptr,  (void*)ptr,   0),
              "GPU Host Allocation",
              __LINE__);
             
             break;
        
        case 2 :
                checkSafe(cudaHostGetDevicePointer((void**)&gpusubptr,  (void*)ptr,   0),
                "Allocating GPU Subtractor",
                __LINE__);        
                
                break;
        
        case 3:
                 checkSafe(cudaHostGetDevicePointer((void**)&gpu_R,  (void*)ptr,   0),
                "Allocating GPU R-Plane",
                __LINE__);        
                
                break;
                
        case 4: 
                checkSafe(cudaHostGetDevicePointer((void**)&gpu_G,  (void*)ptr,   0),
                "Allocating GPU G-Plane",
                __LINE__);        
                
                break;        
        case 5:
                checkSafe(cudaHostGetDevicePointer((void**)&gpu_B,  (void*)ptr,   0),
                "Allocating GPU B-Plane",
                __LINE__);        
                
                break;      
                
        case 6:
                checkSafe(cudaHostGetDevicePointer((void**)&gpu_Y,  (void*)ptr,   0),
                "Allocating GPU Y-Plane",
                __LINE__);        
                
                break;           
                
        case 7:
                checkSafe(cudaHostGetDevicePointer((void**)&gpu_Cr,  (void*)ptr,   0),
                "Allocating GPU Cr-Plane",
                __LINE__);        
                
                break;                
        
        case 8:
                checkSafe(cudaHostGetDevicePointer((void**)&gpu_Cb,  (void*)ptr,   0),
                "Allocating GPU Cb-Plane",
                __LINE__);        
                
                break;                
        
        
        default:
                break;
      }           
             
    return ptr;
} 


__host__ unsigned char* MapCalibrator(int ch)
{
    unsigned char *ptr;
    checkSafe(cudaSetDeviceFlags(cudaDeviceMapHost),"Assigning Device Flags @ Subtractor",__LINE__);
    
    checkSafe(cudaHostAlloc(&ptr,BSIZE * BSIZE * sizeof(unsigned char), cudaHostAllocMapped), "Allocating CPU Subtractor",__LINE__);
    
    switch(ch)
        {
        case 1:
               checkSafe(cudaHostGetDevicePointer((void**)&calibrate_R,  (void*)ptr,   0),
                        "Allocating Calibrator - R Plane",__LINE__);
               break;
        case 2:
               checkSafe(cudaHostGetDevicePointer((void**)&calibrate_G,  (void*)ptr,   0),
                        "Allocating Calibrator - G Plane",__LINE__);
               break;
        
        case 3:
               checkSafe(cudaHostGetDevicePointer((void**)&calibrate_B,  (void*)ptr,   0),
                        "Allocating Calibrator - B Plane",__LINE__);
               break;         
        case 4:
               checkSafe(cudaHostGetDevicePointer((void**)&calibrate_Y,  (void*)ptr,   0),
                        "Allocating Calibrator - Y Plane",__LINE__);                
        
                break;
        case 5:
               checkSafe(cudaHostGetDevicePointer((void**)&calibrate_Cr,  (void*)ptr,   0),
                        "Allocating Calibrator - Cr Plane",__LINE__);
                break;        
        case 6:
               checkSafe(cudaHostGetDevicePointer((void**)&calibrate_Cb,  (void*)ptr,   0),
                        "Allocating Calibrator - Cb Plane",__LINE__);                
                        
                break;  
           
              
        default: break;
        }
    
    
    return ptr;
}


__host__ unsigned char* MapFilter()
{
    
    //Add Finger Count Here As Well
    unsigned char* ptr;
    checkSafe(cudaSetDeviceFlags(cudaDeviceMapHost),"Assigning Device Flags @ Filter",__LINE__); 
    /*Assign Both CPU image and GPU Count*/
    checkSafe(cudaHostAlloc((void**)&ptr, BSIZE * BSIZE * sizeof(unsigned char) ,cudaHostAllocMapped),"Assigning CPU Filter",__LINE__);
    checkSafe(cudaHostGetDevicePointer((void**)&gpufilter,(void*)ptr,0),"Assigning GPU Filter",__LINE__);
    
    
    //std::cout<<"\nGPU LOCATION OF FILTER : "<<(void*)gpufilter<<"\n";
    checkSafe(cudaDeviceSynchronize(),"Synchroniztion @ Filter",__LINE__);
    return ptr;
}

/*
GET A POINTER TO CPU_MEMORY TO ASSIGN THRESHOLD RATHER THAN COPYING ANYTHING TO GPU
ALSO USE A GLOBAL FUNCTION THAT CAN ASSIGN MEMORY IN DEVICE
*/
__host__ int* assign_threshold()
{
        
      
     //---------CALL A GLOBAL FUNCTION THAT CAN SAFELY ASSIGN GPU MEMORY---------//
        checkSafe(cudaMalloc((void**)&pdensity,N * sizeof(int)), "Allocating Density Records",__LINE__);
        
           
        checkSafe(cudaMalloc((void**)&small_images,N * sizeof(Blocks)),"Assigning GPU Block Records",__LINE__);
        checkSafe(cudaMemset((void**)small_images,0,N * sizeof(Blocks)),"Assigning GPU Block Records",__LINE__);
     //-----------NOW MAP DEVICE MEMORY---------------------------
        
     int* ptr;
         checkSafe(cudaSetDeviceFlags(cudaDeviceMapHost), "Assigning Device Flags @ Threshold",__LINE__); 
     
     
     /*
        Assign Both CPU MAX VALUE and GPU Count 
          
     
     */
     
     checkSafe(cudaHostAlloc( (void**)&ptr, 5 * sizeof(int), cudaHostAllocMapped),"Allocating CPU Threshold",__LINE__);
     checkSafe(cudaHostGetDevicePointer((void**)&gputhresh,(void*)ptr,0), "Assigning GPU Threshold",__LINE__);
    
     checkSafe(cudaHostAlloc( (void**)&cpu_maxval,2 * sizeof(int), cudaHostAllocMapped),"Assigning CPU Max_VAL",__LINE__);
     checkSafe(cudaHostGetDevicePointer((void**)&gpu_maxval,(void*)cpu_maxval,0), "Assigning GPU MAX_VAL",__LINE__);
     
     //std::cout<<"\nThe Pointer Allocated in CPU Memory For Fingers is : "<<(void*)ptr<<"\n";
     
     return ptr;
}     
 
 /* I'm Calling Split Blocks According To Symmetric Blocks And Threads  */
 
 void RemoveBackground()
 {
   
    dim3 numblocks(32,24);
    dim3 numthreads(20,20);
    
    Find_YCrCb <<< numblocks,numthreads >>> (gpu_R,gpu_G,gpu_B,gpu_Y,gpu_Cr,gpu_Cb,1);
    
   
    checkSafe(cudaDeviceSynchronize(),"Synchronization At Background Removal",__LINE__);
    
    BGSubtract <<< numblocks,numthreads >>> (gpu_Y,gpu_Cr,gpu_Cb,gpuptr);
    
  
    checkSafe(cudaDeviceSynchronize(),"Synchronization At Background Removal",__LINE__);
 }
 
 
 void ConvertBox()
 {
    dim3 numblocks (4,4);
    dim3 numthreads (20,20);
    
    Find_YCrCb <<< numblocks,numthreads >>> (calibrate_R,calibrate_G,calibrate_B,
                                             calibrate_Y,calibrate_Cr,calibrate_Cb,2);               
 
 
 }
 
 __global__ void Find_YCrCb(unsigned char* img_R,unsigned char* img_G,unsigned char* img_B,
                            unsigned char* img_Y,unsigned char* img_Cr,unsigned char* img_Cb,
                            int ch)
{

  int x = ( blockIdx.x * blockDim.x ) + threadIdx.x;
  int y = ( blockIdx.y * blockDim.y ) + threadIdx.y;
       
  int SPACING;
  
  switch(ch)
  {
    case 1: SPACING = WIDTH ;
            break;
           
    case 2: SPACING = BSIZE ;
      
           break;
           
    default:
    
           break;              
  
  
  }
  
 int pos = x + ( y * SPACING ) ;

 float Y  = ( img_R[pos] * 0.299 ) + ( img_G[pos] * 0.587 ) + ( img_B[pos] * 0.114 );
 float Cr = ( ( img_R[pos] - Y ) * 0.713 ) + DELTA; 
 float Cb = ( ( img_B[pos] - Y ) * 0.564 ) + DELTA;  
 
 img_Y[pos]  = (unsigned char)Y;
 img_Cr[pos] = (unsigned char)Cr;
 img_Cb[pos] = (unsigned char)Cb;

}                            
 
 
 __global__ void BGSubtract(unsigned char* Y,unsigned char* Cr,unsigned char* Cb,unsigned char* dest)
 {
        int x = ( blockIdx.x * blockDim.x ) + threadIdx.x;
        int y = ( blockIdx.y * blockDim.y ) + threadIdx.y;
        
        int pos = x + ( y * WIDTH ) ;
        
        float norm_Cr,norm_Cb,light_adjust;
        int temp,offset;
        
        
        if( pos < isize )
        {
        
          for(int i=0;i<5;i++)
                 {
                    temp = (int)f_ratios[ ( 2 * i ) + 1 ];    
                    
                    light_adjust = (float)(temp % 1000) / (float)Y[pos]; 
                    offset = temp / 1000;
                    

                    
                    norm_Cr = (float)( Cr[pos] * light_adjust) / (float)Y[pos]; 
                    norm_Cb = (float)( Cb[pos] * light_adjust)/ (float)Y[pos];
                           
                    
                 if( checkRange(Cr[pos], 158 , (unsigned char)(19 * light_adjust)) && 
                     checkRange(Cb[pos] ,107 , (unsigned char)(25 * light_adjust)) )
                {     
                     
                    if(checkRange(norm_Cr,f_ratios[2*i],0.40f,0.20f) && 
                      checkRange(norm_Cb, GetPart(f_ratios[(2*i)+1],offset) , 0.20f,0.60f) ) 
                      {
                       
                       dest[pos]=255;                
                      }
                    else
                    {
                       dest[pos]=0;
                  
                    }  

                }  
                
                else
                {
                
                  dest[pos]=0;
                
                }    
             }
             
             
        }
        
         
 
 
 }
 
__global__ void PrintK(int* density)
{
    for(int i=0;i<5;i++)
    {
        printf("Cr : %f Cb : %f\n",f_ratios[2*i],f_ratios[(2*i)+1]);
    
    
    }
    if(density != NULL)
    {
      for(int i=0;i<N;i++)
        {
            printf("Density : %d \t",density[i]);
    
    
        }
    }


}

void PThreshold()
{
 
 PrintK <<<1,1>>> (NULL);

}

__device__ float GetPart(float value,int places)
{
   int num = (int)value;
   
   float rem = value - num;
   
   return (rem + float(places));
}

 
 
__global__ void splitblocks(unsigned char* ptr,Blocks* simage) 
{                                                           
 /* Format of dim3 is (x,y,z)
    If we assign (1,200) that means 1 grid and 200 blocks 
    So
    x=1
    y=200
 */   
 
  
  int global_x = ( blockIdx.x * blockDim.x) + threadIdx.x;
  int global_y = ( blockIdx.y * blockDim.y) + threadIdx.y;
  
  int pixel_loc = global_x + ( global_y * WIDTH );
  
  int blockID = ( global_x / BSIZE ) + ( 8 * ( global_y  /  BSIZE ) ); // 8 is the number of BLOCKS Per ROW
                                                            
  int local_x = global_x % BSIZE ;                          
  int local_y = global_y % BSIZE ;
  
  int pos = local_x + ( local_y * BSIZE );
  /* 
    ASSUMING LINEAR ASSIGNMENT USING ROW MAJOR 
    ONE ROW CONTAINS INDEX 1 TO 640 SO ONE THREAD
    MOVES "IN MULTIPLES OF 80" UPTO 640
    
    THEN AGAIN HE HAS TO MOVE LOCALLY IN Y OR
    VERTICAL DIRECTION
    SO FURTHER MOVEMENT OF (640 x Number of Local Columns)   
    
    THEN FINALLY MOVE IN X-DIRECTION TO GET (X,Y)
    CO-ORDINATE 
    
  */
  

  
   if(pixel_loc<isize)
     {
       simage[blockID].data[pos]=ptr[pixel_loc];
     } 
     
    
}

__host__ int* MapValues()
{





   return NULL;
}


__global__ void write_density(int* pd, Blocks* simage)

    {
        //Blocks* simage) GOES IN FUNCTION LINE
        int i,x=threadIdx.x;
        
        pd[x]=0;      
           
        for(i=0;i<(BSIZE*BSIZE); i++)
        {
             if(simage[x].data[i] == 255 )
               {
                   pd[x]++;
               }
       }
   
}
    
 

__host__ int finger_location()
{
/*Assuming I've Already Have Assigned CPU Image Mapped Pointer */
       
       dim3 sblocks(32,24);
             
       dim3 sthreads(20,20); // NxN array ; Max 512 threads per block 
                             // for GPU's with Compute 2.0 and lower                   
        
        /* 
           SPLIT CURRENT IMAGE FROM IPCAM TO MULTIPLE IMAGES 
           THEN FIND THE PIXEL DENSITIES OF THE IMAGES         
           THEN SYNCHRONIZE THE FINSIHING OF ALL WORKING THREADS
        */
        
        splitblocks <<< sblocks ,sthreads >>> (gpuptr,small_images);
        
        
        //checkSafe(cudaDeviceSynchronize(),"Synchronization @ Finding Fingers",__LINE__);
           
       
                    
        write_density <<< 1,N >>> (pdensity, small_images);
      
        //checkSafe(cudaDeviceSynchronize(),"Synchronization @ Finding Fingers",__LINE__);
        
        PrintK <<<1,1>>> (pdensity);
                      
        /* Simple CALLING Function after Doing Everything */    
         
        find_maxvalue <<<1,1>>> (pdensity,gputhresh,gpu_maxval,small_images,gpufilter); 
        
        
        /*Synchronize All working threads*/
        checkSafe(cudaDeviceSynchronize(),"Synchronization @ Finding Fingers",__LINE__);            
        return cpu_maxval[0];

}

/*Block Thread Process 
Unfinished Cursor Finding Process
*/

__device__ bool checkRange(int value,int threshold,int deviation)
{
    return ( ( value > ( threshold - deviation) ) && (value < ( threshold + deviation) ) );
}

__device__ bool checkRange(unsigned char value,unsigned char threshold,unsigned char deviation)
{

    return ( ( value > ( threshold - deviation) ) && (value < ( threshold + deviation) ) );
}

__device__ bool checkRange(float value,float threshold,float upper_limit,float lower_limit)
{
    return ( ( value > ( threshold - lower_limit) ) && (value < ( threshold + upper_limit) ) );
}



__global__ void find_maxvalue(int* array,int* records,int* value,Blocks* image,unsigned char* filter)
{
    int i,BLOCK=-1;
    
    value[0]= -1;
    
    for(i=0;i<N;i++)
    {
                if(checkRange(array[i],records[0],50) ||
                   checkRange(array[i],records[1],50) ||
                   checkRange(array[i],records[2],50) ||
                   checkRange(array[i],records[3],50) ||
                   checkRange(array[i],records[4],50))
                {  
                BLOCK = i;  
                }
                
               
                         
    }
    
    
        
        
    value[0]=BLOCK;


}

__global__ void Make_Histogram()
{

}


__device__ int checkarc(Blocks* image,int value,unsigned char* filter)
{
        int temp=0,i,j,pos;
       
       /*   BITWISE AND WITH THE VALUE 
            OF THE BLOCK WITH THE FILTER
            CROSS MATCH WITH REFERENCE VALUE 
            LATER......
       */     
        for(i=0;i<BSIZE;i++)
          {
            for(j=0;j<BSIZE;j++)
            {
            
                    pos= i + (j * BSIZE);
                    
                    if( (image[value].data[pos] && filter[pos] ) == 255 )
                    {
                            temp=temp + 1;
                    
                    }
                    
            }
          
          }
        
     return temp;   
}

                     
