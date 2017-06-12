//All The Functions That Can Be Called By The CPU is contained

int finger_location();

unsigned char* CreateFilterAddress();


unsigned char* MapImageToCPU();

int* assign_threshold();

void* MapVariable(int);

void* GetGPUAddress(int*);

void CleanImage();

unsigned char* MapFilter();

/*May Change it Later.
 Assuming 6 Variables
 Copy it only once in memory for processing
 Using simple Malloc Function
*/ 
#define height 480
#define width 640
#define isize 307200
#define N 192
#define bsize 80

/*Store Pixel Density in CUDA Memory And GPU Memory for Analysis */
int *gthresh,*pdensity,*maxval,*gputhresh;

//I will get The Thread ID From The Called Function
/* 

    Swan Dive Into Zero-Copy MEMORY World 
    ALSO CALLED HOST MAPPED MEMORY
    Keeping My Fingers Crossed
    gpuptr stores the gpu location of mapped memory
    cpuptr stores the cpu location of mapped memory
    
    I THINK ALL OF THESE VARIABLES ARE HOST CODE
*/
unsigned char *gpuptr,*cpuptr,*fcount,*gpucount,*gpufilter;




typedef struct
{
    int h;
    int w;
    unsigned char data[bsize][bsize]; //40x40 image
}Blocks;


/*
    Assign Blocks in CUDA Memory for fast execution
    Blocks Is User-Defined Datatype defined above
    UNSURE WHETHER ITS HOST MEMORY OR DEVICE MEMORY
*/

Blocks *small_image;
/*
Use This If U need To Find The Cursor Location
int *loc_x,*loc_y; 
*/
 


void cleanup()
{
    //cudaFreeHost(cpuptr); Not Doing This due to some problems in main function
    cudaFree(small_image);
    
}


__host__ unsigned char* MapImageToCPU()
{
    //Add Finger Count Here As Well
    unsigned char* ptr=NULL;
          fcount=NULL;
    
    cudaSetDeviceFlags(cudaDeviceMapHost); 
    
    /*Assign Both CPU image and GPU Count*/
    
    cudaHostAlloc(&ptr,height*width,cudaHostAllocMapped);
    cudaHostAlloc(&fcount,1,cudaHostAllocMapped);
    
    /*Assign The GPU Pointer To The Location Made by HostAlloc
                        In CPU Memory */
    
    cudaHostGetDevicePointer(&gpuptr,ptr,0);
    cudaHostGetDevicePointer(&gpucount,fcount,0);
    
    cpuptr=ptr;
    
    std::cout<<"\nThe Pointer Allocated in CPU Memory is : "<<&ptr;
    std::cout<<"\nThe Pointer Allocated in GPU Memory is : "<<&gpuptr;
    return ptr;
} 

__host__ void* MapVariable(int size)
{
    
    //Add Finger Count Here As Well
    void* ptr=NULL;
      
    
    cudaSetDeviceFlags(cudaDeviceMapHost); 
    /*Assign Both CPU image and GPU Count*/
    cudaHostAlloc(&ptr,size,cudaHostAllocMapped);
    
    return ptr;
}

__host__ unsigned char* MapFilter()
{
    
    //Add Finger Count Here As Well
    unsigned char* ptr=NULL;
    cudaSetDeviceFlags(cudaDeviceMapHost); 
    /*Assign Both CPU image and GPU Count*/
    cudaHostAlloc(&ptr,bsize*bsize,cudaHostAllocMapped);
    cudaHostGetDevicePointer(&gpufilter,ptr,0);
    return ptr;
}

__host__ void* GetGPUAddress(int* &cpuid)
{
    void* ptr=NULL;
    cudaSetDeviceFlags(cudaDeviceMapHost);
    cudaHostGetDevicePointer(&ptr,cpuid,0);
    return ptr;
}

