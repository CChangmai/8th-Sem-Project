#ifdef __cplusplus
extern "C" {
#endif 
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

// Having a Structure Containing The Default Keywords For Sending Data
char* combine(char*,char[]);


typedef struct
    {   
    const char* Latitude;
    const char* Longitude;
    const char* Accelerometer;
    const char* Degrees; 
    const char* Weather;
    }ecodes;
    
ecodes e = {"LAT","LNG","ACC","DEG","WTH"};   

char* cText(const char* c) // Return A Character Array
{
    
    int s=strlen(c);
    int i=0; //Typical noob
    char* message="Error In Message Transfer";


    printf("String Length Is : %d \n",s);
    char f[4]={c[s-3],c[s-2],c[s-1],'\0'};
    printf("Keyword Extracted : %s \n",f);
    
    char data[11];
    for(i=0;(i<s-3)&&(i<10);i++)
    {
        data[i]=*(c+i);    
    }
    if((s-3)<= 10)
    {
        data[s-3]='\0';
    }
    else
    {
        data[11]='\0';
    }
    printf("Printing Relevant Data : %s \n",data);
   
    
    if(strcmp(f,e.Latitude)) 
    {
    message="Latitude Is : ";
    return combine(message,data);
    }
    else if(strcmp(f,e.Longitude))
    {
    message="Longitude Is";
    return combine(message,data);
    }
    else if(strcmp(f,e.Accelerometer))
    {
    message="Acceleration Is";
    return combine(message,data);
    }
    else if(strcmp(f,e.Weather))
    {
    message="The Weather Is ";
    return combine(message,data);
    }    
    
    return message;    
            
    
}  

char* combine(char* x, char y[])
    {
        int length=strlen(x)+strlen(y);
        char* m=(char*)malloc(length*sizeof(char));
        strcpy(m,x);
        strcat(m,y);
        return m;

    }
    
char* Extract_Clipboard()
{
    char* s=
}    
    
#ifdef __cplusplus
}
#endif
        //Comparison Check
