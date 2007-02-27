/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            VisWindowManager.C                             //
// ************************************************************************* //

#include <visitstream.h>

#include <VisWindowManager.h>


// ****************************************************************************
//  Method: VisWindowManager constructor
//
//  Programmer: Hank Childs
//  Creation:   May 15, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jun 26 17:13:55 PDT 2000
//    Removed unused variable toplevel.
//
// ****************************************************************************

VisWindowManager::VisWindowManager(int *argc, char **argv)
{
    //
    // Allocate memory for the vis windows.
    //
    windows  = new VisWindow*[10];
    nWindows = 10;
    iWindows = 0;

    //
    // Initialize the X stuff
    //
    XtSetLanguageProc(NULL, NULL, NULL);
    XtVaAppInitialize(&app, "Visit", NULL, 0, argc, argv, NULL, NULL);
}


// ****************************************************************************
//  Method: VisWindowManager destructor
//
//  Programmer: Hank Childs
//  Creation:   May 15, 2000
//
// ****************************************************************************

VisWindowManager::~VisWindowManager()
{
    if (windows != NULL)
    {
        for (int i = 0 ; i < iWindows ; i++)
        {
            if (windows[i] != NULL)
            {
                delete windows[i];
                windows[i] = NULL;
            }
        }
        delete [] windows;
        windows = NULL;
    }
}


// ****************************************************************************
//  Method: VisWindowManager::AddWindow
//
//  Purpose:
//      Adds a window.
//
//  Returns:    The index associated with the window.
//
//  Programmer: Hank Childs
//  Creation:   May 15, 2000
//
// ****************************************************************************

int
VisWindowManager::AddWindow()
{
    if (iWindows+1 >= nWindows)
    {
        //
        // Allocate more memory to hold the windows.
        //
        VisWindow **old_windows = windows;
        nWindows *= 2;
        windows = new VisWindow*[nWindows];
        for (int i = 0 ; i < iWindows ; i++)
        {
            windows[i] = old_windows[i];
        }
        delete [] old_windows;
    }

    windows[iWindows] = new VisWindow(app);
    iWindows++;
    return (iWindows-1);
}


// ****************************************************************************
//  Method: VisWindowManager::GetWindow
//
//  Purpose:
//      Fetches the VisWindow associated with the specified index.
//
//  Arguments:
//      index     The index of the VisWindow of interest.
// 
//  Returns:    A pointer to the VisWindow.
//
//  Programmer: Hank Childs
//  Creation:   May 15, 2000
//
// ****************************************************************************

VisWindow *
VisWindowManager::GetWindow(int index)
{
    if (index < 0 || index >= iWindows)
    {
        cerr << "Specified invalid index " << index << endl;
        // throw
        return NULL;
    }

    return windows[index];
}


// ****************************************************************************
//  Method: VisWindowManager::Start
//
//  Purpose:
//      Starts the event handling.
//
//  Programmer: Hank Childs
//  Creation:   May 15, 2000
//
// ****************************************************************************

void
VisWindowManager::Start(void)
{
    XtAppMainLoop(app);
}


