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

#include <QvisSourceManagerWidget.h>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCursor>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QMap>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmapCache>
#include <QPushButton>
#include <QTreeWidget>
#include <QGroupBox>
#include <QWidget>
#include <QToolBar>
#include <QToolButton>

#include <ViewerProxy.h>
#include <PlotList.h>
#include <FileServerList.h>
#include <ExpressionList.h>
#include <Expression.h>
#include <GlobalAttributes.h>
#include <NameSimplifier.h>
#include <PluginManagerAttributes.h>
#include <QvisPlotListBoxItem.h>
#include <QvisPlotListBox.h>
#include <QvisVariableButton.h>
#include <QvisVariablePopupMenu.h>
#include <PlotPluginInfo.h>
#include <WindowInformation.h>
#include <TimingsManager.h>

#include <stdio.h>
#include <stdlib.h>
#include <snprintf.h>

#include <icons/removelastoperator.xpm>
#include <icons/removealloperators.xpm>

#include <icons/plot_add.xpm>
#include <icons/plot_del.xpm>
#include <icons/plot_var.xpm>
#include <icons/plot_atts.xpm>
#include <icons/plot_hide.xpm>
#include <icons/plot_draw.xpm>
#include <icons/oper_add3.xpm>
#include <icons/db_replace.xpm>
#include <icons/db_overlay.xpm>
#include <icons/db_open2.xpm>
#include <icons/db_close2.xpm>
#include <icons/db_reopen.xpm>

#include <DebugStream.h>
//#define DEBUG_PRINT

#define REMOVE_LAST_OPERATOR_ID   1000
#define REMOVE_ALL_OPERATORS_ID   1001

#define VARIABLE_CUTOFF           100

#define DELETE_MENU_TO_FREE_POPUPS

using std::string;
using std::vector;

// ****************************************************************************
// Method: QvisSourceManagerWidget::QvisSourceManagerWidget
//
// Purpose:
//   This is the constructor for the QvisSourceManagerWidget class.
//
// Arguments:
//   parent   : The widget's parent.
//
// Note: Taken/Refactored from QvisPlotManager.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 11:05:13 PST 2010
//
// Modifications:
//
// ****************************************************************************

QvisSourceManagerWidget::QvisSourceManagerWidget(QWidget *parent)
: QGroupBox(tr("Sources"),parent), GUIBase(), SimpleObserver()
{
    plotList = 0;
    globalAtts = 0;
    windowInfo = 0;

    topLayout = new QGridLayout(this);
    topLayout->setColumnStretch(0,1);
    topLayout->setColumnStretch(1,10);
    topLayout->setMargin(0);
    dbActionsToolbar = new QToolBar(this);
    dbActionsToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    dbOpenAction    = dbActionsToolbar->addAction(QIcon(db_open2_xpm),tr("Open"),
                                                  this, SIGNAL(activateFileOpenWindow()));

    dbReopenAction  = dbActionsToolbar->addAction(QIcon(db_close2_xpm),tr("Close"),
                                                  this, SLOT(closeCurrentSource()));

    dbCloseAction   = dbActionsToolbar->addAction(QIcon(db_reopen_xpm),tr("Reopen"),
                                                  this, SLOT(reOpenCurrentSource()));
    dbActionsToolbar->addSeparator();
    dbReplaceAction = dbActionsToolbar->addAction(QIcon(db_replace_xpm),tr("Replace Plots"),
                                                  this, SLOT(replaceWithCurrentSource()));
    dbOverlayAction = dbActionsToolbar->addAction(QIcon(db_overlay_xpm),tr("Overlay Plots"),
                                                  this, SLOT(overlayWithCurrentSource()));

    sourceLabel = new QLabel(tr("Active source"), this);
    sourceLabel->setAlignment(Qt::AlignLeft|Qt::AlignCenter);
    sourceComboBox = new QComboBox(this);

    connect(sourceComboBox, SIGNAL(activated(int)),
            this, SLOT(sourceChanged(int)));


    topLayout->addWidget(dbActionsToolbar,0,0,1,2);
    topLayout->addWidget(sourceLabel,1,0);
    topLayout->addWidget(sourceComboBox,1,1);

}

// ****************************************************************************
// Method: QvisSourceManagerWidget::~QvisSourceManagerWidget
//
// Purpose:
//   This is the destructor for the QvisSourceManagerWidget class.
//
// Programmer: Cyrus Harrison
// Creation:   Fri Mar 12 11:18:23 PST 2010
//
//
// Modifications:
//
//
// ****************************************************************************

QvisSourceManagerWidget::~QvisSourceManagerWidget()
{
    if(plotList)
        plotList->Detach(this);

    if(fileServer)
        fileServer->Detach(this);

    if(globalAtts)
        globalAtts->Detach(this);

    if(windowInfo)
        windowInfo->Detach(this);

}

// ****************************************************************************
// Method: QvisSourceManagerWidget::Update
//
// Purpose:
//   This method is called when the Global Atts or FileServerList that
//   the widget watches is updated.
//
//
// Programmer: Cyrus Harrison
// Creation:   Fri Mar 12 11:18:50 PST 2010
//
// Note: Taken/Refactored from QvisPlotManager.
//
// Modifications:
//
// ****************************************************************************

