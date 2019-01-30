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
#include <InvalidVariableException.h>
#include <UnexpectedValueException.h>
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
//  Arguments:
//      sName        The short name. 
//      lName        The long name.
//      cSName       The class short name. 
//      cLName       The class long name.
//      isMultiM     Is multi mesh or not. 
//      isMat        Is a material var. 
//      isGlob       Is global. 
//      cent         centering (node/cell). 
//      meshAssoc    ID of associated mesh (used for multi-mesh).  
//      avtType      Avt cell type. 
//      aggType      Mili cell type.
//      nType        The type of scalar (float/int/etc.)
//      vecSize      Size of vector (if it is a vector).  
//      cDims        Component dimensions. 
//      vComps       Vector components (as shortnames). 
//
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliVariableMetaData::MiliVariableMetaData(string sName,
                                           string lName,
                                           string cSName,
                                           string cLName,
                                           bool isMultiM,
                                           bool isMat,
                                           bool isGlob,
                                           avtCentering cent,
                                           int nDomains,
                                           int meshAssoc,  
                                           int avtType,
                                           int aggType,
                                           int nType,
                                           int vecSize,
                                           int cDims,
                                           vector<string> vComps)
{
    shortName         = sName; 
    longName          = lName;
    classSName        = cSName;
    classLName        = cLName;
    multiMesh         = isMultiM;
    isMatVar          = isMat;
    isGlobal          = isGlob;
    meshAssociation   = meshAssoc;
    centering         = cent;
    cellTypeAvt       = avtType;
    cellTypeMili      = aggType;
    componentDims     = cDims;
    numType           = nType;
    vectorSize        = vecSize;

    subrecInfo.resize(nDomains);

    vectorComponents.resize(vComps.size());
    for (int i = 0; i < vComps.size(); ++i)
    {
        vectorComponents[i] = vComps[i];
    }

    isElementSet      = false;
    esMappedName      = "";
    path              = "";

    DetermineESStatus();
}


// ***************************************************************************
//  Destructor: MiliVariableMetaData::~MiliVariableMetaData
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
//      A reference to our path string. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

const string&
MiliVariableMetaData::GetPath()
{
    //TODO: what we want is "classLName (classSName)/shortName"
    //      but visit seems to garble this...
    if (path.empty())
    {
        if (multiMesh)
        {
            char mmStart[1024];
            sprintf(mmStart, "Primal (mesh%d)/", meshAssociation);
            path = mmStart;
        }
        else
        {
            path = "Primal";
        }

        if (!classSName.empty())
        {
            path += "/" + classSName;
        }

        //
        // If we have an element set, we need to use the mapped name.
        //
        if (isElementSet) 
        {
            path += "/" + esMappedName;
        }
        else
        {
            path += "/" + shortName;
        }
    }
    return path;
}


// ***************************************************************************
//  Method: MiliVariableMetaData::DetermineESStatus
//
//  Purpose:
//      Determine if this variable is an element set. If it is
//      perform the neccessary extra steps required for 
//      functionality. 
//
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
MiliVariableMetaData::DetermineESStatus(void)
{
    string esId    = "es_";
    string nameSub = shortName.substr(0, 3);
    if (esId == nameSub)
    {
        isElementSet = true;
    }

    //
    // If this is an element set, we need to determine and
    // assign it another name to be used in the visit path
    // besides its element set name. 
    //
    if (isElementSet)
    {
        if (vectorSize <= 0)
        {
            debug1 << "ERROR: we've found an element set that"
                << " is not a vector!" << endl;
            EXCEPTION1(InvalidVariableException, shortName.c_str());
        }

        //
        // The first character will allow us to map to the 
        // non-element set name. 
        //
        char leadingChar = vectorComponents[0][0];

        switch (leadingChar)
        {
            //
            // These are the known variables that can be element sets. 
            // This may need to be updated in the future. 
            //
            case 'e':
                esMappedName = "strain";
                break;
            case 's':
                esMappedName = "stress";
                break;
            default:
                char msg[1024]; 
                sprintf(msg, "An element set of unknown type has been "
                    "encountered! If valid, code needs to be updated");
                EXCEPTION2(UnexpectedValueException, shortName, msg);
                break;
        }
    }
}


