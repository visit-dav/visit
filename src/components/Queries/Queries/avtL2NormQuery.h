// ************************************************************************* //
//                              avtL2NormQuery.h                             //
// ************************************************************************* //

#ifndef AVT_L2NORM_QUERY_H
#define AVT_L2NORM_QUERY_H

#include <query_exports.h>

#include <avtCurveQuery.h>


// ****************************************************************************
//  Class: avtL2NormQuery
//
//  Purpose:
//    A query that calculates the L2-Norm of a curve.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
// ****************************************************************************

class QUERY_API avtL2NormQuery : public avtCurveQuery
{
  public:
                              avtL2NormQuery();
    virtual                  ~avtL2NormQuery();

    virtual const char       *GetType(void)  { return "avtL2NormQuery"; };
    virtual const char       *GetDescription(void)
                                             { return "Calculating L2Norm."; };

  protected:
    virtual double            CurveQuery(int, const float *, const float *);
    virtual std::string       CreateMessage(double);
};


#endif


