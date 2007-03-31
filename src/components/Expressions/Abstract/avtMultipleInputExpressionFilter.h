// ********************************************************************** //
//                             avtMultipleInputExpressionFilter.h           //
// ********************************************************************** //

#ifndef AVT_MULTIPLE_INPUT_EXPRESSION_FILTER_H
#define AVT_MULTIPLE_INPUT_EXPRESSION_FILTER_H

#include <expression_exports.h>
#include <avtExpressionFilter.h>

class     vtkDataArray;

// ****************************************************************************
//  Class: avtMultipleInputExpressionFilter
//
//  Purpose:
//      A filter that performs a calculation on multiple variables.
//
//  Programmer: Sean Ahern
//  Creation:   Fri Jun 14 17:57:37 PDT 2002
//
// ****************************************************************************

class EXPRESSION_API avtMultipleInputExpressionFilter : public avtExpressionFilter
{
  public:
                              avtMultipleInputExpressionFilter() {}
    virtual                  ~avtMultipleInputExpressionFilter() {}

    virtual void              AddInputVariableName(const char *var);

    virtual const char       *GetType(void) { return "avtMultipleInputExpressionFilter"; }
    virtual const char       *GetDescription(void) = 0;

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *) = 0;
    std::vector<char *>       varnames;
};

#endif