// ***************************************************************************
//  Method: MiliVariableMetaData::PrintSelf
//
//  Purpose:
//      Print the class members that are set from the .mili file. 
//
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
MiliVariableMetaData::PrintSelf(void)
{
    cerr << "long name: " << longName << endl;
    cerr << "short name: " << shortName << endl;
    cerr << "es mapped name: " << esMappedName << endl;
    cerr << "class long Name: " << classLName << endl;
    cerr << "class short Name: " << classSName << endl;
    cerr << "path: " << path << endl;
    cerr << "cell type avt: " << cellTypeAvt << endl;
    cerr << "cell type mili: " << cellTypeMili << endl;
    cerr << "centering: " << centering << endl;
    cerr << "mesh assoc: " << meshAssociation << endl;
    cerr << "num type: " << numType << endl;
    cerr << "vec size: " << vectorSize << endl;
    cerr << "component dims: " << componentDims << endl;
    cerr << "is element set: " << isElementSet << endl;
    cerr << "is global: " << isGlobal << endl;
    cerr << "is multi mesh: " << multiMesh << endl;
    cerr << "vector components: " << endl;
    for (int i = 0; i < vectorComponents.size(); ++i)
    {
        cerr << vectorComponents[i] << endl;
    }
}


// ***************************************************************************
//  Constructor: MiliClassMetaData::MiliClassMetaData
//
//  Purpose:
//      Initialize the MiliClassMetaData. 
//
//  Arguments: 
//      sName         The class' short name.
//      lName         The class' long name.
//      scID          The class' superclass ID.
//      totalNEl      The total number of elements belonging to this class. 
//      numDomains    The number of domains. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliClassMetaData::MiliClassMetaData(string sName,
                                     string lName,
                                     int scID,
                                     int totalNEl,
                                     int numDomains)
{
    shortName          = sName;
    longName           = lName;
    totalNumElements   = totalNEl;
    superClassId       = scID;
    DetermineType();
    numDomainElements.resize(numDomains, 0);
    connectivityOffset.resize(numDomains, 0);
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
    {
        connectivityOffset[domain] = offset;
    }
}


// ***************************************************************************
//  Method: MiliClassMetaData::GetConnectivityOffset
//
//  Purpose:
//      Get the Class offset in the given domain. If we have N elements, 
//      and a variable of this Class type, the connectivity offset 
//      will be the starting position of this variable in our 
//      array of N scalars/vectors/etc. This is valuable when a scalar
//      is not applied to all elements. 
//
//  Arguments: 
//      domain    The domain. 
//
//  Returns:
//      The connectivity offset. 
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
    {
        return connectivityOffset[domain];
    }
    return -1;
}


// ***************************************************************************
//  Method: MiliClassMetaData::SetNumElements
//
//  Purpose:
//      Set the number of elements belonging to this class 
//      on a specified domain. 
//
//  Arguments: 
//      domain    The domain of interest. 
//      nEl       The number of elements on this domain. 
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
    {
        numDomainElements[domain] = nEl;
    }
}


// ***************************************************************************
//  Method: MiliClassMetaData::GetNumElements
//
//  Purpose:
//      Get the number of elements belonging to this class
//      on a given domain. 
//
//  Arguments: 
//      domain    The domain of interest. 
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
    {
        return numDomainElements[domain];
    }
    return -1;
}


// ***************************************************************************
//  Method: MiliClassMetaData::DetermineType
//
//  Purpose:
//      Determine a generalized type for this class. 
//
//  Arguments: 
//      superClass    The superclass ID.
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
MiliClassMetaData::DetermineType()
{
    switch(superClassId)
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
//  Constructor: MiliMaterialMetaData::MiliMaterialMetaData
//
//  Arguments: 
//      matName    The name of this material. 
//      matColor   The color of this material. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliMaterialMetaData::MiliMaterialMetaData(string matName,
                                           float *matColor)
{
    name = matName;
    for (int i = 0; i < 3; ++i)
    {
        color[i] = matColor[i];
    }
}


// ***************************************************************************
//  Destructor: MiliMaterialMetaData::~MiliMaterialMetaData
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
//  Constructor: avtMiliMetaData::avtMiliMetaData
//
//  Arguments: 
//      nDomains    The number of domains present in this data. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

avtMiliMetaData::avtMiliMetaData(int nDomains)
{
    numDomains    = nDomains;
    numClasses    = 0;
    numMaterials  = 0;
    miliVariables = NULL;
    miliClasses   = NULL;
    miliMaterials = NULL;
    numCells.resize(numDomains, -1);
    numNodes.resize(numDomains, -1);
}


