// ************************************************************************* //
//                          IntervalTree_Prep.h                              //
// ************************************************************************* //

#ifndef INTERVAL_TREE_PREP_H
#define INTERVAL_TREE_PREP_H

#include <iostream.h>
#include <silo.h>

#include <IntervalTree.h>


// ****************************************************************************
//  Class: IntervalTree
// 
//  Purpose:
//      A derived type of IntervalTree, this is meant exclusively for
//      meshtvprep.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2000
//
// ****************************************************************************

class IntervalTree_Prep : public IntervalTree
{
  public:
                   IntervalTree_Prep();
    virtual       ~IntervalTree_Prep();

    void           AddVar(int, float *);

    void           SetFieldName(char *);
    void           SetNDomains(int);
    void           SetNDims(int);

    int            GetNDims(void)  { return nDims; };
    void           GetRootExtents(float *);

    void           ReadVar(DBmeshvar *, int);
    void           ReadVar(DBpointmesh *, int);
    void           ReadVar(DBquadmesh *, int);
    void           ReadVar(DBquadvar *, int);
    void           ReadVar(DBucdmesh *, int);
    void           ReadVar(DBucdvar *, int);

    void           WrapUp(void);
    virtual void   Write(DBfile *);
      
  protected:
    bool           setNDims;
    bool           setNDomains;
    int            vectorSize;

    // Protected Methods
    void           CollectInformation(void);
    void           ConstructNodeExtents(void);
    void           ConstructTree(void);
    bool           Less(float *, float *, int);
    void           ReadVar(float **, int, int *, int);
    void           SetIntervals(void);
    void           Sort(float *, int *, int, int);
    int            SplitSize(int);

    // Constants
    static char * const    SILO_LOCATION;
};


#endif


