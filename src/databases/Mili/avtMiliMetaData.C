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

#include <algorithm>


// ***************************************************************************
//  Constructor: SubrecInfo::SubrecInfo
//
//  Programmer: Alister Maguire
//  Creation:   May 13, 2019
//
//  Modifications:
//
// ****************************************************************************

SubrecInfo::SubrecInfo(void)
{
    numSubrecs = 0;
}


// ***************************************************************************
//  Destructor: SubrecInfo::~SubrecInfo
//
//  Programmer: Alister Maguire
//  Creation:   May 13, 2019
//
//  Modifications:
//
// ****************************************************************************

SubrecInfo::~SubrecInfo(void)
{
}


// ***************************************************************************
//  SubrecInfo::AddSubrec
//
//  Purpose:
//      Add information about a subrecord. 
//
//  Notes:
//      The subrecords are associated with blocks of data on the mesh
//      that may be segmented by cell type or domain. 
//
//  Arguments:
//      SRId        The subrecord id. 
//      numEl       The number of elements associated with this subrecord. 
//      numDB       The number of "data blocks" associated with this subrecord. 
//      dBRanges    The data block ranges. 
//
//  Programmer: Alister Maguire
//  Creation:   May 13, 2019
//
//  Modifications:
//
// ****************************************************************************

void
SubrecInfo::AddSubrec(const int SRId,
                      const int numEl,
                      const int numDB,
                      const int *dBRanges)
{
    if (indexMap.count(SRId) == 0)
    {
        indexMap[SRId] = numSubrecs++;

        numElements.push_back(numEl);
        numDataBlocks.push_back(numDB);

        //
        // Deep copy the ranges so that we don't have to keep 
        // the subrecords in memory. 
        //
        int limit = numDB * 2;
        intVector nxtRange(limit);

        for (int i = 0; i < limit; ++i)
        {
            nxtRange[i] = dBRanges[i]; 
        }

        dataBlockRanges.push_back(nxtRange);
    }
}


// ***************************************************************************
//  SubrecInfo::GetSubrec
//
//  Purpose:
//      Retrieve information about a subrecord. 
//
//  Arguments:
//      SRId        The subrecord id. 
//      numEl       The number of elements associated with this subrecord. 
//      numDB       The number of "data blocks" associated with this subrecord.
//      dBRanges    The data block ranges. 
//
//  Programmer: Alister Maguire
//  Creation:   May 13, 2019
//
//  Modifications:
//
// ****************************************************************************

void
SubrecInfo::GetSubrec(const int SRId,
                      int &numEl,
                      int &numDB,
                      intVector &dBRanges)
{
    std::map<int, int>::iterator mapItr = indexMap.find(SRId);

    if (mapItr != indexMap.end())
    {
        numEl    = numElements[mapItr->second];
        numDB    = numDataBlocks[mapItr->second];
        dBRanges = dataBlockRanges[mapItr->second];
    }
    else
    {
        numEl = -1;
        numDB = -1;
    }
}


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
//      shared       Is a shared variable. 
//      cent         centering (node/cell). 
//      meshAssoc    ID of associated mesh (used for multi-mesh).  
//      avtType      Avt variable type. 
//      aggType      Mili variable type.
//      nType        The type of scalar (float/int/etc.)
//      vecSize      Size of vector (if it is a vector).  
//      cDims        Component dimensions. 
//      vComps       Vector components (as shortnames). 
//
//  Notes: Global variables and material variables currently DO NOT 
//         have their number of elements ever set. This is because
//         global variables are applied to the entire mesh, and 
//         material variables are associated with materials that
//         are defined on the mesh. 
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
                                           bool shared,
                                           bool isES,
                                           avtCentering cent,
                                           int nDomains,
                                           int meshAssoc,  
                                           int avtType,
                                           int aggType,
                                           int nType,
                                           int vecSize,
                                           int cDims,
                                           stringVector vComps)
{
    shortName         = sName; 
    longName          = lName;
    classSName        = cSName;
    classLName        = cLName;
    multiMesh         = isMultiM;
    isMatVar          = isMat;
    isGlobal          = isGlob;
    isShared          = shared;
    isElementSet      = isES;
    meshAssociation   = meshAssoc;
    centering         = cent;
    varTypeAvt        = avtType;
    varTypeMili       = aggType;
    componentDims     = cDims;
    numType           = nType;
    vectorSize        = vecSize;

    SRIds.resize(nDomains);

    vectorComponents.resize(vComps.size());
    for (int i = 0; i < vComps.size(); ++i)
    {
        vectorComponents[i] = vComps[i];
    }

    path = "";

    //
    // Sand and cause are two special cases dealing with 
    // destroyed elements. 
    //
    isSand        = false;
    isCause       = false;
    string sanded = "sand";
    string cause  = "cause";

    if (shortName == sanded)
    {
        isSand = true;
    }
    else if (shortName == cause)
    {
        isCause = true;
    }
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
string
MiliVariableMetaData::GetVectorComponent(int idx)
{
    if (idx >= 0 && idx < vectorComponents.size())
        return vectorComponents[idx];
    return "";
}


// ***************************************************************************
//  Method: MiliVariableMetaData::AddSubrecId
//
//  Purpose:
//      Add a subrecord index for this variable.
//
//  Arguments: 
//      dom    The domain that this subrecord resides on. 
//      SRId   The subrecord index/ID.
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
MiliVariableMetaData::AddSubrecId(int dom, int SRId)
{
    if (dom > SRIds.size() || dom < 0)
    {
        char msg[128];
        snprintf(msg, 128, "Invalid domain index for subrecord idxs!");
        EXCEPTION1(ImproperUseException, msg);
    }
    SRIds[dom].push_back(SRId);
}


// ***************************************************************************
//  Method: MiliVariableMetaData::GetSubrecIds
//
//  Purpose:
//      Get the subrecord indicies for a given domain. 
//
//  Arguments: 
//      dom    The domain of interest. 
//
//  Returns:
//      A copy of the subrecord index vector 
//      for the given domain. These indicies correspond
//      to the subrecordInfo stored by avtMiliMetaData.
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

intVector 
MiliVariableMetaData::GetSubrecIds(int dom)
{
    if (dom >= 0 && dom < SRIds.size())
    {
        return SRIds[dom];
    }

    intVector empty;
    return empty;
}


// ***************************************************************************
//  Method: MiliVariableMetaData::GetPath
//
//  Purpose:
//      Get the visit path for a variable. 
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
            char mmStart[128];
            snprintf(mmStart, 128, "Primal (mesh%d)/", meshAssociation);
            path = mmStart;
        }
        else
        {
            path = "Primal";
        }

        if (isShared)
        {
            path += "/Shared";
        }
        else if (!classSName.empty())
        {
            path += "/" + classSName;
        }

        path += "/" + shortName;
    }
    return path;
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
    std::cerr << "long name: " << longName << std::endl;
    std::cerr << "short name: " << shortName << std::endl;
    std::cerr << "class long Name: " << classLName << std::endl;
    std::cerr << "class short Name: " << classSName << std::endl;
    std::cerr << "path: " << path << std::endl;
    std::cerr << "var type avt: " << varTypeAvt << std::endl;
    std::cerr << "var type mili: " << varTypeMili << std::endl;
    std::cerr << "centering: " << centering << std::endl;
    std::cerr << "mesh assoc: " << meshAssociation << std::endl;
    std::cerr << "num type: " << numType << std::endl;
    std::cerr << "vec size: " << vectorSize << std::endl;
    std::cerr << "component dims: " << componentDims << std::endl;
    std::cerr << "is element set: " << isElementSet << std::endl;
    std::cerr << "is global: " << isGlobal << std::endl;
    std::cerr << "is multi mesh: " << multiMesh << std::endl;
    std::cerr << "vector components: " << std::endl;
    for (int i = 0; i < vectorComponents.size(); ++i)
    {
        std::cerr << vectorComponents[i] << std::endl;
    }
}


