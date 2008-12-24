/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                     avtWorldSpaceToImageSpaceTransform.C                  //
// ************************************************************************* //

#include <avtWorldSpaceToImageSpaceTransform.h>

#include <math.h>

#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkPointSet.h>
#include <vtkRectilinearGrid.h>

#include <avtDataset.h>
#include <avtExtents.h>
#include <avtIntervalTree.h>
#include <avtMetaData.h>

#include <DebugStream.h>


using     std::vector;


bool   HexIntersectsImageCube(const float [8][3]);


// ****************************************************************************
//  Method: avtWorldSpaceToImageSpaceTransform constructor
//
//  Arguments:
//      vi      The view information for this transform.
//      asp     The aspect ratio of the window (width/height).
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Mar 30 17:31:30 PST 2001
//    Accounted for potential of scaling.
//
//    Hank Childs, Mon Nov 26 18:33:16 PST 2001
//    Made use of aspect ratio.
//
//    Hank Childs, Fri Nov 19 13:41:56 PST 2004
//    Initialize passThruRectilinear.
//
// ****************************************************************************

avtWorldSpaceToImageSpaceTransform::avtWorldSpaceToImageSpaceTransform(
                                             const avtViewInfo &vi, double asp)
{
    scale[0] = 1.;
    scale[1] = 1.;
    scale[2] = 1.;
    aspect   = asp;

    tightenClippingPlanes = false;
    passThruRectilinear = false;

    view = vi;
    transform = vtkMatrix4x4::New();
    CalculateTransform(view, transform, scale, aspect);
}


// ****************************************************************************
//  Method: avtWorldSpaceToImageSpaceTransform constructor
//
//  Arguments:
//      vi      The view information for this transform.
//      s       The scale to transform the image by.
//
//  Programmer: Hank Childs
//  Creation:   March 30, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Nov 26 18:33:16 PST 2001
//    Initialized aspect ratio.
//
//    Hank Childs, Fri Nov 19 13:41:56 PST 2004
//    Initialize passThruRectilinear.
//
// ****************************************************************************

avtWorldSpaceToImageSpaceTransform::avtWorldSpaceToImageSpaceTransform(
                                        const avtViewInfo &vi, const double *s)
{
    scale[0] = s[0];
    scale[1] = s[1]; 
    scale[2] = s[2];
    aspect   = 1.;

    tightenClippingPlanes = false;
    passThruRectilinear = false;

    view = vi;
    transform = vtkMatrix4x4::New();
    CalculateTransform(view, transform, scale, aspect);
}


// ****************************************************************************
//  Method: avtWorldSpaceToImageSpaceTransform destructor
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
// ****************************************************************************

avtWorldSpaceToImageSpaceTransform::~avtWorldSpaceToImageSpaceTransform()
{
    if (transform != NULL)
    {
        transform->Delete();
        transform = NULL;
    }
}


// ****************************************************************************
//  Method: avtWorldSpaceToImageSpaceTransform::CalculateTransform
//
//  Purpose:
//      Calculates the transform appropriate for the view.
//
//  Arguments:
//      view         The view information
//      trans        A place to put the 4 by 4 matrix associated with this
//                   transform.
//      scale        Scale the matrix after calculating the world to image 
//                   transform.
//      asp     The aspect ratio of the window (width/height).
//
//  Programmer: Hank Childs
//  Creation:   December 15, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Mar 30 17:03:39 PST 2001
//    Add scaling after the fact.
//
//    Hank Childs, Sat Aug  4 09:01:09 PDT 2001
//    Account for world space being a right handed coordinate system and
//    image space being a left handed coordinate system.
//
//    Hank Childs, Mon Nov 26 18:33:16 PST 2001
//    Made use of aspect ratio.
//
//    Hank Childs, Mon Jul  7 22:32:37 PDT 2003
//    Add support for image pan and zoom.
//
//    Hank Childs, Mon Dec 20 11:19:49 PST 2004
//    No longer reflect across the X-axis.  It was a mistake to do this in
//    the first place.
//
// ****************************************************************************

