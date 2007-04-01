// ************************************************************************* //
//                         avtCurveComparisonQuery.h                         //
// ************************************************************************* //

#ifndef AVT_CURVE_COMPARISON_QUERY_H
#define AVT_CURVE_COMPARISON_QUERY_H

#include <query_exports.h>

#include <avtMultipleInputQuery.h>

class     avtDatasetSink;


// ****************************************************************************
//  Class: avtCurveComparisonQuery
//
//  Purpose:
//      An abstract query that prepares curves for comparison.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Oct 14 17:19:01 PDT 2004
//    Added method 'AverageYValsForDuplicateX'.
//
// ****************************************************************************

class QUERY_API avtCurveComparisonQuery : public avtMultipleInputQuery
{
  public:
                              avtCurveComparisonQuery();
    virtual                  ~avtCurveComparisonQuery();

  protected:
    virtual void              Execute(void);

    avtDatasetSink           *curve1;
    avtDatasetSink           *curve2;

    virtual double            CompareCurves(int, const float *x1, 
                                            const float *y1, int,
                                            const float *x2, const float *y2) 
                                    = 0;
    virtual std::string       CreateMessage(double) = 0;

    void                      PutOnSameXIntervals(int, const float *, 
                                 const float *, int, const float *,
                                 const float *, std::vector<float> &,
                                 std::vector<float> &, std::vector<float> &);
    void                      AverageYValsForDuplicateX(int n, 
                                                        const float *x, 
                                                        const float *y, 
                                                        vector<float> &X, 
                                                        vector<float> &Y);
 
};


#endif


