// ************************************************************************* //
//                                 InitVTK.C                                 //
// ************************************************************************* //

#include <InitVTK.h>

#if !defined(_WIN32)
#include <vtkGraphicsFactory.h>
#include <vtkImagingFactory.h>
#endif


#include <vtkObjectFactory.h>
#include <vtkDebugStream.h>
#include <vtkVersion.h>

//
// Include any classes that will override vtk classes.
//
#include <vtkVisItTriangle.h>
#include <vtkVisItQuad.h>


//
// A factory that will allow VisIt to override any vtkObject
// with a sub-class of that object. 
//
class VISIT_VTK_API vtkVisItFactory : public vtkObjectFactory
{
  public:
    vtkVisItFactory();
    static vtkVisItFactory* New() { return new vtkVisItFactory;};
    virtual const char* GetVTKSourceVersion(); 
    const char* GetDescription() { return "vtkVisItFactory"; };
 
  protected:
    vtkVisItFactory(const vtkVisItFactory&);
    void operator=(const vtkVisItFactory&);
};

//
// Necessary for each object that will override a vtkObject. 
//
VTK_CREATE_CREATE_FUNCTION(vtkVisItTriangle);
VTK_CREATE_CREATE_FUNCTION(vtkVisItQuad);


const char* 
vtkVisItFactory::GetVTKSourceVersion()
{
    return VTK_SOURCE_VERSION;
}

//
//  Create the overrides so that VTK will use VisIt's class instead.
//
vtkVisItFactory::vtkVisItFactory()
{
  this->RegisterOverride("vtkTriangle", "vtkVisItTriangle",
                         "vtkVisItTriangle override vtkTriangle",
                         1,
                         vtkObjectFactoryCreatevtkVisItTriangle);
  this->RegisterOverride("vtkQuad", "vtkVisItQuad",
                         "vtkVisItQuad override vtkQuad",
                         1,
                         vtkObjectFactoryCreatevtkVisItQuad);
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
//    Kathleen Bonnell, Thu Apr 10 18:27:54 PDT 2003   
//    Register the factory that will allow VisIt to override vtkObjects.
//
// ****************************************************************************

void
InitVTK::Initialize(void)
{
    vtkDebugStream::Initialize();

    // Register the factory that allows VisIt objects to override vtk objects. 
    vtkVisItFactory *factory = vtkVisItFactory::New();
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


