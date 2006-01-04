// ************************************************************************* //
//                            avtMassVoxelExtractor.C                        //
// ************************************************************************* //

#include <avtMassVoxelExtractor.h>

#include <float.h>

#include <avtCellList.h>
#include <avtVolume.h>

#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtMassVoxelExtractor constructor
//
//  Arguments:
//     w     The number of sample points in the x direction (width).
//     h     The number of sample points in the y direction (height).
//     d     The number of sample points in the z direction (depth).
//     vol   The volume to put samples into.
//     cl    The cell list to put cells whose sampling was deferred.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Nov 19 14:50:58 PST 2004
//    Initialize gridsAreInWorldSpace.
//
// ****************************************************************************

avtMassVoxelExtractor::avtMassVoxelExtractor(int w, int h, int d,
                                               avtVolume *vol, avtCellList *cl)
    : avtExtractor(w, h, d, vol, cl)
{
    gridsAreInWorldSpace = false;
    aspect = 1;
    view_to_world_transform = vtkMatrix4x4::New();
    ProportionSpaceToZBufferSpace = new float[depth];
    divisors_X = NULL;
    divisors_Y = NULL;
    divisors_Z = NULL;

    prop_buffer   = new float[3*depth];
    ind_buffer    = new int[3*depth];
    valid_sample  = new bool[depth];
}


// ****************************************************************************
//  Method: avtMassVoxelExtractor destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//
//    Hank Childs, Sun Nov 21 10:35:40 PST 2004
//    Delete the view to world transform.
//
// ****************************************************************************

avtMassVoxelExtractor::~avtMassVoxelExtractor()
{
    view_to_world_transform->Delete();
    delete [] ProportionSpaceToZBufferSpace;
    delete [] prop_buffer;
    delete [] ind_buffer;
    delete [] valid_sample;
    if (divisors_X != NULL)
        delete [] divisors_X;
    if (divisors_Y != NULL)
        delete [] divisors_Y;
    if (divisors_Z != NULL)
        delete [] divisors_Z;
}


// ****************************************************************************
//  Method: avtMassVoxelExtractor::Extract
//
//  Purpose:
//      Extracts the grid into the sample points.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2004
//
// ****************************************************************************

void
avtMassVoxelExtractor::Extract(vtkRectilinearGrid *rgrid)
{
    if (gridsAreInWorldSpace)
        ExtractWorldSpaceGrid(rgrid);
    else
        ExtractImageSpaceGrid(rgrid);
}


// ****************************************************************************
//  Method: avtMassVoxelExtractor::SetGridsAreInWorldSpace
//
//  Purpose:
//      Tells the MVE whether or not it is extracting a world space or image
//      space grid.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2004
//
// ****************************************************************************

void
avtMassVoxelExtractor::SetGridsAreInWorldSpace(bool val, const avtViewInfo &v,
                                               double asp)
{
    gridsAreInWorldSpace = val;
    if (gridsAreInWorldSpace)
    {
        view = v;
        aspect = asp;

        //
        // Set up a VTK camera.  This will allow us to get the direction of
        // each ray and also the origin of each ray (this is simply the
        // position of the camera for perspective projection).
        //
        vtkCamera *cam = vtkCamera::New();
        view.SetCameraFromView(cam);
        cam->GetClippingRange(cur_clip_range);
        vtkMatrix4x4 *mat = cam->GetCompositePerspectiveTransformMatrix(aspect,
                                         cur_clip_range[0], cur_clip_range[1]);
        vtkMatrix4x4::Invert(mat, view_to_world_transform);
        cam->Delete();

        //
        // When we are casting a ray, we sample along the segment that the
        // ray makes when intersecting it with the near and far planes of the
        // view frustum.  The temptation is to sample at even intervals along
        // this segment.  Unfortunately, our alternative, unstructured 
        // volume renderer does its sampling in Z-buffer space.  In addition,
        // when comparing with opaque images, it is more convenient to have
        // samples be done in even intervals in Z-buffer space.
        //
        // So: we are going to do the sampling in even intervals in Z-buffer
        // space (as opposed to world space).  When incrementing over a segment
        // it would be too costly to calculate all of the increments in
        // Z-buffer space.  So instead we will calculate them here once and
        // use them repeatedly when we do our sampling.
        //
        // So here's the gameplan:
        // For a sample K, we are going to want to know what proportion (in
        // world space) to move along the segment to be at position K/N in
        // Z-buffer space along the sample.  Concretely, if we wanted to
        // find sample 125/250 in Z-buffer space, that may map to proportion 
        // 0.8 in world space along the segment.  So S[125] = 0.8.
        //
        // Fortunately, there is a nice property of the projection matrix
        // here.  You can calculate S[K] for one ray, and it will apply to all
        // rays.  I intend to add a derivation showing this.  It was one my
        // whiteboard at one time.  :)
        // 
        float view_near[4];
        float world_near[4];
        view_near[0] = 0;
        view_near[1] = 0;
        view_near[2] = cur_clip_range[0];
        view_near[3] = 1.;

        view_to_world_transform->MultiplyPoint(view_near, world_near);
        if (world_near[3] != 0)
        {
            world_near[0] /= world_near[3];
            world_near[1] /= world_near[3];
            world_near[2] /= world_near[3];
        }

        float view_far[4];
        float world_far[4];
        view_far[0] = 0;
        view_far[1] = 0;
        view_far[2] = cur_clip_range[1];
        view_far[3] = 1.;

        view_to_world_transform->MultiplyPoint(view_far, world_far);
        if (world_far[3] != 0)
        {
            world_far[0] /= world_far[3];
            world_far[1] /= world_far[3];
            world_far[2] /= world_far[3];
        }

        float diff[3];
        diff[0] = world_far[0] - world_near[0];
        diff[1] = world_far[1] - world_near[1];
        diff[2] = world_far[2] - world_near[2];
        float total_dist = 
                     sqrt(diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]);
 
        for (int i = 0 ; i < depth ; i++)
        {
            float view_i[4];
            view_i[0] = 0;
            view_i[1] = 0;
            view_i[2] = cur_clip_range[0] + 
                          (cur_clip_range[1] - cur_clip_range[0])*
                          (float(i)/(depth-1.));
            view_i[3] = 1.;
  
            float world_i[4];
            view_to_world_transform->MultiplyPoint(view_i, world_i);
            if (world_i[3] != 0)
            {
                world_i[0] /= world_i[3];
                world_i[1] /= world_i[3];
                world_i[2] /= world_i[3];
            }
    
            diff[0] = world_i[0] - world_near[0];
            diff[1] = world_i[1] - world_near[1];
            diff[2] = world_i[2] - world_near[2];
            float dist = 
                     sqrt(diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]);
 
            ProportionSpaceToZBufferSpace[i] = dist / total_dist;
        }
    }
}


