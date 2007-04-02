// ************************************************************************* //
//                         avtExternalNodeExpression.h                       //
// ************************************************************************* //

#ifndef AVT_EXTERNAL_NODE_EXPRESSION_H
#define AVT_EXTERNAL_NODE_EXPRESSION_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtExternalNodeExpression
//
//  Purpose:
//      An expression that determines which nodes are external.
//          
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
// ****************************************************************************

class EXPRESSION_API avtExternalNodeExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtExternalNodeExpression();
    virtual                  ~avtExternalNodeExpression();

    virtual const char       *GetType(void) 
                                      { return "avtExternalNodeExpression"; };
    virtual const char       *GetDescription(void)
                                      {return "Determining external nodes"; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void)  { return true; };
};


#endif