/*
GET A POINTER TO CPU_MEMORY TO ASSIGN THRESHOLD RATHER THAN COPYING ANYTHING TO GPU
ALSO USE A GLOBAL FUNCTION THAT CAN ASSIGN MEMORY IN DEVICE
*/
__global__ void assign_GPU_variables()
{
       
}

__host__ int* assign_threshold()
{
        
      
     //---------CALL A GLOBAL FUNCTION THAT CAN SAFELY ASSIGN GPU MEMORY---------//
        cudaMalloc(&pdensity,N*sizeof(int));
        cudaMemset(&pdensity,0,N*sizeof(int));
        
        cudaMalloc(&maxval,sizeof(int));
        cudaMemset(&pdensity,0,sizeof(int));
        
        //cudaMalloc(loc,sizeof(Pt));
        //cudaMalloc(&gpucount,sizeof(int));
        cudaMalloc(&small_image,N * sizeof(Blocks));
     //-----------NOW MAP DEVICE MEMORY---------------------------
     
     int* ptr=NULL;
     cudaSetDeviceFlags(cudaDeviceMapHost); 
     /*Assign Both CPU image and GPU Count*/
     cudaHostAlloc(&ptr,5*sizeof(int),cudaHostAllocMapped);
     cudaHostGetDevicePointer(&gthresh,ptr,0);
     return ptr;
}     
 
 /* I'm Calling Split Blocks According To Symmetric Blocks And Threads  */
 
__global__ void splitblocks(unsigned char* ptr,Blocks* &simage) 
{                                                           
 /* Format of dim3 is (x,y,z)
    If we assign (1,200) that means 1 grid and 200 blocks 
    So
    x=1
    y=200
 */   
 
  int bID=blockIdx.y;
  int x=threadIdx.x;
  int y=threadIdx.y;
  
  int pixel_loc = bID + (width * y)+ x; // We Don't need x here because block ID gives X-coordinate value
  
  /* I'm not doing this in a single loop because all threads might access the same 
     pixel density variable, which might lead to Use of Extra atomic Functions
     So, I separated The Image into a grid for easy access
  */   
  
    if((bID<=N) && ((x*y)<=isize))
     {
            simage[bID].data[x][y]=ptr[pixel_loc];
     } 
}


__global__ void write_density(int* &pd,Blocks* &simage)
    {
        int x=threadIdx.x;
        pd[x]=0;
        int i,j;
        for(i=0;i<N;i++)
        {
            for(j=0;j<N;j++)
            {
               if(simage[x].data[i][j]==1)
               {
                 pd[x]++; //This addition is threadsafe
               }
            
            }
        
        }

    }
    
 

__host__ int finger_location()
{
/*Assuming I've Already Have Assigned CPU Image Mapped Pointer */

        /*Searching Function*/
        dim3 sblocks (1,N);
        dim3 sthreads (bsize,bsize); // NxN array
        
        splitblocks <<< sblocks,sthreads >>> (gpuptr,small_image);
              
        
        
        /* Adding Function */
 
        write_density <<< 1,200 >>> (pdensity,small_image);
 
            
        /*Synchronize All working threads*/
            
        cudaThreadSynchronize();
        
        /* Simple CALLING Function after Doing Everything */    
         
        
        int BLOCK;
        
        for(int i=0;i<N;i++)
        {
           if(pdensity[i]<gputhresh[0])
           {
              BLOCK=i;
           }    
        
        }
         


cleanup();
return BLOCK;

}

void CleanImage()
{
//cudaFreeHost(cpuptr);
}
/*Block Thread Process 

Unfinished Cursor Finding Process

__global__ checkarc(bool direction)
{
       int bID= BlockIdx.y;
       int tx=threadIdx.x;
       

       if(direction == 1) // From Left To Right
       {
          if (small_image[bID].data[circle_y(tx)][tx] == 1)
          {
                    loc.x=                          //I don't know
          
          }       
       
       
       }
        
       else
       {
             
        
        
        
       }



}
*/


/*Dummy Main Function
  Just To Be On The Safe Side
 */

