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

#ifndef QVIS_OUTPUT_WINDOW_H
#define QVIS_OUTPUT_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>

// Forward declarations.
class QString;
class QTextEdit;
class MessageAttributes;

// ****************************************************************************
// Class: QvisOutputWindow
//
// Purpose:
//   This class describes an output window. An output window is
//   basically a dialog box that has a large, scrollable, multi-line
//   edit text field and a dismiss button. It is used to display
//   big messages or the contents of a file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 27 17:25:18 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Sep 6 12:09:37 PDT 2001
//   Made it inherit from QvisPostableWindowObserver.
//
//   Brad Whitlock, Wed Apr  9 10:53:09 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

class GUI_API QvisOutputWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisOutputWindow(MessageAttributes *subj,
                     const QString &caption = QString::null,
                     const QString &shortName = QString::null,
                     QvisNotepadArea *notepad = 0);
    virtual ~QvisOutputWindow();
    virtual void CreateWindowContents();
signals:
    void unreadOutput(bool);
protected:
    virtual void UpdateWindow(bool doAll);
private:
    QTextEdit *outputText;
};

#endif
