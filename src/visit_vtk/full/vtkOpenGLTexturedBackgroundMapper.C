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
#include "vtkOpenGLTexturedBackgroundMapper.h"
#include <vtkCamera.h>
#include <vtkTexture.h>
#include <vtkViewport.h>
#include <vtkWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkMatrix4x4.h>

#include <math.h>

#ifndef VTK_IMPLEMENT_MESA_CXX
#if defined(__APPLE__) && (defined(VTK_USE_CARBON) || defined(VTK_USE_COCOA))
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#else
#include <GL/gl.h>
#endif

// ****************************************************************************
// Method: vtkOpenGLTexturedBackgroundMapper::vtkOpenGLTexturedBackgroundMapper
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 16 10:15:24 PST 2007
//
// Modifications:
//   
// ****************************************************************************

vtkOpenGLTexturedBackgroundMapper::vtkOpenGLTexturedBackgroundMapper()
{
    tex = NULL;
    background = NULL;
    canvas = NULL;
    sphereMode = false;
    imageRepeatX = 1;
    imageRepeatY = 1;
}

// ****************************************************************************
// Method: vtkOpenGLTexturedBackgroundMapper::~vtkOpenGLTexturedBackgroundMapper
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 16 10:15:27 PST 2007
//
// Modifications:
//   
// ****************************************************************************

vtkOpenGLTexturedBackgroundMapper::~vtkOpenGLTexturedBackgroundMapper()
{
}

// ****************************************************************************
// Method: vtkOpenGLTexturedBackgroundMapper::New
//
// Purpose: 
//   Creates an instance of vtkOpenGLTexturedBackgroundMapper.
//
// Arguments:
//
// Returns:    A new instance of vtkOpenGLTexturedBackgroundMapper.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 16 10:15:53 PST 2007
//
// Modifications:
//   
// ****************************************************************************

vtkOpenGLTexturedBackgroundMapper *
vtkOpenGLTexturedBackgroundMapper::New()
{
    return new vtkOpenGLTexturedBackgroundMapper;
}

// ****************************************************************************
// Method: vtkOpenGLTexturedBackgroundMapper::PrintSelf
//
// Purpose: 
//   Prints the mapper data.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 16 10:16:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
vtkOpenGLTexturedBackgroundMapper::PrintSelf(ostream &os, vtkIndent indent)
{
    // Do nothing...
}

// ****************************************************************************
// Method: vtkOpenGLTexturedBackgroundMapper::SetTextureAndRenderer
//
// Purpose: 
//   Sets the texture and renderer pointers that we need before the mapper
//   can really do anything.
//
// Arguments:
//   t : The texture to use for the background.
//   r : The renderer to use for the background. We use a renderer as opposed to
//       a vtkViewport because the vtkTexture class wants a vtkRenderer for its
//       Render method.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 16 10:16:55 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
vtkOpenGLTexturedBackgroundMapper::SetTextureAndRenderers(vtkTexture *t, vtkRenderer *bg,
    vtkRenderer *c)
{
    tex = t;
    background = bg;
    canvas = c;
}

// ****************************************************************************
// Method: vtkOpenGLTexturedBackgroundMapper::SetSphereMode
//
// Purpose: 
//   Sets whether the mapper will draw a sky sphere or a flat image.
//
// Arguments:
//   mode : True for sphere; False for flat.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 19 12:40:35 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
vtkOpenGLTexturedBackgroundMapper::SetSphereMode(bool mode)
{
    sphereMode = mode;
}

// ****************************************************************************
// Method: vtkOpenGLTexturedBackgroundMapper::SetImageRepetitions
//
// Purpose: 
//   Set the number of image repetitions in x,y.
//
// Arguments:
//   nx : The number of image repetitions in x.
//   ny : The number of image repetitions in y.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 19 12:41:09 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
vtkOpenGLTexturedBackgroundMapper::SetImageRepetitions(int nx, int ny)
{
    imageRepeatX = (nx >= 1) ? nx : imageRepeatX;
    imageRepeatY = (ny >= 1) ? ny : imageRepeatY;
}

