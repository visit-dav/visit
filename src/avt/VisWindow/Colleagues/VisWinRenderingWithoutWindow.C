// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      VisWinRenderingWithoutWindow.C                       //
// ************************************************************************* //

#include <VisWinRenderingWithoutWindow.h>

#include <Environment.h>
#include <avtCallback.h>
#include <visit-config.h>
#include <DebugStream.h>

#include <vtkMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtk_glew.h>
#include <vtkRenderer.h>

#define DS_NOT_CHECKED    0
#define DS_NOT_AVAILABLE  1
#define DS_AVAILABLE      2

// ****************************************************************************
//  Method: VisWinRenderingWithoutWindow constructor
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2002
//
//  Modifications:
//    Kathleen Bonnell, Tue Feb 11 11:28:03 PST 2003
//    Removed iren.
//
//    Tom Fogal, Tue Nov 24 11:25:39 MST 2009
//    Make sure to set offscreen before other initialization.
//
//    Kathleen Biagas, Wed Oct 31 17:29:26 PDT 2012
//    'vtkWin32OpenGLRenderWindow' needs OffscreenRendering set, too.
//
//    Cyrus Harrison, Sat Nov  3 19:37:42 PDT 2012
//    For osx w/o mesa, 'vtkCocoaRenderWindow' needs OffscreenRendering set,
//    as well.
//
// ****************************************************************************

VisWinRenderingWithoutWindow::VisWinRenderingWithoutWindow(
                                                    VisWindowColleagueProxy &p)
   : VisWinRendering(p)
{
    //
    // It is kind of assumed that these new calls will return Mesa derived
    // types, but there is nothing from this class's perspective saying we have
    // to -- OpenGL offscreen rendering is fully supported.  So: it might be
    // Mesa that we are getting, but we don't care.
    //
    renWin = vtkRenderWindow::New();

    renWin->OffScreenRenderingOn();
    InitializeRenderWindow(renWin);

    displayStatus = DS_NOT_CHECKED;
}


// ****************************************************************************
//  Method: VisWinRenderingWithoutWindow destructor
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2002
//
//  Modifications:
//    Kathleen Bonnell, Tue Feb 11 11:28:03 PST 2003
//    Removed iren.
//
// ****************************************************************************

VisWinRenderingWithoutWindow::~VisWinRenderingWithoutWindow()
{
    if (renWin != NULL)
    {
        renWin->Delete();
        renWin = NULL;
    }
}

// ****************************************************************************
//  Method: VisWinRenderingWithoutWindow::GetRenderWindow
//
//  Purpose:
//      Gets the render window.  This is a virtual function that allows the
//      base type to operate on the window, even though it is not aware that
//      there is no window.
//
//  Returns:    The render window typed as the base class.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2002
//
// ****************************************************************************

vtkRenderWindow *
VisWinRenderingWithoutWindow::GetRenderWindow(void)
{
    return renWin;
}

// ****************************************************************************
// Method: VisWinRenderingWithoutWindow::RenderRenderWindow
//
// Purpose:
//   Render the render window when it is safe to do so.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 13 16:06:22 PDT 2013
//
// Modifications:
//   Eric Brugger, Fri May 10 14:39:42 PDT 2013
//   I removed support for mangled mesa.
//
//   Brad Whitlock, Fri Oct 17 20:02:39 PDT 2014
//   Check to see if we have X11 before looking for the DISPLAY variable. If
//   have built VisIt or VTK without X11 support then we won't get an X11-based
//   render window. In that case, we'd like to try and render what VTK provides
//   since it is likely a vtkOSOpenGLRenderWindow, on offscreen window. I also
//   improved the warning message.
//
//   Alister Maguire, Tue May 19 12:01:24 PDT 2020
//   Added OSPRay check. When toggling out of OSPRay mode, we need to make
//   sure that shadows are disabled. If we don't, VTK likes to crash.
//
//    Kathleen Biagas, Wed Aug 17, 2022
//    Incorporate ARSanderson's OSPRAY 2.8.0 work for VTK 9.
//
// ****************************************************************************

