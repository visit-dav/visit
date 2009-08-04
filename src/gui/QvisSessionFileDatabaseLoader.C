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

#include <QvisSessionFileDatabaseLoader.h>
#include <ViewerProxy.h>
#include <MessageAttributes.h>
#include <Observer.h>
#include <DebugStream.h>

#include <QTimer>

// ****************************************************************************
// Class: CancelledObserver
//
// Purpose:
//   This class scans the message attributes for "metadata server" and
//   "cancelled" since those strings appear in the message sent by the viewer
//   when the remote metadata server launch is cancelled.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 27 16:22:13 PST 2005
//
// Modifications:
//   
// ****************************************************************************

class CancelledObserver : public Observer
{
public:
    CancelledObserver(Subject *s) : Observer(s)
    {
        cancelled = false;
    }

    virtual ~CancelledObserver()
    {
    }

    virtual void Update(Subject *s)
    {
        MessageAttributes *m = (MessageAttributes *)s;
        cancelled |= m->GetText().find("cancelled") &&
                     m->GetText().find("metadata server");
    }

    bool cancelled;
};

// ****************************************************************************
// Method: QvisSessionFileDatabaseLoader::QvisSessionFileDatabaseLoader
//
// Purpose: 
//   Constructor for the QvisSessionFileDatabaseLoader class.
//
// Arguments:
//   parent : The object's parent.
//   name   : The object's name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 27 16:23:13 PST 2005
//
// Modifications:
//   Cyrus Harrison, Tue Jul  1 09:14:16 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

QvisSessionFileDatabaseLoader::QvisSessionFileDatabaseLoader(QObject *parent) 
:  QObject(parent), GUIBase(), databases(), cancelledObserver(), sessionFile()
{
    index = 0;
    cancelledObserver = 0;
}

// ****************************************************************************
// Method: QvisSessionFileDatabaseLoader::~QvisSessionFileDatabaseLoader
//
// Purpose: 
//   Destructor for the QvisSessionFileDatabaseLoader class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 27 16:23:40 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QvisSessionFileDatabaseLoader::~QvisSessionFileDatabaseLoader()
{
    if(cancelledObserver != 0)
        delete cancelledObserver;
}

// ****************************************************************************
// Method: QvisSessionFileDatabaseLoader::SetDatabases
//
// Purpose: 
//   This method sets the list of databases to be opened.
//
// Arguments:
//   s : The new list of databases.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 27 16:24:20 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void 
QvisSessionFileDatabaseLoader::SetDatabases(const stringVector &s)
{
    databases = s;
}

// ****************************************************************************
// Method: QvisSessionFileDatabaseLoader::Start
//
// Purpose: 
//   Starts loading the files one at a time while also looking for mdserver
//   cancellations.
//
// Arguments:
//   sfn : The name of the session file to restore if we are able to open
//         all of the databases.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 27 16:24:46 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSessionFileDatabaseLoader::Start(const QString &sfn)
{
    index = 0;
    sessionFile = sfn;
    if(cancelledObserver == 0)
        cancelledObserver = new CancelledObserver(
            GetViewerState()->GetMessageAttributes());
    cancelledObserver->cancelled = false;

    ProcessFile();
}

//
// Qt slots
//

// ****************************************************************************
// Method: QvisSessionFileDatabaseLoader::ProcessFile
//
// Purpose: 
//   This is a Qt slot function that processes the next database in the list
//   of databases.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 27 16:25:44 PST 2005
//
// Modifications:
//   Brad Whitlock, Tue Nov 14 15:21:02 PST 2006
//   I made this method emit a new complete signal that sends the databases
//   vector in the argument list.
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisSessionFileDatabaseLoader::ProcessFile()
{
    const char *mName = "QvisSessionFileDatabaseLoader::ProcessFile: ";
    if(databases.size() > 0)
    {
        debug1 << mName << "telling VisIt to load: "
               << databases[index].c_str() << endl;

        emit loadFile(databases[index].c_str());

        ++index;
        if(!cancelledObserver->cancelled)
        {
            if(index < databases.size())
            {
                QTimer::singleShot(100, this, SLOT(ProcessFile()));
            }
            else
            {
                debug1 << mName << "telling VisIt that it can load the "
                          "session file: " << sessionFile.toStdString() << endl;
                
                emit complete(sessionFile);
                emit complete(sessionFile, databases);
            }
        }
        else
        {
            debug1 << mName << "Not opening any more databases or the "
                      "session file because of a cancel" << endl;
            Warning(tr("The launch of a metadata server for accessing data files "
                    "relevant to the requested session file was cancelled so "
                    "the session file will not be opened."));
        }
    }
}

