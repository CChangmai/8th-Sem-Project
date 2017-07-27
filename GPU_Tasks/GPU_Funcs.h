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

void TransferValues(int*,float*);

