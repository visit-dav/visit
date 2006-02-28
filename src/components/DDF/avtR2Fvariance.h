// ************************************************************************* //
//                               avtR2Fvariance.h                            //
// ************************************************************************* //

#ifndef AVT_R2F_VARIANCE_H
#define AVT_R2F_VARIANCE_H

#include <ddf_exports.h>

#include <avtR2Foperator.h>

#include <vector>
#include <string>


// ****************************************************************************
//  Class: avtR2Fvariance
//
//  Purpose:
//      Turns a derived data relation into a derived data function using the
//      statistical operator "variance".
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2006
//
// ****************************************************************************

class AVTDDF_API avtR2Fvariance : public avtR2Foperator
{
  public:
                           avtR2Fvariance(int, double);
    virtual               ~avtR2Fvariance();

    virtual float         *FinalizePass(int);
    virtual void           AddData(int, float);
    virtual int            GetNumberOfPasses(void) { return 2; };

  protected:
    double                *running_total_ave;
    double                *running_total_variance;
    int                   *count;
    int                    pass;
};


#endif


