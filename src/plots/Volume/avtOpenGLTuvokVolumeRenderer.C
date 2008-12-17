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
//                      avtOpenGLTuvokVolumeRenderer.C                   //
// ************************************************************************* //

#include "avtOpenGLTuvokVolumeRenderer.h"

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <BoundingBoxContourer.h>
#include <VolumeAttributes.h>
#include <avtViewInfo.h>
#include <avtCallback.h>
#include <LightList.h>
#include <DebugStream.h>

#include <float.h>

#ifndef VTK_IMPLEMENT_MESA_CXX
  // Include GLEW.
  #include <visit-config.h>
  #ifdef HAVE_LIBGLEW
     #include <GL/glew.h>
     static bool glew_initialized = false;
  #endif

  #if defined(__APPLE__) && (defined(VTK_USE_CARBON) || defined(VTK_USE_COCOA))
    #include <OpenGL/gl.h>
  #else
    #if defined(_WIN32)
       #include <windows.h>
       // On Windows, we have to access glTexImage3D as an OpenGL extension.
       // In case texture3D extension is NOT available.
       static PFNGLTEXIMAGE3DEXTPROC glTexImage3D_ptr = 0;
    #endif
    #include <GL/gl.h>
  #endif
#else
  #include <GL/gl.h>
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

// ****************************************************************************
//  Method: avtOpenGLTuvokVolumeRenderer::avtOpenGLTuvokVolumeRenderer
//
//  Purpose:
//
//  Programmer:  Josh Stratton
//  Creation:    Mon Dec 15 11:58:50 MST 2008
//
//  Modifications:
//
// ****************************************************************************

avtOpenGLTuvokVolumeRenderer::avtOpenGLTuvokVolumeRenderer()
{
  //  cout << controller.DebugOut() << endl;
  /*
    volumetex = NULL;
    volumetexId = 0;

#ifndef VTK_IMPLEMENT_MESA_CXX
#ifdef HAVE_LIBGLEW
    GLenum err = glewInit();
    if(err != GLEW_OK)
    {
        debug1 << "avtOpenGLTuvokVolumeRenderer ctor: "
               << glewGetErrorString(err) << endl;
    }
    else
        glew_initialized = true;
#endif
#endif
  */
}


// ****************************************************************************
//  Method: avtOpenGLTuvokVolumeRenderer::~avtOpenGLTuvokVolumeRenderer
//
//  Purpose:
//
//  Programmer:  Josh Stratton
//  Creation:    Mon Dec 15 11:58:50 MST 2008
//
//    Thomas R. Treadway, Tue Feb  6 17:04:03 PST 2007
//    The gcc-4.x compiler no longer just warns about automatic type conversion.
//
// ****************************************************************************
avtOpenGLTuvokVolumeRenderer::~avtOpenGLTuvokVolumeRenderer()
{
  /*
    delete[] volumetex;
    volumetex = NULL;

    // Assumes context is current!  This is the job of the container class.
    if (volumetexId != 0)
    {
        glDeleteTextures(1, (GLuint*)&volumetexId);
        volumetexId = 0;
    }
  */
}


