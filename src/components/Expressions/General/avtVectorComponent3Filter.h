// ************************************************************************* //
//                          avtVectorComponent3Filter.h                      //
// ************************************************************************* //

#ifndef AVT_VECTOR_COMPONENT_3_FILTER_H
#define AVT_VECTOR_COMPONENT_3_FILTER_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtVectorComponent3Filter
//
//  Purpose:
//      Gets the third component of a vector variable.
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVectorComponent3Filter 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtVectorComponent3Filter() {;};
    virtual                  ~avtVectorComponent3Filter() {;};

    virtual const char       *GetType(void)  
                                    { return "avtVectorComponent3Filter"; };
    virtual const char       *GetDescription(void)
                                    { return "Pulling out the third component "
                                             "of a vector"; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual int               GetVariableDimension() { return 1; }
};


#endif


