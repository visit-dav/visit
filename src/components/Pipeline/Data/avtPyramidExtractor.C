// ************************************************************************* //
//                             avtPyramidExtractor.C                         //
// ************************************************************************* //

#include <avtPyramidExtractor.h>

#include <float.h>

#include <avtCellList.h>
#include <avtVolume.h>


int  avtPyramidExtractor::triangulationTables[32][13] = {
  {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //0
  { 0,  4,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //1
  { 0,  1,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //2
  { 4,  1,  5,  4,  3,  1, -1, -1, -1, -1, -1, -1, -1}, //3
  { 1,  2,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //4
  { 0,  4,  3,  1,  2,  6, -1, -1, -1, -1, -1, -1, -1}, //5
  { 0,  2,  5,  5,  2,  6, -1, -1, -1, -1, -1, -1, -1}, //6
  { 4,  3,  2,  5,  4,  2,  6,  5,  2, -1, -1, -1, -1}, //7
  { 3,  7,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //8
  { 4,  7,  2,  2,  0,  4, -1, -1, -1, -1, -1, -1, -1}, //9
  { 0,  1,  5,  3,  7,  2, -1, -1, -1, -1, -1, -1, -1}, //10
  { 1,  5,  2,  2,  5,  4,  2,  4,  7, -1, -1, -1, -1}, //11
  { 1,  3,  6,  6,  3,  7, -1, -1, -1, -1, -1, -1, -1}, //12
  { 1,  7,  6,  1,  4,  7,  1,  0,  4, -1, -1, -1, -1}, //13
  { 0,  3,  5,  5,  3,  6,  6,  3,  7, -1, -1, -1, -1}, //14
  { 5,  4,  7,  6,  5,  7, -1, -1, -1, -1, -1, -1, -1}, //15
  { 5,  7,  4,  6,  7,  5, -1, -1, -1, -1, -1, -1, -1}, //16 *
  { 0,  5,  3,  5,  6,  3,  6,  7,  3, -1, -1, -1, -1}, //17 *
  { 0,  1,  4,  1,  7,  4,  1,  6,  7, -1, -1, -1, -1}, //18 *
  { 1,  6,  3,  6,  7,  3, -1, -1, -1, -1, -1, -1, -1}, //19 *
  { 1,  2,  5,  2,  4,  5,  2,  7,  4, -1, -1, -1, -1}, //20
  { 0,  7,  3,  0,  5,  7,  5,  2,  7,  5,  1,  2, -1}, //21
  { 0,  2,  4,  4,  2,  7, -1, -1, -1, -1, -1, -1, -1}, //22
  { 3,  2,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //23
  { 3,  4,  2,  2,  4,  5,  2,  5,  6, -1, -1, -1, -1}, //24
  { 0,  5,  2,  5,  6,  2, -1, -1, -1, -1, -1, -1, -1}, //25
  { 0,  1,  6,  0,  6,  4,  4,  6,  3,  6,  2,  3, -1}, //26
  { 1,  6,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //27
  { 3,  4,  1,  4,  5,  1, -1, -1, -1, -1, -1, -1, -1}, //28
  { 0,  5,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //29
  { 0,  3,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //30
  {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}  //31
};

int   avtPyramidExtractor::verticesFromEdges[8][2] =
{
    { 0, 1 },   /* Edge 0 */
    { 1, 2 },   /* Edge 1 */
    { 2, 3 },   /* Edge 2 */
    { 3, 0 },   /* Edge 3 */
    { 0, 4 },   /* Edge 4 */
    { 1, 4 },   /* Edge 5 */
    { 2, 4 },   /* Edge 6 */
    { 3, 4 }    /* Edge 7 */
};


// ****************************************************************************
//  Method: avtPyramidExtractor constructor
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

avtPyramidExtractor::avtPyramidExtractor(int w, int h, int d,
                                         avtVolume *vol, avtCellList *cl)
    : avtExtractor(w, h, d, vol, cl)
{
    ;
}


// ****************************************************************************
//  Method: avtPyramidExtractor destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtPyramidExtractor::~avtPyramidExtractor()
{
    ;
}


// ****************************************************************************
//  Method: avtPyramidExtractor::Extract
//
//  Purpose:
//      Extracts the sample points from the pyramid.  Does this by taking
//      slices along planes with constant x that contains sample points.  Then
//      sends the planes down to a base class method for finding the sample
//      points from a polygon.
//
//  Arguments:
//      pyr       The pyramid to extract from.
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
avtPyramidExtractor::Extract(const avtPyramid &pyr)
{
    int potentialNumSamples = ConstructBounds(pyr.pts, 5);

    if (potentialNumSamples <= 0)
    {
        ContributeSmallCell(pyr.pts, pyr.val, 5);
        return;
    }

    if (sendCellsMode && potentialNumSamples > 64)
    {
        celllist->Store(pyr, minx, maxx, miny, maxy);
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

        int   triIndex = IndexToTriangulationTable(pyr.pts, 5, x);

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
                int pyr_vertex1 = verticesFromEdges[triCase[tri_vertex]][0];
                int pyr_vertex2 = verticesFromEdges[triCase[tri_vertex]][1];

                InterpolateToPlane(pyr.pts[pyr_vertex1], pyr.pts[pyr_vertex2],
                                   pyr.val[pyr_vertex1], pyr.val[pyr_vertex2],
                                   x, y[tri_vertex], z[tri_vertex],
                                   v[tri_vertex], pyr.nVars);
            }

            //
            // Call the base class method for extracting sample points from a
            // triangle.
            //
            ExtractTriangle(xi, y, z, v, pyr.nVars);

            triCase += 3;
        }
    }
}


