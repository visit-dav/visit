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
#include <vtkTexturedBackgroundActor.h>
#include <vtkGraphicsFactory.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkRenderer.h>
#include <vtkTexture.h>

#include "vtkOpenGLTexturedBackgroundMapper.h"
#include "vtkMesaTexturedBackgroundMapper.h"

// ****************************************************************************
// Method: vtkTexturedBackgroundActor::vtkTexturedBackgroundActor
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 16 10:22:36 PST 2007
//
// Modifications:
//   
// ****************************************************************************

vtkTexturedBackgroundActor::vtkTexturedBackgroundActor()
{
    texture = 0;
    imageFile = 0;
}

// ****************************************************************************
// Method: vtkTexturedBackgroundActor::~vtkTexturedBackgroundActor
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 16 10:22:49 PST 2007
//
// Modifications:
//   
// ****************************************************************************

vtkTexturedBackgroundActor::~vtkTexturedBackgroundActor()
{
    // This object owns its mapper so delete the mapper.
    if(GetMapper() != NULL)
        GetMapper()->Delete();

    if(texture != 0)
    {
        texture->Delete();
        texture = 0;
    }

    if(imageFile != 0)
        delete [] imageFile;
}

// ****************************************************************************
// Method: vtkTexturedBackgroundActor::ReleaseGraphicsResources
//
// Purpose: 
//   Release the texture's resources.
//
// Arguments:
//   win : The VTK window that uses the texture.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 19 16:33:33 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
vtkTexturedBackgroundActor::ReleaseGraphicsResources(vtkWindow *win)
{
    if(texture != 0)
        texture->ReleaseGraphicsResources(win);
}

// ****************************************************************************
// Method: vtkTexturedBackgroundActor::New
//
// Purpose: 
//   Creates a new instance of vtkTexturedBackgroundActor
//
// Arguments:
//
// Returns:    a new instance of vtkTexturedBackgroundActor
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 16 10:23:46 PST 2007
//
// Modifications:
//   
// ****************************************************************************

vtkTexturedBackgroundActor *
vtkTexturedBackgroundActor::New()
{
    return new vtkTexturedBackgroundActor;
}

// ****************************************************************************
// Method: vtkTexturedBackgroundActor::PrintSelf
//
// Purpose: 
//   Prints self.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 16 10:24:09 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
vtkTexturedBackgroundActor::PrintSelf(ostream &os, vtkIndent indent)
{
}

// ****************************************************************************
// Method: vtkTexturedBackgroundActor::GetInitializedMapper
//
// Purpose: 
//   Creates a mapper if necessary and returns it.
//
// Arguments:
//
// Returns:    The mapper.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 19 12:32:32 PST 2007
//
// Modifications:
//   
// ****************************************************************************

vtkMapper2D *
vtkTexturedBackgroundActor::GetInitializedMapper()
{
    if(GetMapper() == NULL)
    {
        if(vtkGraphicsFactory::GetUseMesaClasses())
        {
            vtkMesaTexturedBackgroundMapper *m = 
                vtkMesaTexturedBackgroundMapper::New();
            this->SetMapper(m);
        }
        else
        {  
            vtkOpenGLTexturedBackgroundMapper *m = 
                vtkOpenGLTexturedBackgroundMapper::New();
            this->SetMapper(m);
        }
    }
    return GetMapper();
}

// ****************************************************************************
// Method: vtkTexturedBackgroundActor::GetTexture
//
// Purpose: 
//   Gets the texture for the given filename.
//
// Arguments:
//   newFile : The name of the new texture file.
//
// Returns:    A vtkTexture object containing the texture data from the file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 19 16:50:28 PST 2007
//
// Modifications:
//   Kathleen Bonnell, Thu Mar  6 10:07:53 PST 2008
//   Return early if we don't have a new imagefile.
//   
// ****************************************************************************