// ****************************************************************************
// Method: DrawSphere
//
// Purpose: 
//   Draws a textured sphere.
//
// Arguments:
//
// Returns:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 18 14:49:05 PST 2007
//
// Modifications:
//
// ****************************************************************************

void
vtkOpenGLTexturedBackgroundMapper::DrawSphere(int X_RES, int Y_RES, float radius,
    const double *fc)
{
    int npts = (X_RES * (Y_RES-1) + 2);
    int ncells = X_RES * Y_RES;
    float *coords = new float[npts * 3];
    float *tex_coords = new float[npts * 3];
    float *fptr = coords;
    float *tptr = tex_coords;
    float tex_offsets[3] = {0.f, 0.f, 0.f};
    float focalPoint[3];
    focalPoint[0] = (float)fc[0];
    focalPoint[1] = (float)fc[1];
    focalPoint[2] = (float)fc[2];

    for(int ip = 0; ip < Y_RES+1; ++ip)
    {
        if(ip == 0)
        {
            *fptr++ = focalPoint[0] + 0.f;
            *fptr++ = focalPoint[1] + -radius;
            *fptr++ = focalPoint[2] + 0.f;

            *tptr++ = 0.5f;
            *tptr++ = 0.f;
            *tptr++ = 0.f;
        }
        else if(ip == Y_RES)
        {
            *fptr++ = focalPoint[0] + 0.f;
            *fptr++ = focalPoint[1] + radius;
            *fptr++ = focalPoint[2] + 0.f;

            *tptr++ = 0.5f;
            *tptr++ = 1.f;
            *tptr++ = 0.f;
        }
        else
        {
            float texcoord_y = float(ip) / float(Y_RES-1);
            float poleAngle = texcoord_y * M_PI;
            float sign = (poleAngle > M_PI_2) ? -1. : 1;
            float y = radius * cos(poleAngle) * -1;
            float yrad = radius * sin(poleAngle);

            for(int ri = 0; ri < X_RES; ++ri)
            {
                float texcoord_x = float(ri) / float(X_RES-1);
                float angle = texcoord_x * -2. * M_PI;
                float x = yrad * cos(angle + M_PI/2.);
                float z = yrad * sin(angle + M_PI/2.);    

                *fptr++ = focalPoint[0] + x;
                *fptr++ = focalPoint[1] + y;
                *fptr++ = focalPoint[2] + z;

                *tptr++ = 1. - (texcoord_x + tex_offsets[0]);
                *tptr++ = texcoord_y + tex_offsets[1];
                *tptr++ = tex_offsets[2];
            }
        }
    }

    int verts[8];
    int i, row, nextrow;

    // Bottom triangles 
    glBegin(GL_TRIANGLES);
    for(i = 0; i < X_RES; ++i)
    {
        if(i < X_RES-1)
        {
            verts[0] = 0;
            verts[1] = i+2;
            verts[2] = i+1;
        }
        else
        {
            verts[0] = 0;
            verts[1] = 1;
            verts[2] = i+1;
        }

        for(int v = 0; v < 3; ++v)
        {
            int index = verts[v] * 3;
            glTexCoord2fv(&tex_coords[index]);
            glNormal3fv(&coords[index]);
            glVertex3fv(&coords[index]);
        }
    }

    // Top triangles.
    row = 1;
    for(int j = 0; j < Y_RES - 2; ++j)
        row += X_RES;
    int last = X_RES * (Y_RES-1) + 1;
    for(i = 0; i < X_RES; ++i)
    {
        if(i < X_RES-1)
        {
            verts[0] = row + i;
            verts[1] = row + i + 1;
            verts[2] = last;
        }
        else
        {
            verts[0] = row + i;
            verts[1] = row;
            verts[2] = last;
        }

        for(int v = 0; v < 3; ++v)
        {
            int index = verts[v] * 3;
            glTexCoord2fv(&tex_coords[index]);
            glNormal3fv(&coords[index]);
            glVertex3fv(&coords[index]);
        }
    }
    glEnd();

    // Quads for most of the globe.
    glBegin(GL_QUADS);
    row = 1;
    nextrow = X_RES + 1;
    for(int j = 0; j < Y_RES - 2; ++j)
    {
        for(i = 0; i < X_RES; ++i)
        {
            if(i < X_RES-1)
            {
                verts[0] = row + i;
                verts[1] = row + i + 1;
                verts[2] = nextrow + i + 1;
                verts[3] = nextrow + i;
            }
            else
            {
                verts[0] = row + i;
                verts[1] = row;
                verts[2] = nextrow;
                verts[3] = nextrow + i;
            }

            for(int v = 0; v < 4; ++v)
            {
                int index = verts[v] * 3;
                glTexCoord2fv(&tex_coords[index]);
                glNormal3fv(&coords[index]);
                glVertex3fv(&coords[index]);
            }
        }
    
        row += X_RES;
        nextrow += X_RES;
    }
    glEnd();

    delete [] coords;
    delete [] tex_coords;
}

