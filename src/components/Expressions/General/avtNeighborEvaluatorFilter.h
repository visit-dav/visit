// ************************************************************************* //
//                        avtNeighborEvaluatorFilter.h                       //
// ************************************************************************* //

#ifndef AVT_NEIGHBOR_EVALUATOR_FILTER_H
#define AVT_NEIGHBOR_EVALUATOR_FILTER_H


#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtNeighborEvaluatorFilter
//
//  Purpose:
//      A filter that can evaluate neighbor's values.  Options include biggest,
//      smallest, and average.
//
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
//  Modifications:
// 
//    Hank Childs, Thu Jan 20 11:10:08 PST 2005
//    Add better warnings when error conditions are encountered.
//
// ****************************************************************************

class EXPRESSION_API avtNeighborEvaluatorFilter 
    : public avtSingleInputExpressionFilter
{
  public:

    typedef enum
    {
        BIGGEST_NEIGHBOR,
        SMALLEST_NEIGHBOR,
        AVERAGE_NEIGHBOR
    } EvaluationType;

                              avtNeighborEvaluatorFilter();
    virtual                  ~avtNeighborEvaluatorFilter();
    void                      SetEvaluationType(EvaluationType t)
                                         { evaluationType = t; };

    virtual const char       *GetType(void)   
                                    { return "avtNeighborEvaluatorFilter"; };
    virtual const char       *GetDescription(void)
                                    { return "Calculating neighbors values"; };
    virtual void              PreExecute(void);

  protected:
    EvaluationType            evaluationType;
    float                    *buff;
    int                      *nEntries;
    bool                      haveIssuedWarning;

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual avtPipelineSpecification_p
                              PerformRestriction(avtPipelineSpecification_p);

    void                      InitializeEvaluation(int, float *);
    void                      EvaluateNeighbor(int, float);
    void                      FinalizeEvaluation(int);
};


#endif


