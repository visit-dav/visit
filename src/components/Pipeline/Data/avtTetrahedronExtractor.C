// ************************************************************************* //
//                           avtTetrahedronExtractor.C                        //
// ************************************************************************* //

#include <avtTetrahedronExtractor.h>

#include <float.h>

#include <avtCellList.h>
#include <avtVolume.h>


int  avtTetrahedronExtractor::triangulationTables[16][7] = {
  {-1, -1, -1, -1, -1, -1, -1},
  { 0, 3, 2, -1, -1, -1, -1},
  { 0, 1, 4, -1, -1, -1, -1},
  { 3, 2, 4, 4, 2, 1, -1},
  { 1, 2, 5, -1, -1, -1, -1},
  { 3, 5, 1, 3, 1, 0, -1},
  { 0, 2, 5, 0, 5, 4, -1},
  { 3, 5, 4, -1, -1, -1, -1},
  { 3, 4, 5, -1, -1, -1, -1},
  { 0, 4, 5, 0, 5, 2, -1},
  { 0, 5, 3, 0, 1, 5, -1},
  { 5, 2, 1, -1, -1, -1, -1},
  { 3, 4, 1, 3, 1, 2, -1},
  { 0, 4, 1, -1, -1, -1, -1},
  { 0, 2, 3, -1, -1, -1, -1},
  {-1, -1, -1, -1, -1, -1, -1}
};

int   avtTetrahedronExtractor::verticesFromEdges[6][2] =
{
    { 0, 1 },   /* Edge 0 */
    { 1, 2 },   /* Edge 1 */
    { 2, 0 },   /* Edge 2 */
    { 0, 3 },   /* Edge 3 */
    { 1, 3 },   /* Edge 4 */
    { 2, 3 }    /* Edge 5 */
};


// ****************************************************************************
//  Method: avtTetrahedronExtractor constructor
//
//  Arguments:
//     w     The number of sample points in the x direction (width).
//     h     The number of sample points in the y direction (height).
//     d     The number of sample points in the z direction (depth).
//     vol   The volume to put samples into.
//     cl    The cell list to put cells whose sampling was deferred.
//
//  Programmer: Hank Childs
//  Creation:   December 11, 2000
//
// ****************************************************************************

avtTetrahedronExtractor::avtTetrahedronExtractor(int w, int h, int d,
                                                 avtVolume *vol,
                                                 avtCellList *cl)
    : avtExtractor(w, h, d, vol, cl)
{
    ;
}


// ****************************************************************************
//  Method: avtTetrahedronExtractor destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTetrahedronExtractor::~avtTetrahedronExtractor()
{
    ;
}


// ****************************************************************************
//  Method: avtTetrahedronExtractor::Extract
//
//  Purpose:
//      Extracts the sample points from the tetrahedron.  Does this by taking
//      slices along planes with constant x that contains sample points.  Then
//      sends the planes down to a base class method for finding the sample
//      points from a polygon.
//
//  Arguments:
//      tet       The tetrahedron to extract from.
//
//  Programmer:   Hank Childs
//  Creation:     December 11, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Jan 27 21:03:50 PST 2001
//    Accounted for deferring the sampling of cells.
//
//    Hank Childs, Sat Feb  3 15:48:09 PST 2001
//    Changed slicing method from being z first to x first to maximize cache
//    hits.
//
//    Hank Childs, Tue Nov 13 16:14:22 PST 2001
//    Allowed for multiple variables.
//
//    Hank Childs, Wed Jan 23 11:05:37 PST 2002
//    Turned on contribution of small cells.
//
//    Hank Childs, Tue Dec 21 15:53:29 PST 2004
//    Added an optimization for tiling where we only iterate over x-slices
//    within the tile.
//
// ****************************************************************************

void
avtTetrahedronExtractor::Extract(const avtTetrahedron &tet)
{
    int potentialNumSamples = ConstructBounds(tet.pts, 4);

    if (potentialNumSamples <= 0)
    {
        ContributeSmallCell(tet.pts, tet.val, 4);
        return;
    }

    if (sendCellsMode && potentialNumSamples > 64)
    {
        celllist->Store(tet, minx, maxx, miny, maxy);
        return;
    }

    //
    // minx and maxx are calculated in ConstructBounds.
    //
    int minx_iter = (minx < restrictedMinWidth ? restrictedMinWidth : minx);
    int maxx_iter = (maxx > restrictedMaxWidth ? restrictedMaxWidth : maxx);
    for (int xi = minx_iter ; xi <= maxx_iter ; xi++)
    {
        float x = XFromIndex(xi);

        int   triIndex = IndexToTriangulationTable(tet.pts, 4, x);

        //
        // The triCase will have sets of three vertices, each of which makes
        // up a triangle that is part of the intersection of this cell with
        // the plane.  Take each triangle and find the sample points from it.
        //
        int  *triCase  = triangulationTables[triIndex];
        while (*triCase != -1)
        {
            //
            // Find the triangle for this tri case by seeing which edge the
            // triangle intersects and then interpolating along that edge
            // three times to form the triangle.
            //
            float y[3], z[3], v[3][AVT_VARIABLE_LIMIT];
            for (int tri_vertex = 0 ; tri_vertex < 3 ; tri_vertex++)
            {
                int tet_vertex1 = verticesFromEdges[triCase[tri_vertex]][0];
                int tet_vertex2 = verticesFromEdges[triCase[tri_vertex]][1];

                InterpolateToPlane(tet.pts[tet_vertex1], tet.pts[tet_vertex2],
                                   tet.val[tet_vertex1], tet.val[tet_vertex2],
                                   x, y[tri_vertex], z[tri_vertex],
                                   v[tri_vertex], tet.nVars);
            }

            //
            // Call the base class method for extracting sample points from a
            // triangle.
            //
            ExtractTriangle(xi, y, z, v, tet.nVars);

            triCase += 3;
        }
    }
}


