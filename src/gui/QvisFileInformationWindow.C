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

#include <visitstream.h>
#include <qmultilineedit.h>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qstring.h>

#include <FileServerList.h>
#include <QvisFileInformationWindow.h>

#include <avtDatabaseMetaData.h>

//
// Static constants
//
const char *QvisFileInformationWindow::titleSeparator =
"================================================";

// ****************************************************************************
// Method: QvisFileInformationWindow::QvisFileInformationWindow
//
// Purpose: 
//   This is the constructor for the QvisFileInformationWindow class.
//
// Arguments:
//   msgAtts : The MessageAttributes object that the window observes.
//   caption : The name of the window that appears in the window
//             decoration.
//   notepad : A pointer to the notepad area to which the window will post.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 20 13:56:24 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 10:47:21 PDT 2008
//   QString for caption and shortName.
//
// ****************************************************************************

QvisFileInformationWindow::QvisFileInformationWindow(FileServerList *fs,
    const QString &caption, const QString &shortName, QvisNotepadArea *notepad) : 
    QvisPostableWindowObserver(fs, caption, shortName, notepad,
                               QvisPostableWindowObserver::NoExtraButtons,
                               false)
{
    // nothing here
}

// ****************************************************************************
// Method: QvisFileInformationWindow::~QvisFileInformationWindow
//
// Purpose: 
//   Destructor for the QvisFileInformationWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 20 13:56:24 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisFileInformationWindow::~QvisFileInformationWindow()
{
    // nothing here
}

// ****************************************************************************
// Method: QvisFileInformationWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the window's widgets.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 20 13:56:24 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisFileInformationWindow::CreateWindowContents()
{
    // Create a multi line edit to display the text.
    outputText = new QMultiLineEdit(central, "outputText");
    outputText->setMinimumWidth(fontMetrics().width(titleSeparator));
    outputText->setWordWrap(QMultiLineEdit::NoWrap);
    outputText->setReadOnly(true);
    topLayout->addWidget(outputText);
}

// ****************************************************************************
// Method: QvisFileInformationWindow::UpdateWindow
//
// Purpose: 
//   Updates the output window when the FileServerList object that the
//   window watches is updated.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 20 13:56:24 PST 2002
//
// Modifications:
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interfaces to GetMetaData and GetSIL
//
//   Tom Fogal, Sat Aug 25 11:11:14 PDT 2007
//   Changed ostrstream to ostringstream
//
//   Cyrus Harrison, Mon Sep 10 10:08:31 PDT 2007
//   Added call to c_str() from ostringstream output to fix std:string
//   to QString linking error on AIX. 
//   (Also added modification message for Tom's recent change above)
//
//   Brad Whitlock, Tue Apr  8 12:26:44 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************



void
QvisFileInformationWindow::UpdateWindow(bool doAll)
{
    if(fileServer->FileChanged() || doAll)
    {
        const QualifiedFilename &qf = fileServer->GetOpenFile();
        const avtDatabaseMetaData *md =
            fileServer->GetMetaData(qf, GetStateForSource(qf),
                                   !FileServerList::ANY_STATE,
                                    FileServerList::GET_NEW_MD);

        if(md != 0)
        {

            std::ostringstream os;
            os << "File = " << fileServer->GetOpenFile().FullName().c_str()
               << endl;
            os << titleSeparator << endl;
            md->Print(os);
            outputText->setText(os.str().c_str());
        }
        else if(fileServer->GetOpenFile().Empty())
        {
            outputText->setText(tr("There is no open file. File information is "
                                "not available until a file has been opened."));
        }
        else
            outputText->setText(tr("VisIt could not read the file meta-data."));
        outputText->setCursorPosition(0, 0);
    }
}
