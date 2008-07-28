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
//                            avtITAPS_CFileFormat.C                           //
// ************************************************************************* //

#include "iBase.h"
#include "iMesh.h"

#include <snprintf.h>

#include <avtITAPS_CFileFormat.h>

#include <map>
#include <string>
#include <vector>

#include <stdarg.h>

#include <vtkCellType.h>
#include <vtkCharArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtVariableCache.h>

#include <DebugStream.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

using     std::map;
using     std::string;
using     std::vector;

static const int maxCoincidentSets = 6;

// end of list
#define EoL (void*)-1
// no list
#define NoL (0,EoL)

// supports passing variable length list of void pointers that if non-zero
// need to be free'd.
static void
ITAPSErrorCleanupHelper(int dummy, ...)
{
   va_list ap;
   va_start(ap, dummy);
   void *ptr = va_arg(ap, void*);
   while (ptr != EoL)
   {
       if (ptr)
           free(ptr);
       ptr = va_arg(ap, void*);
   }
   va_end(ap);
}

static map<string, int> messageCounts;
static const char *supressMessage = "further warnings regarding this error will be supressed";


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
//
// ****************************************************************************
#ifdef ITAPS_MOAB
#define CheckITAPSError2(IMI, ERR, ARGS, THELINE, THEFILE)                                      \
    if (ERR != 0)                                                                               \
    {                                                                                           \
        char msg[1024];                                                                         \
        char desc[256];                                                                         \
        for (int i = 0; i < sizeof(desc); i++) desc[i] = '\0';                                  \
        int dummyError = ERR;                                                                   \
        iMesh_getDescription(IMI, desc, &dummyError, sizeof(desc));                             \
        SNPRINTF(msg, sizeof(msg), "Encountered ITAPS error (%d) at line %d in file \"%s\"\n"   \
            "The description is...\n"                                                           \
            "    \"%s\"\n", ERR, THELINE, THEFILE, desc);                                       \
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
    }
#else
#define CheckITAPSError2(IMI, ERR, ARGS, THELINE, THEFILE)                                      \
    if (ERR != 0)                                                                               \
    {                                                                                           \
        char msg[1024];                                                                         \
        SNPRINTF(msg, sizeof(msg), "Encountered ITAPS error (%d) at line %d in file \"%s\"\n"   \
            "The description is not available\n", ERR, THELINE, THEFILE);                       \
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
    }
#endif

#define CheckITAPSError(IMI, ARGS) CheckITAPSError2(IMI, itapsError, ARGS, __LINE__, __FILE__)

static int itapsError;

static char** entTypes = 0;
static char** entTopologies = 0;
static char** itapsDataTypeNames = 0;

static void
InitDataTypeNames()
{
    // Only need these when debugging is turned on 
    if (!debug1_real)
        return;

    // No need to generate if we've already created 'em 
    if (entTopologies != 0)
        return;

    // Ensure we map entity topologies to appropriate strings
    int nTopologies = iMesh_ALL_TOPOLOGIES+1;
    entTopologies = new char*[nTopologies];
    entTopologies[iMesh_POINT]          = "point";
    entTopologies[iMesh_LINE_SEGMENT]   = "line segment";
    entTopologies[iMesh_POLYGON]        = "polygon";
    entTopologies[iMesh_TRIANGLE]       = "triangle";
    entTopologies[iMesh_QUADRILATERAL]  = "quadrilateral";
    entTopologies[iMesh_POLYHEDRON]     = "polyhedron";
    entTopologies[iMesh_TETRAHEDRON]    = "tetrahedron";
    entTopologies[iMesh_HEXAHEDRON]     = "hexahedron";
    entTopologies[iMesh_PRISM]          = "prism";
    entTopologies[iMesh_PYRAMID]        = "pyramid";
    entTopologies[iMesh_SEPTAHEDRON]    = "septahedron";
    entTopologies[iMesh_ALL_TOPOLOGIES] = "all topologies";

    // Ensure we map entity types to appropriate strings
    int nTypes = iBase_ALL_TYPES+1;
    entTypes = new char*[nTypes];
    entTypes[iBase_VERTEX]    = "vertex";
    entTypes[iBase_EDGE]      = "edge";
    entTypes[iBase_FACE]      = "face";
    entTypes[iBase_REGION]    = "region";
    entTypes[iBase_ALL_TYPES] = "all types";

    // Ensure we map data type names to appropriate strings
    int nDataTypes = 4;
    itapsDataTypeNames = new char*[nDataTypes];
    itapsDataTypeNames[iBase_INTEGER]       = "integer";
    itapsDataTypeNames[iBase_DOUBLE]        = "double";
    itapsDataTypeNames[iBase_ENTITY_HANDLE] = "ehandle";
    itapsDataTypeNames[iBase_BYTES]         = "bytes";
}


static string
VisIt_iMesh_getTagName(iMesh_Instance theMesh, iBase_TagHandle theTag)
{
    static char tmpName[256];
#ifdef ITAPS_MOAB
    iMesh_getTagName(theMesh, theTag, tmpName, &itapsError, sizeof(tmpName));
#elif ITAPS_GRUMMP
    iMesh_getTagName(theMesh, theTag, tmpName, &itapsError, sizeof(tmpName));
    //iMesh_getTagName(theMesh, theTag, tmpName, sizeof(tmpName), &itapsError);
#endif
    return string(tmpName);
}

static int ITAPSEntityTopologyToVTKZoneType(int ttype)
{
    switch (ttype)
    {
    case iMesh_POINT:         return VTK_VERTEX;
    case iMesh_LINE_SEGMENT:  return VTK_LINE;
    case iMesh_POLYGON:       return VTK_POLYGON;
    case iMesh_TRIANGLE:      return VTK_TRIANGLE;
    case iMesh_QUADRILATERAL: return VTK_QUAD;
    case iMesh_TETRAHEDRON:   return VTK_TETRA;
    case iMesh_HEXAHEDRON:    return VTK_HEXAHEDRON;
    case iMesh_PRISM:         return VTK_WEDGE;
    case iMesh_PYRAMID:       return VTK_PYRAMID;
    }
    return -1;
}

//
// We re-define debug5 here because we use TraverseSetHierarchy for multiple
// purposes and don't want data spewed to debug logs every time we traverse
// the set hierarchy
//
#undef debug5
#define debug5 if (debugOff || !debug5_real) ; else debug5_real

typedef bool (*HandleThisSet)(iMesh_Instance ima, int level, int memidx, bool ises,
    iBase_EntitySetHandle esh, void *cb_data);

// ****************************************************************************
//  Function: TraverseSetHierarchy 
//
//  Purpose: Very verbose description of the set hierarchy. Only used for
//  debugging.
//
//  Modifications:
//    Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
//    Added code to output entity type and topology
// ****************************************************************************

