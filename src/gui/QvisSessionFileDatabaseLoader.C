// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Brad Whitlock, Wed Sep 12 12:14:10 PDT 2012
//   Fix the find tests so they do not erroneously return true.
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
        std::string msg(m->GetText());
        bool findCancelled = msg.find("cancelled") != std::string::npos; 
        bool findMDServer =  msg.find("metadata server") != std::string::npos;
        cancelled |= (findCancelled && findMDServer);
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
            if((size_t)index < databases.size())
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

