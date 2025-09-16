// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtOOFUSExpression.h                       //
// ************************************************************************* //

#ifndef AVT_OOFUS_EXPRESSION_H
#define AVT_OOFUS_EXPRESSION_H

#include <avtExpressionFilter.h>
#include <vtkBitArray.h>

#include <vector>

class     vtkDataArray;
class     vtkIntArray;
class     vtkDataSet;
class     vtkCell;
class     vtkUnstructuredGrid;
class     avtIntervalTree;

// ****************************************************************************
//  Class: avtOOFUSExpression
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   August 18, 2025
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtOOFUSExpression : public avtExpressionFilter
{
  public:
                              avtOOFUSExpression();
    virtual                  ~avtOOFUSExpression();

    virtual const char       *GetType(void)
                                     { return "avtOOFUSExpression"; };
    virtual const char       *GetDescription(void)
                                      {return "Finding OOFUS";};
    virtual int               NumVariableArguments() { return 1; };
    virtual int               GetVariableDimension() { return 1; };
    virtual int               GetNumberOfComponents();

  protected:
    int                       nFinalComps;
    int                       currentProgress;
    int                       totalSteps;
    int                       totalNodes;

    int                       enableGhostNeighbors;

    virtual void              Execute(void);

    virtual avtContract_p
                              ModifyContract(avtContract_p);
    
    virtual bool              CheckForProperGhostZones(vtkDataSet **sets,int nsets);
    virtual void              LabelGhostNeighbors(vtkDataSet *);
    virtual void              LabelBoundaryNeighbors(vtkDataSet *);

    avtCentering              centering;
    vtkDataSet               *cur_mesh;
    // Track if the created array is volume-dependent.
    vtkBitArray              *volumeDependent;

  private:
    void
    CalculateWithoutGhosts(vtkDataArray *in, 
                                               vtkDataArray *out,
                                               int ncomponents,
                                               int ntuples);

    void
    CalculateWithGhosts(vtkDataArray *in,
                                        vtkDataArray *out,
                                        int ncomponents,
                                        int ntuples,
                                        int (getNodeOrCellValid)(vtkDataArray *, int *, int),
                                        vtkDataArray *ghostZones,
                                        int *nodeShouldBeIgnoredPtr);
    std::vector<int>
    IdentifyGhostedNodes(vtkDataSet *in_ds,
                                                           vtkDataArray *ghostZones,
                                                           vtkDataArray *ghostNodes);
    void
    DoOperation(vtkDataArray *in, vtkDataArray *out,
                              int ncomponents, int ntuples, vtkDataSet *in_ds);

    vtkDataArray *
    CreateArray(vtkDataArray *in1);

    vtkDataArray *
    DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex);

    avtDataRepresentation *
    ExecuteData_VTK(avtDataRepresentation *in_dr);

    avtDataRepresentation *
    ExecuteData(avtDataRepresentation *in_dr);

    avtDataTree_p
    ExecuteDataTree(avtDataRepresentation *in_dr);

    void
    ExecuteDataTreeOnThread(avtDataTree_p inDT, avtDataTree_p &outDT);

    void
    execute_2_electric_boogaloo(avtDataTree_p inDT, avtDataTree_p &outDT);
};


#endif