static void
TraverseSetHierarchy(iMesh_Instance aMesh, int level, int memberId, bool isEntitySet,
    iBase_EntitySetHandle esh, bool debugOff,
    HandleThisSet handleSetCb, void *handleSetCb_data)
{
#ifndef MDSERVER
    // If we're not on the MDSERVER, we'll never output debugging stuff
    debugOff = true;
#endif

    // Do the callback if one was requested
    bool shouldRecurse = true;
    if (handleSetCb)
        shouldRecurse = (*handleSetCb)(aMesh, level, memberId, isEntitySet, esh, handleSetCb_data);

    iBase_EntitySetHandle *sets = 0; int sets_allocated = 0;
    int sets_size = 0;
    iBase_EntityHandle *ents = 0; int ents_allocated = 0;
    int ents_size = 0;
    iBase_TagHandle *tags = 0; int tags_allocated = 0; int tags_size = 0;

    // compute an indentation
    std::string ident;
    for (int i = 0; i < level; i++)
        ident += "        ";

    if (isEntitySet)
        debug5 << ident << "For Entity Set at level " << level << ", index " << memberId << ":" << endl;
    else
        debug5 << ident << "For Entity     at level " << level << ", index " << memberId << ":" << endl;
    debug5 << ident << "{" << endl;

    if (!isEntitySet)
    {
        int topo, type;
        iMesh_getEntType(aMesh, esh, &type, &itapsError);
        CheckITAPSError(aMesh, NoL);
        debug5 << ident << "    type = \"" << entTypes[type] << "\"" << endl;
        iMesh_getEntTopo(aMesh, esh, &topo, &itapsError);
        CheckITAPSError(aMesh, NoL);
        debug5 << ident << "    topology = \"" << entTopologies[topo] << "\"" << endl;
    }

    if (isEntitySet)
        iMesh_getAllEntSetTags(aMesh, esh, &tags, &tags_allocated, &tags_size, &itapsError);
    else
        iMesh_getAllTags(aMesh, esh, &tags, &tags_allocated, &tags_size, &itapsError);
    CheckITAPSError(aMesh, (0,tags,EoL));
    if (tags_size)
    {
        debug5 << ident << "    tags = " << tags_size << endl;
        debug5 << ident << "    {" << endl;
        debug5 << ident << "            name                 type     size     value(s)..." << endl;
        debug5 << ident << "     --------------------       -------   ----     -----------" << endl;
        for (int t = 0; t < tags_size; t++)
        {
            char lineBuf[256];
            int typeId;
            iMesh_getTagType(aMesh, tags[t], (int*) &typeId, &itapsError);
            CheckITAPSError(aMesh, NoL);
            int tagSize;
            iMesh_getTagSizeValues(aMesh, tags[t], &tagSize, &itapsError);
            CheckITAPSError(aMesh, NoL); 
            string tagName = VisIt_iMesh_getTagName(aMesh, tags[t]);
            CheckITAPSError(aMesh, NoL); 
            if (typeId == iBase_INTEGER)
            {
                if (tagSize == 1)
                {
                    int theVal;
                    if (isEntitySet)
                        iMesh_getEntSetIntData(aMesh, esh, tags[t], &theVal, &itapsError);
                    else
                        iMesh_getIntData(aMesh, esh, tags[t], &theVal, &itapsError);
                    CheckITAPSError(aMesh, NoL); 
                    if (debug5_real)
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %d", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, theVal);
                    }
                }
                else if (tagSize > 1)
                {
                    int *tagvals = 0; int tagvals_allocated = 0; int tagvals_size = 0;
                    iMesh_getIntArrData(aMesh, &esh, 1, tags[t],
                        &tagvals, &tagvals_allocated, &tagvals_size, &itapsError);
                    CheckITAPSError(aMesh, (0,tagvals,EoL)); 
                    std::string valBuf;
                    for (int k = 0; k < tagvals_size; k++)
                    {
                        char tmpChars[32];
                        sprintf(tmpChars, "%d ", tagvals[k]);
                        valBuf += std::string(tmpChars);
                    }
                    if (tagvals_allocated)
                        free(tagvals);
                    if (debug5_real)
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %s", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, valBuf.c_str());
                    }
                }
                else
                {
                    if (debug5_real)
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %s", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, "invalid size");
                    }
                }
            }
            else if (typeId == iBase_DOUBLE)
            {
                if (tagSize == 1)
                {
                    double theVal;
                    if (isEntitySet)
                        iMesh_getEntSetDblData(aMesh, esh, tags[t], &theVal, &itapsError);
                    else
                        iMesh_getDblData(aMesh, esh, tags[t], &theVal, &itapsError);
                    CheckITAPSError(aMesh, NoL); 
                    if (debug5_real)
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %f", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, theVal);
                    }
                }
                else if (tagSize > 1)
                {
                    double *tagvals = 0; int tagvals_allocated = 0; int tagvals_size = 0;
                    iMesh_getDblArrData(aMesh, &esh, 1, tags[t],
                        &tagvals, &tagvals_allocated, &tagvals_size, &itapsError);
                    CheckITAPSError(aMesh, (0,tagvals,EoL)); 
                    std::string valBuf;
                    for (int k = 0; k < tagvals_size; k++)
                    {
                        char tmpChars[32];
                        sprintf(tmpChars, "%f ", tagvals[k]);
                        valBuf += std::string(tmpChars);
                    }
                    if (tagvals_allocated)
                        free(tagvals);
                    if (debug5_real)
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %s", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, valBuf.c_str());
                    }
                }
                else
                {
                    if (debug5_real)
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %s", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, "invalid size");
                    }
                }
            }
            else if (typeId == iBase_BYTES)
            {
                char *theVal = 0; int theVal_allocated = 0;
                int theValSize = 0;
                if (isEntitySet)
                    iMesh_getEntSetData(aMesh, esh, tags[t], &theVal, &theVal_allocated, &theValSize, &itapsError);
                else
                    iMesh_getData(aMesh, esh, tags[t], &theVal, &theVal_allocated, &theValSize, &itapsError);
                CheckITAPSError(aMesh,(0,theVal,EoL));
                std::string valBuf;
                for (int k = 0; k < theValSize; k++)
                {
                    char tmpChars[32];
                    if (theValSize > 4)
                        sprintf(tmpChars, "%c", theVal[k]);
                    else
                        sprintf(tmpChars, "%hhX", theVal[k]);
                    valBuf += std::string(tmpChars);
                }
                if (theVal_allocated)
                    free(theVal);
                if (debug5_real)
                {
                    SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %s", 
                        tagName.c_str(), itapsDataTypeNames[typeId], tagSize, valBuf.c_str());
                }
            }
            else if (typeId == iBase_ENTITY_HANDLE)
            {
                if (tagSize == 1)
                {
                    iBase_EntitySetHandle theVal;
                    if (isEntitySet)
                        iMesh_getEntSetEHData(aMesh, esh, tags[t], &theVal, &itapsError);
                    else
                        iMesh_getEHData(aMesh, esh, tags[t], &theVal, &itapsError);
                    CheckITAPSError(aMesh, NoL); 
                    if (debug5_real)
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %X", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, theVal, &itapsError);
                    }
                }
                else if (tagSize > 1)
                {
                    iBase_EntitySetHandle *tagvals = 0; int tagvals_allocated = 0; int tagvals_size = 0;
                    iMesh_getEHArrData(aMesh, &esh, 1, tags[t],
                        &tagvals, &tagvals_allocated, &tagvals_size, &itapsError);
                    CheckITAPSError(aMesh, (0,tagvals,EoL)); 
                    std::string valBuf;
                    for (int k = 0; k < tagvals_size; k++)
                    {
                        char tmpChars[32];
                        sprintf(tmpChars, "%X ", tagvals[k]);
                        valBuf += std::string(tmpChars);
                    }
                    if (tagvals_allocated)
                        free(tagvals);
                    if (debug5_real)
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %s", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, valBuf.c_str());
                    }
                }
                else
                {
                    if (debug5_real)
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %s", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, "invalid size");
                    }
                }
            }
            debug5 << ident << "         " << lineBuf << endl;
        }
        debug5 << ident << "    }" << endl;
    }
    else
    {
        debug5 << ident << "    tags = NONE" << endl;
    }

    if (isEntitySet == false)
    {
        debug5 << ident << "}" << endl;
        return;
    }

    iMesh_getEntSets(aMesh, esh, 1, &sets, &sets_allocated, &sets_size, &itapsError);
    CheckITAPSError(aMesh,(0,sets,EoL));
    if (sets_size > 0 && shouldRecurse)
    {
        debug5 << ident << "    entity sets = " << sets_size << endl;
        debug5 << ident << "    {" << endl;
        for (int i = 0; i < sets_size; i++)
            TraverseSetHierarchy(aMesh, level+1, i, true, sets[i], debugOff,
                handleSetCb, handleSetCb_data);
        debug5 << ident << "    }" << endl;
    }
    else
    {
        debug5 << ident << "    entity sets = NONE" << endl;
    }

    iMesh_getEntities(aMesh, esh, iBase_ALL_TYPES,
        iMesh_ALL_TOPOLOGIES, &ents, &ents_allocated, &ents_size, &itapsError);
    CheckITAPSError(aMesh,(0,ents,EoL));
    if (ents_size > 0 && shouldRecurse)
    {
        debug5 << ident << "    entities = " << ents_size << endl;
        debug5 << ident << "    {" << endl;
        for (int i = 0; i < ents_size; i++)
            TraverseSetHierarchy(aMesh, level+1, i, false, ents[i], debugOff,
                handleSetCb, handleSetCb_data);
        debug5 << ident << "    }" << endl;
    }
    else
    {
        debug5 << ident << "    entities = NONE" << endl;
    }

    debug5 << ident << "}" << endl;

    if (sets_allocated)
        free(sets);
    if (ents_allocated)
        free(ents);
    if (tags_allocated)
        free(tags);

