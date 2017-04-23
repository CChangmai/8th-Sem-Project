#ifndef HAS_IOSTREAM
    #include<iostream>
    #define HAS_IOSTREAM
#endif

#include "../Globals/StrSearch.h"

using namespace std;

int main(int argc, char* argv[])
{
/*Testing For Cross-Compilation */
if ((argv[1] == "") || (argv[1]==NULL))
{
    string s;
    cout<<"Enter A Text To Find Keyword : ";
    cin>>s;
    cout<<cText(s.c_str());
}
else
{
    cout<<cText(argv[1]);

}


return 0;
}
