// ************************************************************************* //
//                        avtAreaBetweenCurvesQuery.h                        //
// ************************************************************************* //

#ifndef AVT_AREA_BETWEEN_CURVES_QUERY_H
#define AVT_AREA_BETWEEN_CURVES_QUERY_H

#include <query_exports.h>

#include <avtCurveComparisonQuery.h>

class     avtDatasetSink;


// ****************************************************************************
//  Class: avtAreaBetweenCurvesQuery
//
//  Purpose:
//    A query that calculates the area between two curves.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
// ****************************************************************************

class QUERY_API avtAreaBetweenCurvesQuery : public avtCurveComparisonQuery
{
  public:
                              avtAreaBetweenCurvesQuery();
    virtual                  ~avtAreaBetweenCurvesQuery();


    virtual const char       *GetType(void)
                                       { return "avtAreaBetweenCurvesQuery"; };
    virtual const char       *GetDescription(void)
                                { return "Calculating area between curves."; };

  protected:
    virtual double            CompareCurves(int n1, const float *x1, 
                                            const float *y1, int n2,
                                            const float *x2, const float *y2);
    virtual std::string       CreateMessage(double);
};


#endif