void
avtWorldSpaceToImageSpaceTransform::CalculateTransform(const avtViewInfo &view,
                       vtkMatrix4x4 *trans, const double *scale, double aspect)
{
    vtkMatrix4x4 *viewtrans = vtkMatrix4x4::New();
    if (view.orthographic)
    {
        CalculateOrthographicTransform(view, viewtrans);
    }
    else
    {
        CalculatePerspectiveTransform(view, viewtrans);
    }

    //
    // Scale in case people don't want to worry about VTK's scaling.
    // Also work in an aspect correction.  The aspect correction is to
    // to multiple the aspect ratio (width/height) into the X-scale.  This
    // mirrors how VTK does it.
    //
    vtkMatrix4x4 *scaletrans = vtkMatrix4x4::New();
    scaletrans->Identity();
    scaletrans->SetElement(0, 0, scale[0]/aspect);
    scaletrans->SetElement(1, 1, scale[1]);
    scaletrans->SetElement(2, 2, scale[2]);

    //
    // Now take in the zoom and pan portions.  These are both image space
    // operations.
    //
    vtkMatrix4x4 *imageZoomAndPan = vtkMatrix4x4::New();
    imageZoomAndPan->Identity();
    imageZoomAndPan->SetElement(0, 0, view.imageZoom);
    imageZoomAndPan->SetElement(1, 1, view.imageZoom);
    imageZoomAndPan->SetElement(0, 3, 2*view.imagePan[0]*view.imageZoom);
    imageZoomAndPan->SetElement(1, 3, 2*view.imagePan[1]*view.imageZoom);

    //
    // Right multiplying the matrices in the order you want them to applied
    // "makes sense" to me, so I am going to jump through hoops by Transposing
    // them so I am sure everything will work.
    //
    vtkMatrix4x4 *tmp = vtkMatrix4x4::New();
    viewtrans->Transpose();
    imageZoomAndPan->Transpose();
    vtkMatrix4x4::Multiply4x4(viewtrans, scaletrans, tmp);
    vtkMatrix4x4::Multiply4x4(tmp, imageZoomAndPan, trans);
    trans->Transpose();

    viewtrans->Delete();
    scaletrans->Delete();
    imageZoomAndPan->Delete();
    tmp->Delete();
}


// ****************************************************************************
//  Method: avtWorldSpaceToImageSpaceTransform::GetTransform
//
//  Purpose:
//      Gets the world space to image space transform.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
// ****************************************************************************

vtkMatrix4x4 *
avtWorldSpaceToImageSpaceTransform::GetTransform(void)
{
    return transform;
}


// ****************************************************************************
//  Method: avtWorldSpaceToImageSpaceTransform::CalculatePerspectiveTransform
//
//  Purpose:
//      Gets the transform from world space to image space when the view
//      transform is a perspective transform.
//
//  Arguments:
//      view    The view information.
//      trans   The matrix where the transformation should be placed.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Jan 26 16:17:35 PST 2001
//    Set vtkCamera using view info convenience method.
//
//    Hank Childs, Fri Feb  8 18:21:35 PST 2002
//    Converted the image cube into a digestible format that is like that of
//    a zbuffer.  This is because that format is expected later in the code.
//
//    Hank Childs, Wed Dec 24 09:48:28 PST 2008
//    Change the matrix multiplication, as we now sample in the wbuffer.
//
// ****************************************************************************

