/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                             avtDatabaseMetaData.h                         //
// ************************************************************************* //

#ifndef _AVT_DATABASE_METADATA_H_
#define _AVT_DATABASE_METADATA_H_
#include <dbatts_exports.h>

#include <visitstream.h>
#include <vectortypes.h>

#include <avtTypes.h>
#include <AttributeSubject.h>
#include <ExpressionList.h>

class avtSimulationInformation;

//----------------------------------------------------------------------------
//  Class: avtMeshMetaData
//
//  Modifications:
//
//    Mark C. Miller, August 9, 2004
//    Added containsGlobalZoneIds data member
//
//    Hank Childs, Mon Feb 14 14:11:29 PST 2005
//    Added originalName.
//
//    Jeremy Meredith, Thu Aug 25 09:28:10 PDT 2005
//    Added origin for groups.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added loadBalanceScheme
//
//    Kathleen Bonnell, Fri Feb  3 10:23:36 PST 2006 
//    Added meshCoordType.
//
//    Jeremy Meredith, Fri Aug 25 17:16:38 EDT 2006
//    Added nodesAreCritical and unitCellVectors.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//----------------------------------------------------------------------------
struct DBATTS_API avtMeshMetaData : public AttributeSubject
{
    std::string   name;
    std::string   originalName;
    std::string   blockTitle; 
    std::string   blockPieceName;
    int           numBlocks;
    int           blockOrigin;  // The origin for blocks/domains.
    int           groupOrigin;  // The origin for groups.
    int           cellOrigin;   // The origin for cells within a block.

    int           numGroups;
    intVector     groupIds;     // Entry i is the group id for the ith block.
    std::string   groupTitle;
    std::string   groupPieceName;

    int           spatialDimension;      // the physical or global dimension
    int           topologicalDimension;  // the parametric or local dimension
    avtMeshType   meshType;
    stringVector  blockNames;
    bool          disjointElements;      // A mesh of disjoint cubes, etc.
    avtGhostType  containsGhostZones;
    bool          containsOriginalCells;
    bool          containsOriginalNodes;
    bool          containsGlobalNodeIds;
    bool          containsGlobalZoneIds;

    bool          hasSpatialExtents;
    double        minSpatialExtents[3];
    double        maxSpatialExtents[3];

    std::string   xUnits;
    std::string   yUnits;
    std::string   zUnits;

    std::string   xLabel;
    std::string   yLabel;
    std::string   zLabel;

    bool          validVariable;
    LoadBalanceScheme loadBalanceScheme;
    avtMeshCoordType meshCoordType;
    bool          nodesAreCritical;
    float         unitCellVectors[9];
    bool          rectilinearGridHasTransform;
    double        rectilinearGridTransform[16];

public:
    avtMeshMetaData();
    avtMeshMetaData(const double *, std::string, int, int, int, int, int, int,
                    avtMeshType);
    avtMeshMetaData(std::string, int, int, int, int, int, int, avtMeshType);
    avtMeshMetaData(const avtMeshMetaData&);
    virtual ~avtMeshMetaData();
    const avtMeshMetaData &operator=(const avtMeshMetaData&);
    virtual void SelectAll();
    void SetExtents(const double *);
    void UnsetExtents() { hasSpatialExtents = false; };
    void Print(ostream &, int = 0) const;
};

//----------------------------------------------------------------------------
//  Class: avtScalarMetaData
//
//  Modifications:
//
//    Hank Childs, Mon Feb 14 14:11:29 PST 2005
//    Added originalName.
//
//    Jeremy Meredith, Tue Aug 22 15:21:30 EDT 2006
//    Added support for scalars representing enumerations.
//
//----------------------------------------------------------------------------
struct DBATTS_API avtScalarMetaData : public AttributeSubject
{
    std::string   name;
    std::string   originalName;
    std::string   meshName;

    avtCentering  centering;

    bool          hasDataExtents;
    double        minDataExtents;
    double        maxDataExtents;

    bool          validVariable;
    bool          treatAsASCII;

    bool          hasUnits;
    std::string   units;

