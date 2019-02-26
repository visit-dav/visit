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

    int                   nSR;        
    vector<int>           nElements;
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

                              MiliVariableMetaData(string,
                                                   string,
                                                   string,
                                                   string,
                                                   bool,
                                                   bool,
                                                   bool,
                                                   avtCentering,
                                                   int,
                                                   int,
                                                   int,
                                                   int,
                                                   int,
                                                   int,
                                                   int,
                                                   vector<string> );
                             ~MiliVariableMetaData(void);

    string                    GetLongName(void)
                                { return longName; };

    string                    GetShortName(void)
                                { return shortName; };

    string                    GetClassLongName(void)
                                { return classLName; }; 

    string                    GetClassShortName(void)
                                { return classSName; }; 

    int                       GetAvtCellType(void)
                                { return cellTypeAvt; };

    int                       GetMiliCellType(void)
                                { return cellTypeMili; };

    avtCentering              GetCentering(void)
                                { return centering; };

    int                       GetNumType(void)
                                { return numType; };
         
    int                       GetComponentDims(void) 
                                { return componentDims; }; 

    int                       GetVectorSize(void)
                                { return vectorSize; };

    int                       GetMeshAssociation(void)
                                { return meshAssociation; }; 
    
    bool                      IsElementSet(void)
                                { return isElementSet; };

    bool                      IsMatVar(void)
                                { return isMatVar; };

    bool                      IsGlobal(void)
                                { return isGlobal; };

    bool                      IsSand(void)
                                { return isSand; };

    bool                      IsCause(void)
                                { return isCause; };

    void                      AddSubrecId(int, int);
    vector<int>              &GetSubrecIds(int);
    
    const string             &GetPath(void);

    void                      AddVectorComponent(string compName)
                                { vectorComponents.push_back(compName); };
    string                    GetVectorComponent(int);
    vector<string>           &GetVectorComponents(void)
                                { return vectorComponents; };

    void                      PrintSelf(void);

  private:

    void                      DetermineESStatus(void);
    
    string                    longName;
    string                    shortName;
    string                    esMappedName;
    string                    classLName;
    string                    classSName;
    string                    path;
    
    int                       cellTypeAvt;
    int                       cellTypeMili;

    avtCentering              centering;
    int                       meshAssociation;
    int                       numType;
    int                       vectorSize;
    int                       componentDims;
 
    //
    // Subrecord info
    //
    vector< vector<int> >     SRIds;

    bool                      isElementSet;
    bool                      isMatVar;
    bool                      isGlobal;
    bool                      isSand;
    bool                      isCause;
    bool                      multiMesh;

    vector<string>            vectorComponents;

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
                                      MiliClassMetaData(string,
                                                        string,
                                                        int,
                                                        int,
                                                        int);
                                     ~MiliClassMetaData(void);

    enum ClassType 
      { CELL, NODE, MATERIAL, GLOBAL, SURFACE, PARTICLE, UNKNOWN };

    string                            GetLongName(void)
                                        { return longName; };

    string                            GetShortName(void)
                                        { return shortName; };

    int                               GetTotalNumElements(void)
                                        { return totalNumElements; };

    void                              SetNumElements(int, int);
    int                               GetNumElements(int);

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

    void                              DetermineType();

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
                                        MiliMaterialMetaData(string,
                                                             string);
                                       ~MiliMaterialMetaData(void);

    string                              GetName(void)
                                          { return name; };

    string                              GetColor(void)
                                           { return hexColor; };
 
  private:
    string name;
    float  color[3];
    string hexColor;
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

    void                               SetNumClasses(int);
    int                                GetNumClasses(void)
                                         { return numClasses; }; 

    void                               SetNumVariables(int nVars);
    int                                GetNumVariables(void)
                                         { return numVariables; };

    void                               AddClassMD(int,
                                                  MiliClassMetaData *);
    MiliClassMetaData                 *GetClassMDByShortName(const char *);
    int                                GetClassMDIdxByShortName(const char *);
 
    void                               GetCellTypeCounts(vector<int> &,
                                                         vector<int> &);

    void                               AddVarMD(int, 
                                                MiliVariableMetaData *);
    MiliVariableMetaData              *GetVarMDByIdx(int varIdx);
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
    void                               SetNumMaterials(int);
    int                                GetNumMaterials(void)
                                         { return numMaterials; };
    void                               GetMaterialNames(stringVector &);
    void                               GetMaterialColors(vector<string> &);

    bool                               ContainsSand(void)
                                         { return containsSand; };

    string                             GetSandDir(void)
                                         { return sandDir; };

    int                                GetNumCells(int);
    void                               SetNumCells(int domain, int nCells)
                                         { numCells[domain] = nCells; };

    int                                GetNumNodes(int);
    void                               SetNumNodes(int domain, int nNodes)
                                         { numNodes[domain] = nNodes; };

    int                                GetNumMiliCellTypes(void)
                                         { return numMiliCellTypes; };

    SubrecInfo                        &GetSubrecInfo(int);

  private:

    void                               AddSubrecInfo(int, 
                                                     int,
                                                     int, 
                                                     int *);

    MiliClassMetaData                **miliClasses;
    MiliVariableMetaData             **miliVariables;
    MiliMaterialMetaData             **miliMaterials;

    int                                numDomains;
    int                                numClasses;
    int                                numVariables;
    int                                numMaterials;
    bool                               containsSand;
    string                             sandDir;
    vector<int>                        numCells;
    vector<int>                        numNodes;
    vector<int>                        sandedVarIdxs;//TODO: needed?

    //
    // Subrecord info
    //
    vector< SubrecInfo >               subrecInfo;

    //
    // The number of available mili cell types. 
    //
    const int                          numMiliCellTypes = 8;
};

#endif
