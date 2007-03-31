// ************************************************************************* //
//                             avtMatvfFilter.h                             //
// ************************************************************************* //

#ifndef AVT_MATVF_FILTER_H
#define AVT_MATVF_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ConstExpr;

// ****************************************************************************
//  Class: avtMatvfFilter
//
//  Purpose:
//      Creates the material fraction at each point.
//          
//  Programmer: Sean Ahern
//  Creation:   March 18, 2003
//
// ****************************************************************************

class EXPRESSION_API avtMatvfFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtMatvfFilter() {;};
    virtual                  ~avtMatvfFilter() {;};

    virtual const char       *GetType(void) { return "avtMatvfFilter"; };
    virtual const char       *GetDescription(void)
                                           {return "Assigning random #.";};
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);
  protected:

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void)  { return false; };

    void                      AddMaterial(ConstExpr *);
    std::vector<std::string>  materials;
};


#endif


