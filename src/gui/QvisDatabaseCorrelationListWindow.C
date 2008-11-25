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

#include <QvisDatabaseCorrelationListWindow.h>
#include <QvisDatabaseCorrelationWindow.h>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QPushButton>
#include <QTimer>

#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <GlobalAttributes.h>
#include <NameSimplifier.h>
#include <ViewerProxy.h>

#include <algorithm>

// ****************************************************************************
// Method: QvisDatabaseCorrelationListWindow::QvisDatabaseCorrelationListWindow
//
// Purpose: 
//   Constructor for the QvisDatabaseCorrelationListWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:15:10 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 11:56:23 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisDatabaseCorrelationListWindow::QvisDatabaseCorrelationListWindow(
    DatabaseCorrelationList *cL, const QString &caption,
    const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(cL, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton, false),
    nameMap(), activeCorrelationWindows(), windowsToDelete()
{
    correlationList = cL;
    highlightedCorrelation = -1;
    newCorrelationCounter = 1;
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationListWindow::~QvisDatabaseCorrelationListWindow
//
// Purpose: 
//   Destructor for the QvisDatabaseCorrelationListWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:15:28 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisDatabaseCorrelationListWindow::~QvisDatabaseCorrelationListWindow()
{
    correlationList = 0;

    int i;
    for(i = 0; i < windowsToDelete.size(); ++i)
        delete windowsToDelete[i];
    windowsToDelete.clear();

    for(i = 0; i < activeCorrelationWindows.size(); ++i)
        delete activeCorrelationWindows[i];
    activeCorrelationWindows.clear();
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationListWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:15:51 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//   
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisDatabaseCorrelationListWindow::CreateWindowContents()
{
    QGroupBox *correlationGroup = new QGroupBox(central);
    correlationGroup->setTitle(tr("Database correlations"));
    topLayout->addWidget(correlationGroup, 5);

    // Create the correlation list management widgets.
    QVBoxLayout *innerCorrelationTopLayout = new QVBoxLayout(correlationGroup);
    innerCorrelationTopLayout->setMargin(10);
    innerCorrelationTopLayout->addSpacing(15);
    QHBoxLayout *hLayout = new QHBoxLayout();
    innerCorrelationTopLayout->addLayout(hLayout);
    hLayout->setSpacing(5);

    // Create buttons to manage the list of correlations.
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    hLayout->addLayout(buttonLayout);
    newButton = new QPushButton(tr("New ..."), correlationGroup);
    connect(newButton, SIGNAL(clicked()), this, SLOT(newCorrelation()));
    buttonLayout->addWidget(newButton);

    editButton = new QPushButton(tr("Edit ..."), correlationGroup);
    connect(editButton, SIGNAL(clicked()), this, SLOT(editCorrelation()));
    buttonLayout->addWidget(editButton);

    deleteButton = new QPushButton(tr("Delete"), correlationGroup);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteCorrelation()));
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch(5);

    //
    // Create the list of correlations.
    //
    QVBoxLayout *vLayout = new QVBoxLayout();
    hLayout->addLayout(vLayout);
    correlationListBox = new QListWidget(correlationGroup);
    connect(correlationListBox, SIGNAL(currentRowChanged(int)),
            this, SLOT(highlightCorrelation(int)));
    vLayout->addWidget(correlationListBox, 10);

    //
    // Create the widgets for setting the correlation defaults.
    //
    QGroupBox *automaticCorrelationGroup = new QGroupBox(central);
    automaticCorrelationGroup->setTitle(tr("Automatic database correlation"));
    topLayout->addWidget(automaticCorrelationGroup, 5);

    QVBoxLayout *innerACTopLayout = new QVBoxLayout(automaticCorrelationGroup);
    //innerACTopLayout->setMargin(10);
    //innerACTopLayout->addSpacing(15);
    QGridLayout *adcLayout = new QGridLayout();
    innerACTopLayout->addLayout(adcLayout);
    adcLayout->setSpacing(10);

    // Create the prompt user checkbox.
    promptUser = new QCheckBox(tr("Prompt before creating new correlation"),
                               automaticCorrelationGroup);
    connect(promptUser, SIGNAL(toggled(bool)),
            this, SLOT(promptUserChecked(bool)));
    adcLayout->addWidget(promptUser, 0, 0, 1, 2);

    // Create the "When to correlate" combo box.
    whenToCorrelate = new QComboBox(automaticCorrelationGroup);
    whenToCorrelate->addItem(tr("Always"));
    whenToCorrelate->addItem(tr("Never"));
    whenToCorrelate->addItem(tr("Same number of states"));
    connect(whenToCorrelate, SIGNAL(activated(int)),
            this, SLOT(whenToCorrelateChanged(int)));
    adcLayout->addWidget(whenToCorrelate, 1,1);
    adcLayout->addWidget(new QLabel(tr("When to create correlation")), 1, 0);

    // Create the default correlation method combo box.
    defaultCorrelationMethod = new QComboBox(automaticCorrelationGroup);
    defaultCorrelationMethod->addItem(tr("Padded index"));
    defaultCorrelationMethod->addItem(tr("Stretched index"));
    defaultCorrelationMethod->addItem(tr("Time"));
    defaultCorrelationMethod->addItem(tr("Cycle"));
    connect(defaultCorrelationMethod, SIGNAL(activated(int)),
            this, SLOT(defaultCorrelationMethodChanged(int)));
    adcLayout->addWidget(defaultCorrelationMethod, 2,1);
    adcLayout->addWidget(new QLabel(tr("Default correlation method"), 
                                    automaticCorrelationGroup), 2, 0);
    topLayout->addStretch(2);
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationListWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the correlation list changes and the window
//   must update itself.
//
// Arguments:
//   doAll : Whether we should ignore the selection.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:16:22 PDT 2004
//
// Modifications:
//   Brad Whitlock, Fri Dec 14 17:02:52 PST 2007
//   Made it use ids.
//
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisDatabaseCorrelationListWindow::UpdateWindow(bool doAll)
{
    if(correlationList == 0)
        return;

    if(correlationList->IsSelected(DatabaseCorrelationList::ID_correlations) || doAll)
    {
        //
        // Update the list of correlation names.
        //
        highlightedCorrelation = -1;
        correlationListBox->blockSignals(true);
        correlationListBox->clear();

        //
        // Simplify the current source names before adding them to the list
        // of correlation names.
        //
        NameSimplifier simplifier;
        int i, nC = correlationList->GetNumCorrelations();
        for(i = 0; i < nC; ++i)
        {
            const DatabaseCorrelation &c = correlationList->GetCorrelations(i);
            simplifier.AddName(c.GetName());
        }
        stringVector shortNames;
        simplifier.GetSimplifiedNames(shortNames);
        nameMap.clear();
        for(i = 0; i < shortNames.size(); ++i)
        {
            const DatabaseCorrelation &c = correlationList->GetCorrelations(i);
            nameMap[shortNames[i]] = c.GetName();
            correlationListBox->addItem(shortNames[i].c_str());
        }

        correlationListBox->setEnabled(nC > 1);
        if(nC > 0)
            correlationListBox->setCurrentRow(0);
        highlightedCorrelation = (nC > 0) ? 0 : -1;
        correlationListBox->blockSignals(false);

        //
        // Update the management buttons.
        //
        UpdateButtons();
    }

    if(correlationList->IsSelected(DatabaseCorrelationList::ID_needPermission) || doAll)
    {
        promptUser->blockSignals(true);
        promptUser->setChecked(correlationList->GetNeedPermission());
        promptUser->blockSignals(false);
    }

    if(correlationList->IsSelected(DatabaseCorrelationList::ID_defaultCorrelationMethod) || doAll)
    {
        defaultCorrelationMethod->blockSignals(true);
        defaultCorrelationMethod->setCurrentIndex(
            correlationList->GetDefaultCorrelationMethod());
        defaultCorrelationMethod->blockSignals(false);
    }

    if(correlationList->IsSelected(DatabaseCorrelationList::ID_whenToCorrelate) || doAll)
    {
        whenToCorrelate->blockSignals(true);
        whenToCorrelate->setCurrentIndex((int)correlationList->GetWhenToCorrelate());
        whenToCorrelate->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationListWindow::UpdateButtons
//
// Purpose: 
//   Updates the buttons so we can't ever do something inappropriate with
//   correlations.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:17:18 PDT 2004
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisDatabaseCorrelationListWindow::UpdateButtons()
{
    bool editEnabled = false;
    bool deleteEnabled = false;

    if(highlightedCorrelation >= 0 &&
       highlightedCorrelation < correlationList->GetNumCorrelations())
    {
        // Get the name of the correlation from the list box and send
        // it through nameMap because we likely shortened the name.
        QString hl_name = correlationListBox->item(highlightedCorrelation)->text();
        std::string shortName = hl_name.toStdString();
        std::string realName(nameMap[shortName]);

        // Search for the correlation in the correlation list.
        DatabaseCorrelation *correlation = correlationList->
            FindCorrelation(realName);
        if(correlation)
        {
            //
            // Check to see if the correlation is a source. If it is not
            // then allow it to be deletd or edited.
            //
            GlobalAttributes *globalAtts = GetViewerState()->GetGlobalAttributes();
            const stringVector &src = globalAtts->GetSources();
            if(std::find(src.begin(), src.end(), realName) == src.end())
            {
                editEnabled = true;
                deleteEnabled = true;
            }
        }
    }
    else
        highlightedCorrelation = -1;

    editButton->setEnabled(editEnabled);
    deleteButton->setEnabled(deleteEnabled);
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationListWindow::Apply
//
// Purpose: 
//   Sends the database correlation list to the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:18:09 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisDatabaseCorrelationListWindow::Apply(bool)
{
    correlationList->Notify();
}

//
// Qt slot functions.
//

// ****************************************************************************
// Method: QvisDatabaseCorrelationListWindow::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the Apply button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:18:29 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisDatabaseCorrelationListWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationListWindow::showMinimized
//
// Purpose: 
//   This is a Qt slot function that is called when the window is iconified.
//
// Notes:      The window iconifies its correlation windows.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:18:29 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisDatabaseCorrelationListWindow::showMinimized()
{
    QvisPostableWindowObserver::showMinimized();

    // Show any correlation windows minimized too.
    for(int i = 0; i < activeCorrelationWindows.size(); ++i)
        activeCorrelationWindows[i]->showMinimized();
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationListWindow::showNormal
//
// Purpose: 
//   This is a Qt slot function that is called when the window is deiconified.
//
// Notes:      The window deiconifies its correlation windows.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:18:29 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisDatabaseCorrelationListWindow::showNormal()
{
    QvisPostableWindowObserver::showNormal();

    // Show any correlation windows minimized too.
    for(int i = 0; i < activeCorrelationWindows.size(); ++i)
        activeCorrelationWindows[i]->showNormal();
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationListWindow::newCorrelation
//
// Purpose: 
//   This is a Qt slot function that is called when we want to create a new
//   database correlation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:19:59 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//   
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisDatabaseCorrelationListWindow::newCorrelation()
{
    // Show a dialog to make the user type a name.

    //
    // Make a new correlation name until it is not in the correlation list.
    //
    QString newName;
    do
    {
        newName.sprintf("Correlation%02d", newCorrelationCounter++);
    } while(correlationList->FindCorrelation(newName.toStdString()) != 0);

    //
    // Create a new correlation window and set it up so that when you click
    // apply in it, it creates a new correlation.
    //
    QString caption(tr("Create database correlation: "));
    caption += newName;
    QvisDatabaseCorrelationWindow *win = new QvisDatabaseCorrelationWindow(
        newName, caption);
    connect(win, SIGNAL(deleteMe(QvisWindowBase *)),
            this, SLOT(deleteWindow(QvisWindowBase *)));
    activeCorrelationWindows.push_back(win);
    win->show();
    win->activateWindow();
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationListWindow::editCorrelation
//
// Purpose: 
//   This is a Qt slot function that is called when we want to edit an
//   existing database correlation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:20:36 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//   
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisDatabaseCorrelationListWindow::editCorrelation()
{
    // Show a dialog to make the user type a name.
    if(highlightedCorrelation >= 0)
    {
        QString hl_name = correlationListBox->item(highlightedCorrelation)->text();
        std::string shortName = hl_name.toStdString();
        std::string realName(nameMap[shortName]);

        //
        // Create a new correlation window in "edit" mode so that when you
        // click apply in it, it changes the existing correlation.
        //
        DatabaseCorrelation *c = correlationList->FindCorrelation(realName);
        if(c != 0)
        {
            QString caption(tr("Alter database correlation: %1").arg(realName.c_str()));
            QvisDatabaseCorrelationWindow *win = new QvisDatabaseCorrelationWindow(
                *c, caption);
            connect(win, SIGNAL(deleteMe(QvisWindowBase *)),
                    this, SLOT(deleteWindow(QvisWindowBase *)));
            activeCorrelationWindows.push_back(win);
            win->show();
            win->activateWindow();
        }
    }
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationListWindow::deleteCorrelation
//
// Purpose: 
//   This is a Qt slot function that is called when we want to delete a
//   database correlation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:21:12 PDT 2004
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisDatabaseCorrelationListWindow::deleteCorrelation()
{
    if(highlightedCorrelation >= 0)
    {
        QString hl_name = correlationListBox->item(highlightedCorrelation)->text();
        std::string shortName = hl_name.toStdString();
        std::string realName(nameMap[shortName]);
        GetViewerMethods()->DeleteDatabaseCorrelation(realName);
        editButton->setEnabled(false);
        deleteButton->setEnabled(false);
    }
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationListWindow::promptUserChecked
//
// Purpose: 
//   This is a Qt slot function that is called when the "Prompt user" button
//   is clicked.
//
// Arguments:
//   val : The new value for prompting the user.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:21:12 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisDatabaseCorrelationListWindow::promptUserChecked(bool val)
{
    correlationList->SetNeedPermission(val);
    SetUpdate(false);
    Apply();
}

void
QvisDatabaseCorrelationListWindow::whenToCorrelateChanged(int val)
{
    correlationList->SetWhenToCorrelate(
        (DatabaseCorrelationList::WhenToCorrelate)val);
    SetUpdate(false);
    Apply();
}

void
QvisDatabaseCorrelationListWindow::defaultCorrelationMethodChanged(int val)
{
    correlationList->SetDefaultCorrelationMethod(val);
    SetUpdate(false);
    Apply();
}

void
QvisDatabaseCorrelationListWindow::highlightCorrelation(int index)
{
    highlightedCorrelation = index;
    UpdateButtons();
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationListWindow::deleteWindow
//
// Purpose: 
//   This is a Qt slot function that allows the window to safely delete its
//   child windows.
//
// Arguments:
//   win  : The window to delete.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:22:30 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisDatabaseCorrelationListWindow::deleteWindow(QvisWindowBase *win)
{
    // Add the window to the deletion list.
    windowsToDelete.push_back(win);

    // Remove the window from the list of active windows.
    WindowBaseVector::iterator pos = std::find(activeCorrelationWindows.begin(),
        activeCorrelationWindows.begin(), win);
    if(pos != activeCorrelationWindows.end())
        activeCorrelationWindows.erase(pos);

    // Now that the window has been moved to the delete list, call a delayed
    // method to delete the windows in the delete list. We can't delete it here
    // because the window caused the signal that got us here and deleting it
    // now would hose Qt.
    QTimer::singleShot(100, this, SLOT(delayedDeleteWindows()));
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationListWindow::delayedDeleteWindows
//
// Purpose: 
//   This is a Qt slot function that lets you delete some database correlation
//   windows.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:23:28 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisDatabaseCorrelationListWindow::delayedDeleteWindows()
{
    for(int i = 0; i < windowsToDelete.size(); ++i)
        delete windowsToDelete[i];
    windowsToDelete.clear();
}