    bool          isEnumeration;
    stringVector  enumNames;
    intVector     enumValues;

public:
    avtScalarMetaData();
    avtScalarMetaData(std::string, std::string, avtCentering);
    avtScalarMetaData(std::string, std::string, avtCentering, double, double);
    avtScalarMetaData(const avtScalarMetaData&);
    virtual ~avtScalarMetaData();
    const avtScalarMetaData &operator=(const avtScalarMetaData&);
    virtual void SelectAll();
    void SetExtents(const double *);
    void UnsetExtents() { hasDataExtents = false; };
    void Print(ostream &, int = 0) const;
};

//----------------------------------------------------------------------------
//  Class: avtVectorMetaData
//
//  Modifications:
//
//    Hank Childs, Mon Feb 14 14:11:29 PST 2005
//    Added originalName.
//
//----------------------------------------------------------------------------
struct DBATTS_API avtVectorMetaData : public AttributeSubject
{
    std::string          name;
    std::string          originalName;
    std::string          meshName;

    avtCentering         centering;
    int                  varDim;

    bool                 hasDataExtents;
    double               minDataExtents;
    double               maxDataExtents;

    bool                 validVariable;

    bool                 hasUnits;
    std::string          units;
public:
    avtVectorMetaData();
    avtVectorMetaData(std::string, std::string, avtCentering, int);
    avtVectorMetaData(std::string, std::string, avtCentering, int, 
                      const double *);
    avtVectorMetaData(const avtVectorMetaData&);
    virtual ~avtVectorMetaData();
    const avtVectorMetaData &operator=(const avtVectorMetaData&);
    virtual void SelectAll();
    void SetExtents(const double *);
    void UnsetExtents() { hasDataExtents = false; };
    void Print(ostream &, int = 0) const;
};

//----------------------------------------------------------------------------
//  Class: avtArrayMetaData
//
//----------------------------------------------------------------------------
struct DBATTS_API avtArrayMetaData : public AttributeSubject
{
    std::string          name;
    std::string          originalName;
    std::string          meshName;

    avtCentering         centering;
    int                  nVars;
    stringVector         compNames;

    bool                 validVariable;

    bool                 hasUnits;
    std::string          units;
public:
    avtArrayMetaData();
    avtArrayMetaData(std::string, std::string, avtCentering, int);
    avtArrayMetaData(std::string, std::string, avtCentering, int, 
                     std::vector<std::string> &);
    avtArrayMetaData(const avtArrayMetaData&);
    virtual ~avtArrayMetaData();
    const avtArrayMetaData &operator=(const avtArrayMetaData&);
    virtual void SelectAll();
    void Print(ostream &, int = 0) const;
};

//----------------------------------------------------------------------------
//  Class: avtTensorMetaData
//
//  Modifications:
//
//    Hank Childs, Mon Feb 14 14:11:29 PST 2005
//    Added originalName.
//
//----------------------------------------------------------------------------
struct DBATTS_API avtTensorMetaData : public AttributeSubject
{
    std::string   name;
    std::string   originalName;
    std::string   meshName;
    int           dim;
    avtCentering  centering;
    bool          validVariable;
    bool          hasUnits;
    std::string   units;
public:
    avtTensorMetaData();
    avtTensorMetaData(std::string, std::string, avtCentering, int);
    avtTensorMetaData(const avtTensorMetaData&);
    virtual ~avtTensorMetaData();
    const avtTensorMetaData &operator=(const avtTensorMetaData&);
    virtual void SelectAll();
    void Print(ostream &, int = 0) const;
};

//----------------------------------------------------------------------------
//  Class: avtSymmetricTensorMetaData
//
//  Modifications:
//
//    Hank Childs, Mon Feb 14 14:11:29 PST 2005
//    Added originalName.
//
//----------------------------------------------------------------------------
struct DBATTS_API avtSymmetricTensorMetaData : public AttributeSubject
{
    std::string   name;
    std::string   originalName;
    std::string   meshName;
    int           dim;
    avtCentering  centering;

    bool          validVariable;

