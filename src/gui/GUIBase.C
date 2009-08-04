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

#include <DebugStream.h>
#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <GUIBase.h>
#include <MessageAttributes.h>
#include <StatusSubject.h>
#include <ViewerProxy.h>
#include <FileServerList.h>
#include <GetMetaDataException.h>
#include <UnicodeHelper.h>
#include <WindowInformation.h>

#include <SimpleObserver.h>

#include <QObject>
#include <QApplication>
#include <QCursor>
#include <QLineEdit>

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
// Method: SetViewerProxy
//
// Purpose: 
//   Set the viewer proxy.
//
// Arguments:
//   p : The viewer proxy.
//
// Note:       We have this method now because I made the proxy private in
//             the base class because I want subclasses to use access methods
//             that would make it easier to combine the gui and viewer in
//             a single process if we ever decide to do that.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 13 09:29:47 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
GUIBase::SetViewerProxy(ViewerProxy *p)
{
    viewer = p;
}

// ****************************************************************************
// Method: GUIBase::GetViewerProxy
//
// Purpose: 
//   Get the ViewerProxy.
//
// Returns:    A pointer to the viewer proxy.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 13 09:31:03 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerProxy *
GUIBase::GetViewerProxy() const
{
    return viewer;
}

// ****************************************************************************
// Method: GUIBase::GetViewerState
//
// Purpose: 
//   Returns the ViewerState object.
//
// Returns:    The viewer proxy's ViewerState object.
//
// Note:       We provide this access method to make it less likely that 
//             subclasses will access ViewerState directly through the proxy.
//             This access pattern would make it easier to combine the gui
//             and viewer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 13 09:31:26 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerState *
GUIBase::GetViewerState() const
{
    return viewer->GetViewerState();
}

