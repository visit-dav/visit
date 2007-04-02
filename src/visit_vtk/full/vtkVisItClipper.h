#ifndef VTK_VISIT_CLIPPER_H
#define VTK_VISIT_CLIPPER_H

#include <visit_vtk_exports.h>
#include "vtkDataSetToUnstructuredGridFilter.h"

class vtkImplicitFunction;
class vtkUnstructuredGrid;

// ****************************************************************************
//  Class:  vtkVisItClipper
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
//    Jeremy Meredith, Wed May  5 13:06:14 PDT 2004
//    Renamed without the "3D" because I also made it support 2D.
//    Changed it to a single cutoff for scalars to make the math more robust.
//
// ****************************************************************************
class VISIT_VTK_API vtkVisItClipper
    : public vtkDataSetToUnstructuredGridFilter
{
  public:
    vtkTypeRevisionMacro(vtkVisItClipper,vtkDataSetToUnstructuredGridFilter);
    void PrintSelf(ostream& os, vtkIndent indent);

    static vtkVisItClipper *New();

    virtual void SetClipFunction(vtkImplicitFunction*);
    virtual void SetClipScalars(float*, float);
    virtual void SetInsideOut(bool);

    void SetCellList(int *, int);
    virtual void SetUpClipFunction(int) { ; };


  protected:
    vtkVisItClipper();
    ~vtkVisItClipper();

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
    bool   insideOut;
    vtkImplicitFunction *clipFunction;
    float *scalarArray;
    float  scalarCutoff;
    bool   scalarFlip;

    vtkVisItClipper(const vtkVisItClipper&);  // Not implemented.
    void operator=(const vtkVisItClipper&);  // Not implemented.
};


#endif


