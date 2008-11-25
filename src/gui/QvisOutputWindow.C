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
#include <QPushButton>
#include <QString>
#include <QTabWidget>
#include <QTextEdit>

#include <QvisOutputWindow.h>
#include <QvisMainWindow.h>
#include <MessageAttributes.h>
#include <UnicodeHelper.h>

// ****************************************************************************
// Method: QvisOutputWindow::QvisOutputWindow
//
// Purpose: 
//   This is the constructor for the QvisOutputWindow class. It
//   creates the widgets that make up the window.
//
// Arguments:
//   msgAtts : The MessageAttributes object that the window observes.
//   caption : The name of the window that appears in the window
//             decoration.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 27 17:27:13 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Sep 25 08:50:30 PDT 2001
//   Prevented stretch from being added to the window.
//
//   Brad Whitlock, Wed Apr  9 10:53:31 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisOutputWindow::QvisOutputWindow(MessageAttributes *subj,
    const QString &caption, const QString &shortName, QvisNotepadArea *notepad) : 
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::NoExtraButtons,
                               false)
{
    // nothing here
}

// ****************************************************************************
// Method: QvisOutputWindow::~QvisOutputWindow
//
// Purpose: 
//   Destructor for the QvisOutputWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 27 17:29:22 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisOutputWindow::~QvisOutputWindow()
{
    // nothing here
}

// ****************************************************************************
// Method: QvisOutputWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the window's widgets.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 12:17:28 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Sep 25 08:51:08 PDT 2001
//   Set the minimum width based on part of a sample message from the viewer
//   and the current font size.
//
// ****************************************************************************

void
QvisOutputWindow::CreateWindowContents()
{
    // Create a multi line edit to display the text.
    outputText = new QTextEdit(central);
    outputText->setMinimumWidth(fontMetrics().width("MESSAGE: Closed the "
        "compute engine on host"));
    outputText->setWordWrapMode(QTextOption::WordWrap);
    outputText->setReadOnly(true);
    topLayout->addWidget(outputText);
}

// ****************************************************************************
// Method: QvisOutputWindow::UpdateWindow
//
// Purpose: 
//   Updates the output window when the MessageAttributes object that
//   the window watches is updated.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 16 11:52:07 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Sep 25 09:14:15 PDT 2001
//   Added a newline in between messages.
//
//   Brad Whitlock, Thu Oct 25 18:45:27 PST 2001
//   Modified how the main window is told to update its icon.
//
//   Brad Whitlock, Tue May 20 15:09:11 PST 2003
//   Made it work with the regenerated MessageAttributes.
//
//   Brad Whitlock, Thu May 11 14:59:11 PST 2006
//   Added support for ErrorClear.
//
//   Brad Whitlock, Tue Apr 29 10:21:44 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri May 30 15:40:17 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisOutputWindow::UpdateWindow(bool)
{
    MessageAttributes *msgAtts = (MessageAttributes *)subject;

    // Return without doing anything if it's ErrorClear.
    if(msgAtts->GetSeverity() == MessageAttributes::ErrorClear)
        return;

    // Create a string to add to the output text.
    QString temp;
    if(msgAtts->GetSeverity() == MessageAttributes::Error)
        temp = tr("ERROR: ");
    if(msgAtts->GetSeverity() == MessageAttributes::Message)
        temp = tr("MESSAGE: ");
    if(msgAtts->GetSeverity() == MessageAttributes::Warning)
        temp = tr("WARNING: ");
    temp += MessageAttributes_GetText(*msgAtts);
    temp += QString("\n");

    // Add the line of text.
    outputText->append(temp);

    // If the window is visible then tell the main window to turn its
    // unread icon back to blue.
    if(isVisible() || posted())
        emit unreadOutput(false);
}
