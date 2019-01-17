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

using std::ifstream;
using std::endl;
using std::cerr;


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
MiliVariableMetaData::~MiliVariableMetaData()
{
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
std::string
MiliVariableMetaData::GetVectorComponent(int idx)
{
    if (idx >= 0 && idx < vectorComponents.size())
        return vectorComponents[idx];
    //FIXME: raise error?
    return "";
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
void
MiliVariableMetaData::InitSRContainers(int numDomains)
{
    subrecordIds.resize(numDomains);
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
vector<int> &
MiliVariableMetaData::GetSubrecordIds(int dom)
{
    if (dom > subrecordIds.size() || dom < 0)
    {
        //TODO: raise error
        cerr << "ERROR: Trying to retieve invalid subrecord ID!" << endl;
    }
    return subrecordIds[dom];
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
void
MiliVariableMetaData::AddSubrecordIds(int dom, 
                                      int srIdx)
{
    if (dom >= 0 && dom < subrecordIds.size())
    {
        subrecordIds[dom].push_back(srIdx);        
    }
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
string
MiliVariableMetaData::GetPath()
{
    //TODO: what we want is "classLName (classSName)/shortName"
    //      but visit seems to garble this...
    return classSName + "/" + shortName;
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
MiliClassMetaData::~MiliClassMetaData()
{
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
MiliMaterialMetaData::~MiliMaterialMetaData(void)
{
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
MiliMetaData::MiliMetaData(int nDomains)
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
MiliMetaData::~MiliMetaData()
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

    CleanseSubrecords();
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
void
MiliMetaData::CleanseSubrecords()
{
    //
    // Cleanse the variable subrecords. 
    //
    for (int i = 0; i < subrecords.size(); ++i)
    {
        mc_cleanse_subrec(&subrecords[i]);
    }
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
void
MiliMetaData::SetNumVariables(int nVars)
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
void
MiliMetaData::SetNumMaterials(int nMats)
{
    numMaterials = nMats;
    miliMaterials.clear();
    miliMaterials.reserve(nMats);
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
void
MiliMetaData::SetNumClasses(int nClasses)
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
void
MiliMetaData::AddMiliClassMD(int classIdx, MiliClassMetaData *mcmd)
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************

int
MiliMetaData::GetNumCells(int domain)
{
    if (domain >= 0 && domain < numCells.size())
    {
        return numCells[domain];
    }
    return 0;
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
int
MiliMetaData::GetNumNodes(int domain)
{
    if (domain >= 0 && domain < numNodes.size())
    {
        return numNodes[domain];
    }
    return 0;
}

// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
int
MiliMetaData::GetMiliClassMDIdx(const char *cName)
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
MiliClassMetaData *
MiliMetaData::GetMiliClassMD(const char *vName)
{
    int idx = GetMiliClassMDIdx(vName);
    if (idx > -1 && idx < numClasses)
    {
        return miliClasses[idx];
    }

    return NULL; 
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
void
MiliMetaData::GetCellTypeCounts(vector<int> &cTypes, 
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
// ****************************************************************************

void
MiliMetaData::AddMiliVariableMD(int varIdx, 
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
MiliVariableMetaData *
MiliMetaData::GetMiliVariableMD(const char *vName)
{
    int idx = GetMiliVariableMDIdx(vName);
    if (idx > -1)
    {
        return miliVariables[idx];
    }

    return NULL; 
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
MiliVariableMetaData *
MiliMetaData::GetMiliVariableMD(int varIdx)
{
    if (varIdx >= 0 && varIdx < numVariables)
    {
        return miliVariables[varIdx];
    }

    return NULL; 
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
int
MiliMetaData::GetMiliVariableMDIdx(const char *vName)
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
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
void
MiliMetaData::AddMiliVariableSubrecord(const char *vName, 
                                       int dom,
                                       int srId)
{
    int varIdx = GetMiliVariableMDIdx(vName);
    AddMiliVariableSubrecord(varIdx, dom, srId);
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
void
MiliMetaData::AddMiliVariableSubrecord(int varIdx,
                                       int dom,
                                       int srId)
{
    if (varIdx >= 0 && varIdx < numVariables && 
        miliVariables != NULL && dom < numDomains)
    {
        if (miliVariables[varIdx] != NULL)
        {
            miliVariables[varIdx]->AddSubrecordIds(dom, srId);
        }
        else
            cerr << "INVALID ADD TO VAR SR!!!!!!!!!!" << endl;
    }
    else
        cerr << "INVALID ADD TO VAR SR!!!!!!!!!!" << endl;
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
vector<Subrecord *> 
MiliMetaData::GetMiliVariableSubrecords(int dom,
                                        const char *vName)
{
    int varIdx = GetMiliVariableMDIdx(vName);
    return GetMiliVariableSubrecords(dom, varIdx);
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
vector<Subrecord *> 
MiliMetaData::GetMiliVariableSubrecords(int dom,
                                        int varIdx)
{
    if (dom < 0 || dom > numDomains)
    {
        //TODO: error
        cerr << "INVALID DOMAIN" << endl;
    }

    if (varIdx > numVariables)
    {
        cerr << "INVALID VAR INDEX" << endl;
    }

    if (miliVariables == NULL)
    {
        cerr << "MILI VARIABLES ARE NULL!!" << endl; 
    }
     
    vector<int> srIdxs = miliVariables[varIdx]->GetSubrecordIds(dom);
    vector<Subrecord *> SRs; 
    for (vector<int>::iterator it = srIdxs.begin(); 
         it < srIdxs.end(); ++it)
    {
        SRs.push_back(&subrecords[*it]);
    }

    return SRs;
}



// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
stringVector
MiliMetaData::GetMaterialNames(void)
{
    stringVector matNames;
    matNames.reserve(numMaterials);
    for (int i = 0; i < numMaterials; ++i)
    {
        matNames.push_back(miliMaterials[i].GetName());
    }
    return matNames;
}


// ***************************************************************************
//  Function: 
//
//  Purpose:
//
//  Arguments: 
//           
//  Author:
//
//  Modifications:
//
// ****************************************************************************
void
MiliMetaData::AddSubrecord(int srId, Subrecord sr)
{
    subrecords.push_back(sr);
}