// ****************************************************************************
// Method: vtkOpenGLTexturedBackgroundMapper::DrawImageSphere
//
// Purpose: 
//   Draw the image background as a sky sphere.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 19 12:43:57 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
vtkOpenGLTexturedBackgroundMapper::DrawImageSphere()
{
    // Now, load the texture...
    tex->Render(background);

    // Disable depth test...
    glDepthMask(0);
    // Query and disable lighting.
    GLboolean lighting;
    glGetBooleanv(GL_LIGHTING, &lighting);
    if(lighting)
        glDisable(GL_LIGHTING);

    // Get the matrix mode so we can restore it later.
    GLint mm;
    GLboolean clipped = 0;
    glGetIntegerv(GL_MATRIX_MODE, &mm);

    // Set up the camera using the canvas renderer's camera.
    if(canvas != 0)
    {
        //
        // Set up the modelview matrix
        //
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
        matrix->DeepCopy(canvas->GetActiveCamera()->GetViewTransformMatrix());
#ifdef DEBUG_PRINT
cout << "+++++++++++++++++++++++ Begin ++++++++++++++++++++++++++" << endl;
cout << "Modelview = ";
matrix->Print(cout);
#endif
        matrix->Transpose();
        glMultMatrixd(matrix->Element[0]);
        matrix->Delete();

        //
        // Set up the projection matrix
        //
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();

#ifdef DEBUG_PRINT
canvas->GetActiveCamera()->Print(cout);
#endif
        // Force the focal disk to 1. to prevent zooming.
        double oldFocalDisk = canvas->GetActiveCamera()->GetFocalDisk();
        canvas->GetActiveCamera()->SetFocalDisk(1.);
        // Force the window center to 0,0 to prevent panning.
        double oldWindowCenter[2];
        canvas->GetActiveCamera()->GetWindowCenter(oldWindowCenter);
        canvas->GetActiveCamera()->SetWindowCenter(0., 0.);

        // Set the projection matrix 
        double aspect[2];
        canvas->ComputeAspect();
        canvas->GetAspect(aspect);
        double aspect2[2];
        canvas->vtkViewport::ComputeAspect();
        canvas->vtkViewport::GetAspect(aspect2);
        double aspectModification = aspect[0]*aspect2[1]/(aspect[1]*aspect2[0]);
        int  lowerLeft[2];
        int usize, vsize;
        matrix = vtkMatrix4x4::New();
        matrix->Identity();
        canvas->GetTiledSizeAndOrigin(&usize,&vsize,lowerLeft,lowerLeft+1);
        if(usize && vsize)
        {
            matrix->DeepCopy(canvas->GetActiveCamera()->GetPerspectiveTransformMatrix(
                aspectModification*usize/vsize, -1,1));
#ifdef DEBUG_PRINT
cout << "Projection = ";
matrix->Print(cout);
#endif
            matrix->Transpose();
        }
        glLoadMatrixd(matrix->Element[0]);
        matrix->Delete();

        // Restore the focal disk and window center values
        canvas->GetActiveCamera()->SetFocalDisk(oldFocalDisk);
        canvas->GetActiveCamera()->SetWindowCenter(oldWindowCenter[0], oldWindowCenter[1]);

        // Construct a clipping plane through the focal point that matches 
        // up with the view normal. This clips off the front of the sky
        // sphere that we're drawing.
        glGetBooleanv(GL_CLIP_PLANE5, &clipped);
        double plane[4];
        double *N = canvas->GetActiveCamera()->GetViewPlaneNormal();
        plane[0] = -N[0];
        plane[1] = -N[1];
        plane[2] = -N[2];
        const double *fc = canvas->GetActiveCamera()->GetFocalPoint();
        plane[3] = (fc[0]*N[0] + fc[1]*N[1] + fc[2]*N[2]);
        glClipPlane(GL_CLIP_PLANE5, plane);
        glEnable(GL_CLIP_PLANE5);
    }

    // Repeat the textures some number of times.
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glScalef((float)imageRepeatX, (float)imageRepeatY, 1.);

    // Compute the bounding box of the items in the canvas renderer so we can
    // make an appropriate sized sphere.
    double bounds[6];
    canvas->ComputeVisiblePropBounds(bounds);
    double dX = bounds[1] - bounds[0];
    double dY = bounds[3] - bounds[2];
    double dZ = bounds[5] - bounds[4];
    double bboxRad = sqrt(dX*dX + dY*dY + dZ*dZ);
    double sphereRad = bboxRad * 0.95;

    // Draw the sky sphere.
    const double *fc = canvas->GetActiveCamera()->GetFocalPoint();
    DrawSphere(30, 30, sphereRad, fc);

#ifdef DEBUG_PRINT
cout << "sphereRad = " << sphereRad << endl;
cout << "+++++++++++++++++++++++ End ++++++++++++++++++++++++++" << endl;
#endif
    // Restore the previous texture matrix.
    glPopMatrix();

    // Disable texturing
    glDisable(GL_TEXTURE_2D);

    // Restore matrices
    if(canvas != 0)
    {
        // Restore plane 5 eq.

        if(!clipped)
            glDisable(GL_CLIP_PLANE5);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(mm);
    }
    glMatrixMode(mm);

    // Restore depth test.
    glDepthMask(1);
    // Restore lighting if needed
    if(lighting)
        glEnable(GL_LIGHTING);
}

