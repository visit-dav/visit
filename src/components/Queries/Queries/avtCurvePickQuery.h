// ************************************************************************* //
//                           avtCurvePickQuery.h                            //
// ************************************************************************* //

#ifndef AVT_CURVE_PICK_QUERY_H
#define AVT_CURVE_PICK_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>
#include <PickAttributes.h>


class vtkDataSet;

// ****************************************************************************
//  Class: avtCurvePickQuery
//
//  Purpose:
//      This query finds the closest point on a curve to a given point. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 2, 2003 
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class QUERY_API avtCurvePickQuery : public avtDatasetQuery
{
  public:
                                    avtCurvePickQuery();
    virtual                        ~avtCurvePickQuery();

    virtual const char             *GetType(void)
                                             { return "avtCurvePickQuery"; };
    virtual const char             *GetDescription(void)
                                             { return "Picking curve."; };

    void                            SetPickAtts(const PickAttributes *);
    const PickAttributes           *GetPickAtts(void);

  protected:
    PickAttributes                  pickAtts;
    int                             foundDomain;
    float                           minDist;

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    int                             FindClosestPoint(vtkDataSet *);
};


#endif


