// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtRayTracerBase.C                                //
// ************************************************************************* //

#include <avtRayTracerBase.h>

#include <visit-config.h>

#include <vtkMatrix4x4.h>

#include <avtDataset.h>
#include <avtExtents.h>
#include <avtImage.h>
#include <avtParallel.h>

#include <DebugStream.h>



// ****************************************************************************
//  Method: avtRayTracerBase constructor
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//    Brad Whitlock, Wed Dec 5 11:19:29 PDT 2001
//    Added gradient backgrounds.
//
//    Hank Childs, Mon Jan 16 11:11:47 PST 2006
//    Added kernel based sampling.
//
//    Pascal Grosset, Fri Sep 20 2013
//    Added ray casting slivr & trilinear interpolation
//
//    Kathleen Biagas, Wed Aug 17, 2022
//    Incorporate ARSanderson's OSPRAY 2.8.0 work for VTK 9: Change view
//    to viewInfo and move some initialization into header.
//
// ****************************************************************************

avtRayTracerBase::avtRayTracerBase()
{
    viewInfo.camera[0] = -5.;
    viewInfo.camera[1] = 10.;
    viewInfo.camera[2] = -15.;
    viewInfo.focus[0]  = 0.;
    viewInfo.focus[1]  = 0.;
    viewInfo.focus[2]  = 0.;
    viewInfo.viewAngle = 70.;
    viewInfo.viewUp[0] = 0.;
    viewInfo.viewUp[1] = 0.;
    viewInfo.viewUp[2] = 1.;
    viewInfo.nearPlane = 5.;
    viewInfo.farPlane  = 30.;
    viewInfo.parallelScale = 10;
    viewInfo.orthographic = true;
}


// ****************************************************************************
//  Method: avtRayTracerBase destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtRayTracerBase::~avtRayTracerBase()
{
    ;
}


// ****************************************************************************
//  Method: avtRayTracerBase::SetBackgroundColor
//
//  Purpose:
//      Allows the background color to be set.
//
//  Arguments:
//      b       The new background color.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2001
//
// ****************************************************************************

void
avtRayTracerBase::SetBackgroundColor(const unsigned char b[3])
{
    background[0] = b[0];
    background[1] = b[1];
    background[2] = b[2];
}


// ****************************************************************************
//  Function: GetNumberOfStages
//
//  Purpose:
//      Determines how many stages the ray tracer will take.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Jun  9 13:47:29 PDT 2006
//    Replace 3 with numPerTile (oversight that it wasn't coded that way
//    originally).
//
// ****************************************************************************

int
avtRayTracerBase::GetNumberOfStages(int screenX, int screenY, int screenZ)
{
    int nD = GetNumberOfDivisions(screenX, screenY, screenZ);
    int numPerTile = 3;
#ifdef PARALLEL
    numPerTile = 5;
#endif
    return numPerTile*nD*nD;
}


// ****************************************************************************
//  Function: GetNumberOfDivisions
//
//  Purpose:
//      Determines how many divisions of screen space we should use.  That is,
//      how many tiles should we use.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2005
//
//  Modifications:
//
//    Hank Childs, Sat Sep 26 20:43:55 CDT 2009
//    If we have more than 32 procs, then we have enough memory and don't need
//    to tile.
//
//    Kathleen Biagas, Wed Nov 18 2020
//    Replace VISIT_LONG_LONG with long long.
//
// ****************************************************************************

int
avtRayTracerBase::GetNumberOfDivisions(int screenX, int screenY, int screenZ)
{
    if (PAR_Size() >= 32)
        return 1;

    long long numSamps = screenX*screenY*screenZ;
    int sampLimitPerProc = 25000000; // 25M
    numSamps /= PAR_Size();
    int numTiles = numSamps/sampLimitPerProc;
    int numDivisions = (int) sqrt((double) numTiles);
    if (numDivisions < 1)
        numDivisions = 1;
    int altNumDiv = (int)(screenX / 700.) + 1;
    if (altNumDiv > numDivisions)
        numDivisions = altNumDiv;
    altNumDiv = (int)(screenY / 700.) + 1;
    if (altNumDiv > numDivisions)
        numDivisions = altNumDiv;

    return numDivisions;
}


