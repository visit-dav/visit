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
#include <QvisFileOpenDialog.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qwidgetlist.h>

#include <ViewerProxy.h>
#include <FileServerList.h>
#include <HostProfileList.h>
#include <DBPluginInfoAttributes.h>

#include <BadHostException.h>
#include <ChangeDirectoryException.h>

const int QvisFileOpenDialog::Accepted = 0;
const int QvisFileOpenDialog::Rejected = 1;

// ****************************************************************************
// Method: QvisFileOpenDialog::QvisFileOpenDialog
//
// Purpose:
//   Constructor for the QvisFileOpenDialog class.
//
// Arguments:
//   caption : The name of the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 15 16:28:11 PST 2006
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 10:40:27 PDT 2008
//   Made caption use QString.
//
// ****************************************************************************

QvisFileOpenDialog::QvisFileOpenDialog(const QString &caption) : 
    QvisFileOpenWindow(caption)
{
    in_loop = false;
    _result = Rejected;

    ConnectSubjects(GetViewerState()->GetHostProfileList(),
                    GetViewerState()->GetDBPluginInfoAttributes());

    connect(this, SIGNAL(selectedFile(const QString &)),
            this, SLOT(userSelectedFile(const QString &)));
    connect(this, SIGNAL(selectCancelled()),
            this, SLOT(reject()));
}

// ****************************************************************************
// Method: QvisFileOpenDialog::~QvisFileOpenDialog
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 15 16:28:39 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisFileOpenDialog::~QvisFileOpenDialog()
{
}

//
// Set/Get methods.
//

void
QvisFileOpenDialog::setFilename(const QString &s)
{
    filename = s;
}

QString
QvisFileOpenDialog::getFilename() const
{
    return filename;
}

int
QvisFileOpenDialog::result() const
{
    return _result;
}

void
QvisFileOpenDialog::setResult(int r)
{
    _result = r;
}

// ****************************************************************************
// Method: QvisFileOpenDialog::delayedChangePath
//
// Purpose: 
//   This method tells the dialog to change its host and path once we're
//   back in an event loop.
//
// Arguments:
//   initialFile : The host and path that we're using.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 15 16:29:09 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileOpenDialog::delayedChangePath(const QString &initialFile)
{
    filename = initialFile;
    QTimer::singleShot(500, this, SLOT(changeThePath()));
}

// ****************************************************************************
// Method: QvisFileOpenDialog::getOpenFileNameEx
//
// Purpose: 
//   This method enters into the exec function and waits until the user
//   does something with the File open window.
//
// Arguments:
//   initialFile : The host and path that we're opening.
//
// Returns:    The full filename selected by the user.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 15 16:30:43 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QString
QvisFileOpenDialog::getOpenFileNameEx(const QString &initialFile)
{
    QString ret;

    // The entire state of the file open window depends on the file server
    // so let's back up the file server's state so we can update it.
    std::string host(fileServer->GetHost());
    std::string path(fileServer->GetPath());
    std::string filter(fileServer->GetFilter());

    // Set up a delayed order to change the path.
    delayedChangePath(initialFile);

    // Set the return value for the method based on the event loop return value.
    if(exec() == Accepted)
        ret = filename;

    // Try and restore the host,path,filter into the file server.
    TRY
    {
        fileServer->SetHost(host);
        fileServer->SetPath(path);
        fileServer->SetFilter(filter);
        fileServer->Notify();
    }
    CATCH(VisItException)
    {
    }
    ENDTRY

    return filename;
}

// ****************************************************************************
// Method: QvisFileOpenDialog::getOpenFileName
//
// Purpose: 
//   Static function for getting the name of an existing file using VisIt's
//   File open window.
//
// Arguments:
//   initialFile : The host, path, and filename of the initial file.
//   caption     : The name of the window.
//
// Returns:    The name of the file that the user selected or a null QString if
//             the user did not select a file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 15 16:33:30 PST 2006
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 10:40:51 PDT 2008
//   Made caption use QString.
//  
// ****************************************************************************