funcEnd: ;
}

// Reset our overrided definition for debug5
#undef debug5
#define debug5 if (!debug5_real) ; else debug5_real

static void
GetTagsForEntity(iMesh_Instance aMesh, bool isEntitySet, iBase_EntitySetHandle esh,
    vector<string> &tagNames, vector<int> &tagTypes,
    vector<int> &tagSizes, vector<string> &tagVals)
{
    iBase_TagHandle *tags = 0; int tags_allocated = 0; int tags_size = 0;

    if (isEntitySet)
        iMesh_getAllEntSetTags(aMesh, esh, &tags, &tags_allocated, &tags_size, &itapsError);
    else
        iMesh_getAllTags(aMesh, esh, &tags, &tags_allocated, &tags_size, &itapsError);
    CheckITAPSError(aMesh, (0,tags,EoL));

    if (tags_size)
    {
        for (int t = 0; t < tags_size; t++)
        {
            char lineBuf[256];

            int typeId;
            iMesh_getTagType(aMesh, tags[t], (int*) &typeId, &itapsError);
            CheckITAPSError(aMesh, NoL);
            tagTypes.push_back(typeId);

            int tagSize;
            iMesh_getTagSizeValues(aMesh, tags[t], &tagSize, &itapsError);
            CheckITAPSError(aMesh, NoL); 
            tagSizes.push_back(tagSize);

            string tagName = VisIt_iMesh_getTagName(aMesh, tags[t]);
            CheckITAPSError(aMesh, NoL); 
            tagNames.push_back(tagName);

            tagVals.push_back("");

            if (typeId == iBase_INTEGER)
            {
                if (tagSize == 1)
                {
                    int theVal;
                    if (isEntitySet)
                        iMesh_getEntSetIntData(aMesh, esh, tags[t], &theVal, &itapsError);
                    else
                        iMesh_getIntData(aMesh, esh, tags[t], &theVal, &itapsError);
                    CheckITAPSError(aMesh, NoL); 
                    SNPRINTF(lineBuf, sizeof(lineBuf), "%d", theVal);
                }
                else if (tagSize > 1)
                {
                    int *tagvals = 0; int tagvals_allocated = 0; int tagvals_size = 0;
                    iMesh_getIntArrData(aMesh, &esh, 1, tags[t],
                        &tagvals, &tagvals_allocated, &tagvals_size, &itapsError);
                    CheckITAPSError(aMesh, (0,tagvals,EoL)); 
                    std::string valBuf;
                    for (int k = 0; k < tagvals_size; k++)
                    {
                        char tmpChars[32];
                        sprintf(tmpChars, "%d ", tagvals[k]);
                        valBuf += std::string(tmpChars);
                    }
                    if (tagvals_allocated)
                        free(tagvals);
                    SNPRINTF(lineBuf, sizeof(lineBuf), "%s", valBuf.c_str());
                }
            }
            else if (typeId == iBase_DOUBLE)
            {
                if (tagSize == 1)
                {
                    double theVal;
                    if (isEntitySet)
                        iMesh_getEntSetDblData(aMesh, esh, tags[t], &theVal, &itapsError);
                    else
                        iMesh_getDblData(aMesh, esh, tags[t], &theVal, &itapsError);
                    CheckITAPSError(aMesh, NoL); 
                    SNPRINTF(lineBuf, sizeof(lineBuf), "%f", theVal);
                }
                else if (tagSize > 1)
                {
                    double *tagvals = 0; int tagvals_allocated = 0; int tagvals_size = 0;
                    iMesh_getDblArrData(aMesh, &esh, 1, tags[t],
                        &tagvals, &tagvals_allocated, &tagvals_size, &itapsError);
                    CheckITAPSError(aMesh, (0,tagvals,EoL)); 
                    std::string valBuf;
                    for (int k = 0; k < tagvals_size; k++)
                    {
                        char tmpChars[32];
                        sprintf(tmpChars, "%f ", tagvals[k]);
                        valBuf += std::string(tmpChars);
                    }
                    if (tagvals_allocated)
                        free(tagvals);
                    SNPRINTF(lineBuf, sizeof(lineBuf), "%s", valBuf.c_str());
                }
            }
            else if (typeId == iBase_BYTES)
            {
                char *theVal = 0; int theVal_allocated = 0;
                int theValSize = 0;
                if (isEntitySet)
                    iMesh_getEntSetData(aMesh, esh, tags[t], &theVal, &theVal_allocated, &theValSize, &itapsError);
                else
                    iMesh_getData(aMesh, esh, tags[t], &theVal, &theVal_allocated, &theValSize, &itapsError);
                CheckITAPSError(aMesh,(0,theVal,EoL));
                std::string valBuf;
                for (int k = 0; k < theValSize; k++)
                {
                    char tmpChars[32];
                    if (theValSize > 4)
                        sprintf(tmpChars, "%c", theVal[k]);
                    else
                        sprintf(tmpChars, "%hhX", theVal[k]);
                    valBuf += std::string(tmpChars);
                }
                if (theVal_allocated)
                    free(theVal);
                SNPRINTF(lineBuf, sizeof(lineBuf), "%s", valBuf.c_str());
            }
            else if (typeId == iBase_ENTITY_HANDLE)
            {
                if (tagSize == 1)
                {
                    iBase_EntitySetHandle theVal;
                    if (isEntitySet)
                        iMesh_getEntSetEHData(aMesh, esh, tags[t], &theVal, &itapsError);
                    else
                        iMesh_getEHData(aMesh, esh, tags[t], &theVal, &itapsError);
                    CheckITAPSError(aMesh, NoL); 
                    SNPRINTF(lineBuf, sizeof(lineBuf), "%X", theVal);
                }
                else if (tagSize > 1)
                {
                    iBase_EntitySetHandle *tagvals = 0; int tagvals_allocated = 0; int tagvals_size = 0;
                    iMesh_getEHArrData(aMesh, &esh, 1, tags[t],
                        &tagvals, &tagvals_allocated, &tagvals_size, &itapsError);
                    CheckITAPSError(aMesh, (0,tagvals,EoL)); 
                    std::string valBuf;
                    for (int k = 0; k < tagvals_size; k++)
                    {
                        char tmpChars[32];
                        sprintf(tmpChars, "%X ", tagvals[k]);
                        valBuf += std::string(tmpChars);
                    }
                    if (tagvals_allocated)
                        free(tagvals);
                    SNPRINTF(lineBuf, sizeof(lineBuf), "%s", valBuf.c_str());
                }
            }

            tagVals[tagVals.size()-1] = string(lineBuf);

        }
    }

    if (tags_allocated)
        free(tags);

funcEnd: ;
}

