/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                      avtOpenGL3DTextureVolumeRenderer.C                   //
// ************************************************************************* //

#include <float.h>
#include "avtOpenGL3DTextureVolumeRenderer.h"

#include <avtOpenGLExtensionManager.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <avtCallback.h>
#include <avtViewInfo.h>
#include <BoundingBoxContourer.h>
#include <DebugStream.h>
#include <LightList.h>
#include <VolumeAttributes.h>

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

// ****************************************************************************
//  Method: avtOpenGL3DTextureVolumeRenderer::avtOpenGL3DTextureVolumeRenderer
//
//  Purpose:
//    Initialize the texture memory pointer and the OpenGL texture ID.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 30, 2003
//
//  Modifications:
//    Brad Whitlock, Fri Aug 20 16:52:18 PST 2004
//    Added support for looking up glTexImage3D extension on Windows.
//
//    Brad Whitlock, Fri Sep 15 12:50:14 PDT 2006
//    I removed the code to get the glTexImage3D extension and initialized 
//    the GLEW library.
//
//    Tom Fogal, Sat Jul 25 19:45:26 MDT 2009
//    Use new GLEW initialization wrapper.
//
//    Tom Fogal, Fri Mar 19 16:19:37 MDT 2010
//    Don't initialize GLEW here; assume it is already initialized.
//
// ****************************************************************************

avtOpenGL3DTextureVolumeRenderer::avtOpenGL3DTextureVolumeRenderer()
{
    volumetex = NULL;
    volumetexId = 0;
}


// ****************************************************************************
//  Method: avtOpenGL3DTextureVolumeRenderer::~avtOpenGL3DTextureVolumeRenderer
//
//  Purpose:
//    Destructor.  Free volumetex.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  1, 2003
//
//    Thomas R. Treadway, Tue Feb  6 17:04:03 PST 2007
//    The gcc-4.x compiler no longer just warns about automatic type conversion.
//
// ****************************************************************************
avtOpenGL3DTextureVolumeRenderer::~avtOpenGL3DTextureVolumeRenderer()
{
    delete[] volumetex;
    volumetex = NULL;

    // Assumes context is current!  This is the job of the container class.
    if (volumetexId != 0)
    {
        glDeleteTextures(1, (GLuint*)&volumetexId);
        volumetexId = 0;
    }
}


// ****************************************************************************
//  Method:  avtOpenGL3DTextureVolumeRenderer::Render
//
//  Purpose:
//    Render one image using a 3D texture and view-aligned slice planes.
//
//  Arguments:
//    grid      : the data set to render
//    data,opac : the color/opacity variables
//    view      : the viewing information
//    atts      : the current volume plot attributes
//    vmin/max/size : the min/max/range of the color variable
//    omin/max/size : the min/max/range of the opacity variable
//    gx/gy/gz      : the gradient of the opacity variable
//    gm            : the gradient magnitude, un-normalized
//    gm_max        : the max gradient magnitude over the data set
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  1, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Oct 10 12:39:38 PDT 2003
//    Added check for the (new) HAVE_GL_TEX_IMAGE_3D flag.
//    Also, unconditionally enable it if avtOpenGL3DTextureVolumeRenderer
//    has been redefined, because this means we're doing Mesa, and Mesa
//    always supports 3D texturing.
//
//    Jeremy Meredith, Fri Oct 10 16:29:31 PDT 2003
//    Made it work with Mesa again.
//
//    Hank Childs, Tue May 11 15:24:45 PDT 2004
//    Turn off blending so transparent surfaces can work afterwards.
//
//    Eric Brugger, Tue Jul 27 11:52:38 PDT 2004
//    Add several casts and change a double constant to a float constant
//    to fix compile errors.
//
//    Brad Whitlock, Thu Aug 12 14:38:43 PST 2004
//    I removed the ifdef that prevented the code from building on Windows
//    and moved it into visit-config.h. I also added Windows-specific code.
//
//    Brad Whitlock, Fri Sep 15 12:52:53 PDT 2006
//    I made it use GLEW for getting the texture3D function.
//
//    Kathleen Bonnell, Wed Jan  3 15:51:59 PST 2007 
//    I made it revert (for Windows) to glTexImage3D_ptr if the
//    texture3D extension is not available.
//
//    Hank Childs, Tue Feb  6 15:41:58 PST 2007
//    Give an error that the user can see if 3D texturing is not available.
//
//    Thomas R. Treadway, Tue Feb  6 17:04:03 PST 2007
//    The gcc-4.x compiler no longer just warns about automatic type conversion.
//
//    Gunther H. Weber, Mon May 21 13:46:15 PDT 2007
//    Fixed 3D texture detection problem for Mac OS by checking for
//    GL_VERSION_1_2 before using GLEW.
//
//    Brad Whitlock, Thu Jan 10 14:47:34 PST 2008
//    Added reducedDetail argument.
//
//    Brad Whitlock, Wed Apr 22 12:06:12 PDT 2009
//    I changed the interface.
//
//    Jeremy Meredith, Tue Jan  5 15:49:43 EST 2010
//    Added ability to reduce amount of lighting for low-gradient-mag areas.
//    This was already enabled, to some degree, by default, but it's now both
//    optional and configurable.
//
//    Tom Fogal, Fri Mar 19 16:28:07 MDT 2010
//    Simplified greatly by querying for extension directly, independent of
//    Mesa queries.
//
//    Hank Childs, Wed Oct 12 05:45:27 PDT 2011
//    Increase the number of possible slices to 1000.
//
// ****************************************************************************