QString
QvisFileOpenDialog::getOpenFileName(const QString &initialFile, 
    const QString &caption)
{
    QString filename;

    // Create a new file open window
    QvisFileOpenDialog *dlg = new QvisFileOpenDialog(caption);
    dlg->SetUsageMode(QvisFileOpenDialog::SelectFilename);
    filename = dlg->getOpenFileNameEx(initialFile);
    dlg->deleteLater();

    return filename;
}

//
// Qt slot functions.
//

// ****************************************************************************
// Method: QvisFileOpenDialog::changeThePath
//
// Purpose: 
//   Changes the file server to the specified host and path.
//
// Note:       This is called on a single shot timer from the event loop.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 15 16:36:28 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Jan 22 11:39:38 PST 2008
//   Added code to handle bad hosts and paths so the window does not disappear
//   outright.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisFileOpenDialog::changeThePath()
{
    // Set the host and path to that of the initial file.
    QualifiedFilename f(filename.latin1());
    bool retry_loop = false;
    int nTries = 0;
    QString msg;
    do
    {
        TRY
        {
            fileServer->SetHost(f.host);
            fileServer->SetPath(f.path);
            fileServer->Notify();
            retry_loop = false;
        }
        CATCH(BadHostException)
        {
            msg = tr("VisIt could not access host %1.");
            msg.replace("%1", f.host.c_str());
            Error(msg);
            f.host = "localhost";
            retry_loop = true;
        }
        CATCH(ChangeDirectoryException)
        {
            msg = tr("VisIt could not access the directory: %1 so your "
                     "home directory will be used.");
            msg.replace("%1", f.path.c_str());
            Error(msg);
            f.path = "~";
            retry_loop = true;
        }
        CATCH(VisItException)
        {
            reject();
            retry_loop = false;
        }
        ENDTRY
        ++nTries;
    } while(retry_loop && nTries < 4);
}

// ****************************************************************************
// Method: QvisFileOpenDialog::userSelectedFile
//
// Purpose: 
//   This is a Qt slot function that gets called when the user selects a file
//   from the File open window. We use it to store that filename and call
//   accept() to break out of the event loop and return from the static
//   getOpenFileName function.
//
// Arguments:
//   s : The name of the file that was selected.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 15 16:37:14 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileOpenDialog::userSelectedFile(const QString &s)
{
    filename = s;
    accept();
}

//
// Methods borrowed from QDialog to dialog-ize the File open window.
//

//
// Enters a sub-event loop to wait until the user accepts or rejects
// the dialog.
//
int
QvisFileOpenDialog::exec()
{
    if ( in_loop ) {
    qWarning( "QDialog::exec: Recursive call detected" );
    return -1;
    }

    bool destructiveClose = testWFlags( WDestructiveClose );
    clearWFlags( WDestructiveClose );

    bool wasShowModal = testWFlags( WShowModal );
    setWFlags( WShowModal );
    setResult( Rejected );

    show();

    in_loop = TRUE;
    qApp->enter_loop();

    if ( !wasShowModal )
    clearWFlags( WShowModal );

    int res = result();

    if ( destructiveClose )
    delete this;

    return res;
}


/*! Closes the dialog and sets its result code to \a r. If this dialog
  is shown with exec(), done() causes the local event loop to finish,
  and exec() to return \a r.

  As with QWidget::close(), done() deletes the dialog if the \c
  WDestructiveClose flag is set. If the dialog is the application's
  main widget, the application terminates. If the dialog is the
  last window closed, the QApplication::lastWindowClosed() signal is
  emitted.

  \sa accept(), reject(), QApplication::mainWidget(), QApplication::quit()
*/

void
QvisFileOpenDialog::done( int r )
{
    if(in_loop)
    {
        in_loop = false;
        qApp->exit_loop();
    }

    hide();
    setResult( r );

    // emulate QWidget::close()
    bool isMain = qApp->mainWidget() == this;
    if ( isMain )
    qApp->quit();
    else if ( testWFlags(WDestructiveClose) ) {
    clearWFlags(WDestructiveClose);
    deleteLater();
    }
}

void
QvisFileOpenDialog::accept()
{
    done( Accepted );
}

void
QvisFileOpenDialog::reject()
{
    done( Rejected );
}
