// ************************************************************************* //
//                               avtR2Fmaximum.h                             //
// ************************************************************************* //

#ifndef AVT_R2F_MAXIMUM_H
#define AVT_R2F_MAXIMUM_H

#include <ddf_exports.h>

#include <avtR2Foperator.h>

#include <vector>
#include <string>


// ****************************************************************************
//  Class: avtR2Fmaximum
//
//  Purpose:
//      Turns a derived data relation into a derived data function by
//      consistently taking the maximum.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

class AVTDDF_API avtR2Fmaximum : public avtR2Foperator
{
  public:
                           avtR2Fmaximum(int);
    virtual               ~avtR2Fmaximum();

    virtual float         *FinalizePass(int);
    virtual void           AddData(int, float);

  protected:
    float                 *max;
};


#endif