void
QvisSourceManagerWidget::Update(Subject *TheChangedSubject)
{
    if(plotList == 0 || fileServer == 0 || globalAtts == 0 || windowInfo == 0)
        return;

    if(TheChangedSubject == fileServer && fileServer->FileChanged())
    {
        UpdateSourceList(false);
    }
    else if(TheChangedSubject == globalAtts &&
            globalAtts->IsSelected(GlobalAttributes::ID_sources))
    {

        UpdateSourceList(false);
    }
    else if(TheChangedSubject == windowInfo &&
            windowInfo->IsSelected(WindowInformation::ID_activeSource))
    {
        UpdateSourceList(true);
    }

    // Update the enabled state for the db-related buttons
    UpdateDatabaseIconEnabledStates();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::UpdateSourceList
//
// Purpose:
//   Updates the source list.
//
// Arguments:
//   updateActiveSourceOnly : Tells the method to only update the active
//                            source and not the list of sources.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 29 21:58:49 PST 2004
//
// Note: Taken/Refactored from QvisPlotManager.
//
// Modifications:
//
// ****************************************************************************

void
QvisSourceManagerWidget::UpdateSourceList(bool updateActiveSourceOnly)
{
    const stringVector &sources     = globalAtts->GetSources();
    const std::string &activeSource = windowInfo->GetActiveSource();

    // See if the active source is in the list.
    int i, sourceIndex = -1;
    for(i = 0; i < sources.size(); ++i)
    {
        if(activeSource == sources[i])
        {
            sourceIndex = i;
            break;
        }
    }

    sourceComboBox->blockSignals(true);

    //
    // Populate the menu if we were not told to only update the active source.
    //
    if(!updateActiveSourceOnly)
    {
        //
        // Simplify the current source names and put the short names into
        // the source combo box.
        //
        NameSimplifier simplifier;
        for(i = 0; i < sources.size(); ++i)
            simplifier.AddName(sources[i]);
        stringVector shortSources;
        simplifier.GetSimplifiedNames(shortSources);
        sourceComboBox->clear();
        for(i = 0; i < shortSources.size(); ++i)
            sourceComboBox->addItem(shortSources[i].c_str());
    }

    //
    // Set the current item.
    //
    if(sourceIndex != -1 && sourceIndex != sourceComboBox->currentIndex())
        sourceComboBox->setCurrentIndex(sourceIndex);

    sourceComboBox->blockSignals(false);

    // Set the enabled state on the source combo box.
    bool enabled = (sources.size() > 0);
    sourceLabel->setEnabled(enabled);
    sourceComboBox->setEnabled(enabled);
}



// ****************************************************************************
// Method:  QvisSourceManagerWidget::UpdateDatabaseIconEnabledStates
//
// Purpose:
//    Update the enabled state for the db-related buttons
//
// Arguments:
//   none
//
//
// Note: Taken/Refactored from QvisPlotManager.
//
// Programmer:  Jeremy Meredith
// Creation:    February 19, 2010
//
// ****************************************************************************
void
QvisSourceManagerWidget::UpdateDatabaseIconEnabledStates()
{
    const stringVector &sources = globalAtts->GetSources();
    int index = sourceComboBox->currentIndex();
    if (index < 0 || index >= sources.size())
    {
        // If no files are open, we can't do anything with the open file
        // (note that dbOpen remains enabled, though).
        dbReplaceAction->setEnabled(false);
        dbReopenAction->setEnabled(false);
        dbCloseAction->setEnabled(false);
        dbOverlayAction->setEnabled(false);
        return;
    }

    // Okay, we have a file open.  We can always re-open or attempt to close it
    dbReopenAction->setEnabled(true);
    dbCloseAction->setEnabled(true);

    // If all the current plots belong to the current source, there's
    // no point in allowing a "replace" or "overlay" operation.
    bool different = false;
    string filename = QualifiedFilename(sources[index]).FullName();
    for (int i = 0; i < plotList->GetNumPlots(); ++i)
    {
        if (plotList->operator[](i).GetActiveFlag() == true &&
            plotList->operator[](i).GetDatabaseName() != filename)
            different = true;
    }
    dbReplaceAction->setEnabled(different);
    dbOverlayAction->setEnabled(different);
}

// ****************************************************************************
// Method: QvisSourceManagerWidget::SubjectRemoved
//
// Purpose:
//   Removes the subjects that this widget observes.
//
// Arguments:
//   TheRemovedSubject : The subject that is being removed.
//
//
// Note: Taken/Refactored from QvisPlotManager.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 16:28:44 PST 2000
//
// Modifications:
//
// ****************************************************************************

void
QvisSourceManagerWidget::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(TheRemovedSubject == plotList)
        plotList = 0;
    else if(TheRemovedSubject == globalAtts)
        globalAtts = 0;
    else if(TheRemovedSubject == windowInfo)
        windowInfo = 0;
}

