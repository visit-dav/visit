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

#include <avtGLEWInitializer.h>  // Make sure GLEW gets included first.
#include "vtkTexturedPointsPainter.h"

#include <vtkObjectFactory.h>
#include <vtkStandardPolyDataPainter.h>
#include <vtkWindow.h>

#define SPHERE_TEX_W 64
#define SPHERE_TEX_H 64

// ****************************************************************************
// Class: vtkTexturedPoints::Texturer
//
// Purpose:
//   This class creates a sphere texture and textures points using that texture
//   and the GL point sprite extension.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 24 12:46:48 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

class vtkTexturedPointsPainter::Texturer
{
public:
    Texturer();
    ~Texturer();

    // Description:
    // Called to set up textures, etc when we want to draw textured points.
    void StartFancyPoints();

    // Description:
    // Called to restore previous OpenGL state after drawing textured points.
    void EndFancyPoints();

    // Description:
    // Called to release textures.
    void ReleaseGraphicsResources(vtkWindow *win);

private:
    void MakeTextures();

    // Description:
    // Whether the texture data has been created.
    bool SphereTexturesDataCreated;

    // Description:
    // Whether the texture data has been loaded.
    bool SphereTexturesLoaded;

    // Description:
    // Contains the sphere texture that we use
    unsigned char SphereTexture[SPHERE_TEX_H][SPHERE_TEX_W][2];

    // Description:
    // Contains the name of the texture.
    unsigned int  TextureName;
};

//-----------------------------------------------------------------------------

vtkStandardNewMacro(vtkTexturedPointsPainter);

//-----------------------------------------------------------------------------
vtkTexturedPointsPainter::vtkTexturedPointsPainter() : vtkPointsPainter()
{
    this->DoTexturing = 0;
    this->tex = new Texturer;

    vtkStandardPolyDataPainter *sp = vtkStandardPolyDataPainter::New();
    this->SetDelegatePainter(sp);
    sp->Delete();
}

//-----------------------------------------------------------------------------
vtkTexturedPointsPainter::~vtkTexturedPointsPainter()
{
    delete tex;
}

void
vtkTexturedPointsPainter::ReleaseGraphicsResources(vtkWindow *w)
{
    this->Superclass::ReleaseGraphicsResources(w);

    this->tex->ReleaseGraphicsResources(w);
}

// ****************************************************************************
// Method: vtkTexturedPointsPainter::Render
//
// Purpose: 
//   Render points but turn on point sprite texturing before and after.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 24 17:00:01 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void vtkTexturedPointsPainter::Render(vtkRenderer* renderer, vtkActor* actor, 
    unsigned long typeflags, bool forceCompileOnly)
{
    if(this->DoTexturing)
        this->tex->StartFancyPoints();

    this->vtkPointsPainter::Render(renderer, actor, typeflags, forceCompileOnly);

    if(this->DoTexturing)
        this->tex->EndFancyPoints();
}

//-----------------------------------------------------------------------------
void vtkTexturedPointsPainter::PrintSelf(ostream& os ,vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
    os << indent << "DoTexturing: " << this->DoTexturing << "\n";
}

//-----------------------------------------------------------------------------

// ****************************************************************************
// Method: vtkTexturedPointsPainter::Texturer::Texturer
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 24 17:00:53 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

vtkTexturedPointsPainter::Texturer::Texturer()
{
    this->SphereTexturesDataCreated = false;
    this->SphereTexturesLoaded = false;
    this->TextureName = 0;
}
// ****************************************************************************
// Method: vtkTexturedPointsPainter::Texturer::~Texturer
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 24 17:00:53 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

vtkTexturedPointsPainter::Texturer::~Texturer()
{
}

// ****************************************************************************
// Method: vtkTexturedPointsPainter::Texturer::ReleaseGraphicsResources
//
// Purpose: 
//   Release the texture.
//
// Arguments:
//   win : The window that owns the context.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 24 17:01:12 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
vtkTexturedPointsPainter::Texturer::ReleaseGraphicsResources(vtkWindow *win)
{
    // Free the textures if they have been loaded.
    if (this->SphereTexturesLoaded)
    {
        win->MakeCurrent();
        glDeleteTextures(1, (GLuint*)&this->TextureName);
        this->SphereTexturesLoaded = false;
    }
}

