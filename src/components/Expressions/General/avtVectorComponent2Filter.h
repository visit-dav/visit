// ************************************************************************* //
//                          avtVectorComponent2Filter.h                      //
// ************************************************************************* //

#ifndef AVT_VECTOR_COMPONENT_2_FILTER_H
#define AVT_VECTOR_COMPONENT_2_FILTER_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtVectorComponent2Filter
//
//  Purpose:
//      Gets the second component of a vector variable.
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVectorComponent2Filter 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtVectorComponent2Filter() {;};
    virtual                  ~avtVectorComponent2Filter() {;};

    virtual const char       *GetType(void)  
                                    { return "avtVectorComponent2Filter"; };
    virtual const char       *GetDescription(void)
                                    { return "Pulling out the second "
                                             "component of a vector"; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual int               GetVariableDimension() { return 1; }
};


#endif


