/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                             avtSoftwareShader.C                           //
// ************************************************************************* //

#include <avtSoftwareShader.h>

#include <vtkCamera.h>
#include <vtkLight.h>
#include <vtkMatrix4x4.h>

#include <LightAttributes.h>

#include <avtMatrix.h>
#include <avtViewInfo.h>
#include <avtView3D.h>


extern double estimates[512][3];

// ****************************************************************************
//  Function:  CalculateShadow_Hard
//
//  Purpose:
//    Calculates shadows, but no soft edges -- either 1 or 0 for alpha.
//
//  Programmer:  Hank Childs
//  Creation:    October 24, 2004
//
//  Modifications:
//    Jeremy Meredith, Fri Oct 29 15:50:32 PDT 2004
//    Turned into its own function.
//
// ****************************************************************************
static double CalculateShadow_Hard(int l_width, int l_height,
                                  float *light_image_zbuff,
                                  double z_display,
                                  double f_display[2], int display[2])
{
    bool found_something_close = false;

    //
    // Compare the pixels in display space (using z-buff comparisons).
    //
    for (int m = display[0] - 1 ; m <= display[0] + 1 ; m++)
    {
        if (m < 0 || m >= l_width)
        {
            found_something_close = -1;
            continue;
        }
        for (int n = display[1] - 1 ; n <= display[1] + 1 ; n++)
        {
            if (n < 0 || n >= l_height)
            {
                found_something_close = -1;
                continue;
            }
            int light_idx = n*l_width + m;
            if (z_display < light_image_zbuff[light_idx]+0.01)
                found_something_close = true;
        }
    }


    if (found_something_close)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// ****************************************************************************
//  Function:  CalculateShadow_Antialiased
//
//  Purpose:
//    Calculates shadows, with antialiased edges.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 29, 2004
//
//  Modifications:
//
// ****************************************************************************
static double CalculateShadow_Antialiased(int l_width, int l_height,
                                         float *light_image_zbuff,
                                         double z_display,
                                         double f_display[2], int display[2])
{
    double f0 = f_display[0]-double(display[0]);
    double f1 = f_display[1]-double(display[1]);
    int d0 = display[0];
    int d1 = display[1];
    if (display[0] < l_width-1 && display[1] < l_height-1 &&
        display[0] > 0 && display[1] > 0)
    {
        //
        // Look at the 3x3 window around or light-buffer pixel, and
        // calculate an index into an array (see below) based on which
        // of the 9 pixels say we are in shadow and which ones don't
        //
        int bits=0;
        for (int m = -1 ; m <= +1 ; m++)
        {
            for (int n = -1 ; n <= +1 ; n++)
            {
                int light_idx = (d1+n)*l_width + (d0+m);
                if (z_display < light_image_zbuff[light_idx]+0.02)
                {
                    bits++;
                }
                else
                {
                }
                bits *= 2;
            }
        }
        bits /= 2;

        //
        // If they all said we were in shadow, or if they all said we
        // were not, then return 0/1 for alpha.  Otherwise, guess where
        // the shadow line instersects, and make an attempt to use this
        // information for antialiasing.
        //
        double alpha = 1.0;
        if (bits == 0)
        {
            alpha = 0;
        }
        else if (bits == 511)
        {
            alpha = 1;
        }
        else
        {
            // A = x-normal, B = y-normal, D = normal-offset
            // i.e. line equation Ax+By+D=0
            double A = estimates[bits][0];
            double B = estimates[bits][1];
            double D = estimates[bits][2];

            // No division by zero!
            if (A!=0 || B!=0)
            {
                double tmp = f0*A + f1*B + D;
                double dist = sqrt(tmp*tmp / (A*A + B*B));
                if (tmp < 0)
                    dist *= -1;

                alpha = (dist + 1.) / 2.;
                if (alpha > 1)
                    alpha = 1;
                if (alpha < 0)
                    alpha = 0;
            }
        }

        return alpha;
    }

    return 0;
}

// ****************************************************************************
//  Function:  CalculateShadow_Soft
//
//  Purpose:
//    Calculates soft shadows.  Turns out a little posterized, so I
//    wouldn't recommend using it for now.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 29, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Jun  9 14:43:27 PDT 2006
//    Comment out currently unused variable to address compiler warning.
//
// ****************************************************************************
static double CalculateShadow_Soft(int l_width, int l_height,
                                  float *light_image_zbuff,
                                  double z_display,
                                  double f_display[2], int display[2])
{
    //
    // The gist here is to look at the points in the light's z-buffer
    // and if we are mostly in shadow, then guess the equation of the
    // plane above our scene-geometry point.  Guess where it
    // intersects the scene-geometry plane, and figure out how far our
    // scene-geometry point is from this intersection line.  Then fade
    // out over a large distance.
    //
    // This doesn't work all that well in practice because of two things:
    // 1) The planes shadowing our scene-geometry point are made of
    //    polygons, and at the transition to a new polygon, our distance
    //    estimate changes drastically.
    // 2) At the true edge of the geometry, one or two of our sample
    //    points used to estimate the shadowing plane is not on the
    //    actual shadowing geometry, but instead on the shadowed
    //    geometry, and this gives us an incorrect estimate for our

    double dist = 0;
    if (display[0] > 0 && display[1] > 0)
    {
        int d0 = display[0];
        int d1 = display[1];
        int ndx00 = d1*l_width + d0;
        int ndx01 = (d1-1)*l_width + d0;
        int ndx10 = d1*l_width + (d0-1);
        double z00 = (light_image_zbuff[ndx00] - z_display);
        double z01 = (light_image_zbuff[ndx01] - z_display);
        double z10 = (light_image_zbuff[ndx10] - z_display);
        if (z00 < -.01)
        {
            double A = z10 - z00;
            double B = z00 - z01;
            // double C = 1;
            double D = -z00;
            double f0 = f_display[0]-double(display[0]);
            double f1 = f_display[1]-double(display[1]);
            if (A!=0 || B!=0)
            {
                double tmp = f0*A + f1*B + D;
                dist = sqrt(tmp*tmp / (A*A + B*B));
                if (tmp < 0)
                    dist *= -1;
                // The following line uses the cell center instead
                // of the true cell offset.  This doesn't actually
                // hurt, because we are grading distance over
                // many, many more pixels below (e.g. like 70), and
                // this will affect the shadow alpha by no more than
                // about 1 percent.
                //dist = D / sqrt(A*A + B*B);
            }
        }
        else
        {
            dist=0;
        }
    }

    double alpha = 1;
    if (dist < .0001)
    {
    }
    else
    {
        alpha = 1.0 - ( dist/70. );
        if (alpha > 1)
            alpha = 1;
        if (alpha < 0)
            alpha = 0;
    }
    return alpha;
}

// ****************************************************************************
//  Method: avtSoftwareShader::GetLightDirection
//
//  Purpose:
//      Gets the direction of the light.
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2004
//  
//  Modifications:
//
//    Hank Childs, Mon Nov  1 11:17:25 PST 2004
//    Make use of new avtMatrix functions to calculate correct camera lights.
//
// ****************************************************************************

bool
avtSoftwareShader::GetLightDirection(const LightAttributes &la, 
                                  const avtView3D &cur_view, double *light_dir)
{
    bool canShade = true;
    if (la.GetType() == LightAttributes::Object)
    {
        light_dir[0] = -la.GetDirection()[0];
        light_dir[1] = -la.GetDirection()[1];
        light_dir[2] = -la.GetDirection()[2];
    }
    else if (la.GetType() == LightAttributes::Camera)
    {
        light_dir[0] = la.GetDirection()[0];
        light_dir[1] = la.GetDirection()[1];
        light_dir[2] = la.GetDirection()[2];
        vtkCamera *cam = vtkCamera::New();
        avtViewInfo ccvi;
        cur_view.SetViewInfoFromView(ccvi);
        ccvi.SetCameraFromView(cam);

        double pos[3];
        cam->GetPosition(pos);
        double focus[3];
        cam->GetFocalPoint(focus);
        double up[3];
        cam->GetViewUp(up);

        avtMatrix mat;
        mat.MakeRotation(pos, focus, up);

        avtVector v = mat * (avtVector(light_dir));
        light_dir[0] = -v.x;
        light_dir[1] = -v.y;
        light_dir[2] = -v.z;
        cam->Delete();
    }
    else
        canShade = false;

    if (canShade)
    {
        double mag = light_dir[0]*light_dir[0] + light_dir[1]*light_dir[1] +
                     light_dir[2]*light_dir[2];
        mag = sqrt(mag);
        if (mag != 0.)
        {
            light_dir[0] /= mag;
            light_dir[1] /= mag;
            light_dir[2] /= mag;
        }
    }

    return canShade;
}


// ****************************************************************************
//  Method: avtSoftwareShader::AddShadows
//
//  Purpose:
//      Adds shadows to the images.
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2004
//
//  Modifications:
//    Jeremy Meredith, Fri Oct 29 14:44:57 PDT 2004
//    Allow for a different size light source image than output image.
//    Added ability to calculate antialiased/soft shadows by
//    abstracting out the relevant pieces.  Removed unused code since
//    it is now safely in version control.  Calculate aspect inside
//    this method instead of passing in.  It is not obvious in this
//    method, but we are now using an orthographic view for the light.
//
// ****************************************************************************

void
avtSoftwareShader::AddShadows(avtImage_p light_image, avtImage_p current_image,
                              avtView3D &light_dir, avtView3D &current_view, 
                              double strength)
{
    //
    // Set up the buffers we will be reading from and writing to.
    //
    unsigned char *rgb = current_image->GetImage().GetRGBBuffer();
    float *cur_image_zbuff = current_image->GetImage().GetZBuffer();
    float *light_image_zbuff = light_image->GetImage().GetZBuffer();
    int rs, cs;
    current_image->GetImage().GetSize(&rs, &cs);
    int l_width, l_height;
    light_image->GetImage().GetSize(&l_height, &l_width);
 
    //
    // Calculate aspect ratios
    //
    double aspect = double(cs)/double(rs);
    double light_aspect = double(l_width)/double(l_height);

    //
    // Set up VTK camera objects corresponding to the current view.  This will
    // give us the matrix to use to transform our points.
    //
    avtViewInfo ccvi;
    current_view.SetViewInfoFromView(ccvi);
    vtkCamera *cur_cam = vtkCamera::New();
    ccvi.SetCameraFromView(cur_cam);
    double cur_clip_range[2];
    cur_cam->GetClippingRange(cur_clip_range);
    vtkMatrix4x4 *cur_trans =
                       cur_cam->GetCompositePerspectiveTransformMatrix(aspect,
                                       cur_clip_range[0], cur_clip_range[1]);
    vtkMatrix4x4 *cur_inverse = vtkMatrix4x4::New();
    vtkMatrix4x4::Invert(cur_trans, cur_inverse);

    //
    // Now do the same thing for "light view".
    //
    avtViewInfo lvi;
    light_dir.SetViewInfoFromView(lvi);
    vtkCamera *light_cam = vtkCamera::New();
    lvi.SetCameraFromView(light_cam);
    double light_clip_range[2];
    light_cam->GetClippingRange(light_clip_range);
    vtkMatrix4x4 *light_trans =
                     light_cam->GetCompositePerspectiveTransformMatrix(
                                    light_aspect,
                                    light_clip_range[0], light_clip_range[1]);
    vtkMatrix4x4 *light_inverse = vtkMatrix4x4::New();
    vtkMatrix4x4::Invert(light_trans, light_inverse);

    //
    // Now iterate over every pixel in the scene and see if the light can
    // see the same spots as the camera.
    //
    int j;
    for (j = 0 ; j < rs ; j++)
    {
        for (int i = 0 ; i < cs ; i++)
        {
            int idx = j*cs + i;

            //
            // If there is nothing in the real image, then there is nothing
            // to cross-reference against -- return now before doing any work.
            //
            if (cur_image_zbuff[idx] > 0.9999)
                continue;

            //
            // Convert from "display"/screen space to view space.
            //
            double view[4];
            view[0] = (i - cs/2.)/(cs/2.);
            view[1] = (j - rs/2.)/(rs/2.);
            // I expected the z to be from 0 to 1, but the VTK matrices
            // require you to perform this manipulation with the clipping
            // range.
            view[2] = cur_image_zbuff[idx]*
                                   (cur_clip_range[1]-cur_clip_range[0])
                       + cur_clip_range[0] ;
            view[3] = 1.;

            double world[4];
            cur_inverse->MultiplyPoint(view, world);
            if (world[3] != 0.)
            {
                world[0] /= world[3];
                world[1] /= world[3];
                world[2] /= world[3];
            }
            world[3] = 1.;

            //
            // Okay, we now know the world space coordinates of this pixel.
            // For ease of reference, let's call this PPOS.
            // If we put PPOS into the light transformation matrix, then it
            // will give us a point in view space for the light transform.
            //
            double view_light[4];
            light_trans->MultiplyPoint(world, view_light);
            if (view_light[3] != 0.)
            {
                view_light[0] /= view_light[3];
                view_light[1] /= view_light[3];
                view_light[2] /= view_light[3];
            }

            //
            // Now PPOS is in the light's view space.  Let's convert to screen
            // space so we can compare to the light's z-buffer value.
            // (Note that we are comparing z-buffer values rather than
            // converting these back into world space.)
            //
            double f_display[2];
            int   display[2];
            double z_display;
            f_display[0] = (view_light[0] * l_width/2. + l_width/2.);
            f_display[1] = (view_light[1] * l_height/2. + l_height/2.);
            display[0] = (int)(f_display[0]);
            display[1] = (int)(f_display[1]);
            z_display = (view_light[2] - light_clip_range[0]) /
                        (light_clip_range[1] - light_clip_range[0]);


            int mode = 0;
            double alpha;
            switch (mode)
            {
              case 0:
                alpha = CalculateShadow_Hard(l_width,l_height,
                                             light_image_zbuff,
                                             z_display, f_display, display);
                break;
              case 1:
                alpha = CalculateShadow_Antialiased(l_width,l_height,
                                             light_image_zbuff,
                                             z_display, f_display, display);
                break;
              case 2:
                alpha = CalculateShadow_Soft(l_width,l_height,
                                             light_image_zbuff,
                                             z_display, f_display, display);
                break;
              default:
                // Error
                alpha = 1;
                break;
            }

            unsigned char r = rgb[3*idx+0];
            unsigned char g = rgb[3*idx+1];
            unsigned char b = rgb[3*idx+2];

            double scale = 1.0 - (strength * (1.0 - alpha));
            r = (unsigned char)(scale * double(r));
            g = (unsigned char)(scale * double(g));
            b = (unsigned char)(scale * double(b));

            rgb[3*idx+0] = r;
            rgb[3*idx+1] = g;
            rgb[3*idx+2] = b;
        }
    }

    cur_cam->Delete();
    cur_inverse->Delete();
    light_cam->Delete();
    light_inverse->Delete();
}



// ****************************************************************************
//  Method:  avtSoftwareShader::AddDepthCueing
//
//  Purpose:
//    Apply depth cueing to the scene.  Note that this can be
//    applied independent of the view angle, so "fog" is not
//    quite the right implementation.
//
//  Arguments:
//    current_image        the image to apply depth cueing to
//    current_view         the view used to create that image
//    start                the point at which depth cueing begins
//    end                  the point at which depth cueing ends
//    cuecolor             typically the background color
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2007
//
// ****************************************************************************
void
avtSoftwareShader::AddDepthCueing(avtImage_p current_image,
                                  avtView3D &current_view, 
                                  const double start[3],
                                  const double end[3],
                                  unsigned char cuecolor[3])
{
    double direction[3] = {end[0]-start[0], end[1]-start[1], end[2]-start[2]};
    double mag = (direction[0]*direction[0] +
                  direction[1]*direction[1] +
                  direction[2]*direction[2]);
    if (mag == 0)
        return;

    //
    // Set up the buffers we will be reading from and writing to.
    //
    unsigned char *rgb = current_image->GetImage().GetRGBBuffer();
    float *cur_image_zbuff = current_image->GetImage().GetZBuffer();
    int rs, cs;
    current_image->GetImage().GetSize(&rs, &cs);
 
    //
    // Calculate aspect ratios
    //
    double aspect = double(cs)/double(rs);

    //
    // Set up VTK camera objects corresponding to the current view.  This will
    // give us the matrix to use to transform our points.
    //
    avtViewInfo ccvi;
    current_view.SetViewInfoFromView(ccvi);
    vtkCamera *cur_cam = vtkCamera::New();
    ccvi.SetCameraFromView(cur_cam);
    double cur_clip_range[2];
    cur_cam->GetClippingRange(cur_clip_range);
    vtkMatrix4x4 *cur_trans =
                       cur_cam->GetCompositePerspectiveTransformMatrix(aspect,
                                       cur_clip_range[0], cur_clip_range[1]);
    vtkMatrix4x4 *cur_inverse = vtkMatrix4x4::New();
    vtkMatrix4x4::Invert(cur_trans, cur_inverse);

    //
    // Now iterate over every pixel in the scene and see if the light can
    // see the same spots as the camera.
    //
    int j;
    for (j = 0 ; j < rs ; j++)
    {
        for (int i = 0 ; i < cs ; i++)
        {
            int idx = j*cs + i;

            //
            // If there is nothing in the real image, then there is nothing
            // to cross-reference against -- return now before doing any work.
            //
            if (cur_image_zbuff[idx] > 0.9999)
                continue;

            //
            // Convert from "display"/screen space to view space.
            //
            double view[4];
            view[0] = (i - cs/2.)/(cs/2.);
            view[1] = (j - rs/2.)/(rs/2.);
            // I expected the z to be from 0 to 1, but the VTK matrices
            // require you to perform this manipulation with the clipping
            // range.
            view[2] = cur_image_zbuff[idx]*
                                   (cur_clip_range[1]-cur_clip_range[0])
                       + cur_clip_range[0] ;
            view[3] = 1.;

            // And then to world space.
            double world[4];
            cur_inverse->MultiplyPoint(view, world);
            if (world[3] != 0.)
            {
                world[0] /= world[3];
                world[1] /= world[3];
                world[2] /= world[3];
            }
            world[3] = 1.;

            // Get the distance factor (in terms of the direction vec length)
            double scale = (direction[0]*(world[0]-start[0]) +
                            direction[1]*(world[1]-start[1]) +
                            direction[2]*(world[2]-start[2])) / mag;
            // must be between 0 and 1
            if (scale > 1)
                scale = 1;
            if (scale < 0)
                scale = 0;

            // fade it to the cue color
            unsigned char r = rgb[3*idx+0];
            unsigned char g = rgb[3*idx+1];
            unsigned char b = rgb[3*idx+2];

            r = (unsigned char)((1.-scale) * double(r) + (scale) * double(cuecolor[0]));
            g = (unsigned char)((1.-scale) * double(g) + (scale) * double(cuecolor[1]));
            b = (unsigned char)((1.-scale) * double(b) + (scale) * double(cuecolor[2]));

            // and set it back
            rgb[3*idx+0] = r;
            rgb[3*idx+1] = g;
            rgb[3*idx+2] = b;
        }
    }

    cur_cam->Delete();
    cur_inverse->Delete();
}



// ****************************************************************************
//  Method: avtSoftwareShader::FindLightView
//
//  Purpose:
//      Given the image to be shaded and the camera that generated it,
//      and given the light direction, and given the desired light
//      source image aspect ratio: calculate the avtView3D needed for
//      a light source camera to encompass all of the geometry in the
//      scene, or any geometry closer to the camera that could cast a
//      shadow on it.
//
//  Programmer: Jeremy Meredith
//  Creation:   October 28, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Jun  9 14:43:27 PDT 2006
//    Removed unused variable.
//
// ****************************************************************************

avtView3D
avtSoftwareShader::FindLightView(avtImage_p current_image,
                                 avtView3D &current_view,
                                 double *light_dir,
                                 double light_aspect)
{
    avtView3D light_view = current_view;
    light_view.perspective = false;
    light_view.parallelScale = 1;
    light_view.normal[0] = light_dir[0];
    light_view.normal[1] = light_dir[1];
    light_view.normal[2] = light_dir[2];
    light_view.imageZoom = 1;
    light_view.imagePan[0] = 0;
    light_view.imagePan[1] = 0;
    light_view.nearPlane = -10;
    light_view.farPlane  =  10;
    double dp_with_yaxis = fabs(light_view.normal[0]*0 +
                                light_view.normal[1]*1 +
                                light_view.normal[2]*0);
    if (fabs(dp_with_yaxis) > .95)
    {
        light_view.viewUp[0] = 0;
        light_view.viewUp[1] = 0;
        light_view.viewUp[2] = 1;
    }
    else
    {
        light_view.viewUp[0] = 0;
        light_view.viewUp[1] = 1;
        light_view.viewUp[2] = 0;
    }

    //
    // Set up the buffers we will be reading from and writing to.
    //
    float *cur_image_zbuff = current_image->GetImage().GetZBuffer();
    int rs, cs;
    current_image->GetImage().GetSize(&rs, &cs);
    double aspect = double(cs)/double(rs);

    //
    // Set up VTK camera objects corresponding to the current view.  This will
    // give us the matrix to use to transform our points.
    //
    avtViewInfo ccvi;
    current_view.SetViewInfoFromView(ccvi);
    vtkCamera *cur_cam = vtkCamera::New();
    ccvi.SetCameraFromView(cur_cam);
    double cur_clip_range[2];
    cur_cam->GetClippingRange(cur_clip_range);
    vtkMatrix4x4 *cur_trans =
                       cur_cam->GetCompositePerspectiveTransformMatrix(aspect,
                                        cur_clip_range[0], cur_clip_range[1]);
    vtkMatrix4x4 *cur_inverse = vtkMatrix4x4::New();
    vtkMatrix4x4::Invert(cur_trans, cur_inverse);


    //
    // Now do the same thing for "light view".
    //
    avtViewInfo lvi;
    light_view.SetViewInfoFromView(lvi);
    vtkCamera *light_cam = vtkCamera::New();
    lvi.SetCameraFromView(light_cam);
    double light_clip_range[2];
    light_cam->GetClippingRange(light_clip_range);
    vtkMatrix4x4 *light_trans =
                     light_cam->GetCompositePerspectiveTransformMatrix(
                                    light_aspect,
                                    light_clip_range[0], light_clip_range[1]);
    vtkMatrix4x4 *light_inverse = vtkMatrix4x4::New();
    vtkMatrix4x4::Invert(light_trans, light_inverse);

    //
    // We are going to find the center of the visible scene geometry in
    // "light source view space", and then use that (converted back
    // to world coordinates) as the the new "focus" of the light view.
    // Then we use the extents in "light source view space" to determine
    // the parallel scale and near/far planes of the light view.
    //
    double xmin = +1e37;
    double xmax = -1e37;
    double ymin = +1e37;
    double ymax = -1e37;
    double zmin = +1e37;
    double zmax = -1e37;

    bool found = false;

    for (int j = 0 ; j < rs ; j++)
    {
        for (int i = 0 ; i < cs ; i++)
        {
            int idx = j*cs + i;

            //
            // If there is nothing in the real image, then there is nothing
            // to cross-reference against -- return now before doing any work.
            //
            if (cur_image_zbuff[idx] == 1)
                continue;

            //
            // Convert from "display"/screen space to view space.
            //
            double view[4];
            view[0] = (i - cs/2.)/(cs/2.);
            view[1] = (j - rs/2.)/(rs/2.);
            // I expected the z to be from 0 to 1, but the VTK matrices
            // require you to perform this manipulation with the clipping
            // range.
            view[2] = cur_image_zbuff[idx]*
                                   (cur_clip_range[1]-cur_clip_range[0])
                       + cur_clip_range[0] ;
            view[3] = 1.;

            double world[4];
            cur_inverse->MultiplyPoint(view, world);
            if (world[3] != 0.)
            {
                world[0] /= world[3];
                world[1] /= world[3];
                world[2] /= world[3];
            }
            world[3] = 1.;

            //
            // Okay, we now know the world space coordinates of this pixel.
            // For ease of reference, let's call this PPOS.
            // If we put PPOS into the light transformation matrix, then it
            // will give us a point in view space for the light transform.
            //
            double view_light[4];
            light_trans->MultiplyPoint(world, view_light);
            if (view_light[3] != 0.)
            {
                view_light[0] /= view_light[3];
                view_light[1] /= view_light[3];
                view_light[2] /= view_light[3];
            }

            //
            // update the extents.  And mark if we ever found anything.
            //
            if (view_light[0] < xmin)  xmin = view_light[0];
            if (view_light[1] < ymin)  ymin = view_light[1];
            if (view_light[2] < zmin)  zmin = view_light[2];
            if (view_light[0] > xmax)  xmax = view_light[0];
            if (view_light[1] > ymax)  ymax = view_light[1];
            if (view_light[2] > zmax)  zmax = view_light[2];
            found = true;
        }
    }

    if (found)
    {
        // Update the focus
        double viewcenter_light[4] = {(xmin+xmax) / 2.,
                                     (ymin+ymax) / 2.,
                                     (zmin+zmax) / 2.,
                                     1.};
        double viewcenter_world[4];
        light_inverse->MultiplyPoint(viewcenter_light, viewcenter_world);

        light_view.focus[0] = viewcenter_world[0];
        light_view.focus[1] = viewcenter_world[1];
        light_view.focus[2] = viewcenter_world[2];

        // Update the parallel scale and near/far planes
        double xdist = xmax-xmin;
        double ydist = ymax-ymin;
        double zdist = zmax-zmin;

        double max_xy_scale = xdist > ydist ? xdist : ydist;
        light_view.parallelScale = max_xy_scale * .55;

        light_view.farPlane  = +zdist*.55;
        light_view.nearPlane = -zdist*3.5; 
        // Note: 0.5 is what we should set for the nearPlane to encompass
        // the geometry visible in the scene for correctness.  However,
        // there is the very likely possibility that things not in the
        // final scene will still cast shadows on the geometry in the 
        // final scene -- and only geometry within the ortho frustum
        // but closer to the light source than what is in the scene can
        // do it.  Thus, we move the near plane back by some safety factor.
        // In this case, we picked 4x the actual scene depth, losing
        // a total of 2 bits of depth buffer precision.
        // We could go higher if we want to sacrifice more bits.
    }
    else
    {
        // Warning -- no pixels in output image.  It doesn't really
        // matter what we do in this case, since no geometry will be
        // shaded anyway.
    }
    cur_cam->Delete();
    cur_inverse->Delete();
    light_cam->Delete();
    light_inverse->Delete();

    return light_view;
}

// ****************************************************************************
// These are estimates of A,B,D in the equation Ax+By+D = 0
// given boolean data for a 3x3 cell array.  Look at the file
// Generate3x3DividingEstimates.C for more details.
// ****************************************************************************
double estimates[512][3] = {
{0, 0, 0},                        // 0
{0.707107, 0.707107, -1.08538},   // 1
{0, 0, 0},                        // 2
{0.920115, 0.391649, -0.758571},  // 3
{0.707107, -0.707107, -1.08538},  // 4
{0, 0, 0},                        // 5
{0.920115, -0.391649, -0.758571}, // 6
{0.060049, 0.998195, -0.487295},  // 7
{0, 0, 0},                        // 8
{0.391649, 0.920115, -0.758571},  // 9
{0, 0, 0},                        // 10
{0.707107, 0.707107, -0.381742},  // 11
{0, 0, 0},                        // 12
{0, 0, 0},                        // 13
{0, 0, 0},                        // 14
{0.914832, 0.403836, -0.156818},  // 15
{0, 0, 0},                        // 16
{0, 0, 0},                        // 17
{0, 0, 0},                        // 18
{0, 0, 0},                        // 19
{0, 0, 0},                        // 20
{0, 0, 0},                        // 21
{0, 0, 0},                        // 22
{0, 0, 0},                        // 23
{0, 0, 0},                        // 24
{0, 0, 0},                        // 25
{0, 0, 0},                        // 26
{0, 0, 0},                        // 27
{0, 0, 0},                        // 28
{0, 0, 0},                        // 29
{0, 0, 0},                        // 30
{0.914832, 0.403836, 0.156818},   // 31
{0, 0, 0},                        // 32
{0, 0, 0},                        // 33
{0, 0, 0},                        // 34
{0, 0, 0},                        // 35
{0.391649, -0.920115, -0.758571}, // 36
{0, 0, 0},                        // 37
{0.707107, -0.707107, -0.381742}, // 38
{0.914832, -0.403835, -0.156818}, // 39
{0, 0, 0},                        // 40
{0, 0, 0},                        // 41
{0, 0, 0},                        // 42
{0, 0, 0},                        // 43
{0, 0, 0},                        // 44
{0, 0, 0},                        // 45
{0, 0, 0},                        // 46
{0, 0, 0},                        // 47
{0, 0, 0},                        // 48
{0, 0, 0},                        // 49
{0, 0, 0},                        // 50
{0, 0, 0},                        // 51
{0, 0, 0},                        // 52
{0, 0, 0},                        // 53
{0, 0, 0},                        // 54
{0.914832, -0.403835, 0.156818},  // 55
{0, 0, 0},                        // 56
{0, 0, 0},                        // 57
{0, 0, 0},                        // 58
{0, 0, 0},                        // 59
{0, 0, 0},                        // 60
{0, 0, 0},                        // 61
{0, 0, 0},                        // 62
{0.060049, 0.998195, 0.487295},   // 63
{-0.707107, 0.707107, -1.08538},  // 64
{0, 0, 0},                        // 65
{0, 0, 0},                        // 66
{0, 0, 0},                        // 67
{0, 0, 0},                        // 68
{0, 0, 0},                        // 69
{0, 0, 0},                        // 70
{0, 0, 0},                        // 71
{-0.391649, 0.920115, -0.758571}, // 72
{-4.37114e-08, 1, -0.487295},     // 73
{0, 0, 0},                        // 74
{0.403836, 0.914832, -0.156818},  // 75
{0, 0, 0},                        // 76
{0, 0, 0},                        // 77
{0, 0, 0},                        // 78
{0, 0, 0},                        // 79
{0, 0, 0},                        // 80
{0, 0, 0},                        // 81
{0, 0, 0},                        // 82
{0, 0, 0},                        // 83
{0, 0, 0},                        // 84
{0, 0, 0},                        // 85
{0, 0, 0},                        // 86
{0, 0, 0},                        // 87
{0, 0, 0},                        // 88
{0, 0, 0},                        // 89
{0, 0, 0},                        // 90
{0.403836, 0.914832, 0.156818},   // 91
{0, 0, 0},                        // 92
{0, 0, 0},                        // 93
{0, 0, 0},                        // 94
{0.707107, 0.707107, 0.381742},   // 95
{0, 0, 0},                        // 96
{0, 0, 0},                        // 97
{0, 0, 0},                        // 98
{0, 0, 0},                        // 99
{0, 0, 0},                        // 100
{0, 0, 0},                        // 101
{0, 0, 0},                        // 102
{0, 0, 0},                        // 103
{0, 0, 0},                        // 104
{0, 0, 0},                        // 105
{0, 0, 0},                        // 106
{0, 0, 0},                        // 107
{0, 0, 0},                        // 108
{0, 0, 0},                        // 109
{0, 0, 0},                        // 110
{0, 0, 0},                        // 111
{0, 0, 0},                        // 112
{0, 0, 0},                        // 113
{0, 0, 0},                        // 114
{0, 0, 0},                        // 115
{0, 0, 0},                        // 116
{0, 0, 0},                        // 117
{0, 0, 0},                        // 118
{0, 0, 0},                        // 119
{0, 0, 0},                        // 120
{0, 0, 0},                        // 121
{0, 0, 0},                        // 122
{0, 0, 0},                        // 123
{0, 0, 0},                        // 124
{0, 0, 0},                        // 125
{0, 0, 0},                        // 126
{0.920115, 0.391649, 0.758571},   // 127
{0, 0, 0},                        // 128
{0, 0, 0},                        // 129
{0, 0, 0},                        // 130
{0, 0, 0},                        // 131
{0, 0, 0},                        // 132
{0, 0, 0},                        // 133
{0, 0, 0},                        // 134
{0, 0, 0},                        // 135
{0, 0, 0},                        // 136
{0, 0, 0},                        // 137
{0, 0, 0},                        // 138
{0, 0, 0},                        // 139
{0, 0, 0},                        // 140
{0, 0, 0},                        // 141
{0, 0, 0},                        // 142
{0, 0, 0},                        // 143
{0, 0, 0},                        // 144
{0, 0, 0},                        // 145
{0, 0, 0},                        // 146
{0, 0, 0},                        // 147
{0, 0, 0},                        // 148
{0, 0, 0},                        // 149
{0, 0, 0},                        // 150
{0, 0, 0},                        // 151
{0, 0, 0},                        // 152
{0, 0, 0},                        // 153
{0, 0, 0},                        // 154
{0, 0, 0},                        // 155
{0, 0, 0},                        // 156
{0, 0, 0},                        // 157
{0, 0, 0},                        // 158
{0, 0, 0},                        // 159
{0, 0, 0},                        // 160
{0, 0, 0},                        // 161
{0, 0, 0},                        // 162
{0, 0, 0},                        // 163
{0, 0, 0},                        // 164
{0, 0, 0},                        // 165
{0, 0, 0},                        // 166
{0, 0, 0},                        // 167
{0, 0, 0},                        // 168
{0, 0, 0},                        // 169
{0, 0, 0},                        // 170
{0, 0, 0},                        // 171
{0, 0, 0},                        // 172
{0, 0, 0},                        // 173
{0, 0, 0},                        // 174
{0, 0, 0},                        // 175
{0, 0, 0},                        // 176
{0, 0, 0},                        // 177
{0, 0, 0},                        // 178
{0, 0, 0},                        // 179
{0, 0, 0},                        // 180
{0, 0, 0},                        // 181
{0, 0, 0},                        // 182
{0, 0, 0},                        // 183
{0, 0, 0},                        // 184
{0, 0, 0},                        // 185
{0, 0, 0},                        // 186
{0, 0, 0},                        // 187
{0, 0, 0},                        // 188
{0, 0, 0},                        // 189
{0, 0, 0},                        // 190
{0, 0, 0},                        // 191
{-0.920115, 0.391649, -0.758571}, // 192
{0, 0, 0},                        // 193
{0, 0, 0},                        // 194
{0, 0, 0},                        // 195
{0, 0, 0},                        // 196
{0, 0, 0},                        // 197
{0, 0, 0},                        // 198
{0, 0, 0},                        // 199
{-0.707107, 0.707107, -0.381742}, // 200
{-0.403836, 0.914832, -0.156818}, // 201
{0, 0, 0},                        // 202
{0, 0, 0},                        // 203
{0, 0, 0},                        // 204
{0, 0, 0},                        // 205
{0, 0, 0},                        // 206
{0, 0, 0},                        // 207
{0, 0, 0},                        // 208
{0, 0, 0},                        // 209
{0, 0, 0},                        // 210
{0, 0, 0},                        // 211
{0, 0, 0},                        // 212
{0, 0, 0},                        // 213
{0, 0, 0},                        // 214
{0, 0, 0},                        // 215
{0, 0, 0},                        // 216
{-0.403836, 0.914832, 0.156818},  // 217
{0, 0, 0},                        // 218
{-4.37114e-08, 1, 0.487295},      // 219
{0, 0, 0},                        // 220
{0, 0, 0},                        // 221
{0, 0, 0},                        // 222
{0.391649, 0.920115, 0.758571},   // 223
{0, 0, 0},                        // 224
{0, 0, 0},                        // 225
{0, 0, 0},                        // 226
{0, 0, 0},                        // 227
{0, 0, 0},                        // 228
{0, 0, 0},                        // 229
{0, 0, 0},                        // 230
{0, 0, 0},                        // 231
{0, 0, 0},                        // 232
{0, 0, 0},                        // 233
{0, 0, 0},                        // 234
{0, 0, 0},                        // 235
{0, 0, 0},                        // 236
{0, 0, 0},                        // 237
{0, 0, 0},                        // 238
{0, 0, 0},                        // 239
{0, 0, 0},                        // 240
{0, 0, 0},                        // 241
{0, 0, 0},                        // 242
{0, 0, 0},                        // 243
{0, 0, 0},                        // 244
{0, 0, 0},                        // 245
{0, 0, 0},                        // 246
{0, 0, 0},                        // 247
{0, 0, 0},                        // 248
{0, 0, 0},                        // 249
{0, 0, 0},                        // 250
{0, 0, 0},                        // 251
{0, 0, 0},                        // 252
{0, 0, 0},                        // 253
{0, 0, 0},                        // 254
{0.707107, 0.707107, 1.08538},    // 255
{-0.707107, -0.707107, -1.08538}, // 256
{0, 0, 0},                        // 257
{0, 0, 0},                        // 258
{0, 0, 0},                        // 259
{0, 0, 0},                        // 260
{0, 0, 0},                        // 261
{0, 0, 0},                        // 262
{0, 0, 0},                        // 263
{0, 0, 0},                        // 264
{0, 0, 0},                        // 265
{0, 0, 0},                        // 266
{0, 0, 0},                        // 267
{0, 0, 0},                        // 268
{0, 0, 0},                        // 269
{0, 0, 0},                        // 270
{0, 0, 0},                        // 271
{0, 0, 0},                        // 272
{0, 0, 0},                        // 273
{0, 0, 0},                        // 274
{0, 0, 0},                        // 275
{0, 0, 0},                        // 276
{0, 0, 0},                        // 277
{0, 0, 0},                        // 278
{0, 0, 0},                        // 279
{0, 0, 0},                        // 280
{0, 0, 0},                        // 281
{0, 0, 0},                        // 282
{0, 0, 0},                        // 283
{0, 0, 0},                        // 284
{0, 0, 0},                        // 285
{0, 0, 0},                        // 286
{0, 0, 0},                        // 287
{-0.391649, -0.920115, -0.758571},// 288
{0, 0, 0},                        // 289
{0, 0, 0},                        // 290
{0, 0, 0},                        // 291
{1.19249e-08, -1, -0.487295},     // 292
{0, 0, 0},                        // 293
{0.403836, -0.914832, -0.156818}, // 294
{0, 0, 0},                        // 295
{0, 0, 0},                        // 296
{0, 0, 0},                        // 297
{0, 0, 0},                        // 298
{0, 0, 0},                        // 299
{0, 0, 0},                        // 300
{0, 0, 0},                        // 301
{0, 0, 0},                        // 302
{0, 0, 0},                        // 303
{0, 0, 0},                        // 304
{0, 0, 0},                        // 305
{0, 0, 0},                        // 306
{0, 0, 0},                        // 307
{0, 0, 0},                        // 308
{0, 0, 0},                        // 309
{0.403836, -0.914832, 0.156818},  // 310
{0.707107, -0.707107, 0.381742},  // 311
{0, 0, 0},                        // 312
{0, 0, 0},                        // 313
{0, 0, 0},                        // 314
{0, 0, 0},                        // 315
{0, 0, 0},                        // 316
{0, 0, 0},                        // 317
{0, 0, 0},                        // 318
{0.920115, -0.391649, 0.758571},  // 319
{0, 0, 0},                        // 320
{0, 0, 0},                        // 321
{0, 0, 0},                        // 322
{0, 0, 0},                        // 323
{0, 0, 0},                        // 324
{0, 0, 0},                        // 325
{0, 0, 0},                        // 326
{0, 0, 0},                        // 327
{0, 0, 0},                        // 328
{0, 0, 0},                        // 329
{0, 0, 0},                        // 330
{0, 0, 0},                        // 331
{0, 0, 0},                        // 332
{0, 0, 0},                        // 333
{0, 0, 0},                        // 334
{0, 0, 0},                        // 335
{0, 0, 0},                        // 336
{0, 0, 0},                        // 337
{0, 0, 0},                        // 338
{0, 0, 0},                        // 339
{0, 0, 0},                        // 340
{0, 0, 0},                        // 341
{0, 0, 0},                        // 342
{0, 0, 0},                        // 343
{0, 0, 0},                        // 344
{0, 0, 0},                        // 345
{0, 0, 0},                        // 346
{0, 0, 0},                        // 347
{0, 0, 0},                        // 348
{0, 0, 0},                        // 349
{0, 0, 0},                        // 350
{0, 0, 0},                        // 351
{0, 0, 0},                        // 352
{0, 0, 0},                        // 353
{0, 0, 0},                        // 354
{0, 0, 0},                        // 355
{0, 0, 0},                        // 356
{0, 0, 0},                        // 357
{0, 0, 0},                        // 358
{0, 0, 0},                        // 359
{0, 0, 0},                        // 360
{0, 0, 0},                        // 361
{0, 0, 0},                        // 362
{0, 0, 0},                        // 363
{0, 0, 0},                        // 364
{0, 0, 0},                        // 365
{0, 0, 0},                        // 366
{0, 0, 0},                        // 367
{0, 0, 0},                        // 368
{0, 0, 0},                        // 369
{0, 0, 0},                        // 370
{0, 0, 0},                        // 371
{0, 0, 0},                        // 372
{0, 0, 0},                        // 373
{0, 0, 0},                        // 374
{0, 0, 0},                        // 375
{0, 0, 0},                        // 376
{0, 0, 0},                        // 377
{0, 0, 0},                        // 378
{0, 0, 0},                        // 379
{0, 0, 0},                        // 380
{0, 0, 0},                        // 381
{0, 0, 0},                        // 382
{0, 0, 0},                        // 383
{-0.920115, -0.391649, -0.758571},// 384
{0, 0, 0},                        // 385
{0, 0, 0},                        // 386
{0, 0, 0},                        // 387
{0, 0, 0},                        // 388
{0, 0, 0},                        // 389
{0, 0, 0},                        // 390
{0, 0, 0},                        // 391
{0, 0, 0},                        // 392
{0, 0, 0},                        // 393
{0, 0, 0},                        // 394
{0, 0, 0},                        // 395
{0, 0, 0},                        // 396
{0, 0, 0},                        // 397
{0, 0, 0},                        // 398
{0, 0, 0},                        // 399
{0, 0, 0},                        // 400
{0, 0, 0},                        // 401
{0, 0, 0},                        // 402
{0, 0, 0},                        // 403
{0, 0, 0},                        // 404
{0, 0, 0},                        // 405
{0, 0, 0},                        // 406
{0, 0, 0},                        // 407
{0, 0, 0},                        // 408
{0, 0, 0},                        // 409
{0, 0, 0},                        // 410
{0, 0, 0},                        // 411
{0, 0, 0},                        // 412
{0, 0, 0},                        // 413
{0, 0, 0},                        // 414
{0, 0, 0},                        // 415
{-0.707107, -0.707107, -0.381742},// 416
{0, 0, 0},                        // 417
{0, 0, 0},                        // 418
{0, 0, 0},                        // 419
{-0.403836, -0.914832, -0.156818},// 420
{0, 0, 0},                        // 421
{0, 0, 0},                        // 422
{0, 0, 0},                        // 423
{0, 0, 0},                        // 424
{0, 0, 0},                        // 425
{0, 0, 0},                        // 426
{0, 0, 0},                        // 427
{0, 0, 0},                        // 428
{0, 0, 0},                        // 429
{0, 0, 0},                        // 430
{0, 0, 0},                        // 431
{0, 0, 0},                        // 432
{0, 0, 0},                        // 433
{0, 0, 0},                        // 434
{0, 0, 0},                        // 435
{-0.403836, -0.914832, 0.156818}, // 436
{0, 0, 0},                        // 437
{1.19249e-08, -1, 0.487295},      // 438
{0.391649, -0.920115, 0.758571},  // 439
{0, 0, 0},                        // 440
{0, 0, 0},                        // 441
{0, 0, 0},                        // 442
{0, 0, 0},                        // 443
{0, 0, 0},                        // 444
{0, 0, 0},                        // 445
{0, 0, 0},                        // 446
{0.707107, -0.707107, 1.08538},   // 447
{-1, -8.74228e-08, -0.487295},    // 448
{0, 0, 0},                        // 449
{0, 0, 0},                        // 450
{0, 0, 0},                        // 451
{0, 0, 0},                        // 452
{0, 0, 0},                        // 453
{0, 0, 0},                        // 454
{0, 0, 0},                        // 455
{-0.914832, 0.403836, -0.156818}, // 456
{0, 0, 0},                        // 457
{0, 0, 0},                        // 458
{0, 0, 0},                        // 459
{0, 0, 0},                        // 460
{0, 0, 0},                        // 461
{0, 0, 0},                        // 462
{0, 0, 0},                        // 463
{0, 0, 0},                        // 464
{0, 0, 0},                        // 465
{0, 0, 0},                        // 466
{0, 0, 0},                        // 467
{0, 0, 0},                        // 468
{0, 0, 0},                        // 469
{0, 0, 0},                        // 470
{0, 0, 0},                        // 471
{-0.914832, 0.403836, 0.156818},  // 472
{-0.707107, 0.707107, 0.381742},  // 473
{0, 0, 0},                        // 474
{-0.391649, 0.920115, 0.758571},  // 475
{0, 0, 0},                        // 476
{0, 0, 0},                        // 477
{0, 0, 0},                        // 478
{0, 0, 0},                        // 479
{-0.914832, -0.403836, -0.156818},// 480
{0, 0, 0},                        // 481
{0, 0, 0},                        // 482
{0, 0, 0},                        // 483
{0, 0, 0},                        // 484
{0, 0, 0},                        // 485
{0, 0, 0},                        // 486
{0, 0, 0},                        // 487
{0, 0, 0},                        // 488
{0, 0, 0},                        // 489
{0, 0, 0},                        // 490
{0, 0, 0},                        // 491
{0, 0, 0},                        // 492
{0, 0, 0},                        // 493
{0, 0, 0},                        // 494
{0, 0, 0},                        // 495
{-0.914832, -0.403836, 0.156818}, // 496
{0, 0, 0},                        // 497
{0, 0, 0},                        // 498
{0, 0, 0},                        // 499
{-0.707107, -0.707107, 0.381742}, // 500
{0, 0, 0},                        // 501
{-0.391649, -0.920115, 0.758571}, // 502
{0, 0, 0},                        // 503
{-1, -8.74228e-08, 0.487295},     // 504
{-0.920115, 0.391649, 0.758571},  // 505
{0, 0, 0},                        // 506
{-0.707107, 0.707107, 1.08538},   // 507
{-0.920115, -0.391649, 0.758571}, // 508
{0, 0, 0},                        // 509
{-0.707107, -0.707107, 1.08538},  // 510
{0, 0, 0}                         // 511
};

