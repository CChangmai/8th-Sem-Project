//All The Functions That Can Be Called By The CPU is contained

int finger_location();

unsigned char* CreateFilterAddress();

void RemoveBackground();

unsigned char* MapImageToCPU();

unsigned char* MapSubtractor();

int* assign_threshold();

void* MapVariable(int);

void* GetGPUAddress(int*);

void CleanImage();

unsigned char* MapFilter();


void checkSafe(cudaError_t,std::string,int);

__device__ int checkarc(unsigned char*,unsigned char*);

__device__ bool checkRange(int,int,int);

__global__ void find_maxvalue(int*,int*,int *);

__global__ void BGSubtract(unsigned char*,unsigned char*);



/*May Change it Later.
 Assuming 6 Variables
 Copy it only once in memory for processing
 Using simple Malloc Function
*/ 
#define height 480
#define width 640
#define isize 307200
#define N 48

#ifndef HAS_SIZE
    #define BSIZE 80
    #define HAS_SIZE
#endif    

/*Store Pixel Density in CUDA Memory And GPU Memory for Analysis */
int *pdensity,*cpu_maxval,*gpu_maxval,*gputhresh;

//I will get The Thread ID From The Called Function
/* 

    Swan Dive Into Zero-Copy MEMORY World 
    ALSO CALLED HOST MAPPED MEMORY
    Keeping My Fingers Crossed
    VARIABLE NAMES STARTING WITH g OR gpu stores the gpu location of mapped memory
    cpuptr stores the cpu location of mapped memory
    
    I THINK ALL OF THESE VARIABLES ARE HOST CODE
*/
unsigned char *gpuptr,*gpucount,*gpufilter,*gpusubptr; //FOR ACCESS BY BOTH CPU AND GPU


typedef struct
{
   unsigned char data[BSIZE*BSIZE]; //80x80 image
}Blocks;



__global__ void splitblocks(unsigned char*, Blocks*);
//__global__ void write_density(int*,Blocks*);
__global__ void write_density(int*,unsigned char*);
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
}


void checkSafe(cudaError_t err,std::string message,int line)
{

    if(err != cudaSuccess)
    {
        std::cout<<"Error At Function : "<<message<<"@ line "<<line<<"\n";
        std::cout<<cudaGetErrorString(err)<<std::endl;
    }


}

__host__ unsigned char* MapImageToCPU()
{
    //Add Finger Count Here As Well
    unsigned char* ptr;
                   
                      
    checkSafe(cudaSetDeviceFlags(cudaDeviceMapHost),"Assigning Device Flags @ MapImageToCPU",__LINE__); 
    
    /*Assign Both CPU image and GPU Count*/
    
    checkSafe(cudaHostAlloc((void**)&ptr, height * width * sizeof(unsigned char) ,cudaHostAllocMapped), "Allocating CPU Main Image",__LINE__);
        
    /*Assign The GPU Pointer To The Location Made by HostAlloc
                        In CPU Memory */
      
    
    checkSafe(cudaHostGetDevicePointer((void**)&gpuptr,  (void*)ptr,   0),"GPU Host Allocation",__LINE__);
       
    return ptr;
} 


__host__ unsigned char* MapSubtractor()
{
    unsigned char *ptr;
    checkSafe(cudaSetDeviceFlags(cudaDeviceMapHost),"Assigning Device Flags @ Subtractor",__LINE__);
    
    checkSafe(cudaHostAlloc(&ptr,height * width * sizeof(unsigned char), cudaHostAllocMapped), "Allocating CPU Subtractor",__LINE__);
    checkSafe(cudaHostGetDevicePointer((void**)&gpusubptr,  (void*)ptr,   0),"Allocating GPU Subtractor",__LINE__);
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
        checkSafe(cudaMalloc(&pdensity,N * sizeof(int)), "Allocating Density Records",__LINE__);
        checkSafe(cudaMemset(pdensity,0,N * sizeof(int)), "Assigning 0 to Density",__LINE__);
   
        checkSafe(cudaMalloc(&small_images,N * sizeof(Blocks)),"Assigning GPU Block Records",__LINE__);
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
   
    dim3 numblocks(2,24);
    dim3 numthreads(BSIZE,BSIZE);
    
    BGSubtract <<< numblocks,numthreads >>> (gpuptr,gpusubptr);
    
    checkSafe(cudaThreadSynchronize(),"Synchronization At Background Removal",__LINE__);
 }
 
 
 __global__ void BGSubtract(unsigned char* img1,unsigned char* img2)
 {
        int pos = ( blockIdx.x * gridDim.x ) + threadIdx.x + ( threadIdx.y * blockDim.x );
 
        if( pos < isize )
        {
                img1[pos]= img1[pos] ^ img2[pos];
        
        }
 
 
 }
 