// ****************************************************************************
//  Method:  avtOpenGLTuvokVolumeRenderer::Render
//
//  Purpose:
//
//  Arguments:
//    grid      : the data set to render
//    data,opac : the color/opacity variables
//    view      : the viewing information
//    atts      : the current volume plot attributes
//    vmin/max/size : the min/max/range of the color variable
//    omin/max/size : the min/max/range of the opacity variable
//    gx/gy/gz      : the gradient of the opacity variable
//    gmn           : the gradient magnitude, normalized to the max grad mag
//
//  Programmer:  Josh Stratton
//  Creation:    Mon Dec 15 11:58:50 MST 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtOpenGLTuvokVolumeRenderer::Render(vtkRectilinearGrid *grid,
                                         vtkDataArray *data,
                                         vtkDataArray *opac,
                                         const avtViewInfo &view,
                                         const VolumeAttributes &atts,
                                         float vmin, float vmax, float vsize,
                                         float omin, float omax, float osize,
                                         float *gx, float *gy, float *gz,
                                         float *gmn, bool reducedDetail)
{
  cout << "Rendering with Tuvok" << endl;
  /*
    static bool haveIssuedWarning = false;
#ifndef VTK_IMPLEMENT_MESA_CXX
    // OpenGL mode
#if !defined(GL_VERSION_1_2)
#ifdef HAVE_LIBGLEW
    // If we have GLEW then we're in the OpenGL version and we should
    // be sure that the extension exists on the display.
    if(glew_initialized && !GLEW_EXT_texture3D)
    {
#ifdef _WIN32
        // On Windows, glTexImage3D is an OpenGL extension. We have to look
        // up the function pointer.
        if(glTexImage3D_ptr == 0)
            glTexImage3D_ptr = (PFNGLTEXIMAGE3DEXTPROC)wglGetProcAddress("glTexImage3D");
        if(glTexImage3D_ptr == 0)
        {
            debug1 << "The glTexImage3D function was not located." << endl;
            if (!haveIssuedWarning)
            {
                avtCallback::IssueWarning("3D textured volume rendering is not "
                           "available, because the OpenGL functions cannot be "
                           "located.");
                haveIssuedWarning = true;
            }
            return;
        }
#else
        debug1 << "avtOpenGLTuvokVolumeRenderer::Render: "
                  "returning because there is no texture3D extension."
               << endl;
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("3D textured volume rendering is not "
                       "available, because the 3D texturing extensions can "
                       "not be located.");
            haveIssuedWarning = true;
        }
        return;
#endif
    }
#endif
#endif
#endif

    // Get the transfer function
    int ncolors=256;
    int nopacities=256;
    unsigned char rgba[256*4];
    atts.GetTransferFunction(rgba);
    
    // Get the dimensions
    int dims[3];
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
    if (volumetex && (atts != oldAtts || lights != oldLights))
    {
        glDeleteTextures(1, (GLuint*)&volumetexId);
        volumetexId = 0;
        delete[] volumetex;
        volumetex = NULL;
    }
    oldAtts = atts;
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

                    float  v = data->GetTuple1(index);
                    float  o = opac->GetTuple1(index);

                    // drop empty ones
                    if (v < -1e+37)
                        continue;

                    // normalize the value
                    v = (v < vmin ? vmin : v);
                    v = (v > vmax ? vmax : v);
                    v = (v-vmin)/vsize;
                    o = (o < omin ? omin : o);
                    o = (o > omax ? omax : o);
                    o = (o-omin)/osize;

                    // opactity map
                    float opacity;
                    opacity = float(rgba[int(o*(nopacities-1))*4 + 3])*
                        atts.GetOpacityAttenuation()/256.;
                    opacity = MAX(0,MIN(1,opacity));

                    // drop transparent splats 
                    //if (opacity < .0001)
                    //    continue;

                    // do shading
                    float brightness;
                    const bool shading = atts.GetLightingFlag();
                    if (shading)
                    {
                        // Get the gradient
                        float gi = gx[index];
                        float gj = gy[index];
                        float gk = gz[index];

                        // Amount of shading should be somewhat proportional
                        // to the magnitude of the gradient
                        float gm = pow(gmn[index], 0.25f);

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

#ifndef VTK_IMPLEMENT_MESA_CXX
        // OpenGL mode
#ifdef GL_VERSION_1_2
        // OpenGL supports glTexImage3D.
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, newnx, newny, newnz,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, volumetex);
#elif HAVE_LIBGLEW 
        if (GLEW_EXT_texture3D)
        {
            // glTexImage3D via GLEW.
            glTexImage3DEXT(GL_TEXTURE_3D, 0, GL_RGBA, newnx, newny, newnz,
                            0, GL_RGBA, GL_UNSIGNED_BYTE, volumetex);
        }
#ifdef _WIN32
        else if (glTexImage3D_ptr != 0)
        {
            glTexImage3D_ptr(GL_TEXTURE_3D, 0, GL_RGBA, newnx, newny, newnz,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, volumetex);
        }
#endif
#endif
#else
        // Mesa mode
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, newnx, newny, newnz,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, volumetex);
#endif
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
    view.SetCameraFromView(camera);
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
    int ns = atts.GetNum3DSlices();
    if (ns < 1)
        ns = 1;
    if (ns > 500)
        ns = 500;

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
    opac->Delete();
    data->Delete();
    camera->Delete();
  */
}


