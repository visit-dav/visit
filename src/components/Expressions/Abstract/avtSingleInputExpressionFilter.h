// ********************************************************************** //
//                             avtSingleInputExpressionFilter.h           //
// ********************************************************************** //

#ifndef AVT_SINGLE_INPUT_EXPRESSION_FILTER_H
#define AVT_SINGLE_INPUT_EXPRESSION_FILTER_H


#include <avtExpressionFilter.h>

class     vtkDataArray;

// ****************************************************************************
//  Class: avtSingleInputExpressionFilter
//
//  Purpose:
//      A filter that performs a calculation on a single variable.
//
//  Programmer: Sean Ahern
//  Creation:   Wed Jun 12 16:43:09 PDT 2002
//
// ****************************************************************************

class EXPRESSION_API avtSingleInputExpressionFilter : public avtExpressionFilter
{
  public:
                              avtSingleInputExpressionFilter() {}
    virtual                  ~avtSingleInputExpressionFilter() {}

    void                      AddInputVariableName(const char *var)
        {SetActiveVariable(var);}

    virtual const char       *GetType(void) { return "avtSingleInputExpressionFilter"; }
    virtual const char       *GetDescription(void) = 0;

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *) = 0;
};

#endif
