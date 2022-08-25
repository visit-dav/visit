// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             vtkAxisDepthSort.h                            //
// ************************************************************************* //

#ifndef __vtkAxisDepthSort_h
#define __vtkAxisDepthSort_h
#include <visit_vtk_exports.h>

#include <vtkPolyDataAlgorithm.h>

// ****************************************************************************
//  Class: vtkAxisDepthSort
//
//  Purpose:
//    This will sort poly data along all six axes (+x, -x, +y, -y, +z, -z).
//
//  Programmer: Hank Childs
//  Creation:   July 13, 2002
//
//  Modifications:
//    Brad Whitlock, Mon Jul 15 15:43:22 PST 2002
//    Added API.
//
//    Eric Brugger, Wed Jan  9 10:54:24 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************

class VISIT_VTK_API vtkAxisDepthSort : public vtkPolyDataAlgorithm
{
  public:
    vtkTypeMacro(vtkAxisDepthSort, vtkPolyDataAlgorithm);

    static vtkAxisDepthSort      *New();

    vtkPolyData                  *GetPlusXOutput();
    vtkPolyData                  *GetMinusXOutput();
    vtkPolyData                  *GetPlusYOutput();
    vtkPolyData                  *GetMinusYOutput();
    vtkPolyData                  *GetPlusZOutput();
    vtkPolyData                  *GetMinusZOutput();

  protected:
                                  vtkAxisDepthSort();
    virtual                      ~vtkAxisDepthSort() {;}

    int                           RequestData(vtkInformation *,
                                      vtkInformationVector **,
                                      vtkInformationVector *) override;

  private:
                                  vtkAxisDepthSort(const vtkAxisDepthSort &) = delete;
                                  void operator=(const vtkAxisDepthSort &) = delete;
};


#endif