    bool          hasUnits;
    std::string   units;
public:
    avtSymmetricTensorMetaData();
    avtSymmetricTensorMetaData(std::string, std::string, avtCentering, int);
    avtSymmetricTensorMetaData(const avtSymmetricTensorMetaData&);
    virtual ~avtSymmetricTensorMetaData();
    const avtSymmetricTensorMetaData 
                                &operator=(const avtSymmetricTensorMetaData&);
    virtual void SelectAll();
    void Print(ostream &, int = 0) const;
};

//----------------------------------------------------------------------------
//  Class: avtMaterialMetaData
//
//  Modifications:
//
//    Hank Childs, Mon Feb 14 14:11:29 PST 2005
//    Added originalName.
//
//    Mark C. Miller, Thu Jul 13 22:41:56 PDT 2006
//    Added colorNames
//
//----------------------------------------------------------------------------
struct DBATTS_API avtMaterialMetaData : public AttributeSubject
{
    std::string   name;
    std::string   originalName;
    std::string   meshName;

    int           numMaterials;
    stringVector  materialNames;
    stringVector  colorNames;
    
    bool          validVariable;

public:
    avtMaterialMetaData();
    avtMaterialMetaData(std::string, std::string, int, 
                        stringVector);
    avtMaterialMetaData(std::string, std::string, int, 
                        stringVector, stringVector);
    avtMaterialMetaData(const avtMaterialMetaData&);
    virtual ~avtMaterialMetaData();
    const avtMaterialMetaData &operator=(const avtMaterialMetaData&);
    virtual void SelectAll();
    void Print(ostream &, int = 0) const;
};

//----------------------------------------------------------------------------
//  Class: avtSpeciesMetaData
//
//  Modifications:
//
//    Hank Childs, Mon Feb 14 14:11:29 PST 2005
//    Added originalName.
//
//----------------------------------------------------------------------------
struct DBATTS_API avtMatSpeciesMetaData : public AttributeSubject
{
    int          numSpecies;
    stringVector speciesNames;

    bool         validVariable;

public:
    avtMatSpeciesMetaData();
    avtMatSpeciesMetaData(int, stringVector);
    avtMatSpeciesMetaData(const avtMatSpeciesMetaData&);
    virtual ~avtMatSpeciesMetaData();
    const avtMatSpeciesMetaData &operator=(const avtMatSpeciesMetaData&);
    virtual void SelectAll();
};

struct DBATTS_API avtSpeciesMetaData : public AttributeSubject
{
    std::string                          name;
    std::string                          originalName;
    std::string                          meshName;
    std::string                          materialName;
    int                                  numMaterials;
    std::vector<avtMatSpeciesMetaData*>  species;

    bool                                 validVariable;

public:
    avtSpeciesMetaData();
    avtSpeciesMetaData(std::string, std::string, std::string, int,
                     intVector, std::vector<stringVector >);
    avtSpeciesMetaData(const avtSpeciesMetaData&);
    virtual ~avtSpeciesMetaData();
    const avtSpeciesMetaData &operator=(const avtSpeciesMetaData&);
    virtual void SelectAll();
    void Print(ostream &, int = 0) const;
protected:
    virtual AttributeGroup *CreateSubAttributeGroup(int);
};

//----------------------------------------------------------------------------
//  Class: avtCurveMetaData
//
//  Modifications:
//
//    Hank Childs, Mon Feb 14 14:11:29 PST 2005
//    Added originalName.
//
//    Kathleen Bonnell, Thu Aug  3 08:42:33 PDT 2006 
//    Added centering and Data Extents. 
//
//----------------------------------------------------------------------------
struct DBATTS_API avtCurveMetaData : public AttributeSubject
{
    std::string                          name;
    std::string                          originalName;
    std::string                          xUnits;
    std::string                          xLabel;
    std::string                          yUnits;
    std::string                          yLabel;

    avtCentering                         centering;

    bool                                 validVariable;

