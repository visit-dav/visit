// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 InitVTK.C                                 //
// ************************************************************************* //

#include <InitVTK.h>
#include <InitVTKLite.h>

#include <vtkObjectFactory.h>
#include <vtkVersion.h>
#include <vtkVisItCellDataToPointData.h>
#include <visit-config.h>


//
// A factory that will allow VisIt to override any vtkObject
// with a sub-class of that object.
//
class vtkVisItObjectFactory : public vtkObjectFactory
{
  public:
    vtkVisItObjectFactory();
    static vtkVisItObjectFactory* New() { return new vtkVisItObjectFactory; }
    const char* GetVTKSourceVersion() override;
    const char* GetDescription() override { return "vtkVisItObjectFactory"; }

  protected:
    vtkVisItObjectFactory(const vtkVisItObjectFactory&);
    void operator=(const vtkVisItObjectFactory&);
};

//
// Necessary for each object that will override a vtkObject.
//
VTK_CREATE_CREATE_FUNCTION(vtkVisItCellDataToPointData)

#include <vtkVisItDataSetWriter.h>
VTK_CREATE_CREATE_FUNCTION(vtkVisItDataSetWriter)

const char*
vtkVisItObjectFactory::GetVTKSourceVersion()
{
    return VTK_SOURCE_VERSION;
}

//
//  Create the overrides so that VTK will use VisIt's class instead.
//
//  Modifications:
//    Dave Bremer, Fri May 25 11:48:48 PDT 2007
//    Added override for vtkVisItStructuredGrid and vtkVisItRectilinearGrid
//    (our versions are much more memory friendly).  Note: since
//    vtkVisItCellDataToPointData is in this module, this isn't really a
//    "graphics" factory any more.  Maybe it should be renamed.  I decided
//    to pile on instead.
//
//    Dave Bremer, Thu Jun 14 16:47:37 PDT 2007
//    Disable my previous change.  It causes a problem I don't understand
//    yet, seemingly related to reading a vtkRectilinearGrid or vtkStructuredGrid
//    from a file.
//
//    Dave Bremer, Mon Jun 18 17:44:43 PDT 2007
//    Reinstantiated use of vtkVisItStructuredGrid and vtkVisItRectilinearGrid.
//
//
vtkVisItObjectFactory::vtkVisItObjectFactory()
{
  this->RegisterOverride("vtkCellDataToPointData", "vtkVisItCellDataToPointData",
                         "vtkVisItCellDataToPointData override vtkCellDataToPointData",
                         1,
                         vtkObjectFactoryCreatevtkVisItCellDataToPointData);

  this->RegisterOverride("vtkDataSetWriter", "vtkVisItDataSetWriter",
                         "vtkVisItDataSetWriter override vtkDataSetWriter",
                         1,
                         vtkObjectFactoryCreatevtkVisItDataSetWriter);
}

// ****************************************************************************
//  Method: InitVTK::Initialize
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
//    Kathleen Biagas, Thu Jul 17, 2025
//    Stifle vtkLogger output to terminal by setting its verbosity level OFF.
//    Avoids lots of console output, even from vtkWarning or vtkDebug macros
//    which we capture in our debug logs. 
//
//    Kathleen Biagas, Wed Oct 1, 2025
//    Move vtkLogger logic to InitVTKLite.  Pass component argument
//    to InitVTKLite::Initialize.
//
// ****************************************************************************

void
InitVTK::Initialize(const std::string &component)
{
    InitVTKLite::Initialize(component);

    // Register the factory that allows VisIt objects to override vtk objects.
    vtkVisItObjectFactory *factory = vtkVisItObjectFactory::New();
    vtkObjectFactory::RegisterFactory(factory);
    factory->Delete();
}