void
avtOpenGL3DTextureVolumeRenderer::Render(
    const avtVolumeRendererImplementation::RenderProperties &props,
    const avtVolumeRendererImplementation::VolumeData &volume)
{
    static bool haveIssuedWarning = false;

#ifdef HAVE_LIBGLEW
    if(!(avt::glew::supported("GL_VERSION_1_2") ||
         avt::glew::supported("GL_EXT_texture3D")))
#endif
    {
        if(!haveIssuedWarning)
        {
            avtCallback::IssueWarning("Volume rendering based on 3D textures "
                                      "is not supported on your GPU.");
            haveIssuedWarning = true;
        }
        return;
    }

    // Get the transfer function
    int ncolors=256;
    int nopacities=256;
    unsigned char rgba[256*4];
    props.atts.GetTransferFunction(rgba);
    
    // Get the dimensions
    int dims[3];
    vtkRectilinearGrid *grid = (vtkRectilinearGrid *)volume.grid;
    grid->GetDimensions(dims);

    int nx=dims[0];
    int ny=dims[1];
    int nz=dims[2];

    // Find the smallest power of two in each dimension 
    // that will accomodate this data set
    int newnx = MAX(int(pow(2.0,1+int(log(double(nx-1))/log(2.0)))),1);
    int newny = MAX(int(pow(2.0,1+int(log(double(ny-1))/log(2.0)))),1);
    int newnz = MAX(int(pow(2.0,1+int(log(double(nz-1))/log(2.0)))),1);

    // Get the new lighting parameters
    LightList lights = avtCallback::GetCurrentLightList();

    // Determine if we need to invalidate the old texture
    if (volumetex && (props.atts != oldAtts || lights != oldLights))
    {
        glDeleteTextures(1, (GLuint*)&volumetexId);
        volumetexId = 0;
        delete[] volumetex;
        volumetex = NULL;
    }
    oldAtts = props.atts;
    oldLights = lights;


    //
    // Extract the lighting information from the actual light list
    //

    float light[4] = {0,0,1, 0};
    float ambient = 0.0;

    // Find an ambient light
    int i;
    for (i=0; i<lights.NumLights(); i++)
    {
        const LightAttributes &l = lights.GetLight(i);
        if (l.GetEnabledFlag() && l.GetType()==LightAttributes::Ambient)
        {
            // Take it's overall brightness
            double rgba[4];
            l.GetColor().GetRgba(rgba);
            ambient = l.GetBrightness() * (rgba[0]+rgba[1]+rgba[2])/3.;
            break;
        }
    }

    // Find a directional (object or camera) light
    for (i=0; i<lights.NumLights(); i++)
    {
        const LightAttributes &l = lights.GetLight(i);
        if (l.GetEnabledFlag() && l.GetType()!=LightAttributes::Ambient)
        {
            // Take it's direction
            const double *dir = l.GetDirection();
            light[0] = dir[0];
            light[1] = dir[1];
            light[2] = dir[2];
            break;
        }
    }

    // If we want to transform the light so it is attached to the camera:
    //I->MultiplyPoint(light, light);


    //
    // Create the 3D texture if we need to
    //

    if (!volumetex)
    {
        int nels=newnx*newny*newnz;
        volumetex = new unsigned char[4*nels];
        int outindex = -1;
        for (int k=0; k<newnz; k++)
        {
            for (int j=0; j<newny; j++)
            {
                for (int i=0; i<newnx; i++)
                {
                    int ijk[3]={i,j,k};
                    outindex++;
                    volumetex[outindex*4 + 0] = 0;
                    volumetex[outindex*4 + 1] = 0;
                    volumetex[outindex*4 + 2] = 0;
                    volumetex[outindex*4 + 3] = 0;

                    if (i>=nx || j>=ny || k>=nz)
                    {
                        // out of bounds data
                        continue;
                    }

                    int index = grid->ComputePointId(ijk);

                    float  v = volume.data.data->GetTuple1(index);
                    float  o = volume.opacity.data->GetTuple1(index);

                    // drop empty ones
                    if (v < -1e+37)
                        continue;

                    // normalize the value
                    v = (v < volume.data.min ? volume.data.min : v);
                    v = (v > volume.data.max ? volume.data.max : v);
                    v = (v-volume.data.min)/volume.data.size;
                    o = (o < volume.opacity.min ? volume.opacity.min : o);
                    o = (o > volume.opacity.max ? volume.opacity.max : o);
                    o = (o-volume.opacity.min)/volume.opacity.size;

                    // opactity map
                    float opacity;
                    opacity = float(rgba[int(o*(nopacities-1))*4 + 3])*
                        props.atts.GetOpacityAttenuation()/256.;
                    opacity = MAX(0,MIN(1,opacity));

                    // drop transparent splats 
                    //if (opacity < .0001)
                    //    continue;

                    // do shading
                    float brightness;
                    const bool shading = props.atts.GetLightingFlag();
                    if (shading)
                    {
                        // Get the gradient
                        float gi = volume.gx[index];
                        float gj = volume.gy[index];
                        float gk = volume.gz[index];

                        // Amount of shading should be somewhat proportional
                        // to the magnitude of the gradient
                        float gm = 1.0;
                        if (props.atts.GetLowGradientLightingReduction() !=
                                                      VolumeAttributes::Off)
                        {
                           double lp = 1.0;
                           switch (props.atts.GetLowGradientLightingReduction())
                           {
                             case VolumeAttributes::Lowest:  lp = 1./16.;break;
                             case VolumeAttributes::Lower:   lp = 1./8.; break;
                             case VolumeAttributes::Low:     lp = 1./4.; break;
                             case VolumeAttributes::Medium:  lp = 1./2.; break;
                             case VolumeAttributes::High:    lp = 1.;    break;
                             case VolumeAttributes::Higher:  lp = 2.;    break;
                             case VolumeAttributes::Highest: lp = 4.;    break;
                             default: break;
                           }
                           if (props.atts.GetLowGradientLightingClampFlag())
                           {
                               gm = volume.gm[index] /
                                 props.atts.GetLowGradientLightingClampValue();
                           }
                           else
                           {
                               gm = volume.gmn[index];
                           }
                           gm = pow((double)gm, lp);
                        }
                        if (gm < 0)
                            gm = 0;
                        if (gm > 1)
                            gm = 1;

                        // Get the base lit brightness based on the 
                        // light direction and the gradient
                        float grad[3] = {gi,gj,gk};
                        float lightdir[3] = {light[0],light[1],light[2]};
                        brightness = vtkMath::Dot(grad,lightdir);
                        if (brightness<0) brightness *= -1;

                        // Modulate by the gradient magnitude
                        brightness = (1.0 - gm)*1.0 + gm*brightness;

                        // Modulate by the amount of ambient lighting
                        brightness = (1.0 - ambient)*brightness + ambient;
                    }
                    else
                    {
                        // No shading ata ll
                        brightness=1;
                    }
                
                    // Determine the actual color and opacity now
                    int colorindex = int(ncolors * v);
                    if (colorindex < 0)
                        colorindex = 0;
                    if (colorindex >= ncolors)
                        colorindex =  ncolors-1;
                    int colorindex4 = colorindex*4;
                    float scaledbrightness = brightness/255.;
                    float r,g,b;
                    r = float(rgba[colorindex4 + 0])*scaledbrightness;
                    g = float(rgba[colorindex4 + 1])*scaledbrightness;
                    b = float(rgba[colorindex4 + 2])*scaledbrightness;

                    volumetex[outindex*4 + 0] = (unsigned char)(r*255);
                    volumetex[outindex*4 + 1] = (unsigned char)(g*255);
                    volumetex[outindex*4 + 2] = (unsigned char)(b*255);
                    volumetex[outindex*4 + 3] = (unsigned char)(opacity*255);
                }
            }
        }

        // Create the texture
        //glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glGenTextures(1, (GLuint*)&volumetexId);
        glBindTexture(GL_TEXTURE_3D, volumetexId);


        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, newnx, newny, newnz,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, volumetex);
    }

    //
    // We have the texture; now draw with it
    //

    // Set up OpenGL parameters
    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_3D, volumetexId);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_3D);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    bool alreadyBlending = glIsEnabled(GL_BLEND);
    if (!alreadyBlending)
        glEnable(GL_BLEND);
    glDepthMask(false);

    // Set up camera parameters
    vtkCamera *camera = vtkCamera::New();
    props.view.SetCameraFromView(camera);
    vtkMatrix4x4 *cameraMatrix = camera->GetViewTransformMatrix();

    //
    // Contour the bounding box at a user specified number of depths
    //

    BoundingBoxContourer bbox;

    // Extract the depth values at the corners of our bounding box
    int mapStructuredToUnstructured[8] = {0, 1, 3, 2, 4, 5, 7, 6};
    int bbox_index = 0;
    float minz =  FLT_MAX;
    float maxz = -FLT_MAX;
    for (int k=0; k<nz; k+=nz-1)
    {
        for (int j=0; j<ny; j+=ny-1)
        {
            for (int i=0; i<nx; i+=nx-1)
            {
                int ijk[] = {i,j,k};
                double worldpt[4] = {0,0,0,1};
                grid->GetPoint(grid->ComputePointId(ijk), worldpt);

                // Get the world space coordinates

                // The contourer expects an unstructured hex
                int pt_index = mapStructuredToUnstructured[bbox_index];

                bbox.x[pt_index] = worldpt[0];
                bbox.y[pt_index] = worldpt[1];
                bbox.z[pt_index] = worldpt[2];

                // Get the texture coordinates
                bbox.r[pt_index] = float(i) / float(newnx + 0) + (0.5 / float(newnx));
                bbox.s[pt_index] = float(j) / float(newny + 0) + (0.5 / float(newny));
                bbox.t[pt_index] = float(k) / float(newnz + 0) + (0.5 / float(newnz));

                // Get the camera space coordinates
                double viewpt[4];
                cameraMatrix->MultiplyPoint(worldpt, viewpt);
                float dist = viewpt[2];

                bbox.v[pt_index] = dist;

                if (dist < minz)
                    minz = dist;
                if (dist > maxz)
                    maxz = dist;

                bbox_index++;

                if (nx == 1)
                    break;
            }

            if (ny == 1)
                break;
        }

        if (nz == 1)
            break;
    }

    // Determine the depth values we need
    int pt0 = mapStructuredToUnstructured[0];
    int ptn = mapStructuredToUnstructured[7];
    float dx = bbox.x[pt0] - bbox.x[ptn];
    float dy = bbox.y[pt0] - bbox.y[ptn];
    float dz = bbox.z[pt0] - bbox.z[ptn];
    float dist = sqrt(dx*dx + dy*dy + dz*dz);

    // Do the actual contouring 
    glBegin(GL_TRIANGLES);
    glColor4f(1.,1.,1.,1.);
    int ns = props.atts.GetNum3DSlices();
    if (ns < 1)
        ns = 1;
    if (ns > 1000)
        ns = 1000;

    float tr[15], ts[15], tt[15];
    float vx[15], vy[15], vz[15];
    int   ntriangles;
    for (int z=0; z<ns; z++)
    {
        float value = (float(z)+0.5)/float(ns);

        bbox.ContourTriangles(minz + dist*value,
                              ntriangles, tr, ts, tt, vx, vy, vz);

        for (int t=0; t<ntriangles*3; t++)
        {
            glTexCoord3f(tr[t], ts[t], tt[t]);
            glVertex3f(vx[t], vy[t], vz[t]);
        }
    }
    glEnd();

    // Set some GL parameters back to their expected values and free memory
    glDepthMask(true);
    glDisable(GL_TEXTURE_3D);
    if (!alreadyBlending)
        glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    camera->Delete();
}
