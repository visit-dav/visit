#ifndef GUI_BASE_H
#define GUI_BASE_H
#include <gui_exports.h>
#include <string>

// Forward declarations
class QString;
class FileServerList;
class MessageAttributes;
class ViewerProxy;
struct StatusSubject;
struct QualifiedFilename;
class SimpleObserver;

// ****************************************************************************
// Class: GUIBase
//
// Purpose:
//   This class contains useful methods that are needed in all windows
//   and all widgets, for instance, error messages, status bar stuff.
//   They all need a pointer to the viewer proxy too. This class is
//   therefore a base class for all of those classes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 16:40:18 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Aug 23 8:31:23 PDT 2001
//   Added makeDefaultConfirm attribute.
//
//   Brad Whitlock, Wed Mar 13 14:21:34 PST 2002
//   Added methods to set the cursor.
//
//   Brad Whitlock, Thu May 9 16:26:29 PST 2002
//   Made fileServer a static member and added OpenDataFile function.
//
//   Brad Whitlock, Mon Jul 29 14:39:30 PST 2002
//   I added a reOpen argument to OpenDataFile.
//
//   Brad Whitlock, Thu May 15 12:33:11 PDT 2003
//   I added a timeState argument to OpenDataFile.
//
//   Brad Whitlock, Wed Oct 22 12:16:01 PDT 2003
//   I added an addDefaultPlots argument to OpenDataFile.
//
//   Brad Whitlock, Mon Nov 3 10:44:14 PDT 2003
//   Renamed OpenDataFile to SetOpenDataFile and removed its addDefaultPlots
//   argument.
//
//   Brad Whitlock, Fri Jan 30 11:40:54 PDT 2004
//   I added methods to get the state for the active source and to open
//   the active source in the file server.
//
//.  Brad Whitlock, Mon May 3 17:27:11 PST 2004
//   I added an argument to GetTimeSliderStateForDatabaseState.
//
// ****************************************************************************

class GUI_API GUIBase
{
public:
    GUIBase();
    virtual ~GUIBase();

    // Functions to set information global to all of the window classes.
    void ConnectMessageAttr(MessageAttributes *attr);
public:
    static ViewerProxy       *viewer;
protected:
    void Error(const QString &msg);
    void Warning(const QString &msg);
    void Message(const QString &msg);

    void Status(const QString &msg, int milliseconds = 10000);
    void ClearStatus();

    bool AutoUpdate() const;

    void SetWaitCursor();
    void RestoreCursor();

    bool SetOpenDataFile(const QualifiedFilename &qf,
                         int timeState,
                         SimpleObserver *sob = 0,
                         bool reOpen = false);

    void OpenActiveSourceInFileServer();
    int  GetStateForSource(const QualifiedFilename &source) const;
    int  GetTimeSliderStateForDatabaseState(const std::string &tsName,
                                            const QualifiedFilename &source,
                                            int dbState) const;
protected:
    static StatusSubject     *statusSubject;
    static bool              writeToConsole;
    static bool              autoUpdate;
    static bool              makeDefaultConfirm;
    static MessageAttributes *msgAttr;
    static FileServerList    *fileServer;
};

#endif
