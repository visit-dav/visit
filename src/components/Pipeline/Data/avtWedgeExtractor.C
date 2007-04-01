// ************************************************************************* //
//                             avtWedgeExtractor.C                           //
// ************************************************************************* //

#include <avtWedgeExtractor.h>

#include <float.h>

#include <avtCellList.h>
#include <avtVolume.h>


int  avtWedgeExtractor::triangulationTables[64][13] = {
  {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //0
  { 0,  6,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //1
  { 0,  1,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //2
  { 6,  1,  7,  6,  2,  1, -1, -1, -1, -1, -1, -1, -1}, //3
  { 1,  2,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //4
  { 6,  1,  0,  6,  8,  1, -1, -1, -1, -1, -1, -1, -1}, //5
  { 0,  2,  8,  7,  0,  8, -1, -1, -1, -1, -1, -1, -1}, //6
  { 7,  6,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //7
  { 3,  5,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //8
  { 3,  5,  0,  5,  2,  0, -1, -1, -1, -1, -1, -1, -1}, //9
  { 0,  1,  7,  6,  3,  5, -1, -1, -1, -1, -1, -1, -1}, //10
  { 1,  7,  3,  1,  3,  5,  1,  5,  2, -1, -1, -1, -1}, //11
  { 2,  8,  1,  6,  3,  5, -1, -1, -1, -1, -1, -1, -1}, //12
  { 0,  3,  1,  1,  3,  5,  1,  5,  8, -1, -1, -1, -1}, //13
  { 6,  3,  5,  0,  8,  7,  0,  2,  8, -1, -1, -1, -1}, //14
  { 7,  3,  5,  7,  5,  8, -1, -1, -1, -1, -1, -1, -1}, //15
  { 7,  4,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //16
  { 7,  4,  3,  0,  6,  2, -1, -1, -1, -1, -1, -1, -1}, //17
  { 0,  1,  3,  1,  4,  3, -1, -1, -1, -1, -1, -1, -1}, //18
  { 1,  4,  3,  1,  3,  6,  1,  6,  2, -1, -1, -1, -1}, //19
  { 7,  4,  3,  2,  8,  1, -1, -1, -1, -1, -1, -1, -1}, //20
  { 7,  4,  3,  6,  1,  0,  6,  8,  1, -1, -1, -1, -1}, //21
  { 0,  4,  3,  0,  8,  4,  0,  2,  8, -1, -1, -1, -1}, //22
  { 6,  8,  3,  3,  8,  4, -1, -1, -1, -1, -1, -1, -1}, //23
  { 6,  7,  4,  6,  4,  5, -1, -1, -1, -1, -1, -1, -1}, //24
  { 0,  7,  5,  7,  4,  5,  2,  0,  5, -1, -1, -1, -1}, //25
  { 1,  6,  0,  1,  5,  6,  1,  4,  5, -1, -1, -1, -1}, //26 *
  { 2,  1,  5,  5,  1,  4, -1, -1, -1, -1, -1, -1, -1}, //27
  { 2,  8,  1,  6,  7,  5,  7,  4,  5, -1, -1, -1, -1}, //28
  { 0,  7,  5,  7,  4,  5,  0,  5,  1,  1,  5,  8, -1}, //29
  { 0,  2,  8,  0,  8,  4,  0,  4,  5,  0,  5,  6, -1}, //30
  { 8,  4,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //31
  { 4,  8,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //32
  { 4,  8,  5,  0,  6,  2, -1, -1, -1, -1, -1, -1, -1}, //33
  { 4,  8,  5,  0,  1,  7, -1, -1, -1, -1, -1, -1, -1}, //34
  { 4,  8,  5,  6,  1,  7,  6,  2,  1, -1, -1, -1, -1}, //35
  { 1,  5,  4,  2,  5,  1, -1, -1, -1, -1, -1, -1, -1}, //36
  { 1,  5,  4,  1,  6,  5,  1,  0,  6, -1, -1, -1, -1}, //37
  { 5,  4,  7,  5,  7,  0,  5,  0,  2, -1, -1, -1, -1}, //38 *
  { 6,  4,  7,  6,  5,  4, -1, -1, -1, -1, -1, -1, -1}, //39 *
  { 6,  3,  8,  3,  4,  8, -1, -1, -1, -1, -1, -1, -1}, //40 *
  { 0,  3,  4,  0,  4,  8,  0,  8,  2, -1, -1, -1, -1}, //41 *
  { 7,  0,  1,  6,  3,  4,  6,  4,  8, -1, -1, -1, -1}, //42
  { 1,  7,  3,  1,  3,  2,  2,  3,  8,  8,  3,  4, -1}, //43 *
  { 2,  6,  1,  6,  3,  1,  3,  4,  1, -1, -1, -1, -1}, //44 *
  { 0,  3,  1,  1,  3,  4, -1, -1, -1, -1, -1, -1, -1}, //45 *
  { 7,  0,  4,  4,  0,  2,  4,  2,  3,  3,  2,  6, -1}, //46 *
  { 7,  3,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //47 *
  { 7,  8,  5,  7,  5,  3, -1, -1, -1, -1, -1, -1, -1}, //48
  { 0,  6,  2,  7,  8,  5,  7,  5,  3, -1, -1, -1, -1}, //49
  { 0,  1,  3,  1,  5,  3,  1,  8,  5, -1, -1, -1, -1}, //50
  { 2,  1,  6,  6,  1,  3,  5,  1,  8,  3,  1,  5, -1}, //51
  { 1,  3,  7,  1,  5,  3,  1,  2,  5, -1, -1, -1, -1}, //52
  { 1,  0,  6,  1,  6,  5,  1,  5,  7,  7,  5,  3, -1}, //53
  { 0,  2,  5,  0,  5,  3, -1, -1, -1, -1, -1, -1, -1}, //54
  { 3,  6,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //55
  { 7,  8,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //56
  { 0,  7,  8,  0,  8,  2, -1, -1, -1, -1, -1, -1, -1}, //57
  { 0,  1,  6,  1,  8,  6, -1, -1, -1, -1, -1, -1, -1}, //58
  { 2,  1,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //59
  { 6,  7,  1,  6,  1,  2, -1, -1, -1, -1, -1, -1, -1}, //60 *
  { 0,  7,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //61 *
  { 0,  2,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //62 *
  {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}  //63
};

int   avtWedgeExtractor::verticesFromEdges[9][2] =
{
    { 0, 1 },   /* Edge 0 */
    { 1, 2 },   /* Edge 1 */
    { 2, 0 },   /* Edge 2 */
    { 3, 4 },   /* Edge 3 */
    { 4, 5 },   /* Edge 4 */
    { 5, 3 },   /* Edge 5 */
    { 0, 3 },   /* Edge 6 */
    { 1, 4 },   /* Edge 7 */
    { 2, 5 },   /* Edge 8 */
};


// ****************************************************************************
//  Method: avtWedgeExtractor constructor
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

avtWedgeExtractor::avtWedgeExtractor(int w, int h, int d,
                                     avtVolume *vol, avtCellList *cl)
    : avtExtractor(w, h, d, vol, cl)
{
    ;
}


// ****************************************************************************
//  Method: avtWedgeExtractor destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtWedgeExtractor::~avtWedgeExtractor()
{
    ;
}


// ****************************************************************************
//  Method: avtWedgeExtractor::Extract
//
//  Purpose:
//      Extracts the sample points from the wedge.  Does this by taking
//      slices along planes with constant x that contains sample points.  Then
//      sends the planes down to a base class method for finding the sample
//      points from a polygon.
//
//  Arguments:
//      wdg       The wedge to extract from.
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
// ****************************************************************************

void
avtWedgeExtractor::Extract(const avtWedge &wdg)
{
    int potentialNumSamples = ConstructBounds(wdg.pts, 6);

    if (potentialNumSamples <= 0)
    {
        ContributeSmallCell(wdg.pts, wdg.val, 6);
        return;
    }

    if (sendCellsMode && potentialNumSamples > 64)
    {
        celllist->Store(wdg, minx, maxx, miny, maxy);
        return;
    }

    //
    // minx and maxx are calculated in ConstructBounds.
    //
    for (int xi = minx ; xi <= maxx ; xi++)
    {
        float x = XFromIndex(xi);

        int   triIndex = IndexToTriangulationTable(wdg.pts, 6, x);

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
                int wdg_vertex1 = verticesFromEdges[triCase[tri_vertex]][0];
                int wdg_vertex2 = verticesFromEdges[triCase[tri_vertex]][1];

                InterpolateToPlane(wdg.pts[wdg_vertex1], wdg.pts[wdg_vertex2],
                                   wdg.val[wdg_vertex1], wdg.val[wdg_vertex2],
                                   x, y[tri_vertex], z[tri_vertex],
                                   v[tri_vertex], wdg.nVars);
            }

            //
            // Call the base class method for extracting sample points from a
            // triangle.
            //
            ExtractTriangle(xi, y, z, v, wdg.nVars);

            triCase += 3;
        }
    }
}


