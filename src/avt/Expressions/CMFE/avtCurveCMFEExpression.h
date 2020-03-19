// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtCurveCMFEExpression.h                         //
// ************************************************************************* //

#ifndef AVT_CURVE_CMFE_EXPRESSION_H
#define AVT_CURVE_CMFE_EXPRESSION_H

#include <avtCMFEExpression.h>

class     vtkDataArray;
class     vtkRectilinearGrid;

// ****************************************************************************
//  Class: avtCurveCMFEExpression
//
//  Purpose:
//      Does a connectivity based cross-mesh field evaluation.
//          
//  Programmer: Kathleen Bonnell 
//  Creation:   November 18, 2008 
//
//  Modifications:
//    Kathleen Bonnell, Tue Feb 24 11:47:56 PST 2009
//    Added methods HasDupX, RemoveDuplicateX, and MergeCurvesToSameXInterval.
//
// ****************************************************************************

class EXPRESSION_API avtCurveCMFEExpression : public avtCMFEExpression
{
  public:
                              avtCurveCMFEExpression();
    virtual                  ~avtCurveCMFEExpression();

    virtual const char       *GetType(void){ return "avtCurveCMFEExpression"; };
    virtual const char       *GetDescription(void)
                                           {return "Evaluating field";};
  protected:
    virtual avtDataTree_p     PerformCMFE(avtDataTree_p, avtDataTree_p,
                                          const std::string &,
                                          const std::string &);
    avtDataTree_p             ExecuteTree(avtDataTree_p, avtDataTree_p,
                                          const std::string &,
                                          const std::string &);
    virtual bool              UseIdenticalSIL(void) { return true; };

  private:

    bool                      HasDupX(vtkRectilinearGrid *);
    vtkRectilinearGrid       *RemoveDuplicateX(vtkRectilinearGrid *);
    vtkRectilinearGrid       *MergeCurvesToSameXIntervals(vtkRectilinearGrid *,
                                                   vtkRectilinearGrid *,
                                                   const std::string &);
};


#endif