// ***************************************************************************
//  constructor: MiliElementSetMetaData::MiliElementSetMetaData
//
//  Purpose:
//      Initialized the MiliElementSetMetaData. 
//
//  Arguments:
//      sName        The short name. 
//      lName        The long name.
//      cSName       The class short name. 
//      cLName       The class long name.
//      isMultiM     Is multi mesh or not. 
//      isMat        Is a material var. 
//      isGlob       Is global. 
//      shared       Is a shared variable. 
//      cent         centering (node/cell). 
//      meshAssoc    ID of associated mesh (used for multi-mesh).  
//      avtType      Avt variable type. 
//      aggType      Mili variable type.
//      nType        The type of scalar (float/int/etc.)
//      vecSize      Size of vector (if it is a vector).  
//      cDims        Component dimensions. 
//      vComps       Vector components (as shortnames). 
//      gVecSizes    The vector sizes for each "group" in the ES.
//      gAvtTypes    The avt types for each "group" in the ES.
//      gMiliTypes   The mili types for each "group" in the ES.
//
//  Programmer: Alister Maguire
//  Creation:   April 26, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliElementSetMetaData::MiliElementSetMetaData(string sName,
                                               string lName,
                                               string cSName,
                                               string cLName,
                                               bool isMultiM,
                                               bool isMat,
                                               bool isGlob,
                                               bool shared,
                                               avtCentering cent,
                                               int nDomains,
                                               int meshAssoc,  
                                               int avtType,
                                               int aggType,
                                               int nType,
                                               int vecSize,
                                               int cDims,
                                               stringVector vComps,
                                               stringVector gNames,
                                               intVector gVSizes,
                                               intVector gAvtTypes,
                                               intVector gMiliTypes,
                                               boolVector gIsShared)
: MiliVariableMetaData(sName,
                       lName,
                       cSName,
                       cLName,
                       isMultiM,
                       isMat,
                       isGlob,
                       shared,
                       true,
                       cent,
                       nDomains,
                       meshAssoc,  
                       avtType,
                       aggType,
                       nType,
                       vecSize,
                       cDims,
                       vComps)
{
    groupShortNames = gNames;
    groupVecSizes   = gVSizes;
    groupAvtTypes   = gAvtTypes;
    groupMiliTypes  = gMiliTypes;
    groupIsShared   = gIsShared;
    numGroups       = groupShortNames.size();

    groupComponentIdxs.reserve(numGroups);

    int componentIdx = 0;
    int gIdx = 0;
    for (std::vector<int>::iterator it = groupVecSizes.begin();
         it != groupVecSizes.end(); ++it, ++gIdx) 
    {

        //
        // Figure out the group indices. 
        //
        std::vector<int> idxs;
        idxs.reserve(*it);
        int j = 0;

        while (j < (*it))
        {
            idxs.push_back(componentIdx++);
            j++;
        }

        groupComponentIdxs.push_back(idxs);

        //
        // Since we're here, figure out the paths as well. 
        //
        string groupPath;

        if (multiMesh)
        {
            char mmStart[128];
            snprintf(mmStart, 128, "Primal (mesh%d)/", meshAssociation);
            groupPath = mmStart;
        }
        else
        {
            groupPath = "Primal";
        }

        if (groupIsShared[gIdx])
        {
            groupPath += "/Shared";
        }
        else if (!classSName.empty())
        {
            groupPath += "/" + classSName;
        }

        groupPath += "/" + groupShortNames[gIdx];

        groupPaths.push_back(groupPath);
    }
}


// ***************************************************************************
//  destructor: MiliElementSetMetaData::~MiliElementSetMetaData
//
//  Purpose:
//      Destroy the object. 
//
//  Programmer: Alister Maguire
//  Creation:   April 26, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliElementSetMetaData::~MiliElementSetMetaData(void)
{
}


// ***************************************************************************
//  Method: MiliElementSetMetaData::GetPath
//
//  Purpose:
//      Since this is an element set that might contain multiple groups, 
//      this method should not be used. The path should be empty. 
//
//  Returns:
//      A reference to our path string (empty). 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

const string&
MiliElementSetMetaData::GetPath()
{
    return path;
}


// ***************************************************************************
//  Method: MiliElementSetMetaData::GetGroupPath
//
//  Purpose:
//      Get the visit path for an element set group. 
//
//  Arguments:
//      groupIdx    The group index of interest. 
//
//  Returns:
//      If groupIdx is valid, the group's path. Otherwise, an empty string. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

string
MiliElementSetMetaData::GetGroupPath(int groupIdx)
{
    if (groupIdx < groupPaths.size())
    {
        return groupPaths[groupIdx]; 
    }
    return "";
}


// ***************************************************************************
//  Method: MiliElementSetMetaData::GetGroupPath
//
//  Purpose:
//      Get the visit path for an element set group. 
//
//  Arguments:
//      groupName    The group name of interest. 
//
//  Returns:
//      If groupName is valid, the group's path. Otherwise, an empty string. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

string
MiliElementSetMetaData::GetGroupPath(string groupName)
{
    for (int i = 0; i < groupShortNames.size(); ++i)
    {
        if (groupShortNames[i] == groupName)
        {
            return GetGroupPath(i);
        }
    }
    return "";
}


// ***************************************************************************
//  Method: MiliElementSetMetaData::GetGroupIdxByPath
//
//  Purpose:
//      Retrieve a group index from a given path. 
//
//  Arguments:
//      groupPath    The group path of interest. 
//
//  Returns:
//      If groupPath is valid, the associated group index is returned. 
//      Otherwise, -1. 
//           
//  Programmer: Alister Maguire
//  Creation:   May 6, 2019
//
//  Modifications:
//
// ****************************************************************************