static bool 
GetTopLevelSets(iMesh_Instance ima, int level, int memidx, bool ises,
    iBase_EntitySetHandle esh, void *cb_data)
{
    // don't descend deeper than one level
    if (level > 1)
        return false;

    vector<string> tagNames;
    vector<int>    tagTypes;
    vector<int>    tagSizes;
    vector<string> tagVals;

    map<string, vector<iBase_EntitySetHandle> > *theSets = 
        (map<string, vector<iBase_EntitySetHandle> > *) cb_data;

    GetTagsForEntity(ima, ises, esh, tagNames, tagTypes, tagSizes, tagVals);

    for (int i = 0; i < tagNames.size(); i++)
    {
        if (tagNames[i] == "CATEGORY")
            (*theSets)[tagVals[i]].push_back(esh);
    }

    return true;
}


// ****************************************************************************
//  Method: avtITAPS_C constructor
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Wed Mar 7 17:15:33 PST 2007
//
//  Modifications:
//    Mark C. Miller, Tue Jun 26 11:27:48 PDT 2007
//    Modified for C interface to ITAPS
//
// ****************************************************************************

avtITAPS_CFileFormat::avtITAPS_CFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    InitDataTypeNames();
    vmeshFileName = filename;
    geomDim = topoDim = 0;
    vertEnts = edgeEnts = faceEnts = regnEnts = 0;
    numVerts = numEdges = numFaces = numRegns = 0;
    vertEnts_allocated = edgeEnts_allocated =
    faceEnts_allocated = regnEnts_allocated = 0;
    haveMixedElementMesh = false;
}

avtITAPS_CFileFormat::~avtITAPS_CFileFormat()
{
    messageCounts.clear();
}


// ****************************************************************************
//  Method: avtITAPS_CFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Wed Mar 7 17:15:33 PST 2007
//
//  Modifications:
//    Mark C. Miller, Tue Jun 26 11:27:48 PDT 2007
//    Modified for C interface to ITAPS
//
// ****************************************************************************

void
avtITAPS_CFileFormat::FreeUpResources(void)
{
    if (vertEnts && vertEnts_allocated)
        free(vertEnts);
    vertEnts = 0;
    vertEnts_allocated = 0;
    if (edgeEnts && edgeEnts_allocated)
        free(edgeEnts);
    edgeEnts = 0;
    edgeEnts_allocated = 0;
    if (faceEnts && faceEnts_allocated)
        free(faceEnts);
    faceEnts = 0;
    faceEnts_allocated = 0;
    if (regnEnts && regnEnts_allocated)
        free(regnEnts);
    regnEnts = 0;
    regnEnts_allocated = 0;
}


// ****************************************************************************
//  Method: avtITAPS_CFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Wed Mar 7 17:15:33 PST 2007
//
//  Modifications:
//
//    Mark C. Miller, Thu Mar 22 09:37:55 PDT 2007
//    Added code to detect variable centering and populate md with variable
//    info
//
//    Mark C. Miller, Tue Jun 26 11:27:48 PDT 2007
//    Modified for C interface to ITAPS
//
//    Mark C. Miller, Tue Apr 22 21:45:41 PDT 2008
//    Big changes to support MOAB's subsets
//
// ****************************************************************************