    bool                                 hasDataExtents;
    double                               minDataExtents;
    double                               maxDataExtents;


public:
    avtCurveMetaData();
    avtCurveMetaData(std::string);
    avtCurveMetaData(std::string, double, double);
    avtCurveMetaData(const avtCurveMetaData&);
    virtual ~avtCurveMetaData();
    const avtCurveMetaData &operator=(const avtCurveMetaData&);
    virtual void SelectAll();
    void SetExtents(const double *);
    void UnsetExtents() { hasDataExtents = false; };
    void Print(ostream &, int = 0) const;
};


//----------------------------------------------------------------------------
//  Class: avtSILCollectionMetaData
//----------------------------------------------------------------------------
struct DBATTS_API avtSILCollectionMetaData : public AttributeSubject
{
   std::string                         classOfCollection;
   std::string                         defaultMemberBasename;
   int                                 collectionSize;
   int                                 collectionIdOfParent;
   int                                 indexOfParent;
   int                                 collectionIdOfChildren;
   intVector                           indicesOfChildren;

public:
    typedef enum { 
       Class,
       PureCollection,
       CollectionAndSets,
       Unknown
    } CollectionType;

    avtSILCollectionMetaData();
    avtSILCollectionMetaData(const avtSILCollectionMetaData&);
    avtSILCollectionMetaData(
       std::string _classOfCollection, std::string _defaultMemberBasename,
       int _collectionSize,
       int _collectionIdOfParent, int _indexOfParent,
       int _collectionIdOfChildren, int *_indicesOfChildren);
    virtual ~avtSILCollectionMetaData();
    const avtSILCollectionMetaData &operator=(const avtSILCollectionMetaData&);
    virtual void SelectAll();
    void Print(ostream &, int = 0) const;
    int GetSize(void) const { return collectionSize; };
    CollectionType GetType(void) const;
    const std::string& GetClassName(void) const { return classOfCollection; };
};

//----------------------------------------------------------------------------
//  Class: avtSILMetaData
//----------------------------------------------------------------------------
struct DBATTS_API avtSILMetaData : public AttributeSubject
{
    std::string                             meshName;

    std::vector<int>                        classIds;
    std::vector<int>                        classDisjointFlags;
    int                                     theStorageChunkClassId;
    std::vector<avtSILCollectionMetaData*>  collections;

protected:
    virtual AttributeGroup *CreateSubAttributeGroup(int);

public:
    avtSILMetaData();
    avtSILMetaData(std::string _meshName);
    avtSILMetaData(const avtSILMetaData&);
    virtual ~avtSILMetaData();
    const avtSILMetaData &operator=(const avtSILMetaData&);
    virtual void SelectAll();
    void Print(ostream &, int = 0) const;

    int GetCollectionClassId(std::string& className) const;

    int AddCollectionClass(std::string className, std::string defaultMemberBasename,
           int numSetsInClass, int pairwiseDisjoint = 0,
           bool hideFromWhole = false, bool isStorageChunkClass = false);

    int AddCollection(std::string classOfCollection,
           std::string defaultMemberBasename, int collectionSize,
           int collectionIdOfParent, int indexOfParent,
           int collectionIdOfChildren = 0, int *indicesOfChildren = NULL);

    void Validate(void);
};

//----------------------------------------------------------------------------
//  Class: avtLabelMetaData
//
//  Modifications:
//
//----------------------------------------------------------------------------
struct DBATTS_API avtLabelMetaData : public AttributeSubject
{
    std::string   name;
    std::string   originalName;
    std::string   meshName;
    avtCentering  centering;
    bool          validVariable;
public:
    avtLabelMetaData();
    avtLabelMetaData(const std::string &, const std::string &, avtCentering);
    avtLabelMetaData(const avtLabelMetaData&);
    virtual ~avtLabelMetaData();
    const avtLabelMetaData &operator=(const avtLabelMetaData&);
    virtual void SelectAll();
    void Print(ostream &, int = 0) const;
};

