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
    iMesh_getTagName(theMesh, theTag, tmpName, &itapsError, sizeof(tmpName));
    return string(tmpName);
}

int
avtITAPS_CUtility::VTKZoneTypeToITAPSEntityTopology(int vtk_zonetype)
{
    // Note that there is no value for 'UNKNOWN' or 'NOT SET'
    int imesh_zonetype = -1; 
    switch (vtk_zonetype)
    {
        // 0D
        case VTK_VERTEX:     imesh_zonetype = iMesh_POINT;         break;

        // 1D
        case VTK_LINE:       imesh_zonetype = iMesh_LINE_SEGMENT;  break;

        // 2D
        case VTK_POLYGON:    imesh_zonetype = iMesh_POLYGON;       break;
        case VTK_TRIANGLE:   imesh_zonetype = iMesh_TRIANGLE;      break;
        case VTK_QUAD:       imesh_zonetype = iMesh_QUADRILATERAL; break;

        // 3D
        case VTK_TETRA:      imesh_zonetype = iMesh_TETRAHEDRON;   break;
        case VTK_PYRAMID:    imesh_zonetype = iMesh_PYRAMID;       break;
        case VTK_WEDGE:      imesh_zonetype = iMesh_PRISM;         break;
        case VTK_VOXEL:
        case VTK_HEXAHEDRON: imesh_zonetype = iMesh_HEXAHEDRON;    break;
    }
    return imesh_zonetype;
}

// 
//    Mark C. Miller, Wed Jan 14 17:56:30 PST 2009
//    Removed extraneous case for VTK_VOXEL, the result of a
//    bad cut-n-paste.
//
int avtITAPS_CUtility::ITAPSEntityTopologyToVTKZoneType(int ttype)
{
    switch (ttype)
    {
    // 0D
    case iMesh_POINT:         return VTK_VERTEX;

    // 1D
    case iMesh_LINE_SEGMENT:  return VTK_LINE;

    // 2D
    case iMesh_POLYGON:       return VTK_POLYGON;
    case iMesh_TRIANGLE:      return VTK_TRIANGLE;
    case iMesh_QUADRILATERAL: return VTK_QUAD;

    // 3D
    case iMesh_TETRAHEDRON:   return VTK_TETRA;
    case iMesh_PYRAMID:       return VTK_PYRAMID;
    case iMesh_PRISM:         return VTK_WEDGE;
    case iMesh_HEXAHEDRON:    return VTK_HEXAHEDRON;
    }
    return -1;
}

//
// We re-define debug4/5 here because we use TraverseSetHierarchy for multiple
// purposes and don't want data spewed to debug logs every time we traverse
// the set hierarchy even when debug logs are enabled.
//
#undef debug5
#define debug5 if (debugOff || !(DebugStream::Level5())) ; else (DebugStream::Stream5())
#undef debug4
#define debug4 if (debugOff || !(DebugStream::Level4())) ; else (DebugStream::Stream4())

// ****************************************************************************
//  Function: TraverseSetHierarchy 
//
//  Purpose: Very verbose description of the set hierarchy. Only used for
//  debugging.
//
//  Modifications:
//    Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
//    Added code to output entity type and topology
//
//    Mark C. Miller, Tue Apr 21 15:54:20 PDT 2009
//    Broad changes to better manage debug output to different level logs.
//    Needed to add additional arg for EntityHandle as distinct from
//    EntitySetHandle to confirm to new iMesh/iBase specification.
//
//    Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//    Changed interface to DebugStream to obtain current debug level.
// ****************************************************************************

