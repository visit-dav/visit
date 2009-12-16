/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>


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
// ****************************************************************************

void
VisWinRenderingWithoutWindow::RealizeRenderWindow(void)
{
  // We'd like to do something lighter weight, but this seems to be the only
  // way to *force* VTK to initialize in all cases.  The good news is that this
  // method is typically called before we've got data in the RW, so it
  // shouldn't be as heavy as it looks at first glance.
  renWin->SetSize(300,300);
  renWin->Render();
}
