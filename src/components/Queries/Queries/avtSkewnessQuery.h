// ************************************************************************* //
//                            avtSkewnessQuery.h                             //
// ************************************************************************* //

#ifndef AVT_SKEWNESS_QUERY_H
#define AVT_SKEWNESS_QUERY_H

#include <query_exports.h>

#include <avtCurveQuery.h>


// ****************************************************************************
//  Class: avtSkewnessQuery
//
//  Purpose:
//    A query that calculates the skewness of a distribution.
//
//    Skewness is a measure of "symmetry".  A distribution has "symmetry"
//    if it similar to the left and to the right of the center point.
//
//    More information about skewness can be found at:
//    http://mathworld.wolfram.com/Skewness.html
//
//  Programmer: Hank Childs
//  Creation:   August 5, 2005
//
// ****************************************************************************

class QUERY_API avtSkewnessQuery : public avtCurveQuery
{
  public:
                              avtSkewnessQuery();
    virtual                  ~avtSkewnessQuery();

    virtual const char       *GetType(void)  { return "avtSkewnessQuery"; };
    virtual const char       *GetDescription(void)
                                           { return "Calculating skewness."; };

  protected:
    virtual double            CurveQuery(int, const float *, const float *);
    virtual std::string       CreateMessage(double);
};


#endif