// ****************************************************************************
//  Method: avtMassVoxelExtractor::ExtractWorldSpaceGrid
//
//  Purpose:
//      Extracts a grid that has already been put into world space.  This case
//      typically corresponds to resampling.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Dec 10 10:36:40 PST 2004
//    Use the "restricted" screen space.  This will normally be the whole
//    screen space, but if we are tiling, then it will correspond to only
//    the tile.
//
// ****************************************************************************

void
avtMassVoxelExtractor::ExtractWorldSpaceGrid(vtkRectilinearGrid *rgrid)
{
    //
    // Some of our sampling routines need a chance to pre-process the data.
    // Register the grid here so we can do that.
    //
    RegisterGrid(rgrid);

    //
    // Set up a list of ranges to look at.
    //
    const int max_ranges = 100; // this should be bigger than log(max(W,H))
    int width_min[max_ranges];
    int width_max[max_ranges];
    int height_min[max_ranges];
    int height_max[max_ranges];
    int curRange = 0;
    width_min[curRange] = restrictedMinWidth;
    width_max[curRange] = restrictedMaxWidth+1;
    height_min[curRange] = restrictedMinHeight;
    height_max[curRange] = restrictedMaxHeight+1;
    curRange++;

    while (curRange > 0)
    {
        //
        // Determine what range we are dealing with on this iteration.
        //
        curRange--;
        int w_min = width_min[curRange];
        int w_max = width_max[curRange];
        int h_min = height_min[curRange];
        int h_max = height_max[curRange];

        //
        // Let's find out if this range can even intersect the dataset.
        // If not, just skip it.
        //
        if (!FrustumIntersectsGrid(w_min, w_max, h_min, h_max))
        {
            continue;
        }

        int num_rays = (w_max-w_min)*(h_max-h_min);
        if (num_rays > 5)
        {
            //
            // Break the range down into smaller subchunks and recurse.
            //
            int w_mid = (w_min+w_max) / 2;
            int h_mid = (h_min+h_max) / 2;
            width_min[curRange] = w_min; width_max[curRange] = w_mid;
            height_min[curRange] = h_min; height_max[curRange] = h_mid;
            curRange++;
            width_min[curRange] = w_mid; width_max[curRange] = w_max;
            height_min[curRange] = h_min; height_max[curRange] = h_mid;
            curRange++;
            width_min[curRange] = w_min; width_max[curRange] = w_mid;
            height_min[curRange] = h_mid; height_max[curRange] = h_max;
            curRange++;
            width_min[curRange] = w_mid; width_max[curRange] = w_max;
            height_min[curRange] = h_mid; height_max[curRange] = h_max;
            curRange++;
        }
        else
        {
            //
            // We have a small amount of rays, so just evaluate them.
            //
            for (int i = w_min ; i < w_max ; i++)
                for (int j = h_min ; j < h_max ; j++)
                {
                    float origin[4];
                    float terminus[4];
                    GetSegment(i, j, origin, terminus);
                    SampleAlongSegment(origin, terminus, i, j);
                }
        }
    }
}


// ****************************************************************************
//  Method: avtMassVoxelExtractor::RegisterGrid
//
//  Purpose:
//      Registers a rectilinear grid.  This is called in combination with
//      SampleAlongSegment, which assumes that data members in this class have
//      been set up.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2004
//
// ****************************************************************************

