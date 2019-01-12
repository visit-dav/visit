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

#ifndef DDTSIM_H
#define DDTSIM_H

#include <cstddef>
#include "ddtsim_constants.h"

#include "visitsim.h"
#include "visitdata.h"
#include "libsimlibrary.h"

//! Structure used to convay state information to the implementation of the VisIt callbacks
/*! (ddtSimGetMetaData, ddtSimGetMesh, ddtSimGetVariable, ddtSimGetDomainList) */
typedef struct {
    LibsimLibrary* libsim;
    VisItSim* sim;
    VisItData* data;
} VisItDataPointers;

//! Structure used to convay commands from ddt-debugger to ddtsim
/*! ddt-debugger will use gdb to modify the contents of this structure directly, without performing any
 * memory allocations or deallocations. When the ddtsim signal handler is next called (when the
 * program is running again and free from risk of deadlocks over memory management functions)
 * these instructions will be copied into DDTSim::visitData, and any dynamic data generated. */
typedef struct
{
    bool break_to_ddt;
    VisItMesh mesh;
    VisItVariable variable;

    //! The maximum number of characters permited in a mesh or variable name. Last character must be '\0'.
    /*! This should not be modified by ddt-debuger (hence the name), as it is hard-coded into
     *  the library. This variable is here only so ddt-debugger can access it should it be desired */
    int max_name_length__no_modify;
} ddtsim_interface_type;

enum MpiMode { NOMPI, OPENMPI, MPICH2, MPT3 };

class DDTSim
{
public:
    static DDTSim* getInstance();
    static void error(const char*,const char* var=NULL);
    static void error(const char*,int,const char *var=NULL);
    static void error(const char*,int,int);
    static void error(const char*,const char*,const char*);

    static void init();
    static bool initialised()       { return mInstance != NULL; }
    static void setProgram(const char *name, const char *path);
    static void setSharedPath(const char *ddtSharedPath);
    static void setMpi(MpiMode m) { mMpiMode = m; }
    static void setVisItPaths(const char *visitPath, const char *visitArchPath);
    static MpiMode mpi() { return mMpiMode; }

    void visitloop();
    bool isParallel()               { return visitSim.size() > 1; }
    bool isLogging()                { return mLogging; }
    void update(ddtsim_interface_type&);

private:
    int processVisItCommand();

    static DDTSim* mInstance;
    static char mProgramName[MAX_NAME_LENGTH];
    static char mProgramPath[MAX_PATH_LENGTH];
    static char mDDTSharedPath[MAX_PATH_LENGTH]; //!< Path to writable directory shared by frontend and target
    static char mAbsSimFileName[MAX_SIMFILE_NAME_LENGTH];
    static char mVisItPath[MAX_PATH_LENGTH];     //!< Path to VisIt launch script
    static char mVisItArchPath[MAX_PATH_LENGTH]; //!< Path to VisIt arch-specific directory
    static MpiMode mMpiMode;        //! Set using \c setMpiMode() before calling \c init()

    bool mLogging;                  //! Should we log visit mesh/variable commands for debug purposes

    VisItSim visitSim;
    VisItData visitData;
    LibsimLibrary libsim;

    DDTSim(const char*, int par_rank, int par_size);
    VisItDataPointers pointers;
};

void DDT_Init_sig();

#endif // DDTVISIT_H
