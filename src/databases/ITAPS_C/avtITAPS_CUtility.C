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

#include "iBase.h"
#include "iMesh.h"

#include <snprintf.h>
#include <map>
#include <string>
#include <vector>
#include <stdarg.h>

#include <avtITAPS_CUtility.h>
#include <avtCallback.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <vtkCellType.h>

using     std::map;
using     std::string;
using     std::vector;

map<string, int> avtITAPS_CUtility::messageCounts;
int avtITAPS_CUtility::itapsError;
char** avtITAPS_CUtility::entTypes = 0;
char** avtITAPS_CUtility::entTopologies = 0;
char** avtITAPS_CUtility::itapsDataTypeNames = 0;
const char *avtITAPS_CUtility::supressMessage = "further warnings regarding this error will be supressed";

// supports passing variable length list of void pointers that if non-zero
// need to be free'd.
void
avtITAPS_CUtility::ITAPSErrorCleanupHelper(int dummy, ...)
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


using namespace avtITAPS_CUtility;

void
avtITAPS_CUtility::InitDataTypeNames()
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

string
avtITAPS_CUtility::VisIt_iMesh_getTagName(iMesh_Instance theMesh, iBase_TagHandle theTag)
{
    static char tmpName[256];
#ifdef ITAPS_MOAB
    iMesh_getTagName(theMesh, theTag, tmpName, &itapsError, sizeof(tmpName));
#elif ITAPS_GRUMMP
    iMesh_getTagName(theMesh, theTag, tmpName, sizeof(tmpName), &itapsError);
#endif
    return string(tmpName);
}

int avtITAPS_CUtility::ITAPSEntityTopologyToVTKZoneType(int ttype)
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

