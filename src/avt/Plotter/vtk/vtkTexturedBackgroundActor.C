// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <vtkTexturedBackgroundActor.h>
#include <vtkGraphicsFactory.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkRenderer.h>
#include <vtkTexture.h>

#include "vtkOpenGLTexturedBackgroundMapper.h"

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
//   Tom Fogal, Tue Apr 27 13:06:28 MDT 2010
//   Remove special case for Mesa.
//
// ****************************************************************************

vtkMapper2D *
vtkTexturedBackgroundActor::GetInitializedMapper()
{
    if(GetMapper() == NULL)
    {
        vtkOpenGLTexturedBackgroundMapper *m =
            vtkOpenGLTexturedBackgroundMapper::New();
        this->SetMapper(m);
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

    vtkOpenGLTexturedBackgroundMapper *m =
        (vtkOpenGLTexturedBackgroundMapper *)GetInitializedMapper();
    m->SetTextureAndRenderers(texture, bg, canvas);

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
//   Tom Fogal, Tue Apr 27 13:06:28 MDT 2010
//   Remove special case for Mesa.
//
// ****************************************************************************

void
vtkTexturedBackgroundActor::SetSphereMode(bool sphereMode)
{
    vtkOpenGLTexturedBackgroundMapper *m =
        (vtkOpenGLTexturedBackgroundMapper *)GetInitializedMapper();
    m->SetSphereMode(sphereMode);
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
//   Tom Fogal, Tue Apr 27 13:06:28 MDT 2010
//   Remove special case for Mesa.
//
// ****************************************************************************

void
vtkTexturedBackgroundActor::SetImageRepetitions(int nx, int ny)
{
    vtkOpenGLTexturedBackgroundMapper *m =
        (vtkOpenGLTexturedBackgroundMapper *)GetInitializedMapper();
    m->SetImageRepetitions(nx, ny);
}
