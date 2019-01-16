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

class MiliVariableMetaData
{

  public:
                              MiliVariableMetaData();
                             ~MiliVariableMetaData(void);

    void                      SetLongName(string lName)
                                { longName = lName; };
    string                    GetLongName(void)
                                { return longName; };

    void                      SetShortName(string sName)
                                { shortName = sName; };
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

    void                      SetMeshAssociation(int meshIdx) 
                                { meshAssociation = meshIdx; };
    int                       GetMeshAssociation(void)
                                { return meshAssociation; }; 
    
    void                      SetIsPrimal(bool primal)
                                { isPrimal = primal; };
    bool                      GetIsPrimal(void)
                                { return isPrimal; };

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
    void                      AddSubrecord(int, 
                                           int,
                                           Subrecord); 

    void                      AddSubrecordIds(int,
                                              int);
    vector<int>              &GetSubrecordIds(int);
    

    string                    GetPath(void);

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
    bool                         isPrimal;
    bool                         isElementSet;
    bool                         isMatVar;
    bool                         isGlobal;

    vector< vector<int> >        subrecordIds; 
    vector<string>               vectorComponents;
};


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

    int                               GetConnectivityCount(void)
                                        { return connectivityCount; };

    void                              SetConnectivityOffset(int dom, 
                                                            int offset);
    int                               GetConnectivityOffset(int dom);
    

  private:

    string                            longName;
    string                            shortName;
    int                               totalNumElements;
    int                               superClassId;

    vector<string>                    variables;//TODO: we could probably make this memory malloc. 
    vector<int>                       numDomainElements;

    int                               connectivityCount;
    vector<int>                       connectivityOffset;
    ClassType                         classType;

    void                              DetermineType(int);

};


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


class MiliMetaData
{
  public:

    //TODO: should we put this in mili to ensure future compatibility?
    const int MiliCellTypes[8] =
      { M_TRUSS, M_BEAM, M_TRI, M_QUAD, M_TET, M_PYRAMID, M_WEDGE, M_HEX };

                                       MiliMetaData(int);
                                      ~MiliMetaData(void);

    void                               CleanseSubrecords(void);

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

    void                               AddMiliClassMD(int,
                                                      MiliClassMetaData *);
    MiliClassMetaData                 *GetMiliClassMD(const char *);
    int                                GetMiliClassMDIdx(const char *);
    //FIXME: may not need now                                       
    vector<MiliClassMetaData *>        GetMiliClassMDOfType(int);
 
    void                               GetCellTypeCounts(vector<int> &,
                                                         vector<int> &);

    //FIXME: use this or direct access?
    void                               AddMiliVariableMD(int, 
                                                         MiliVariableMetaData *);
    MiliVariableMetaData              *GetMiliVariableMD(int varIdx);
    MiliVariableMetaData              *GetMiliVariableMD(const char *);
    int                                GetMiliVariableMDIdx(const char *);

    void                               AddMiliVariableSubrecord(int,
                                                                int,
                                                                int);

    void                               AddMiliVariableSubrecord(const char *, 
                                                                int,
                                                                int);

    vector<Subrecord *>                GetMiliVariableSubrecords(int,
                                                                 const char *);
    vector<Subrecord *>                GetMiliVariableSubrecords(int,
                                                                 int);

    void                               AddMiliMaterialMD(MiliMaterialMetaData mmd)
                                         { miliMaterials.push_back(mmd); };
    vector<MiliMaterialMetaData>      &GetAllMiliMaterialMD(void)
                                         { return miliMaterials; };

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

    void                               AddSubrecord(int, Subrecord); 

    void                               InitMaterialAssociations(int size);
                                       

  private:

    //TODO: refactor to use malloc (save space). 
    MiliClassMetaData                **miliClasses;
    MiliVariableMetaData             **miliVariables;
    vector<MiliMaterialMetaData>       miliMaterials;

    int                                numDomains;
    int                                numClasses;
    int                                numVariables;//TODO: we should probably name this "numClassVariables"
                                                    // as it is NOT the total number of variables. 

    //
    // The number of available mili cell types. 
    //
    const int                          numMiliCellTypes = 8;

    int                                numMaterials;
    vector<int>                        numCells;
    vector<int>                        numNodes;

    MapNode                            varIdxMap;
    MapNode                            classIdxMap;

    vector<Subrecord>                  subrecords;
};

#endif
