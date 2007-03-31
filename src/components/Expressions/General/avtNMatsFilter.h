// ************************************************************************* //
//                              avtNMatsFilter.h                             //
// ************************************************************************* //

#ifndef AVT_NMATS_FILTER_H
#define AVT_NMATS_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ConstExpr;

// ****************************************************************************
//  Class: avtNMatsFilter
//
//  Purpose:
//      Determines the number of materials in a zone.
//          
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
// ****************************************************************************

class EXPRESSION_API avtNMatsFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtNMatsFilter() {;};
    virtual                  ~avtNMatsFilter() {;};

    virtual const char       *GetType(void) { return "avtNMatsFilter"; };
    virtual const char       *GetDescription(void)
                                           {return "Finding NMats";};
  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void)  { return false; };
};


#endif


