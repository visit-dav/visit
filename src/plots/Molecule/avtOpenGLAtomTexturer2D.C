/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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

#include "avtOpenGLAtomTexturer2D.h"
#include <vtkConfigure.h>
#include <math.h>
#include <DebugStream.h>

#include <cstring>

//
// If we're creating the OpenGL version of this class then check to see if
// the GLEW library is available so we can use shaders.
//

#ifndef VTK_IMPLEMENT_MESA_CXX
  #include <visit-config.h>
  #include <avtGLEWInitializer.h>
  #if defined(__APPLE__) && (defined(VTK_USE_CARBON) || defined(VTK_USE_COCOA))
    #include <OpenGL/gl.h>
  #else
    #if defined(_WIN32)
       #include <windows.h>
    #endif
    #include <GL/gl.h>
  #endif
#endif

#define SPHERE_TEX_W 256
#define SPHERE_TEX_H 256

// ****************************************************************************
// Method: avtOpenGLAtomTexturer2D::avtOpenGLAtomTexturer2D
//
// Purpose: Constructor
//   
// Programmer: Brad Whitlock
// Creation:   Mon Mar 27 17:22:27 PST 2006
//
// Modifications:
//   Jeremy Meredith, Thu Apr 22 14:09:38 EDT 2010
//   Derived from 3D atom texturer.
//   
// ****************************************************************************

avtOpenGLAtomTexturer2D::avtOpenGLAtomTexturer2D()
{
    sphereTexturesDataCreated = false;
    sphereTexturesLoaded = false;
    textureName = 0;

    isBlendEnabled = 0;
    blendFunc0 = 0;
    blendFunc1 = 0;
    needAlphaTest = 0;
    isAlphaTestEnabled = 0;
    alphaTestFunc = 0;
    alphaTestRef = 0.;
}

// ****************************************************************************
// Method: avtOpenGLAtomTexturer2D::~avtOpenGLAtomTexturer2D
//
// Purpose: Destructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 27 17:22:46 PST 2006
//
// Modifications:
//   Jeremy Meredith, Thu Apr 22 14:09:38 EDT 2010
//   Derived from 3D atom texturer.
//   
// ****************************************************************************

avtOpenGLAtomTexturer2D::~avtOpenGLAtomTexturer2D()
{
    if(sphereTexturesLoaded)
        glDeleteTextures(1, &textureName);
}

// ****************************************************************************
// Method: avtOpenGLAtomTexturer2D::BeginSphereTexturing
//
// Purpose: 
//   Sets up a sphere texture that we will apply to all of the imposter quads.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 27 17:18:28 PST 2006
//
// Modifications:
//   Jeremy Meredith, Thu Apr 22 14:09:38 EDT 2010
//   Derived from 3D atom texturer.
//   
// ****************************************************************************

bool
avtOpenGLAtomTexturer2D::BeginSphereTexturing()
{
    // Create the rextures
    if(!sphereTexturesDataCreated)
    {
        MakeTextures();
        sphereTexturesDataCreated = true;
    }

    // Create and bind the textures if we have not done that yet.
    if(!sphereTexturesLoaded)
    {
        glGenTextures(1, &textureName);

        // Set up the first texture
        glBindTexture(GL_TEXTURE_2D, textureName);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, 2, SPHERE_TEX_W, SPHERE_TEX_H,
                     0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, sphereTexture);

        sphereTexturesLoaded = true;
    }


    //
    // Get whether GL_BLEND is enabled.
    //
    glGetIntegerv(GL_BLEND, &isBlendEnabled);
    if(isBlendEnabled == 0)
        glEnable(GL_BLEND);

    glGetIntegerv(GL_BLEND_SRC, &blendFunc0);
    glGetIntegerv(GL_BLEND_DST, &blendFunc1);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //
    // Get the AlphaTest mode to restore it later.
    //
    GLint dt = 0;
    glGetIntegerv(GL_DEPTH_TEST, &dt);
    if(dt == 1)
    {
        // Get the current alpha test function
        glGetIntegerv(GL_ALPHA_TEST, &isAlphaTestEnabled);
        if(isAlphaTestEnabled)
        {
            glGetIntegerv(GL_ALPHA_TEST_FUNC, &alphaTestFunc);
            glGetFloatv(GL_ALPHA_TEST_REF, &alphaTestRef);
        }
        else
            glEnable(GL_ALPHA_TEST);

        // Set the alpha testing mode and function.
        glAlphaFunc(GL_GREATER, 0.7);

        needAlphaTest = 1;
    }
    else
        needAlphaTest = 0;

    //
    // Turn on the texture
    //
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureName);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

    return true;
}

