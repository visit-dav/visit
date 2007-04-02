// ************************************************************************* //
//                              avtR2Foperator.h                             //
// ************************************************************************* //

#ifndef AVT_R2F_OPERATOR_H
#define AVT_R2F_OPERATOR_H

#include <ddf_exports.h>

#include <vector>
#include <string>


// ****************************************************************************
//  Class: avtR2Foperator
//
//  Purpose:
//      This is an abstract type for operators that turn derived data relations
//      into derived data functions.  Examples of derived types are averages,
//      standard deviations, etc.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Feb 25 15:24:49 PST 2006
//    Added undefinedVal.
//
// ****************************************************************************

class AVTDDF_API avtR2Foperator
{
  public:
                           avtR2Foperator(int, double);
    virtual               ~avtR2Foperator();

    virtual int            GetNumberOfPasses(void) { return 1; };
    virtual float         *FinalizePass(int) = 0;
    virtual void           AddData(int, float) = 0;

  protected:
    int                    nBins;
    double                 undefinedVal;
};


#endif


