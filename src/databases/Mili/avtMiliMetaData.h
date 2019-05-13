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

#include <avtTypes.h>
#include <vectortypes.h>

#include <map>

extern "C" {
#include <mili.h>
}

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"



//
// Info needed by the vtkLabel class. 
//
typedef struct LabelPositionInfo
{
    int          numBlocks;
    intVector    rangesBegin;      
    intVector    rangesEnd;
    intVector    idsBegin;
    intVector    idsEnd;
} LabelPositionInfo;


//
// Ease of use info for shared variables. 
//
typedef struct SharedVariableInfo
{
    std::string  shortName;
    intVector    variableIndicies;
} SharedVariableInfo;


// ****************************************************************************
//  Class: SubrecInfo
//
//  Purpose:
//      A container to store info about the subrecords. 
//
//      Subrecords contain information about the data that resides within 
//      a mili database. This container holds a minimal amount of that 
//      information that we need on the visit end.
//
//  Programmer:  Alister Maguire
//  Creation:    May 13, 2019
//
//  Modifications:
//
// ****************************************************************************

class SubrecInfo
{

   public:
                 SubrecInfo(void);
                ~SubrecInfo(void);

    void         AddSubrec(const int,
                           const int,
                           const int,
                           const int *);

    void         GetSubrec(const int,
                           int &, 
                           int &,
                           intVector &);

  private:

    int                      nSR;        
    intVector                numElements;
    intVector                numDataBlocks;
    std::vector< intVector > dataBlockRanges;   

    //
    // Because the subrecords can be segmented across domains, 
    // we need to map the subrecord Ids to their vector indicies. 
    //
    std::map<int, int>       indexMap;
}; 


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

                               MiliVariableMetaData(std::string,
                                                    std::string,
                                                    std::string,
                                                    std::string,
                                                    bool,
                                                    bool,
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
                                                    stringVector);

    virtual                   ~MiliVariableMetaData(void);

    std::string                GetLongName(void)
                                 { return longName; };

    std::string                GetShortName(void)
                                 { return shortName; };

    std::string                GetClassShortName(void)
                                 { return classSName; };

    std::string                GetClassLongName(void)
                                 { return classLName; };

    int                        GetAvtVarType(void)
                                 { return varTypeAvt; };

    int                        GetMiliVarType(void)
                                 { return varTypeMili; };

    avtCentering               GetCentering(void)
                                 { return centering; };

    int                        GetNumType(void)
                                 { return numType; };
         
    int                        GetComponentDims(void) 
                                 { return componentDims; }; 

    int                        GetVectorSize(void)
                                 { return vectorSize; };

    int                        GetMeshAssociation(void)
                                 { return meshAssociation; }; 
    
    bool                       IsElementSet(void)
                                 { return isElementSet; };

    bool                       IsMatVar(void)
                                 { return isMatVar; };

    bool                       IsGlobal(void)
                                 { return isGlobal; };

    bool                       IsSand(void)
                                 { return isSand; };

    bool                       IsCause(void)
                                 { return isCause; };

    bool                       IsShared(void)
                                 { return isShared; };

    void                       AddSubrecId(int, int);
    intVector                 &GetSubrecIds(int);
    
    virtual const std::string &GetPath(void);

    void                       AddVectorComponent(std::string compName)
                                 { vectorComponents.push_back(compName); };
    std::string                GetVectorComponent(int);
    stringVector              &GetVectorComponents(void)
                                 { return vectorComponents; };

    void                       PrintSelf(void);

  protected:

    std::string               classLName;
    std::string               classSName;
    std::string               longName;
    std::string               shortName;
    std::string               path;
    
    int                       varTypeAvt;
    int                       varTypeMili;

    avtCentering              centering;
    int                       meshAssociation;
    int                       numType;
    int                       vectorSize;
    int                       componentDims;
 
    //
    // Subrecord ids
    //
    std::vector< intVector >  SRIds;

    bool                      isElementSet;
    bool                      isMatVar;
    bool                      isGlobal;
    bool                      isSand;
    bool                      isCause;
    bool                      isShared;
    bool                      multiMesh;

    stringVector              vectorComponents;
};


