// ************************************************************************* //
//                          avtVectorComponent1Filter.h                      //
// ************************************************************************* //

#ifndef AVT_VECTOR_COMPONENT_1_FILTER_H
#define AVT_VECTOR_COMPONENT_1_FILTER_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtVectorComponent1Filter
//
//  Purpose:
//      Gets the first component of a vector variable.
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVectorComponent1Filter 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtVectorComponent1Filter() {;};
    virtual                  ~avtVectorComponent1Filter() {;};

    virtual const char       *GetType(void)  
                                    { return "avtVectorComponent1Filter"; };
    virtual const char       *GetDescription(void)
                                    { return "Pulling out the first component "
                                             "of a vector"; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual int               GetVariableDimension() { return 1; }
};


#endif


