// ************************************************************************* //
//                               Init.h                                      //
// ************************************************************************* //

#ifndef INIT_H
#define INIT_H

#include <misc_exports.h>


typedef void  (*ErrorFunction)(void *, const char *);


// ****************************************************************************
//  Namespace: Init
//
//  Purpose:
//      A module that does initialization procedures.  It has been extended to
//      contain callbacks for components that allow it to get the compnent
//      name, issue warnings, etc.
//
//  Programmer: "Hank Childs"    (Added documentation)
//  Creation:   August 8, 2003   (Added documentation)
//
// ****************************************************************************

namespace Init
{
    MISC_API void Initialize(int &argc, char *argv[], int r=0, int n=1, bool strip=true);
    MISC_API void SetComponentName(const char *); 
    MISC_API const char * GetExecutableName(); 
    MISC_API const char * GetComponentName(); 
    MISC_API bool  IsComponent(const char *);
    MISC_API void ComponentIssueError(const char *);
    MISC_API void ComponentRegisterErrorFunction(ErrorFunction, void *);
}

#endif


