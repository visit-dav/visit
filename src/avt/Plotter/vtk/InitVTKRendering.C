// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <InitVTKRendering.h>

#include <vtkVisItDataSetMapper.h>
#include <vtkOpenGLPointMapper.h>

#include <vtkObjectFactory.h>
#include <vtkVersion.h>


#include <avtCallback.h>
#include <DebugStream.h>

//
// The following calls initialize VTK modules.  They are built-in factories
// similar to what is done below, but are standard VTK, providing a lot of
// OpenGL overrides for rendering classes.
//
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)
VTK_MODULE_INIT(vtkRenderingOpenGL2)

//
// A factory that will allow VisIt to override any vtkObject
// with a sub-class of that object.
//
class vtkVisItGraphicsFactory : public vtkObjectFactory
{
  public:
    static vtkVisItGraphicsFactory * New();
    vtkTypeMacro(vtkVisItGraphicsFactory, vtkObjectFactory)

    const char* GetDescription() override { return "vtkVisItGraphicsFactory"; };
    const char* GetVTKSourceVersion() override;

  protected:
    vtkVisItGraphicsFactory();

  private:
    vtkVisItGraphicsFactory(const vtkVisItGraphicsFactory&);
    void operator=(const vtkVisItGraphicsFactory&);
};

vtkStandardNewMacro(vtkVisItGraphicsFactory)

//
// Necessary for each object that will override a vtkObject.
//
VTK_CREATE_CREATE_FUNCTION(vtkVisItDataSetMapper);
VTK_CREATE_CREATE_FUNCTION(vtkOpenGLPointMapper);

const char*
vtkVisItGraphicsFactory::GetVTKSourceVersion()
{
    return VTK_SOURCE_VERSION;
}

//
//  Create the overrides so that VTK will use VisIt's class instead.
//
//  Modifications:
//    Hank Childs, Fri Oct 22 07:13:35 PDT 2004
//    Added vtkVisItCellDataToPointData.
//
//    Brad Whitlock, Wed Jul 13 15:11:53 PST 2005
//    Added vtkOSMesaRenderWindow on MacOS X.
//
//    Brad Whitlock, Fri Aug 26 10:29:26 PDT 2005
//    Added override for vtkDataSetMapper.
//
//    Dave Bremer, Fri May 25 11:48:48 PDT 2007
//    Added override for vtkVisItStructuredGrid and vtkVisItRectilinearGrid
//    (our versions are much more memory friendly).  Note: since 
//    vtkVisItCellDataToPointData is in this module, this isn't really a
//    "graphics" factory any more.  Maybe it should be renamed.  I decided
//    to pile on instead.
//
//    Dave Bremer, Thu Jun 14 16:47:37 PDT 2007
//    Disable my previous change.  It causes a problem I don't understand yet,
//    seemingly related to reading a vtkRectilinearGrid or vtkStructuredGrid
//    from a file.
//
//    Dave Bremer, Mon Jun 18 17:44:43 PDT 2007
//    Reinstantiated use of vtkVisItStructuredGrid and vtkVisItRectilinearGrid.
//
//    Brad Whitlock, Thurs Nov 15 16:25 PST 2007
//    Added override of vtkOpenGLTexture that is compatible with our
//    vtkQtRenderWindow.
//
//    Tom Fogal, Tue Apr 27 11:42:46 MDT 2010
//    Fix Mesa special case: our OGL classes can handle that.
//
//    Brad Whitlock, Wed Aug 10 11:52:31 PDT 2011
//    Use VTK's new polydata mapper by eliminating our override of
//    vtkOpenGLPolyDataMapper.
//
//    Kathleen Biagas, Mon Jul 18 10:02:30 PDT 2016
//    Remove vtkVisItDataSetMapper
//
//    Kathleen Biagas, Wed Aug 17 09:27:12 PDT 2016
//    Add vtkOpenGLPointMapper.
//
//    Kathleen Biagas, Tue Apr 13 2021
//    Add vtkVisItDataSetMapper.
//
//    Kathleen Biagas, Thu July 22, 2021
//    Remove vtkVisItRectilinearGrid and vtkStructuredGrid overrides for
//    they are defined in visit_vtk/full/InitVTK
//

vtkVisItGraphicsFactory::vtkVisItGraphicsFactory()
{
  this->RegisterOverride("vtkDataSetMapper", "vtkVisItDataSetMapper",
                         "vtkVisItDataSetMapper override vtkDataSetMapper",
                         1,
                         vtkObjectFactoryCreatevtkVisItDataSetMapper);
  this->RegisterOverride("vtkPointMapper",
    "vtkOpenGLPointMapper",
    "vtkOpenGLPointMapper override vtkPointMapper",
    1,
    vtkObjectFactoryCreatevtkOpenGLPointMapper);
}

// ****************************************************************************
//  Method: InitVTKRendering::Initialize
//
//  Purpose:
//      Initialize the VTK portion of the program.  Separated from Init so
//      there would not be a VTK dependence on the GUI.
//
//  Programmer: Hank Childs
//  Creation:   April 24, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Apr 10 18:27:54 PDT 2003   
//    Register the factory that will allow VisIt to override vtkObjects.
//
//    Kathleen Bonnell, Wed Nov 12 16:51:56 PST 2003 
//    Comment out the VisItFactory until it is actually needed again.
//
//    Hank Childs, Thu Jan 22 17:31:23 PST 2004
//    Use the InitVTKNoGraphics Initialize routine to minimize duplication.
//
//    Hank Childs, Wed May  5 10:15:48 PDT 2004
//    Use the VisIt graphics factory to override the standard polydata mapper.
//
//    Carson Brownlee, Sun May  6 16:25:28 PDT 2012
//    Add support for Manta.
//
//    Qi WU, Sat Jun 10 22:21:27 MST 2018
//    Remove the old VISIT_OSPRAY configurations because they are too old. 
//
// ****************************************************************************

void
InitVTKRendering::Initialize(void)
{
    // Register the factory that allows VisIt objects to override vtk objects.
    vtkVisItGraphicsFactory *factory = vtkVisItGraphicsFactory::New();
    vtkObjectFactory::RegisterFactory(factory);
    factory->Delete();
}