void
avtMassVoxelExtractor::RegisterGrid(vtkRectilinearGrid *rgrid)
{
    int  i;

    X = (float *) rgrid->GetXCoordinates()->GetVoidPointer(0);
    Y = (float *) rgrid->GetYCoordinates()->GetVoidPointer(0);
    Z = (float *) rgrid->GetZCoordinates()->GetVoidPointer(0);
    rgrid->GetDimensions(dims);

    vtkDataArray *arr = rgrid->GetCellData()->GetArray("avtGhostZones");
    if (arr != NULL)
        ghosts = (unsigned char *) arr->GetVoidPointer(0);
    else
        ghosts = NULL;

    ncell_arrays = 0;
    for (i = 0 ; i < rgrid->GetCellData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = rgrid->GetCellData()->GetArray(i);
        if (arr->GetDataType() != VTK_FLOAT)
            continue;
        if (arr->GetNumberOfComponents() != 1)
            continue;
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        cell_arrays[ncell_arrays++] = (float *) arr->GetVoidPointer(0);
    }
    npt_arrays = 0;
    for (i = 0 ; i < rgrid->GetPointData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = rgrid->GetPointData()->GetArray(i);
        if (arr->GetDataType() != VTK_FLOAT)
            continue;
        if (arr->GetNumberOfComponents() != 1)
            continue;
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        pt_arrays[npt_arrays++] = (float *) arr->GetVoidPointer(0);
    }

    if (divisors_X != NULL)
        delete [] divisors_X;
    if (divisors_Y != NULL)
        delete [] divisors_Y;
    if (divisors_Z != NULL)
        delete [] divisors_Z;

    //
    // We end up dividing by the term A[i+1]/A[i] a whole bunch.  So store
    // out its inverse so that we can do cheap multiplication.  This gives us
    // a 5% performance boost.
    //
    divisors_X = new float[dims[0]-1];
    for (i = 0 ; i < dims[0] - 1 ; i++)
        divisors_X[i] = (X[i+1] == X[i] ? 1. : 1./(X[i+1]-X[i]));
    divisors_Y = new float[dims[1]-1];
    for (i = 0 ; i < dims[1] - 1 ; i++)
        divisors_Y[i] = (Y[i+1] == Y[i] ? 1. : 1./(Y[i+1]-Y[i]));
    divisors_Z = new float[dims[2]-1];
    for (i = 0 ; i < dims[2] - 1 ; i++)
        divisors_Z[i] = (Z[i+1] == Z[i] ? 1. : 1./(Z[i+1]-Z[i]));
}


// ****************************************************************************
//  Method: avtMassVoxelExtractor::GetSegment
//
//  Purpose:
//      Gets a line segment based on a pixel location.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2004
//
// ****************************************************************************

void
avtMassVoxelExtractor::GetSegment(int w, int h, float *origin, float *terminus)
    const
{
    float view[4];

    //
    // The image is being reflected across a center vertical line.  This is the
    // easiest place to fix it.  I haven't tracked down where it is, but it
    // would appear to a problem with the interface from our images to VTK's.
    //
    view[0] = (w - width/2.)/(width/2.);
    view[1] = (h - height/2.)/(height/2.);
    view[2] = cur_clip_range[0];
    view[3] = 1.;
    view_to_world_transform->MultiplyPoint(view, origin);
    if (origin[3] != 0.)
    {
        origin[0] /= origin[3];
        origin[1] /= origin[3];
        origin[2] /= origin[3];
    }

    view[0] = (w - width/2.)/(width/2.);
    view[1] = (h - height/2.)/(height/2.);
    view[2] = cur_clip_range[1];
    view[3] = 1.;
    view_to_world_transform->MultiplyPoint(view, terminus);
    if (terminus[3] != 0.)
    {
        terminus[0] /= terminus[3];
        terminus[1] /= terminus[3];
        terminus[2] /= terminus[3];
    }
}


// ****************************************************************************
//  Method: avtMassVoxelExtractor::FrustumIntersectsGrid
//
//  Purpose:
//      Determines if a frustum intersects the grid.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2004
//
// ****************************************************************************

bool
avtMassVoxelExtractor::FrustumIntersectsGrid(int w_min, int w_max, int h_min,
                                             int h_max) const
{
    //
    // Start off by getting the segments corresponding to the bottom left (bl),
    // upper left (ul), bottom right (br), and upper right (ur) rays.
    //
    float bl_start[4];
    float bl_end[4];
    GetSegment(w_min, h_min, bl_start, bl_end);

    float ul_start[4];
    float ul_end[4];
    GetSegment(w_min, h_max, ul_start, ul_end);

    float br_start[4];
    float br_end[4];
    GetSegment(w_max, h_min, br_start, br_end);

    float ur_start[4];
    float ur_end[4];
    GetSegment(w_max, h_max, ur_start, ur_end);

    //
    // Now use those segments to construct bounding planes.  If the grid is
    // not on the plus side of the bounding planes, then none of the frustum
    // will intersect the grid.
    //
    // Note: the plus side of the plane is dependent on the order that these
    // points are sent into the routine "FindPlaneNormal".  There are some
    // subtleties with putting the arguments in the right order.
    //
    float normal[3];
    FindPlaneNormal(bl_start, bl_end, ul_start, normal);
    if (!GridOnPlusSideOfPlane(bl_start, normal))
        return false;
    FindPlaneNormal(bl_start, br_start, br_end, normal);
    if (!GridOnPlusSideOfPlane(bl_start, normal))
        return false;
    FindPlaneNormal(ur_start, ul_start, ur_end, normal);
    if (!GridOnPlusSideOfPlane(ur_start, normal))
        return false;
    FindPlaneNormal(ur_start, ur_end, br_start, normal);
    if (!GridOnPlusSideOfPlane(ur_start, normal))
        return false;

    return true;
}


