#ifndef __vtkVisItPolyDataNormals_h
#define __vtkVisItPolyDataNormals_h

#include <visit_vtk_exports.h>
#include <vtkPolyDataAlgorithm.h>

class vtkPolyData;
class vtkInformation;
class vtkInformationVector;

// ****************************************************************************
//  Class:  vtkVisItPolyDataNormals
//
//  Purpose:
//    Calculate cell or point centered normals.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 12, 2003
//
// ****************************************************************************
class VISIT_VTK_API vtkVisItPolyDataNormals : public vtkPolyDataAlgorithm
{
  public:
    vtkTypeRevisionMacro(vtkVisItPolyDataNormals,vtkPolyDataAlgorithm);

    static vtkVisItPolyDataNormals *New();

    void SetFeatureAngle(float fa)  { FeatureAngle        = fa;    }
    void SetSplitting(bool s)       { Splitting           = s;     }
    void SetNormalTypeToCell()      { ComputePointNormals = false; }
    void SetNormalTypeToPoint()     { ComputePointNormals = true;  }

  protected:
    vtkVisItPolyDataNormals();
    ~vtkVisItPolyDataNormals() {};

    // Usual data generation method
    int RequestData(vtkInformation*, vtkInformationVector**,
                     vtkInformationVector*);
    void ExecutePointWithoutSplitting(vtkPolyData*, vtkPolyData*);
    void ExecutePointWithSplitting(vtkPolyData*, vtkPolyData*);
    void ExecuteCell(vtkPolyData *, vtkPolyData*);

    float FeatureAngle;
    bool  Splitting;
    bool  ComputePointNormals;

  private:
    vtkVisItPolyDataNormals(const vtkVisItPolyDataNormals&);  // Not implemented.
    void operator=(const vtkVisItPolyDataNormals&);  // Not implemented.
};

#endif
