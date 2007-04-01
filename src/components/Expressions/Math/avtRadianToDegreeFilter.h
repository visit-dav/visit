// ************************************************************************* //
//                         avtRadianToDegreeFilter.h                         //
// ************************************************************************* //

#ifndef AVT_RADIAN_TO_DEGREE_FILTER_H
#define AVT_RADIAN_TO_DEGREE_FILTER_H

#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtRadianToDegreeFilter
//
//  Purpose:
//      Converts radian angles to degree angles.
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtRadianToDegreeFilter : public avtUnaryMathFilter
{
  public:
                              avtRadianToDegreeFilter();
    virtual                  ~avtRadianToDegreeFilter();

    virtual const char       *GetType(void) 
                                 { return "avtRadianToDegreeFilter"; };
    virtual const char       *GetDescription(void) 
                                 { return "Converting radians to degrees"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