int
MiliElementSetMetaData::GetGroupIdxByPath(const char *groupPath)
{
    string truePath = groupPath;
    if (strstr(groupPath, avtMiliMetaData::GetSandDir()) == groupPath) 
    {
        string strVPath(groupPath);
        size_t sDirPos = strVPath.find_first_of("/");
        truePath       = strVPath.substr(sDirPos + 1);
    }

    int gIdx = 0;
    for (stringVector::iterator gItr = groupPaths.begin();
         gItr != groupPaths.end(); ++gItr, ++gIdx)
    {
        if ((*gItr) == truePath)
        {
            return gIdx;
        }
    }
    return -1;
}


// ***************************************************************************
//  Method: MiliElementSetMetaData::GetGroupShortName
//
//  Purpose:
//      Get a group short name by index. 
//
//  Arguments:
//      gIdx    The group index. 
//
//  Returns:
//      If gIdx is valid, the group short name is returned. Otherwise, 
//      an empty string. 
//           
//  Programmer: Alister Maguire
//  Creation:   May 6, 2019
//
//  Modifications:
//
// ****************************************************************************

string
MiliElementSetMetaData::GetGroupShortName(int gIdx)
{
    if (gIdx >= 0 && gIdx < groupShortNames.size())
    {
        return groupShortNames[gIdx];
    }
    string empty;
    return empty;
}


// ***************************************************************************
//  Method: MiliElementSetMetaData::GetGroupVecSize
//
//  Purpose:
//      Get the vector size for a group. 
//
//  Arguments:
//      gIdx    The group index. 
//
//  Returns:
//      If gIdx is valid, the group vec size is returned. Otherwise, -1.
//           
//  Programmer: Alister Maguire
//  Creation:   May 6, 2019
//
//  Modifications:
//
// ****************************************************************************

int
MiliElementSetMetaData::GetGroupVecSize(int gIdx)
{
    if (gIdx >= 0 && gIdx < groupVecSizes.size())
    {
        return groupVecSizes[gIdx];
    }
    return -1;
}


// ***************************************************************************
//  Method: MiliElementSetMetaData::GroupIsShared
//
//  Purpose:
//      Determine if a particular group is shared or not. 
//
//  Arguments:
//      gIdx    The group index. 
//
//  Returns:
//      true if the group is shared. False otherwise. 
//           
//  Programmer: Alister Maguire
//  Creation:   May 6, 2019
//
//  Modifications:
//
// ****************************************************************************

bool
MiliElementSetMetaData::GroupIsShared(int gIdx)
{
    if (gIdx >= 0 && gIdx < groupIsShared.size())
    {
        return groupIsShared[gIdx];
    }
    return false;
}


// ***************************************************************************
//  Method: MiliElementSetMetaData::GetGroupVecComponents
//
//  Purpose:
//      Get the vector components for the given group. 
//
//  Arguments:
//      gIdx    The group index. 
//
//  Returns:
//      If gIdx is valid, the group vec components are returned. Otherwise, an
//      empty vector. 
//           
//  Programmer: Alister Maguire
//  Creation:   May 6, 2019
//
//  Modifications:
//
// ****************************************************************************

stringVector
MiliElementSetMetaData::GetGroupVecComponents(int gIdx)
{
    stringVector vComps;
    intVector cIdxs;

    if (gIdx >= 0 && gIdx < groupComponentIdxs.size())
    {
        cIdxs = groupComponentIdxs[gIdx]; 
    }

    for (intVector::iterator it = cIdxs.begin();
         it != cIdxs.end(); ++it)
    {
        vComps.push_back(vectorComponents[(*it)]);
    }

    return vComps;
}


// ***************************************************************************
//  Method: MiliElementSetMetaData::GetGroupComponentIdxs
//
//  Purpose:
//      Get the component indicies for a given group. 
//
//  Arguments: 
//      groupName    The name of the group of interest. 
//
//  Returns:
//      If the name is valid, a const reference to the vector containing the 
//      component indicies for this group. Otherwise, an empty vector. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

intVector 
MiliElementSetMetaData::GetGroupComponentIdxs(string groupName)
{
    for (int i = 0; i < groupShortNames.size(); ++i)
    {
        if (groupShortNames[i] == groupName)
        {
            return GetGroupComponentIdxs(i);
        }
    }
    intVector empty;
    return empty;
}


// ***************************************************************************
//  Method: MiliElementSetMetaData::GetGroupComponentIdxs
//
//  Purpose:
//      Get the component indicies for a given group. 
//
//  Arguments: 
//      groupIdx    The index of the group of interest. 
//
//  Returns:
//      If the index is valid, a const reference to the vector containing 
//      the component indicies for this group. Otherwise, an empty vector. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