void
avtWorldSpaceToImageSpaceTransform::CalculatePerspectiveTransform(
                                  const avtViewInfo &view, vtkMatrix4x4 *trans)
{
    vtkCamera *camera = vtkCamera::New();
    view.SetCameraFromView(camera);

    //
    // VTK right multiplies points, but I think about things in a left-multiply
    // setting, so get the camera transform and transpose it.
    //
    vtkMatrix4x4 *camtrans = camera->GetViewTransformMatrix();
    camtrans->Transpose();

    //
    // Calculation of the viewing matrix comes from Ken Joy's On-Line
    // Computer Graphics Notes.
    // http://graphics.cs.ucdavis.edu/education/GraphicsNotes
    //         /Viewing-Transformation/Viewing-Transformation.html
    //
    vtkMatrix4x4 *viewtrans = vtkMatrix4x4::New();
    viewtrans->Zero();
    const double pi = 3.14159265358979323846;
    double viewAngleRadians = view.viewAngle * 2. * pi / 360.;
    double cot = 1./tan(viewAngleRadians / 2.);
    viewtrans->SetElement(0, 0, cot);
    viewtrans->SetElement(1, 1, cot);
    viewtrans->SetElement(2, 2, (view.farPlane+view.nearPlane) / (view.farPlane-view.nearPlane));
    viewtrans->SetElement(2, 3, -1.);
    viewtrans->SetElement(3, 2, (2*view.farPlane*view.nearPlane) / (view.farPlane-view.nearPlane));

    vtkMatrix4x4 *imageCubeToZBuffer = vtkMatrix4x4::New();
    imageCubeToZBuffer->Identity();
    if (0) // we no longer do this, because we now go straight to the w-buffer.
    {
         //
         // The transformation we have done so far puts us into the image cube, but
         // we would like to match up with z-buffering, so we would like the closest
         // things to be at z=0 and the furthest to be at z=1.  (the image cube has
         // the front at z=1 and the back at z=-1).
         //
         imageCubeToZBuffer->SetElement(2, 2, -0.5);
         imageCubeToZBuffer->SetElement(3, 2, 0.5);
    }

    //
    // Multiply all of our intermediate matrices together.
    //
    vtkMatrix4x4 *tmp1 = vtkMatrix4x4::New();
    vtkMatrix4x4::Multiply4x4(camtrans, viewtrans, tmp1);
    vtkMatrix4x4::Multiply4x4(tmp1, imageCubeToZBuffer, trans);
    
    //
    // Okay, since we are giving this back to VTK, we have to make it suitable
    // for right multiplication again through a transposition.
    //
    trans->Transpose();

    camera->Delete();
    viewtrans->Delete();
    imageCubeToZBuffer->Delete();
    tmp1->Delete();
}


// ****************************************************************************
//  Method: avtWorldSpaceToImageSpaceTransform::CalculateOrthographicTransform
//
//  Purpose:
//      Gets the transform from world space to image space when the view
//      transform is an orthographic transform.
//
//  Arguments:
//      view    The view information.
//      trans   The matrix where the transformation should be placed.
//
//  Programmer: Hank Childs
//  Creation:   December 12, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Jan 26 16:17:35 PST 2001
//    Set vtkCamera using view info convenience method.
//
//    Hank Childs, Wed Feb 14 10:58:59 PST 2001
//    Have image cube go from z=0 to 1 rather than z=-1 to 1 to correspond with
//    z-buffer output.
//
// ****************************************************************************

void
avtWorldSpaceToImageSpaceTransform::CalculateOrthographicTransform(
                                  const avtViewInfo &view, vtkMatrix4x4 *trans)
{
    vtkCamera *camera = vtkCamera::New();
    view.SetCameraFromView(camera);

    //
    // VTK right multiplies points, but I think about things in a left-multiply
    // setting, so get the camera transform and transpose it.
    //
    vtkMatrix4x4 *camtrans = camera->GetViewTransformMatrix();
    camtrans->Transpose();

    //
    // Start by shifting our cube (-> "frustum" if we had perspective)
    // to the plane of the camera.
    //
    vtkMatrix4x4 *trans_to_plane = vtkMatrix4x4::New();
    trans_to_plane->Identity();
    trans_to_plane->SetElement(3, 2, view.nearPlane);

    //
    // Since we are doing orthographic projection, we can calculate the
    // distance to edge of the no-longer-pinhole-camera by seeing what view
    // angle is and how far it is to the near plane and taking a tangent.
    //
    double  xy_correction = view.parallelScale;
    double  z_correction  = (view.farPlane - view.nearPlane);
    vtkMatrix4x4  *scale1 = vtkMatrix4x4::New();
    scale1->Identity();
    scale1->SetElement(0, 0, 1./xy_correction);
    scale1->SetElement(1, 1, 1./xy_correction);
    scale1->SetElement(2, 2, 1./z_correction);

    //
    // Now put the cube squarely where we want "image space" to be by
    // reflecting it across the z-axis.
    //
    vtkMatrix4x4 *trans_to_imgspc = vtkMatrix4x4::New();
    trans_to_imgspc->Identity();
    trans_to_imgspc->SetElement(2, 2, -1.);

    //
    // Now multiply together to get our final matrix.  We could have calculated
    // this beforehand, but I didn't think it would be as clear.
    //
    vtkMatrix4x4 *temp1 = vtkMatrix4x4::New();
    vtkMatrix4x4 *viewtrans = vtkMatrix4x4::New();
    vtkMatrix4x4::Multiply4x4(trans_to_plane, scale1, temp1);
    vtkMatrix4x4::Multiply4x4(temp1, trans_to_imgspc, viewtrans);
    
    //
    // Okay, since we are giving this back to VTK, we have to make it suitable
    // for right multiplication again through a transposition.
    //
    vtkMatrix4x4::Multiply4x4(camtrans, viewtrans, trans);
    trans->Transpose();

    camera->Delete();
    viewtrans->Delete();
    trans_to_plane->Delete();
    scale1->Delete();
    trans_to_imgspc->Delete();
    temp1->Delete();
}


