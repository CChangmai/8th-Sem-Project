

gpu:
    export PATH=$PATH1 nvcc -std=c++11 -c GPU_Tasks/GPU2.cu -o GPU.o 
all:
    g++ -std=c++11 -L/usr/local/cuda/lib64 -o FINAL CCode.o GPU_Tasks/GPU.o -lcuda -lcudart `pkg-config --cflags --libs opencv`
CPU:
    g++ -g OnlyCPU.cpp -o CPU -std=c++11 -lpthread `pkg-config --cflags --libs opencv`
clean:
    rm -f CPU PROJECT