// ****************************************************************************
//  Class: MiliElementSetMetaData
//
//  Purpose:
//      A container for mili element set variables.  
//
//      The primary purpose of this is to extend the MiliVariableMetaData
//      class to handle "groups" within element sets. In a nut-shell, 
//      element sets are considered a single vector variable, but their 
//      vector components are often grouped into separate variables for 
//      visualization purposes. This adds a layer of complications that 
//      this class seeks to help ease.  
//
//  Programmer:  Alister Maguire
//  Creation:    May 6, 2019
//
//  Modifications:
//
// ****************************************************************************

class MiliElementSetMetaData : public MiliVariableMetaData
{

  public:

                      MiliElementSetMetaData(std::string,
                                             std::string,
                                             std::string,
                                             std::string,
                                             bool,
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
                                             stringVector,
                                             stringVector,
                                             intVector,
                                             intVector,
                                             intVector,
                                             boolVector);

                     ~MiliElementSetMetaData(void);

    virtual const std::string      &GetPath(void);
    std::string                     GetGroupPath(int);
    std::string                     GetGroupPath(std::string);
    const stringVector             &GetGroupPaths(void)
                                      { return groupPaths; };

    int                             GetGroupIdxByPath(const char *);

    std::string                     GetGroupShortName(int);
    const stringVector             &GetGroupShortNames(void)
                                      { return groupShortNames; };

    int                             GetGroupVecSize(int);
    const intVector                &GetGroupVecSizes(void)
                                      { return groupVecSizes; };

    stringVector                    GetGroupVecComponents(int);

    const intVector                &GetGroupAvtTypes(void)
                                      { return groupAvtTypes; };

    const intVector                &GetGroupMiliTypes(void)
                                      { return groupMiliTypes; };

    bool                            GroupIsShared(int);
    const boolVector               &GetGroupIsShared(void)
                                      { return groupIsShared; };

    intVector                       GetGroupComponentIdxs(std::string);
    intVector                       GetGroupComponentIdxs(int);

  private:

    int                             numGroups;
    stringVector                    groupShortNames;
    intVector                       groupVecSizes;
    intVector                       groupAvtTypes;
    intVector                       groupMiliTypes;
    std::vector<bool>               groupIsShared;
    stringVector                    groupPaths;
    std::vector< std::vector<int> > groupComponentIdxs;
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
                                      MiliClassMetaData(std::string,
                                                        std::string,
                                                        int,
                                                        int);
                                     ~MiliClassMetaData(void);

    enum ClassType 
      { CELL, NODE, MATERIAL, GLOBAL, SURFACE, PARTICLE, UNKNOWN };

    std::string                       GetLongName(void)
                                        { return longName; };

    std::string                       GetShortName(void)
                                        { return shortName; };

    void                              SetNumElements(int, int);
    int                               GetNumElements(int);

    int                               GetSuperClassId(void)
                                        { return superClassId; };

    stringVector                      GetMiliVariables(void)
                                        { return variables; };
    void                              AddMiliVariable(std::string mv)
                                        { variables.push_back(mv); };

    ClassType                         GetClassType(void)
                                        { return classType; };

    void                              SetConnectivityOffset(int, 
                                                            int);
    int                               GetConnectivityOffset(int);

    void                              PopulateLabelIds(int, 
                                                       int *,
                                                       int,
                                                       int *);

    const LabelPositionInfo          *GetLabelPositionInfoPtr(int);

    bool                              GetElementLabels(int, stringVector &);

    stringVector                     *GetElementLabelsPtr(int);

    int                               GetMaxLabelLength(int);

  private:

    void                              DetermineType(void);

    void                              GenerateElementLabels(int);

    void                              PopulateLabelPositions(int, int, int *);

    std::string                       longName;
    std::string                       shortName;
    int                               superClassId;

    std::vector<LabelPositionInfo>    labelPositions;
    std::vector< intVector >          labelIds;
    std::vector< stringVector >       elementLabels;
    boolVector                        labelsGenerated;
    intVector                         maxLabelLengths;