// ****************************************************************************
// Method: avtOpenGLAtomTexturer2D::EndSphereTexturing
//
// Purpose: 
//   Disables texuring for the imposter quads.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 27 17:19:08 PST 2006
//
// Modifications:
//   Jeremy Meredith, Thu Apr 22 14:09:38 EDT 2010
//   Derived from 3D atom texturer.
//   
// ****************************************************************************

void
avtOpenGLAtomTexturer2D::EndSphereTexturing()
{
    if(needAlphaTest)
    {
        if(isAlphaTestEnabled)
            glAlphaFunc(alphaTestFunc, alphaTestRef);
        else
            glDisable(GL_ALPHA_TEST);
    }

    if(isBlendEnabled == 0)
        glDisable(GL_BLEND);

    // Restore the old blend function.
    glBlendFunc(blendFunc0, blendFunc1);

    glDisable(GL_TEXTURE_2D);
}

// ****************************************************************************
// Method: avtOpenGLAtomTexturer2D::MakeTextures
//
// Purpose: 
//   Makes a sphere texture that we'll apply to imposter quads.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 27 17:19:31 PST 2006
//
// Modifications:
//   Jeremy Meredith, Thu Apr 22 14:09:38 EDT 2010
//   Derived from 3D atom texturer.
//   Converted to a flat circle for 2D use.
//   
// ****************************************************************************

void
avtOpenGLAtomTexturer2D::MakeTextures()
{
   int i, j;

   float dx = SPHERE_TEX_H * 0.5f;
   float dy = SPHERE_TEX_H * 0.5f;
   float R = dx-2;

   float minT = 0.;
   float maxT = 0.;

   GLubyte texture[SPHERE_TEX_H][SPHERE_TEX_H][2];

   for (j = 0; j < SPHERE_TEX_H; j++)
   {
      float y = (float(j) / float(SPHERE_TEX_H-1)) * 2. - 1.;
      for (i = 0; i < SPHERE_TEX_W; i++)
      {
         float x = (float(i) / float(SPHERE_TEX_W-1)) * 2. - 1.;
         float x2y2 = sqrt(x*x + y*y);
         if(x2y2 < 1.)
         {
             float z = sqrt(1. - x2y2);
             GLubyte rc = (GLubyte)(z * 255.);

             texture[j][i][0] = (GLubyte) 0;
             texture[j][i][1] = (GLubyte) 255;
         }
         else
         {
             texture[j][i][0] = (GLubyte) 0;
             texture[j][i][1] = (GLubyte) 0;
         }
      }
   }

   //
   // Blur the texture a little
   //
   float kernel[3][3] = {
       {0.125, 0.2, 0.125},
       {0.200, 1.0, 0.200},
       {0.125, 0.2, 0.125}};
   float kernelSum = kernel[0][0] + kernel[0][1] + kernel[0][2] + 
                     kernel[1][0] + kernel[1][1] + kernel[1][2] + 
                     kernel[2][0] + kernel[2][1] + kernel[2][2];
   for (j = 0; j < SPHERE_TEX_H; j++)
   {
       for (i = 0; i < SPHERE_TEX_W; i++)
       {
           if(i >= 1 && i < SPHERE_TEX_W-1 &&
              j >= 1 && j < SPHERE_TEX_H-1)
           {
               for(int c = 0; c < 2; ++c)
               {
                   float t = kernel[0][0] * float(texture[j-1][i-1][c]) + 
                             kernel[0][1] * float(texture[j-1][i][c]) + 
                             kernel[0][2] * float(texture[j-1][i+1][c]) + 
                             kernel[1][0] * float(texture[j][i-1][c]) + 
                             kernel[1][1] * float(texture[j][i][c]) + 
                             kernel[1][2] * float(texture[j][i+1][c]) + 
                             kernel[2][0] * float(texture[j+1][i-1][c]) + 
                             kernel[2][1] * float(texture[j+1][i][c]) + 
                             kernel[2][2] * float(texture[j+1][i+1][c]);
                   t /= kernelSum;
                   sphereTexture[j][i][c] = (GLubyte)t;
               }
           }
           else
           {
               sphereTexture[j][i][0] = (GLubyte)0;
               sphereTexture[j][i][1] = (GLubyte)0;
           }
       }
   }
}