void
VisWinRenderingWithoutWindow::RenderRenderWindow(void)
{
#ifdef VISIT_OSPRAY
    if (GetOsprayRendering() && modeIsPerspective)
    {
        if (canvas->GetPass() == NULL)
        {
            canvas->SetPass(osprayPass);
        }
    }
    else
    {
        canvas->SetUseShadows(false);
        canvas->SetPass(0);
    }
#elif defined(HAVE_OSPRAY)
    if (osprayRendering && viewIs3D)
    {
        if (canvas->GetPass() == NULL)
        {
            canvas->SetUseShadows(osprayShadows);
            canvas->SetPass(osprayPass);
        }
    }
    else
    {
        canvas->SetUseShadows(false);
        canvas->SetPass(0);
    }
#endif

#if defined(__unix__) && !defined(__APPLE__) && defined(HAVE_LIBX11) && !defined(HAVE_OSMESA)
    if(displayStatus == DS_NOT_CHECKED)
    {
        // On X11 systems not using mangled mesa, make sure that the DISPLAY is set.
        if(Environment::get("DISPLAY").empty())
            displayStatus = DS_NOT_AVAILABLE;
        else
            displayStatus = DS_AVAILABLE;
    }

    if(displayStatus == DS_AVAILABLE)
    {
        GetRenderWindow()->Render();
    }
    else
    {
        avtCallback::IssueWarning("VisIt was not built with support for "
            "software-based offscreen rendering. This is often the case when "
            "the --mesa flag was not passed to the build_visit script.\n\n"
            "This means that the DISPLAY environment variable must be set to a "
            "valid X-server display in order to render an image. If you are running "
            "client/server, you may be able to work around this issue by -X to the "
            "SSH arguments in the host profile for the remote computer. The best"
            "alternative is to rebuild VisIt with --mesa support on the remote "
            "computer.");
    }
#else
    GetRenderWindow()->Render();
#endif
}

// ****************************************************************************
//  Method: VisWinRenderingWithoutWindow::RealizeRenderWindow
//
//  Purpose:
//      Forces VTK to initialize the underlying window.
//
//  Programmer: Tom Fogal
//  Creation:   December 9th, 2009
//
//
//  Modifications:
//   Cyrus Harrison, Sat Nov  3 23:51:13 PDT 2012
//   Force large window size for offscreen cocoa.
//
//   Brad Whitlock, Wed Mar 13 16:07:35 PDT 2013
//   Don't directly call Render.
//
//   Burlen Loring, Thu Oct  8 13:41:39 PDT 2015
//   log vtk/open gl capabilities, fix indentation
//
// ****************************************************************************


#define safes(arg) (arg?((const char *)arg):"")


void
VisWinRenderingWithoutWindow::RealizeRenderWindow(void)
{
    // We'd like to do something lighter weight, but this seems to be the only
    // way to *force* VTK to initialize in all cases.  The good news is that this
    // method is typically called before we've got data in the RW, so it
    // shouldn't be as heavy as it looks at first glance.

    //
    // SetSize doesn't work as expected with vtkCocoaRenderWindow in
    // an offscreen setting.
    // B/c of this we are forced to create a large window, the size of
    // which bounds our offscreen rendering.
    //
    if(std::string(renWin->GetClassName()) == "vtkCocoaRenderWindow")
    {
        //TODO: we may want to query to find the largest valid size
        // OSX limits windows to 10Kx10K, however OpenGL contexts
        // are limited further.
        renWin->SetSize(4096,4096);
        renWin->SetPosition(-10000,-10000);
    }
    else
    {
        renWin->SetSize(300,300);
    }
    RenderRenderWindow();

    debug2 << "render window is a " << renWin->GetClassName() << endl;
    vtkOpenGLRenderWindow *glrw = dynamic_cast<vtkOpenGLRenderWindow*>(renWin);
    if (!glrw) return;

    const char *glvers = safes(glGetString(GL_VERSION));
    debug2 << "  GLVersion: " << glvers << endl;
    // if we need more information, perhaps we should call
    // vtkOpenGLRenderWindow::ReportCapabilities()

}

