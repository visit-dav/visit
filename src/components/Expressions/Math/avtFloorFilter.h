// ************************************************************************* //
//                              avtFloorFilter.h                             //
// ************************************************************************* //

#ifndef AVT_FLOOR_FILTER_H
#define AVT_FLOOR_FILTER_H


#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtFloorFilter
//
//  Purpose:
//      A filter that calculates the floor of a floating point quantity.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

class EXPRESSION_API avtFloorFilter : public avtUnaryMathFilter
{
  public:
                              avtFloorFilter();
    virtual                  ~avtFloorFilter();

    virtual const char       *GetType(void)  { return "avtFloorFilter"; };
    virtual const char       *GetDescription(void) 
                                         { return "Calculating the floor"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


