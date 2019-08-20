// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtExpectedValueQuery.h                          //
// ************************************************************************* //

#ifndef AVT_EXPECTED_VALUE_QUERY_H
#define AVT_EXPECTED_VALUE_QUERY_H

#include <query_exports.h>

#include <avtCurveQuery.h>

#include <string>

// ****************************************************************************
//  Class: avtExpectedValueQuery
//
//  Purpose:
//    A query that calculates the expected value of a probability density
//    function.  That is, for a function f(x), it calculate the integral
//    x*f(x) over the entire domain, which return the expected value.
//
//  Programmer: Hank Childs
//  Creation:   August 25, 2006
//
// ****************************************************************************

class QUERY_API avtExpectedValueQuery : public avtCurveQuery
{
  public:
                              avtExpectedValueQuery();
    virtual                  ~avtExpectedValueQuery();

    virtual const char       *GetType(void)  { return "avtExpectedValueQuery"; };
    virtual const char       *GetDescription(void)
                                     { return "Calculating expected value."; };

  protected:
    virtual double            CurveQuery(int, const float *, const float *);
    virtual std::string       CreateMessage(double);
};


#endif


