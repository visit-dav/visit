// ************************************************************************* //
//                       avtL2NormBetweenCurvesQuery.h                       //
// ************************************************************************* //

#ifndef AVT_L2NORM_BETWEEN_CURVES_QUERY_H
#define AVT_L2NORM_BETWEEN_CURVES_QUERY_H

#include <query_exports.h>

#include <avtCurveComparisonQuery.h>

class     avtDatasetSink;


// ****************************************************************************
//  Class: avtL2NormBetweenCurvesQuery
//
//  Purpose:
//    A query that calculates the L2-Norm between two curves.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
// ****************************************************************************

class QUERY_API avtL2NormBetweenCurvesQuery : public avtCurveComparisonQuery
{
  public:
                              avtL2NormBetweenCurvesQuery();
    virtual                  ~avtL2NormBetweenCurvesQuery();

    virtual const char       *GetType(void)  
                                     { return "avtL2NormBetweenCurvesQuery"; };
    virtual const char       *GetDescription(void)
                                     { return "Calculating L2Norm."; };

  protected:
    virtual double            CompareCurves(int n1, const float *x1, 
                                            const float *y1, int n2,
                                            const float *x2, const float *y2);
    virtual std::string       CreateMessage(double);
};


#endif


