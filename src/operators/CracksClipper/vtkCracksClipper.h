// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTK_CRACKS_CLIPPER_H
#define VTK_CRACKS_CLIPPER_H

#include <vtkImplicitFunction.h>
#include <vtkVisItClipper.h>

class vtkImplicitBoolean;

class AlwaysNegative : public vtkImplicitFunction
{
public:
  vtkTypeMacro(AlwaysNegative, vtkImplicitFunction);
  static AlwaysNegative *New();

  double EvaluateFunction(double x[3]) override
      { return -1.; }
  void EvaluateGradient(double x[3], double g[3]) override
      { g[0] = g[1] = g[2] = 1.; } 

  void SetReturnNeg(bool val) { returnNeg = val; }

protected:
  AlwaysNegative();
  ~AlwaysNegative();

private:
  bool returnNeg;
  AlwaysNegative(const AlwaysNegative&);  // Not implemented.
  void operator=(const AlwaysNegative&);  // Not implemented.
};


// ****************************************************************************
//  Class:  vtkCracksClipper
//
//  Purpose:
//    Clips a dataset using an implicit function, defined on a per-cell
//    basis based upon crack-direction vectors and strain variables.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    August 22, 2005 
//
//  Modifications:
//    Kathleen Bonnell, Thu Sep 23 13:28:57 MST 2010
//    Changed inheritance to vtkDataSetToUnstructuredGridFilter and added all
//    functionality from 1.12 version of vtkVisItClipper class.  Changes made
//    to vtkVisItClipper for 2.0 don't meet needs of CracksClipper, which 
//    applies a different Clip Function to every cell. 
//
//    Kathleen Biagas, Fri Aug 10 12:16:36 PDT 2012
//    Make this class inherit from vtkVisItClipper again.
//
// ****************************************************************************

class vtkCracksClipper : public vtkVisItClipper
{
  public:
    vtkTypeMacro(vtkCracksClipper,vtkVisItClipper);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    static vtkCracksClipper *New();
    void SetUseOppositePlane(bool val) { useOppositePlane = val;};

    vtkSetStringMacro(CrackDir);
    vtkSetStringMacro(CrackWidth);
    vtkSetStringMacro(CellCenters);

    void ModifyClip(vtkDataSet *, vtkIdType) override;

  protected:
    vtkCracksClipper();
    ~vtkCracksClipper();

    vtkCracksClipper(const vtkCracksClipper&);  // Not implemented.
    void operator=(const vtkCracksClipper&);  // Not implemented.

    vtkImplicitBoolean *cf;
    AlwaysNegative *emptyFunc;
    bool useOppositePlane;

    char *CrackDir;
    char *CrackWidth;
    char *CellCenters;
};



#endif


