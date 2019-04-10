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

MiliVariableMetaData::MiliVariableMetaData(std::string sName,
                                           std::string lName,
                                           std::string cSName,
                                           std::string cLName,
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
                                           stringVector vComps)
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

    SRIds.resize(nDomains);

    vectorComponents.resize(vComps.size());
    for (int i = 0; i < vComps.size(); ++i)
    {
        vectorComponents[i] = vComps[i];
    }

    isElementSet = false;
    esMappedName = "";
    path         = "";

    //
    // Sand and cause are two special cases dealing with 
    // destroyed elements. 
    //
    isSand             = false;
    isCause            = false;
    std::string sanded = "sand";
    std::string cause  = "cause";

    if (shortName == sanded)
    {
        isSand = true;
    }
    else if (shortName == cause)
    {
        isCause = true;
    }

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
        char msg[1024];
        sprintf(msg, "Invalid domain index for subrecord idxs!");
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
//      A reference to the subrecord index vector 
//      for the given domain. These indicies correspond
//      to the subrecordInfo stored by avtMiliMetaData.
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

intVector &
MiliVariableMetaData::GetSubrecIds(int dom)
{
    if (dom > SRIds.size() || dom < 0)
    {
        char msg[1024];
        sprintf(msg, "Invalid domain index for subrecord idxs!");
        EXCEPTION1(ImproperUseException, msg);
    }
    return SRIds[dom];
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

const std::string&
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
    std::string esId    = "es_";
    std::string nameSub = shortName.substr(0, 3);
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
                << " is not a vector!";
            EXCEPTION1(InvalidVariableException, shortName.c_str());
        }

        //
        // The vector should contain dimensional based variables
        // with a leading charact signifiyting it's mapped name. 
        // Since the vectors can also contain other elements, let's
        // first find a dimensional variable. 
        //
        int targetIdx;
        for (targetIdx = 0; targetIdx < vectorSize; ++targetIdx)
        {
            if (vectorComponents[targetIdx].size() > 1)
            {
                if (vectorComponents[targetIdx][1] == 'x')
                {
                    break;
                }
            }
        }
        char leadingChar = vectorComponents[targetIdx][0];

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
    std::cerr << "long name: " << longName << std::endl;
    std::cerr << "short name: " << shortName << std::endl;
    std::cerr << "es mapped name: " << esMappedName << std::endl;
    std::cerr << "class long Name: " << classLName << std::endl;
    std::cerr << "class short Name: " << classSName << std::endl;
    std::cerr << "path: " << path << std::endl;
    std::cerr << "cell type avt: " << cellTypeAvt << std::endl;
    std::cerr << "cell type mili: " << cellTypeMili << std::endl;
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

