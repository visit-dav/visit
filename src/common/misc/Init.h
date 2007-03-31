#ifndef INIT_H
#define INIT_H
#include <misc_exports.h>

namespace Init
{
    void MISC_API Initialize(int &argc, char *argv[], int r=0, int n=1, bool strip=true);
    void MISC_API SetComponentName(const char *); 
    const char * MISC_API GetExecutableName(); 
    const char * MISC_API GetComponentName(); 
}

#endif