void
avtITAPS_CUtility::TraverseSetHierarchy(iMesh_Instance aMesh, int level,
    int memberId, bool isEntitySet, iBase_EntityHandle eh, iBase_EntitySetHandle esh, bool debugOff,
    HandleThisSet handleSetCb, void *handleSetCb_data)
{
#ifndef MDSERVER
    // If we're not on the MDSERVER, we'll never output debugging stuff
    debugOff = true;
#endif

    // Do the callback if one was requested
    bool shouldRecurse = true;
    if (isEntitySet && handleSetCb)
        shouldRecurse = (*handleSetCb)(aMesh, level, memberId, isEntitySet, eh, esh, handleSetCb_data);

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
    {
        debug4 << ident << "For Entity Set at level " << level << ", index " << memberId << ":" << endl;
        debug4 << ident << "{" << endl;
    }
    else
    {
        debug5 << ident << "For Entity     at level " << level << ", index " << memberId << ":" << endl;
        debug5 << ident << "{" << endl;
    }

    if (!isEntitySet)
    {
        int topo, type;
        iMesh_getEntType(aMesh, eh, &type, &itapsError);
        if (level<2) CheckITAPSError(aMesh, iMesh_getEntType, NoL);
        debug5 << ident << "    type = \"" << entTypes[type] << "\"" << endl;
        iMesh_getEntTopo(aMesh, eh, &topo, &itapsError);
        if (level<2) CheckITAPSError(aMesh, iMesh_getEntTopo, NoL);
        debug5 << ident << "    topology = \"" << entTopologies[topo] << "\"" << endl;
    }

    if (isEntitySet)
        iMesh_getAllEntSetTags(aMesh, esh, &tags, &tags_allocated, &tags_size, &itapsError);
    else
        iMesh_getAllTags(aMesh, eh, &tags, &tags_allocated, &tags_size, &itapsError);
    if (level<2) CheckITAPSError(aMesh, iMesh_getAllTags, (0,tags,EoL));
    if (tags_size)
    {
        if (isEntitySet)
        {
            debug4 << ident << "    tags = " << tags_size << endl;
            debug4 << ident << "    {" << endl;
            debug4 << ident << "            name                 type     size     value(s)..." << endl;
            debug4 << ident << "     --------------------       -------   ----     -----------" << endl;
        }
        else
        {
            debug5 << ident << "    tags = " << tags_size << endl;
            debug5 << ident << "    {" << endl;
            debug5 << ident << "            name                 type     size     value(s)..." << endl;
            debug5 << ident << "     --------------------       -------   ----     -----------" << endl;
        }
        for (int t = 0; t < tags_size; t++)
        {
            char lineBuf[256];
            int typeId;
            iMesh_getTagType(aMesh, tags[t], (int*) &typeId, &itapsError);
            if (level<2) CheckITAPSError(aMesh, iMesh_getTagType, NoL);
            int tagSize;
            iMesh_getTagSizeValues(aMesh, tags[t], &tagSize, &itapsError);
            if (level<2) CheckITAPSError(aMesh, iMesh_getTagSizeValues, NoL); 
            string tagName = VisIt_iMesh_getTagName(aMesh, tags[t]);
            if (level<2) CheckITAPSError(aMesh, VisIt_iMesh_getTagName, NoL); 
            if (typeId == iBase_INTEGER)
            {
                if (tagSize == 1)
                {
                    int theVal;
                    if (isEntitySet)
                        iMesh_getEntSetIntData(aMesh, esh, tags[t], &theVal, &itapsError);
                    else
                        iMesh_getIntData(aMesh, eh, tags[t], &theVal, &itapsError);
                    if (level<2) CheckITAPSError(aMesh, iMesh_getXXIntData, NoL); 
                    if (DebugStream::Level4())
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %d", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, theVal);
                    }
                }
                else if (tagSize > 1)
                {
                    int *tagvals = (int *) malloc(tagSize*sizeof(int));
                    if (isEntitySet)
                        iMesh_getEntSetIntData(aMesh, esh, tags[t], tagvals, &itapsError);
                    else
                        iMesh_getIntData(aMesh, eh, tags[t], tagvals, &itapsError);
                    if (level<2) CheckITAPSError(aMesh, iMesh_getIntData, (0,tagvals,EoL)); 
                    std::string valBuf;
                    for (int k = 0; k < tagSize; k++)
                    {
                        char tmpChars[32];
                        sprintf(tmpChars, "%d ", tagvals[k]);
                        valBuf += std::string(tmpChars);
                    }
                    free(tagvals);
                    if (DebugStream::Level4())
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %s", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, valBuf.c_str());
                    }
                }
                else
                {
                    if (DebugStream::Level4())
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
                        iMesh_getDblData(aMesh, eh, tags[t], &theVal, &itapsError);
                    if (level<2) CheckITAPSError(aMesh, iMesh_getXXDblData, NoL); 
                    if (DebugStream::Level4())
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %f", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, theVal);
                    }
                }
                else if (tagSize > 1)
                {
                    double *tagvals = (double *) malloc(tagSize*sizeof(double)); 
                    if (isEntitySet)
                        iMesh_getEntSetDblData(aMesh, esh, tags[t], tagvals, &itapsError);
                    else
                        iMesh_getDblData(aMesh, eh, tags[t], tagvals, &itapsError);
                    if (level<2) CheckITAPSError(aMesh, iMesh_getDblData, (0,tagvals,EoL)); 
                    std::string valBuf;
                    for (int k = 0; k < tagSize; k++)
                    {
                        char tmpChars[32];
                        sprintf(tmpChars, "%f ", tagvals[k]);
                        valBuf += std::string(tmpChars);
                    }
                    free(tagvals);
                    if (DebugStream::Level4())
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %s", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, valBuf.c_str());
                    }
                }
                else
                {
                    if (DebugStream::Level4())
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
                    iMesh_getData(aMesh, eh, tags[t], &theVal, &theVal_allocated, &theValSize, &itapsError);
                if (level<2) CheckITAPSError(aMesh, iMesh_getXXData, (0,theVal,EoL));
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
                if (DebugStream::Level4())
                {
                    SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %s", 
                        tagName.c_str(), itapsDataTypeNames[typeId], tagSize, valBuf.c_str());
                }
            }
            else if (typeId == iBase_ENTITY_HANDLE)
            {
                if (tagSize == 1)
                {
                    iBase_EntityHandle theVal;
                    if (isEntitySet)
                        iMesh_getEntSetEHData(aMesh, esh, tags[t], &theVal, &itapsError);
                    else
                        iMesh_getEHData(aMesh, eh, tags[t], &theVal, &itapsError);
                    if (level<2) CheckITAPSError(aMesh, iMesh_getXXEHData, NoL); 
                    if (DebugStream::Level4())
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %X", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, theVal, &itapsError);
                    }
                }
                else if (tagSize > 1)
                {
                    iBase_EntityHandle *tagvals = (iBase_EntityHandle*) malloc(tagSize*sizeof(iBase_EntityHandle)); 
                    if (isEntitySet)
                        iMesh_getEntSetEHData(aMesh, esh, tags[t], tagvals, &itapsError);
                    else
                        iMesh_getEHData(aMesh, eh, tags[t], tagvals, &itapsError);
                    if (level<2) CheckITAPSError(aMesh, iMesh_getEHData, (0,tagvals,EoL)); 
                    std::string valBuf;
                    for (int k = 0; k < tagSize; k++)
                    {
                        char tmpChars[32];
                        sprintf(tmpChars, "%X ", tagvals[k]);
                        valBuf += std::string(tmpChars);
                    }
                    free(tagvals);
                    if (DebugStream::Level4())
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %s", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, valBuf.c_str());
                    }
                }
                else
                {
                    if (DebugStream::Level4())
                    {
                        SNPRINTF(lineBuf, sizeof(lineBuf), "% 16s     % 8s     %03d     %s", 
                            tagName.c_str(), itapsDataTypeNames[typeId], tagSize, "invalid size");
                    }
                }
            }
            if (isEntitySet)
                debug4 << ident << "         " << lineBuf << endl;
            else
                debug5 << ident << "         " << lineBuf << endl;
        }
        if (isEntitySet)
            debug4 << ident << "    }" << endl;
        else
            debug5 << ident << "    }" << endl;
    }
    else
    {
        if (isEntitySet)
            debug4 << ident << "    tags = NONE" << endl;
        else
            debug5 << ident << "    tags = NONE" << endl;
    }

    if (isEntitySet)
        debug4 << ident << "}" << endl;
    else
    {
        debug5 << ident << "}" << endl;
        return;
    }

    iMesh_getEntSets(aMesh, esh, 1, &sets, &sets_allocated, &sets_size, &itapsError);
    if (level<2) CheckITAPSError(aMesh, iMesh_getEntSets, (0,sets,EoL));
    if (sets_size > 0 && shouldRecurse)
    {
        debug4 << ident << "    entity sets = " << sets_size << endl;
        debug4 << ident << "    {" << endl;
        for (int i = 0; i < sets_size; i++)
            TraverseSetHierarchy(aMesh, level+1, i, true, eh, sets[i], debugOff,
                handleSetCb, handleSetCb_data);
        debug4 << ident << "    }" << endl;
    }
    else
    {
        debug4 << ident << "    entity sets = NONE" << endl;
    }

    iMesh_getEntities(aMesh, esh, iBase_ALL_TYPES,
        iMesh_ALL_TOPOLOGIES, &ents, &ents_allocated, &ents_size, &itapsError);
    if (level<2) CheckITAPSError(aMesh, iMesh_getEntities, (0,ents,EoL));
    if (ents_size > 0 && shouldRecurse)
    {
        debug5 << ident << "    entities = " << ents_size << endl;
        debug5 << ident << "    {" << endl;
        for (int i = 0; i < ents_size; i++)
            TraverseSetHierarchy(aMesh, level+1, i, false, ents[i], esh, DebugStream::Level5() || debugOff,
                handleSetCb, handleSetCb_data);
        debug5 << ident << "    }" << endl;
    }
    else
    {
        debug5 << ident << "    entities = NONE" << endl;
    }

    if (isEntitySet)
        debug4 << ident << "}" << endl;
    else
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
#undef debug4
#define debug5 if (!(DebugStream::Level5())) ; else (DebugStream::Stream5())
#define debug4 if (!(DebugStream::Level4())) ; else (DebugStream::Stream4())


