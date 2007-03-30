// ************************************************************************* //
//                              Field_Prep.h                                 //
// ************************************************************************* //

#ifndef FIELD_PREP_H
#define FIELD_PREP_H

#include <Field.h>
#include <IntervalTree_Prep.h>
#include <Value_Prep.h>


// ****************************************************************************
//  Class: Field_Prep
// 
//  Purpose:
//      A derived class of Field that is meant exclusively for meshtvprep.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 14:21:56 PDT 2000
//      Added aliasedDimsObject.
//
//      Hank Childs, Tue Jun 13 12:05:17 PDT 2000
//      Added routines to allow for separation of creating Field object and
//      interval tree.
//
// ****************************************************************************

class Field_Prep : public Field
{
  public:
                        Field_Prep();
                       ~Field_Prep();

    char               *GetMeshName() { return mesh; };
    void                SetName(char *, char *);
    void                SetMeshName(char *);
    void                SetNDomains(int);

    void                DetermineUcdMeshSize(int, DBfile *, char *);
    void                DeterminePointMeshSize(int, DBfile *, char *);
    void                DetermineQuadMeshSize(int, DBfile *, char *);
    void                DetermineUcdvarSize(int, DBfile *, char *);
    void                DetermineMeshvarSize(int, DBfile *, char *);
    void                DetermineQuadvarSize(int, DBfile *, char *);

    void                ReadVarAndWriteArrays(DBfile *, DBmeshvar *, int);
    void                ReadVarAndWriteArrays(DBfile *, DBpointmesh *, int);
    void                ReadVarAndWriteArrays(DBfile *, DBquadmesh *, int);
    void                ReadVarAndWriteArrays(DBfile *, DBquadvar *, int);
    void                ReadVarAndWriteArrays(DBfile *, DBucdmesh *, int);
    void                ReadVarAndWriteArrays(DBfile *, DBucdvar *, int);

    void                IntervalTreeReadVar(DBpointmesh *, int);
    void                IntervalTreeReadVar(DBmeshvar *, int);
    void                IntervalTreeReadVar(DBquadmesh *, int);
    void                IntervalTreeReadVar(DBquadvar *, int);
    void                IntervalTreeReadVar(DBucdmesh *, int);
    void                IntervalTreeReadVar(DBucdvar *, int);

    void                Consolidate(void);
    void                WrapUp(void);
    void                WrapUpIntervalTree(void);
    virtual void        Write(DBfile *);
    virtual void        WriteIntervalTree(DBfile *);

    IntervalTree_Prep  *GetIntervalTree(void)  
                            { return aliasedIntervalTreeObject; };

  protected:
    bool                readCentering;
    bool                readDataType;
    bool                readMeshName;
    bool                readUnits;

    Value_Prep         *aliasedValueObject;
    Value_Prep         *aliasedDimsObject;
    Value_Prep         *aliasedMixedValueObject;
    IntervalTree_Prep  *aliasedIntervalTreeObject;

    // Protected methods
    void                ReadVar(char *, int, int);
    virtual void        CreateValues(void);
 
    // Constants
    static int    const   N_DIMS_LIMIT;
    static char * const   SILO_LOCATION;
};


#endif