MiliClassMetaData::MiliClassMetaData(std::string sName,
                                     std::string lName,
                                     int scID,
                                     int numDomains)
{
    shortName       = sName;
    longName        = lName;
    superClassId    = scID;
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
        labelPositions[i].shortName = sName;
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
        //int offset = GetConnectivityOffset(domain);//FIXME: needed?
        int numEl = 0;
        for (int i = 0; i < numBlocks; ++i)
        {
            int baseIdx   = i * 2;
            int begin     = blockRanges[baseIdx];
            int end       = blockRanges[baseIdx + 1];
            int rangeSize = end - begin + 1;
            labelPositions[domain].rangesBegin.push_back(begin);
            labelPositions[domain].rangesEnd.push_back(end);
            labelPositions[domain].idsBegin.push_back(numEl);
            labelPositions[domain].idsEnd.push_back(numEl + rangeSize - 1);
            numEl++; 
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
                sprintf(cLabel, "%s %i", cSName, (*idItr));
                std::string sLabel = std::string(cLabel);
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

MiliMaterialMetaData::MiliMaterialMetaData(std::string matName,
                                           std::string matColor)
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
    // are all found under the sandDir. 
    //
    containsSand  = false;
    sandDir       = "sand_mesh";

    miliVariables = NULL;
    miliClasses   = NULL;
    miliMaterials = NULL;
    numCells.resize(numDomains, -1);
    numNodes.resize(numDomains, -1);
    subrecInfo.resize(numDomains);
    zoneBasedLabels.resize(numDomains, stringVector());
    nodeBasedLabels.resize(numDomains, stringVector());
    zoneLabelPositions.resize(numDomains);
    nodeLabelPositions.resize(numDomains);
    maxZoneLabelLengths.resize(numDomains, 0);
    maxNodeLabelLengths.resize(numDomains, 0);
    zoneLabelsGenerated.resize(numDomains, false);
    nodeLabelsGenerated.resize(numDomains, false);

    for (int i = 0; i < numDomains; ++i)
    {
        zoneLabelPositions[i].numBlocks = 0;
        zoneLabelPositions[i].shortName = "";
        nodeLabelPositions[i].numBlocks = 0;
        nodeLabelPositions[i].shortName = "";
    }
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
    if (domain >= 0 and domain < numDomains)
    {
        numCells[domain] = nCells;
        zoneBasedLabels[domain].resize(nCells, std::string(""));
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
        nodeBasedLabels[domain].resize(nNodes, std::string(""));

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
    std::string sName = mvmd->GetShortName();

    if (varIdx < 0 || varIdx > numVariables)
    {
        char msg[1024];
        sprintf(msg, "Invalid index for adding MiliVariableMetaData!");
        EXCEPTION1(ImproperUseException, msg);
    }

    if (miliVariables == NULL)
    {
        char msg[1024];
        sprintf(msg, "Attempting to add MiliVariableMetaData to "
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
    if (strstr(vPath, sandDir.c_str()) == vPath) 
    {
        std::string strVPath(vPath);
        size_t sDirPos       = strVPath.find_first_of("/");
        std::string truePath = strVPath.substr(sDirPos + 1);
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
    std::string strVPath(vPath);

    if (strstr(vPath, sandDir.c_str()) == vPath) 
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
            if (miliVariables[i]->GetPath() == strVPath.c_str())
            {
                return i;
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
//      srId      The subrecord Id. 
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
                                  int srId,
                                  Subrecord *SR)
 
{
    if (varIdx >= 0 && varIdx < numVariables && 
        miliVariables != NULL && dom < numDomains)
    {
        if (miliVariables[varIdx] != NULL)
        {
            //
            // Check if we've added the info for this subrecord
            // yet. SR IDs map to their index.  
            //
            if (srId >= subrecInfo[dom].nSR)
            {
                AddSubrecInfo(dom, 
                              SR->qty_objects,
                              SR->qty_blocks,
                              SR->mo_blocks);
            }

            miliVariables[varIdx]->AddSubrecId(dom, srId);
        }
        else
        {
            char msg[1024];
            sprintf(msg, "Attempting to add MD to uninitialized container!");
            EXCEPTION1(ImproperUseException, msg);
        }
    }
    else
    {
        char msg[1024];
        sprintf(msg, "Invalid index assignment requested!");
        EXCEPTION1(ImproperUseException, msg);
    }
}


// ***************************************************************************
//  Method: avtMiliMetaData::AddSubrecInfo
//
//  Purpose:
//      Create a SubrecInfo object, and store it in our list
//      of subrecord information for later retrieval.  
//
//  Arguments: 
//      dom         The domain of this subrecord. 
//      nElems      The number of elements (zones/nodes) associated with 
//                  this subrecrod. 
//      nDB         The number of "data blocks" associated with this subrecord.
//      DBRanges    The ranges associated with the data blocks. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtMiliMetaData::AddSubrecInfo(int   dom, 
                               int   nElems,
                               int   nDB,
                               int  *DBRanges)
{
    subrecInfo[dom].nSR++;
    subrecInfo[dom].nElements.push_back(nElems);
    subrecInfo[dom].nDataBlocks.push_back(nDB);

    //
    // Deep copy the ranges so that we don't have to keep 
    // the subrecords in memory. 
    //
    int limit = nDB * 2;
    intVector nxtRange(limit);
    for (int i = 0; i < limit; ++i)
    {
        nxtRange[i] = DBRanges[i]; 
    }

    subrecInfo[dom].dataBlockRanges.push_back(nxtRange);
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
                int offset = miliClasses[classIdx]->GetConnectivityOffset(domain);
                MiliClassMetaData::ClassType classType = 
                    miliClasses[classIdx]->GetClassType();

                if (classType == MiliClassMetaData::CELL)
                {
                    if ((offset + (classLabels->size())) > numCells[domain])
                    {
                        char msg[1024];
                        sprintf(msg, "Number of cell labels " 
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

                    const LabelPositionInfo *posInfo = miliClasses[classIdx]->
                        GetLabelPositionInfoPtr(domain);

                    if (posInfo == NULL)
                    {
                        continue;
                    }

                    for (int i = 0; i < posInfo->numBlocks; ++i)
                    {
                        zoneLabelPositions[domain].rangesBegin.push_back(
                            posInfo->rangesBegin[i]);
                        zoneLabelPositions[domain].rangesEnd.push_back(
                            posInfo->rangesEnd[i]);
                        zoneLabelPositions[domain].idsBegin.push_back(
                            posInfo->idsBegin[i]);
                        zoneLabelPositions[domain].idsEnd.push_back(
                            posInfo->idsEnd[i]);
                    }
                    //FIXME: need to add shortname for every class...
                    zoneLabelPositions[domain].numBlocks += posInfo->numBlocks;
                    zoneLabelPositions[domain].shortName  = posInfo->shortName;
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
                int offset = miliClasses[classIdx]->GetConnectivityOffset(domain);
                MiliClassMetaData::ClassType classType = 
                    miliClasses[classIdx]->GetClassType();

                if(classType == MiliClassMetaData::NODE)
                {
                    if ((offset + (classLabels->size())) > numNodes[domain])
                    {
                        char msg[1024];
                        sprintf(msg, "Number of node labels "
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

                    const LabelPositionInfo *posInfo = miliClasses[classIdx]->
                        GetLabelPositionInfoPtr(domain);

                    if (posInfo == NULL)
                    {
                        continue;
                    }

                    for (int i = 0; i < posInfo->numBlocks; ++i)
                    {
                        nodeLabelPositions[domain].rangesBegin.push_back(
                            posInfo->rangesBegin[i]);
                        nodeLabelPositions[domain].rangesEnd.push_back(
                            posInfo->rangesEnd[i]);
                        nodeLabelPositions[domain].idsBegin.push_back(
                            posInfo->idsBegin[i]);
                        nodeLabelPositions[domain].idsEnd.push_back(
                            posInfo->idsEnd[i]);
                    }
                    nodeLabelPositions[domain].numBlocks += posInfo->numBlocks;
                    nodeLabelPositions[domain].shortName  = posInfo->shortName;
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
        char msg[1024];
        sprintf(msg, "Invalid domain for labels pointer!");
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
        char msg[1024];
        sprintf(msg, "Invalid domain for labels pointer!");
        EXCEPTION1(ImproperUseException, msg);
    }

    if (!nodeLabelsGenerated[domain])
    {
        GenerateNodeBasedLabels(domain);
    }
    return &nodeBasedLabels[domain];
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetZoneLabelPositionsPtr
//
//  Purpose:
//      Get a const pointer to the zone label positions.
//
//  Arguments: 
//      domain    The domain of interest.  
//
//  Returns:
//      If valid, a const pointer to the zone label position info. Otherwise,
//      NULL. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

const LabelPositionInfo *
avtMiliMetaData::GetZoneLabelPositionsPtr(int domain)
{
    if (domain < 0 || domain >= numDomains)
    { 
        return &zoneLabelPositions[domain];
    }
    return NULL;
}


// ***************************************************************************
//  Method: avtMiliMetaData::GetNodeLabelPositionsPtr
//
//  Purpose:
//      Get a const pointer to the node label positions.
//
//  Arguments: 
//      domain    The domain of interest.  
//
//  Returns:
//      If valid, a const pointer to the zone label position info. Otherwise,
//      NULL. 
//           
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

const LabelPositionInfo*
avtMiliMetaData::GetNodeLabelPositionsPtr(int domain)
{
    if (domain < 0 || domain >= numDomains)
    { 
        return &nodeLabelPositions[domain];
    }
    return NULL;
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
//  Programmer: Alister Maguire
//  Creation:   Jan 15, 2019
//
//  Modifications:
//
// ****************************************************************************

SubrecInfo&
avtMiliMetaData::GetSubrecInfo(int dom)
{
    return subrecInfo[dom];
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
        char msg[1024];
        sprintf(msg, "Invalid index assignment requested!");
        EXCEPTION1(ImproperUseException, msg);
    }

    if (miliMaterials == NULL)
    {
        char msg[1024];
        sprintf(msg, "Attempting to add MD to uninitialized container!");
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