void
avtITAPS_CUtility::GetTagsForEntity(iMesh_Instance aMesh, bool isEntitySet,
    iBase_EntityHandle eh, iBase_EntitySetHandle esh, vector<string> &tagNames, vector<int> &tagTypes,
    vector<int> &tagSizes, vector<string> &tagVals, int level)
{
    iBase_TagHandle *tags = 0; int tags_allocated = 0; int tags_size = 0;

    if (isEntitySet)
        iMesh_getAllEntSetTags(aMesh, esh, &tags, &tags_allocated, &tags_size, &itapsError);
    else
        iMesh_getAllTags(aMesh, eh, &tags, &tags_allocated, &tags_size, &itapsError);
    if (level<2) CheckITAPSError(aMesh, iMesh_getAllXXTags, (0,tags,EoL));

    if (tags_size)
    {
        for (int t = 0; t < tags_size; t++)
        {
            char lineBuf[256];

            int typeId;
            iMesh_getTagType(aMesh, tags[t], (int*) &typeId, &itapsError);
            if (level<2) CheckITAPSError(aMesh, iMesh_getTagType, NoL);
            tagTypes.push_back(typeId);

            int tagSize;
            iMesh_getTagSizeValues(aMesh, tags[t], &tagSize, &itapsError);
            if (level<2) CheckITAPSError(aMesh, iMesh_getTagSizeValues, NoL); 
            tagSizes.push_back(tagSize);

            string tagName = VisIt_iMesh_getTagName(aMesh, tags[t]);
            if (level<2) CheckITAPSError(aMesh, VisIt_iMesh_getTagName, NoL); 
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
                        iMesh_getIntData(aMesh, eh, tags[t], &theVal, &itapsError);
                    if (level<2) CheckITAPSError(aMesh, iMesh_getXXIntData, NoL); 
                    SNPRINTF(lineBuf, sizeof(lineBuf), "%d", theVal);
                }
                else if (tagSize > 1)
                {
                    int *tagvals = (int *) malloc(tagSize*sizeof(int)); 
                    if (isEntitySet)
                        iMesh_getEntSetIntData(aMesh, esh, tags[t], tagvals, &itapsError);
                    else
                        iMesh_getIntData(aMesh, eh, tags[t], tagvals, &itapsError);
                    if (level<2) CheckITAPSError(aMesh, iMesh_getIntData, (0,tagvals,EoL)); 
                    std::string valBuf;
                    for (int k = 0; k < tagSize; k++)
                    {
                        char tmpChars[32];
                        sprintf(tmpChars, "%d ", tagvals[k]);
                        valBuf += std::string(tmpChars);
                    }
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
                        iMesh_getDblData(aMesh, eh, tags[t], &theVal, &itapsError);
                    if (level<2) CheckITAPSError(aMesh, iMesh_getXXDblData, NoL); 
                    SNPRINTF(lineBuf, sizeof(lineBuf), "%f", theVal);
                }
                else if (tagSize > 1)
                {
                    double *tagvals = (double*) malloc(tagSize*sizeof(double));
                    if (isEntitySet)
                        iMesh_getEntSetDblData(aMesh, esh, tags[t], tagvals, &itapsError);
                    else
                        iMesh_getDblData(aMesh, eh, tags[t], tagvals, &itapsError);
                    if (level<2) CheckITAPSError(aMesh, iMesh_getDblData, (0,tagvals,EoL)); 
                    std::string valBuf;
                    for (int k = 0; k < tagSize; k++)
                    {
                        char tmpChars[32];
                        sprintf(tmpChars, "%f ", tagvals[k]);
                        valBuf += std::string(tmpChars);
                    }
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
                    iMesh_getData(aMesh, eh, tags[t], &theVal, &theVal_allocated, &theValSize, &itapsError);
                if (level<2) CheckITAPSError(aMesh, iMesh_getXXData, (0,theVal,EoL));
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
                    iBase_EntityHandle theVal;
                    if (isEntitySet)
                        iMesh_getEntSetEHData(aMesh, esh, tags[t], &theVal, &itapsError);
                    else
                        iMesh_getEHData(aMesh, eh, tags[t], &theVal, &itapsError);
                    if (level<2) CheckITAPSError(aMesh, iMesh_getXXEHData, NoL); 
                    SNPRINTF(lineBuf, sizeof(lineBuf), "%X", theVal);
                }
                else if (tagSize > 1)
                {
                    iBase_EntityHandle *tagvals = (iBase_EntityHandle*) malloc(tagSize*sizeof(iBase_EntityHandle));
                    if (isEntitySet)
                        iMesh_getEntSetEHData(aMesh, esh, tags[t], tagvals, &itapsError);
                    else
                        iMesh_getEHData(aMesh, eh, tags[t], tagvals, &itapsError);
                    if (level<2) CheckITAPSError(aMesh, iMesh_getEHData, (0,tagvals,EoL)); 
                    std::string valBuf;
                    for (int k = 0; k < tagSize; k++)
                    {
                        char tmpChars[32];
                        sprintf(tmpChars, "%X ", tagvals[k]);
                        valBuf += std::string(tmpChars);
                    }
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


// ****************************************************************************
//  Function: GetTopLevelSets 
//
//  Purpose: Filter function used in traversing set hierarchy. Return
//  sets meeting certain criteria at the top of the hierarchy. 
//
//  Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
// ****************************************************************************
bool 
avtITAPS_CUtility::GetTopLevelSets(iMesh_Instance ima, int level, int memidx,
    bool ises, iBase_EntityHandle eh, iBase_EntitySetHandle esh, void *cb_data)
{
    vector<string> tagNames;
    vector<int>    tagTypes;
    vector<int>    tagSizes;
    vector<string> tagVals;

    if (!ises) return false;

    map<string, vector<iBase_EntitySetHandle> > *theSets = 
        (map<string, vector<iBase_EntitySetHandle> > *) cb_data;

    GetTagsForEntity(ima, ises, eh, esh, tagNames, tagTypes, tagSizes, tagVals, level);

    for (int i = 0; i < tagNames.size(); i++)
    {
        if (tagNames[i] == "CATEGORY")
            (*theSets)[tagVals[i]].push_back(esh);
        else if (tagNames[i] == "PARALLEL_PARTITION" && level == 1)
            (*theSets)[tagNames[i]].push_back(esh);
    }

    if (level > 0)
        return false;
    return true;
}

// ****************************************************************************
//  Function: GetTagStuff 
//
//  Purpose: Utility function to get tag info on an entity set or entity
//  but will presently NOT return tag values except on entity set.
//
//  Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
// ****************************************************************************
int avtITAPS_CUtility::GetTagStuff(iMesh_Instance ima, iBase_EntitySetHandle set,
    int ent_type, string name, int *type, int *size, void **vals)
{
    int err;
    IMESH_ADEF(iBase_TagHandle, tags);

    //
    // Get all tags either on the EntSet or on the first Ent of specified type
    // in the set.
    //
    if (0 <= ent_type && ent_type < iBase_ALL_TYPES)
    {
        int has_data;
        iBase_EntityHandle oneEnt;
        iMesh_EntityIterator entIt;

        iMesh_initEntIter(ima, set, ent_type, iMesh_ALL_TOPOLOGIES, &entIt, &err);
        iMesh_getNextEntIter(ima, entIt, &oneEnt, &has_data, &err);
        if (has_data && err == iBase_SUCCESS)
            iMesh_getAllTags(ima, oneEnt, IMESH_AARG(tags), &err); 
        iMesh_endEntIter(ima, entIt, &err);
    }
    else
    {
        iMesh_getAllEntSetTags(ima, set, IMESH_AARG(tags), &err);
    }
    if (err != iBase_SUCCESS)
        return err;

    for (int i = 0; i < tags_size; i++)
    {
        if (VisIt_iMesh_getTagName(ima, tags[i]) == name)
        {
            if (type)
                iMesh_getTagType(ima, tags[i], type, &err);
            if (err != iBase_SUCCESS)
                return err;
            if (size)
                iMesh_getTagSizeValues(ima, tags[i], size, &err);
            if (err != iBase_SUCCESS)
                return err;
            if (vals)
            {
                // We don't support getting vals on entities here yet
                if (0 <= ent_type && ent_type < iBase_ALL_TYPES)
                    return iBase_FAILURE;

                int tmp_type = type ? *type : 0;
                int tmp_size = size ? *size : 0;
                if (!type)
                    iMesh_getTagType(ima, tags[i], &tmp_type, &err);
                if (!size)
                    iMesh_getTagSizeValues(ima, tags[i], &tmp_size, &err);
                switch(tmp_type)
                {
                    case iBase_BYTES:
                    {
                        int vals_allocated = 0, vals_size = 0;
                        if (*vals == 0)
                        {
                            *vals = (void *) malloc(tmp_size * 1);
                            vals_allocated = tmp_size;
                        }
                        iMesh_getEntSetData(ima, set, tags[i], (char**) vals, &vals_allocated, &vals_size, &err);
                        break;
                    }
                    case iBase_INTEGER:
                        if (*vals == 0)
                            *vals = (void *) malloc(tmp_size * sizeof(int));
                        iMesh_getEntSetIntData(ima, set, tags[i], (int*) *vals, &err);
                        break;
                    case iBase_DOUBLE:
                        if (*vals == 0)
                            *vals = (void *) malloc(tmp_size * sizeof(double));
                        iMesh_getEntSetDblData(ima, set, tags[i], (double*) *vals, &err);
                        break;
                    case iBase_ENTITY_HANDLE:
                    {
                        int vals_allocated = 0, vals_size = 0;
                        if (*vals == 0)
                        {
                            *vals = (void *) malloc(tmp_size * sizeof(iBase_EntityHandle));
                            vals_allocated = tmp_size * sizeof(iBase_EntityHandle);
                        }
                        iMesh_getEntSetData(ima, set, tags[i], (char**) vals, &vals_allocated, &vals_size, &err);
                        //iMesh_getEntSetEHData(ima, set, tags[i], *vals, &err);
                        break;
                    }
                }
            }

            IMESH_AFREE(tags);
            return err;

        }
    }

    IMESH_AFREE(tags);
    return iBase_FAILURE;
}