intVector 
MiliElementSetMetaData::GetGroupComponentIdxs(int groupIdx)
{
    if (groupIdx < groupComponentIdxs.size())
    {
        return groupComponentIdxs[groupIdx];
    }
    intVector empty;
    return empty;
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
                                     int numDomains)
{
    shortName    = sName;
    longName     = lName;
    superClassId = scID;
    DetermineType();
    numDomainElements.resize(numDomains, 0);
    connectivityOffset.resize(numDomains, 0);
    labelIds.resize(numDomains);
    elementLabels.resize(numDomains);
    labelsGenerated.resize(numDomains, false);
    maxLabelLengths.resize(numDomains, 0);
    labelPositions.resize(numDomains);

    for (int i = 0; i < numDomains; ++i)
    {
        labelPositions[i].numBlocks = 0;
    }
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
//      domain    The domain of interest. 
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
//  Method: MiliClassMetaData::PopulateLabelIds
//
//  Purpose:
//      Populate the class label ids and ranges. 
//
//  Arguments: 
//      domain        The domain of interest. 
//      ids           An array of ids of size "number of 
//                    elements on this domain".
//      numBlocks     The number of blocks that our labels span.
//      blockRanges   The block ranges for the labels. 
//           
//  Programmer: Alister Maguire
//  Creation:   April 3, 2019
//
//  Modifications:
//
// ****************************************************************************

void 
MiliClassMetaData::PopulateLabelIds(int domain, 
                                    int *ids,
                                    int numBlocks,
                                    int *blockRanges)
{
    if (domain >= 0 && domain < labelIds.size())
    {
        int nEl = numDomainElements[domain];
        for (int i = 0; i < nEl; ++i)
        {
            labelIds[domain][i] = ids[i];
        }

        if (numBlocks > 0)
        {
            PopulateLabelPositions(domain, numBlocks, blockRanges);
        }
    }
}


// ***************************************************************************
//  Method: MiliClassMetaData::PopulateLabelPositions
//
//  Purpose:
//      Populate the class label positions. This information is needed by
//      the vtkElementLabelArray.
//
//  Arguments: 
//      domain        The domain of interest. 
//      numBlocks     The number of blocks/segments of labels for this class. 
//      blockRanges   The block ranges. 
//           
//  Programmer: Alister Maguire
//  Creation:   April 9, 2019
//
//  Modifications:
//
// ****************************************************************************

void 
MiliClassMetaData::PopulateLabelPositions(int  domain, 
                                          int  numBlocks,
                                          int *blockRanges)
{
    if (domain >= 0 && domain < labelPositions.size())
    {
        int idPos = GetConnectivityOffset(domain);
        for (int i = 0; i < numBlocks; ++i)
        {
            int baseIdx   = i * 2;
            int begin     = blockRanges[baseIdx];
            int end       = blockRanges[baseIdx + 1];
            int rangeSize = end - begin + 1;
            labelPositions[domain].rangesBegin.push_back(begin);
            labelPositions[domain].rangesEnd.push_back(end);
            labelPositions[domain].idsBegin.push_back(idPos);
            labelPositions[domain].idsEnd.push_back(idPos + rangeSize - 1);
            idPos += rangeSize; 
        }
        labelPositions[domain].numBlocks = numBlocks;
    }
}


// ***************************************************************************
//  Method: MiliClassMetaData::GetLabelPositionInfoPtr
//
//  Purpose:
//      Get label position info for this class. 
//
//  Arguments: 
//      domain        The domain of interest. 
//
//  Returns:
//      A const pointer to the label position info for this class if it 
//      exists. Otherwise, NULL. 
//           
//  Programmer: Alister Maguire
//  Creation:   April 9, 2019
//
//  Modifications:
//
// ****************************************************************************

const LabelPositionInfo * 
MiliClassMetaData::GetLabelPositionInfoPtr(int domain)
{
    if (domain >= 0 && domain < labelPositions.size())
    { 
        return &labelPositions[domain];
    }
    return NULL;
}


// ***************************************************************************
//  Method: MiliClassMetaData::GetElementLabels
//
//  Purpose:
//      Get the class labels.
//
//  Arguments: 
//      domain        The domain of interest. 
//      outLabels     A reference to a vector for storing the requested labels.
//
//  Returns:
//      true if the retrieval was successful. Otherwise, false. 
//           
//  Programmer: Alister Maguire
//  Creation:   April 3, 2019
//
//  Modifications:
//
// ****************************************************************************

bool
MiliClassMetaData::GetElementLabels(int domain, stringVector &outLabels)
{
    if (domain >= 0 && domain < elementLabels.size())
    {
        if (!labelsGenerated[domain])        
        {
            GenerateElementLabels(domain);
        }

        outLabels = elementLabels[domain];
        return true;
    }

    return false;
}


// ***************************************************************************
//  Method: MiliClassMetaData::GetElementLabelsPtr
//
//  Purpose:
//      Get a pointer to the class labels.
//
//  Arguments: 
//      domain    The domain of interest. 
//
//  Returns:
//      A pointer to the desired labels if successful. Otherwise, NULL. 
//           
//  Programmer: Alister Maguire
//  Creation:   April 3, 2019
//
//  Modifications:
//
// ****************************************************************************

stringVector *
MiliClassMetaData::GetElementLabelsPtr(int domain)
{
    if (domain >= 0 && domain < elementLabels.size())
    {
        if (!labelsGenerated[domain])        
        {
            GenerateElementLabels(domain);
        }

        return &elementLabels[domain];
    }

    return NULL;
}


// ***************************************************************************
//  Method: MiliClassMetaData::GetMaxLabelLength
//
//  Purpose:
//      Get the max label length for this class. 
//
//  Arguments: 
//      domain    The domain of interest. 
//
//  Returns:
//      The max label length on the given domain.
//           
//  Programmer: Alister Maguire
//  Creation:   April 3, 2019
//
//  Modifications:
//
// ****************************************************************************

int
MiliClassMetaData::GetMaxLabelLength(int domain)
{
    if (domain >= 0 && domain < maxLabelLengths.size())
    {
        if (!labelsGenerated[domain])
        {
            GenerateElementLabels(domain);
        }
        return maxLabelLengths[domain];
    }

    return 0;
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
//  Notes: Global variables and material variables currently DO NOT 
//         have their number of elements ever set. This is because
//         global variables are applied to the entire mesh, and 
//         material variables are associated with materials that
//         are defined on the mesh. 
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
    if (domain >= 0 && domain < labelIds.size())
    {
        numDomainElements[domain] = nEl;
        labelIds[domain].resize(nEl, -1);
        elementLabels[domain].resize(nEl, "");
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
//  Returns:
//      The number of elements belonging to this class on the
//      given domain. 
//
//  Notes: Global variables and material variables currently DO NOT 
//         have their number of elements ever set. This is because
//         global variables are applied to the entire mesh, and 
//         material variables are associated with materials that
//         are defined on the mesh. 
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
//  Method: MiliClassMetaData::GenerateElementLabels
//
//  Purpose:
//      Attempt to generate the class labels. 
//
//  Arguments:
//      domain    The domain of interest. 
//           
//  Programmer: Alister Maguire
//  Creation:   April 3, 2019
//
//  Modifications:
//
// ****************************************************************************

void
MiliClassMetaData::GenerateElementLabels(int domain)
{
    if (domain >= 0 && domain < labelIds.size())
    {
        if (labelIds[domain].size() == 0)
        {
            return;
        }
        else if (labelIds[domain][0] == -1)
        {
            //
            // If we don't have label ids, just use the short name. 
            //
            int numEl = numDomainElements[domain];
            for (int i = 0; i < numEl; ++i)
            { 
                elementLabels[domain][i] = shortName;
                maxLabelLengths[domain]  = std::max(int(shortName.size()), 
                    maxLabelLengths[domain]);
            }
        }
        else
        {
            const char *cSName = shortName.c_str();
            int pos = 0;

            for (intVector::iterator idItr = labelIds[domain].begin();
                 idItr != labelIds[domain].end(); idItr++)
            {
                char cLabel[256];
                snprintf(cLabel, 256, "%s %i", cSName, (*idItr));
                string sLabel = string(cLabel);
                elementLabels[domain][pos++] = sLabel;

                maxLabelLengths[domain] = std::max(int(sLabel.size()), 
                    maxLabelLengths[domain]);
            }
        }
        labelsGenerated[domain] = true;
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
                                           string matColor)
{
    name     = matName;
    hexColor =  matColor;
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

    //
    // In mili, sanded elements are those that have been
    // "destroyed" during the simulation process. By default,
    // we ghost them out. However, we also allow access to 
    // a mesh that keeps these elements intact. These variables
    // are all found in the sand path (see GetSandDir()). 
    //
    containsSand  = false;

    miliVariables = NULL;
    miliClasses   = NULL;
    miliMaterials = NULL;
    numCells.resize(numDomains, -1);
    numNodes.resize(numDomains, -1);
    subrecInfo.resize(numDomains);
    zoneBasedLabels.resize(numDomains, stringVector());
    nodeBasedLabels.resize(numDomains, stringVector());
    maxZoneLabelLengths.resize(numDomains, 0);
    maxNodeLabelLengths.resize(numDomains, 0);
    zoneLabelsGenerated.resize(numDomains, false);
    nodeLabelsGenerated.resize(numDomains, false);
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

    const size_t sharedSize = sharedVariables.size();
    for (int i = 0; i < sharedSize; ++i) 
    {
        if (sharedVariables[i] != NULL)
        {
            delete sharedVariables[i];
            sharedVariables[i] = NULL;
        }
    }
    sharedVariables.clear();
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
//      NOTE: mcmd enters ownership of avtMiliMetaData and all
//            memory is managed internally from then on. 
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
        char expected[128]; 
        char recieved[128];
        snprintf(expected, 128, "an index betwen 0 and %d", numClasses - 1);
        snprintf(recieved, 128, "%d", classIdx);
        EXCEPTION2(UnexpectedValueException, expected, recieved);
    }

    if (miliClasses == NULL)
    {
        char msg[128];
        snprintf(msg, 128, "Attempting to add MD to uninitialized container!");
        EXCEPTION1(ImproperUseException, msg);
    }

    if (miliClasses[classIdx] != NULL)
    {
        delete miliClasses[classIdx];
    }

    miliClasses[classIdx] = mcmd;
}


// ***************************************************************************
//  Method: avtMiliMetaData::SetNumCells
//
//  Purpose:
//      Set the number of cells for a given domain. 
//
//  Arguments: 
//      domain    The domain of interest. 
//      nCells    The number of cells on the given domain. 
//
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::SetNumCells(int domain, int nCells)
{
    if (domain >= 0 && domain < numDomains)
    {
        numCells[domain] = nCells;
        zoneBasedLabels[domain].resize(nCells, string(""));
    }
}


// ***************************************************************************
//  Method: avtMiliMetaData::SetNumNodes
//
//  Purpose:
//      Set the number of nodes for the given domain. 
//
//  Arguments: 
//      domain    The domain of interest. 
//      nNodes    The number of nodes on the given domain. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::SetNumNodes(int domain, int nNodes)
{
    if (domain >= 0 && domain < numDomains)
    {
        numNodes[domain] = nNodes;
        nodeBasedLabels[domain].resize(nNodes, string(""));

        //
        // See if we have a node class. If so, tell it 
        // how many elements it has. 
        //
        MiliClassMetaData *nodeMD = GetClassMDByShortName("node");

        if (nodeMD != NULL)
        {
            nodeMD->SetNumElements(domain, nNodes);
        }
    }
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetNumCells
//
//  Purpose:
//      Get the number of cells on the given domain. 
//
//  Arguments: 
//      domain    The domain of interest. 
//
//  Returns:
//      The number of cells on the given domain, if it's valid. 
//      If it's not valid, 0 is returned. 
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
//  Method: avtMiliMetaData::GetNumNodes
//
//  Purpose:
//      Get the number of nodes on the given domain. 
//
//  Arguments: 
//      domain    The domain of interest. 
//
//  Returns:
//      The number of nodes on the given domain, if it's valid. 
//      If it's not valid, 0 is returned. 
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
//  Method: avtMiliMetaData::GetClassMDIdxByShortName
//
//  Purpose:
//      Get the index of a MiliClassMetaData in our container. 
//
//  Arguments: 
//      cName    The shortname of the class.
//
//  Returns:
//      If valid, the index to the requested class. If not valid, 
//      NULL is returned. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

int
avtMiliMetaData::GetClassMDIdxByShortName(const char *cName)
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
//  Method: avtMiliMetaData::GetCellBasedClassMD
//
//  Purpose:
//      Retrieve a vector of pointers to all mili class MD
//      that are cell based. 
//
//  Arguments:
//      outMD    The vector to contain the class MD.  
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::GetCellBasedClassMD(
    std::vector<MiliClassMetaData *> &outMD)
{
    outMD.clear();

    for (int classIdx = 0; classIdx < numClasses; ++classIdx)
    {
        MiliClassMetaData *classMD = miliClasses[classIdx];
        MiliClassMetaData::ClassType type = classMD->GetClassType();

        if (type == MiliClassMetaData::CELL)
        {
            outMD.push_back(classMD);
        }
    }
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetNodeBasedClassMD
//
//  Purpose:
//      Retrieve a vector of pointers to all mili class MD
//      that are node based. 
//
//  Arguments:
//      outMD    The vector to contain the class MD.  
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::GetNodeBasedClassMD(
    std::vector<MiliClassMetaData *> &outMD)
{
    outMD.clear();

    for (int classIdx = 0; classIdx < numClasses; ++classIdx)
    {
        MiliClassMetaData *classMD = miliClasses[classIdx];
        MiliClassMetaData::ClassType type = classMD->GetClassType();

        if (type == MiliClassMetaData::NODE)
        {
            outMD.push_back(classMD);
        }
    }
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetClassMDByShortName
//
//  Purpose:
//      Get the meta data for a Class with the given name. 
//
//  Arguments: 
//      vName    The Class name.  
//
//  Returns:
//      If valid, the Class is returned. Otherwise, NULL is returned. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliClassMetaData *
avtMiliMetaData::GetClassMDByShortName(const char *vName)
{
    int idx = GetClassMDIdxByShortName(vName);
    if (idx > -1 && idx < numClasses)
    {
        return miliClasses[idx];
    }

    return NULL; 
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetCellTypeCounts
//
//  Purpose:
//       Retrieve two pieces of information; First, a list designating
//       which mili superclasses are in our data and are a cell type. 
//       Second, a list designating the number of Classes in our
//       data the corresond to each superclass.
//
//       Example: suppose our data contains 2 Classes that are under
//       the superclass with ID 3 and 1 Class that is under the superclass
//       with ID 5. Our arrays would be populated as follows:
//           cTypes   = {3, 5} 
//           ctCounts = {2, 1}
//
//  Arguments: 
//      cTypes    A reference to an array to hold superclass IDs. 
//      ctCounts  A reference to an array to hold superclass counts. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::GetCellTypeCounts(intVector &cTypes, 
                                   intVector &ctCounts)
{
    if (miliClasses == NULL)
    {
        return;
    }

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
        if (miliClasses[i] != NULL)
        {
            if (miliClasses[i]->GetClassType() == MiliClassMetaData::CELL)
            {
                miliSuperClassCount[miliClasses[i]->GetSuperClassId()]++;
            }
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
//  Method: avtMiliMetaData::AddVarMD
//
//  Purpose:
//      Add a MiliVariableMetaData object to our container. 
//
//  Arguments: 
//      varIdx    Where in our container to store the object.
//      mvmd      The MiliVariableMetaData object. 
//
//      NOTE: mvmd enters ownership of avtMiliMetaData and all
//            memory is managed internally from then on. 
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
    string sName = mvmd->GetShortName();

    if (varIdx < 0 || varIdx > numVariables)
    {
        char msg[128];
        snprintf(msg, 128, "Invalid index for adding MiliVariableMetaData!");
        EXCEPTION1(ImproperUseException, msg);
    }

    if (miliVariables == NULL)
    {
        char msg[128];
        snprintf(msg, 128, "Attempting to add MiliVariableMetaData to "
            "an uninitialized container!");
        EXCEPTION1(ImproperUseException, msg);
    }

    if (miliVariables[varIdx] != NULL)
    {
        delete miliVariables[varIdx];
        miliVariables[varIdx] = NULL;
    }

    if (mvmd->IsSand())
    {
        containsSand = true;
    }
    miliVariables[varIdx] = mvmd;

    //
    // If this is a shared variable, add some info about it
    // for easy access in the future. 
    //
    if (mvmd->IsShared())
    {
        if (mvmd->IsElementSet())
        {
            boolVector gIsShared = 
                ((MiliElementSetMetaData *)mvmd)->GetGroupIsShared();
            stringVector gNames = 
                ((MiliElementSetMetaData *)mvmd)->GetGroupShortNames();

            int gIdx = 0;
            for (boolVector::iterator gItr = gIsShared.begin();
                 gItr != gIsShared.end(); gItr++, gIdx++)
            {
                if (*gItr)
                {
                    AddSharedVariableInfo(gNames[gIdx], varIdx, true);
                }
            }
        }
        else
        {
            AddSharedVariableInfo(mvmd->GetShortName(), 
                varIdx, false);
        }
    }
}


// ***************************************************************************
//  Method: GetVarMDByShortName
//
//  Purpose:
//      Get a MiliVariableMetaData from our container given 
//      its shortname. 
//
//  Arguments: 
//      vName    The shortname of our desired variable. 
//      cName    The shortname of the variables class (a single
//               variable can be associated with multiple classes). 
//
//  Returns:
//      The desired MiliVariableMetaData if it's valid. If not valid,
//      NULL is returned. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliVariableMetaData *
avtMiliMetaData::GetVarMDByShortName(const char *vName,
                                     const char *cName)
{
    int idx = GetVarMDIdxByShortName(vName, cName);
    if (idx > -1)
    {
        return miliVariables[idx];
    }

    return NULL; 
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetVarMDByPath
//
//  Purpose:
//      Get a MiliVariableMetaData from our container given
//      its visit path. 
//
//  Arguments: 
//      vPath    The visit path (ex: Primal/beams/stress)
//
//  Returns:
//      The desired MiliVariableMetaData if it's valid. If not valid,
//      NULL is returned. 
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
    int idx = -1;
    if (strstr(vPath, GetSandDir()) == vPath) 
    {
        string strVPath(vPath);
        size_t sDirPos  = strVPath.find_first_of("/");
        string truePath = strVPath.substr(sDirPos + 1);
        idx = GetVarMDIdxByPath(truePath.c_str());
    }
    else
    {
        idx = GetVarMDIdxByPath(vPath);
    }

    if (idx > -1)
    {
        return miliVariables[idx];
    }

    return NULL; 
}

// ***************************************************************************
//  Method: avtMiliMetaData::GetVarMDByIdx
//
//  Purpose:
//      Get a MiliVariableMetaData given its container index. 
//
//  Arguments: 
//      varIdx    The container index. 
//      
//  Returns:
//      The desired MiliVariableMetaData if it's valid. If not valid,
//      NULL is returned. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

MiliVariableMetaData *
avtMiliMetaData::GetVarMDByIdx(int varIdx)
{
    if (varIdx >= 0 && varIdx < numVariables)
    {
        return miliVariables[varIdx];
    }

    return NULL; 
}


// ***************************************************************************
//  Method: GetVarMDIdxByShortName
//
//  Purpose:
//      Get a container index to the MiliVariableMetaData having
//      the given name. 
//
//  Arguments: 
//      vName    The variable name. 
//      cName    The shortname of the variables class (a single
//               variable can be associated with multiple classes). 
//
//  Returns:
//      If the name is valid, the container index is returned. Otherwise, 
//      -1 is returned. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

int
avtMiliMetaData::GetVarMDIdxByShortName(const char *vName,
                                        const char *cName)
{
    if (miliVariables == NULL)
    {
        return -1;
    }
    for (int i = 0; i < numVariables; ++i)
    {
        if (miliVariables[i] != NULL) 
        {
            if (miliVariables[i]->GetShortName() == vName &&
                miliVariables[i]->GetClassShortName() == cName)
            {
                return i;
            }
            else if (miliVariables[i]->IsElementSet() && 
                miliVariables[i]->GetClassShortName() == cName)
            {
                //
                // If this is an element set, we need to check against
                // all of its group names. IMPORTANT: we must check the
                // group names after we've checked the short name (above)
                // so that users can still request the element set itself
                // (v.s. one of it's groups). Taking the lazy route...
                //
                stringVector groupNames = 
                    ((MiliElementSetMetaData *)miliVariables[i])->
                    GetGroupShortNames();

                for (stringVector::iterator gItr = groupNames.begin();
                     gItr != groupNames.end(); ++gItr)
                {
                    if (vName == (*gItr))
                    {
                        return i;
                    }
                }
            }
        }
    }
    return -1;
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetVarMDIdxByPath
//
//  Purpose:
//      Get a container index to the MiliVariableMetaData having
//      the given path. 
//
//  Arguments: 
//      vPath    The variable's visit path. 
//
//  Returns:
//      If the path is valid, the container index is returned. Otherwise,
//      -1 is returned. 
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
    string strVPath(vPath);

    if (strstr(vPath, GetSandDir()) == vPath) 
    {
        size_t sDirPos = strVPath.find_first_of("/");
        strVPath       = strVPath.substr(sDirPos + 1);
    }

    if (miliVariables == NULL)
    {
        return -1;
    }
    for (int i = 0; i < numVariables; ++i)
    {
        if (miliVariables[i] != NULL) 
        {
            if (miliVariables[i]->GetPath() == strVPath)
            {
                return i;
            }
            else if (miliVariables[i]->IsElementSet())
            {
                //
                // If this is an element set, we need to check against
                // all of its group paths. IMPORTANT: we must check the
                // group paths after we've checked the short name (above)
                // so that users can still request the element set itself
                // (v.s. one of it's groups). Taking the lazy route...
                //
                stringVector groupPaths = 
                    ((MiliElementSetMetaData *)miliVariables[i])->
                    GetGroupPaths();

                int gIdx = 0;
                for (stringVector::iterator gItr = groupPaths.begin();
                     gItr != groupPaths.end(); ++gItr, ++gIdx)
                {
                    if ((*gItr) == strVPath)
                    {
                        return i;
                    }
                }
            }
        }
    }
    return -1;
}


// ***************************************************************************
//  Method: avtMiliMetaData::AddVarSubrecInfo
//
//  Purpose:
//      Add subrecord information, and tell an associated variable
//      where to find it. 
//
//      IMPORTANT: this method assumes that the subrecord ids are added
//          sequentially. This method SHOULD NOT be used for adding 
//          subrecord ids in non-sequential order. 
//
//  Arguments: 
//      varIdx    The index of the associated variable. 
//      dom       The domain of interest. 
//      SRId      The subrecord Id. 
//      SR        A pointer to the subrecord. 
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
                                  int SRId,
                                  Subrecord *SR)
 
{
    if (varIdx >= 0 && varIdx < numVariables && 
        miliVariables != NULL && dom < numDomains)
    {
        if (miliVariables[varIdx] != NULL)
        {
            subrecInfo[dom].AddSubrec(SRId,
                                      SR->qty_objects,
                                      SR->qty_blocks,
                                      SR->mo_blocks);

            miliVariables[varIdx]->AddSubrecId(dom, SRId);
        }
        else
        {
            char msg[128];
            snprintf(msg, 128, 
                "Attempting to add MD to uninitialized container!");
            EXCEPTION1(ImproperUseException, msg);
        }
    }
    else
    {
        char msg[128];
        snprintf(msg, 128, "Invalid index assignment requested!");
        EXCEPTION1(ImproperUseException, msg);
    }
}


// ***************************************************************************
//  Method: avtMiliMetaData::GenerateZoneBasedLabels
//
//  Purpose:
//      Generated the zone based labels. 
//
//  Arguments: 
//      domain    The domain of interest. 
//           
//  Programmer: Alister Maguire
//  Creation:   April 3, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::GenerateZoneBasedLabels(int domain)
{
    if (domain >= 0 &&  domain < numDomains)
    {
        for (int classIdx = 0; classIdx < numClasses; ++classIdx)
        {
            stringVector *classLabels = 
                miliClasses[classIdx]->GetElementLabelsPtr(domain);
            if (classLabels != NULL)
            {
                int offset = miliClasses[classIdx]->
                    GetConnectivityOffset(domain);
                MiliClassMetaData::ClassType classType = 
                    miliClasses[classIdx]->GetClassType();

                if (classType == MiliClassMetaData::CELL)
                {
                    if ((offset + (classLabels->size())) > numCells[domain])
                    {
                        char msg[128];
                        snprintf(msg, 128, "Number of cell labels " 
                            "exceeds number of cells?!?");
                        EXCEPTION1(ImproperUseException, msg);
                    }

                    if (zoneBasedLabels.size() == 0)
                    {
                        debug2 << "zone labels haven't been initialized yet??";
                        zoneBasedLabels[domain].resize(numCells[domain], ""); 
                    }

                    int cIdx = offset;
                    for (stringVector::iterator labelItr = classLabels->begin();
                         labelItr != classLabels->end(); labelItr++)
                    {
                        if (cIdx >= numCells[domain])
                        {
                            debug1 << "label index out of bounds?!?";
                            break;
                        }
                        zoneBasedLabels[domain][cIdx++] = (*labelItr);
                        maxZoneLabelLengths[domain] = 
                            std::max(int(labelItr->size()), 
                            maxZoneLabelLengths[domain]);
                    }
                }
            }
        }
        zoneLabelsGenerated[domain] = true;
    }
}


// ***************************************************************************
//  Method: avtMiliMetaData::GenerateNodeBasedLabels
//
//  Purpose:
//      Generated the node based labels. 
//
//  Arguments: 
//      domain    The domain of interest. 
//           
//  Programmer: Alister Maguire
//  Creation:   April 3, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::GenerateNodeBasedLabels(int domain)
{
    if (domain >= 0 &&  domain < numDomains)
    {
        for (int classIdx = 0; classIdx < numClasses; ++classIdx)
        {
            stringVector *classLabels = 
                miliClasses[classIdx]->GetElementLabelsPtr(domain);
            if (classLabels != NULL)
            {
                int offset = miliClasses[classIdx]->
                    GetConnectivityOffset(domain);
                MiliClassMetaData::ClassType classType = 
                    miliClasses[classIdx]->GetClassType();

                if(classType == MiliClassMetaData::NODE)
                {
                    if ((offset + (classLabels->size())) > numNodes[domain])
                    {
                        char msg[128];
                        snprintf(msg, 128, "Number of node labels "
                            "exceeds number of nodes?!?");
                        EXCEPTION1(ImproperUseException, msg);
                    }

                    if (nodeBasedLabels.size() == 0)
                    {
                        debug2 << "node labels haven't been initialized yet??";
                        nodeBasedLabels[domain].resize(numNodes[domain], ""); 
                    }

                    int nIdx = offset;
                    for (stringVector::iterator labelItr = classLabels->begin();
                         labelItr != classLabels->end(); labelItr++)
                    {
                        if (nIdx >= numNodes[domain])
                        {
                            debug1 << "label index out of bounds?!?";
                            break;
                        }
                        nodeBasedLabels[domain][nIdx++] = (*labelItr);
                        maxNodeLabelLengths[domain] = 
                            std::max(int(labelItr->size()), 
                            maxNodeLabelLengths[domain]);
                    }
                }
            }
        }
        nodeLabelsGenerated[domain] = true;
    }
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetZoneBasedLabelsPtr
//
//  Purpose:
//      Get a pointer to the zone labels vector.
//
//  Arguments: 
//      domain    The domain of interest.  
//           
//  Returns:
//      If valid, a pointer to the labels. NULL otherwise. 
//
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

const stringVector *
avtMiliMetaData::GetZoneBasedLabelsPtr(int domain)
{
    if (domain < 0 || domain >= numDomains)
    {
        char msg[128];
        snprintf(msg, 128, "Invalid domain for labels pointer!");
        EXCEPTION1(ImproperUseException, msg);
    }

    if (!zoneLabelsGenerated[domain])
    {
        GenerateZoneBasedLabels(domain);
    }
    return &zoneBasedLabels[domain];
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetNodeBasedLabelsPtr
//
//  Purpose:
//      Get a pointer to the node labels vector.
//
//  Arguments: 
//      domain    The domain of interest.  
//
//  Returns:
//      If valid, a pointer to the labels. NULL otherwise. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

const stringVector *
avtMiliMetaData::GetNodeBasedLabelsPtr(int domain)
{
    if (domain < 0 || domain >= numDomains)
    {
        char msg[128];
        snprintf(msg, 128, "Invalid domain for labels pointer!");
        EXCEPTION1(ImproperUseException, msg);
    }

    if (!nodeLabelsGenerated[domain])
    {
        GenerateNodeBasedLabels(domain);
    }
    return &nodeBasedLabels[domain];
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetMaxZoneLabelLength
//
//  Purpose:
//      Get the maximum zone label length. 
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
avtMiliMetaData::GetMaxZoneLabelLength(int domain)
{
    if (domain >= 0 && domain < numDomains)
    {
        if (!zoneLabelsGenerated[domain])
        {
            GenerateZoneBasedLabels(domain);
        }

        return maxZoneLabelLengths[domain];
    }

    return 0;
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetMaxNodeLabelLength
//
//  Purpose:
//      Get the maximum node label length. 
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
avtMiliMetaData::GetMaxNodeLabelLength(int domain)
{
    if (domain >= 0 && domain < numDomains)
    {
        if (!nodeLabelsGenerated[domain])
        {
            GenerateNodeBasedLabels(domain);
        }

        return maxNodeLabelLengths[domain];
    }

    return 0;
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetSubrecInfo
//
//  Purpose:
//      Get the subrecord info for a given domain. 
//
//  Arguments: 
//      dom    The domain of interest.  
//
//  Returns:
//      If available, the requested subrecord info is returned. Otherwise, 
//      NULL. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

SubrecInfo *
avtMiliMetaData::GetSubrecInfo(int dom)
{
    if (dom >= 0 && dom < numDomains)
    {
        return &subrecInfo[dom];
    }

    return NULL;
}


// ***************************************************************************
//  Method: avtMiliMetaData::AddMaterialMD
//
//  Purpose:
//      Add information about a material to our meta data. 
//
//  Arguments: 
//      matIdx    The index to add the MD to. 
//      mmmd      The material meta data to add 
//
//      NOTE: mmmd enters ownership of avtMiliMetaData and all
//            memory is managed internally from then on. 
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
        char msg[128];
        snprintf(msg, 128, "Invalid index assignment requested!");
        EXCEPTION1(ImproperUseException, msg);
    }

    if (miliMaterials == NULL)
    {
        char msg[128];
        snprintf(msg, 128, "Attempting to add MD to uninitialized container!");
        EXCEPTION1(ImproperUseException, msg);
    }

    if (miliMaterials[matIdx] != NULL)
    {
        delete miliMaterials[matIdx];
        miliMaterials = NULL;
    }

    miliMaterials[matIdx] = mmmd;
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetMaterialNames
//
//  Purpose:
//      Retrieve the list of material names associated with
//      this dataset. 
//
//  Arguments: 
//      matNames    The material names. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::GetMaterialNames(stringVector &matNames)
{
    if (miliMaterials == NULL)
    {
        return;
    }

    matNames.clear();
    matNames.reserve(numMaterials);

    for (int i = 0; i < numMaterials; ++i)
    {
        if (miliMaterials[i] != NULL)
        {
            matNames.push_back(miliMaterials[i]->GetName());
        }
    }
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetMaterialColors
//
//  Purpose:
//      Get the material colors. 
//
//  Returns:
//      A vector of material colors. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::GetMaterialColors(stringVector &matColors)
{
    if (miliMaterials == NULL)
    {
        return;
    }

    matColors.clear();    
    matColors.reserve(numMaterials);

    for (int i = 0; i < numMaterials; ++i)
    {
        if (miliMaterials[i] != NULL)
        {
            matColors.push_back(miliMaterials[i]->GetColor());
        }
    }
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetSharedVariableInfo
//
//  Purpose:
//      Retrieve info about a shared variable.
//
//  Arguments: 
//      shortName     The short name of the desired variable. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

SharedVariableInfo *
avtMiliMetaData::GetSharedVariableInfo(const char *shortName)
{
    std::vector<SharedVariableInfo *>::iterator sItr;
    for (sItr = sharedVariables.begin(); sItr != sharedVariables.end();
         ++sItr)
    {
        //
        // If this is a known shared variable, just add the 
        // indicies. 
        //
        if ((*sItr)->shortName == shortName)
        {
            return (*sItr);
        }
    }

    return NULL;
}


// ***************************************************************************
//  Method: avtMiliMetaData::AddSharedVariableInfo
//
//  Purpose:
//      Add info about a shared variable. This can either be a new
//      shared var or information about a known shared var. 
//
//      FYI: mili allows a single variable to be shared across
//      multiple classes. This can complicate things when one
//      is an element set and another is not. We keep track of
//      some lookup info to ease future access. 
//
//  Arguments: 
//      shortName     The short name of our variable. 
//      varIdx        The variable index in relation to the MD container. 
//      isES          Is this variable an element set?
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
//      Alister Maguire, Fri Jun 28 15:01:24 PDT 2019
//      Set isLive to false.
//
// ****************************************************************************

void
avtMiliMetaData::AddSharedVariableInfo(string shortName,
                                       int varIdx, 
                                       bool isES)
{
    bool newVar = true;

    std::vector<SharedVariableInfo *>::iterator sItr;
    for (sItr = sharedVariables.begin(); sItr != sharedVariables.end();
         ++sItr)
    {
        //
        // If this is a known shared variable, just add the 
        // indicies. 
        //
        if ((*sItr)->shortName == shortName)
        {
            newVar = false;
            (*sItr)->variableIndicies.push_back(varIdx);

            //
            // Check if this variable changes the ES status. 
            //
            if ((*sItr)->isAllES && !isES)
            {
                (*sItr)->isAllES = false;
            }
        }
    }

    if (newVar)
    {
        //
        // This is a new variable. Add it. 
        //
        SharedVariableInfo *newShared = new SharedVariableInfo();
        newShared->shortName = shortName;
        newShared->isAllES   = false;
        newShared->isLive    = false;
        newShared->variableIndicies.push_back(varIdx);
        sharedVariables.push_back(newShared);
    }
}


// ***************************************************************************
//  Method: avtMiliMetaData::ContainsESFlag
//
//  Purpose:
//      Determine if a string contains an element set flag. This is used
//      to determine if a variable is an element set from its short name. 
//
//  Arguments: 
//      shortName     The short name to test. 
//      sNSize        The length of shortName.  
//           
//  Programmer: Alister Maguire
//  Creation:   May 21, 2019
//
//  Modifications:
//
// ****************************************************************************

bool
avtMiliMetaData::ContainsESFlag(const char *shortName, int sNSize)
{
    const char *esFlag = "es_";
    const int flagSize = 3;

    if (sNSize < flagSize)
    {
        return false; 
    }

    int i = 0;
    while (i < flagSize)
    {
        if (shortName[i] != esFlag[i])
        {
            return false;
        }
        ++i;
    }

    return true;
}