// ****************************************************************************
// Method: vtkOpenGLTexturedBackgroundMapper::DrawImageFlat
//
// Purpose: 
//   Draws the image flat in the window.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 19 12:45:23 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
vtkOpenGLTexturedBackgroundMapper::DrawImageFlat()
{
    // Now, load the texture...
    tex->Render(background);

    // Save matrices and load identity matrices.
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();

    // Repeat the textures some number of times.
    glScalef((float)imageRepeatX, (float)imageRepeatY, 1.);

    // Disable depth test...
    glDepthMask(0);
    glDisable(GL_LIGHTING);

    // Draw a simple background made of 2 triangles.
    glBegin(GL_TRIANGLES);
    const float c[][3] = {{-1.,-1.,0.}, {1.,-1.,0.}, {1.,1.,0.}, {-1.,1.,0.}};
    const float tc[][2] = {{0.,0.}, {1.,0.}, {1.,1.}, {0.,1.}};
    const float white[] = {1.,1.,1.};
    const int tri[] = {0,1,2,0,2,3};
    glBegin(GL_TRIANGLES);
    for(int i = 0; i < 6; ++i)
    {
        glColor3fv(white);
        glTexCoord2fv(tc[tri[i]]);
        glVertex3fv(c[tri[i]]);
    }
    glEnd();

    // Disable textures.
    glDisable(GL_TEXTURE_2D);

    // Restore depth test.
    glDepthMask(1);
    glEnable(GL_LIGHTING);

    // Restore matrices
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ****************************************************************************
// Method: vtkOpenGLTexturedBackgroundMapper::RenderOpaqueGeometry
//
// Purpose: 
//   Renders the texture to the background of the window.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 16 10:18:55 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
vtkOpenGLTexturedBackgroundMapper::RenderOpaqueGeometry(vtkViewport *, 
    vtkActor2D *)
{
    if(tex == NULL || background == NULL || canvas == NULL)
        return;

    if(sphereMode)
        DrawImageSphere();
    else
        DrawImageFlat();
}
