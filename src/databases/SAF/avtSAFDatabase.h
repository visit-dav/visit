// ************************************************************************* //
//                               avtSAFDatabase.h                            //
// ************************************************************************* //

#ifndef AVT_SAF_DATABASE_H
#define AVT_SAF_DATABASE_H
#include <database_exports.h>

#include <avtDatasetDatabase.h>
#include <vtkSystemIncludes.h>

class vtkUnstructuredGrid;

//
// The extern "C" around saf.h shouldn't be necessary but SAF doesn't have
// enough of them in their header files so this is done as a work around.
//
extern "C"
{
#include <saf.h>
}

struct SILNode
{
    SAF_Set            setSet;
    char              *setName;
    int                setId;
    bool               setSelfStored;
    int                setCellType;           // VTK cell type
    SAF_Rel            setTopoRelation;
    int                nCats;
    char             **catNames;
    SILCategoryRole   *catRoles;
    int               *catNSubsets;
    struct SILNode  ***catSubsets;
};

// ****************************************************************************
//  Class: avtSAFDatabase
//
//  Purpose:
//      A stand-in for the avtSAFDatabase class used by VisIt.
//
//  Programmer: Jim Reus
//  Creation:   16Mar2001
//
//  Modifications:
//
//    Hank Childs, Fri Aug 17 16:35:12 PDT 2001
//    Inherited from avtDatasetDatabase.
//
//    Eric Brugger, Thu Mar  7 08:12:09 PST 2002
//    I added the Query method.
//
//    Eric Brugger, Thu Sep  5 11:24:47 PDT 2002
//    Completely rewritten.
//
// ****************************************************************************

class DATABASE_API avtSAFDatabase : public avtDatasetDatabase
{
  public:
                              avtSAFDatabase(const char*);
    virtual                  ~avtSAFDatabase();

    virtual avtDataTree_p     GetOutput(avtDataSpecification_p,
                                        avtSourceFromDatabase *);

    virtual void              GetAuxiliaryData(avtDataSpecification_p,
                                               VoidRefList &,
                                               const char*,
                                               void*);

    virtual void              Query(avtDataSpecification_p,
                                    PickAttributes *);

  protected:

    virtual void              SetDatabaseMetaData(avtDatabaseMetaData*);
    virtual void              PopulateSIL(avtSIL*);

  private:
                              avtSAFDatabase();
                              avtSAFDatabase(const avtSAFDatabase&);

    int                       AddSet(avtSIL*, struct SILNode*, bool);
    void                      GetDataSets(vtkDataSet **, avtSILRestriction_p,
                                          int, SAF_Field, SAF_Field);
    int                       GetFieldNComponents(SAF_Field);
    vtkUnstructuredGrid      *GetUnstructuredGrid(int, SAF_Field, SAF_Field);
    void                      PopulateTopologySets(SILNode *);
    struct SILNode           *ReadSet(SAF_Set, int, SAF_Cat*);
    void                      ReadSetInformation();

    SAF_Db                    db;
    DSL_Boolean_t             initialized;
    DSL_Boolean_t             opened;

    SAF_Cat                   nodeCat;
    SAF_Cat                   edgeCat;
    SAF_Cat                   faceCat;
    SAF_Cat                   elemCat;

    int                       nSuites;
    SAF_Suite                *suites;
    char                    **suiteNames;
    SAF_Field                *suiteCoords;
    size_t                   *suiteCoordsCount;
    int                     **suiteCoordsValues;
    SAF_StateFld             *stateFields;

    int                       nStates;
    SAF_StateTmpl             stateTemplate;
    int                       nFields;
    SAF_Field               **fields;
    char                    **fieldNames;
    int                      *fieldNComponents;
    DSL_Boolean_t            *fieldSelfStored;
    char                    **fieldMeshNames;
    avtCentering             *fieldCentering;
    int                       coords;

    int                       nTopSets;
    SAF_Set                  *topSets;

    struct SILNode           *SIL;
    int                       SILId;

    int                       nTopologySets;
    SILNode                 **topologySets;

    int                       iblock;
};

#endif