//----------------------------------------------------------------------------
//  Class: avtDefaultPlotMetaData
//----------------------------------------------------------------------------
struct DBATTS_API avtDefaultPlotMetaData : public AttributeSubject
{
    std::string   pluginID;
    std::string   plotVar;
    stringVector  plotAttributes;
    bool          validVariable;

public:
    avtDefaultPlotMetaData();
    avtDefaultPlotMetaData(std::string, std::string);
    avtDefaultPlotMetaData(const avtDefaultPlotMetaData&);
    virtual ~avtDefaultPlotMetaData();
    const avtDefaultPlotMetaData &operator=(const avtDefaultPlotMetaData&);
    void AddAttribute(const std::string&);
    virtual void SelectAll();
    void Print(ostream &, int = 0) const;
};

//----------------------------------------------------------------------------
//  Class: avtDatabaseMetaData
//
//  Modifications:
//    Mark C. Miller, Tue Aug 10 14:16:36 PDT 2004
//    Added method GetContainsGhosts()
//
//    Kathleen Bonnell, Wed Aug 25 08:37:12 PDT 2004 
//    Added method GetAllVariableNames. 
//
//    Jeremy Meredith, Thu Aug 12 11:39:38 PDT 2004
//    changed simulation information to be a separate class.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added formatCanDoDomainDecomposition and Set/Get methods
//
//    Kathleen Bonnell, Tue Jan 25 07:59:28 PST 2005 
//    Added GetAllMeshNames. 
//
//    Hank Childs, Mon Feb 14 14:11:29 PST 2005
//    Added support for forbidden characters.
//
//    Brad Whitlock, Fri Apr 1 15:04:48 PST 2005
//    Added support for avtLabelMetaData.
//
//    Jeremy Meredith, Thu Apr 28 17:48:21 PDT 2005
//    Added a non-const accessor for the simulation info.
//
//    Brad Whitlock, Mon May 2 19:07:56 PST 2005
//    Added some boilerplate AttributeSubject methods.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    const qualified args to SetCycles/SetTimes
//    Added AreAllCycles/TimesAccurateAndValid
//
//    Hank Childs, Tue Jul 19 11:04:49 PDT 2005
//    Add array variables.
//
//    Hank Childs, Sun Feb 19 10:57:47 PST 2006
//    Add a Boolean array to "DetermineVarType", since the database does not
//    want results from expressions.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added ConvertCSGDomainToBlockAndRegion due to manner in which CSG
//    mesh "domains" are handled
//----------------------------------------------------------------------------

class DBATTS_API avtDatabaseMetaData : public AttributeSubject
{
    bool         hasTemporalExtents;
    double       minTemporalExtents;
    double       maxTemporalExtents;
    int          numStates;
    bool         isVirtualDatabase;
    bool         mustRepopulateOnStateChange;
    bool         mustAlphabetizeVariables;
    bool         formatCanDoDomainDecomposition;

    bool         useCatchAllMesh;

    std::string  timeStepPath;
    stringVector timeStepNames;
    intVector    cycles;
    intVector    cyclesAreAccurate;
    doubleVector times;
    intVector    timesAreAccurate;

    std::string  databaseName;
    std::string  fileFormat;
    std::string  databaseComment;

    ExpressionList exprList;

    std::vector<avtMeshMetaData *>              meshes;
    std::vector<avtScalarMetaData *>            scalars;
    std::vector<avtVectorMetaData *>            vectors;
    std::vector<avtTensorMetaData *>            tensors;
    std::vector<avtSymmetricTensorMetaData *>   symm_tensors;
    std::vector<avtArrayMetaData *>             arrays;
    std::vector<avtMaterialMetaData *>          materials;
    std::vector<avtSpeciesMetaData *>           species;
    std::vector<avtCurveMetaData *>             curves;
    std::vector<avtLabelMetaData *>             labels;
    std::vector<avtDefaultPlotMetaData *>       defaultPlots;
    std::vector<avtSILMetaData *>               sils;

    bool                                        isSimulation;
    avtSimulationInformation                   *simInfo;

public:
    avtDatabaseMetaData();
    avtDatabaseMetaData(const avtDatabaseMetaData&);
    const avtDatabaseMetaData &operator=(const avtDatabaseMetaData&);
    virtual ~avtDatabaseMetaData();

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    int          GetNumStates(void) const { return numStates; };
    void         SetNumStates(int);
    void         SetTemporalExtents(double, double);

