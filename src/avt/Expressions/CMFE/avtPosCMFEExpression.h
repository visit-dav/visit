// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtPosCMFEExpression.h                         //
// ************************************************************************* //

#ifndef AVT_POS_CMFE_EXPRESSION_H
#define AVT_POS_CMFE_EXPRESSION_H

#include <avtCMFEExpression.h>

#include <avtIntervalTree.h>

class     vtkCell;
class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ConstExpr;


// ****************************************************************************
//  Class: avtPosCMFEExpression
//
//  Purpose:
//      Does a position based cross-mesh field evaluation.
//          
//  Programmer: Hank Childs
//  Creation:   October 10, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 18 11:04:51 EST 2007
//    Report explicitly that this filter does NOT understand transformed
//    rectilinear grids.  This method should default to returning false
//    anyway, but there are specific reasons this filter cannot yet be
//    optimized in this fashion, so ensure that even if other CMFE's change
//    to default to true, this one remains false until it can be fixed.
//
//    Hank Childs, Tue Mar 13 08:26:09 PDT 2012
//    Define virtual method OnlyRequiresSpatiallyOverlappingData.
//
// ****************************************************************************

class EXPRESSION_API avtPosCMFEExpression : public avtCMFEExpression
{
  public:
                              avtPosCMFEExpression();
    virtual                  ~avtPosCMFEExpression();

    virtual const char       *GetType(void){ return "avtPosCMFEExpression"; };
    virtual const char       *GetDescription(void)
                                           {return "Evaluating field";};
  protected:
    virtual avtDataTree_p     PerformCMFE(avtDataTree_p, avtDataTree_p,
                                          const std::string &,
                                          const std::string &);
    virtual bool              UseIdenticalSIL(void) { return false; };
    virtual bool              HasDefaultVariable(void) { return true; };
    virtual bool              FilterUnderstandsTransformedRectMesh();
    virtual bool              OnlyRequiresSpatiallyOverlappingData()
                                          { return true; };
};


#endif


