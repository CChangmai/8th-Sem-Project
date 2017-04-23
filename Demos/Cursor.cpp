#include<iostream>
#define HAS_IOSTREAM

#include "../Globals/Cursor.h"

int main(int argc, char *argv[])
{
   if( argc > 1)
   {
    MovePointer(int(*argv[1]),int(*argv[2]));
    
   }
return 0;

}

