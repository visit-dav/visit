// Copyright (c) 2011, Allinea
// All rights reserved.
//
// This file has been contributed to the VisIt project, which is
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "dllibrary.h"

//! Dlopen the specified library. Library will be dlclose-d when this object is destroyed.
DlLibrary::DlLibrary(const char* lib)
{
    library = strdup(lib);
    dlerror();      // Clear the dl error code, if there is one
    handle = dlopen(lib,RTLD_NOW | RTLD_LOCAL);
    char* error = dlerror();

    if (!handle)
    {
        fprintf(stderr,"(ddtsim,dllibrary) Unable to load library dynamic library %s: %s",lib,error);
    }
}

//! Object destructor, close the library
DlLibrary::~DlLibrary()
{
    delete library;
    library = NULL;

    if (handle!=NULL)
    {
        dlclose(handle);
    }
}

//! Obtain a pointer to the specified symbol in the library opened by this object
void* DlLibrary::symbol(const char* sym)
{
    dlerror();                         // Clear the dl error code, if there is one
    void* value = dlsym(handle, sym);  // Get pointer to symbol from the library
    char* error = dlerror();
    if (error != NULL)
    {
        fprintf(stderr,"(ddtsim,dllibrary) Failed to get symbol '%s' from dynamic library %s: %s",sym,library,error);
        return NULL;
    }
    else if (value == NULL)
    {
        fprintf(stderr, "(ddtsim,dllibrary) Encountered NULL value for '%s' from dynamic library %s: %s",sym,library,error);
        return NULL;
    }
    return value;
}
