// ************************************************************************* //
//                            avtKurtosisQuery.h                             //
// ************************************************************************* //

#ifndef AVT_KURTOSIS_QUERY_H
#define AVT_KURTOSIS_QUERY_H

#include <query_exports.h>

#include <avtCurveQuery.h>


// ****************************************************************************
//  Class: avtKurtosisQuery
//
//  Purpose:
//    A query that calculates the kurtosis of a distribution.
//
//    Kurtosis measures whether a distribution is peaked or flat when 
//    compared to a normal distribution.
//
//    More information about kurtosis can be found at:
//    http://mathworld.wolfram.com/Kurtosis.html
//
//  Programmer: Hank Childs
//  Creation:   August 5, 2005
//
// ****************************************************************************

class QUERY_API avtKurtosisQuery : public avtCurveQuery
{
  public:
                              avtKurtosisQuery();
    virtual                  ~avtKurtosisQuery();

    virtual const char       *GetType(void)  { return "avtKurtosisQuery"; };
    virtual const char       *GetDescription(void)
                                           { return "Calculating kurtosis."; };

  protected:
    virtual double            CurveQuery(int, const float *, const float *);
    virtual std::string       CreateMessage(double);
};


#endif


