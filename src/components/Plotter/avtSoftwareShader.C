// ************************************************************************* //
//                             avtSoftwareShader.C                           //
// ************************************************************************* //

#include <avtSoftwareShader.h>

#include <vtkCamera.h>
#include <vtkLight.h>
#include <vtkMatrix4x4.h>

#include <LightAttributes.h>

#include <avtViewInfo.h>
#include <avtView3D.h>


// ****************************************************************************
//  Method: avtSoftwareShader::GetLightDirection
//
//  Purpose:
//      Gets the direction of the light.
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2004
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
        vtkCamera *cam = vtkCamera::New();
        avtViewInfo ccvi;
        cur_view.SetViewInfoFromView(ccvi);
        ccvi.SetCameraFromView(cam);
        vtkMatrix4x4 *lt = cam->GetCameraLightTransformMatrix();
        vtkLight *light = vtkLight::New();
        light->SetTransformMatrix(lt);
        float ld[3];
        ld[0] = la.GetDirection()[0];
        ld[1] = la.GetDirection()[1];
        ld[2] = la.GetDirection()[2];
        light->SetPosition(ld);
        light->GetTransformedPosition(ld);
        light_dir[0] = -ld[0];
        light_dir[1] = -ld[1];
        light_dir[2] = -ld[2];
        light->Delete();
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
// ****************************************************************************