    void         SetMustRepopulateOnStateChange(bool mode)
                     { mustRepopulateOnStateChange = mode; };
    bool         GetMustRepopulateOnStateChange(void) const
                     { return mustRepopulateOnStateChange; };

    void         SetMustAlphabetizeVariables(bool mode)
                     { mustAlphabetizeVariables = mode; };
    bool         GetMustAlphabetizeVariables() const
                     { return mustAlphabetizeVariables; };

    bool         GetUseCatchAllMesh(void) const
                     { return useCatchAllMesh; };
    void         SetUseCatchAllMesh(bool useIt)
                     { useCatchAllMesh = useIt; };

    bool         GetFormatCanDoDomainDecomposition(void) const
                     { return formatCanDoDomainDecomposition; };
    void         SetFormatCanDoDomainDecomposition(bool can);

    void         SetIsVirtualDatabase(bool val) { isVirtualDatabase = val; };
    bool         GetIsVirtualDatabase() const { return isVirtualDatabase; };

    void         SetIsSimulation(bool val) { isSimulation = val; }
    bool         GetIsSimulation() const { return isSimulation; }
    void         SetSimInfo(const avtSimulationInformation&);
    const avtSimulationInformation &GetSimInfo() const;
          avtSimulationInformation &GetSimInfo();

    const intVector &GetCycles() const { return cycles; };
    void         SetCycle(int, int);
    void         SetCycles(const intVector &);
    void         SetCycleIsAccurate(bool, int);
    void         SetCyclesAreAccurate(bool);
    bool         IsCycleAccurate(int) const;
    bool         AreAllCyclesAccurateAndValid(int=-1) const;

    const doubleVector &GetTimes() const { return times; };
    void         SetTime(int, double);
    void         SetTimes(const doubleVector &);
    void         SetTimeIsAccurate(bool, int);
    void         SetTimesAreAccurate(bool);
    bool         IsTimeAccurate(int) const;
    bool         AreAllTimesAccurateAndValid(int=-1) const;

    void         SetDatabaseName(const std::string &dsn) {databaseName = dsn;};
    const std::string &GetDatabaseName() const { return databaseName; };

    void         SetFileFormat(const std::string &ff) {fileFormat = ff;};
    const std::string &GetFileFormat() const { return fileFormat; };

    void         SetTimeStepPath(const std::string &tsp);
    const std::string &GetTimeStepPath() const { return timeStepPath; };
    void         SetTimeStepNames(const stringVector &tsn);
    const stringVector &GetTimeStepNames() const { return timeStepNames; };

    void         SetDatabaseComment(const std::string &comment)
                     { databaseComment = comment; };
    const std::string &GetDatabaseComment() const { return databaseComment; };

    void         ReplaceForbiddenCharacters(std::vector<char> &,
                                            std::vector<std::string> &);

    void         Add(avtMeshMetaData *);
    void         Add(avtScalarMetaData *);
    void         Add(avtVectorMetaData *);
    void         Add(avtTensorMetaData *);
    void         Add(avtSymmetricTensorMetaData *);
    void         Add(avtArrayMetaData *);
    void         Add(avtMaterialMetaData *);
    void         Add(avtSpeciesMetaData *);
    void         Add(avtCurveMetaData *);
    void         Add(avtSILMetaData *);
    void         Add(avtLabelMetaData *);
    void         Add(avtDefaultPlotMetaData *);

    int GetNumMeshes()        const { return meshes.size();       };
    int GetNumScalars()       const { return scalars.size();      };
    int GetNumVectors()       const { return vectors.size();      };
    int GetNumTensors()       const { return tensors.size();      };
    int GetNumSymmTensors()   const { return symm_tensors.size();      };
    int GetNumArrays()        const { return arrays.size();      };
    int GetNumMaterials()     const { return materials.size();    };
    int GetNumSpecies()       const { return species.size();      };
    int GetNumCurves()        const { return curves.size();       };
    int GetNumSILs()          const { return sils.size();         };
    int GetNumLabels()        const { return labels.size();       };
    int GetNumDefaultPlots()  const { return defaultPlots.size(); };

