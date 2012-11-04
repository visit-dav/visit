/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                      VisWinRenderingWithoutWindow.C                       //
// ************************************************************************* //

#include <VisWinRenderingWithoutWindow.h>

#include <vtkMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#if __APPLE__
#include <vtkCocoaRenderWindow.h>
#include "VisWinRenderingCocoaHideWindow.h"
void UnMapWindow(vtkRenderWindow* v)
{
    vtkCocoaRenderWindow* vx=dynamic_cast<vtkCocoaRenderWindow*>(v);
    if(vx) VisWinRenderingCocoa::HideRenderWindow(vx->GetRootWindow());
}
#elif __unix__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <vtkXOpenGLRenderWindow.h>
void UnMapWindow(vtkRenderWindow* v)
{
    vtkXOpenGLRenderWindow* vx = dynamic_cast<vtkXOpenGLRenderWindow*>(v);
    if(vx) XUnmapWindow(vx->GetDisplayId(),vx->GetWindowId());
}
#else //Windows does not show window OffScreenRenderingMode
void UnMapWindow(vtkRenderWindow* v) { /*do nothing..*/  }
#endif

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
    if(std::string(renWin->GetClassName()) == "vtkOSOpenGLRenderWindow" ||
       std::string(renWin->GetClassName()) == "vtkWin32OpenGLRenderWindow" ||
       std::string(renWin->GetClassName()) == "vtkCocoaRenderWindow")
        renWin->OffScreenRenderingOn();
    InitializeRenderWindow(renWin);
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
// ****************************************************************************

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
  renWin->Render();

  /// HKTODO: Verify this solution is robust
  /// If not Mesa-based Offscreen rendering: the other
  /// versions seem to popup an empty window, the UnMap logic
  /// hides this Window, unfortunately without this window
  /// the software rendering on Linux does not work
  std::string cname = renWin->GetClassName();
  if(cname != "vtkOSOpenGLRenderWindow") UnMapWindow(renWin);
  // Cyrus' Note: when using vtk-5.8.0.a, we have a fix to make sure
  // that offscreen cocoa windows aren't mapped. Since Hari 
  // needs to work with other versions of vtk, the above is still
  // necessary.
}

// ****************************************************************************
//  Method: VisWinRenderingWithoutWindow::SetImmediateRenderingMode
//
//  Purpose:
//      Prevent VTK from using display lists.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 30 18:20:10 PDT 2011
//
//  Notes: This offscreen rendering colleague is responsible for rendering
//         using Mesa. As of VTK 5.8 and Mesa 7.8.2, there are some rare
//         glitches with offscreen Mesa vbo's that cause text to go missing.
//         By forcing all of VTK to not use display lists, we avoid the 
//         problems.
//
//  Modifications:
//
// ****************************************************************************

void
VisWinRenderingWithoutWindow::SetImmediateModeRendering(bool)
{
    // In offscreen rendering, never use display lists.
    vtkMapper::GlobalImmediateModeRenderingOn();
}
