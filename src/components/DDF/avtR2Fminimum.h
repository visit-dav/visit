// ************************************************************************* //
//                               avtR2Fminimum.h                             //
// ************************************************************************* //

#ifndef AVT_R2F_MINIMUM_H
#define AVT_R2F_MINIMUM_H

#include <ddf_exports.h>

#include <avtR2Foperator.h>

#include <vector>
#include <string>


// ****************************************************************************
//  Class: avtR2Fminimum
//
//  Purpose:
//      Turns a derived data relation into a derived data function by
//      consistently taking the minimum.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Feb 25 15:24:49 PST 2006
//    Add undefined value in constructor.
//
// ****************************************************************************

class AVTDDF_API avtR2Fminimum : public avtR2Foperator
{
  public:
                           avtR2Fminimum(int, double);
    virtual               ~avtR2Fminimum();

    virtual float         *FinalizePass(int);
    virtual void           AddData(int, float);

  protected:
    float                 *min;
};


#endif


