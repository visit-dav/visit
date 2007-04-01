// ************************************************************************* //
//                             InitVTKNoGraphics.C                           //
// ************************************************************************* //

#include <InitVTKNoGraphics.h>

#include <vtkObjectFactory.h>
#include <vtkDebugStream.h>
#include <vtkVersion.h>

//
// Include any classes that will override vtk classes.
//


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
}


// ****************************************************************************
//  Method: InitVTKNoGraphics::Initialize
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
//    Kathleen Bonnell, Wed Nov 12 16:51:56 PST 2003 
//    Comment out the VisItFactory until it is actually needed again.
//
//    Hank Childs, Thu Jan 22 16:47:27 PST 2004
//    Renamed to InitVTKNoGraphics.
//
// ****************************************************************************

void
InitVTKNoGraphics::Initialize(void)
{
    vtkDebugStream::Initialize();

#if 0
    // Register the factory that allows VisIt objects to override vtk objects. 
    vtkVisItFactory *factory = vtkVisItFactory::New();
    vtkObjectFactory::RegisterFactory(factory);
    factory->Delete();
#endif
}


