// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               VisItInit.h                                      //
// ************************************************************************* //

#ifndef INIT_H
#define INIT_H

#include <misc_exports.h>


typedef void  (*ErrorFunction)(void *, const char *);
typedef int   (*ThreadIDFunction)(void *);


// ****************************************************************************
//  Namespace: VisItInit
//
//  Purpose:
//      A module that does initialization procedures.  It has been extended to
//      contain callbacks for components that allow it to get the compnent
//      name, issue warnings, etc.
//
//  Programmer: "Hank Childs"    (Added documentation)
//  Creation:   August 8, 2003   (Added documentation)
//
//  Modifications:
//
//    Hank Childs, Tue Jun  1 11:47:36 PDT 2004
//    Added Finalize (should I have renamed this namespace?).
//
//    Mark C. Miller, Tue Mar  8 18:06:19 PST 2005
//    Added ComponentNameToID and ComponentIDToName
//
//    Jeremy Meredith, Wed May 25 13:24:24 PDT 2005
//    Added ability to disable custom signal handlers.
//
//    Hank Childs, Sat Jul  4 14:38:17 PDT 2015
//    Add support for getting number of threads / thread ID.
//    Also remove const that causes compiler warnings.
//    
// ****************************************************************************

namespace VisItInit
{
    MISC_API void Initialize(int &argc, char *argv[], int r=0, int n=1,
                             bool strip=true, bool sigs=true);
    MISC_API void Finalize();
    MISC_API void SetComponentName(const char *); 
    MISC_API const char * GetExecutableName(); 
    MISC_API const char * GetComponentName(); 
    MISC_API int ComponentNameToID(const char *);
    MISC_API const char * ComponentIDToName(const int);
    MISC_API bool  IsComponent(const char *);
    MISC_API void ComponentIssueError(const char *);
    MISC_API void ComponentRegisterErrorFunction(ErrorFunction, void *);
    const char *const CatchAllMeshName = "for active plot";

    MISC_API int  GetNumberOfThreads();
    MISC_API void SetNumberOfThreads(int);
    MISC_API int  GetMyThreadID();
    MISC_API void RegisterThreadIDFunction(ThreadIDFunction, void *);
}

#endif
