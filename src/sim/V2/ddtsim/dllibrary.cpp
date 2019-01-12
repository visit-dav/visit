/*****************************************************************************
*
* Copyright (c) 2011, Allinea
* All rights reserved.
*
* This file has been contributed to the VisIt project, which is
* Copyright (c) Lawrence Livermore National Security, LLC. For  details, see
* https://visit.llnl.gov/.  The full copyright notice is contained in the 
* file COPYRIGHT located at the root of the VisIt distribution or at 
* http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
