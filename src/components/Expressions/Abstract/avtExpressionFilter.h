// ************************************************************************* //
//                            avtExpressionFilter.h                          //
// ************************************************************************* //

#ifndef AVT_EXPRESSION_FILTER_H
#define AVT_EXPRESSION_FILTER_H

#include <expression_exports.h>
#include <avtStreamer.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtExpressionFilter
//
//  Purpose:
//      This is a base class that lets derived types not worry about how to set
//      up a derived variable.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2002
//
// ****************************************************************************

class EXPRESSION_API avtExpressionFilter : public avtStreamer
{
  public:
                             avtExpressionFilter();
    virtual                 ~avtExpressionFilter();

    void                     SetOutputVariableName(const char *);

  protected:
    char                    *outputVariableName;

    virtual bool             IsPointVariable();

    virtual void             PreExecute(void);
    virtual void             PostExecute(void);
    virtual vtkDataSet      *ExecuteData(vtkDataSet *, int, std::string);
    virtual void             RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p
                             PerformRestriction(avtPipelineSpecification_p);

    virtual int              GetVariableDimension() { return 1; };
    virtual vtkDataArray    *DeriveVariable(vtkDataSet *) = 0;
};


#endif


