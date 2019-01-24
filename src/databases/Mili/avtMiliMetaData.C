/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                             avtMiliMetaData.C                             //
// ************************************************************************* //

#include <avtMiliMetaData.h>
#include <visitstream.h>

#include <ImproperUseException.h>
#include <DebugStream.h>

using std::ifstream;
using std::endl;
using std::cerr;


// ***************************************************************************
//  constructor: MiliVariableMetaData::MiliVariableMetaData
//
//  Purpose:
//      Initialized the MiliVariableMetaData. 
//
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliVariableMetaData::MiliVariableMetaData()
{
    longName          = "";
    shortName         = ""; 
    classLName        = "";
    classSName        = "";
    path              = "";
    cellTypeAvt       = -1;
    cellTypeMili      = -1;
    centering         = AVT_NODECENT;;
    componentDims     = -1;
    vectorSize        = -1;
    meshAssociation   = -1;
    numType           = M_FLOAT;
    isElementSet      = false;
    isMatVar          = false;
    isGlobal          = false;
}


// ***************************************************************************
//  Destructor: MiliVariableMetaData::~MiliVariableMetaData
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************
MiliVariableMetaData::~MiliVariableMetaData()
{
}


// ***************************************************************************
//  Method: MiliVariableMetaData::GetVectorComponent
//
//  Purpose:
//     Get the component of a vector variable.  
//
//  Arguments: 
//      idx    The index of the vector component. 
//
//  Returns:
//      If the index is valid, the variable name of the
//      component is returned as a string. 
//      Otherwise, an empty string is returned. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************
std::string
MiliVariableMetaData::GetVectorComponent(int idx)
{
    if (idx >= 0 && idx < vectorComponents.size())
        return vectorComponents[idx];
    return "";
}


// ***************************************************************************
//  Method: MiliVariableMetaData::InitSRcontainers
//
//  Purpose:
//      Initialize the subrecord containers. 
//
//  Arguments: 
//      numDomains    The number of domains. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
MiliVariableMetaData::InitSRContainers(int numDomains)
{
    subrecInfo.resize(numDomains);
}


// ***************************************************************************
//  Method: MiliVariableMetaData::GetSubrecordIds
//
//  Purpose:
//      Get the subrecord ids for a given domain. 
//
//  Arguments: 
//
//  Returns:
//      A reference to the subrecord id vector 
//      for the given domain. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

vector<int> &
MiliVariableMetaData::GetSubrecordIds(int dom)
{
    if (dom > subrecInfo[dom].nSR || dom < 0)
    {
        char msg[1024];
        sprintf(msg, "Invalid domain index for subrecord ids!\n");
        EXCEPTION1(ImproperUseException, msg);
    }
    return subrecInfo[dom].SRIDs;
}


// ***************************************************************************
//  Method: MiliVariableMetaData::AddSubrecordInfo
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
MiliVariableMetaData::AddSubrecordInfo(int dom, 
                                       int srId,
                                       int nElems,
                                       int nDB,
                                       int *DBRanges)
{
    subrecInfo[dom].nSR++;
    subrecInfo[dom].nElements.push_back(nElems);
    subrecInfo[dom].SRIDs.push_back(srId);
    subrecInfo[dom].nDataBlocks.push_back(nDB);

    //
    // Deep copy the ranges so that we don't have to keep 
    // the subrecords in memory. 
    //
    int limit = nDB * 2;
    vector<int> nxtRange(limit);
    for (int i = 0; i < limit; ++i)
    {
        nxtRange[i] = DBRanges[i]; 
    }

    subrecInfo[dom].dataBlockRanges.push_back(nxtRange);
}


// ***************************************************************************
//  Method: MiliVariableMetaData::GetSubrecordInfo
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

SubrecInfo &
MiliVariableMetaData::GetSubrecordInfo(int dom)
{
    if (dom > subrecInfo.size() || dom < 0)
    {
        char msg[1024];
        sprintf(msg, "Invalid domain index for subrecord ids!\n");
        EXCEPTION1(ImproperUseException, msg);
    }
    return subrecInfo[dom];
}


// ***************************************************************************
//  Method: MiliVariableMetaData::GetPath
//
//  Purpose:
//      Get the visit path for a variable. 
//
//  Arguments: 
//
//  Returns:
//      The path as a string. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

string
MiliVariableMetaData::GetPath()
{
    //TODO: what we want is "classLName (classSName)/shortName"
    //      but visit seems to garble this...

    if (path.empty())
    {
        path = "Primal/";
        if (!classSName.empty())
        {
            path += classSName + "/";
        }
        path += shortName;
    }
    return path;
}


// ***************************************************************************
//  Constructor: MiliClassMetaData::MiliClassMetaData
//
//  Purpose:
//      Initialize the MiliClassMetaData. 
//
//  Arguments: 
//      numDomains    The number of domains. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliClassMetaData::MiliClassMetaData(int numDomains)
{
    longName           = "";
    shortName          = "";
    totalNumElements   = 0;
    superClassId       = -1;
    classType          = UNKNOWN;
    numDomainElements.resize(numDomains, -1);
    connectivityOffset.resize(numDomains, -1);
}


