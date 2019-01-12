#ifndef VTK_STRUCTURED_CREATE_TRIANGLES_H
#define VTK_STRUCTURED_CREATE_TRIANGLES_H

// ****************************************************************************
// Function: vtkStructuredCreateTriangles
//
// Purpose:
//   Template function that creates triangle geometry for structured and 
//   rectilinear grids based on slices that use a distance function.
//
// Notes:      This code used to be replicated 4x in vtkSlicer and 
//             vtkVisItContourFilter.
//
// Programmer: VisIt Team
// Creation:   Mon Mar 12 17:07:20 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

template <typename T, typename DistanceFunction>
inline void
vtkStructuredCreateTriangles(vtkSurfaceFromVolume &sfv, 
    const vtkIdType *CellList, vtkIdType CellListSize, vtkIdType nCells,
    const int pt_dims[3], const DistanceFunction &func)
{
    vtkIdType i, j, cell_dims[3];
    cell_dims[0] = pt_dims[0]-1;
    cell_dims[1] = pt_dims[1]-1;
    cell_dims[2] = pt_dims[2]-1;
    vtkIdType strideY = cell_dims[0];
    vtkIdType strideZ = cell_dims[0]*cell_dims[1];
    vtkIdType ptstrideY = pt_dims[0];
    vtkIdType ptstrideZ = pt_dims[0]*pt_dims[1];
    int X_val[8] = { 0, 1, 1, 0, 0, 1, 1, 0 };
    int Y_val[8] = { 0, 0, 1, 1, 0, 0, 1, 1 };
    int Z_val[8] = { 0, 0, 0, 0, 1, 1, 1, 1 };
    int nToProcess = (CellList != NULL ? CellListSize : nCells);
    for (i = 0 ; i < nToProcess ; i++)
    {
        vtkIdType cellId = (CellList != NULL ? CellList[i] : i);
        vtkIdType cellI = cellId % cell_dims[0];
        vtkIdType cellJ = (cellId/strideY) % cell_dims[1];
        vtkIdType cellK = (cellId/strideZ);
        int lookup_case = 0;
        T dist[8];
        for (j = 7 ; j >= 0 ; j--)
        {
            dist[j] = func(cellI, cellJ, cellK, X_val[j], Y_val[j], Z_val[j]);
            if (dist[j] >= 0)
                lookup_case++;
            if (j > 0)
                lookup_case *= 2;
        }

        int *triangulation_case = hexTriangulationTable[lookup_case];
        while (*triangulation_case != -1)
        {
            int tri[3];
            for (j = 0 ; j < 3 ; j++)
            {
                vtkIdType pt1 = hexVerticesFromEdges[triangulation_case[j]][0];
                vtkIdType pt2 = hexVerticesFromEdges[triangulation_case[j]][1];
                if (pt2 < pt1)
                {
                   vtkIdType tmp = pt2;
                   pt2 = pt1;
                   pt1 = tmp;
                }
                T dir = dist[pt2] - dist[pt1];
                T amt = 0. - dist[pt1];
                T percent = 1. - (amt / dir);
                vtkIdType ptId1 = (cellI + X_val[pt1]) +
                                  (cellJ + Y_val[pt1])*ptstrideY +
                                  (cellK + Z_val[pt1])*ptstrideZ;
                vtkIdType ptId2 = (cellI + X_val[pt2]) +
                                  (cellJ + Y_val[pt2])*ptstrideY +
                                  (cellK + Z_val[pt2])*ptstrideZ;
                tri[j] = sfv.AddPoint(ptId1, ptId2, percent);
            }
            sfv.AddTriangle(cellId, tri[0], tri[1], tri[2]);
            triangulation_case += 3;
        }
    }
}

// ****************************************************************************
// Function: vtkUnstructuredCreateTriangles
//
// Purpose:
//   Template function that can be used to help create triangle geometry for 
//   unstructured grids.
//
// Notes:      This code used to be replicated 4x in vtkSlicer and 
//             vtkVisItContourFilter.
//
// Programmer: VisIt Team
// Creation:   Mon Mar 12 17:07:20 PDT 2012
//
// Modifications:
//   
// ****************************************************************************


template <typename T, typename DistanceFunction>
inline void
vtkUnstructuredCreateTriangles(vtkSurfaceFromVolume &sfv,
    vtkIdType cellId, const vtkIdType *pts, vtkIdType npts,
    const int *triangulation_table, const int *vertices_from_edges, int tt_step,
    const DistanceFunction &func)
{
    const int max_pts = 8;
    T dist[max_pts];
    int lookup_case = 0;
    for (vtkIdType j = npts-1 ; j >= 0 ; j--)
    {
        dist[j] = func(pts[j]);
        if (dist[j] >= 0)
            lookup_case++;
        if (j > 0)
            lookup_case *= 2;
    }

    const int *triangulation_case = triangulation_table +lookup_case*tt_step;
    while (*triangulation_case != -1)
    {
        vtkIdType tri[3];
        for (int j = 0 ; j < 3 ; j++)
        {
            int pt1 = vertices_from_edges[2*triangulation_case[j]];
            int pt2 = vertices_from_edges[2*triangulation_case[j]+1];
            if (pt2 < pt1)
            {
               int tmp = pt2;
               pt2 = pt1;
               pt1 = tmp;
            }
            T dir = dist[pt2] - dist[pt1];
            T amt = T(0.) - dist[pt1];
            T percent = T(1.) - (amt / dir);
            tri[j] = sfv.AddPoint(pts[pt1], pts[pt2], percent);
        }
        sfv.AddTriangle(cellId, tri[0], tri[1], tri[2]);
        triangulation_case += 3;
    }
}
#endif
