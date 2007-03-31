#include <DebugStream.h>
#include <GUIBase.h>
#include <MessageAttributes.h>
#include <StatusSubject.h>
#include <ViewerProxy.h>
#include <FileServerList.h>
#include <GetMetaDataException.h>
#include <GlobalAttributes.h>
#include <SimpleObserver.h>

#include <qapplication.h>
#include <qcursor.h>

// Static member attributes.
MessageAttributes *GUIBase::msgAttr = 0;
ViewerProxy *GUIBase::viewer = 0;
StatusSubject *GUIBase::statusSubject = 0;
bool GUIBase::writeToConsole = true;
bool GUIBase::autoUpdate = false;
bool GUIBase::makeDefaultConfirm = true;
FileServerList *GUIBase::fileServer = 0;

// ****************************************************************************
// Method: GUIBase::GUIBase
//
// Purpose: 
//   Constructor for the GUIBase class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 16:43:57 PST 2000
//
// Modifications:
//   
// ****************************************************************************

GUIBase::GUIBase()
{
    // nothing
}

// ****************************************************************************
// Method: GUIBase::~GUIBase
//
// Purpose: 
//   Destructor for the GUIBase class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 16:43:57 PST 2000
//
// Modifications:
//   
// ****************************************************************************

GUIBase::~GUIBase()
{
    // nothing
}

// ****************************************************************************
// Method: GUIBase::Error
//
// Purpose: 
//   This method writes an error message into the message state
//   object. This causes the error message to be displayed.
//
// Arguments:
//   msg : The text to put in the message.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 18:15:49 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Apr 28 15:49:58 PST 2003
//   Added ability to write to the console.
//
//   Brad Whitlock, Tue May 20 15:12:44 PST 2003
//   Made it work with the regenerated MessageAttributes.
//
// ****************************************************************************

void
GUIBase::Error(const QString &msg)
{
    if(msgAttr == 0)
        return;

    if(writeToConsole)
    {
        cerr << "Error: " << msg.latin1() << endl;
    }
    else
    {
        msgAttr->SetText(std::string(msg.latin1()));
        msgAttr->SetSeverity(MessageAttributes::Error);
        msgAttr->Notify();
    }

    // Write to the log as well.
    debug1 << "Error: " << msg.latin1() << endl;
}

// ****************************************************************************
// Method: GUIBase::Warning
//
// Purpose: 
//   This method writes a warning message into the message state
//   object. This causes the warning message to be displayed.
//
// Arguments:
//   msg : The text to put in the message.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 18:15:49 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Apr 28 15:49:58 PST 2003
//   Added ability to write to the console.
//
//   Brad Whitlock, Tue May 20 15:12:44 PST 2003
//   Made it work with the regenerated MessageAttributes.
//
// ****************************************************************************

void
GUIBase::Warning(const QString &msg)
{
    if(msgAttr == 0)
        return;

    if(writeToConsole)
    {
        cerr << "Warning: " << msg.latin1() << endl;
    }
    else
    {
        msgAttr->SetText(std::string(msg.latin1()));
        msgAttr->SetSeverity(MessageAttributes::Warning);
        msgAttr->Notify();
    }

    // Write to the log as well.
    debug2 << "Warning: " << msg.latin1() << endl;
}

// ****************************************************************************
// Method: GUIBase::Message
//
// Purpose: 
//   This method writes a message into the message state
//   object. This causes the message to be displayed.
//
// Arguments:
//   msg : The text to put in the message.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 18:15:49 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Apr 28 15:49:58 PST 2003
//   Added ability to write to the console.
//
//   Brad Whitlock, Tue May 20 15:12:44 PST 2003
//   Made it work with the regenerated MessageAttributes.
//
// ****************************************************************************

void
GUIBase::Message(const QString &msg)
{
    if(msgAttr == 0)
        return;

    if(writeToConsole)
    {
        cerr << "Message: " << msg.latin1() << endl;
    }
    else
    {
        msgAttr->SetText(std::string(msg.latin1()));
        msgAttr->SetSeverity(MessageAttributes::Message);
        msgAttr->Notify();
    }

    // Write to the log as well.
    debug3 << "Message: " << msg.latin1() << endl;
}

// ****************************************************************************
// Method: GUIBase::Status
//
// Purpose: 
//   Sends a status message to the main window that causes it to
//   display the message in its statusbar.
//
// Arguments:
//   msg         : The message to display.
//   millisecond : The number of milliseconds to display the message.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 16 14:27:56 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
GUIBase::Status(const QString &msg, int milliseconds)
{
    // Set the status subject's fields and notify its observer,
    // the main window.
    statusSubject->clearing = false;
    statusSubject->text = msg;
    statusSubject->milliseconds = milliseconds;
    statusSubject->Notify();
}

// ****************************************************************************
// Method: GUIBase::ClearStatus
//
// Purpose: 
//   Sends a status message to the main window that causes it to
//   clear the message in its statusbar.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 16 14:29:00 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
GUIBase::ClearStatus()
{
    // Set the status subject's fields and notify its observer,
    // the main window.
    statusSubject->clearing = true;
    statusSubject->Notify();
}

