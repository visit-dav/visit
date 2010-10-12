/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

#include <QvisSelectionsWindow.h>

#include <EngineList.h>
#include <Plot.h>
#include <PlotList.h>
#include <SelectionProperties.h>
#include <SelectionList.h>
#include <ViewerProxy.h>
#include <QualifiedFilename.h>
#include <QvisFileOpenDialog.h>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSplitter>

// ****************************************************************************
// Method: QvisSelectionsWindow::QvisSelectionsWindow
//
// Purpose: 
//   This is the constructor for the QvisSelectionsWindow class.
//
// Arguments:
//   selectionList_  The SelectionList subject to observe
//   
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//
// ****************************************************************************

QvisSelectionsWindow::QvisSelectionsWindow(const QString &caption,
    const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowSimpleObserver(caption, shortName, notepad,
        QvisPostableWindowSimpleObserver::NoExtraButtons, false)
{
    selectionList = 0;
    plotList = 0;
    engineList = 0;
}

// ****************************************************************************
// Method: QvisSelectionsWindow::~QvisSelectionsWindow
//
// Purpose: 
//   Destructor for the QvisSelectionsWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//   
// ****************************************************************************
QvisSelectionsWindow::~QvisSelectionsWindow()
{
    if(selectionList != 0)
        selectionList->Detach(this);
    if(plotList != 0)
        plotList->Detach(this);
    if(engineList != 0)
        engineList->Detach(this);
}

void
QvisSelectionsWindow::ConnectSelectionList(SelectionList *s)
{
    selectionList = s;
    selectionList->Attach(this);
}

void
QvisSelectionsWindow::ConnectPlotList(PlotList *s)
{
    plotList = s;
    plotList->Attach(this);
}

void
QvisSelectionsWindow::ConnectEngineList(EngineList *el)
{
    engineList = el;
    engineList->Attach(this);
}

void
QvisSelectionsWindow::SubjectRemoved(Subject *s)
{
    if(selectionList == s)
        selectionList = 0;

    if(plotList == s)
        plotList = 0;

    if(engineList == s)
        engineList = 0;
}

// ****************************************************************************
// Method: QvisSelectionsWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisSelectionsWindow::CreateWindowContents()
{
    automaticallyApply = new QCheckBox(tr("Automatically apply updated selections"), central);
    connect(automaticallyApply, SIGNAL(toggled(bool)),
            this, SLOT(automaticallyApplyChanged(bool)));
    topLayout->addWidget(automaticallyApply);

    QSplitter *mainSplitter = new QSplitter(central);
    topLayout->addWidget(mainSplitter);
    topLayout->setStretchFactor(mainSplitter, 100);

    //
    // Selections group
    //
    QGroupBox *f1 = new QGroupBox(tr("Selections"));
    mainSplitter->addWidget(f1);
    QGridLayout *listLayout = new QGridLayout(f1);
    listLayout->setMargin(5);

    selectionListBox = new QListWidget(f1);
    connect(selectionListBox, SIGNAL(itemSelectionChanged()),
            this, SLOT(UpdateWindowSingleItem()));
    listLayout->addWidget(selectionListBox, 1,0, 1,2);

    newButton = new QPushButton(tr("New"), f1);
    connect(newButton, SIGNAL(pressed()),
            this, SLOT(addSelection()));
    listLayout->addWidget(newButton, 2,0);

    deleteButton = new QPushButton(tr("Delete"), f1);
    connect(deleteButton, SIGNAL(pressed()),
            this, SLOT(deleteSelection()));
    listLayout->addWidget(deleteButton, 2,1);

    updateButton = new QPushButton(tr("Update"), f1);
    connect(updateButton, SIGNAL(pressed()),
            this, SLOT(updateSelection()));
    listLayout->addWidget(updateButton, 3,0);
#define SELECTIONS_ARENT_DONE
    saveButton = new QPushButton(tr("Save"), f1);
    connect(saveButton, SIGNAL(pressed()),
            this, SLOT(saveSelection()));
    listLayout->addWidget(saveButton, 4,0);

    loadButton = new QPushButton(tr("Load"), f1);
    connect(loadButton, SIGNAL(pressed()),
            this, SLOT(loadSelection()));
    listLayout->addWidget(loadButton, 4,1);

    //
    // Properties group
    //
    QGroupBox *f2 = new QGroupBox(tr("Properties"));
    mainSplitter->addWidget(f2);
    QGridLayout *definitionLayout = new QGridLayout(f2);
    definitionLayout->setMargin(5);

    nameEditLabel = new QLabel(tr("Name"), f2);
    nameEdit = new QLineEdit(f2);
#ifndef SELECTIONS_ARENT_DONE
    connect(nameEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(nameTextChanged(const QString&)));
#else
    // for now.
    nameEdit->setEnabled(false);
#endif
    definitionLayout->addWidget(nameEditLabel, 0,0, 1,1);
    definitionLayout->addWidget(nameEdit, 0,1, 1,3);

    QLabel *plotNameEditLabel = new QLabel(tr("Plot"), f2);
    plotNameEditLabel->setToolTip(tr("Plot the defines the selection"));
    plotNameLabel = new QLabel(f2);
    definitionLayout->addWidget(plotNameEditLabel, 1,0, 1,1);
    definitionLayout->addWidget(plotNameLabel, 1,1, 1,3);

#ifndef SELECTIONS_ARENT_DONE
    editorTabs = new QTabWidget(f2);
    definitionLayout->addWidget(editorTabs, 2,0,1,4);
    definitionLayout->setRowStretch(2, 10);

    editorTabs->addTab(CreateRangeTab(f2), tr("Range"));
    editorTabs->addTab(CreateHistogramTab(f2), tr("Histogram"));
    editorTabs->addTab(CreateStatisticsTab(f2), tr("Statistics"));
#else
    definitionLayout->setRowStretch(2, 10);
#endif
}

QWidget *
QvisSelectionsWindow::CreateRangeTab(QWidget *parent)
{
    return new QWidget(parent);
}

QWidget *
QvisSelectionsWindow::CreateHistogramTab(QWidget *parent)
{
    return new QWidget(parent);
}

QWidget *
QvisSelectionsWindow::CreateStatisticsTab(QWidget *parent)
{
    return new QWidget(parent);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the window's subject is changed. The
//   subject tells this window what attributes changed and we put the
//   new values into those widgets.
//
// Arguments:
//   doAll : If this flag is true, update all the widgets regardless
//           of whether or not they are selected.  Currently ignored.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//   Brad Whitlock, Mon Oct 11 16:31:15 PDT 2010
//   I made it observe the engine list so we can set the load selection button's
//   enabled state to true if we have 1 engine but no plots.
//
// ****************************************************************************

void
QvisSelectionsWindow::UpdateWindow(bool doAll)
{
    if(selectionList == 0 || plotList == 0)
        return;

    if(SelectedSubject() == selectionList ||
       SelectedSubject() == engineList ||
       doAll)
    {
        automaticallyApply->blockSignals(true);
        automaticallyApply->setChecked(selectionList->GetAutoApplyUpdates());
        automaticallyApply->blockSignals(false);

        int cur = selectionListBox->currentRow();
        selectionListBox->blockSignals(true);
        selectionListBox->clear();
        for (int i = 0; i < selectionList->GetNumSelections(); ++i)
            selectionListBox->addItem(selectionList->GetSelections(i).GetName().c_str());
        if(selectionListBox->count() > 0)
        {
            cur = (cur < selectionListBox->count()) ? cur : 0;
            cur = (cur < 0) ? 0 : cur;
            selectionListBox->setCurrentRow(cur);
        }
        selectionListBox->blockSignals(false);

        UpdateWindowSingleItem();
    }

    if(SelectedSubject() == plotList || doAll)
    {
        // Only enable the new Button if the plot list's first selected plot has
        // been drawn.
        int index = plotList->FirstSelectedIndex();
        bool enabled = false;
        if(index != -1)
        {
            enabled = (plotList->GetPlots(index).GetStateType() == 
                       Plot::Completed);
        }
        newButton->setEnabled(enabled);
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::Apply
//
// Purpose: 
//   This is a Qt slot function that is called when the Apply button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::Apply(bool forceUpdate)
{

}

//
// Qt slots
//

// ****************************************************************************
// Method: QvisSelectionsWindow::automaticallyApplyChanged
//
// Purpose: 
//   Sets the named selection auto apply mode.
//
// Arguments:
//   val : Whether the selections update automatically when plots change.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 11 16:22:37 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::automaticallyApplyChanged(bool val)
{
    GetViewerMethods()->SetNamedSelectionAutoApply(val);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::GetLoadHost
//
// Purpose: 
//   Get the host to use for the load button.
//
// Arguments:
//
// Returns:    The most likely host name to use for loading a selection.
//
// Note:       We use the first selected plot's host name as the selection
//             load host. Otherwise, we use the only engine's host name.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 11 15:15:51 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

QString
QvisSelectionsWindow::GetLoadHost() const
{
    QString loadHost; 

    int index = plotList->FirstSelectedIndex();
    if(index != -1)
    {
        QualifiedFilename db(plotList->GetPlots(index).GetDatabaseName());
        loadHost = QString(db.host.c_str());
    }
    if(loadHost.isEmpty())
    {
        stringVector engines(engineList->GetEngines());
        if(engines.size() == 1) 
            loadHost = QString(engines[0].c_str());
    }

    // If loadHost == the expanded localhost name then we want to
    // just return localhost. This prevents us from opening an extra mdserver
    // when we want to open a selection on localhost.
    if(GetViewerProxy()->GetLocalHostName() == loadHost.toStdString())
        loadHost = "localhost";

    return loadHost;
}

// ****************************************************************************
//  Method:  QvisSelectionsWindow::UpdateWindowSingleItem
//
//  Purpose:
//    Update the pane of the window where a single selection is being edited.
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Aug  6 15:44:09 PDT 2010
//
//  Modifications:
//    Brad Whitlock, Mon Oct 11 15:15:37 PDT 2010
//    Set the enabled state for the load button.
//
// ****************************************************************************

void
QvisSelectionsWindow::UpdateWindowSingleItem()
{
    int index = selectionListBox->currentRow();
    nameEdit->blockSignals(true);
    if (index <  0)
    {
        nameEdit->setText("");
        plotNameLabel->setText(tr("none"));
    }
    else
    {
        const SelectionProperties &sel = (*selectionList)[index];
        nameEdit->setText(sel.GetName().c_str());
        if(sel.GetOriginatingPlot().empty())
            plotNameLabel->setText(tr("none"));
        else
            plotNameLabel->setText(sel.GetOriginatingPlot().c_str());
    }
    nameEdit->blockSignals(false);

    deleteButton->setEnabled(index >= 0);
    saveButton->setEnabled(index >= 0);

    // Set the enabled state of the load button.
    loadButton->setEnabled(!GetLoadHost().isEmpty());

#ifndef SELECTIONS_ARENT_DONE
    nameEdit->setEnabled(index >= 0);
#endif
    updateButton->setEnabled(index >= 0);
    nameEditLabel->setEnabled(index >= 0);
}

// ****************************************************************************
// Method: QvisSelectionsWindow::addSelection
//
// Purpose: 
//   This is a Qt slot function that adds a new Selection that is empty.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisSelectionsWindow::addSelection()
{
    // Find an unused Selection name
    int newid = 1;
    bool okay = false;
    QString newName;
    while (!okay)
    {
        newName.sprintf("selection%d", newid);
        if(selectionList->GetSelection(newName.toStdString()) >= 0)
            newid++;
        else
            okay = true;
    }

    newName = QInputDialog::getText(this, "VisIt", 
        tr("Enter a name for the new selection."),
        QLineEdit::Normal, newName, &okay);
    if(okay && !newName.isEmpty())
    {
        // Create a temporary entry in the selectionListBox so the new selection
        // will get selected when its data comes from the viewer.
        selectionListBox->blockSignals(true);
        selectionListBox->addItem(newName);
        selectionListBox->setCurrentRow(selectionListBox->count()-1);
        selectionListBox->blockSignals(false);

        // Tell the viewer to add the new named selection
        GetViewerMethods()->CreateNamedSelection(newName.toStdString());
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::delSelection
//
// Purpose: 
//   This is a Qt slot function that is called to delete a Selection.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisSelectionsWindow::deleteSelection()
{
    if(selectionListBox->currentItem() != 0)
    {
        GetViewerMethods()->DeleteNamedSelection(selectionListBox->currentItem()->
            text().toStdString());
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::updateSelection
//
// Purpose: 
//   This is a Qt slot function that is called to update a Selection.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisSelectionsWindow::updateSelection()
{
    if(selectionListBox->currentItem() != 0)
    {
        GetViewerMethods()->UpdateNamedSelection(selectionListBox->currentItem()->
            text().toStdString());
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::loadSelection
//
// Purpose: 
//   This is a Qt slot function that is called to load a selection.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisSelectionsWindow::loadSelection()
{
    QString loadHost(GetLoadHost());
    if(loadHost.isEmpty())
    {
        Warning(tr("VisIt can't determine the host for the compute engine to "
                   "use for loading the selection").arg(loadHost));
    }
    else
    {
        // Get the list of files at ~/.visit on the remote side that end with .ns
        QString loadFile(loadHost + ":~/.visit/");
        QString selName = QvisFileOpenDialog::getOpenFileName(loadFile,
            "*.ns", tr("Load selection from file"));
        if(!selName.isEmpty())
        {
            QualifiedFilename f(selName.toStdString());
            GetViewerMethods()->LoadNamedSelection(f.filename.substr(0, f.filename.size()-3),
                                                   loadHost.toStdString(), "");
        }
    }
}

// ****************************************************************************
// Method: QvisSelectionsWindow::saveSelection
//
// Purpose: 
//   This is a Qt slot function that is called to save a selection.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  6 15:44:09 PDT 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisSelectionsWindow::saveSelection()
{
    if(selectionListBox->currentItem() != 0)
    {
        GetViewerMethods()->SaveNamedSelection(selectionListBox->currentItem()->
            text().toStdString());
    }
}

// ****************************************************************************
//  Method:  QvisSelectionsWindow::nameTextChanged
//
//  Purpose:
//    Slot function when any change happens to the selection names.
//
//  Arguments:
//    text       the new text.
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Aug  6 15:44:09 PDT 2010
//
//  Modifications:
//
// ****************************************************************************

void
QvisSelectionsWindow::nameTextChanged(const QString &text)
{
    qDebug("rename the named selection as %s", text.toStdString().c_str());
}

// ****************************************************************************
// Method: QvisSelectionsWindow::highlightSelection
//
// Purpose: 
//   Slot function that highlights a specific selection in the list.
//
// Arguments:
//   selName : The name of the selection to highlight.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 11 10:52:48 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsWindow::highlightSelection(const QString &selName)
{
    QList<QListWidgetItem *> items = selectionListBox->findItems(selName, Qt::MatchFixedString);
    if(items.count() > 0)
        selectionListBox->setCurrentItem(items.first());
}