void
avtSoftwareShader::AddShadows(avtImage_p light_image, avtImage_p current_image,
                              avtView3D &light_dir, avtView3D &current_view, 
                              double aspect, double strength)
{
    //
    // Set up the buffers we will be reading from and writing to.
    //
    unsigned char *rgb = current_image->GetImage().GetRGBBuffer();
    float *cur_image_zbuff = current_image->GetImage().GetZBuffer();
    float *light_image_zbuff = light_image->GetImage().GetZBuffer();
    int rs, cs;
    current_image->GetImage().GetSize(&rs, &cs);

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
                     light_cam->GetCompositePerspectiveTransformMatrix(aspect,
                                    light_clip_range[0], light_clip_range[1]);
    vtkMatrix4x4 *light_inverse = vtkMatrix4x4::New();
    vtkMatrix4x4::Invert(light_trans, light_inverse);

    //
    // Now iterate over every pixel in the scene and see if the light can
    // see the same spots as the camera.
    //
    bool *should_shade = new bool[rs*cs];
    for (int j = 0 ; j < rs ; j++)
        for (int i = 0 ; i < cs ; i++)
        {
            int idx = j*cs + i;

            //
            // If there is nothing in the real image, then there is nothing
            // to cross-reference against -- return now before doing any work.
            //
            if (cur_image_zbuff[idx] > 0.99)
                continue;

            //
            // Convert from "display"/screen space to view space.
            //
            float view[4];
            view[0] = (i - cs/2.)/(cs/2.);
            view[1] = (j - rs/2.)/(rs/2.);
            // I expected the z to be from 0 to 1, but the VTK matrices
            // require you to perform this manipulation with the clipping
            // range.
            view[2] = cur_image_zbuff[idx]*
                                   (cur_clip_range[1]-cur_clip_range[0])
                       + cur_clip_range[0] ;
            view[3] = 1.;

            float world[4];
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
            float view_light[4];
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
            int display[2];
            float z_display;
            display[0] = (int)(view_light[0] * cs/2. + cs/2.);
            display[1] = (int)(view_light[1] * rs/2. + rs/2.);
            z_display = (view_light[2] - light_clip_range[0]) /
                        (light_clip_range[1] - light_clip_range[0]);

            bool found_something_close = false;

            //
            // Compare the pixels in display space (using z-buff comparisons).
            //
            for (int m = display[0] - 1 ; m <= display[0] + 1 ; m++)
            {
                if (m < 0 || m >= cs)
                    continue;
                for (int n = display[1] - 1 ; n <= display[1] + 1 ; n++)
                {
                    if (n < 0 || n >= rs)
                        continue;
                    int light_idx = n*cs + m;
                    if (z_display < light_image_zbuff[light_idx]+0.001)
                        found_something_close = true;
                }
            }


/*
 * HERE IS AN ATTEMPT TO COMPARE THE PIXELS IN WORLD SPACE.
 * THIS IS HERE IN CASE WE DECIDE NOT TO DO THE COMPARISONS IN DISPLAY
 * SPACE (WHICH IS WHAT IS BEING DONE ABOVE.
 *
            int light_idx = display[1]*cs + display[0];
            float z = light_image_zbuff[light_idx];
            float z_view = z*(light_clip_range[1] - light_clip_range[0])
                        + light_clip_range[0];
            float l[4];
            l[0] = view_light[0];
            l[1] = view_light[1];
            l[2] = z_view;
            l[3] = 1.;
            float w2[4];
            light_inverse->MultiplyPoint(l, w2);
            w2[0] /= w2[3];
            w2[1] /= w2[3];
            w2[2] /= w2[3];
            w2[3] = 1.;
            float xd=world[0] - w2[0];
            float yd=world[1] - w2[1];
            float zd=world[2] - w2[2];
            float dist = xd*xd+yd*yd+zd*zd;
            if (dist < 0.01)
                found_something_close = true;
 */

            if (!found_something_close)
                should_shade[idx] = true;
            else
                should_shade[idx] = 0;
        }
    double amount_remaining = 1. - strength;
    for (int j = 0 ; j < rs ; j++)
        for (int i = 0 ; i < cs ; i++)
        {
            int idx = j*cs + i;
            if (cur_image_zbuff[idx] > 0.99)
                continue;
            if (should_shade[idx])
            {
                rgb[3*idx+0] = (unsigned char)(rgb[3*idx+0]*amount_remaining);
                rgb[3*idx+1] = (unsigned char)(rgb[3*idx+1]*amount_remaining);
                rgb[3*idx+2] = (unsigned char)(rgb[3*idx+2]*amount_remaining);
            }
/*
 * BELOW IS AN ATTEMPT TO DO SOFT SHADOWS.  
 * IT NEEDED TO DO Z-BUFFER COMPARISONS AND BETTER AVERAGING.
 * IT'S A STARTING PLACE IF SOMEONE ELSE WANTS TO DO THIS, SO I'M LEAVING
 * IT IN.
 *
            else
            {
                continue;
                int m,n,k;
                for (k = 1 ; k < 4 ; k++)
                {
                    bool foundShade = false;
                    if (j-k > 0)
                    {
                        for (m = i - k ; m <= i + k ; m++)
                        {
                            if (m < 0 || m >= cs)
                                continue;
                            int idx2 = (j-k)*cs + m;
                            if (should_shade[idx2])
                                foundShade = true;
                         }
                    }
                    if (j+k < rs)
                    {
                        for (m = i - k ; m <= i + k ; m++)
                        {
                            if (m < 0 || m >= cs)
                                continue;
                            int idx2 = (j+k)*cs + m;
                            if (should_shade[idx2])
                                foundShade = true;
                         }
                    }
                    if (i-k > 0)
                    {
                        for (n = j - k ; n <= j + k ; n++)
                        {
                            if (n < 0 || n >= rs)
                                continue;
                            int idx2 = n*cs + (i-k);
                            if (should_shade[idx2])
                                foundShade = true;
                         }
                    }
                    if (i+k < cs)
                    {
                        for (n = j - k ; n <= j + k ; n++)
                        {
                            if (n < 0 || n >= rs)
                                continue;
                            int idx2 = n*cs + (i+k);
                            if (should_shade[idx2])
                                foundShade = true;
                         }
                    }
                    if (foundShade)
                    {
                        float multiplier = 1.;
                        switch (k)
                        {
                           case 1:
                             multiplier = 0.625;
                             break;
                           case 2:
                             multiplier = 0.75;
                             break;
                           case 3:
                             multiplier = 0.875;
                             break;
                        }
                        rgb[3*idx+0] *= multiplier;
                        rgb[3*idx+1] *= multiplier;
                        rgb[3*idx+2] *= multiplier;
                    }
                }
            }
   */
        }

    delete [] should_shade;
    cur_cam->Delete();
    cur_inverse->Delete();
    light_cam->Delete();
    light_inverse->Delete();
}


