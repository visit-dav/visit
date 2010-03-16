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

#ifndef QVIS_SOURCE_MANAGER_WIDGET_H
#define QVIS_SOURCE_MANAGER_WIDGET_H
#include <gui_exports.h>

#include <QAction>
#include <QMap>
#include <QStringList>
#include <QWidget>
#include <QGroupBox>

#include <GUIBase.h>
#include <SimpleObserver.h>


// Forward declarations.
class  PlotList;
class  FileServerList;
class  GlobalAttributes;
class  WindowInformation;

class  QComboBox;
class  QGridLayout;
class  QLabel;
class  QToolBar;
class  QToolButton;

// ****************************************************************************
// Class: QvisSourceManagerWidget
//
// Purpose:
//      Holds widgets that provide source selection and manipulation.
//
// Note:
//   These widgets were moved out of QvisPlotManager into this class.
//
// Programmer: Cyrus Harrison
// Creation:   Fri Mar 12 10:55:05 PST 2010
//
// Modifications:
//
// ****************************************************************************

class GUI_API QvisSourceManagerWidget : public QGroupBox, public GUIBase,
    public SimpleObserver
{
    Q_OBJECT
public:
    QvisSourceManagerWidget(QWidget *parent = 0);
    ~QvisSourceManagerWidget();

    virtual void Update(Subject *);
    virtual void SubjectRemoved(Subject *);

    void ConnectPlotList(PlotList *);
    void ConnectGlobalAttributes(GlobalAttributes *);
    void ConnectFileServer(FileServerList *);
    void ConnectWindowInformation(WindowInformation *);

signals:
    void activateFileOpenWindow();

private:
    void UpdateSourceList(bool updateActiveSourceOnly);
    void UpdateDatabaseIconEnabledStates();

private slots:
    void closeCurrentSource();
    void reOpenCurrentSource();
    void replaceWithCurrentSource();
    void overlayWithCurrentSource();
    void sourceChanged(int);

private:
    // State objects that this widget observes.
    PlotList                *plotList;
    GlobalAttributes        *globalAtts;
    WindowInformation       *windowInfo;

    QGridLayout  *topLayout;

    QToolBar     *dbActionsToolbar;
    QAction      *dbOpenAction;
    QAction      *dbReopenAction;
    QAction      *dbCloseAction;
    QAction      *dbReplaceAction;
    QAction      *dbOverlayAction;

    QLabel       *sourceLabel;
    QComboBox    *sourceComboBox;
};

#endif
