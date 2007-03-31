// ************************************************************************* //
//                              avtIdentityFilter.h                          //
// ************************************************************************* //

#ifndef AVT_IDENTITY_FILTER_H
#define AVT_IDENTITY_FILTER_H

#include <avtUnaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtIdentityFilter
//
//  Purpose:
//      A filter that takes the identity of a variable.
//
//  Programmer: Hank Childs
//  Creation:   December 9, 2003
//
// ****************************************************************************

class EXPRESSION_API avtIdentityFilter : public avtUnaryMathFilter
{
  public:
                              avtIdentityFilter() {;};
    virtual                  ~avtIdentityFilter() {;};

    virtual const char       *GetType(void)  { return "avtIdentityFilter"; };
    virtual const char       *GetDescription(void) 
                                    { return "Applying the identity filter"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


