// ************************************************************************* //
//                              avtResradFilter.h                            //
// ************************************************************************* //

#ifndef AVT_RESRAD_FILTER_H
#define AVT_RESRAD_FILTER_H

#include <avtBinaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtResradFilter
//
//  Purpose:
//      A filter that calculates resrad -- which adjusts the resolution
//      using a monte carlo resampling with a given radius.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

class EXPRESSION_API avtResradFilter : public avtBinaryMathFilter
{
  public:
                              avtResradFilter();
    virtual                  ~avtResradFilter();

    virtual const char       *GetType(void) 
                                 { return "avtResradFilter"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating resrad"; };

  protected:
    bool             haveIssuedWarning;

    virtual void     PreExecute(void);
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


