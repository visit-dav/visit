#ifndef INIT_H
#define INIT_H
#include <misc_exports.h>

namespace Init
{
    MISC_API void Initialize(int &argc, char *argv[], int r=0, int n=1, bool strip=true);
    MISC_API void SetComponentName(const char *); 
    MISC_API const char * GetExecutableName(); 
    MISC_API const char * GetComponentName(); 
}

#endif
