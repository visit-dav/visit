// ************************************************************************* //
//                          avtVectorDecomposeFilter.h                      //
// ************************************************************************* //

#ifndef AVT_VECTOR_DECOMPOSE_FILTER_H
#define AVT_VECTOR_DECOMPOSE_FILTER_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtVectorDecomposeFilter
//
//  Purpose:
//      Gets a component of a vector variable.
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2002
//
//  Modifications:
//      Sean Ahern, Thu Mar  6 01:59:23 America/Los_Angeles 2003
//      Merged the vector component filters together.
//
// ****************************************************************************

class EXPRESSION_API avtVectorDecomposeFilter 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtVectorDecomposeFilter(int w) {which_comp = w;}
    virtual                  ~avtVectorDecomposeFilter() {;}

    virtual const char       *GetType(void)  
                                    { return "avtVectorDecomposeFilter"; };
    virtual const char       *GetDescription(void)
                                    { return "Pulling out a component of a vector"; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual int               GetVariableDimension() { return 1; }
    int                       which_comp;
};


#endif


