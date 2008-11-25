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

#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <QMap>

// Forward declarations.
class EngineList;
class QComboBox;
class QGroupBox;
class QLabel;
class QProgressBar;
class QPushButton;
class StatusAttributes;

// ****************************************************************************
// Class: QvisEngineWindow
//
// Purpose:
//   This class implements a window that presents the list of engines to the
//   user so the engines' statuses can be watched and engines can be killed.
//
// Notes:      This window obseves multiple subjects.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:27:54 PST 2001
//
// Modifications:
//    Jeremy Meredith, Fri Jun 29 15:12:08 PDT 2001
//    Separated the single status/progress bar into one which reports
//    total status and one which reports current stage progress.
//
//    Brad Whitlock, Wed Nov 27 13:47:30 PST 2002
//    I added widgets to show more engine information.
//
//    Brad Whitlock, Wed Feb 25 09:22:29 PDT 2004
//    I added a "Clear cache" button.
//
//    Jeremy Meredith, Tue Mar 30 09:32:41 PST 2004
//    I added support for simulations.
//
//    Brad Whitlock, Wed Apr  9 11:00:33 PDT 2008
//    QString for caption, shortName.
//
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************

class GUI_API QvisEngineWindow : public QvisPostableWindowObserver
{
    Q_OBJECT

public:
    QvisEngineWindow(EngineList *engineList,
                     const QString &caption = QString::null,
                     const QString &shortName = QString::null,
                     QvisNotepadArea *notepad = 0);
    virtual ~QvisEngineWindow();
    virtual void CreateWindowContents();
    virtual void Update(Subject *TheChangedSubject);
    virtual void SubjectRemoved(Subject *TheRemovedSubject);

    void ConnectStatusAttributes(StatusAttributes *s);
private:
    void UpdateWindow(bool doAll);
    void UpdateStatusArea();
    void UpdateInformation(int index);
    void AddStatusEntry(const QString &key);
    void RemoveStatusEntry(const QString &key);
    void UpdateStatusEntry(const QString &key);
private slots:
    void closeEngine();
    void interruptEngine();
    void selectEngine(int index);
    void clearCache();
private:
    QMap<QString, StatusAttributes*>   statusMap;
    EngineList       *engines;
    StatusAttributes *statusAtts;
    Subject          *caller;
    QString           activeEngine;
    
    QLabel           *engineLabel;
    QComboBox        *engineCombo;
    QGroupBox        *engineInfo;
    QLabel           *engineNP;
    QLabel           *engineNN;
    QLabel           *engineLB;
    QLabel           *totalStatusLabel;
    QProgressBar     *totalProgressBar;
    QLabel           *stageStatusLabel;
    QProgressBar     *stageProgressBar;
    QPushButton      *interruptEngineButton;
    QPushButton      *closeEngineButton;
    QPushButton      *clearCacheButton;
};

#endif
