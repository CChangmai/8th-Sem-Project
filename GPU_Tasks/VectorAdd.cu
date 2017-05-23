#include<stdio.h>

#define N 200





__global__ void addvector(int* a,int* b,int* c)
    {
        int x=threadIdx.x;
        
        if(x<N)
        {
        c[x]=a[x]+b[x];
        }


    }








int main()
{
    int ch,arr[N],brr[N],result[N],*gpu1,*gpu2,*res;
    
    printf("Enter A Number Between 1 and 200 : ");
    scanf("%d",&ch);
    
    ch=(ch>N)?N:ch;
    
    printf("Enter Numbers Of First Array : \n");
    
    int i;
        for(i=0;i<ch;i++)
        {
            scanf("%d",&arr[i]);
           
        }
        
   printf("Enter Numbers Of Second Array : \n");     
        
        for(i=0;i<ch;i++)
        {
            scanf("%d",&brr[i]);
        }            
    
        for(i=ch;i<N;i++)
        {
        arr[i]=0;
        brr[i]=0;
        }
    
    
   // dim3 numBlocks(1,1);
   // dim3 numThreads(N,1);
    
    
        
    cudaMalloc((void**)&gpu1,N * sizeof(int));    
    cudaMalloc((void**)&gpu2,N * sizeof(int));
    cudaMalloc((void**)&res,N * sizeof(int));
    
    cudaMemcpy(gpu1,arr,N * sizeof(int),cudaMemcpyHostToDevice);
    cudaMemcpy(gpu2,brr,N * sizeof(int),cudaMemcpyHostToDevice);
    
    
    
    addvector<<<1,N>>>(gpu1,gpu2,res);
    cudaThreadSynchronize();
   
    cudaMemcpy(result,res,N * sizeof(int),cudaMemcpyDeviceToHost);
    
    
    
    
       printf("The Result Of Addiing The Array Elements Are : \n");
       
       for(i=0;i<ch;i++)
        {
            printf("%d\t",result[i]);
           
        }
    printf("\n");    
        
    cudaFree(gpu1);
    cudaFree(gpu2);
    cudaFree(res);
        
    
   return 0; 

}