// ****************************************************************************
// Method: GUIBase::GetViewerMethods
//
// Purpose: 
//   Returns the GetViewerMethods object.
//
// Returns:    The viewer proxy's GetViewerMethods object.
//
// Note:       We provide this access method to make it less likely that 
//             subclasses will access GetViewerMethods directly through the .
//             proxy This access pattern would make it easier to combine the 
//             gui and viewer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 13 09:31:26 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerMethods *
GUIBase::GetViewerMethods() const
{
    return viewer->GetViewerMethods();
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
//   Brad Whitlock, Tue Apr 29 10:17:36 PDT 2008
//   Added support for sending the unicode string.
//
// ****************************************************************************

void
GUIBase::Error(const QString &msg)
{
    if(msgAttr == 0)
        return;

    if(writeToConsole)
    {
        cerr << "Error: " << msg.toStdString() << endl;
    }
    else
    {
        MessageAttributes_SetText(*msgAttr, msg);
        msgAttr->SetSeverity(MessageAttributes::Error);
        msgAttr->Notify();
    }

    // Write to the log as well.
    debug1 << "Error: " << msg.toStdString() << endl;
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
//   Brad Whitlock, Tue Apr 29 10:17:36 PDT 2008
//   Added support for sending the unicode string.
//
// ****************************************************************************

void
GUIBase::Warning(const QString &msg)
{
    if(msgAttr == 0)
        return;

    if(writeToConsole)
    {
        cerr << "Warning: " << msg.toStdString() << endl;
    }
    else
    {
        MessageAttributes_SetText(*msgAttr, msg);
        msgAttr->SetSeverity(MessageAttributes::Warning);
        msgAttr->Notify();
    }

    // Write to the log as well.
    debug2 << "Warning: " << msg.toStdString() << endl;
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
//   Brad Whitlock, Tue Apr 29 10:17:36 PDT 2008
//   Added support for sending the unicode string.
//
// ****************************************************************************

void
GUIBase::Message(const QString &msg)
{
    if(msgAttr == 0)
        return;

    if(writeToConsole)
    {
        cerr << "Message: " << msg.toStdString() << endl;
    }
    else
    {
        MessageAttributes_SetText(*msgAttr, msg);
        msgAttr->SetSeverity(MessageAttributes::Message);
        msgAttr->Notify();
    }

    // Write to the log as well.
    debug3 << "Message: " << msg.toStdString() << endl;
}

// ****************************************************************************
// Method: GUIBase::Information
//
// Purpose: 
//   This method writes an informative message into the message state
//   object. This causes the message to be displayed.
//
// Arguments:
//   msg : The text to put in the message.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 18 14:40:00 PST 2008
//
// Modifications:
//   Brad Whitlock, Tue Apr 29 10:17:36 PDT 2008
//   Added support for sending the unicode string.
//
// ****************************************************************************

void
GUIBase::Information(const QString &msg)
{
    if(msgAttr == 0)
        return;

    if(writeToConsole)
    {
        cerr << "Message: " << msg.toStdString() << endl;
    }
    else
    {
        MessageAttributes_SetText(*msgAttr, msg);
        msgAttr->SetSeverity(MessageAttributes::Information);
        msgAttr->Notify();
    }

    // Write to the log as well.
    debug3 << "Message: " << msg.toStdString() << endl;
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
//   Brad Whitlock, Tue Apr 6 08:26:37 PDT 2004
//   I removed some code that set globalAtts.
//
//   Hank Childs, Fri Jan 12 09:56:29 PST 2007
//   Do not issue an error if you can't open a file ... the viewer will do this
//   for us.
//
// ****************************************************************************

bool
GUIBase::SetOpenDataFile(const QualifiedFilename &qf, int timeState,
    SimpleObserver *sob, bool reOpen)
{
    const char *mName = "GUIBase::SetOpenDataFile";
    bool retval = true;

    //
    // Clears any information about the specified file and causes it to be
    // read again from the mdserver.
    //
    if(reOpen)
    {
        fileServer->ClearFile(qf);
        fileServer->CloseFile();
        if(sob)
            sob->SetUpdate(false);
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
            if(reOpen)
            {
                msg = QObject::tr("Reopening ",mName) + QString(qf.filename.c_str())
                    + QObject::tr(" on ", mName) + QString(qf.host.c_str());
            }
            else
            {
                msg = QObject::tr("Opening ", mName) + QString(qf.filename.c_str())
                    + QObject::tr(" on ", mName) + QString(qf.host.c_str());
            }
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
        }
        CATCH2(GetMetaDataException, gmde)
        {
            // Clear the status bar.
            ClearStatus();

            // Tell the user about the error.
            QString msg = QObject::tr("VisIt could not open the file %1.\n\n"
                                      "The metadata server returned the "
                                      "following message:\n\n%2", mName).
                          arg(QString(qf.FullName().c_str())).
                          arg(QString(gmde.Message().c_str()));
            debug1 << msg.toStdString().c_str() << endl;
            debug1 << "Not issuing an error message because the viewer will "
                   << "cover that." << endl;
            //Error(msg);
            retval = false;
        }
        ENDTRY
    }

    return retval;
}

// ****************************************************************************
// Method: GUIBase::GetStateForSource
//
// Purpose: 
//   Returns the state for the specified source taking into account the
//   active time slider and the correlations that exist.
//
// Arguments:
//   source : The name of the source whose state we want.
//
// Returns:    The current state for the specified source.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 27 21:16:59 PST 2004
//
// Modifications:
//   
// ****************************************************************************

int
GUIBase::GetStateForSource(const QualifiedFilename &source) const
{
    int retval = 0;

    WindowInformation *windowInfo = GetViewerState()->GetWindowInformation();
    int activeTS = windowInfo->GetActiveTimeSlider();

    if(activeTS >= 0)
    {
        // Try and find a correlation for the active time slider so we
        // can get the number of states in the correlation.
        const stringVector &tsNames = windowInfo->GetTimeSliders();
        const std::string &activeTSName = tsNames[activeTS];
        DatabaseCorrelationList *cL = GetViewerState()->GetDatabaseCorrelationList();
        DatabaseCorrelation *correlation = cL->FindCorrelation(activeTSName);
 
        //
        // We found a correlation for the active time slider.
        //
        if(correlation != 0)
        {
            //
            // See if the correlation for the active time slider involves
            // the source that we're interested in. If so, return the
            // correlated time state using the active time slider's state.
            //
            std::string sourceStr(source.FullName());
            const intVector &currentStates = windowInfo->GetTimeSliderCurrentStates();
            int activeTSState = currentStates[activeTS];
            int cts = correlation->GetCorrelatedTimeState(sourceStr, activeTSState);
            if(cts >= 0)
            {
                retval = cts;
            }
            else if((correlation = cL->FindCorrelation(sourceStr)) != 0)
            {
                //
                // The correlation did not involve the desired source but
                // we were able to find the simple correlation for the
                // desired source. Return its correlated time state using
                // the correlation that we just found and the active time
                // state for that correlation.
                //
                for(int i = 0; i < windowInfo->GetTimeSliders().size(); ++i)
                {
                    if(tsNames[i] == sourceStr)
                    {
                        cts = correlation->GetCorrelatedTimeState(sourceStr,
                                            currentStates[i]);
                        if(cts >= 0)
                            retval = cts;
                        break;
                    }
                }
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: GUIBase::GetTimeSliderStateForDatabaseState
//
// Purpose: 
//   Returns the first state in the current time slider where the database
//   has a specific database index.
//
// Arguments:
//   tsName  : The name of the active time slider.
//   source  : The source that we're interested in.
//   dbState : The database state that we're looking for.
//
// Returns:    The inverted correlation time state that we want.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 3 18:52:44 PST 2004
//
// Modifications:
//   Brad Whitlock, Mon May 3 16:51:20 PST 2004
//   I made the name of the time slider be passed in so the method can be a
//   little more general.
//
// ****************************************************************************

int
GUIBase::GetTimeSliderStateForDatabaseState(const std::string &activeTSName,
    const QualifiedFilename &source, int dbState) const
{
    int retval = dbState;

    if(activeTSName != "")
    {
        WindowInformation *windowInfo = GetViewerState()->GetWindowInformation();

        // Try and find a correlation for the active time slider so we
        // can get the number of states in the correlation.
        DatabaseCorrelationList *cL = GetViewerState()->GetDatabaseCorrelationList();
        DatabaseCorrelation *correlation = cL->FindCorrelation(activeTSName);
 
        //
        // We found a correlation for the active time slider.
        //
        if(correlation != 0)
        {
            //
            // See if the correlation for the active time slider involves
            // the source that we're interested in. If so, return the
            // inverse correlated time state, which is the correlation time
            // state where the specified database has the given dbState.
            //
            std::string sourceStr(source.FullName());
            int cts = correlation->GetInverseCorrelatedTimeState(sourceStr, dbState);
            if(cts >= 0)
                retval = cts;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: GUIBase::OpenActiveSourceInFileServer
//
// Purpose: 
//   This method is called when the file in the file server does not match
//   the active source. When that happens, we make the file server use the
//   active source so existing code that relies on the file server for the
//   open file still works.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 11:36:42 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
GUIBase::OpenActiveSourceInFileServer()
{
    WindowInformation *windowInfo = GetViewerState()->GetWindowInformation();
    const std::string &activeSource = windowInfo->GetActiveSource();
    QualifiedFilename qf(activeSource);

    if (activeSource == "notset" || activeSource == "")
    {
        debug3 << "OpenFileInFileServer: no active source. Closing file in "
                  "file server list. " << endl;
        fileServer->CloseFile();
        fileServer->Notify();
    }
    else if(fileServer->GetOpenFile() != qf)
    {
        TRY
        {
            // Get the state for the current source taking into account
            // the active time slider and the correlations that exist.
            int state = GetStateForSource(qf);
            debug3 << "OpenFileInFileServer: File server list's open file "
                   << "does not match the active source. Opening "
                   << activeSource.c_str() << ", state=" << state
                   << " in the file server list." << endl;

            fileServer->OpenFile(qf, state);
            fileServer->Notify();
        }
        CATCH(GetMetaDataException)
        {
            ; // Usually, the filename was bad.
        }
        ENDTRY
    }
}

// ****************************************************************************
// Method: GUIBase::ResettingError
//
// Purpose: 
//   Issues a very common error message used when input values are not valid.
//
// Arguments:
//   name : The name of the invalid property.
//   val  : The last good value.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:34:41 PDT 2008
//
// Modifications:
//   Cyrus Harrison,   Tue Aug 19 08:12:20 PDT 2008
//   Changed to value(s) to account for single value case.
//
// ****************************************************************************

void
GUIBase::ResettingError(const QString &name, const QString &val)
{
    Error(QObject::tr("The value(s) for '%1' were invalid. "
             "Resetting to the last good value(s) of %2.", "ResettingError").
             arg(name).arg(val));
}

// ****************************************************************************
// Method: GUIBase::DoublesToQString
//
// Purpose: 
//   Converts an array of doubles to a string wherein each double is space
//   delimited.
//
// Arguments:
//   vals : The values to convert to a string.
//   nvals : The number of values to convert.
//
// Returns:    A QString representation of the doubles.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:35:31 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QString
GUIBase::DoublesToQString(const double *vals, int nvals)
{
    QString s;
    for(int i = 0; i < nvals; ++i)
    {
        s += QString().setNum(vals[i]);
        if(i < nvals-1)
            s += " ";
    }
    return s;
}

// ****************************************************************************
// Method: GUIBase::DoublesToQString
//
// Purpose: 
//   Converts a vector of doubles to a string wherein each double is space
//   delimited.
//
// Arguments:
//   vals : The values to convert to a string.
//   nvals : The number of values to convert.
//
// Returns:    A QString representation of the doubles.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:35:31 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QString
GUIBase::DoublesToQString(const doubleVector &vals)
{
    QString s;
    for(size_t i = 0; i < vals.size(); ++i)
    {
        s += QString().setNum(vals[i]);
        if(i < vals.size()-1)
            s += " ";
    }
    return s;
}

// ****************************************************************************
// Method: GUIBase::DoubleToQString
//
// Purpose: 
//   Converts a double to a string.
//
// Arguments:
//   val : The value to convert to a string.
//
// Returns:    A QString representation of the doubles.
//
// Note:       This method is provided to make the single value case look
//             more like the array case for consistency.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:35:31 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QString
GUIBase::DoubleToQString(double val)
{
    return QString().setNum(val);
}

// ****************************************************************************
// Method: GUIBase::FloatsToQString
//
// Purpose: 
//   Converts an array of floats to a string wherein each double is space
//   delimited.
//
// Arguments:
//   vals : The values to convert to a string.
//   nvals : The number of values to convert.
//
// Returns:    A QString representation of the floats.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:35:31 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QString
GUIBase::FloatsToQString(const float *vals, int nvals)
{
    QString s;
    for(int i = 0; i < nvals; ++i)
    {
        s += QString().setNum(vals[i]);
        if(i < nvals-1)
            s += " ";
    }
    return s;
}

// ****************************************************************************
// Method: GUIBase::FloatToQString
//
// Purpose: 
//   Converts a float to a string.
//
// Arguments:
//   val : The value to convert to a string.
//
// Returns:    A QString representation of the float.
//
// Note:       This method is provided to make the single value case look
//             more like the array case for consistency.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:35:31 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QString
GUIBase::FloatToQString(float val)
{
    return QString().setNum(val);
}

// ****************************************************************************
// Method: GUIBase::IntsToQString
//
// Purpose: 
//   Converts an array of ints to a string wherein each double is space
//   delimited.
//
// Arguments:
//   vals : The values to convert to a string.
//   nvals : The number of values to convert.
//
// Returns:    A QString representation of the ints.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:35:31 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QString
GUIBase::IntsToQString(const int *vals, int nvals)
{
    QString s;
    for(int i = 0; i < nvals; ++i)
    {
        s += QString().setNum(vals[i]);
        if(i < nvals-1)
            s += " ";
    }
    return s;
}

// ****************************************************************************
// Method: GUIBase::IntsToQString
//
// Purpose: 
//   Converts a vector of ints to a string wherein each double is space
//   delimited.
//
// Arguments:
//   vals : The values to convert to a string.
//
// Returns:    A QString representation of the ints.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:35:31 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QString
GUIBase::IntsToQString(const intVector &vals)
{
    QString s;
    for(size_t i = 0; i < vals.size(); ++i)
    {
        s += QString().setNum(vals[i]);
        if(i < vals.size()-1)
            s += " ";
    }
    return s;
}

// ****************************************************************************
// Method: GUIBase::IntToQString
//
// Purpose: 
//   Converts an int to a string.
//
// Arguments:
//   val : The value to convert to a string.
//
// Returns:    A QString representation of the int.
//
// Note:       This method is provided to make the single value case look
//             more like the array case for consistency.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:35:31 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QString
GUIBase::IntToQString(int val)
{
    return QString().setNum(val);
}

// ****************************************************************************
// Method: GUIBase::LineEditGetDoubles
//
// Purpose: 
//   Converts a line edit's text into an array of doubles.
//
// Arguments:
//   lineEdit : The line edit widget whose text we want to extract.
//   vals     : The destination array for the doubles.
//   maxVals  : The size of the destination array.
//
// Returns:    True if maxVals doubles were read; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:38:12 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
GUIBase::LineEditGetDoubles(QLineEdit *lineEdit, double *vals, int maxVals)
{
    return QStringToDoubles(lineEdit->displayText().trimmed(), vals, maxVals);
}

// ****************************************************************************
// Method: GUIBase::LineEditGetDoubles
//
// Purpose: 
//   Converts a line edit's text into an array of doubles.
//
// Arguments:
//   lineEdit : The line edit widget whose text we want to extract.
//   vals     : The destination vector for the doubles.
//   maxVals  : The size of the destination array.
//
// Returns:    True if maxVals doubles were read; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:38:12 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
GUIBase::LineEditGetDoubles(QLineEdit *lineEdit, doubleVector &vals, int maxVals)
{
    return QStringToDoubles(lineEdit->displayText().trimmed(), vals, maxVals);
}

// ****************************************************************************
// Method: GUIBase::LineEditGetDouble
//
// Purpose: 
//   Converts a line edit's text into a double.
//
// Arguments:
//   lineEdit : The line edit widget whose text we want to extract.
//   val      : The value in which to store the double.
//
// Returns:    True if a double was read; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:38:12 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
GUIBase::LineEditGetDouble(QLineEdit *lineEdit, double &val)
{
    bool okay = false;
    val = lineEdit->displayText().trimmed().toDouble(&okay);
    return okay;
}

// ****************************************************************************
// Method: GUIBase::LineEditGetFloats
//
// Purpose: 
//   Converts a line edit's text into an array of floats.
//
// Arguments:
//   lineEdit : The line edit widget whose text we want to extract.
//   vals     : The destination array for the floats.
//   maxVals  : The size of the destination array.
//
// Returns:    True if maxVals floats were read; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:38:12 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
GUIBase::LineEditGetFloats(QLineEdit *lineEdit, float *vals, int maxVals)
{
    return QStringToFloats(lineEdit->displayText().trimmed(), vals, maxVals);
}

// ****************************************************************************
// Method: GUIBase::LineEditGetFloat
//
// Purpose: 
//   Converts a line edit's text into a float.
//
// Arguments:
//   lineEdit : The line edit widget whose text we want to extract.
//   val      : The value in which to store the float.
//
// Returns:    True if a float was read; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:38:12 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
GUIBase::LineEditGetFloat(QLineEdit *lineEdit, float &val)
{
    bool okay = false;
    val = lineEdit->displayText().trimmed().toFloat(&okay);
    return okay;
}

// ****************************************************************************
// Method: GUIBase::LineEditGetInts
//
// Purpose: 
//   Converts a line edit's text into an array of Ints.
//
// Arguments:
//   lineEdit : The line edit widget whose text we want to extract.
//   vals     : The destination array for the Ints.
//   maxVals  : The size of the destination array.
//
// Returns:    True if maxVals Ints were read; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:38:12 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
GUIBase::LineEditGetInts(QLineEdit *lineEdit, int *vals, int maxVals)
{
    return QStringToInts(lineEdit->displayText().trimmed(), vals, maxVals);
}

// ****************************************************************************
// Method: GUIBase::LineEditGetInts
//
// Purpose: 
//   Converts a line edit's text into an array of Ints.
//
// Arguments:
//   lineEdit : The line edit widget whose text we want to extract.
//   vals     : The destination vector for the Ints.
//   maxVals  : The size of the destination array.
//
// Returns:    True if maxVals Ints were read; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:38:12 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
GUIBase::LineEditGetInts(QLineEdit *lineEdit, intVector &vals, int maxVals)
{
    return QStringToInts(lineEdit->displayText().trimmed(), vals, maxVals);
}

// ****************************************************************************
// Method: GUIBase::LineEditGetInt
//
// Purpose: 
//   Converts a line edit's text into an int.
//
// Arguments:
//   lineEdit : The line edit widget whose text we want to extract.
//   val      : The value in which to store the int.
//
// Returns:    True if a int was read; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:38:12 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
GUIBase::LineEditGetInt(QLineEdit *lineEdit, int &val)
{
    bool okay = false;
    val = lineEdit->displayText().trimmed().toInt(&okay);
    return okay;
}

// ****************************************************************************
// Method: GUIBase::QStringToDoubles
//
// Purpose: 
//   Converts a QString into an array of doubles.
//
// Arguments:
//   str     : The string from which to extract the doubles.
//   vals    : The destination array for the doubles.
//   maxVals : The number of values in the destination array.
//
// Returns:  True if maxVals doubles were created from the string. False otherwise.  
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:41:41 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
GUIBase::QStringToDoubles(const QString &str, double *vals, int maxVals)
{
    bool retval = false;

    if(!str.isEmpty())
    {
        bool okay = true;
        QStringList s = str.split(" ", QString::SkipEmptyParts);
        for(int i = 0; i < maxVals && okay; ++i)
        {
            if(i < s.size())
                vals[i] = s[i].toDouble(&okay);
            else
                vals[i] = 0.;
        }
        retval = okay;
    }

    return retval;
}

// ****************************************************************************
// Method: GUIBase::QStringToDoubles
//
// Purpose: 
//   Converts a QString into an array of doubles.
//
// Arguments:
//   str     : The string from which to extract the doubles.
//   vals    : The destination array for the doubles.
//   maxVals : The number of values in the destination array.
//
// Returns:  True if maxVals doubles were created from the string. False otherwise.  
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:41:41 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
GUIBase::QStringToDoubles(const QString &str, doubleVector &vals, int maxVals)
{
    bool retval = false;

    if(!str.isEmpty())
    {
        bool okay = true;
        QStringList s = str.split(" ", QString::SkipEmptyParts);
        for(int i = 0; i < s.size() && okay; ++i)
        {
            if(maxVals == -1 || i < maxVals)
                vals.push_back(s[i].toDouble(&okay));
            else
                vals.push_back(0.);
        }
        retval = okay;
    }

    return retval;
}

// ****************************************************************************
// Method: GUIBase::QStringToFloats
//
// Purpose: 
//   Converts a QString into an array of floats.
//
// Arguments:
//   str     : The string from which to extract the floats.
//   vals    : The destination array for the floats.
//   maxVals : The number of values in the destination array.
//
// Returns:  True if maxVals floats were created from the string. False otherwise.  
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:41:41 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
GUIBase::QStringToFloats(const QString &str, float *vals, int maxVals)
{
    bool retval = false;

    if(!str.isEmpty())
    {
        bool okay = true;
        QStringList s = str.split(" ", QString::SkipEmptyParts);
        for(int i = 0; i < maxVals && okay; ++i)
        {
            if(i < s.size())
                vals[i] = s[i].toFloat(&okay);
            else
                vals[i] = 0.;
        }
        retval = okay;
    }

    return retval;
}

// ****************************************************************************
// Method: GUIBase::QStringToInts
//
// Purpose: 
//   Converts a QString into an array of ints.
//
// Arguments:
//   str     : The string from which to extract the ints.
//   vals    : The destination array for the ints.
//   maxVals : The number of values in the destination array.
//
// Returns:  True if maxVals ints were created from the string. False otherwise.  
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:41:41 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
GUIBase::QStringToInts(const QString &str, int *vals, int maxVals)
{
    bool retval = false;

    if(!str.isEmpty())
    {
        bool okay = true;
        QStringList s = str.split(" ", QString::SkipEmptyParts);
        for(int i = 0; i < maxVals && okay; ++i)
        {
            if(i < s.size())
                vals[i] = s[i].toInt(&okay);
            else
                vals[i] = 0;
        }
        retval = okay;
    }

    return retval;
}

// ****************************************************************************
// Method: GUIBase::QStringToInts
//
// Purpose: 
//   Converts a QString into a vector of ints.
//
// Arguments:
//   str     : The string from which to extract the ints.
//   vals    : The destination array for the ints.
//   maxVals : The number of values in the destination array.
//
// Returns:  True if maxVals ints were created from the string. False otherwise.  
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 10:41:41 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
GUIBase::QStringToInts(const QString &str, intVector &vals, int maxVals)
{
    bool retval = false;

    if(!str.isEmpty())
    {
        bool okay = true;
        QStringList s = str.split(" ", QString::SkipEmptyParts);
        for(int i = 0; i < s.size() && okay; ++i)
        {
            if(maxVals == -1 || i < maxVals)
                vals.push_back(s[i].toInt(&okay));
            else
                vals.push_back(0);
        }
        retval = okay;
    }

    return retval;
}
