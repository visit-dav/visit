// ************************************************************************* //
//                             avtDatabaseMetaData.h                         //
// ************************************************************************* //

#ifndef _AVT_DATABASE_METADATA_H_
#define _AVT_DATABASE_METADATA_H_
#include <dbatts_exports.h>

#include <iostream.h>
#include <vectortypes.h>

#include <avtTypes.h>
#include <AttributeSubject.h>


//----------------------------------------------------------------------------
//  Class: avtMeshMetaData
//----------------------------------------------------------------------------
struct DBATTS_API avtMeshMetaData : public AttributeSubject
{
    std::string   name;
    std::string   blockTitle; 
    std::string   blockPieceName;
    int           numBlocks;
    int           blockOrigin;  // The origin for blocks/domains.
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

    bool          hasSpatialExtents;
    float         minSpatialExtents[3];
    float         maxSpatialExtents[3];

    std::string   xUnits;
    std::string   yUnits;
    std::string   zUnits;

    bool          validVariable;

public:
    avtMeshMetaData();
    avtMeshMetaData(const float *, std::string, int, int, int, int, int,
                    avtMeshType);
    avtMeshMetaData(std::string, int, int, int, int, int, avtMeshType);
    avtMeshMetaData(const avtMeshMetaData&);
    virtual ~avtMeshMetaData();
    const avtMeshMetaData &operator=(const avtMeshMetaData&);
    virtual void SelectAll();
    void SetExtents(const float *);
    void UnsetExtents() { hasSpatialExtents = false; };
    void Print(ostream &, int = 0) const;
};

//----------------------------------------------------------------------------
//  Class: avtScalarMetaData
//----------------------------------------------------------------------------
struct DBATTS_API avtScalarMetaData : public AttributeSubject
{
    std::string   name;
    std::string   meshName;

    avtCentering  centering;

    bool          hasDataExtents;
    float         minDataExtents;
    float         maxDataExtents;

    bool          validVariable;

public:
    avtScalarMetaData();
    avtScalarMetaData(std::string, std::string, avtCentering);
    avtScalarMetaData(std::string, std::string, avtCentering, float, float);
    avtScalarMetaData(const avtScalarMetaData&);
    virtual ~avtScalarMetaData();
    const avtScalarMetaData &operator=(const avtScalarMetaData&);
    virtual void SelectAll();
    void SetExtents(const float *);
    void UnsetExtents() { hasDataExtents = false; };
    void Print(ostream &, int = 0) const;
};

//----------------------------------------------------------------------------
//  Class: avtVectorMetaData
//----------------------------------------------------------------------------
struct DBATTS_API avtVectorMetaData : public AttributeSubject
{
    std::string          name;
    std::string          meshName;

    avtCentering         centering;
    int                  varDim;

    bool                 hasDataExtents;
    floatVector          minDataExtents;
    floatVector          maxDataExtents;

    bool                 validVariable;

public:
    avtVectorMetaData();
    avtVectorMetaData(std::string, std::string, avtCentering, int);
    avtVectorMetaData(std::string, std::string, avtCentering, int, 
                      const float *);
    avtVectorMetaData(const avtVectorMetaData&);
    virtual ~avtVectorMetaData();
    const avtVectorMetaData &operator=(const avtVectorMetaData&);
    virtual void SelectAll();
    void SetExtents(const float *);
    void UnsetExtents() { hasDataExtents = false; };
    void Print(ostream &, int = 0) const;
};

//----------------------------------------------------------------------------
//  Class: avtMaterialMetaData
//----------------------------------------------------------------------------
struct DBATTS_API avtMaterialMetaData : public AttributeSubject
{
    std::string   name;
    std::string   meshName;

    int           numMaterials;
    stringVector  materialNames;
    
    bool          validVariable;

public:
    avtMaterialMetaData();
    avtMaterialMetaData(std::string, std::string, int, 
                        stringVector);
    avtMaterialMetaData(const avtMaterialMetaData&);
    virtual ~avtMaterialMetaData();
    const avtMaterialMetaData &operator=(const avtMaterialMetaData&);
    virtual void SelectAll();
    void Print(ostream &, int = 0) const;
};

//----------------------------------------------------------------------------
//  Class: avtSpeciesMetaData
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
//----------------------------------------------------------------------------
struct DBATTS_API avtCurveMetaData : public AttributeSubject
{
    std::string                          name;
    bool                                 validVariable;

public:
    avtCurveMetaData();
    avtCurveMetaData(std::string);
    avtCurveMetaData(const avtCurveMetaData&);
    virtual ~avtCurveMetaData();
    const avtCurveMetaData &operator=(const avtCurveMetaData&);
    virtual void SelectAll();
    void Print(ostream &, int = 0) const;
};


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//  Class: avtDatabaseMetaData
//----------------------------------------------------------------------------
class DBATTS_API avtDatabaseMetaData : public AttributeSubject
{
    bool         hasTemporalExtents;
    double       minTemporalExtents;
    double       maxTemporalExtents;
    int          numStates;
    bool         isVirtualDatabase;

