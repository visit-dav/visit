// ************************************************************************* //
//                                 InitVTK.C                                 //
// ************************************************************************* //

#include <InitVTK.h>
#include <InitVTKNoGraphics.h>
#include <vtkVisItCellDataToPointData.h>
#include <vtkVisItOpenGLPolyDataMapper.h>
#include <vtkVisItMesaPolyDataMapper.h>
#if defined(__APPLE__)
#include <vtkOSMesaRenderWindow.h>
#endif

#include <vtkObjectFactory.h>
#include <vtkVersion.h>
#if !defined(_WIN32)
#include <vtkGraphicsFactory.h>
#include <vtkImagingFactory.h>
#endif


//
// A factory that will allow VisIt to override any vtkObject
// with a sub-class of that object.
//
class vtkVisItGraphicsFactory : public vtkObjectFactory
{
  public:
    vtkVisItGraphicsFactory();
    static vtkVisItGraphicsFactory* New() { return new vtkVisItGraphicsFactory;};
    virtual const char* GetVTKSourceVersion();
    const char* GetDescription() { return "vtkVisItGraphicsFactory"; };

  protected:
    vtkVisItGraphicsFactory(const vtkVisItGraphicsFactory&);
    void operator=(const vtkVisItGraphicsFactory&);
};

//
// Necessary for each object that will override a vtkObject.
//
VTK_CREATE_CREATE_FUNCTION(vtkVisItOpenGLPolyDataMapper);
VTK_CREATE_CREATE_FUNCTION(vtkVisItMesaPolyDataMapper);
VTK_CREATE_CREATE_FUNCTION(vtkVisItCellDataToPointData);
#if defined(__APPLE__)
VTK_CREATE_CREATE_FUNCTION(vtkOSMesaRenderWindow);
#endif


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
vtkVisItGraphicsFactory::vtkVisItGraphicsFactory()
{
  this->RegisterOverride("vtkOpenGLPolyDataMapper", "vtkVisItOpenGLPolyDataMapper",
                         "vtkVisItOpenGLPolyDataMapper override vtkOpenGLPolyDataMapper",
                         1,
                         vtkObjectFactoryCreatevtkVisItOpenGLPolyDataMapper);
  this->RegisterOverride("vtkMesaPolyDataMapper", "vtkVisItMesaPolyDataMapper",
                         "vtkVisItMesaPolyDataMapper override vtkMesaPolyDataMapper",
                         1,
                         vtkObjectFactoryCreatevtkVisItMesaPolyDataMapper);
  this->RegisterOverride("vtkCellDataToPointData", "vtkVisItCellDataToPointData",
                         "vtkVisItCellDataToPointData override vtkCellDataToPointData",
                         1,
                         vtkObjectFactoryCreatevtkVisItCellDataToPointData);
#if defined(__APPLE__)
  this->RegisterOverride("vtkCarbonRenderWindow", "vtkOSMesaRenderWindow",
                         "vtkOSMesaRenderWindow override vtkCarbonRenderWindow",
                         1,
                         vtkObjectFactoryCreatevtkOSMesaRenderWindow);
#endif
}

// ****************************************************************************
//  Method: InitVTK::Initialize
//
//  Purpose:
//      Initialize the VTK portion of the program.  Separated from Init so
//      there would not be a VTK dependence on the GUI.  The only 
//      initialization that happens currently is the vtk print statements are
//      re-routed to the debug stream.
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
// ****************************************************************************

void
InitVTK::Initialize(void)
{
    InitVTKNoGraphics::Initialize();

    // Register the factory that allows VisIt objects to override vtk objects.
    vtkVisItGraphicsFactory *factory = vtkVisItGraphicsFactory::New();
    vtkObjectFactory::RegisterFactory(factory);
    factory->Delete();
}


// ****************************************************************************
//  Method: InitVTK::Force/Unforce Mesa
//
//  Purpose:
//      Forces all VTK rendering objects to be instantiated as Mesa object.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Feb  5 09:09:08 PST 2003
//    Use vtkGrahpicsFactory and vtkImagingFactory static variables to
//    handle Mesa objects.
//
// ****************************************************************************

void
InitVTK::ForceMesa(void)
{
#if !defined(_WIN32)
    vtkGraphicsFactory::SetUseMesaClasses(1);
    vtkImagingFactory::SetUseMesaClasses(1);
#endif
}


void
InitVTK::UnforceMesa(void)
{
#if !defined(_WIN32)
    vtkGraphicsFactory::SetUseMesaClasses(0);
    vtkImagingFactory::SetUseMesaClasses(0);
#endif
}


