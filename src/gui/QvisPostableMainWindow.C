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
#include <QvisPostableMainWindow.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>

// ****************************************************************************
// Method: QvisPostableMainWindow::QvisPostableMainWindow
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   captionString : The window caption.
//   shortName     : The window name that appears in the notepad title.
//   n             : The notepad.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 24 09:39:38 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

QvisPostableMainWindow::QvisPostableMainWindow(const QString &captionString,
    const QString &shortName, QvisNotepadArea *n) : QvisPostableWindow(captionString, shortName, n)
{
    contentsWidget = 0;
    contentsLayout = 0;
    addLayoutStretch = false;

    CreateEntireWindow();
    dismissButton->setEnabled(false);
    isCreated = true;
}

// ****************************************************************************
// Method: QvisPostableMainWindow::~QvisPostableMainWindow
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 24 09:40:24 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

QvisPostableMainWindow::~QvisPostableMainWindow()
{
}

// ****************************************************************************
// Method: QvisPostableMainWindow::CreateWindowContents
//
// Purpose: 
//   Creates a widget that we'll use for the parent of the main window stuff.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 24 09:40:38 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableMainWindow::CreateWindowContents()
{
    topLayout->setMargin(0);
    contentsWidget = new QWidget(central);
    topLayout->addWidget(contentsWidget);
    contentsLayout = new QVBoxLayout(contentsWidget);
    contentsLayout->setMargin(0);
}

void
QvisPostableMainWindow::CreateNode(DataNode *)
{
}

void
QvisPostableMainWindow::SetFromNode(DataNode *, const int *borders)
{
}

// ****************************************************************************
// Method: QvisPostableMainWindow::ContentsWidget
//
// Purpose: 
//   Return the contents widget.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 24 09:41:03 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisPostableMainWindow::ContentsWidget()
{
    return contentsWidget;
}

// ****************************************************************************
// Method: QvisPostableMainWindow::ContentsLayout
//
// Purpose: 
//   Return the contents layout.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 24 09:41:46 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

QVBoxLayout *
QvisPostableMainWindow::ContentsLayout()
{
    return contentsLayout;
}