void
avtITAPS_CFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int i, j;

    // remove extension from filename
    string tmpFileName = vmeshFileName;
    int q = vmeshFileName.size();
    if (vmeshFileName[q-6] == '.' && vmeshFileName[q-5] == 'v' &&
        vmeshFileName[q-4] == 'm' && vmeshFileName[q-3] == 'e' &&
        vmeshFileName[q-2] == 's' && vmeshFileName[q-1] == 'h')
        tmpFileName = string(vmeshFileName, 0, q-6);
    else if (vmeshFileName[q-6] == '.' && vmeshFileName[q-5] == 'c' &&
             vmeshFileName[q-4] == 'u' && vmeshFileName[q-3] == 'b')
        tmpFileName = string(vmeshFileName, 0, q-4);
    else if (vmeshFileName[q-6] == '.' && vmeshFileName[q-5] == 'v' &&
             vmeshFileName[q-4] == 't' && vmeshFileName[q-3] == 'k')
        tmpFileName = string(vmeshFileName, 0, q-4);

    char dummyStr[32];
    iMesh_newMesh(dummyStr, &itapsMesh, &itapsError, 0);
    CheckITAPSError(itapsMesh, NoL);
    iMesh_getRootSet(itapsMesh, &rootSet, &itapsError);
    CheckITAPSError(itapsMesh, NoL);

    // ok, try loading the mesh.
    try
    {
        iMesh_load(itapsMesh, rootSet, tmpFileName.c_str(), dummyStr, &itapsError,
            tmpFileName.length(), 0);
        CheckITAPSError(itapsMesh, NoL);

        // determine spatial and topological dimensions of mesh
        int geomDim;
        iMesh_getGeometricDimension(itapsMesh, &geomDim, &itapsError);
        CheckITAPSError(itapsMesh, NoL);
        topoDim = -1;
        iMesh_getNumOfType(itapsMesh, rootSet, iBase_VERTEX, &numVerts, &itapsError);
        CheckITAPSError(itapsMesh, NoL);
        if (numVerts > 0)
            topoDim = 0;
        iMesh_getNumOfType(itapsMesh, rootSet, iBase_EDGE, &numEdges, &itapsError);
        CheckITAPSError(itapsMesh, NoL);
        if (numEdges > 0)
            topoDim = 1;
        iMesh_getNumOfType(itapsMesh, rootSet, iBase_FACE, &numFaces, &itapsError);
        CheckITAPSError(itapsMesh, NoL);
        if (numFaces > 0)
            topoDim = 2;
        iMesh_getNumOfType(itapsMesh, rootSet, iBase_REGION, &numRegns, &itapsError);
        CheckITAPSError(itapsMesh, NoL);
        if (numRegns > 0)
            topoDim = 3;

        //
        // Decide if we've got a 'mixed element' mesh
        //
        if ((numEdges > 0 && numFaces > 0) ||
            (numEdges > 0 && numRegns > 0) ||
            (numFaces > 0 && numRegns > 0))
            haveMixedElementMesh = true;

        //
        // Figure out which top-level class we should treat as the 'domain'
        // class.
        //
        string domainSetClassName = "Unknown";
        if (topoDim == 0)
        {
            domainEntType = iBase_VERTEX;
            domainSetClassName = "Vertex";
        }
        else if (topoDim == 1)
        {
            domainEntType = iBase_EDGE;
            domainSetClassName = "Curve";
        }
        else if (topoDim == 2)
        {
            domainEntType = iBase_FACE;
            domainSetClassName = "Surface";
        }
        else if (topoDim == 3)
        {
            domainEntType = iBase_REGION;
            domainSetClassName = "Volume";
        }

        // This first call is purely to provide opportunity to output stuff
        // to debug logs.
        const bool debugOff = true;
        if (debug5_real)
            TraverseSetHierarchy(itapsMesh, 0, 0, true, rootSet, !debugOff, 0, 0);

        // Ok, do some real work to get top-level sets
        TraverseSetHierarchy(itapsMesh, 0, 0, true, rootSet, debugOff,
            GetTopLevelSets, &topLevelSets);

        //
        // Find the 'domain' set class and assign its sets for the domains.
        //
        map<string, vector<iBase_EntitySetHandle> >::iterator tlsit;
        if (topLevelSets.size() == 0)
        {
            domainSets.push_back(rootSet);
            domainSetClassName = "Root";
        }
        else
        {
            for (tlsit = topLevelSets.begin(); tlsit != topLevelSets.end(); tlsit++)
            {
                if (tlsit->first == domainSetClassName)
                {
                    domainSets = tlsit->second;
                    debug5 << "selected \"" << tlsit->first << "\" as the domain class" << endl;
                    break;
                }
                debug5 << "skipping \"" << tlsit->first << "\" as candidate domain class" << endl;
            }
        }

        //
        // Define the mesh
        //
        avtMeshMetaData *mmd = new avtMeshMetaData("mesh", domainSets.size(),
            0, 0, 0, geomDim, topoDim, AVT_UNSTRUCTURED_MESH);
        mmd->blockTitle = domainSetClassName == "Vertex" ? "Verticies" : domainSetClassName + "s";
        mmd->blockPieceName = domainSetClassName;
        md->Add(mmd);

        //
        // Add enumerated scalar variables for top-level subset classes
        // other than the domains.
        //
        for (tlsit = topLevelSets.begin(); tlsit != topLevelSets.end(); tlsit++)
        {
            // skip the 'domain' class
            if (tlsit->first == domainSetClassName)
                continue;

            string className = tlsit->first == "Vertex" ? "Verticies" : tlsit->first + "s";
            avtScalarMetaData *smd = new avtScalarMetaData(className, "mesh", AVT_NODECENT);
            smd->hideFromGUI = true;
            smd->SetEnumerationType(avtScalarMetaData::ByNChooseR);
            smd->SetEnumNChooseRN(tlsit->second.size());
            smd->SetEnumNChooseRMaxR(maxCoincidentSets);
            smd->SetEnumAlwaysExcludeValue(-1.0);
            smd->SetEnumPartialCellMode(avtScalarMetaData::Dissect);

            for (int k = 0; k < tlsit->second.size(); k++)
            {
                char tmpName[64];
                SNPRINTF(tmpName, sizeof(tmpName), "%s_%03d", tlsit->first.c_str(), k);
                smd->AddEnumNameValue(tmpName, k);
            }

            md->Add(smd);
            avtScalarMetaData::BuildEnumNChooseRMap(tlsit->second.size(), maxCoincidentSets,
                pascalsTriangleMaps[className]);
        }

        //
        // Because its so convenient, add the zonetype expression as a
        // variable on the mesh
        //
        Expression expr;
        expr.SetName("zonetype");
        expr.SetDefinition("zonetype(mesh)");
        expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&expr);

        //
        // How to determine tag (field) centering WITHOUT invoking problem
        // sized data work; e.g. an array of entity handles? We pick the
        // first entity in each type class (0d, 1d, 2d, and 3d) and
        // find all the tags defined on those entities and then
        // register them as node or zone centered variables.
        //
        int entTypeClass;
        for (entTypeClass = 0; entTypeClass < 4; entTypeClass++)
        {
            // initialize an entity iterator and get the first entity
            iMesh_EntityIterator entIt;
            iMesh_initEntIter(itapsMesh, rootSet, (iBase_EntityType) entTypeClass,
                iMesh_ALL_TOPOLOGIES, &entIt, &itapsError);
            CheckITAPSError(itapsMesh, NoL);
            iBase_EntityHandle oneEntity;
            int has_data;
            iMesh_getNextEntIter(itapsMesh, entIt, &oneEntity, &has_data, &itapsError);
            CheckITAPSError(itapsMesh, NoL);
            iMesh_endEntIter(itapsMesh, entIt, &itapsError);
            CheckITAPSError(itapsMesh, NoL);

            // get all the tags defined on this one entity
            iBase_TagHandle *tagsOnOneEntity = 0; int tagsOnOneEntity_allocated = 0;
            int tagsOnOneEntity_size = 0;
            iMesh_getAllTags(itapsMesh, oneEntity, &tagsOnOneEntity,
                &tagsOnOneEntity_allocated, &tagsOnOneEntity_size, &itapsError);
            CheckITAPSError(itapsMesh, (0,tagsOnOneEntity,EoL));

            // make a vector of the found handles and copy it
            // to the saved list of primitive tag handles
            vector<iBase_TagHandle> tmpTagHandles;
            for (int kk = 0; kk < tagsOnOneEntity_size; kk++)
                tmpTagHandles.push_back(tagsOnOneEntity[kk]);
            primitiveTagHandles[entTypeClass] = tmpTagHandles;
            if (tagsOnOneEntity_allocated)
                free(tagsOnOneEntity);
        }

        for (entTypeClass = 0; entTypeClass < 4; entTypeClass++)
        {
            avtCentering centering = entTypeClass == 0 ? AVT_NODECENT : AVT_ZONECENT;

            vector<iBase_TagHandle> tagHandles = primitiveTagHandles[entTypeClass];

            for (int tagIdx = 0; tagIdx < tagHandles.size(); tagIdx++)
            {
                iBase_TagHandle theTag = tagHandles[tagIdx]; 
                string tagName = VisIt_iMesh_getTagName(itapsMesh, theTag);
                CheckITAPSError(itapsMesh, NoL);
                int valSize;
                iMesh_getTagSizeValues(itapsMesh, theTag, &valSize, &itapsError);
                CheckITAPSError(itapsMesh, NoL);
                avtVarType var_type = GuessVarTypeFromNumDimsAndComps(geomDim, valSize);

                // ITAPS can sometimes define same tag on multiple entities
                bool shouldSkip = false;
                if (entTypeClass > 0)
                {
                    vector<iBase_TagHandle> tagHandlesOnVerts = primitiveTagHandles[0];
                    for (int t = 0; t < tagHandlesOnVerts.size(); t++)
                    {
                        string vertTagName = VisIt_iMesh_getTagName(itapsMesh, tagHandlesOnVerts[t]);
                        CheckITAPSError(itapsMesh, NoL);
                        if (tagName == vertTagName)
                        {
                            shouldSkip = true;
                            break;
                        }
                    }
                }
                if (shouldSkip)
                    continue;

                if (var_type == AVT_SCALAR_VAR)
                    AddScalarVarToMetaData(md, tagName.c_str(), "mesh", centering);
                else if (var_type == AVT_VECTOR_VAR)
                    AddVectorVarToMetaData(md, tagName.c_str(), "mesh", centering, valSize);
                else if (var_type == AVT_SYMMETRIC_TENSOR_VAR)
                    AddSymmetricTensorVarToMetaData(md, tagName.c_str(), "mesh", centering, valSize);
                else if (var_type == AVT_TENSOR_VAR)
                    AddTensorVarToMetaData(md, tagName.c_str(), "mesh", centering, valSize);
                else
                {
                    vector<string> memberNames;
                    for (int c = 0; c < valSize; c++)
                    {
                        char tmpName[64];
                        SNPRINTF(tmpName, sizeof(tmpName), "%s_%03d", tagName.c_str(), c);
                        memberNames.push_back(tmpName);
                    }
                    AddArrayVarToMetaData(md, tagName.c_str(), memberNames, "mesh", centering);
                }
            }
        }
    }
    catch (iBase_Error TErr)
    {
        char msg[512];
        char desc[256];
        desc[0] = '\0';
        int tmpError = itapsError;
#ifdef ITAPS_MOAB
        iMesh_getDescription(itapsMesh, desc, &itapsError, sizeof(desc));
#elif ITAPS_GRUMMP
#endif
        SNPRINTF(msg, sizeof(msg), "Encountered ITAPS error (%d) \"%s\""
            "\nUnable to open file!", tmpError, desc); 
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
    }