// ***************************************************************************
//  Destructor: MiliClassMetaData::MiliClassMetaData
//
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliClassMetaData::~MiliClassMetaData()
{
}


// ***************************************************************************
//  Method: MiliClassMetaData::SetSuperClassId
//
//  Purpose:
//      Set the superclass id. 
//
//  Arguments: 
//      superClass    The superclass id. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
MiliClassMetaData::SetSuperClassId(int superClass)
{
    superClassId = superClass;
    DetermineType(superClassId);
}


// ***************************************************************************
//  Method: MiliClassMetaData::SetConnectivityOffset
//
//  Purpose:
//      Set the connectivity offset for this class. 
//
//  Arguments: 
//      domain     The domain. 
//      offest     The offset. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void 
MiliClassMetaData::SetConnectivityOffset(int domain, int offset)
{
    if (domain >= 0 && domain < connectivityOffset.size())
        connectivityOffset[domain] = offset;
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

int 
MiliClassMetaData::GetConnectivityOffset(int domain)
{
    if (domain >= 0 && domain < connectivityOffset.size())
        return connectivityOffset[domain];
    return -1;
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void 
MiliClassMetaData::SetNumElements(int domain, int nEl)
{
    if (domain >= 0 && domain < numDomainElements.size())
        numDomainElements[domain] = nEl;
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

int 
MiliClassMetaData::GetNumElements(int domain)
{
    if (domain >= 0 && domain < numDomainElements.size())
        return numDomainElements[domain];
    return -1;
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
MiliClassMetaData::DetermineType(int superClass)
{
    switch(superClass)
    {
        case M_UNIT:
            classType = UNKNOWN;
            break;
        case M_NODE:
            classType = NODE;
            break;
        case M_TRUSS:
        case M_BEAM:
        case M_TRI:
        case M_QUAD:
        case M_TET:
        case M_PYRAMID:
        case M_WEDGE:
        case M_HEX:
        //case M_TET10:
            classType = CELL;
            break;
        case M_MAT:
            classType = MATERIAL;
            break;
        case M_MESH:
            classType = GLOBAL;
            break;
        case M_SURFACE:
            classType = SURFACE;
            break;
        case M_PARTICLE:
            classType = PARTICLE;
            break;
        default:
            classType = UNKNOWN; 
            break;
    }
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliMaterialMetaData::MiliMaterialMetaData(void)
{
    name = "";
    for (int i = 0; i < 3; ++i)
    {
        color[i] = 0.0;
    }
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliMaterialMetaData::~MiliMaterialMetaData(void)
{
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
MiliMaterialMetaData::SetColor(float *inColor)
{
    for (int i = 0; i < 3; ++i)
    {
        color[i] = inColor[i];
    }
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

avtMiliMetaData::avtMiliMetaData(int nDomains)
:
  numDomains(nDomains),
  numClasses(0),
  numVariables(0),
  numMaterials(0)
{
    miliVariables   = NULL;
    miliClasses     = NULL;
    numCells.resize(numDomains, -1);
    numNodes.resize(numDomains, -1);
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

avtMiliMetaData::~avtMiliMetaData()
{
    if (miliVariables != NULL)
    {
        for (int i = 0; i < numVariables; ++i)
        {
            if (miliVariables[i] != NULL)
            {
                delete miliVariables[i];
            }
        }

        delete [] miliVariables;
    }

    if (miliClasses != NULL)
    {
        for (int i = 0; i < numClasses; ++i)
        {
            if (miliClasses[i] != NULL)
            {
                delete miliClasses[i];
            }
        }

        delete [] miliClasses;
    }
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::SetNumVariables(int nVars)
{
    if (miliVariables != NULL)
    {
        for (int i = 0; i < numVariables; ++i)
        {
            if (miliVariables[i] != NULL)
            {
                delete miliVariables[i];
            }
        }

        delete [] miliVariables;
    }

    numVariables  = nVars;
    miliVariables = new MiliVariableMetaData *[numVariables];
    for (int i = 0; i < numVariables; ++i)
    {
        miliVariables[i] = NULL;
    }
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::SetNumMaterials(int nMats)
{
    numMaterials = nMats;
    miliMaterials.clear();
    miliMaterials.reserve(nMats);
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::SetNumClasses(int nClasses)
{
    if (miliClasses != NULL)
    {
        for (int i = 0; i < numClasses; ++i)
        {
            if (miliClasses[i] != NULL)
            {
                delete miliClasses[i];
            }
        }

        delete [] miliClasses;
    }

    numClasses  = nClasses;
    miliClasses = new MiliClassMetaData *[numClasses];
    for (int i = 0; i < numClasses; ++i)
    {
        miliClasses[i] = NULL;
    }
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::AddMiliClassMD(int classIdx, 
                                MiliClassMetaData *mcmd)
{
    if (classIdx < 0 || classIdx > numClasses)
    {
        //TODO: throw error
        return;
    }

    if (miliClasses == NULL)
    {
        //TODO: throw error
        return; 
    }

    if (miliClasses[classIdx] != NULL)
    {
        delete miliClasses[classIdx];
    }

    miliClasses[classIdx] = mcmd;
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

int
avtMiliMetaData::GetNumCells(int domain)
{
    if (domain >= 0 && domain < numCells.size())
    {
        return numCells[domain];
    }
    return 0;
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

int
avtMiliMetaData::GetNumNodes(int domain)
{
    if (domain >= 0 && domain < numNodes.size())
    {
        return numNodes[domain];
    }
    return 0;
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

int
avtMiliMetaData::GetMiliClassMDIdx(const char *cName)
{
    if (miliClasses == NULL)
    {
        return -1;
    }
    for (int i = 0; i < numClasses; ++i)
    {
        if (miliClasses[i] != NULL) 
        {
            if (miliClasses[i]->GetShortName() == cName)
            {
                return i;
            }
        }
    }
    return -1;
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliClassMetaData *
avtMiliMetaData::GetMiliClassMD(const char *vName)
{
    int idx = GetMiliClassMDIdx(vName);
    if (idx > -1 && idx < numClasses)
    {
        return miliClasses[idx];
    }

    return NULL; 
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::GetCellTypeCounts(vector<int> &cTypes, 
                                   vector<int> &ctCounts)
{
    //
    // First, count the occurrence of each super class
    // that is a cell. 
    //
    int miliSuperClassCount[M_QTY_SUPERCLASS];
    for (int i = 0; i < M_QTY_SUPERCLASS; ++i)
    {
        miliSuperClassCount[i] = 0;
    }

    ctCounts.reserve(numMiliCellTypes);
    cTypes.reserve(numMiliCellTypes);

    for (int i = 0; i < numClasses; ++i)
    {
        if (miliClasses[i]->GetClassType() == MiliClassMetaData::CELL)
        {
            miliSuperClassCount[miliClasses[i]->GetSuperClassId()]++;
        }
    }

    //
    // Compress our information for caller's use. 
    //
    for (int i = 0; i < M_QTY_SUPERCLASS; ++i)
    {
        if (miliSuperClassCount[i] > 0)
        {
            cTypes.push_back(i);
            ctCounts.push_back(miliSuperClassCount[i]);
        }
    }
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::AddMiliVariableMD(int varIdx, 
                                   MiliVariableMetaData *mvmd)
{
    std::string sName = mvmd->GetShortName();

    if (varIdx < 0 || varIdx > numVariables)
    {
        //TODO: throw error
        return;
    }

    if (miliVariables == NULL)
    {
        //TODO: throw error
        return; 
    }

    if (miliVariables[varIdx] != NULL)
    {
        //
        // For some reason, the subrecords don't like
        // to be cleansed in the destructor. 
        //
        //TODO: uncomment or move when done changing. 
        delete miliVariables[varIdx];
    }

    mvmd->InitSRContainers(numDomains);
    miliVariables[varIdx] = mvmd;
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliVariableMetaData *
avtMiliMetaData::GetMiliVariableMDByName(const char *vName)
{
    int idx = GetMiliVariableMDIdx(vName);
    if (idx > -1)
    {
        return miliVariables[idx];
    }

    return NULL; 
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliVariableMetaData *
avtMiliMetaData::GetMiliVariableMD(int varIdx)
{
    if (varIdx >= 0 && varIdx < numVariables)
    {
        return miliVariables[varIdx];
    }

    return NULL; 
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

int
avtMiliMetaData::GetMiliVariableMDIdx(const char *vName)
{
    if (miliVariables == NULL)
    {
        return -1;
    }
    for (int i = 0; i < numVariables; ++i)
    {
        if (miliVariables[i] != NULL) 
        {
            if (miliVariables[i]->GetShortName() == vName)
            {
                return i;
            }
        }
    }
    return -1;
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::AddMiliVariableSubrecInfo(int varIdx,
                                           int dom,
                                           int srId,
                                           Subrecord *sr)
 
{
    if (varIdx >= 0 && varIdx < numVariables && 
        miliVariables != NULL && dom < numDomains)
    {
        if (miliVariables[varIdx] != NULL)
        {
            miliVariables[varIdx]->AddSubrecordInfo(dom, 
                                                    srId,
                                                    sr->qty_objects,
                                                    sr->qty_blocks,
                                                    sr->mo_blocks);
        }
        //FIXME: add error handling. 
        else
            cerr << "INVALID ADD TO VAR SR!!!!!!!!!!" << endl;
    }
    else
        cerr << "INVALID ADD TO VAR SR!!!!!!!!!!" << endl;
}


// ***************************************************************************
//  Method: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

stringVector
avtMiliMetaData::GetMaterialNames(void)
{
    stringVector matNames;
    matNames.reserve(numMaterials);
    for (int i = 0; i < numMaterials; ++i)
    {
        matNames.push_back(miliMaterials[i].GetName());
    }
    return matNames;
}