// ****************************************************************************
// Method:     vtkTexturedPointsPainter::Texturer::StartFancyPoints
//
// Purpose: 
//   Sets up point texturing.
//
// Arguments:
//   atts : The GL state before setting up point texturing.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 25 14:58:09 PST 2005
//
// Modifications:
//   Brad Whitlock, Thu Nov 3 13:21:08 PST 2005
//   Added conditional compilation.
//
//   Brad Whitlock, Tue Dec 6 13:37:58 PST 2005
//   Changed to 1-pass texturing.
//
//   Thomas R. Treadway, Tue Feb  6 17:04:03 PST 2007
//   The gcc-4.x compiler no longer just warns about automatic type conversion.
//
//   Tom Fogal, Tue Apr 27 11:23:40 MDT 2010
//   Simplify point sprite detection and note when we can't use it.
//
// ****************************************************************************

void
vtkTexturedPointsPainter::Texturer::StartFancyPoints()
{
    if(!GLEW_ARB_point_sprite)
    {
        // Point sprites are not supported
        return; 
    }

    // Create the textures
    if(!this->SphereTexturesDataCreated)
    {
        this->MakeTextures();
        this->SphereTexturesDataCreated = true;
    }

    // Push color and texture attributes so we can restore them later.
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT);

    // Create and bind the textures if we have not done that yet.
    if(!this->SphereTexturesLoaded)
    {
        glGenTextures(1, (GLuint*)&this->TextureName);

        // Set up the first texture
        glBindTexture(GL_TEXTURE_2D, this->TextureName);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, 2, SPHERE_TEX_W, SPHERE_TEX_H,
                     0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, this->SphereTexture);

        this->SphereTexturesLoaded = true;
    }

    //
    // Turn on alpha blending.
    //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //
    // Turn on alpha testing.
    //
    int dt = 0;
    glGetIntegerv(GL_DEPTH_TEST, (GLint*)&dt);
    if(dt == 1)
    {
        // Set the alpha testing mode and function.
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, 0.7);
    }

    //
    // Turn on the point sprite extension
    //
#define MY_POINT_SPRITE_ARB  0x8861
#define MY_COORD_REPLACE_ARB 0x8862
    glEnable(MY_POINT_SPRITE_ARB);

    //
    // Turn on the texture
    //
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, this->TextureName);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    glTexEnvi(MY_POINT_SPRITE_ARB,
              MY_COORD_REPLACE_ARB,
              GL_TRUE);
}

// ****************************************************************************
// Method: vtkTexturedPointsPainter::Texturer::EndFancyPoints
//
// Purpose: 
//   Turns off point texturing.
//
// Arguments:
//   atts : The previous GL state to be restored.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 25 14:58:49 PST 2005
//
// Modifications:
//   Brad Whitlock, Tue Dec 6 13:39:33 PST 2005
//   I changed it to 1-pass texturing.
//
// ****************************************************************************

void
vtkTexturedPointsPainter::Texturer::EndFancyPoints()
{
    if(!GLEW_ARB_point_sprite)
    {
        // Point sprites are not supported
        return; 
    }

    // Restore the state we had before.
    glPopAttrib();
    glDisable(MY_POINT_SPRITE_ARB);
}

// ****************************************************************************
// Method: vtkTexturedPointsPainter::Texturer::MakeTextures
//
// Purpose: 
//   Calculates a simple, transparent shaded sphere texture and a mask texture.
//
// Note:       We're not using a compiled-in data array in case we later want
//             to do lighting, etc to make the texture better reflect the
//             environment.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 25 14:59:22 PST 2005
//
// Modifications:
//   Brad Whitlock, Mon Aug 29 10:04:32 PDT 2005
//   Fixed it so the first texture's alpha is based on the Z value as computed
//   for a sphere. The last equation was a kludge.
//
//   Brad Whitlock, Tue Dec 6 13:41:18 PST 2005
//   I changed it to 1-pass texturing.
//
//   Hank Childs, Fri Jun  9 13:13:20 PDT 2006
//   Remove unused variable.
//
// ****************************************************************************

void
vtkTexturedPointsPainter::Texturer::MakeTextures()
{
   int i, j;

   /* float dx = SPHERE_TEX_H * 0.5f; */
   /* float dy = SPHERE_TEX_H * 0.5f; */
   /* float R = dx-2; */

   /* float minT = 0.; */
   /* float maxT = 0.; */

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

             texture[j][i][0] = (GLubyte) 255 - rc;
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
                   this->SphereTexture[j][i][c] = (GLubyte)t;
               }
           }
           else
           {
               this->SphereTexture[j][i][0] = (GLubyte)0;
               this->SphereTexture[j][i][1] = (GLubyte)0;
           }
       }
   }
}