funcEnd: ;
}

// ****************************************************************************
//  Method: avtITAPS_CFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Wed Mar 7 17:15:33 PST 2007
//
//  Modifications:
//    Mark C. Miller, Tue Jun 26 11:27:48 PDT 2007
//    Modified for C interface to ITAPS
//
//    Mark C. Miller, Tue Apr 22 21:45:41 PDT 2008
//    Big changes to support MOAB's subsets
//
// ****************************************************************************

vtkDataSet *
avtITAPS_CFileFormat::GetMesh(int domain, const char *meshname)
{
    int i, j;

    try
    {
        //
        // Get the entity set handle for this domain
        //
        iBase_EntitySetHandle theSet = (void*) -1; // CHECK 
        int setCount = 0;
        int entType = -1;
        theSet = domainSets[domain];
        entType = domainEntType;

        if (theSet == (void*) -1 || entType == -1)
        {
            char msg[256];
            SNPRINTF(msg, sizeof(msg), "Unable to find entity set handle for domain %d", domain);
            EXCEPTION1(ImproperUseException, msg);
        }

        int coords2Size, mapSize;
        double *coords2 = 0; int coords2_allocated = 0;
        int *inEntSetMap = 0; int inEntSetMap_allocated = 0;
        iBase_StorageOrder storageOrder2 = iBase_UNDETERMINED;
        iMesh_getAllVtxCoords(itapsMesh, theSet, &coords2, &coords2_allocated, &coords2Size,
            &inEntSetMap, &inEntSetMap_allocated, &mapSize, (int*) &storageOrder2, &itapsError);
        CheckITAPSError(itapsMesh, (0,coords2,inEntSetMap,EoL));
        int vertCount = mapSize;
        if (vertCount == 0)
        {
            if (coords2_allocated)
                free(coords2);
            if (inEntSetMap_allocated)
                free(inEntSetMap);
            return 0;
        }

        //
        // If its a 1D or 2D mesh, the coords could be 3-tuples where
        // the 'extra' values are always zero or they could be reduced
        // dimensioned tuples. The ITAPS interface doesn't specify.
        //
        int tupleSize = coords2Size / vertCount;

        //
        // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
        //
        vtkPoints *points  = vtkPoints::New();
        points->SetNumberOfPoints(vertCount);
        float *pts = (float *) points->GetVoidPointer(0);
        for (i = 0; i < vertCount; i++)
        {
            if (storageOrder2 == iBase_INTERLEAVED)
            {
                for (j = 0; j < tupleSize; j++)
                    pts[i*3+j] = (float) coords2[i*tupleSize+j];
                for (j = tupleSize; j < 3; j++)
                    pts[i*3+j] = 0.0;
            }
            else if (storageOrder2 == iBase_BLOCKED)
            {
                for (j = 0; j < tupleSize; j++)
                    pts[i*3+j] = (float) coords2[j*vertCount+i];
                for (j = tupleSize; j < 3; j++)
                    pts[i*3+j] = 0.0;
            }
        }
        if (coords2_allocated)
            free(coords2);
        if (inEntSetMap_allocated)
            free(inEntSetMap);

        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New(); 
        ugrid->SetPoints(points);
        points->Delete();

        int *offset = 0; int offsetSize; int offset_allocated = 0;
        int *index = 0; int indexSize; int index_allocated = 0;
        iMesh_EntityTopology *topos = 0; int toposSize; int topos_allocated = 0;
        iMesh_getVtxCoordIndex(itapsMesh, theSet,
            entType, iMesh_ALL_TOPOLOGIES, entType, 
            &offset, &offset_allocated, &offsetSize,
            &index, &index_allocated, &indexSize,
            (int**) &topos, &topos_allocated, &toposSize,
            &itapsError);
        CheckITAPSError(itapsMesh, (0,topos,offset,index,EoL));

        for (int off = 0; off < offsetSize; off++)
        {
            int vtkZoneType = ITAPSEntityTopologyToVTKZoneType(topos[off]);
            if (vtkZoneType == -1)
                continue;
            vtkIdType vertIds[256];
            int jj = 0;
            for (int idx = offset[off];
                 idx < ((off+1) < offsetSize ? offset[off+1] : indexSize); idx++)
                vertIds[jj++] = index[idx];
            ugrid->InsertNextCell(vtkZoneType, jj, vertIds);
        }

        if (topos_allocated)
            free(topos);
        if (offset_allocated)
            free(offset);
        if (index_allocated)
            free(index);

        return ugrid;

    }
    catch (iBase_Error TErr)
    {
        char msg[512];
        char desc[256];
        desc[0] = '\0';
        int tmpError = itapsError;
#ifdef ITAPS_MOAB
        iMesh_getDescription(itapsMesh, desc, &itapsError, sizeof(desc));
#elif ITAPS_GRUMMP
#endif
        SNPRINTF(msg, sizeof(msg), "Encountered ITAPS error (%d) \"%s\""
            "\nUnable to open file!", tmpError, desc); 
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
        return 0;
    }
funcEnd: ;
}


