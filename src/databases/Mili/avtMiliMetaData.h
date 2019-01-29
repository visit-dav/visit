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
//                             avtMiliMetaData.h                             //
// ************************************************************************* //

#ifndef AVT_MILI_META_DATA_H 
#define AVT_MILI_META_DATA_H

#include <vector>
#include <string>
#include <avtTypes.h>
#include <MapNode.h>

extern "C" {
#include <mili.h>
}

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

using std::string;
using std::vector;

//
// We don't need the entire subrecord, so let's just keep 
// a small verison instead. 
//
typedef struct SubrecInfo
{
    SubrecInfo(void)
    {
        nSR = 0;          
    };

    //TODO: add squeeze?
    int                   nSR;        
    vector<int>           nElements;
    vector<int>           SRIDs;
    vector<int>           nDataBlocks;
    vector< vector<int> > dataBlockRanges;   
} SubrecInfo;


// ****************************************************************************
//  Class: MiliVariableMetaData
//
//  Purpose:
//      A container for mili variable meta data. 
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 16, 2019
//
//  Modifications:
//
// ****************************************************************************

class MiliVariableMetaData
{

  public:

                              MiliVariableMetaData(void);
                             ~MiliVariableMetaData(void);

    void                      SetLongName(string lName)
                                { longName = lName; };
    string                    GetLongName(void)
                                { return longName; };

    void                      SetShortName(string sName)
                                { shortName = sName; };

    //TODO: we should considering returning references to strings. 
    string                    GetShortName(void)
                                { return shortName; };

    void                      SetClassLongName(string cName)
                                { classLName = cName; };
    string                    GetClassLongName(void)
                                { return classLName; }; 

    void                      SetClassShortName(string cName)
                                { classSName = cName; };
    string                    GetClassShortName(void)
                                { return classSName; }; 

    void                      SetAvtCellType(int cType)
                                { cellTypeAvt = cType; };
    int                       GetAvtCellType(void)
                                { return cellTypeAvt; };

    void                      SetMiliCellType(int cType)
                                { cellTypeMili = cType; };
    int                       GetMiliCellType(void)
                                { return cellTypeMili; };

    void                      SetCentering(int cent)
                                { centering = avtCentering(cent); }; 
    avtCentering              GetCentering(void)
                                { return centering; };

    void                      SetNumType(int nType)
                                { numType = nType; };
    int                       GetNumType(void)
                                { return numType; };
         
    void                      SetComponentDims(int d)
                                { componentDims = d; };
    int                       GetComponentDims(void) 
                                { return componentDims; }; 

    void                      SetVectorSize(int vs)
                                { vectorSize = vs; };
    int                       GetVectorSize(void)
                                { return vectorSize; };

    void                      SetMeshAssociation(bool isMulti,
                                                 int meshIdx) 
                                { meshAssociation = meshIdx; 
                                  multiMesh       = isMulti; };
    int                       GetMeshAssociation(void)
                                { return meshAssociation; }; 
    
    void                      SetIsElementSet(bool es)
                                { isElementSet = es; };
    bool                      GetIsElementSet(void)
                                { return isElementSet; };

    void                      SetIsMatVar(bool isMat)
                                { isMatVar = isMat; };
    bool                      GetIsMatVar(void)
                                { return isMatVar; };

    void                      SetIsGlobal(bool isG)
                                { isGlobal = isG; };
    bool                      GetIsGlobal(void)
                                { return isGlobal; };

    void                      InitSRContainers(int);
    void                      AddSubrecordInfo(int, 
                                               int, 
                                               int,
                                               int, 
                                               int *);
    SubrecInfo               &GetSubrecordInfo(int); 
    vector<int>              &GetSubrecordIds(int);
    
    const string             &GetPath(void);

    void                      AddVectorComponent(string compName)
                                { vectorComponents.push_back(compName); };
    string                    GetVectorComponent(int);
    vector<string>           &GetVectorComponents(void)
                                { return vectorComponents; };

  private:
    
    string                       longName;
    string                       shortName;
    string                       classLName;
    string                       classSName;
    string                       path;
    
    int                          cellTypeAvt;
    int                          cellTypeMili;

    avtCentering                 centering;
    int                          componentDims;
    int                          vectorSize;
    int                          meshAssociation;
    int                          numType;
 
    //
    // Subrecord info
    //
    vector< SubrecInfo >         subrecInfo;

    bool                         isElementSet;
    bool                         isMatVar;
    bool                         isGlobal;
    bool                         multiMesh;

    vector<string>               vectorComponents;

};


// ****************************************************************************
//  Class: MiliClassMetaData
//
//  Purpose:
//      A container for mili class meta data. 
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 16, 2019
//
//  Modifications:
//
// ****************************************************************************

class MiliClassMetaData
{

  public:
                                      MiliClassMetaData(int);
                                     ~MiliClassMetaData(void);

