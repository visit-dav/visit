// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

using std::string;

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
    string       shortName;
    intVector    variableIndicies;
    bool         isAllES;
    bool         isLive;
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

    int                      numSubrecs;        
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
//      Special cases:
//          material variables: defined on materials only. 
//          global: a single value across the entire mesh.
//          shared: a variable that is shared across cell types. 
//          element sets: arrays of integration points. Before rendering, 
//              a single integration point is chosen for display. 
//          sand: elements that have been destroyed during a simulation. 
//          cause: a value denoting the cause of destruction. 
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

    string                     GetLongName(void)
                                 { return longName; };

    string                     GetShortName(void)
                                 { return shortName; };

    string                     GetClassShortName(void)
                                 { return classSName; };

    string                     GetClassLongName(void)
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
    intVector                  GetSubrecIds(int);
    
    virtual const string       &GetPath(void);

    void                       AddVectorComponent(string compName)
                                 { vectorComponents.push_back(compName); };
    string                     GetVectorComponent(int);
    stringVector              &GetVectorComponents(void)
                                 { return vectorComponents; };

    void                       PrintSelf(void);

  protected:

    string                    classLName;
    string                    classSName;
    string                    longName;
    string                    shortName;
    string                    path;
    
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

                      MiliElementSetMetaData(string,
                                             string,
                                             string,
                                             string,
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

    virtual const string           &GetPath(void);
    string                          GetGroupPath(int);
    string                          GetGroupPath(string);
    const stringVector             &GetGroupPaths(void)
                                      { return groupPaths; };

    int                             GetGroupIdxByPath(const char *);

    string                          GetGroupShortName(int);
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

    intVector                       GetGroupComponentIdxs(string);
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
//      Mili variables are defined and retrieved by "Classes" they are 
//      associated with. Some of the classes are cell types, others are
//      material, node, etc. Variables can be defined across multiple 
//      classes.  
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
                                                        int);
                                     ~MiliClassMetaData(void);

    enum ClassType 
      { CELL, NODE, MATERIAL, GLOBAL, SURFACE, PARTICLE, UNKNOWN };

    string                            GetLongName(void)
                                        { return longName; };

    string                            GetShortName(void)
                                        { return shortName; };

    void                              SetNumElements(int, int);
    int                               GetNumElements(int);

    int                               GetSuperClassId(void)
                                        { return superClassId; };

    stringVector                      GetMiliVariables(void)
                                        { return variables; };
    void                              AddMiliVariable(string mv)
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

    string                            longName;
    string                            shortName;
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
                                        MiliMaterialMetaData(string,
                                                             string);
                                       ~MiliMaterialMetaData(void);

    string                              GetName(void)
                                          { return name; };

    string                              GetColor(void)
                                          { return hexColor; };
 
  private:
    string      name;
    float       color[3];
    string      hexColor;
};


// ****************************************************************************
//  Class: avtMiliMetaData
//
//  Purpose:
//      A container for storing and accessing all mili meta data. This is the 
//      main interface between avtMiliFileFormat and the meta data info that
//      is retrieved from the .mili file and the subrecord database. 
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

    static const char               *GetSandDir(void)
                                       { return "sand_mesh"; };

    static bool                      ContainsESFlag(const char *, int);

  private:

    void                             AddSharedVariableInfo(string,
                                                           int,
                                                           bool);

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
    string                           sandDir;
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