vtkTexture *
vtkTexturedBackgroundActor::GetTexture(const char *newFile)
{
    if (newFile == 0 || strlen(newFile) == 0)
        return NULL;

    vtkTexture *tex = 0;
    bool replace = false;
    if(imageFile != 0)
    {
        if(strcmp(imageFile, newFile) != 0)
        {
            delete [] imageFile;
            replace = true;
        }
        else
            replace = (texture == 0);
    }
    else
        replace = true;

    if(replace)
    {
        // Save off the new filename.
        imageFile = new char[strlen(newFile)+1];
        strcpy(imageFile, newFile);

        // Try to read the file using different readers...
        vtkImageReader2 *r =
            vtkImageReader2Factory::CreateImageReader2(imageFile);

        // If we got a valid reader:
        if(r != 0 && r->CanReadFile(imageFile) == 3)
        {
            // Read the image as a texture.
            r->SetFileName(imageFile);
            r->Update();

            tex = vtkTexture::New();
            tex->SetInputConnection(r->GetOutputPort());
            tex->InterpolateOn();
            tex->Update();
        }

        if(r != 0)
            r->Delete();
    }
    else
    {
        // Return the existing texture ptr.
        tex = texture;
    }

    return tex;
}

// ****************************************************************************
// Method: vtkTexturedBackgroundActor::SetTextureAndRenderer
//
// Purpose: 
//   Sets the texture and renderer that will be used for drawing the actor.
//
// Arguments:
//   imgFile : The name of the texture file to use.
//   bg      : The renderer to use for drawing the actor.
//   canvas  : The renderer to use for the camera.
//
// Returns:    -1 on failure, 0 otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 16 10:25:01 PST 2007
//
// Modifications:
//   
// ****************************************************************************

int
vtkTexturedBackgroundActor::SetTextureAndRenderers(const char *imgFile, 
    vtkRenderer *bg, vtkRenderer *canvas)
{
    // Get the new texture.
    vtkTexture *tex = GetTexture(imgFile);
    // If the texture pointers are different then assign the new texture.
    if(tex != texture)
    {
        // Delete the old texture
        if(texture != 0)
            texture->Delete();
        texture = tex;
    }

    if(vtkGraphicsFactory::GetUseMesaClasses())
    {
        vtkMesaTexturedBackgroundMapper *m = 
            (vtkMesaTexturedBackgroundMapper *)GetInitializedMapper();
        m->SetTextureAndRenderers(texture, bg, canvas);
    }
    else
    {  
        vtkOpenGLTexturedBackgroundMapper *m = 
            (vtkOpenGLTexturedBackgroundMapper *)GetInitializedMapper();
        m->SetTextureAndRenderers(texture, bg, canvas);
    }

    return (texture == 0) ? -1 : 0;
}

// ****************************************************************************
// Method: vtkTexturedBackgroundActor::SetSphereMode
//
// Purpose: 
//   Sets whether the actor is drawn in sphere mode.
//
// Arguments:
//   sphereMode : True for sphere mode; False for flat mode.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 19 12:34:23 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
vtkTexturedBackgroundActor::SetSphereMode(bool sphereMode)
{
    if(vtkGraphicsFactory::GetUseMesaClasses())
    {
        vtkMesaTexturedBackgroundMapper *m = 
            (vtkMesaTexturedBackgroundMapper *)GetInitializedMapper();
        m->SetSphereMode(sphereMode);
    }
    else
    {  
        vtkOpenGLTexturedBackgroundMapper *m = 
            (vtkOpenGLTexturedBackgroundMapper *)GetInitializedMapper();
        m->SetSphereMode(sphereMode);
    }
}

// ****************************************************************************
// Method: vtkTexturedBackgroundActor::SetImageRepetitions
//
// Purpose: 
//   Sets the number of image repetitions.
//
// Arguments:
//   nx : The number of repetitions in X.
//   ny : The number of repetitions in Y.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 19 12:34:55 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
vtkTexturedBackgroundActor::SetImageRepetitions(int nx, int ny)
{
    if(vtkGraphicsFactory::GetUseMesaClasses())
    {
        vtkMesaTexturedBackgroundMapper *m = 
            (vtkMesaTexturedBackgroundMapper *)GetInitializedMapper();
        m->SetImageRepetitions(nx, ny);
    }
    else
    {  
        vtkOpenGLTexturedBackgroundMapper *m = 
            (vtkOpenGLTexturedBackgroundMapper *)GetInitializedMapper();
        m->SetImageRepetitions(nx, ny);
    }
}