// ****************************************************************************
//  Method: avtRayTracerBase::SetView
//
//  Purpose:
//      Sets the viewing matrix of the camera.
//
//  Arguments:
//      v       The view info.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//    Kathleen Biagas, Wed Aug 17, 2022
//    Incorporate ARSanderson's OSPRAY 2.8.0 work for VTK 9: Change view
//    to viewInfo.
//
// ****************************************************************************

void
avtRayTracerBase::SetView(const avtViewInfo &vInfo)
{
    viewInfo = vInfo;
    modified = true;
}


// ****************************************************************************
//  Method: avtRayTracerBase::SetRayFunction
//
//  Purpose:
//      Sets the ray function for the ray tracer.
//
//  Arguments:
//      rf      The new ray function.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 20:37:01 PST 2001
//    Removed pixelizer from its coupling with a ray function.
//
// ****************************************************************************

void
avtRayTracerBase::SetRayFunction(avtRayFunction *rf)
{
    rayfoo    = rf;
}


// ****************************************************************************
//  Method: avtRayTracerBase::SetScreen
//
//  Purpose:
//      Tells the ray tracer the size of the screen and thus how many rays
//      to send out.
//
//  Arguments:
//      screenX       The number of pixels in width.
//      screenY       The number of pixels in height.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2000
//
// ****************************************************************************

void
avtRayTracerBase::SetScreen(int screenX, int screenY)
{
    if (screenX > 0 && screenY > 0)
    {
        screen[0] = screenX;
        screen[1] = screenY;
        modified  = true;
    }
}


// ****************************************************************************
//  Method: avtRayTracerBase::SetSamplesPerRay
//
//  Purpose:
//      Sets the number of samples to be taken along each ray.
//
//  Arguments:
//      samps   The number of samples that should be taken along each ray.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2000
//
// ****************************************************************************

void
avtRayTracerBase::SetSamplesPerRay(int samps)
{
    if (samps > 0)
    {
        samplesPerRay = samps;
        modified      = true;
    }
}


// ****************************************************************************
//  Method: avtRayTracerBase::InsertOpaqueImage
//
//  Purpose:
//      Allows for an opaque image to be inserted into the middle of the
//      rendering.
//
//  Arguments:
//      img     The image to be used in the ray tracer.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2001
//
// ****************************************************************************

void
avtRayTracerBase::InsertOpaqueImage(avtImage_p img)
{
    opaqueImage = img;
}


// ****************************************************************************
//  Method: avtRayTracerBase::ReleaseData
//
//  Purpose:
//      Releases the problem size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 16, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Sep 24 08:25:28 PDT 2002
//    I forgot the call to the base class's release data.
//
// ****************************************************************************

void
avtRayTracerBase::ReleaseData(void)
{
    avtDatasetToImageFilter::ReleaseData();
    if (*opaqueImage != NULL)
    {
        opaqueImage->ReleaseData();
    }
}


// ****************************************************************************
//  Method: avtRayTracerBase::ModifyContract
//
//  Purpose:
//      Restricts the data of interest.  Does this by getting the spatial
//      extents and culling around the viewInfo.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jan  8 13:59:45 PST 2001
//    Put in check to make sure the spatial extents still apply before trying
//    use them for culling.
//
//    Hank Childs, Sun Mar 25 11:44:12 PST 2001
//    Account for new interface with data object information.
//
//    Hank Childs, Wed Nov 21 11:53:21 PST 2001
//    Don't assume we can get an interval tree.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Hank Childs, Thu May 29 09:44:17 PDT 2008
//    No longer remove domains that cannot contribute to final picture,
//    because that decision is made here one time for many renders.  If you
//    choose one set, it may change later for a different render.
//
// ****************************************************************************

avtContract_p
avtRayTracerBase::ModifyContract(avtContract_p spec)
{
    avtContract_p rv = new avtContract(spec);
    rv->NoStreaming();
    return rv;
}


// ****************************************************************************
//  Method:  avtRayTracerBase::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************
bool
avtRayTracerBase::FilterUnderstandsTransformedRectMesh()
{
    // There's nothing special about the raytracer that might
    // not understand these meshes.  At a lower level, filters
    // like the sample point extractor report this correctly.
    return true;
}


