// ************************************************************************* //
//                                 InitVTK.C                                 //
// ************************************************************************* //

#include <InitVTK.h>
#include <InitVTKNoGraphics.h>

#if !defined(_WIN32)
#include <vtkGraphicsFactory.h>
#include <vtkImagingFactory.h>
#endif


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
// ****************************************************************************

void
InitVTK::Initialize(void)
{
    InitVTKNoGraphics::Initialize();
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