    stringVector                      variables;
    intVector                         numDomainElements;
    intVector                         connectivityOffset;

    ClassType                         classType;

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
                                        MiliMaterialMetaData(std::string,
                                                             std::string);
                                       ~MiliMaterialMetaData(void);

    std::string                         GetName(void)
                                          { return name; };

    std::string                         GetColor(void)
                                           { return hexColor; };
 
  private:
    std::string name;
    float       color[3];
    std::string hexColor;
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

    void                             SetNumClasses(int);
    int                              GetNumClasses(void)
                                       { return numClasses; }; 

    void                             SetNumVariables(int nVars);
    int                              GetNumVariables(void)
                                       { return numVariables; };

    void                             AddClassMD(int,
                                                MiliClassMetaData *);
    MiliClassMetaData               *GetClassMDByShortName(const char *);
    int                              GetClassMDIdxByShortName(const char *);
    void                             GetCellBasedClassMD(
                                         std::vector<MiliClassMetaData *> &);
    void                             GetNodeBasedClassMD(
                                         std::vector<MiliClassMetaData *> &);
 
    void                             GetCellTypeCounts(intVector &,
                                                       intVector &);

    void                             AddVarMD(int, 
                                              MiliVariableMetaData *);
    MiliVariableMetaData            *GetVarMDByIdx(int varIdx);
    MiliVariableMetaData            *GetVarMDByShortName(const char *, 
                                                         const char *);
    MiliVariableMetaData            *GetVarMDByPath(const char *);
    int                              GetVarMDIdxByShortName(const char *, 
                                                            const char *);
    int                              GetVarMDIdxByPath(const char *);

    void                             AddVarSubrecInfo(int,
                                                      int,
                                                      int,
                                                      Subrecord *);

    void                             AddMaterialMD(int, 
                                                   MiliMaterialMetaData *);
    void                             SetNumMaterials(int);
    int                              GetNumMaterials(void)
                                       { return numMaterials; };
    void                             GetMaterialNames(stringVector &);
    void                             GetMaterialColors(stringVector &);

    bool                             ContainsSand(void)
                                       { return containsSand; };

    std::string                      GetSandDir(void)
                                       { return sandDir; };

    void                             SetNumCells(int, int);
    int                              GetNumCells(int);

    void                             SetNumNodes(int , int);
    int                              GetNumNodes(int);

    int                              GetNumMiliCellTypes(void)
                                       { return numMiliCellTypes; };

    const stringVector              *GetZoneBasedLabelsPtr(int);
    const stringVector              *GetNodeBasedLabelsPtr(int);

    int                              GetMaxZoneLabelLength(int);
    int                              GetMaxNodeLabelLength(int);

    SubrecInfo                      *GetSubrecInfo(int);

    SharedVariableInfo              *GetSharedVariableInfo(const char *);

  private:

    void                             AddSharedVariableInfo(std::string,
                                                           int);

    void                             GenerateZoneBasedLabels(int);
    void                             GenerateNodeBasedLabels(int);


    MiliClassMetaData              **miliClasses;
    MiliVariableMetaData           **miliVariables;
    MiliMaterialMetaData           **miliMaterials;

    int                              numDomains;
    int                              numClasses;
    int                              numVariables;
    int                              numMaterials;
    bool                             containsSand;
    std::string                      sandDir;
    intVector                        numCells;
    intVector                        numNodes;

    std::vector <stringVector >      zoneBasedLabels;
    std::vector <stringVector >      nodeBasedLabels;

    intVector                        maxZoneLabelLengths;
    intVector                        maxNodeLabelLengths;
    boolVector                       zoneLabelsGenerated;
    boolVector                       nodeLabelsGenerated;

    //
    // Subrecord info.
    //
    std::vector< SubrecInfo >        subrecInfo;

    //
    // Shared variable info. 
    //
    std::vector<SharedVariableInfo *> sharedVariables;

    //
    // The number of available mili cell types. 
    //
    const int                         numMiliCellTypes = 8;
};

#endif
