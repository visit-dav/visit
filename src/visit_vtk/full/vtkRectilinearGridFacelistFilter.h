// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkRectilinearGridFacelistFilter - get facelists for a rectilinear grid
// .SECTION Description
// vtkRectilinearGridFacelistFilter is a filter that extracts facelists from a
// rectilinear grid.

// .SECTION See Also
// vtkGeometryFilter

#ifndef __vtkRectilinearGridFacelistFilter_h
#define __vtkRectilinearGridFacelistFilter_h
#include <visit_vtk_exports.h>

#include "vtkPolyDataAlgorithm.h"

#include <visit-config.h>

class     vtkIdTypeArray;
class     vtkRectilinearGrid;

#include <vector>

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Sun Nov  9 12:37:15 PST 2003
//    Added separate consolidation routines that work with and without ghost
//    zones.
//
//    Hank Childs, Tue Jan 24 10:11:22 PST 2006
//    Add support for ghost nodes.
//
//    Kathleen Biagas, Thu Aug 11, 2022
//    Add new signature for ConsolidateFacesWithGhostZones for VTK9: add a new
//    array for offsets as they are now stored in a separate array than
//    connectivity.
//
// ****************************************************************************

class VISIT_VTK_API vtkRectilinearGridFacelistFilter :
  public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkRectilinearGridFacelistFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Create a vtkRectilinearGridFacelistFilter.
  static vtkRectilinearGridFacelistFilter *New();

  // Description:
  // Set/Get ForceFaceConsolidation
  vtkSetMacro(ForceFaceConsolidation, int);
  vtkGetMacro(ForceFaceConsolidation, int);

protected:
  vtkRectilinearGridFacelistFilter();
  ~vtkRectilinearGridFacelistFilter() {};

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;
  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

  int  ForceFaceConsolidation;

private:
  vtkRectilinearGridFacelistFilter(const vtkRectilinearGridFacelistFilter&);
  void operator=(const vtkRectilinearGridFacelistFilter&);

  void ConsolidateFacesWithoutGhostZones(vtkRectilinearGrid *, vtkPolyData *);
  vtkPolyData *ConsolidateFacesWithGhostZones(vtkPolyData *, vtkIdTypeArray *,
               vtkIdTypeArray *, std::vector<int>&, std::vector<int>&,std::vector<int>&);

  static const vtkIdType quads2[1][4];
  static const vtkIdType quads3[6][4];
};

#endif
