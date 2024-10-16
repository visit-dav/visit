// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtMaxReduceExpression.h                       //
// ************************************************************************* //

#ifndef AVT_MAX_REDUCE_EXPRESSION_H
#define AVT_MAX_REDUCE_EXPRESSION_H

#include <avtExpressionFilter.h>

#include <vector>

class     vtkDataArray;
class     vtkIntArray;
class     vtkDataSet;
class     vtkCell;
class     vtkUnstructuredGrid;
class     avtIntervalTree;

// ****************************************************************************
//  Class: avtMaxReduceExpression
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   TODO
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtMaxReduceExpression : public avtExpressionFilter
{
  public:
                              avtMaxReduceExpression();
    virtual                  ~avtMaxReduceExpression();

    virtual const char       *GetType(void)
                                     { return "avtMaxReduceExpression"; };
    virtual const char       *GetDescription(void)
                                      {return "Finding connected components";};
    virtual int               NumVariableArguments() { return 1; };
    virtual int               GetVariableDimension() { return 1; };
    virtual bool              IsPointVariable(void) { return false; }
    virtual int               GetFinalMax();
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);


  protected:
    double                    nFinalMax;
    int                       currentProgress;
    int                       totalSteps;

    int                       enableGhostNeighbors;

    virtual void              Execute(void);

    virtual avtContract_p
                              ModifyContract(avtContract_p);
    
    virtual bool              CheckForProperGhostZones(vtkDataSet **sets,int nsets);
    virtual void              LabelGhostNeighbors(vtkDataSet *);
    virtual void              LabelBoundaryNeighbors(vtkDataSet *);
    
    virtual vtkIntArray      *SingleSetLabel(vtkDataSet *, int &);

    virtual int               MultiSetResolve(int,
                                              const BoundarySet &,
                                              const std::vector<vtkDataSet*> &,
                                              const std::vector<vtkIntArray*> &);

    virtual void              MultiSetList(int,
                                           const BoundarySet &,
                                           const std::vector<vtkDataSet*> &,
                                           const std::vector<vtkIntArray*> &,
                                           std::vector<int> &,
                                           std::vector<int> &);

    virtual int               GlobalLabelShift(int,
                                               const std::vector<vtkIntArray*> &);

    virtual int               GlobalResolve(int,
                                            BoundarySet &,
                                            const std::vector<vtkDataSet*> &,
                                            const std::vector<vtkIntArray*> &);

    virtual int               GlobalUnion(int,
                                          const std::vector<int>&,
                                          const std::vector<int>&,
                                          const std::vector<vtkIntArray*> &);

    virtual void              ShiftLabels(vtkIntArray *, int);
};


#endif