// ****************************************************************************
//  Method: avtITAPS_CFileFormat::GetNodalSubsetVar
//
//  Purpose: Return a node-centered variable indicating subset membership of
//  each node in the assocaited domain (chunk of mesh).
//
//  I would have liked to use iMesh_getEntities and then iMesh_isEntContained.
//  However, I ran into problems where getEntities for entType of iBase_VERTEX
//  did NOT return as many entities as getAllVtxCoords (which returns the
//  coordinate of vertex entities) for same domain set.
//
//  Programmer: Mark C. Miller 
//  Creation:   Tue Apr 22 14:09:11 PDT 2008
//
// ****************************************************************************
vtkDataArray *
avtITAPS_CFileFormat::GetNodalSubsetVar(int domain, const char *varname,
    const vector<iBase_EntitySetHandle> &theSets)
{
    //
    // Look up the mesh in the cache.
    //
    string meshName = metadata->MeshForVar(varname);
    const avtMeshMetaData *mmd = metadata->GetMesh(meshName);
    vtkDataSet *ds = (vtkDataSet *) cache->GetVTKObject(meshName.c_str(),
                                            avtVariableCache::DATASET_NAME,
                                            timestep, domain, "_all");
    if (ds == 0)
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Cannot find cached mesh \"%s\" for domain %d to "
            "paint subset variable \"%s\"", meshName.c_str(), domain, varname);
        EXCEPTION1(ImproperUseException, msg);
    }

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(ds);
    vtkFloatArray *result = 0;
    try {

        int i,j;

        //
        // Build a map from coordinate values to node indices.
        //
        result = vtkFloatArray::New();
        result->SetNumberOfTuples(ugrid->GetPoints()->GetNumberOfPoints());
        float *p = (float *) result->GetVoidPointer(0);
        float *pts = (float *) ugrid->GetPoints()->GetVoidPointer(0);
        map<float, map<float, map<float, int> > > coordToIndexMap;
        for (i = 0; i < ugrid->GetPoints()->GetNumberOfPoints(); i++)
        {
            coordToIndexMap[pts[3*i+0]][pts[3*i+1]][pts[3*i+2]] = i;
            p[i] = -1;
        }

        for (j = 0; j < theSets.size(); j++)
        {
            int coords2Size, mapSize;
            double *coords2 = 0; int coords2_allocated = 0;
            int *inEntSetMap = 0; int inEntSetMap_allocated = 0;
            iBase_StorageOrder storageOrder2 = iBase_UNDETERMINED;
            iMesh_getAllVtxCoords(itapsMesh, theSets[j], &coords2, &coords2_allocated, &coords2Size,
                &inEntSetMap, &inEntSetMap_allocated, &mapSize, (int*) &storageOrder2, &itapsError);
            int vertCount = mapSize;
            if (vertCount == 0)
            {
                if (coords2_allocated)
                    free(coords2);
                if (inEntSetMap_allocated)
                    free(inEntSetMap);
                continue;
            }

            int tupleSize = coords2Size / vertCount;
            for (int k = 0; k < vertCount; k++)
            {
                int l;
                float thePoint[3];
                if (storageOrder2 == iBase_INTERLEAVED)
                {
                    for (l = 0; l < tupleSize; l++)
                        thePoint[l] = (float) coords2[k*tupleSize+l];
                    for (l = tupleSize; l < 3; l++)
                        thePoint[l] = 0.0;
                }
                else if (storageOrder2 == iBase_BLOCKED)
                {
                    for (l = 0; l < tupleSize; l++)
                        thePoint[l] = (float) coords2[l*vertCount+k];
                    for (l = tupleSize; l < 3; l++)
                        thePoint[l] = 0.0;
                }

                int entIndex = -1;
                map<float, map<float, map<float, int> > >::const_iterator itx =
                    coordToIndexMap.find(thePoint[0]);
                if (itx != coordToIndexMap.end())
                {
                    map<float, map<float, int> >::const_iterator ity =
                        itx->second.find(thePoint[1]);
                    if (ity != itx->second.end())
                    {
                        map<float, int>::const_iterator itz =
                            ity->second.find(thePoint[2]);
                        if (itz != ity->second.end())
                        {
                            entIndex = itz->second;
                        }
                    }
                }

                if (entIndex == -1)
                    continue;

                if (p[entIndex] == -1) // haven't assigned this vertex to any set yet
                    p[entIndex] = j;
                else                   // its already been assigned, so add to it
                {
                    double curval = p[entIndex];
                    avtScalarMetaData::UpdateValByInsertingDigit(&curval,
                        theSets.size(), maxCoincidentSets, pascalsTriangleMaps[varname], j);
                    p[entIndex] = float(curval);
                }
            }

            if (coords2_allocated)
                free(coords2);
            if (inEntSetMap_allocated)
                free(inEntSetMap);

        }
    }
    catch (iBase_Error TErr)
    {
        char msg[512];
        char desc[256];
        desc[0] = '\0';
        int tmpError = itapsError;
#ifdef ITAPS_MOAB
        iMesh_getDescription(itapsMesh, desc, &itapsError, sizeof(desc));
#elif ITAPS_GRUMMP
#endif
        SNPRINTF(msg, sizeof(msg), "Encountered ITAPS error (%d) \"%s\""
            "\nUnable to open file!", tmpError, desc); 
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
        return 0;
    }

    return result;
}

// ****************************************************************************
//  Method: avtITAPS_CFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Wed Mar 7 17:15:33 PST 2007
//
//  Modifications:
//    Mark C. Miller, Tue Jun 26 11:27:48 PDT 2007
//    Modified for C interface to ITAPS
//
//    Mark C. Miller, Tue Apr 22 21:45:41 PDT 2008
//    Big changes to support MOAB's subsets
//
// ****************************************************************************

