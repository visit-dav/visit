// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_FACTORIAL_H
#define AVT_FACTORIAL_H
#include <math_exports.h>
#include <vectortypes.h>


// ****************************************************************************
//  Class:  avtFactorial
//
//  Purpose:
//    An interface for computing factorials. Provides a memoization option 
//    to speed up multiple calls (enabled by default). 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
//  Modifications:
//
// ****************************************************************************

class MATH_API avtFactorial
{
  public:
    ~avtFactorial();
    static double Eval(int n);
    static void   SetMemoize(bool v);

  private:
    avtFactorial();
    static bool          memoize;
    static doubleVector  factorialTable;
};



#endif
