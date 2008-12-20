/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
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

// ************************************************************************* //
//                            avtITAPS_CUtility.C                            //
// ************************************************************************* //
#ifndef AVT_ITAPS_C_UTILITY_H
#define AVT_ITAPS_C_UTILITY_H

#include <snprintf.h>
#include <stdarg.h>

#include <avtCallback.h>

#include <map>
#include <string>
#include <vector>

using std::map;
using std::string;
using std::vector;

// end of list
#define EoL (void*)-1
// no list
#define NoL (0,EoL)


// ****************************************************************************
//  Macro: CheckITAPSError2
//
//  Purpose: Very useful macro for checking error condition of ITAPS
//  implementation after a call into the implementation. This macro does work
//  to get an error description, issues a VisIt warning, keeps track of how
//  many times a given warning has been issued and suppresses above 5 aborts
//  (via 'goto') to the end of the function from which it is called and
//  ensures that all pointers allocated prior to the abort get freed.
//  
//  Modifications:
//
//    Mark C. Miller, Mon Oct 27 13:59:48 PDT 2008
//    Made it dribble information of functions that completed succesfully
//    to debug4 also.
//
// ****************************************************************************
#if !defined(ITAPS_GRUMMP)
#define CheckITAPSError2(IMI, ERR, FN, ARGS, THELINE, THEFILE)                                  \
    if (ERR != 0)                                                                               \
    {                                                                                           \
        char msg[1024];                                                                         \
        char desc[256];                                                                         \
        for (int i = 0; i < sizeof(desc); i++) desc[i] = '\0';                                  \
        int dummyError = ERR;                                                                   \
        iMesh_getDescription(IMI, desc, &dummyError, sizeof(desc));                             \
        SNPRINTF(msg, sizeof(msg), "Encountered ITAPS error (%d) after call to \"%s\""          \
            " at line %d in file \"%s\"\nThe description is...\n"                               \
            "    \"%s\"\n", ERR, #FN, THELINE, THEFILE, desc);                                  \
        if (messageCounts.find(msg) == messageCounts.end())                                     \
            messageCounts[msg] = 1;                                                             \
        else                                                                                    \
            messageCounts[msg]++;                                                               \
        if (messageCounts[msg] < 6)                                                             \
        {                                                                                       \
            if (!avtCallback::IssueWarning(msg))                                                \
                cerr << msg << endl;                                                            \
        }                                                                                       \
        else if (messageCounts[msg] == 6)                                                       \
        {                                                                                       \
            if (!avtCallback::IssueWarning(supressMessage))                                     \
                cerr << supressMessage << endl;                                                 \
        }                                                                                       \
        ITAPSErrorCleanupHelper ARGS;                                                           \
        goto funcEnd;                                                                           \
    }                                                                                           \
    else                                                                                        \
    {                                                                                           \
        debug4 << "Made it past call to \"" << #FN << "\" at line "                             \
               << THELINE << " in file " << THEFILE << endl;                                    \
    }
#else
#define CheckITAPSError2(IMI, ERR, FN, ARGS, THELINE, THEFILE)                                  \
    if (ERR != 0)                                                                               \
    {                                                                                           \
        char msg[1024];                                                                         \
        SNPRINTF(msg, sizeof(msg), "Encountered ITAPS error (%d) after call to \"%s\""          \
            "at line %d in file \"%s\"\nThe description is not available\n",                    \
            ERR, #FN, THELINE, THEFILE);                                                        \
        if (messageCounts.find(msg) == messageCounts.end())                                     \
            messageCounts[msg] = 1;                                                             \
        else                                                                                    \
            messageCounts[msg]++;                                                               \
        if (messageCounts[msg] < 6)                                                             \
        {                                                                                       \
            if (!avtCallback::IssueWarning(msg))                                                \
                cerr << msg << endl;                                                            \
        }                                                                                       \
        else if (messageCounts[msg] == 6)                                                       \
        {                                                                                       \
            if (!avtCallback::IssueWarning(supressMessage))                                     \
                cerr << supressMessage << endl;                                                 \
        }                                                                                       \
        ITAPSErrorCleanupHelper ARGS;                                                           \
        goto funcEnd;                                                                           \
    }                                                                                           \
    else                                                                                        \
    {                                                                                           \
        debug4 << "Made it past call to \"" << #FN << "\" at line "                             \
               << THELINE << " in file " << THEFILE << endl;                                    \
    }
#endif

#define CheckITAPSError(IMI, FN, ARGS) CheckITAPSError2(IMI, itapsError, FN, ARGS, __LINE__, __FILE__)

typedef bool (*HandleThisSet)(iMesh_Instance ima, int level, int memidx, bool ises,
    iBase_EntitySetHandle esh, void *cb_data);

namespace avtITAPS_CUtility
{
    extern int itapsError;
    extern char** entTypes;
    extern char** entTopologies;
    extern char** itapsDataTypeNames;
    extern map<string, int> messageCounts;
    extern const char *avtITAPS_CUtility::supressMessage;

    void ITAPSErrorCleanupHelper(int dummy, ...);
    void InitDataTypeNames();
    string VisIt_iMesh_getTagName(iMesh_Instance theMesh, iBase_TagHandle theTag);
    int ITAPSEntityTopologyToVTKZoneType(int ztype);
    int VTKZoneTypeToITAPSEntityTopology(int ztype);
    void TraverseSetHierarchy(iMesh_Instance aMesh, int level, int memberId,
        bool isEntitySet, iBase_EntitySetHandle esh, bool debugOff,
        HandleThisSet handleSetCb, void *handleSetCb_data);
    void GetTagsForEntity(iMesh_Instance aMesh, bool isEntitySet,
        iBase_EntitySetHandle esh, vector<string> &tagNames, vector<int> &tagTypes,
        vector<int> &tagSizes, vector<string> &tagVals, int level);
    bool GetTopLevelSets(iMesh_Instance ima, int level, int memidx, bool ises,
        iBase_EntitySetHandle esh, void *cb_data);
}
#endif
