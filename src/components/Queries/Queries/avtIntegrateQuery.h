// ************************************************************************* //
//                            avtIntegrateQuery.h                            //
// ************************************************************************* //

#ifndef AVT_INTEGRATE_QUERY_H
#define AVT_INTEGRATE_QUERY_H

#include <query_exports.h>

#include <avtCurveQuery.h>


// ****************************************************************************
//  Class: avtIntegrateQuery
//
//  Purpose:
//    A query that calculates the integral of a curve.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class QUERY_API avtIntegrateQuery : public avtCurveQuery
{
  public:
                              avtIntegrateQuery();
    virtual                  ~avtIntegrateQuery();

    virtual const char       *GetType(void)  { return "avtIntegrateQuery"; };
    virtual const char       *GetDescription(void)
                                           { return "Integrating function."; };

  protected:
    virtual double            CurveQuery(int, const float *, const float *);
    virtual std::string       CreateMessage(double);
};


#endif


