// ************************************************************************* //
//                         avtDegreeToRadianFilter.h                         //
// ************************************************************************* //

#ifndef AVT_DEGREE_TO_RADIAN_FILTER_H
#define AVT_DEGREE_TO_RADIAN_FILTER_H

#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtDegreeToRadianFilter
//
//  Purpose:
//      Converts radian angles to degree angles.
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2002
//
// ****************************************************************************

class EXPRESSION_API avtDegreeToRadianFilter : public avtUnaryMathFilter
{
  public:
                              avtDegreeToRadianFilter() {;};
    virtual                  ~avtDegreeToRadianFilter() {;};

    virtual const char       *GetType(void) 
                                 { return "avtDegreeToRadianFilter"; };
    virtual const char       *GetDescription(void) 
                                 { return "Converting degrees to radians "; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


