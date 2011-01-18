/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#ifndef QVIS_SELECTIONS_WINDOW_H
#define QVIS_SELECTIONS_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>
#include <AttributeSubject.h>

// Forward declarations
class QPushButton;
class QCheckBox;
class QGroupBox;
class QLabel;
class QListWidget;
class QTabWidget;

class PlotList;
class SelectionList;
class EngineList;

// ****************************************************************************
// Class: QvisSelectionsWindow
//
// Purpose:
//   This class contains the widgets that manipulate selections
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:35:12 PDT 2010
//
// Modifications:
//
// ****************************************************************************

class GUI_API QvisSelectionsWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisSelectionsWindow(const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea * notepad = 0);
    virtual ~QvisSelectionsWindow();
    virtual void CreateWindowContents();

    void ConnectSelectionList(SelectionList *);
    void ConnectPlotList(PlotList *);
    void ConnectEngineList(EngineList *);

    virtual void SubjectRemoved(Subject *);
public  slots:
    void highlightSelection(const QString &);
protected:
    void    UpdateWindow(bool doAll);
    void    Apply(bool forceUpdate = false);

    QString GetLoadHost() const;
private slots:
    void    automaticallyApplyChanged(bool val);
    void    UpdateWindowSingleItem();
    void    addSelection();
    void    deleteSelection();
    void    saveSelection();
    void    loadSelection();
    void    updateSelection();

    void    nameTextChanged(const QString &);

private:
    QWidget *CreateRangeTab(QWidget *parent);
    QWidget *CreateHistogramTab(QWidget *parent);
    QWidget *CreateStatisticsTab(QWidget *parent);

    // Widgets and layouts.
    QCheckBox          *automaticallyApply;
    QListWidget        *selectionListBox;
    QPushButton        *newButton;
    QPushButton        *deleteButton;
    QPushButton        *loadButton;
    QPushButton        *saveButton;
    QPushButton        *updateButton;

    QLineEdit          *nameEdit;
    QLabel             *nameEditLabel;
    QLabel             *plotNameLabel;
    QTabWidget         *editorTabs;

    // State information
    SelectionList      *selectionList;
    PlotList           *plotList;
    EngineList         *engineList;
};

#endif