vtkDataArray *
avtITAPS_CFileFormat::GetVar(int domain, const char *varname)
{
    vtkDataArray *result = 0;

    //
    // Check if this is a request for one of the 'special' subsetting variables.
    //
    map<string, vector<iBase_EntitySetHandle> >::iterator tlsit;
    for (tlsit = topLevelSets.begin(); tlsit != topLevelSets.end(); tlsit++)
    {
        string className = tlsit->first == "Vertex" ? "Verticies" : tlsit->first + "s";
        if (className == varname)
            return GetNodalSubsetVar(domain, varname, tlsit->second);
    }

    //
    // Scan through known primitive tags looking for one with the given name
    //
    int entType;
    iBase_TagHandle tagToGet = 0;
    for (entType = 0; entType < 4; entType++)
    {
        vector<void*> tagHandles = primitiveTagHandles[entType];
        for (int tagIdx = 0; tagIdx < tagHandles.size(); tagIdx++)
        {
            iBase_TagHandle theTag = tagHandles[tagIdx]; 
            string tagName = VisIt_iMesh_getTagName(itapsMesh, theTag);
            CheckITAPSError(itapsMesh, NoL);
            int nmsz = tagName.length();
            if (string(tagName, 0, nmsz) == string(varname, 0, nmsz))
            {
                tagToGet = theTag;
                goto tagFound;
            }
        }
    }
    EXCEPTION1(InvalidVariableException, varname);

tagFound:
    try
    {
        //
        // Get the array of entities if we haven't already
        //
        iBase_EntityHandle **entHandles_p = 0;
        int ents_size = 0, *ents_allocated_p = 0;

        switch (entType)
        {
            case iBase_VERTEX:
                entHandles_p = &vertEnts;
                ents_size = numVerts;
                ents_allocated_p = &vertEnts_allocated;
                break;
            case iBase_EDGE:
                entHandles_p = &edgeEnts;
                ents_size = numFaces;
                ents_allocated_p = &edgeEnts_allocated;
                break;
            case iBase_FACE:
                entHandles_p = &faceEnts;
                ents_size = numEdges;
                ents_allocated_p = &faceEnts_allocated;
                break;
            case iBase_REGION:
                entHandles_p = &regnEnts;
                ents_size = numRegns;
                ents_allocated_p = &regnEnts_allocated;
                break;
        }
        if (*entHandles_p == 0)
        {
            iMesh_getEntities(itapsMesh, domainSets[domain], entType,
                iMesh_ALL_TOPOLOGIES, entHandles_p, ents_allocated_p, &ents_size, &itapsError);
            CheckITAPSError(itapsMesh, (0,*entHandles_p,EoL));
            int expectedSize;
            switch (entType)
            {
                case iBase_VERTEX:
                    vertEnts = *entHandles_p; expectedSize = numVerts; break;
                case iBase_EDGE:
                    edgeEnts = *entHandles_p; expectedSize = numEdges; break;
                case iBase_FACE:
                    faceEnts = *entHandles_p; expectedSize = numFaces; break;
                case iBase_REGION:
                    regnEnts = *entHandles_p; expectedSize = numRegns; break;
            }
            if (expectedSize != ents_size)
            {
                char tmpMsg[256];
                SNPRINTF(tmpMsg, sizeof(tmpMsg), "for variable \"%s\" "
                    "getEntities() returned %d entities but VisIt expected %d",
                    varname, ents_size, expectedSize);
                EXCEPTION1(InvalidVariableException, tmpMsg);
            }
        }

        //
        // Now, get the tag data and put it in an appropriate vtk data array 
        //
        int tagSizeValues;
        iMesh_getTagSizeValues(itapsMesh, tagToGet, &tagSizeValues, &itapsError);
        CheckITAPSError(itapsMesh, NoL);
        int tagTypeId;
        iMesh_getTagType(itapsMesh, tagToGet, &tagTypeId, &itapsError);
        CheckITAPSError(itapsMesh, NoL);
        int arraySize;
        switch (tagTypeId)
        {
            case iBase_INTEGER:
            {
                int *intArray; int intArray_allocated = 0;
                iMesh_getIntArrData(itapsMesh, *entHandles_p, ents_size, tagToGet,
                    &intArray, &intArray_allocated, &arraySize, &itapsError); 
                CheckITAPSError(itapsMesh, (0,intArray,EoL)); 
                if (arraySize != ents_size * tagSizeValues)
                {
                    char tmpMsg[256];
                    SNPRINTF(tmpMsg, sizeof(tmpMsg), "getIntArrData() returned %d values "
                        " but VisIt expected %d * %d", arraySize, ents_size, tagSizeValues);
                    EXCEPTION1(InvalidVariableException, tmpMsg);
                }
                vtkIntArray *ia = vtkIntArray::New();
                ia->SetNumberOfComponents(tagSizeValues);
                ia->SetNumberOfTuples(ents_size);
                for (int i = 0; i < arraySize; i++)
                    ia->SetValue(i, intArray[i]);
                result = ia;
                if (intArray_allocated)
                    free(intArray);
                break;
            }
            case iBase_DOUBLE:
            {
                double *dblArray; int dblArray_allocated = 0;
                iMesh_getDblArrData(itapsMesh, *entHandles_p, ents_size, tagToGet,
                    &dblArray, &dblArray_allocated, &arraySize, &itapsError); 
                CheckITAPSError(itapsMesh, (0,dblArray,EoL)); 
                if (arraySize != ents_size * tagSizeValues)
                {
                    char tmpMsg[256];
                    SNPRINTF(tmpMsg, sizeof(tmpMsg), "getDblArrData() returned %d values "
                        " but VisIt expected %d * %d", arraySize, ents_size, tagSizeValues);
                    EXCEPTION1(InvalidVariableException, tmpMsg);
                }
                vtkDoubleArray *da = vtkDoubleArray::New();
                da->SetNumberOfComponents(tagSizeValues);
                da->SetNumberOfTuples(ents_size);
                for (int i = 0; i < arraySize; i++)
                    da->SetValue(i, dblArray[i]);
                result = da;
                if (dblArray_allocated)
                    free(dblArray);
                break;
            }
            case iBase_ENTITY_HANDLE:
            case iBase_BYTES:
            {
                char tmpMsg[256];
                SNPRINTF(tmpMsg, sizeof(tmpMsg), "Unable to handle data type of \"%s\"",
                    itapsDataTypeNames[tagTypeId]);
                EXCEPTION1(InvalidVariableException, tmpMsg);
            }
        }
    }
    catch (iBase_Error TErr)
    {
        char msg[512];
        char desc[256];
        desc[0] = '\0';
        int tmpError = itapsError;
#ifdef ITAPS_MOAB
        iMesh_getDescription(itapsMesh, desc, &itapsError, sizeof(desc));
#elif ITAPS_GRUMMP
#endif
        SNPRINTF(msg, sizeof(msg), "Encountered ITAPS error (%d) \"%s\""
            "\nUnable to open file!", tmpError, desc); 
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
        return 0;
    }

funcEnd:
    return result;
}


// ****************************************************************************
//  Method: avtITAPS_CFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Wed Mar 7 17:15:33 PST 2007
//
// ****************************************************************************

vtkDataArray *
avtITAPS_CFileFormat::GetVectorVar(int domain, const char *varname)
{
    return GetVar(domain, varname);
}