    enum ClassType 
      { CELL, NODE, MATERIAL, GLOBAL, SURFACE, PARTICLE, UNKNOWN };

    void                              SetLongName(string lName)
                                        { longName = lName; };
    string                            GetLongName(void)
                                        { return longName; };

    void                              SetShortName(string sName)
                                        { shortName = sName; };
    string                            GetShortName(void)
                                        { return shortName; };

    void                              SetTotalNumElements(int nEl)
                                        { totalNumElements = nEl; };
    int                               GetTotalNumElements(void)
                                        { return totalNumElements; };

    void                              SetNumElements(int, int);
    int                               GetNumElements(int);

    void                              SetSuperClassId(int);
    int                               GetSuperClassId(void)
                                        { return superClassId; };

    vector<string>                    GetMiliVariables(void)
                                        { return variables; };
    void                              AddMiliVariable(string mv)
                                        { variables.push_back(mv); };

    ClassType                         GetClassType(void)
                                        { return classType; };

    void                              SetConnectivityOffset(int dom, 
                                                            int offset);
    int                               GetConnectivityOffset(int dom);

  private:

    string                            longName;
    string                            shortName;
    int                               totalNumElements;
    int                               superClassId;

    vector<string>                    variables;
    vector<int>                       numDomainElements;

    vector<int>                       connectivityOffset;
    ClassType                         classType;

    void                              DetermineType(int);

};


// ****************************************************************************
//  Class: MiliMaterialMetaData
//
//  Purpose:
//      A container for mili material meta data. 
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 16, 2019
//
//  Modifications:
//
// ****************************************************************************

class MiliMaterialMetaData
{
  public:
                                        MiliMaterialMetaData(void);
                                       ~MiliMaterialMetaData(void);

    void                                SetName(string nm)
                                          { name = nm; };
    string                              GetName(void)
                                          { return name; };

    void                                SetColor(float *);
    float                              *GetColor(void) 
                                          { return color; };
 
  private:
    string name;
    float  color[3];
};


// ****************************************************************************
//  Class: avtMiliMetaData
//
//  Purpose:
//      A container storing and accessing all mili meta data. This is the 
//      main interface between avtMiliFileFormat and the meta data info. 
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 16, 2019
//
//  Modifications:
//
// ****************************************************************************

class avtMiliMetaData
{
  public:

    const int MiliCellTypes[8] =
      { M_TRUSS, M_BEAM, M_TRI, M_QUAD, M_TET, M_PYRAMID, M_WEDGE, M_HEX };

                                       avtMiliMetaData(int);
                                      ~avtMiliMetaData(void);

    //TODO: create a method to populate meta data from a json file?

    void                               SetNumClasses(int);
    int                                GetNumClasses(void)
                                         { return numClasses; }; 

    void                               SetNumVariables(int nVars);
    int                                GetNumVariables(void)
                                         { return numVariables; };

    void                               SetNumMaterials(int);
    int                                GetNumMaterials(void)
                                         { return numMaterials; };

    void                               AddClassMD(int,
                                                  MiliClassMetaData *);
    MiliClassMetaData                 *GetClassMD(const char *);
    int                                GetClassMDIdx(const char *);
 
    void                               GetCellTypeCounts(vector<int> &,
                                                         vector<int> &);

    void                               AddVarMD(int, 
                                                MiliVariableMetaData *);
    MiliVariableMetaData              *GetVarMD(int varIdx);
    MiliVariableMetaData              *GetVarMDByShortName(const char *);
    MiliVariableMetaData              *GetVarMDByPath(const char *);
    int                                GetVarMDIdxByShortName(const char *);
    int                                GetVarMDIdxByPath(const char *);

    void                               AddVarSubrecInfo(int,
                                                        int,
                                                        int,
                                                        Subrecord *);

    void                               AddMaterialMD(int, 
                                                     MiliMaterialMetaData *);

    vector<string>                     GetMaterialNames(void);

    int                                GetNumCells(int);
    void                               SetNumCells(int domain, int nCells)
                                         { numCells[domain] = nCells; };

    int                                GetNumNodes(int);
    void                               SetNumNodes(int domain, int nNodes)
                                         { numNodes[domain] = nNodes; };

    int                                GetNumMiliCellTypes(void)
                                         { return numMiliCellTypes; };

    //vector<vector<float>>    GetMaterialColors(void)
    //                                     { return materialColors; };

  private:

    MiliClassMetaData                **miliClasses;
    MiliVariableMetaData             **miliVariables;
    MiliMaterialMetaData             **miliMaterials;

    int                                numDomains;
    int                                numClasses;
    int                                numVariables;
    int                                numMaterials;
    vector<int>                        numCells;
    vector<int>                        numNodes;

    //
    // The number of available mili cell types. 
    //
    const int                          numMiliCellTypes = 8;
};

#endif