// ****************************************************************************
//  Method: avtWorldSpaceToImageSpaceTransform::ModifyContract
//
//  Purpose:
//      Calculates the domains list by culling with the domains' spatial
//      extents.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Jan 13 09:52:24 PST 2006
//    Don't use the interval tree if spatial meta-data has been invalidated.
//
//    Hank Childs, Thu May 29 10:15:23 PDT 2008
//    Use the aspect ratio of the screen, as we will cull away domains 
//    incorrectly if we don't.
//
// ****************************************************************************

avtContract_p
avtWorldSpaceToImageSpaceTransform::ModifyContract(avtContract_p spec)
{
    avtContract_p rv = spec;
    if (GetInput()->GetInfo().GetValidity().GetSpatialMetaDataPreserved())
    {
        avtIntervalTree *tree = GetMetaData()->GetSpatialExtents();
        if (tree != NULL)
        {
            vector<int> domains;
            GetDomainsList(view, domains, tree, aspect);
    
            rv = new avtContract(spec);
            rv->GetDataRequest()->GetRestriction()->RestrictDomains(domains);
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: avtWorldSpaceToImageSpaceTransform::GetDomainsList
//
//  Purpose:
//      Takes an interval tree of spatial extents and performs a poor mans
//      culling (ie using extents only) with the view information.
//
//  Arguments:
//      view      The current view.
//      domains   The object to place the domains into.
//      tree      The spatial extents interval tree.
//      aspect    The aspect ratio of the window.
//
//  Programmer:   Hank Childs
//  Creation:     December 15, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Nov 26 18:56:05 PST 2001
//    Account for transform code now taking an aspect ratio.
//
//    Hank Childs, Thu May 29 10:20:13 PDT 2008
//    Add an argument for the aspect ratio, as the wrong domains can be culled
//    out without it.
//
// ****************************************************************************

void
avtWorldSpaceToImageSpaceTransform::GetDomainsList(const avtViewInfo &view,
                          vector<int> &domains, const avtIntervalTree *tree, 
                          double aspect)
{
    //
    // Find the world space to image space transform for this view.
    //
    double scale[3] = { 1., 1., 1. };
    vtkMatrix4x4 *trans = vtkMatrix4x4::New();
    CalculateTransform(view, trans, scale, aspect);

    //
    // Go through each domain in the interval tree and figure out if its
    // bounding box is in the frustum of the view.
    //
    domains.clear();
    int nDomains = tree->GetNLeaves();
    for (int i = 0 ; i < nDomains ; i++)
    {
        double  extents[6];
        int    domain = tree->GetLeafExtents(i, extents);
        float  outpt[8][3];
        for (int j = 0 ; j < 8 ; j++)
        {
            float pt[4];
            pt[0] = ((j&1) == 1 ? extents[1] : extents[0]);
            pt[1] = ((j&2) == 2 ? extents[3] : extents[2]);
            pt[2] = ((j&4) == 4 ? extents[5] : extents[4]);
            pt[3] = 1.;

            float out[4];
            trans->MultiplyPoint(pt, out);
            outpt[j][0] = out[0] / out[3];
            outpt[j][1] = out[1] / out[3];
            outpt[j][2] = out[2] / out[3];
        }
        if (HexIntersectsImageCube(outpt))
        {
            domains.push_back(domain);
        }
    }

    //
    // Clean up memory.
    //
    trans->Delete();
}


// ****************************************************************************
//  Function: HexIntersectsImageCube
//
//  Purpose:
//      Determines if the hex sent in as an argument intersects the image
//      cube [-1, 1], [-1, 1], [-1, 1].
//
//  Arguments:
//      hex     The points to a hexahedron.
//
//  Returns:    true if any part of the hex is in the image cube, false if not.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 2000
//
// ****************************************************************************

bool
HexIntersectsImageCube(const float hex[8][3])
{
    int   i;

    //
    // See if one of the points is inside the cube.
    //
    for (i = 0 ; i < 8 ; i++)
    {
        if (-1. <= hex[i][0] && hex[i][0] <= 1. &&
            -1. <= hex[i][1] && hex[i][1] <= 1. &&
            -1. <= hex[i][2] && hex[i][2] <= 1.)
        {
            return true;
        }
    }

    //
    // See if the hex is entirely to the side of one side of the cube.
    //
    int  xcount = 0, ycount = 0, zcount = 0;
    for (i = 0 ; i < 8 ; i++)
    {
        if (hex[i][0] < -1.)
        {
            xcount--;
        }
        if (hex[i][0] > 1.)
        {
            xcount++;
        }
        if (hex[i][1] < -1.)
        {
            ycount--;
        }
        if (hex[i][1] > 1.)
        {
            ycount++;
        }
        if (hex[i][2] < -1.)
        {
            zcount--;
        }
        if (hex[i][2] > 1.)
        {
            zcount++;
        }
    }
    if (xcount == -8 || xcount == 8 || ycount == -8 || ycount == 8
        || zcount == -8 || zcount == 8)
    {
        return false;
    }

    //
    // This is probably not the best, but I don't have a copy of Graphics
    // Gems in front of me.
    //
    // Just make a sphere around the image cube and the hex and see if they
    // intersect.
    //
    float hex_center[3] = { 0., 0., 0. };
    for (i = 0 ; i < 8 ; i++)
    {
        hex_center[0] += hex[i][0];
        hex_center[1] += hex[i][1];
        hex_center[2] += hex[i][2];
    }
    hex_center[0] /= 8.;
    hex_center[1] /= 8.;
    hex_center[2] /= 8.;

    float max_hex_dist_squared = 0.;
    for (i = 0 ; i < 8 ; i++)
    {
        float d_squared = (hex[i][0]-hex_center[0])*(hex[i][0]-hex_center[0])
                        + (hex[i][1]-hex_center[1])*(hex[i][1]-hex_center[1])
                        + (hex[i][2]-hex_center[2])*(hex[i][2]-hex_center[2]);
        if (d_squared > max_hex_dist_squared)
        {
            max_hex_dist_squared = d_squared;
        }
    }
    float max_hex_dist = sqrt(max_hex_dist_squared);

    float image_cube_radius = sqrt(3.);
    float image_cube_to_hex_dist_squared = (hex_center[0])*(hex_center[0])
                                         + (hex_center[1])*(hex_center[1])
                                         + (hex_center[2])*(hex_center[2]);
    float image_cube_to_hex_dist = sqrt(image_cube_to_hex_dist_squared);
    
    //
    // If the radii of the two sphere is bigger than the distance between the
    // two centers, then the spheres intersect.
    //
    return (image_cube_radius + max_hex_dist >= image_cube_to_hex_dist);
}


// ****************************************************************************
//  Method: avtWorldSpaceToImageSpaceTransform::UpdateDataObjectInfo
//
//  Purpose:
//      Indicate that the spatial meta data is invalid after execution.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

void
avtWorldSpaceToImageSpaceTransform::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();
}


// ****************************************************************************
//  Method: avtWorldSpaceToImageSpaceTransform::PreExecute
//
//  Purpose:
//      This is called right before Execute is called.  This is the first
//      time that we have access to the whole dataset. 
//      See if we should tighten the clipping planes by looking at the
//      bounding box of the dataset (if appropriate).
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2002
//
// ****************************************************************************

void
avtWorldSpaceToImageSpaceTransform::PreExecute(void)
{
    avtTransform::PreExecute();

    if (!tightenClippingPlanes)
    {
        //
        // Nothing for us to do.
        //
        return;
    }

    double dbounds[6];
    avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
    avtExtents *exts = datts.GetEffectiveSpatialExtents();
    if (exts->HasExtents())
    {
        exts->CopyTo(dbounds);
    }
    else
    {
        GetSpatialExtents(dbounds);
    }

    //
    // Multiply our current transform by each of the points in the bounding
    // box, keeping track of the furthest and closest point.
    //
    double nearest     =  1.;
    int    nearestInd  = -1;
    double farthest    =  0.;
    int    farthestInd = -1;
    for (int i = 0 ; i < 8 ; i++)
    {
        double pt[4];
        pt[0] = (i & 1 ? dbounds[1] : dbounds[0]);
        pt[1] = (i & 2 ? dbounds[3] : dbounds[2]);
        pt[2] = (i & 4 ? dbounds[5] : dbounds[4]);
        pt[3] = 1.;
        
        double outpt[4];
        transform->MultiplyPoint(pt, outpt);

        if (outpt[2] > farthest)
        {
            farthestInd = i;
            farthest    = outpt[2];
        }
        if (outpt[2] < nearest)
        {
            nearestInd = i;
            nearest    = outpt[2];
        }
    }

    bool resetNearest = true;
    if (nearestInd == -1 || nearest <= 0.)
    {
        resetNearest = false;
    }

    double vecFromCameraToPlaneX = view.focus[0] - view.camera[0];
    double vecFromCameraToPlaneY = view.focus[1] - view.camera[1];
    double vecFromCameraToPlaneZ = view.focus[2] - view.camera[2];
    double vecMag = (vecFromCameraToPlaneX*vecFromCameraToPlaneX)
                  + (vecFromCameraToPlaneY*vecFromCameraToPlaneY)
                  + (vecFromCameraToPlaneZ*vecFromCameraToPlaneZ);
    vecMag = sqrt(vecMag);

    if (resetNearest)
    {
        double X = (nearestInd & 1 ? dbounds[1] : dbounds[0]);
        double Y = (nearestInd & 2 ? dbounds[3] : dbounds[2]);
        double Z = (nearestInd & 4 ? dbounds[5] : dbounds[4]);

        //
        // My best attempt at explaining what is going on is below is the
        // "farthest" case.
        //
        double vecFromCameraToNearestX = X - view.camera[0];
        double vecFromCameraToNearestY = Y - view.camera[1];
        double vecFromCameraToNearestZ = Z - view.camera[2];

        double dot = vecFromCameraToPlaneX*vecFromCameraToNearestX
                   + vecFromCameraToPlaneY*vecFromCameraToNearestY
                   + vecFromCameraToPlaneZ*vecFromCameraToNearestZ;

        double newNearest = dot / vecMag;
        newNearest = newNearest - (view.farPlane-newNearest)*0.01; // fudge
        if (newNearest > view.nearPlane)
        {
            view.nearPlane = newNearest;
        }
        else
        {
            resetNearest = false;
        }
    }

    bool resetFarthest = true;
    if (farthestInd == -1 || farthest >= 1.)
    {
        resetFarthest = false;
    }

    if (resetFarthest)
    {
        double X = (farthestInd & 1 ? dbounds[1] : dbounds[0]);
        double Y = (farthestInd & 2 ? dbounds[3] : dbounds[2]);
        double Z = (farthestInd & 4 ? dbounds[5] : dbounds[4]);

        double vecFromCameraToFarthestX = X - view.camera[0];
        double vecFromCameraToFarthestY = Y - view.camera[1];
        double vecFromCameraToFarthestZ = Z - view.camera[2];

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
        double dot = vecFromCameraToPlaneX*vecFromCameraToFarthestX
                   + vecFromCameraToPlaneY*vecFromCameraToFarthestY
                   + vecFromCameraToPlaneZ*vecFromCameraToFarthestZ;

        //
        // dot = cos X * mag(A) * mag(B)
        // We know cos X = mag(C) / mag(A)   C = adjacent, A = hyp.
        // Then mag(C) = cos X * mag(A).
        // So mag(C) = dot / mag(B).
        //
        double newFarthest = dot / vecMag;

        newFarthest = newFarthest + (newFarthest-view.nearPlane)*0.01; // fudge
        if (newFarthest < view.farPlane)
        {
            view.farPlane = newFarthest;
        }
        else
        {
            resetFarthest = false;
        }
    }

    if (resetNearest || resetFarthest)
    {
        CalculateTransform(view, transform, scale, aspect);
    }
}


// ****************************************************************************
//  Method: avtWorldSpaceToImageSpaceTransform::ExecuteData
//
//  Purpose:
//     Pass rectilinear datasets through if specified.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Hank Childs, Wed Dec 24 09:48:59 PST 2008
//    Add support for sampling in the wbuffer.
//
// ****************************************************************************

vtkDataSet *
avtWorldSpaceToImageSpaceTransform::ExecuteData(vtkDataSet *in_ds, int domain,
                                                std::string label)
{
    if (passThruRectilinear && 
                            in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        return in_ds;
    }

    // Since we're applying a transform to the data, an existing
    // implied transform will need to change.  Update it here.
    avtDataAttributes &inatts = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outatts = GetOutput()->GetInfo().GetAttributes();
    if (inatts.GetRectilinearGridHasTransform())
    {
        // Get the world-to-image transform and its inverse.
        const double *W_to_I = *transform->Element;
        double        I_to_W[16];
        vtkMatrix4x4::Invert(W_to_I, I_to_W);

        // We need to apply a similarity transform to get the new
        // rectilinear grid transform that should be passed down.
        const double *old_xform = inatts.GetRectilinearGridTransform();
        double        new_xform[16];

        double        tmp[16];

        vtkMatrix4x4::Multiply4x4(W_to_I, old_xform, tmp);
        vtkMatrix4x4::Multiply4x4(tmp, I_to_W, new_xform);

        // We've created the new transform; set it in the output.
        outatts.SetRectilinearGridHasTransform(true);
        outatts.SetRectilinearGridTransform(new_xform);
    }

    vtkDataSet *mid_ds = avtTransform::ExecuteData(in_ds, domain, label);

    // We only need to convert to w-buffer coords if we have perspective view.
    if (view.orthographic)
        return mid_ds;

    // If we have a rectilinear grid, it will be dealt with in the
    // mass voxel extractor.  Otherwise, we need to change the z-buffer
    // to a w-buffer, so we can space out the sample points better.
    if (mid_ds->GetDataObjectType() == VTK_STRUCTURED_GRID ||
        mid_ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
        vtkPointSet *ps = (vtkPointSet *) mid_ds;
        vtkPoints *pts = ps->GetPoints();
        int npts = pts->GetNumberOfPoints();
        for (int i = 0 ; i < npts ; i++)
        {
            double pt[3];
            pts->GetPoint(i, pt);
            double fp = view.farPlane;
            double np = view.nearPlane;
            // Current transform puts near at 1 and far at -1.  Reverse
            pt[2] *= -1.0;
            // Map to actual distance from camera.
            pt[2] = (-2*fp*np)
                     / ((pt[2]*(fp-np)) - (fp+np));
            // Now normalize based on near and far.
            pt[2] = (pt[2] - np) / (fp-np);
            pts->SetPoint(i, pt);
        }
    }

    return mid_ds;
}



// ****************************************************************************
//  Method:  avtWorldSpaceToImageSpaceTransform::FilterUnderstandsTransformedRectMesh
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
avtWorldSpaceToImageSpaceTransform::FilterUnderstandsTransformedRectMesh()
{
    // We've fixed this filter to do the right thing for these meshes.
    return true;
}