// ****************************************************************************
// Method: GUIBase::AutoUpdate
//
// Purpose: 
//   Returns a flag indicating whether or not autoupdate is on.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 15:04:32 PST 2000
//
// Modifications:
//   
// ****************************************************************************

bool
GUIBase::AutoUpdate() const
{
    return autoUpdate;
}

// ****************************************************************************
// Method: GUIBase::ConnectMessageAttr
//
// Purpose: 
//   Sets the message object that is used to tell the message window
//   that there is a message.
//
// Arguments:
//   attr : A pointer to the message state object that is used to
//          communicate error messages.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 18:17:48 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
GUIBase::ConnectMessageAttr(MessageAttributes *attr)
{
    msgAttr = attr;
}

// ****************************************************************************
// Method: GUIBase::SetWaitCursor
//
// Purpose: 
//   Sets the cursor to a wait cursor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 13 14:21:34 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
GUIBase::SetWaitCursor()
{
#if QT_VERSION >= 300
    QApplication::setOverrideCursor(Qt::WaitCursor);
#else
    QApplication::setOverrideCursor(WaitCursor);
#endif
}

// ****************************************************************************
// Method: GUIBase::RestoreCursor
//
// Purpose: 
//   Restores the cursor to the last cursor used.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 13 14:21:34 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
GUIBase::RestoreCursor()
{
    QApplication::restoreOverrideCursor();
}

// ****************************************************************************
// Method: GUIBase::SetOpenDataFile
//
// Purpose: 
//   Opens the specified data file using the file server.
//
// Arguments:
//   qf              : The qualified file name that we want to open.
//   timeState       : The time state that we want to open.
//   sob             : The caller of this method. We pass it if we do not want
//                     the caller to update as a result of calling this method.
//   reOpen          : Whether we should reopen the database instead of opening.
//
// Returns:    True if the operation is a success.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 9 16:29:48 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Jul 29 14:39:54 PST 2002
//   I added the reOpen argument to tell the file server to re-open the
//   file and clear anything that it knows about the file.
//
//   Brad Whitlock, Mon Aug 19 17:02:00 PST 2002
//   I removed the code to tell the viewer the number of time steps that
//   are in the animation because the viewer now knows that information and
//   it was causing plots to recalculate when viewer pipeline caching was
//   turned off.
//
//   Eric Brugger, Mon Dec 16 12:32:14 PST 2002
//   I removed the code that sets the number of states and the current state
//   in the global attributes.  This was unnecessary since the viewer is
//   responsible for doing that.
//
//   Brad Whitlock, Wed May 14 13:02:50 PST 2003
//   I added support for reopening a file at a later time state.
//
//   Brad Whitlock, Thu May 15 12:34:06 PDT 2003
//   I added support for opening a file at a later time state.
//
//   Brad Whitlock, Wed Oct 22 12:16:43 PDT 2003
//   I added support for telling the viewer that we don't want to add
//   default plots when we open a database.
//
//   Brad Whitlock, Mon Nov 3 09:34:03 PDT 2003
//   I added code to prevent the file panel from updating prematurely. I also
//   renamed the method and ripped out the code that told the viewer to
//   open the database. I removed the addDefaultPlots argument.
//
// ****************************************************************************

bool
GUIBase::SetOpenDataFile(const QualifiedFilename &qf, int timeState,
    SimpleObserver *sob, bool reOpen)
{
    bool retval = true;
    GlobalAttributes *globalAtts = viewer->GetGlobalAttributes();

    //
    // Clears any information about the specified file and causes it to be
    // read again from the mdserver.
    //
    if(reOpen)
    {
        fileServer->ClearFile(qf);
        fileServer->CloseFile();
        fileServer->Notify();
    }

    //
    // If the file is not already open, read it in and tell the observers.
    //
    if(fileServer->GetOpenFile() != qf)
    {
        TRY
        {
            // Display a message while we open the file.
            QString msg;
            msg.sprintf("Opening %s on %s", qf.filename.c_str(),
                qf.host.c_str());
            Status(msg);

            // Tell the fileServer to open the file specified by filename.
            // This will cause it to read the file's MetaData.
            fileServer->OpenFile(qf, timeState);
            // Prevent the file panel from updating because the globalAtts
            // from the viewer, that the file panel uses to display the right
            // database highlight, do not yet contain the right timeState and
            // updating the file panel now causes it to switch to the wrong
            // time state for a moment.
            if(sob)
                sob->SetUpdate(false);
            fileServer->Notify();
            ClearStatus();

            // Set some important values in the globalAtts and tell the viewer.
            globalAtts->SetCurrentFile(qf.FullName());
            if(sob)
                sob->SetUpdate(false);
            globalAtts->Notify();
        }
        CATCH2(GetMetaDataException, gmde)
        {
            // Clear the status bar.
            ClearStatus();

            // Tell the user about the error.
            QString msg;
            msg.sprintf("VisIt could not open the file %s.\n\nThe metadata "
                        "server returned the following message:\n\n%s",
                        qf.FullName().c_str(), gmde.GetMessage().c_str());
            Error(msg);
            retval = false;
        }
        ENDTRY
    }

    return retval;
}