// ****************************************************************************
//  Method: avtMassVoxelExtractor::FindPlaneNormal
//
//  Purpose:
//      Finds the normal to a plane using three points.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2004
//
// ****************************************************************************

void
avtMassVoxelExtractor::FindPlaneNormal(const float *pt1, const float *pt2,
                                       const float *pt3, float *normal)
{
    //
    // Set up vectors P1P2 and P1P3.
    //
    float v1[3];
    float v2[3];

    v1[0] = pt2[0] - pt1[0];
    v1[1] = pt2[1] - pt1[1];
    v1[2] = pt2[2] - pt1[2];
    v2[0] = pt3[0] - pt1[0];
    v2[1] = pt3[1] - pt1[1];
    v2[2] = pt3[2] - pt1[2];

    //
    // The normal is the cross-product of these two vectors.
    //
    normal[0] = v1[1]*v2[2] - v1[2]*v2[1];
    normal[1] = v1[2]*v2[0] - v1[0]*v2[2];
    normal[2] = v1[0]*v2[1] - v1[1]*v2[0];
}


// ****************************************************************************
//  Method: avtMassVoxelExtractor::GridOnPlusSideOfPlane
//
//  Purpose:
//      Determines if a grid is on the plus side of a plane.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2004
//
// ****************************************************************************