// ****************************************************************************
//
// Methods to attach to the plot list, fileserver, global atts and windowinfo
// objects.
//
// ****************************************************************************

void
QvisSourceManagerWidget::ConnectPlotList(PlotList *pl)
{
    plotList = pl;
    plotList->Attach(this);
}

// ****************************************************************************
void
QvisSourceManagerWidget::ConnectFileServer(FileServerList *)
{
    fileServer->Attach(this);
}

// ****************************************************************************
void
QvisSourceManagerWidget::ConnectGlobalAttributes(GlobalAttributes *ga)
{
    globalAtts = ga;
    globalAtts->Attach(this);
}

// ****************************************************************************
void
QvisSourceManagerWidget::ConnectWindowInformation(WindowInformation *wi)
{
    windowInfo = wi;
    windowInfo->Attach(this);
}


// ****************************************************************************
// Method: QvisPlotManagerWidget::sourceChanged
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to activate a source
//   that is already open.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 29 22:02:49 PST 2004
//
//
// Note: Taken/Refactored from QvisPlotManager.
//
// Modifications:
//
// ****************************************************************************

void
QvisSourceManagerWidget::sourceChanged(int index)
{
    const stringVector &sources = globalAtts->GetSources();
    if(index >= 0 && index < sources.size())
        GetViewerMethods()->ActivateDatabase(sources[index]);
}


// ****************************************************************************
// Method:  QvisSourceManagerWidget::reOpenCurrentSource
//
// Purpose:
//   reopens current source
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 19, 2010
//
//
// Note: Taken/Refactored from QvisPlotManager.
//
//
// ****************************************************************************
void
QvisSourceManagerWidget::reOpenCurrentSource()
{
    int index = sourceComboBox->currentIndex();
    const stringVector &sources = globalAtts->GetSources();
    if(index >= 0 && index < sources.size())
    {
        //
        // Make the file that we reopened be the new open file. Since we're
        // reopening, this will take care of freeing the old metadata and SIL.
        //
        QualifiedFilename fileName(sources[index]);
        int timeState = GetStateForSource(fileName);
        SetOpenDataFile(fileName, timeState, 0, true);

        // Tell the viewer to replace all of the plots having
        // databases that match the file we're re-opening.
        GetViewerMethods()->ReOpenDatabase(sources[index], false);
    }
}


// ****************************************************************************
// Method:  QvisSourceManagerWidget::closeCurrentSource
//
// Purpose:
//   close the current source, if possible
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 19, 2010
//
//
// Note: Taken/Refactored from QvisPlotManager.
//
// ****************************************************************************
void
QvisSourceManagerWidget::closeCurrentSource()
{
    int index = sourceComboBox->currentIndex();
    const stringVector &sources = globalAtts->GetSources();
    if(index >= 0 && index < sources.size())
    {
        //
        // Clear out the metadata and SIL for the file.
        //
        fileServer->ClearFile(sources[index]);

        //
        // Tell the viewer to replace close the specified database. If the
        // file is not being used then the viewer will allow it. Otherwise
        // the viewer will issue a warning message.
        //
        GetViewerMethods()->CloseDatabase(sources[index]);
    }
}

// ****************************************************************************
// Method:  QvisSourceManagerWidget::replaceWithCurrentSource
//
// Purpose:
//   replace the active plots using the current source
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 19, 2010
//
//
// Note: Taken/Refactored from QvisPlotManager.
//
// ****************************************************************************
void
QvisSourceManagerWidget::replaceWithCurrentSource()
{
    int index = sourceComboBox->currentIndex();
    const stringVector &sources = globalAtts->GetSources();
    if(index >= 0 && index < sources.size())
    {
        //
        // Make the file that we reopened be the new open file. Since we're
        // reopening, this will take care of freeing the old metadata and SIL.
        //
        QualifiedFilename fileName(sources[index]);
        int timeState = GetStateForSource(fileName);
        SetOpenDataFile(fileName, timeState, 0, false);

        // Tell the viewer to replace the database.
        GetViewerMethods()->ReplaceDatabase(fileName.FullName().c_str(),
                                            timeState,
                                            true); // true==active plots only
    }
}

// ****************************************************************************
// Method:  QvisSourceManagerWidget::overlayWithCurrentSource
//
// Purpose:
//   duplicate the active plots with the current source
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 19, 2010
//
//
// Note: Taken/Refactored from QvisPlotManager.
//
// ****************************************************************************
void
QvisSourceManagerWidget::overlayWithCurrentSource()
{
    int index = sourceComboBox->currentIndex();
    const stringVector &sources = globalAtts->GetSources();
    if(index >= 0 && index < sources.size())
    {
        //
        // Make the file that we reopened be the new open file. Since we're
        // reopening, this will take care of freeing the old metadata and SIL.
        //
        QualifiedFilename fileName(sources[index]);
        int timeState = GetStateForSource(fileName);
        SetOpenDataFile(fileName, timeState, 0, false);

        // Tell the viewer to overlay the database.
        GetViewerMethods()->OverlayDatabase(fileName.FullName().c_str(),
                                            timeState);
    }
}
