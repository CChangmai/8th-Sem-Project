
GPUFLAGS=-L/usr/local/cuda/lib64 -lcuda -lcudart
PATH1=$PATH:/usr/local/cuda/lib64


gpu: GPU_Tasks/GPU2.cu
     export PATH=$PATH1 nvcc -std=c++11 -c GPU_Tasks/GPU2.cu -o GPU.o 
     
    
    

all:CCode.o GPU_Tasks/GPU.o
     g++ -std=c++11 -L/usr/local/cuda/lib64 -o FINAL CCode.o GPU_Tasks/GPU.o -lcuda -lcudart `pkg-config --cflags --libs opencv`



CPU:CCode.o
    g++ -std=c++11 -c Main.cpp -o CCode.o `pkg-config --cflags --libs opencv`


clean:

