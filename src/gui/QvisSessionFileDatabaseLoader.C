#include <QvisSessionFileDatabaseLoader.h>
#include <ViewerProxy.h>
#include <MessageAttributes.h>
#include <Observer.h>
#include <DebugStream.h>

#include <qtimer.h>

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
//   
// ****************************************************************************

QvisSessionFileDatabaseLoader::QvisSessionFileDatabaseLoader(QObject *parent, const char *name) :
    QObject(parent, name), GUIBase(), databases(), cancelledObserver(),
    sessionFile()
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
            viewer->GetMessageAttributes());
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
                debug1 << mName << "telling VisIt that is can load the "
                          "session file: " << sessionFile.latin1() << endl;
                
                emit complete(sessionFile);
            }
        }
        else
        {
            debug1 << mName << "Not opening any more databases or the "
                      "session file because of a cancel" << endl;
            Warning("The launch of a metadata server for accessing data files "
                    "relevant to the requested session file was cancelled so "
                    "the session file will not be opened.");
        }
    }
}