void
avtITAPS_CUtility::TraverseSetHierarchy(iMesh_Instance aMesh, int level,
    int memberId, bool isEntitySet, iBase_EntitySetHandle esh, bool debugOff,
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
        CheckITAPSError(aMesh, iMesh_getEntType, NoL);
        debug5 << ident << "    type = \"" << entTypes[type] << "\"" << endl;
        iMesh_getEntTopo(aMesh, esh, &topo, &itapsError);
        CheckITAPSError(aMesh, iMesh_getEntTopo, NoL);
        debug5 << ident << "    topology = \"" << entTopologies[topo] << "\"" << endl;
    }

    if (isEntitySet)
        iMesh_getAllEntSetTags(aMesh, esh, &tags, &tags_allocated, &tags_size, &itapsError);
    else
        iMesh_getAllTags(aMesh, esh, &tags, &tags_allocated, &tags_size, &itapsError);
    CheckITAPSError(aMesh, iMesh_getAllTags, (0,tags,EoL));
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
            CheckITAPSError(aMesh, iMesh_getTagType, NoL);
            int tagSize;
            iMesh_getTagSizeValues(aMesh, tags[t], &tagSize, &itapsError);
            CheckITAPSError(aMesh, iMesh_getTagSizeValues, NoL); 
            string tagName = VisIt_iMesh_getTagName(aMesh, tags[t]);
            CheckITAPSError(aMesh, VisIt_iMesh_getTagName, NoL); 
            if (typeId == iBase_INTEGER)
            {
                if (tagSize == 1)
                {
                    int theVal;
                    if (isEntitySet)
                        iMesh_getEntSetIntData(aMesh, esh, tags[t], &theVal, &itapsError);
                    else
                        iMesh_getIntData(aMesh, esh, tags[t], &theVal, &itapsError);
                    CheckITAPSError(aMesh, iMesh_getXXIntData, NoL); 
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
                    CheckITAPSError(aMesh, iMesh_getIntArrData, (0,tagvals,EoL)); 
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
                    CheckITAPSError(aMesh, iMesh_getXXDblData, NoL); 
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
                    CheckITAPSError(aMesh, iMesh_getDblArrData, (0,tagvals,EoL)); 
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
                CheckITAPSError(aMesh, iMesh_getXXData, (0,theVal,EoL));
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
                    CheckITAPSError(aMesh, iMesh_getXXEHData, NoL); 
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
                    CheckITAPSError(aMesh, iMesh_getEHArrData, (0,tagvals,EoL)); 
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
    CheckITAPSError(aMesh, iMesh_getEntSets, (0,sets,EoL));
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
    CheckITAPSError(aMesh, iMesh_getEntities, (0,ents,EoL));
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

void
avtITAPS_CUtility::GetTagsForEntity(iMesh_Instance aMesh, bool isEntitySet,
    iBase_EntitySetHandle esh, vector<string> &tagNames, vector<int> &tagTypes,
    vector<int> &tagSizes, vector<string> &tagVals)
{
    iBase_TagHandle *tags = 0; int tags_allocated = 0; int tags_size = 0;

    if (isEntitySet)
        iMesh_getAllEntSetTags(aMesh, esh, &tags, &tags_allocated, &tags_size, &itapsError);
    else
        iMesh_getAllTags(aMesh, esh, &tags, &tags_allocated, &tags_size, &itapsError);
    CheckITAPSError(aMesh, iMesh_getAllXXTags, (0,tags,EoL));

    if (tags_size)
    {
        for (int t = 0; t < tags_size; t++)
        {
            char lineBuf[256];

            int typeId;
            iMesh_getTagType(aMesh, tags[t], (int*) &typeId, &itapsError);
            CheckITAPSError(aMesh, iMesh_getTagType, NoL);
            tagTypes.push_back(typeId);

            int tagSize;
            iMesh_getTagSizeValues(aMesh, tags[t], &tagSize, &itapsError);
            CheckITAPSError(aMesh, iMesh_getTagSizeValues, NoL); 
            tagSizes.push_back(tagSize);

            string tagName = VisIt_iMesh_getTagName(aMesh, tags[t]);
            CheckITAPSError(aMesh, VisIt_iMesh_getTagName, NoL); 
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
                    CheckITAPSError(aMesh, iMesh_getXXIntData, NoL); 
                    SNPRINTF(lineBuf, sizeof(lineBuf), "%d", theVal);
                }
                else if (tagSize > 1)
                {
                    int *tagvals = 0; int tagvals_allocated = 0; int tagvals_size = 0;
                    iMesh_getIntArrData(aMesh, &esh, 1, tags[t],
                        &tagvals, &tagvals_allocated, &tagvals_size, &itapsError);
                    CheckITAPSError(aMesh, iMesh_getIntArrData, (0,tagvals,EoL)); 
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
                    CheckITAPSError(aMesh, iMesh_getXXDblData, NoL); 
                    SNPRINTF(lineBuf, sizeof(lineBuf), "%f", theVal);
                }
                else if (tagSize > 1)
                {
                    double *tagvals = 0; int tagvals_allocated = 0; int tagvals_size = 0;
                    iMesh_getDblArrData(aMesh, &esh, 1, tags[t],
                        &tagvals, &tagvals_allocated, &tagvals_size, &itapsError);
                    CheckITAPSError(aMesh, iMesh_getDblArrData, (0,tagvals,EoL)); 
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
                CheckITAPSError(aMesh, iMesh_getXXData, (0,theVal,EoL));
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
                    CheckITAPSError(aMesh, iMesh_getXXEHData, NoL); 
                    SNPRINTF(lineBuf, sizeof(lineBuf), "%X", theVal);
                }
                else if (tagSize > 1)
                {
                    iBase_EntitySetHandle *tagvals = 0; int tagvals_allocated = 0; int tagvals_size = 0;
                    iMesh_getEHArrData(aMesh, &esh, 1, tags[t],
                        &tagvals, &tagvals_allocated, &tagvals_size, &itapsError);
                    CheckITAPSError(aMesh, iMesh_getEHArrData, (0,tagvals,EoL)); 
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

bool 
avtITAPS_CUtility::GetTopLevelSets(iMesh_Instance ima, int level, int memidx,
    bool ises, iBase_EntitySetHandle esh, void *cb_data)
{
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

    if (level > 0)
        return false;
    return true;
}