    std::string  timeStepPath;
    stringVector timeStepNames;
    intVector    cycles;
    intVector    cyclesAreAccurate;
    doubleVector times;
    intVector    timesAreAccurate;

    stringVector expressionNames;
    stringVector expressionDefns;
    intVector    expressionTypes;

    std::vector<avtMeshMetaData *>     meshes;
    std::vector<avtScalarMetaData *>   scalars;
    std::vector<avtVectorMetaData *>   vectors;
    std::vector<avtMaterialMetaData *> materials;
    std::vector<avtSpeciesMetaData *>  species;
    std::vector<avtCurveMetaData *>    curves;

public:
    avtDatabaseMetaData();
    avtDatabaseMetaData(const avtDatabaseMetaData&);
    const avtDatabaseMetaData &operator=(const avtDatabaseMetaData&);
    virtual ~avtDatabaseMetaData();

    int          GetNumStates(void) const { return numStates; };
    void         SetNumStates(int);
    void         SetTemporalExtents(double, double);

    void         SetIsVirtualDatabase(bool val) { isVirtualDatabase = val; };
    bool         GetIsVirtualDatabase() const { return isVirtualDatabase; };

    const intVector &GetCycles() const { return cycles; };
    void         SetCycle(int, int);
    void         SetCycles(intVector &);
    void         SetCycleIsAccurate(bool, int);
    void         SetCyclesAreAccurate(bool);
    bool         IsCycleAccurate(int) const;

    const doubleVector &GetTimes() const { return times; };
    void         SetTime(int, double);
    void         SetTimes(doubleVector &);
    void         SetTimeIsAccurate(bool, int);
    void         SetTimesAreAccurate(bool);
    bool         IsTimeAccurate(int) const;

    void         SetTimeStepPath(const std::string &tsp);
    const std::string &GetTimeStepPath() const { return timeStepPath; };
    void         SetTimeStepNames(const stringVector &tsn);
    const stringVector &GetTimeStepNames() const { return timeStepNames; };

    void         Add(avtMeshMetaData *);
    void         Add(avtScalarMetaData *);
    void         Add(avtVectorMetaData *);
    void         Add(avtMaterialMetaData *);
    void         Add(avtSpeciesMetaData *);
    void         Add(avtCurveMetaData *);

    int GetNumMeshes()    const { return meshes.size();     };
    int GetNumScalars()   const { return scalars.size();    };
    int GetNumVectors()   const { return vectors.size();    };
    int GetNumMaterials() const { return materials.size();  };
    int GetNumSpecies()   const { return species.size();    };
    int GetNumCurves()    const { return curves.size();    };

    const avtMeshMetaData     *GetMesh(int) const;
    const avtMeshMetaData     *GetMesh(const std::string&) const;
    const avtScalarMetaData   *GetScalar(int) const;
    const avtScalarMetaData   *GetScalar(const std::string&) const;
    const avtVectorMetaData   *GetVector(int) const;
    const avtVectorMetaData   *GetVector(const std::string&) const;
    const avtMaterialMetaData *GetMaterial(int) const;
    const avtMaterialMetaData *GetMaterial(const std::string&) const;
    const avtSpeciesMetaData  *GetSpecies(int) const;
    const avtSpeciesMetaData  *GetSpecies(const std::string&) const;
    const avtCurveMetaData    *GetCurve(int) const;
    const avtCurveMetaData    *GetCurve(const std::string&) const;

    void         SetBlocksForMesh(int index, int nBlocks);
    void         SetContainsGhostZones(std::string name, avtGhostType);
    void         SetContainsOriginalCells(std::string name, bool);
    void         AddGroupInformation(int nGroups, int nBlocks,
                                     intVector &blockIds);
    void         UnsetExtents();

    void         AddExpression(const std::string &, const std::string &,
                               avtVarType);
    void         GetExpression(int, std::string &, std::string &, avtVarType&);
    int          GetNumberOfExpressions(void);

    int          GetNDomains(std::string);
    avtVarType   DetermineVarType(std::string);
    avtSubsetType   DetermineSubsetType(const std::string &);
    std::string  MeshForVar(std::string);
    std::string  MaterialOnMesh(std::string);
    std::string  SpeciesOnMesh(std::string);

    const avtMaterialMetaData *GetMaterialOnMesh(std::string);
    const avtSpeciesMetaData  *GetSpeciesOnMesh(std::string);

    void         Print(ostream &, int = 0) const;

    void         SetExtents(std::string, const float *);

    virtual void SelectAll();
    virtual AttributeGroup *CreateSubAttributeGroup(int);

private:
    bool VarIsCompound(const std::string &inVar);
    void ParseCompoundForVar(const std::string &inVar, std::string &outVar);
    void ParseCompoundForMesh(const std::string &inVar, std::string &meshName);
    void ParseCompoundForCategory(const std::string &inVar, std::string &meshName);
};


#endif