    const avtMeshMetaData        *GetMesh(int) const;
    const avtMeshMetaData        *GetMesh(const std::string&) const;
    const avtScalarMetaData      *GetScalar(int) const;
    const avtScalarMetaData      *GetScalar(const std::string&) const;
    const avtVectorMetaData      *GetVector(int) const;
    const avtVectorMetaData      *GetVector(const std::string&) const;
    const avtTensorMetaData      *GetTensor(int) const;
    const avtTensorMetaData      *GetTensor(const std::string&) const;
    const avtSymmetricTensorMetaData *GetSymmTensor(int) const;
    const avtSymmetricTensorMetaData *GetSymmTensor(const std::string&) const;
    const avtArrayMetaData       *GetArray(int) const;
    const avtArrayMetaData       *GetArray(const std::string&) const;
    const avtMaterialMetaData    *GetMaterial(int) const;
    const avtMaterialMetaData    *GetMaterial(const std::string&) const;
    const avtSpeciesMetaData     *GetSpecies(int) const;
    const avtSpeciesMetaData     *GetSpecies(const std::string&) const;
    const avtCurveMetaData       *GetCurve(int) const;
    const avtCurveMetaData       *GetCurve(const std::string&) const;
    const avtSILMetaData         *GetSIL(int) const;
    const avtSILMetaData         *GetSIL(const std::string&) const;
    const avtLabelMetaData       *GetLabel(int) const;
    const avtLabelMetaData       *GetLabel(const std::string&) const;
    const avtDefaultPlotMetaData *GetDefaultPlot(int) const;

    void         SetBlocksForMesh(int index, int nBlocks);
    void         SetContainsGhostZones(std::string name, avtGhostType);
    avtGhostType GetContainsGhostZones(std::string name) const;
    void         SetContainsOriginalCells(std::string name, bool);
    void         SetContainsOriginalNodes(std::string name, bool);
    void         SetContainsGlobalNodeIds(std::string name, bool);
    void         SetContainsGlobalZoneIds(std::string name, bool);
    void         AddGroupInformation(int nGroups, int nBlocks,
                                     intVector &blockIds);
    void         UnsetExtents();

    void                AddExpression(Expression *);
    const Expression   *GetExpression(int) const;
    int                 GetNumberOfExpressions(void) const;

    bool         ConvertCSGDomainToBlockAndRegion(
                     const char *const var, int *domain, int *region) const; 

    int          GetNDomains(std::string) const;
    avtVarType   DetermineVarType(std::string, bool = true) const;
    avtSubsetType   DetermineSubsetType(const std::string &) const;
    std::string  MeshForVar(std::string) const;
    std::string  MaterialOnMesh(std::string) const;
    std::string  SpeciesOnMesh(std::string) const;

    const avtMaterialMetaData *GetMaterialOnMesh(std::string) const;
    const avtSpeciesMetaData  *GetSpeciesOnMesh(std::string) const;
    const avtSILMetaData      *GetSILForMesh(std::string) const;

    void         Print(ostream &, int = 0) const;

    void         SetExtents(std::string, const double *);

    virtual void SelectAll();
    virtual AttributeGroup *CreateSubAttributeGroup(int);

    const stringVector GetAllVariableNames(const std::string &) const;
    const stringVector GetAllMeshNames(void) const;

    void  RegisterWarningCallback(void (*)(const char *));

private:
    bool VarIsCompound(const std::string &inVar) const;
    void ParseCompoundForVar(const std::string &inVar, std::string &outVar)
                                                                   const;
    void ParseCompoundForMesh(const std::string &inVar, std::string &meshName)
                                                                   const;
    void ParseCompoundForCategory(const std::string &inVar, 
                                            std::string &meshName) const;

    static void (*WarningCallback)(const char *);
    static bool haveWarningCallback;
    void        IssueWarning(const char *);
};


#endif
