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
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Mon Jan 23 09:45:07 PST 2006
//    Declare the correct variable dimension.
//
// ****************************************************************************

class EXPRESSION_API avtMagnitudeFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtMagnitudeFilter();
    virtual                  ~avtMagnitudeFilter();

    virtual const char       *GetType(void)   { return "avtMagnitudeFilter"; };
    virtual const char       *GetDescription(void)
                              {return "Calculating Magnitude of Each Vector";};
  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual int               GetVariableDimension(void) { return 1; };
};


#endif


