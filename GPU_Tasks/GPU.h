//All The Functions That Can Be Called By The CPU is contained

int finger_location();

unsigned char* CreateFilterAddress();


unsigned char* MapImageToCPU();

int* assign_threshold();

void* MapVariable(int);

void* GetGPUAddress(int*);

void CleanImage();

unsigned char* MapFilter();

__device__ int checkarc(unsigned char[][BSIZE],unsigned char*);

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
int *gthresh,*pdensity,*maxval,*gputhresh;

//I will get The Thread ID From The Called Function
/* 

    Swan Dive Into Zero-Copy MEMORY World 
    ALSO CALLED HOST MAPPED MEMORY
    Keeping My Fingers Crossed
    VARIABLE NAMES STARTING WITH g OR gpu stores the gpu location of mapped memory
    cpuptr stores the cpu location of mapped memory
    
    I THINK ALL OF THESE VARIABLES ARE HOST CODE
*/
unsigned char *gpuptr,*gpucount;
__device__ unsigned char *gpufilter; //FOR ACCESS BY BOTH CPU AND GPU

unsigned char* createImageBuffer(unsigned int bytes)
{
    unsigned char *ptr = NULL;
    cudaSetDeviceFlags(cudaDeviceMapHost);
    cudaHostAlloc(&ptr, bytes, cudaHostAllocMapped);
    std::cout<<"Adddress Pointed To : "<<(void*)ptr<<std::endl;
    return ptr;
}




typedef struct
{
    int h;
    int w;
    unsigned char data[BSIZE][BSIZE]; //40x40 image
}Blocks;


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


__host__ unsigned char* MapImageToCPU()
{
    //Add Finger Count Here As Well
    unsigned char* ptr;
                   
                      
    cudaSetDeviceFlags(cudaDeviceMapHost); 
    
    /*Assign Both CPU image and GPU Count*/
    
    cudaHostAlloc((void**)&ptr,height*width,cudaHostAllocMapped);
        
    /*Assign The GPU Pointer To The Location Made by HostAlloc
                        In CPU Memory */
      
    
    cudaHostGetDevicePointer((void**)&gpuptr,  (void*)ptr,   0);
   
    
    std::cout<<"\nThe Pointer Allocated in CPU Memory is : "<<(void*)ptr;
    std::cout<<"\nThe Pointer Allocated in GPU Memory is : "<<(void*)gpuptr;
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
    unsigned char* ptr;
    cudaSetDeviceFlags(cudaDeviceMapHost); 
    /*Assign Both CPU image and GPU Count*/
    cudaHostAlloc((void**)&ptr,BSIZE*BSIZE,cudaHostAllocMapped);
    cudaHostGetDevicePointer((void**)&gpufilter,(void*)ptr,0);
    
    std::cout<<"GPU LOCATION OF FILTER : "<<(void*)gpufilter<<"endl";
    return ptr;
}

__host__ void* GetGPUAddress(int* &cpuid)
{
    void* ptr;
    cudaSetDeviceFlags(cudaDeviceMapHost);
    cudaHostGetDevicePointer((void**)&ptr,(void*)cpuid,0);
    return ptr;
}

/*
GET A POINTER TO CPU_MEMORY TO ASSIGN THRESHOLD RATHER THAN COPYING ANYTHING TO GPU
ALSO USE A GLOBAL FUNCTION THAT CAN ASSIGN MEMORY IN DEVICE

__global__ void assign_GPU_variables()
{
       
}
*/
__host__ int* assign_threshold()
{
        
      
     //---------CALL A GLOBAL FUNCTION THAT CAN SAFELY ASSIGN GPU MEMORY---------//
        cudaMalloc(&pdensity,N*sizeof(int));
        cudaMemset(&pdensity,0,N*sizeof(int));
        
        cudaMalloc(&maxval,sizeof(int));
        cudaMemset(&pdensity,0,sizeof(int));
        
        //cudaMalloc(loc,sizeof(Pt));
        //cudaMalloc(&gpucount,sizeof(int));
        cudaMalloc(&small_images,N * sizeof(Blocks));
     //-----------NOW MAP DEVICE MEMORY---------------------------
     
     int* ptr;
     cudaSetDeviceFlags(cudaDeviceMapHost); 
     /*Assign Both CPU image and GPU Count*/
     cudaHostAlloc(&ptr,5*sizeof(int),cudaHostAllocMapped);
     cudaHostGetDevicePointer((void**)&gthresh,(void*)ptr,0);
     std::cout<<"\nThe Pointer Allocated in CPU Memory For Fingers is : "<<(void*)ptr<<"\n";
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
  
  /* 
     I'm not doing this in a single loop because all threads might access the same 
     pixel density variable, which might lead to Use of Extra atomic Functions
     So, I separated The Image into a grid for easy access
  */   
  
    if(pixel_loc<isize)
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
        
        if( checkarc(simage[x].data,gpufilter) > 100 )
        {
                pd[x]+=1000;
        
        }

    }
    
 

__host__ int finger_location()
{
/*Assuming I've Already Have Assigned CPU Image Mapped Pointer */

        /*Searching Function*/
        dim3 sblocks (1,N);
        dim3 sthreads (BSIZE,BSIZE); // NxN array
        
        /* 
           SPLIT CURRENT IMAGE FROM IPCAM TO MULTIPLE IMAGES 
           THEN FIND THE PIXEL DENSITIES OF THE IMAGES         
           THEN SYNCHRONIZE THE FINSIHING OF ALL WORKING THREADS
        */
        
        splitblocks <<< sblocks,sthreads >>> (gpuptr,small_images);
        
        cudaThreadSynchronize();
              
        write_density <<< 1,200 >>> (pdensity,small_images);
 
            
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
cudaFreeHost(gpuptr);
}
/*Block Thread Process 

Unfinished Cursor Finding Process
*/
__device__ int checkarc(unsigned char value[][BSIZE],unsigned char* filter)
{
        int temp=0,i,j;
       
       /*   BITWISE AND WITH THE VALUE 
            OF THE BLOCK WITH THE FILTER
            CROSS MATCH WITH REFERENCE VALUE 
            LATER......
       */     
        for(i=0;i<BSIZE;i++)
          {
            for(j=0;j<BSIZE;j++)
            {
                    if( (value[i][j] && filter[i + (BSIZE*j)] ) == 255 )
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

