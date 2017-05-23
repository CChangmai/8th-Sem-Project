#include<iostream>


/*May Change it Later.
 Assuming 6 Variables
 Copy it only once in memory for processing
 Using simple Malloc Function
*/ 
#define height 640
#define width 480
#define isize 307200
#define N 192
#define bsize 100

/*Store Pixel Density in CUDA Memory for Analysis */
int pdensity[N],*gthresh; 


//I will get The Thread ID From The Called Function
/* 

    Swan Dive Into Zero-Copy MEMORY World 
    Keeping My Fingers Crossed
    gpuptr stores the gpu location of mapped memory
    cpuptr stores the cpu location of mapped memory
*/
unsigned char *gpuptr,*cpuptr,*fcount,*gpucount;


/*unsigned char* curve_data;*/ //To See Pixel Location

typedef struct block
{
    int height;
    int width;
    unsigned char data[bsize][bsize]; //40x40 image
}Blocks;


/*
    Assign Blocks in CUDA Memory for fast execution
    Blocks Is User-Defined Datatype defined above
*/

Blocks *small_image;
/*
Use This If U need To Find The Cursor Location
int *loc_x,*loc_y; 
*/
 


__global__ void cleanup()
{
    //cudaFreeHost(cpuptr); Not Doing This due to some problems in main function
    cudaFree(small_image);
    
}


unsigned char* MapImageToCPU()
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
    
    cudaHostGetDevicePointer(&gpuptr,ptr);
    cudaHostGetDevicePointer(&gpucount,fcount);
    
    cpuptr=ptr;
    return ptr;
} 

void* MapVariable(int size)
{
    
    //Add Finger Count Here As Well
    void* ptr=NULL;
      
    
    cudaSetDeviceFlags(cudaDeviceMapHost); 
    /*Assign Both CPU image and GPU Count*/
    cudaHostAlloc(&ptr,size,cudaHostAllocMapped);
    
    return ptr;
}

unsigned char* MapFilter()
{
    
    //Add Finger Count Here As Well
    unsigned char* ptr=NULL;
    cudaSetDeviceFlags(cudaDeviceMapHost); 
    /*Assign Both CPU image and GPU Count*/
    cudaHostAlloc(&ptr,bsize*bsize,cudaHostAllocMapped);
    cudaHostGetDevicePointer(&filter,ptr);
    return ptr;
}

void* GetGPUAddress(int* &cpuid)
{
    void* ptr=NULL;
    cudaSetDeviceFlags(cudaDeviceMapHost);
    cudaHostGetDevicePointer(&ptr,cpuid);
    return ptr;
}

/*
GET A POINTER TO CPU_MEMORY TO ASSIGN THRESHOLD RATHER THAN COPYING ANYTHING TO GPU
*/
int* assign_threshold()
{
        
        cudaMalloc(pdensity,N*sizeof(int));
        cudaMemset(pdensity,0,N*sizeof(int));
        //cudaMalloc(loc,sizeof(Pt));
        cudaMalloc(gpucount,sizeof(int));
        cudaMalloc(small_image,N * sizeof(Blocks));
     
     
     //-----------NOW MAP DEVICE MEMORY---------------------------
     
     int* ptr=NULL;
     cudaSetDeviceFlags(cudaDeviceMapHost); 
     /*Assign Both CPU image and GPU Count*/
     cudaHostAlloc(&ptr,5*sizeof(int),cudaHostAllocMapped);
     cudaHostGetDevicePointer(&gthresh,ptr);
     return *ptr;
}     
 
 /* I'm Calling Split Blocks According To Symmetric Blocks And Threads  */
 
__global__ void splitblocks() 
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
  
  int pixel_loc = int(&gpuptr)+((bID*blockDim.y)+y); // We Don't need x here because block ID gives X-coordinate value
  
  /* I'm not doing this in a single loop because all threads might access the same 
     pixel density variable, which might lead to Use of Extra atomic Functions
     So, I separated The Image into a grid for easy access
  */   
  
    if((bID<=N) && ((x*y)<=isize)
     {
            small_image[bID]->data[x][y]=gpuptr[pixel_loc];
     } 
}


__global__ void write_density()
    {
        int x=threadIdx.x;
        pdensity[x]=0;
        int i,j;
        for(i=0;i<N;i++)
        {
            for(j=0;j<N;j++)
            {
               if(small_image[x]->data[i][j]==1)
               {
                 pdensity[x]++; //This addition is threadsafe
               }
            
            }
        
        }

    }

int finger_location()
{
/*Assuming I've Already Have Assigned CPU Image Mapped Pointer */

        int BLOCK=0;
        /*Searching Function*/
        dim3 sblocks (1,N);
        dim3 sthreads (bsize,bsize); // NxN array
        
        splitblocks <<< sblocks,sthreads >>> ();
              
        
        
        /* Adding Function */
 
        write_density <<< 1,200 >>> ();
 
            
        /*Synchronize All working threads*/
            
        cudaThreadSynchronize();
        
        /* Simple CALLING Function after Doing Everything */     
        for(i=0;i<N;i++)
        {
           if(pdensity[i]<gthresh[0])
           {
              BLOCK=i;
           }    
        
         }      
        
cleanup();
return BLOCK;

}

void CleanImage()
{
cudaFreeHost(cpuptr);
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