__global__ void splitblocks(unsigned char* ptr,Blocks* simage) 
{                                                           
 /* Format of dim3 is (x,y,z)
    If we assign (1,200) that means 1 grid and 200 blocks 
    So
    x=1
    y=200
 */   
 
  int bID=blockIdx.x;
  int x=threadIdx.x;
  int y=threadIdx.y;
  
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
  
  int pixel_loc = (bID * BSIZE) + (y * width) + x ; 
  int blockID   = x + (y * BSIZE); 
  /* 
     I'm not doing this in a single loop because all threads might access the same 
     pixel density variable, which might lead to Use of Extra atomic Functions
     So, I separated The Image into a grid for easy access
  */   
  
   // if(pixel_loc<isize)
   //  {
            simage[bID].data[blockID]=ptr[pixel_loc];
   //  } 
      
}


__global__ void write_density(int* pd,unsigned char* simage)

    {
        //Blocks* simage) GOES IN FUNCTION LINE
        /*int i,j,x=threadIdx.x;
        pd[x]=0;
        for(i=0;i<N;i++)
        {
            for(j=0;j<N;j++)
            {
               if(simage[x].data[( i * BSIZE) + j] == 0 )
               {
                 atomicAdd(&pd[x],1); //This addition is threadsafe
               }
            
            }
        
        }
        */
        
        
         
  int bID=blockIdx.x;
  int x=threadIdx.x;
  int y=threadIdx.y;
  
  int pixel_loc = (bID * BSIZE) + (y * width) + x ; 
  int blockID   = x + (y * BSIZE); 
  
               if(simage[pixel_loc] == 0 )
               {
                 atomicAdd(&pd[blockID],1); //This addition is threadsafe
               }
  
        
    }
    
 

__host__ int finger_location()
{
/*Assuming I've Already Have Assigned CPU Image Mapped Pointer */

        /*Searching Function*/
        //dim3 sblocks (1,N);
             
       dim3 sthreads (BSIZE,BSIZE); // NxN array
        
        /* 
           SPLIT CURRENT IMAGE FROM IPCAM TO MULTIPLE IMAGES 
           THEN FIND THE PIXEL DENSITIES OF THE IMAGES         
           THEN SYNCHRONIZE THE FINSIHING OF ALL WORKING THREADS
        */
        
        //splitblocks <<< N ,sthreads >>> (gpuptr,small_images);
        //checkSafe(cudaThreadSynchronize(),"Synchronization @ Finding Fingers",__LINE__);
                    
        write_density <<< N,sthreads >>> (pdensity, gpuptr);
        checkSafe(cudaThreadSynchronize(),"Synchronization @ Finding Fingers",__LINE__);    
                 
                      
        /* Simple CALLING Function after Doing Everything */    
         
        find_maxvalue <<<1,1>>> (pdensity,gputhresh,gpu_maxval); 
        
        /*Synchronize All working threads*/
        
        checkSafe(cudaThreadSynchronize(),"Synchronization @ Finding Fingers",__LINE__);
        
            
        std::cout<<(void*)gpuptr<<"\n";    
            
      
 
 return cpu_maxval[0];

}

void CleanImage()
{
cudaFreeHost(gpuptr);
}
/*Block Thread Process 
Unfinished Cursor Finding Process
*/

__device__ bool checkRange(int value,int threshold,int deviation)
{
return ( ( value > ( threshold - deviation) ) && (value < ( threshold + deviation) ) );

}


__global__ void find_maxvalue(int* array,int* records,int* value)
{
    int i,j,BLOCK=-1;
    
    for(i=0;i<N;i++)
    {
            for(j=0;j<5;j++)
            {
                if( checkRange(array[i],records[j],100) == 1 )
                {  
                BLOCK = i;  
                }
                         
            }
    
    }

    value[0]=BLOCK;


}




__device__ int checkarc(unsigned char* value,unsigned char* filter)
{
        int temp=0,i,j,pos=0;
       
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
                    
                    if( (value[pos] && filter[pos] ) == 255 )
                    {
                            temp=temp+1;
                    
                    }
                    
            }
          
          }
        
     return temp;   
}

/*Dummy Main Function
  Just To Be On The Safe Side
 */