bool
avtMassVoxelExtractor::GridOnPlusSideOfPlane(const float *origin, 
                                             const float *normal) const
{
    float x_min = X[0];
    float x_max = X[dims[0]-1];
    float y_min = Y[0];
    float y_max = Y[dims[1]-1];
    float z_min = Z[0];
    float z_max = Z[dims[2]-1];

    for (int i = 0 ; i < 8 ; i++)
    {
        float pt[3];
        pt[0] = (i & 1 ? x_max : x_min);
        pt[1] = (i & 2 ? y_max : y_min);
        pt[2] = (i & 4 ? z_max : z_min);

        //
        // The plane is of the form Ax + By + Cz - D = 0.
        //
        // Using the origin, we can calculate D:
        // D = A*origin[0] + B*origin[1] + C*origin[2]
        //
        // We want to know if 'pt' gives:
        // A*pt[0] + B*pt[1] + C*pt[2] - D >=? 0.
        //
        // We can substitute in D to get
        // A*(pt[0]-origin[0]) + B*(pt[1]-origin[1]) + C*(pt[2-origin[2]) ?>= 0
        //
        float val  = normal[0]*(pt[0] - origin[0])
                   + normal[1]*(pt[1] - origin[1])
                   + normal[2]*(pt[2] - origin[2]);
        if (val >= 0)
            return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtMassVoxelExtractor::FindSegmentIntersections
//
//  Purpose:
//      Finds the intersection points of a line segment and a rectilinear grid.
//  
//  Programmer: Hank Childs
//  Creation:   November 21, 2004
//
// ****************************************************************************

bool
avtMassVoxelExtractor::FindSegmentIntersections(const float *origin, 
                                  const float *terminus, int &start, int &end)
{
    float  t, x, y, z;

    int num_hits = 0;
    float hits[6]; // Should always be 2 or 0.

    float x_min = X[0];
    float x_max = X[dims[0]-1];
    float y_min = Y[0];
    float y_max = Y[dims[1]-1];
    float z_min = Z[0];
    float z_max = Z[dims[2]-1];

    //
    // If the terminus and the origin have the same X, then we will find
    // the intersection at another face.
    //
    if (terminus[0] != origin[0])
    {
        //
        // See if we hit the X-min face.
        //
        t = (x_min - origin[0]) / (terminus[0] - origin[0]);
        y = origin[1] + t*(terminus[1] - origin[1]);
        z = origin[2] + t*(terminus[2] - origin[2]);
        if (y_min <= y && y <= y_max && z_min <= z && z <= z_max)
        {
            hits[num_hits++] = t;
        }

        //
        // See if we hit the X-max face.
        //
        t = (x_max - origin[0]) / (terminus[0] - origin[0]);
        y = origin[1] + t*(terminus[1] - origin[1]);
        z = origin[2] + t*(terminus[2] - origin[2]);
        if (y_min <= y && y <= y_max && z_min <= z && z <= z_max)
        {
            hits[num_hits++] = t;
        }
    }

    //
    // If the terminus and the origin have the same Y, then we will find
    // the intersection at another face.
    //
    if (terminus[1] != origin[1])
    {
        //
        // See if we hit the Y-min face.
        //
        t = (y_min - origin[1]) / (terminus[1] - origin[1]);
        x = origin[0] + t*(terminus[0] - origin[0]);
        z = origin[2] + t*(terminus[2] - origin[2]);
        if (x_min <= x && x <= x_max && z_min <= z && z <= z_max)
        {
            hits[num_hits++] = t;
        }

        //
        // See if we hit the Y-max face.
        //
        t = (y_max - origin[1]) / (terminus[1] - origin[1]);
        x = origin[0] + t*(terminus[0] - origin[0]);
        z = origin[2] + t*(terminus[2] - origin[2]);
        if (x_min <= x && x <= x_max && z_min <= z && z <= z_max)
        {
            hits[num_hits++] = t;
        }
    }

    //
    // If the terminus and the origin have the same Z, then we will find
    // the intersection at another face.
    //
    if (terminus[2] != origin[2])
    {
        //
        // See if we hit the Z-min face.
        //
        t = (z_min - origin[2]) / (terminus[2] - origin[2]);
        x = origin[0] + t*(terminus[0] - origin[0]);
        y = origin[1] + t*(terminus[1] - origin[1]);
        if (x_min <= x && x <= x_max && y_min <= y && y <= y_max)
        {
            hits[num_hits++] = t;
        }

        //
        // See if we hit the Z-max face.
        //
        t = (z_max - origin[2]) / (terminus[2] - origin[2]);
        x = origin[0] + t*(terminus[0] - origin[0]);
        y = origin[1] + t*(terminus[1] - origin[1]);
        if (x_min <= x && x <= x_max && y_min <= y && y <= y_max)
        {
            hits[num_hits++] = t;
        }
    }

    if (num_hits == 0)
        return false;
    if (num_hits == 1) // hit one corner of dataset.
        return false;

    //
    // We are expecting exactly two hits.  If we don't get that, then
    // we probably hit an edge of the dataset.  Give up on this optimization.
    //
    if (num_hits != 2)
    {
        start = 0;
        end = depth-1;
        return true;
    }

    if (hits[0] > hits[1])
    {
        float t = hits[0];
        hits[0] = hits[1];
        hits[1] = t;
    }

    if (hits[0] < 0 && hits[1] < 0)
        // Dataset on back side of camera -- no intersection.
        return false;
    if (hits[0] > 1. && hits[1] > 1.)
        // Dataset past far clipping plane -- no intersection.
        return false;

    if (view.orthographic)
    {
        start = (int) floor(depth*hits[0]);
        end   = (int) ceil(depth*hits[1]);
    }
    else
    {
        //
        // The values here are proportions along a line segment.  That is not
        // what we want.  What we really want is depth in Z.  So convert our
        // proportions to a depth value.
        //
        // So the projection matrix for angle A, near plane n, and far plane f
        // is:
        // cot (A/2)   0      0      0
        //     0   cot(A/2)   0      0
        //     0       0   f+n/f-n   -1
        //     0       0   2fn/f-n   0
        // 
        // So we can map our proportion to a value of Z (in world space) and
        // then apply the projection matrix to get its mapping into Z-buffer
        // space. 
        //
        // If we are proportion t along a segment, then our mapping into Z is
        // Z=-(near + t*(far - near)).
        // 
        // Then multiplying (X, Y, Z, 1)
        // gives:
        //   (-,-,Z*f+n/f-n + 2fn/f-n, -Z)
        //
        // Normalizing by W gives:
        // Z-buffer = Z*f+n/f-n + 2fn/f-n.
        //
        float f = cur_clip_range[1];
        float n = cur_clip_range[0];
        float zs = -(n +hits[0]*(f-n));
        float ze = -(n +hits[1]*(f-n));
        float term1 = (f+n)/(f-n);
        float term2 = (2*f*n)/(f-n);
        float s1 = (zs*term1 + term2) / -zs;
        float e1 = (ze*term1 + term2) / -ze;
    
        //
        // This will the near plane to 1 and the far plane to -1.  We actually 
        // want the near plane to be at 0 and the far plane at 1, so do an 
        // additional mapping.
        //
        float s = 1. - (s1+1)/2.;
        float e = 1. - (e1+1)/2.;

        //
        // Now convert to integers so that we can do our sampling at regular
        // intervals.
        //
        start = (int) floor(s*depth);
        end = (int) ceil(e*depth);
    }

    if (start < 0)
        start = 0;
    if (end > depth)
        end = depth;

    return true;
}


// ****************************************************************************
//  Method: avtMassVoxelExtractor::SampleVariable
//
//  Purpose:
//      Actually samples the variable into our temporaray structure.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2004
//
// ****************************************************************************

void
avtMassVoxelExtractor::SampleVariable(int first, int last, int w, int h)
{
    bool inrun = false;
    int  count = 0;
    avtRay *ray = volume->GetRay(w, h);
    bool calc_cell_index = ((ncell_arrays > 0) || (ghosts != NULL));
    for (int i = first ; i < last ; i++)
    {
        const int *ind = ind_buffer + 3*i;
        const float *prop = prop_buffer + 3*i;

        int index = 0;
        if (calc_cell_index)
            index = ind[2]*((dims[0]-1)*(dims[1]-1)) + ind[1]*(dims[0]-1) +
                    ind[0];
  
        if (ghosts != NULL)
        {
            if (ghosts[index] != 0)
                valid_sample[i] = false;
        }

        if (!valid_sample[i] && inrun)
        {
            ray->SetSamples(i-count, i-1, tmpSampleList);
            inrun = false;
            count = 0;
        }
        if (!valid_sample[i])
            continue;

        int  l;

        int var_index = 0;
        for (l = 0 ; l < ncell_arrays ; l++)
        {
            tmpSampleList[count][var_index++] = cell_arrays[l][index];
        }
        if (npt_arrays > 0)
        {
            int index0 = (ind[2])*dims[0]*dims[1] +(ind[1])*dims[0] + (ind[0]);
            int index1 = (ind[2])*dims[0]*dims[1] + 
                                                 (ind[1])*dims[0] + (ind[0]+1);
            int index2 = (ind[2])*dims[0]*dims[1] + 
                                                 (ind[1]+1)*dims[0] + (ind[0]);
            int index3 = (ind[2])*dims[0]*dims[1] + 
                                                (ind[1]+1)*dims[0]+ (ind[0]+1);
            int index4 = (ind[2]+1)*dims[0]*dims[1] + 
                                                   (ind[1])*dims[0] + (ind[0]);
            int index5 = (ind[2]+1)*dims[0]*dims[1] + 
                                                  (ind[1])*dims[0]+ (ind[0]+1);
            int index6 = (ind[2]+1)*dims[0]*dims[1] + 
                                                  (ind[1]+1)*dims[0]+ (ind[0]);
            int index7 = (ind[2]+1)*dims[0]*dims[1] +
                                               (ind[1]+1)*dims[0] + (ind[0]+1);
            float x_right = prop[0];
            float x_left = 1. - prop[0];
            float y_top = prop[1];
            float y_bottom = 1. - prop[1];
            float z_back = prop[2];
            float z_front = 1. - prop[2];
            for (l = 0 ; l < npt_arrays ; l++)
            {
                float *pt_array = pt_arrays[l];
                float val = 
                      x_left*y_bottom*z_front*pt_array[index0] +
                      x_right*y_bottom*z_front*pt_array[index1] +
                      x_left*y_top*z_front*pt_array[index2] +
                      x_right*y_top*z_front*pt_array[index3] +
                      x_left*y_bottom*z_back*pt_array[index4] +
                      x_right*y_bottom*z_back*pt_array[index5] +
                      x_left*y_top*z_back*pt_array[index6] +
                      x_right*y_top*z_back*pt_array[index7];
                tmpSampleList[count][var_index++] = val;
            }    
        }

        inrun = true;
        count++;
    }
        
    //
    // Make sure we get runs at the end.
    //
    if (inrun)
        ray->SetSamples(last-count, last-1, tmpSampleList);
}


// ****************************************************************************
//  Function: FindMatch
//
//  Purpose:
//      Traverses an ordered array in logarithmic time.
//
//  Programmer: Hank Childs
//  Creation:   November 22, 2004
//
// ****************************************************************************

static inline int FindMatch(const float *A, const float &a, const int &nA)
{
    if ((a < A[0]) || (a > A[nA-1]))
        return -1;

    int low = 0;
    int hi  = nA-1;
    while ((hi - low) > 1)
    {
        int guess = (hi+low)/2;
        if (A[guess] == a)
            return guess;
        if (a < A[guess])
            hi = guess;
        else
            low = guess;
    }


    return low;
}


// ****************************************************************************
//  Method: avtMassVoxelExtractor::SampleAlongSegment
//
//  Purpose:
//      Samples the grid along a line segment.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2004
//
//  Modifications:
//
//    Hank Childs, Tue Jan  3 17:26:11 PST 2006
//    Fix bug that ultimately led to UMR where sampling occurred along 
//    invalid values.
//
// ****************************************************************************

void
avtMassVoxelExtractor::SampleAlongSegment(const float *origin, 
                                          const float *terminus, int w, int h)
{
    int first = 0;
    int last = 0;
    bool hasIntersections = FindSegmentIntersections(origin, terminus,
                                                     first, last);
    if (!hasIntersections)
        return;

    bool foundHit = false;
    int curX = -1;
    int curY = -1;
    int curZ = -1;
    bool xGoingUp = (terminus[0] > origin[0]);
    bool yGoingUp = (terminus[1] > origin[1]);
    bool zGoingUp = (terminus[2] > origin[2]);

    float x_dist = (terminus[0]-origin[0]);
    float y_dist = (terminus[1]-origin[1]);
    float z_dist = (terminus[2]-origin[2]);

    float pt[3];
    bool hasSamples = false;
 
    for (int i = first ; i < last ; i++)
    {
        int *ind = ind_buffer + 3*i;
        float *dProp = prop_buffer + 3*i;
        valid_sample[i] = false;

        pt[0] = origin[0] + ProportionSpaceToZBufferSpace[i]*x_dist;
        pt[1] = origin[1] + ProportionSpaceToZBufferSpace[i]*y_dist;
        pt[2] = origin[2] + ProportionSpaceToZBufferSpace[i]*z_dist;

        ind[0] = -1;
        ind[1] = -1;
        ind[2] = -1;

        if (!foundHit)
        {
            //
            // We haven't found any hits previously.  Exhaustively search
            // through arrays and try to find a hit.
            //
            ind[0] = FindMatch(X, pt[0], dims[0]);
            if (ind[0] >= 0)
                dProp[0] = (pt[0] - X[ind[0]]) * divisors_X[ind[0]];
            ind[1] = FindMatch(Y, pt[1], dims[1]);
            if (ind[1] >= 0)
                dProp[1] = (pt[1] - Y[ind[1]]) * divisors_Y[ind[1]];
            ind[2] = FindMatch(Z, pt[2], dims[2]);
            if (ind[2] >= 0)
                dProp[2] = (pt[2] - Z[ind[2]]) * divisors_Z[ind[2]];
        }
        else
        {
            //
            // We have found a hit before.  Try to locate the next sample 
            // based on what we already found.
            //
            if (xGoingUp)
            {
                for ( ; curX < dims[0]-1 ; curX++)
                {
                    if (pt[0] >= X[curX] && pt[0] <= X[curX+1])
                    {
                        dProp[0] = (pt[0] - X[curX]) * divisors_X[curX];
                        ind[0] = curX;
                        break;
                    }
                }
            }
            else
            {
                for ( ; curX >= 0 ; curX--)
                {
                    if (pt[0] >= X[curX] && pt[0] <= X[curX+1])
                    {
                        dProp[0] = (pt[0] - X[curX]) * divisors_X[curX];
                        ind[0] = curX;
                        break;
                    }
                }
            }
            if (yGoingUp)
            {
                for ( ; curY < dims[1]-1 ; curY++)
                {
                    if (pt[1] >= Y[curY] && pt[1] <= Y[curY+1])
                    {
                        dProp[1] = (pt[1] - Y[curY]) * divisors_Y[curY];
                        ind[1] = curY;
                        break;
                    }
                }
            }
            else
            {
                for ( ; curY >= 0 ; curY--)
                {
                    if (pt[1] >= Y[curY] && pt[1] <= Y[curY+1])
                    {
                        dProp[1] = (pt[1] - Y[curY]) * divisors_Y[curY];
                        ind[1] = curY;
                        break;
                    }
                }
            }
            if (zGoingUp)
            {
                for ( ; curZ < dims[2]-1 ; curZ++)
                {
                    if (pt[2] >= Z[curZ] && pt[2] <= Z[curZ+1])
                    {
                        dProp[2] = (pt[2] - Z[curZ]) * divisors_Z[curZ];
                        ind[2] = curZ;
                        break;
                    }
                }
            }
            else
            {
                for ( ; curZ >= 0 ; curZ--)
                {
                    if (pt[2] >= Z[curZ] && pt[2] <= Z[curZ+1])
                    {
                        dProp[2] = (pt[2] - Z[curZ]) * divisors_Z[curZ];
                        ind[2] = curZ;
                        break;
                    }
                }
            }
        }

        bool intersectedDataset = !(ind[0] < 0 || ind[1] < 0 || ind[2] < 0);
        if (!intersectedDataset)
        {
            if (!foundHit) 
            {
                // We still haven't found the start.  Keep looking.
                continue;
            }
            else
            {
                // This is the true terminus.
                last = i;
                break;
            }
        }
        else  // Did intersect data set.
        {
            if (!foundHit)
            {
                // This is the first true sample.  "The true start"
                first = i;
            }
        }

        valid_sample[i] = true;
        foundHit = true;
        hasSamples = true;

        curX = ind[0];
        curY = ind[1];
        curZ = ind[2];
    }

    if (hasSamples)
        SampleVariable(first, last, w, h);
}


// ****************************************************************************
//  Method: avtMassVoxelExtractor::FindIndex
//
//  Purpose:
//      Finds the index that corresponds to a point.
//
//  Programmer:   Hank Childs
//  Creation:     December 14, 2003
//
// ****************************************************************************

inline int FindIndex(const float &pt, const int &last_hit, const int &n,
                     const float *vals)
{
    int i;

    for (i = last_hit ; i < n-1 ; i++)
    {
        if (pt >= vals[i] && (pt <= vals[i+1]))
            return i;
    }

    for (i = 0 ; i < last_hit ; i++)
    {
        if (pt >= vals[i] && (pt <= vals[i+1]))
            return i;
    }

    return -1;
}


// ****************************************************************************
//  Method: avtMassVoxelExtractor::ExtractImageSpaceGrid
//
//  Purpose:
//      Extracts a grid that has already been put into image space.  This case
//      typically corresponds to resampling.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Aug 27 16:00:57 PDT 2004
//    Rename ghost data array.
//
//    Hank Childs, Fri Nov 19 14:50:58 PST 2004
//    Renamed from Extract.
//
//    Hank Childs, Mon Jul 11 14:01:28 PDT 2005
//    Fix indexing issue with ghost zones ['5712].
//
// ****************************************************************************

void
avtMassVoxelExtractor::ExtractImageSpaceGrid(vtkRectilinearGrid *rgrid)
{
    int  i, j, k, l;

    int dims[3];
    rgrid->GetDimensions(dims);
    const int nX = dims[0];
    const int nY = dims[1];
    const int nZ = dims[2];

    float *x = (float *) rgrid->GetXCoordinates()->GetVoidPointer(0);
    float *y = (float *) rgrid->GetYCoordinates()->GetVoidPointer(0);
    float *z = (float *) rgrid->GetZCoordinates()->GetVoidPointer(0);

    int last_x_hit = 0;
    int last_y_hit = 0;
    int last_z_hit = 0;

    vtkUnsignedCharArray *ghosts = (vtkUnsignedCharArray *)rgrid->GetCellData()
                                                   ->GetArray("avtGhostZones");
    std::vector<float *> cell_arrays;
    for (i = 0 ; i < rgrid->GetCellData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = rgrid->GetCellData()->GetArray(i);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        cell_arrays.push_back((float *) arr->GetVoidPointer(0));
    }
    std::vector<float *> pt_arrays;
    for (i = 0 ; i < rgrid->GetPointData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = rgrid->GetPointData()->GetArray(i);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        pt_arrays.push_back((float *) arr->GetVoidPointer(0));
    }

    int startX = SnapXLeft(x[0]);
    int stopX  = SnapXRight(x[nX-1]);
    int startY = SnapYBottom(y[0]);
    int stopY  = SnapYTop(y[nY-1]);
    int startZ = SnapZFront(z[0]);
    int stopZ  = SnapZBack(z[nZ-1]);
    for (j = startY ; j <= stopY ; j++)
    {
        float yc = YFromIndex(j);
        int yind = FindIndex(yc, last_y_hit, nY, y);
        if (yind == -1)
            continue;
        last_y_hit = yind;

        float y_bottom  = 0.;
        float y_top = 1.;
        if (pt_arrays.size() > 0)
        {
            float y_range = y[yind+1] - y[yind];
            y_bottom = 1. - (yc - y[yind])/y_range;
            y_top = 1. - y_bottom;
        }
        for (i = startX ; i <= stopX ; i++)
        {
            float xc = XFromIndex(i);
            int xind = FindIndex(xc, last_x_hit, nX, x);
            if (xind == -1)
                continue;
            last_x_hit = xind;

            float x_left  = 0.;
            float x_right = 1.;
            if (pt_arrays.size() > 0)
            {
                float x_range = x[xind+1] - x[xind];
                x_left = 1. - (xc - x[xind])/x_range;
                x_right = 1. - x_left;
            }

            last_z_hit = 0;
            int count = 0;
            int firstZ = -1;
            int lastZ  = stopZ;
            for (k = startZ ; k <= stopZ ; k++)
            {
                float zc = ZFromIndex(k);
                int zind = FindIndex(zc, last_z_hit, nZ, z);
                if (zind == -1)
                {
                    if (firstZ == -1)
                        continue;
                    else
                    {
                        lastZ = k-1;
                        break;
                    }
                }
                if ((count == 0) && (firstZ == -1))
                    firstZ = k;
                last_z_hit = zind;

                //
                // Don't sample from ghost zones.
                //
                if (ghosts != NULL)
                {
                    int index = zind*((nX-1)*(nY-1)) + yind*(nX-1) + xind;
                    if (ghosts->GetValue(index) != 0)
                        continue;
                }

                float z_front  = 0.;
                float z_back = 1.;
                if (pt_arrays.size() > 0)
                {
                    float z_range = z[zind+1] - z[zind];
                    z_front = 1. - (zc - z[zind])/z_range;
                    z_back = 1. - z_front;
                }

                int var_index = 0;
                for (l = 0 ; l < cell_arrays.size() ; l++)
                {
                    int index = zind*((nX-1)*(nY-1)) + yind*(nX-1) + xind;
                    tmpSampleList[count][var_index++] = cell_arrays[l][index];
                }
                if (pt_arrays.size() > 0)
                {
                    int index0 = (zind)*nX*nY + (yind)*nX + (xind);
                    int index1 = (zind)*nX*nY + (yind)*nX + (xind+1);
                    int index2 = (zind)*nX*nY + (yind+1)*nX + (xind);
                    int index3 = (zind)*nX*nY + (yind+1)*nX + (xind+1);
                    int index4 = (zind+1)*nX*nY + (yind)*nX + (xind);
                    int index5 = (zind+1)*nX*nY + (yind)*nX + (xind+1);
                    int index6 = (zind+1)*nX*nY + (yind+1)*nX + (xind);
                    int index7 = (zind+1)*nX*nY + (yind+1)*nX + (xind+1);
                    for (l = 0 ; l < pt_arrays.size() ; l++)
                    {
                        float *pt_array = pt_arrays[l];
                        float val = 
                              x_left*y_bottom*z_front*pt_array[index0] +
                              x_right*y_bottom*z_front*pt_array[index1] +
                              x_left*y_top*z_front*pt_array[index2] +
                              x_right*y_top*z_front*pt_array[index3] +
                              x_left*y_bottom*z_back*pt_array[index4] +
                              x_right*y_bottom*z_back*pt_array[index5] +
                              x_left*y_top*z_back*pt_array[index6] +
                              x_right*y_top*z_back*pt_array[index7];
                        tmpSampleList[count][var_index++] = val;
                    }    
                }
                count++;
            }

            if (count > 0)
            {
                avtRay *ray = volume->GetRay(i, j);
                ray->SetSamples(firstZ, lastZ, tmpSampleList);
            }
        }
    }
}


