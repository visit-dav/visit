#ifndef VTK_VISIT_CLIPPER_H
#define VTK_VISIT_CLIPPER_H

#include <visit_vtk_exports.h>
#include "vtkDataSetToUnstructuredGridFilter.h"

class vtkImplicitFunction;
class vtkUnstructuredGrid;

// ****************************************************************************
//  Class:  vtkVisItClipper3D
//
//  Purpose:
//    Clips a dataset using an implicit function.  This is a wholesale
//    replacement for the internal VTK clipper for structured, rectilinear,
//    and unstructured datasets with three topological dimensions and cell
//    types that are a part of the normal zoo.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 11, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 30 16:15:50 PST 2004
//    Added scalar array and scalar range for isovolume.
//
//    Jeremy Meredith, Mon Feb 16 19:08:16 PST 2004
//    Added PolyData support.
//
// ****************************************************************************
class VISIT_VTK_API vtkVisItClipper3D
    : public vtkDataSetToUnstructuredGridFilter
{
  public:
    vtkTypeRevisionMacro(vtkVisItClipper3D,vtkDataSetToUnstructuredGridFilter);
    void PrintSelf(ostream& os, vtkIndent indent);

    static vtkVisItClipper3D *New();

    virtual void SetClipFunction(vtkImplicitFunction*);
    virtual void SetClipScalars(float*, float, float);
    virtual void SetInsideOut(bool);

    void SetCellList(int *, int);


  protected:
    vtkVisItClipper3D();
    ~vtkVisItClipper3D();

    void Execute();
    void RectilinearGridExecute();
    void StructuredGridExecute();
    void UnstructuredGridExecute();
    void PolyDataExecute();
    void GeneralExecute();
    void ClipDataset(vtkDataSet *, vtkUnstructuredGrid *);

    int *CellList;
    int  CellListSize;
  private:
    bool insideOut;
    vtkImplicitFunction *clipFunction;
    float *scalarArray;
    float minValue;
    float maxValue;
    float avgValue;
    float halfDist;

    vtkVisItClipper3D(const vtkVisItClipper3D&);  // Not implemented.
    void operator=(const vtkVisItClipper3D&);  // Not implemented.
};


#endif


