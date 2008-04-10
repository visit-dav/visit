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

#ifndef QVIS_DATABASECORRELATIONLIST_WINDOW_H
#define QVIS_DATABASECORRELATIONLIST_WINDOW_H
#include <QvisPostableWindowObserver.h>
#include <map>

class DatabaseCorrelationList;
class QCheckBox;
class QComboBox;
class QListBox;
class QPushButton;

// ****************************************************************************
// Class: QvisDatabaseCorrelationListWindow
//
// Purpose:
//   This window allows users to create new database correlations and edit
//   existing correlations.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 15:14:01 PST 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 11:55:59 PDT 2008
//   QString for caption, shortName.
//  
// ****************************************************************************

class GUI_API QvisDatabaseCorrelationListWindow :
    public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisDatabaseCorrelationListWindow(DatabaseCorrelationList *cL,
                                      const QString &caption = QString::null,
                                      const QString &shortName = QString::null,
                                      QvisNotepadArea *notepad = 0);

    virtual ~QvisDatabaseCorrelationListWindow();

public slots:
    virtual void showMinimized();
    virtual void showNormal();
protected slots:
    void apply();
    void newCorrelation();
    void editCorrelation();
    void deleteCorrelation();
    void highlightCorrelation(int);
    void promptUserChecked(bool);
    void whenToCorrelateChanged(int);
    void defaultCorrelationMethodChanged(int);

    void deleteWindow(QvisWindowBase *);
    void delayedDeleteWindows();
protected:
    virtual void UpdateWindow(bool doAll);
    virtual void CreateWindowContents();
    void UpdateButtons();
    void Apply(bool = false);

    int                                highlightedCorrelation;
    int                                newCorrelationCounter;
    std::map<std::string, std::string> nameMap;

    WindowBaseVector                   activeCorrelationWindows;
    WindowBaseVector                   windowsToDelete;

    DatabaseCorrelationList *correlationList;

    // Widgets and layouts.
    QPushButton             *newButton;
    QPushButton             *editButton;
    QPushButton             *deleteButton;
    QListBox                *correlationListBox;
    QCheckBox               *promptUser;
    QComboBox               *whenToCorrelate;
    QComboBox               *defaultCorrelationMethod;
};

#endif
