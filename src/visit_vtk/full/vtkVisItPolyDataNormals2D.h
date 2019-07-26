// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef __vtkVisItPolyDataNormals2D_h
#define __vtkVisItPolyDataNormals2D_h

#include <visit_vtk_exports.h>
#include <vtkPolyDataAlgorithm.h>

class vtkPolyData;
class vtkInformation;
class vtkInformationVector;

// ****************************************************************************
//  Class:  vtkVisItPolyDataNormals2D
//
//  Purpose:
//    Calculate cell or point centered normals.
//
//  Note: Derived from avtSurfaceNormalExpression version of function
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 10, 2014
//
// ****************************************************************************
class VISIT_VTK_API vtkVisItPolyDataNormals2D : public vtkPolyDataAlgorithm
{
  public:
    vtkTypeMacro(vtkVisItPolyDataNormals2D,vtkPolyDataAlgorithm);

    static vtkVisItPolyDataNormals2D *New();

    void SetNormalTypeToCell()      { ComputePointNormals = false; }
    void SetNormalTypeToPoint()     { ComputePointNormals = true;  }

  protected:
    vtkVisItPolyDataNormals2D();
    ~vtkVisItPolyDataNormals2D() {};

    // Usual data generation method
    int RequestData(vtkInformation*, vtkInformationVector**,
                     vtkInformationVector*) override;
    void ExecutePoint(vtkPolyData*, vtkPolyData*);
    void ExecuteCell(vtkPolyData *, vtkPolyData*);

    bool  ComputePointNormals;

  private:
    vtkVisItPolyDataNormals2D(const vtkVisItPolyDataNormals2D&);  // Not implemented.
    void operator=(const vtkVisItPolyDataNormals2D&);  // Not implemented.
};

#endif
