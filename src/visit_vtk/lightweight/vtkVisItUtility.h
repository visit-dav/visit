// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             vtkVisItUtility.h                             //
// ************************************************************************* //

#ifndef VTK_VISIT_UTILITY_H
#define VTK_VISIT_UTILITY_H

#include <visit_vtk_light_exports.h>
#include <vtkType.h>

class vtkCell;
class vtkDataSet;
class vtkObject;
class vtkPoints;
class vtkRectilinearGrid;

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Sat Mar 18 14:16:09 PST 2006
//    Added function CellContainsPoint.
//
//    Kathleen Bonnell, Thu Jun 11 08:20:11 PDT 2009
//    Added optional tolerance argument to PointsEqual.
//
//    Kathleen Bonnell, Thu Feb 17 08:53:30 PST 2011
//    Added CreateEmptyRGrid.
//
//    Tom Fogal, Tue Sep 27 12:25:55 MDT 2011
//    Add API for keeping track of static VTK objects.
//
//    Brad Whitlock, Wed Mar 21 12:01:11 PDT 2012
//    I added NewPoints.
//
//    Gunther H. Weber, Thu Aug 22 10:15:58 PDT 2013
//    Added option to allow negaitve inddices (false by default) to
//    GetLogicalIndices.
//
//    Mark C. Miller, Wed Oct  1 19:44:34 PDT 2014
//    Add SafeDoubleToFloat
//
//    Eric Brugger, Thu Nov 16 17:29:08 PST 2017
//    I made the default data type for Create1DRGrid and CreateEmptyRGrid
//    be VTK_DOUBLE instead of VTK_FLOAT.
//
// ****************************************************************************

namespace vtkVisItUtility
{
    VISIT_VTK_LIGHT_API vtkPoints  *GetPoints(vtkDataSet *);
    VISIT_VTK_LIGHT_API vtkPoints  *NewPoints(vtkDataSet *);
    VISIT_VTK_LIGHT_API void        GetLogicalIndices(vtkDataSet *, const bool,
                                                const int, int [3],
                                                const bool = false,
                                                const bool = true,
                                                const bool = false);
    VISIT_VTK_LIGHT_API int         CalculateRealID(const int, const bool,
                                              vtkDataSet *ds);
    VISIT_VTK_LIGHT_API int         ComputeStructuredCoordinates(
                                              vtkRectilinearGrid *,
                                              double x[3], int ijk[3]);
    VISIT_VTK_LIGHT_API int         FindCell(vtkDataSet *, double pt[3]);
    VISIT_VTK_LIGHT_API void        GetDimensions(vtkDataSet *, int[3]);
    VISIT_VTK_LIGHT_API int         NodeGhostIdFromNonGhost(vtkDataSet *ds,
                                        const int);
    VISIT_VTK_LIGHT_API int         ZoneGhostIdFromNonGhost(vtkDataSet *ds,
                                        const int);
    VISIT_VTK_LIGHT_API int         CalculateGhostIdFromNonGhost(
                                        vtkDataSet *ds,
                                        const int cellId,
                                        const bool forCell);
    VISIT_VTK_LIGHT_API int         GetLocalElementForGlobal(
                                        vtkDataSet *ds,
                                        const int elementId,
                                        const bool forCell);

    VISIT_VTK_LIGHT_API void       GetCellCenter(vtkCell* cell, double center[3]);
    VISIT_VTK_LIGHT_API bool       ContainsMixedGhostZoneTypes(vtkDataSet *);
    VISIT_VTK_LIGHT_API bool       CellContainsPoint(vtkCell *, const double *);
    VISIT_VTK_LIGHT_API void       WriteDataSet(vtkDataSet*, const char *);
    VISIT_VTK_LIGHT_API vtkRectilinearGrid *
                                   Create1DRGrid(int nXCoords,
                                                 int type = VTK_DOUBLE);
    VISIT_VTK_LIGHT_API vtkRectilinearGrid *
                                   CreateEmptyRGrid(int nXCoords,
                                                    int nYCoords = 1,
                                                    int nZCoords = 1,
                                                    int type = VTK_DOUBLE);
    VISIT_VTK_LIGHT_API bool       PointsEqual(double p1[3], double p2[3],
                                               const double *_eps = 0);
    VISIT_VTK_LIGHT_API void       RegisterStaticVTKObject(vtkObject*);
    VISIT_VTK_LIGHT_API void       CleanupStaticVTKObjects();
    VISIT_VTK_LIGHT_API float      SafeDoubleToFloat(double);
}
#endif
