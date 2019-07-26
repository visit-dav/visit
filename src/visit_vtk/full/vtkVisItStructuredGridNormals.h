// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef __vtkVisItStructuredGridNormals_h
#define __vtkVisItStructuredGridNormals_h

#include <visit_vtk_exports.h>
#include <vtkStructuredGridAlgorithm.h>

class vtkStructuredGrid;
class vtkInformation;
class vtkInformationVector;

// ****************************************************************************
//  Class:  vtkVisItStructuredGridNormals
//
//  Purpose:
//    Calculate cell or point centered normals.
//
//  Programmer:  Hank Childs
//  Creation:    December 28, 2006
//
// ****************************************************************************

class VISIT_VTK_API vtkVisItStructuredGridNormals : public vtkStructuredGridAlgorithm
{
  public:
    vtkTypeMacro(vtkVisItStructuredGridNormals,vtkStructuredGridAlgorithm);

    static vtkVisItStructuredGridNormals *New();

    void SetNormalTypeToCell()      { ComputePointNormals = false; }
    void SetNormalTypeToPoint()     { ComputePointNormals = true;  }

  protected:
    vtkVisItStructuredGridNormals();
    ~vtkVisItStructuredGridNormals() {};

    // Usual data generation method
    int RequestData(vtkInformation*, vtkInformationVector**,
                     vtkInformationVector*) override;
    void ExecutePoint(vtkStructuredGrid*, vtkStructuredGrid*);
    void ExecuteCell(vtkStructuredGrid *, vtkStructuredGrid*);

    bool  ComputePointNormals;

  private:
    vtkVisItStructuredGridNormals(const vtkVisItStructuredGridNormals&);  // Not implemented.
    void operator=(const vtkVisItStructuredGridNormals&);  // Not implemented.
};

#endif


