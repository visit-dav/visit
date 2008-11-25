/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <QLayout>
#include <QvisDelayedWindow.h>

// ****************************************************************************
// Method: QvisDelayedWindow::QvisDelayedWindow
//
// Purpose: 
//   Constructor for the QvisDelayedWindow class.
//
// Arguments:
//   captionString : The caption that appears in the window decoration.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:01:16 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Aug 30 13:50:54 PST 2000
//   Made it inherit from QvisWindowBase.
//
//   Brad Whitlock, Mon Sep 30 07:51:17 PDT 2002
//   I added window flags.
//
//   Brad Whitlock, Wed Apr  9 10:31:45 PDT 2008
//   Changed ctor args.
//
//   Brad Whitlock, Thu Jun 19 14:14:27 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisDelayedWindow::QvisDelayedWindow(const QString &captionString, Qt::WindowFlags f) : 
    QvisWindowBase(captionString, f)
{
    isCreated = false;
}

// ****************************************************************************
// Method: QvisDelayedWindow::~QvisDelayedWindow
//
// Purpose: 
//   Destructor for the QvisDelayedWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:01:52 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QvisDelayedWindow::~QvisDelayedWindow()
{
    isCreated = false;
}

// ****************************************************************************
// Method: QvisDelayedWindow::show
//
// Purpose: 
//   This is a slot function that shows the window. It creates the
//   window first if it has not yet been created. It raises the
//   window if it is already showing.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:02:16 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisDelayedWindow::show()
{
    if(!isCreated)
    {
        // Create the window and show it.
        CreateEntireWindow();
        isCreated = true;

        // Update the widgets based on the state information.
        UpdateWindow(true);

        // Show the window
        QvisWindowBase::show();
    }
    else if(isVisible())
    {
        // The window is created and is already showing.
        raise();
    }
    else
    {
        // Show the window
        QvisWindowBase::show();
    }
}

// ****************************************************************************
// Method: QvisDelayedWindow::raise
//
// Purpose: 
//   This is a slot function that raises the window. It creates the
//   window first if it has not yet been created.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:03:07 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisDelayedWindow::raise()
{
    if(!isCreated)
    {
        // Create the window
        CreateEntireWindow();
        isCreated = true;

        // Update the widgets to the right values
        UpdateWindow(true);
    }

    // raise the window
    QvisWindowBase::raise();
}

// ****************************************************************************
// Method: QvisDelayedWindow::hide
//
// Purpose: 
//   This is a slot function that hides the window if it has been
//   created.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:04:16 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisDelayedWindow::hide()
{
    if(isCreated)
    {
        QvisWindowBase::hide();
    }
}

// ****************************************************************************
// Method: QvisDelayedWindow::GetCentralWidget
//
// Purpose: 
//   Returns a pointer to the window's central widget.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:05:08 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisDelayedWindow::GetCentralWidget()
{
    return central;
}

// ****************************************************************************
// Method: QvisDelayedWindow::CreateEntireWindow
//
// Purpose: 
//   Creates the central widget and the top layout. It then calls the
//   subclass's CreateWindowContents method which actually adds the bulk of
//   the widgets to the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:05:38 PDT 2000
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 11:34:31 PDT 2002
//   Added an early return if the window exists.
//
//   Brad Whitlock, Thu Jun 19 14:15:13 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisDelayedWindow::CreateEntireWindow()
{
    // If the window is created then return.
    if(isCreated)
        return;

    // Create the central widget and the top layout.
    central = new QWidget(this);
    setCentralWidget(central);
    topLayout = new QVBoxLayout(central);
    topLayout->setSpacing(10);

    // Call the Sub-class's CreateWindowContents function to create the
    // internal parts of the window.
    CreateWindowContents();
}

// ****************************************************************************
// Method: QvisDelayedWindow::UpdateWindow
//
// Purpose: 
//   This function is called by the subclass's CreateWindowContents method.
//   It is supposed to be overridden in subclasses so it does something
//   useful.
//
// Arguments:
//   doAll : Whether or not to update all the widgets in the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:06:31 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisDelayedWindow::UpdateWindow(bool)
{
    // do nothing.
}