// ***************************************************************************
//  Destructor: avtMiliMetaData::~avtMiliMetaData
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

    if (miliMaterials != NULL)
    {
        for (int i = 0; i < numMaterials; ++i)
        {
            if (miliMaterials[i] != NULL)
            {
                delete miliMaterials[i];
            }
        }

        delete [] miliMaterials;
    }
}


// ***************************************************************************
//  Method: avtMiliMetaData::SetNumVariables
//
//  Purpose:
//      Set the number of variables in our data, and instantiate
//      the miliVariables container. 
//
//  Arguments: 
//      nVars    The number of variables present in our data. 
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
//  Method: avtMiliMetaData::SetNumMaterials
//
//  Purpose:
//      Set the number of materials in our data, and initialize the 
//      miliMaterials container. 
//
//  Arguments: 
//      nMats    The number of materials in our dataset. 
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
    if (miliMaterials != NULL)
    {
        for (int i = 0; i < numMaterials; ++i)
        {
            if (miliMaterials[i] != NULL)
            {
                delete miliMaterials[i];
            }
        }

        delete [] miliMaterials;
    }

    numMaterials  = nMats;
    miliMaterials = new MiliMaterialMetaData *[numMaterials];
    for (int i = 0; i < numMaterials; ++i)
    {
        miliMaterials[i] = NULL;
    }
}


// ***************************************************************************
//  Method: avtMiliMetaData::SetNumClasses
//
//  Purpose:
//      Set the number of classes in our dataset, and initialize the 
//      miliClasses container. 
//
//  Arguments: 
//      nClasses    The number of classes in our dataset.
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
//  Method: avtMiliMetaData::AddClassMD
//
//  Purpose:
//      Add a mili Class meta data object to our container. 
//
//  Arguments: 
//      classIdx    The index of the Clas md. 
//      mcmd        The mili class meta data. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::AddClassMD(int classIdx, 
                            MiliClassMetaData *mcmd)
{
    if (classIdx < 0 || classIdx >= numClasses)
    {
        char expected[1024]; 
        char recieved[1024];
        sprintf(expected, "an index betwen 0 and %d", numClasses - 1);
        sprintf(recieved, "%d", classIdx);
        EXCEPTION2(UnexpectedValueException, expected, recieved);
    }

    if (miliClasses == NULL)
    {
        char msg[1024];
        sprintf(msg, "Attempting to add MD to uninitialized container!");
        EXCEPTION1(ImproperUseException, msg);
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
avtMiliMetaData::GetClassMDIdx(const char *cName)
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
avtMiliMetaData::GetClassMD(const char *vName)
{
    int idx = GetClassMDIdx(vName);
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
avtMiliMetaData::AddVarMD(int varIdx, 
                          MiliVariableMetaData *mvmd)
{
    std::string sName = mvmd->GetShortName();

    if (varIdx < 0 || varIdx > numVariables)
    {
        
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
        delete miliVariables[varIdx];
    }

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
avtMiliMetaData::GetVarMDByShortName(const char *vName)
{
    int idx = GetVarMDIdxByShortName(vName);
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
avtMiliMetaData::GetVarMDByPath(const char *vPath)
{
    int idx = GetVarMDIdxByPath(vPath);
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
avtMiliMetaData::GetVarMD(int varIdx)
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
avtMiliMetaData::GetVarMDIdxByShortName(const char *vName)
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

int
avtMiliMetaData::GetVarMDIdxByPath(const char *vPath)
{
    if (miliVariables == NULL)
    {
        return -1;
    }
    for (int i = 0; i < numVariables; ++i)
    {
        if (miliVariables[i] != NULL) 
        {
            if (miliVariables[i]->GetPath() == vPath)
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
avtMiliMetaData::AddVarSubrecInfo(int varIdx,
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

void
avtMiliMetaData::AddMaterialMD(int matIdx, 
                               MiliMaterialMetaData *mmmd)
{
    if (matIdx < 0 || matIdx > numMaterials)
    {
        //TODO: throw error
        return;
    }

    if (miliMaterials == NULL)
    {
        //TODO: throw error
        return; 
    }

    if (miliMaterials[matIdx] != NULL)
    {
        delete miliMaterials[matIdx];
    }

    miliMaterials[matIdx] = mmmd;
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
        matNames.push_back(miliMaterials[i]->GetName());
    }
    return matNames;
}