// ****************************************************************************
//   Method: avtRayTracerBase::TightenClippingPlanes
//
//   Purpose:
//       Tightens the clipping planes, so that more samples fall within
//       the view frustum.
//
//   Notes:      This code was originally in
//               avtWorldSpaceToImageSpaceTransform::PreExecute.
//
//   Programmer: Hank Childs
//   Creation:   December 24, 2008
//
//   Modifications:
//
//     Hank Childs, Sat Sep 26 20:43:55 CDT 2009
//     Fixed bug for tightening planes when the camera is inside the volume.
//
//     Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//     Change extents names.
//
//     Kathleen Biagas, Wed Aug 17, 2022
//     Incorporate ARSanderson's OSPRAY 2.8.0 work for VTK 9: Change view
//     to viewInfo.
//
// ****************************************************************************

void
avtRayTracerBase::TightenClippingPlanes(const avtViewInfo &viewInfo,
                                    vtkMatrix4x4 *transform,
                                    double &newNearPlane, double &newFarPlane)
{
    newNearPlane = viewInfo.nearPlane;
    newFarPlane  = viewInfo.farPlane;

    double dbounds[6];
    avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
    avtExtents *exts = datts.GetDesiredSpatialExtents();
    if (exts->HasExtents())
    {
        exts->CopyTo(dbounds);
    }
    else
    {
        GetSpatialExtents(dbounds);
    }

    double vecFromCameraToPlaneX = viewInfo.focus[0] - viewInfo.camera[0];
    double vecFromCameraToPlaneY = viewInfo.focus[1] - viewInfo.camera[1];
    double vecFromCameraToPlaneZ = viewInfo.focus[2] - viewInfo.camera[2];
    double vecMag = (vecFromCameraToPlaneX*vecFromCameraToPlaneX)
                  + (vecFromCameraToPlaneY*vecFromCameraToPlaneY)
                  + (vecFromCameraToPlaneZ*vecFromCameraToPlaneZ);
    vecMag = sqrt(vecMag);

    double farthest = 0.;
    double nearest  = 0.;
    for (int i = 0 ; i < 8 ; i++)
    {
        double X = (i & 1 ? dbounds[1] : dbounds[0]);
        double Y = (i & 2 ? dbounds[3] : dbounds[2]);
        double Z = (i & 4 ? dbounds[5] : dbounds[4]);

        //
        // We are now constructing the dot product of our two vectors.  Note
        // That this will give us cosine of their angles times the magnitude
        // of the camera-to-plane vector times the magnitude of the
        // camera-to-farthest vector.  We want the magnitude of a new vector,
        // the camera-to-closest-point-on-plane-vector.  That vector will
        // lie along the same vector as the camera-to-plane and it forms
        // a triangle with the camera-to-farthest-vector.  Then we have the
        // same angle between them and we can re-use the cosine we calculate.
        //
        double vecFromCameraToX = X - viewInfo.camera[0];
        double vecFromCameraToY = Y - viewInfo.camera[1];
        double vecFromCameraToZ = Z - viewInfo.camera[2];

        //
        // dot = cos X * mag(A) * mag(B)
        // We know cos X = mag(C) / mag(A)   C = adjacent, A = hyp.
        // Then mag(C) = cos X * mag(A).
        // So mag(C) = dot / mag(B).
        //
        double dot = vecFromCameraToPlaneX*vecFromCameraToX
                   + vecFromCameraToPlaneY*vecFromCameraToY
                   + vecFromCameraToPlaneZ*vecFromCameraToZ;

        double dist = dot / vecMag;
        double newNearest  = dist - (viewInfo.farPlane-dist)*0.01; // fudge
        double newFarthest = dist + (dist-viewInfo.nearPlane)*0.01; // fudge
        if (i == 0)
        {
            farthest = newFarthest;
            nearest  = newNearest;
        }
        else
        {
            if (newNearest < nearest)
                nearest  = newNearest;
            if (newFarthest > farthest)
                farthest = newFarthest;
        }
    }

    if (nearest > viewInfo.nearPlane)
        newNearPlane = nearest;

    if (farthest < viewInfo.farPlane)
        newFarPlane = farthest;
}

