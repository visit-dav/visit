// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTK_VISIT_CLIPPER_H
#define VTK_VISIT_CLIPPER_H
#include <visit_vtk_exports.h>

#include "vtkUnstructuredGridAlgorithm.h"

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
//    Jeremy Meredith, Tue Aug 29 13:43:56 EDT 2006
//    Added support for forcing removal of entire cells instead of clipping.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added flag to use zero-crossings of the implicit function. 
//    Added flat to compute inside and out in single execute.
//
//    Hank Childs, Sat Sep 29 11:14:58 PDT 2007
//    Make change to argument of function (SetClipScalars), which is necessary
//    for non-traditional cell types (hex-20).  Also add support for data
//    types beyond floats.
//
//    Brad Whitlock, Tue Mar 27 15:30:23 PDT 2012
//    Group members into FilterState class so it's easier to pass the filter's
//    state to internal template functions.
//
//    Kathleen Biagas, Tue Aug 15 11:22:11 MST 2012
//    Renamed SetUpClipFunction to ModifyClip.  Added SetPrecomputeClipScalars.
//
// ****************************************************************************

class VISIT_VTK_API vtkVisItClipper : public vtkUnstructuredGridAlgorithm
{
  public:
    vtkTypeMacro(vtkVisItClipper,vtkUnstructuredGridAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    static vtkVisItClipper *New();

    virtual void SetRemoveWholeCells(bool);
    virtual void SetClipFunction(vtkImplicitFunction*);
    virtual void SetClipScalars(vtkDataArray *, float);
    virtual void SetInsideOut(bool);
    virtual void SetComputeInsideAndOut(bool);
    virtual void SetUseZeroCrossings(bool);
    virtual vtkUnstructuredGrid *GetOtherOutput();

    void SetCellList(const vtkIdType *, vtkIdType);
    void SetPrecomputeClipScalars(const bool v);
    virtual void ModifyClip(vtkDataSet *, vtkIdType) {; };

    struct FilterState
    {
        FilterState();
       ~FilterState();
        void SetCellList(const vtkIdType *, vtkIdType);
        void SetClipFunction(vtkImplicitFunction *func);
        void SetClipScalars(vtkDataArray *, double);
        void ClipDataset(vtkDataSet *in_ds, vtkUnstructuredGrid *out_ds);

        const vtkIdType     *CellList;
        vtkIdType            CellListSize;
  
        vtkImplicitFunction *clipFunction;
        vtkDataArray        *scalarArrayAsVTK;
        double               scalarCutoff;

        vtkUnstructuredGrid *otherOutput;

        bool                 removeWholeCells;
        bool                 insideOut;
        bool                 useZeroCrossings;
        bool                 computeInsideAndOut;
        bool                 precomputeClipScalars;
    };

  protected:
    vtkVisItClipper();
    ~vtkVisItClipper();

    virtual int RequestData(vtkInformation *,
                            vtkInformationVector **,
                            vtkInformationVector *) override;
    virtual int FillInputPortInformation(int port, vtkInformation *info) override;

    void ClipDataset(vtkDataSet *, vtkUnstructuredGrid *);

  private:
    // Contains the state for the filter.
    FilterState state;

    vtkVisItClipper(const vtkVisItClipper&);  // Not implemented.
    void operator=(const vtkVisItClipper&);  // Not implemented.
};

#endif
