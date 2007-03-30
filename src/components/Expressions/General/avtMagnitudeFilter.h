// ************************************************************************* //
//                            avtMagnitudeFilter.h                           //
// ************************************************************************* //

#ifndef AVT_MAGNITUDE_FILTER_H
#define AVT_MAGNITUDE_FILTER_H


#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtMagnitudeFilter
//
//  Purpose:
//      A filter that takes vector data and creates a scalar variable based on
//      its magnitude. 
//          
//  Programmer: Matthew Haddox
//  Creation:   July 30, 2002
//
// ****************************************************************************

class EXPRESSION_API avtMagnitudeFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtMagnitudeFilter() {;};
    virtual                  ~avtMagnitudeFilter() {;};

    virtual const char       *GetType(void)   { return "avtMagnitudeFilter"; };
    virtual const char       *GetDescription(void)
                              {return "Calculating Magnitude of Each Vector";};
  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
};


#endif


