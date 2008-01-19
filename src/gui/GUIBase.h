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

#ifndef GUI_BASE_H
#define GUI_BASE_H
#include <gui_exports.h>
#include <string>

// Forward declarations
class QString;
class FileServerList;
class MessageAttributes;
class ViewerMethods;
class ViewerProxy;
class ViewerState;
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
//   Brad Whitlock, Tue Feb 13 09:21:03 PDT 2007
//   I added GetViewerMethods and GetViewerState.
//
//   Brad Whitlock, Fri Jan 18 14:38:53 PST 2008
//   Added Information method.
//
// ****************************************************************************

class GUI_API GUIBase
{
public:
    GUIBase();
    virtual ~GUIBase();

    // Functions to set information global to all of the window classes.
    void ConnectMessageAttr(MessageAttributes *attr);
protected:
    void           SetViewerProxy(ViewerProxy *);
    ViewerProxy   *GetViewerProxy() const;
    ViewerState   *GetViewerState() const;
    ViewerMethods *GetViewerMethods() const;

    void Error(const QString &msg);
    void Warning(const QString &msg);
    void Message(const QString &msg);
    // Shows right away, cleared by other message types
    void Information(const QString &msg);

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
private:
    static ViewerProxy       *viewer;
protected:
    static StatusSubject     *statusSubject;
    static bool              writeToConsole;
    static bool              autoUpdate;
    static bool              makeDefaultConfirm;
    static MessageAttributes *msgAttr;
    static FileServerList    *fileServer;
};

#endif
