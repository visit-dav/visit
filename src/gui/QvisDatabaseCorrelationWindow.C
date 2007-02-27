/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <QvisDatabaseCorrelationWindow.h>

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>

#include <DatabaseCorrelationList.h>
#include <DatabaseCorrelation.h>
#include <GlobalAttributes.h>
#include <NameSimplifier.h>
#include <ViewerProxy.h>

#include <map>

//
// Static members
//
int QvisDatabaseCorrelationWindow::instanceCount = 0;

// ****************************************************************************
// Method: QvisDatabaseCorrelationWindow::QvisDatabaseCorrelationWindow
//
// Purpose: 
//   Constructor for the QvisDatabaseCorrelationWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:25:11 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisDatabaseCorrelationWindow::QvisDatabaseCorrelationWindow(
    const QString &correlationName, const char *caption) :
    QvisWindowBase(caption), createMode(true)
{
    DatabaseCorrelation emptyCorrelation;
    emptyCorrelation.SetName(correlationName.latin1());
    CreateWidgets(emptyCorrelation);
}

QvisDatabaseCorrelationWindow::QvisDatabaseCorrelationWindow(
    const DatabaseCorrelation &correlation, const char *caption) :
    QvisWindowBase(caption), createMode(false)
{
    CreateWidgets(correlation);
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationWindow::~QvisDatabaseCorrelationWindow
//
// Purpose: 
//   Destructor for the QvisDatabaseCorrelationWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:25:43 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisDatabaseCorrelationWindow::~QvisDatabaseCorrelationWindow()
{
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationWindow::CreateWidgets
//
// Purpose: 
//   Creates widgets for the window using the specified database correlation.
//
// Arguments:
//   correlation : The database correlation to use when creating the window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:25:59 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisDatabaseCorrelationWindow::CreateWidgets(
    const DatabaseCorrelation &correlation)
{
    // Set the name of this window.
    QString s; s.sprintf("QvisDatabaseCorrelationWindow%04d", instanceCount++);
    setName(s.latin1());

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *topLayout = new QVBoxLayout(central);
    topLayout->setMargin(10);
    topLayout->setSpacing(5);

    // Create the name line edit.
    QGridLayout *gLayout = new QGridLayout(topLayout, 2, 2);
    correlationNameLineEdit = new QLineEdit(central, "correlationNameLineEdit");
    correlationNameLineEdit->setText(correlation.GetName().c_str());
    correlationNameLineEdit->setEnabled(createMode);
    QLabel *nameLabel = new QLabel(correlationNameLineEdit, "Name", central,
        "nameLabel");
    nameLabel->setEnabled(createMode);
    gLayout->addWidget(nameLabel, 0, 0);
    gLayout->addWidget(correlationNameLineEdit, 0, 1);

    // Create the correlation method combobox.
    correlationMethodComboBox = new QComboBox(central, "correlationMethodComboBox");
    correlationMethodComboBox->insertItem("Padded index");
    correlationMethodComboBox->insertItem("Stretched index");
    correlationMethodComboBox->insertItem("Time");
    correlationMethodComboBox->insertItem("Cycle");
    int method = (int)correlation.GetMethod();
    correlationMethodComboBox->setCurrentItem(method);
    gLayout->addWidget(correlationMethodComboBox, 1, 1);
    gLayout->addWidget(new QLabel(correlationMethodComboBox,
        "Correlation method", central), 1, 0);
    topLayout->addSpacing(10);

    // Create the widgets that let us add sources to the database correlation.
    QGridLayout *srcLayout = new QGridLayout(topLayout, 5, 3);
    srcLayout->setSpacing(5);
    const int S[] = {1, 5, 1, 1, 5};
    int i;
    for(i = 0; i < 5; ++i)
        srcLayout->setRowStretch(i, S[i]);
    srcLayout->addWidget(new QLabel("Sources", central, "Sources"), 0, 0);
    srcLayout->addWidget(new QLabel("Correlated sources", central,
        "CorrelatedSources"), 0, 2);

    //
    // Simplify the source names.
    //
    NameSimplifier simplifier;
    const stringVector &sources = GetViewerState()->GetGlobalAttributes()->GetSources();
    for(i = 0; i < sources.size(); ++i)
        simplifier.AddName(sources[i]);
    stringVector shortSources;
    simplifier.GetSimplifiedNames(shortSources);
    std::map<std::string, std::string> shortToLong, longToShort;
    for(i = 0; i < sources.size(); ++i)
    {
        shortToLong[shortSources[i]] = sources[i];
        longToShort[sources[i]] = shortSources[i];
    }

    // Create and populate the list of sources.
    sourcesListBox = new QListBox(central, "sourcesListBox");
    sourcesListBox->setSelectionMode(QListBox::Multi);
    for(i = 0; i < sources.size(); ++i)
    {
        if(!correlation.UsesDatabase(sources[i]))
            sourcesListBox->insertItem(shortSources[i].c_str());
    }
    if(sources.size() > 0)
        sourcesListBox->setCurrentItem(0);
    connect(sourcesListBox, SIGNAL(highlighted(int)),
            this, SLOT(setAddButtonEnabled(int)));
    srcLayout->addMultiCellWidget(sourcesListBox, 1, 4, 0, 0);

    // Create and populate the list of correlated sources.
    correlatedSourcesListBox = new QListBox(central, "correlatedSourcesListBox");
    correlatedSourcesListBox->setSelectionMode(QListBox::Multi);
    const stringVector &dbs = correlation.GetDatabaseNames();
    for(i = 0; i < correlation.GetNumDatabases(); ++i)
        correlatedSourcesListBox->insertItem(longToShort[dbs[i]].c_str());
    if(dbs.size() > 0)
        correlatedSourcesListBox->setCurrentItem(0);
    connect(correlatedSourcesListBox, SIGNAL(highlighted(int)),
            this, SLOT(setRemoveButtonEnabled(int)));
    srcLayout->addMultiCellWidget(correlatedSourcesListBox, 1, 4, 2, 2);

    // Create the add and remove buttons.
    addButton = new QPushButton("-->", central, "addButton");
    connect(addButton, SIGNAL(clicked()),
            this, SLOT(addSources()));
    srcLayout->addWidget(addButton, 2, 1);

    removeButton = new QPushButton("<--", central, "removeButton");
    connect(removeButton, SIGNAL(clicked()),
            this, SLOT(removeSources()));
    srcLayout->addWidget(removeButton, 3, 1);

    UpdateAddRemoveButtonsEnabledState();

    // Add the action and cancel buttons.
    topLayout->addSpacing(10);
    QHBoxLayout *actionButtonLayout = new QHBoxLayout(topLayout);
    QPushButton *actionButton = new QPushButton(
        createMode?"Create database correlation" : "Alter database correlation", central,
        "actionButton");
    connect(actionButton, SIGNAL(clicked()),
            this, SLOT(actionClicked()));
    actionButtonLayout->addWidget(actionButton);
    actionButtonLayout->addStretch(10);
    QPushButton *cancelButton = new QPushButton("Cancel", central, "cancelButton");
    connect(cancelButton, SIGNAL(clicked()),
            this, SLOT(cancelClicked()));
    actionButtonLayout->addWidget(cancelButton);
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationWindow::SelectedCount
//
// Purpose: 
//   Determines how many items are selected in the specified list box.
//
// Arguments:
//   lb : The list box to check.
//
// Returns:    The number of selected items in the specified list box.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:26:55 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

int
QvisDatabaseCorrelationWindow::SelectedCount(const QListBox *lb) const
{
    int selCount = 0;
    for(int i = 0; i < lb->count(); ++i)
        if(lb->isSelected(i))
            ++selCount;

    return selCount;
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationWindow::UpdateAddRemoveButtonsEnabledState
//
// Purpose: 
//   Updates the enabled state for the add/remove buttons.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:27:38 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisDatabaseCorrelationWindow::UpdateAddRemoveButtonsEnabledState()
{
    addButton->setEnabled(SelectedCount(sourcesListBox) > 0);
    removeButton->setEnabled(SelectedCount(correlatedSourcesListBox) > 0);
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationWindow::TransferItems
//
// Purpose: 
//   Transfers the selected items in one list box to the other list box.
//
// Arguments:
//   srcLB  : The source list box that is losing items.
//   destLB : The list box that is gaining items.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:28:06 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisDatabaseCorrelationWindow::TransferItems(QListBox *srcLB, QListBox *destLB)
{
    stringVector src;
    stringVector dest;
    int i;
    for(i = 0; i < srcLB->count(); ++i)
    {
        if(srcLB->isSelected(i))
            dest.push_back(srcLB->text(i).latin1());
        else
            src.push_back(srcLB->text(i).latin1());
    }

    for(i = 0; i < destLB->count(); ++i)
        dest.push_back(destLB->text(i).latin1());

    srcLB->blockSignals(true);
    srcLB->clear();
    for(i = 0; i < src.size(); ++i)
        srcLB->insertItem(src[i].c_str());
    if(src.size() > 0)
        srcLB->setCurrentItem(0);
    srcLB->blockSignals(false);

    destLB->blockSignals(true);
    destLB->clear();
    for(i = 0; i < dest.size(); ++i)
        destLB->insertItem(dest[i].c_str());
    if(dest.size() > 0)
        destLB->setCurrentItem(0);
    destLB->blockSignals(false);

    UpdateAddRemoveButtonsEnabledState();
}

//
// Qt slot functions
//

void
QvisDatabaseCorrelationWindow::setAddButtonEnabled(int)
{
    UpdateAddRemoveButtonsEnabledState();
}

void
QvisDatabaseCorrelationWindow::setRemoveButtonEnabled(int)
{
    UpdateAddRemoveButtonsEnabledState();
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationWindow::addSources
//
// Purpose: 
//   This is a Qt slot function that is called when you want to add sources
//   to the correlation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:29:28 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisDatabaseCorrelationWindow::addSources()
{
    TransferItems(sourcesListBox, correlatedSourcesListBox);
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationWindow::removeSources
//
// Purpose: 
//   This is a Qt slot function that is called when you want to remove sources
//   from the correlation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:29:28 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisDatabaseCorrelationWindow::removeSources()
{
    TransferItems(correlatedSourcesListBox, sourcesListBox);
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationWindow::actionClicked
//
// Purpose: 
//   This is a Qt slot function that is called when you click the button to
//   either create or alter the database correlation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:30:20 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisDatabaseCorrelationWindow::actionClicked()
{
    std::string name;
    stringVector dbs;
    int method = correlationMethodComboBox->currentItem();

    //
    // Get the name from the line edit.
    //
    name = correlationNameLineEdit->displayText().stripWhiteSpace().latin1();

    //
    // If we're creating a new correlation, check the name in the line edit.
    //
    if(createMode)
    {
        if(name.size() < 1)
        {
            Warning("A new database correlation must have a name.");
            correlationNameLineEdit->setActiveWindow();
            correlationNameLineEdit->setFocus();
            correlationNameLineEdit->setSelection(0,
                correlationNameLineEdit->displayText().length());
            return;
        }
        else
        {
            // See if the name is already used.
            DatabaseCorrelationList *cL = GetViewerState()->GetDatabaseCorrelationList();
            if(cL->FindCorrelation(name))
            {
                Warning("The given database correlation name is already "
                        "being used. Please change the name of this "
                        "correlation.");
                correlationNameLineEdit->setActiveWindow();
                correlationNameLineEdit->setFocus();
                correlationNameLineEdit->setSelection(0,
                    correlationNameLineEdit->displayText().length());
                return;
            }
        }
    }

    //
    // Simplify the current source names.
    //
    NameSimplifier simplifier;
    const stringVector &sources = GetViewerState()->GetGlobalAttributes()->GetSources();
    int i;
    for(i = 0; i < sources.size(); ++i)
        simplifier.AddName(sources[i]);
    stringVector shortSources;
    simplifier.GetSimplifiedNames(shortSources);
    std::map<std::string, std::string> shortToLong;
    for(i = 0; i < sources.size(); ++i)
        shortToLong[shortSources[i]] = sources[i];

    // Get the sources from the correlated source list.
    for(i = 0; i < correlatedSourcesListBox->count(); ++i)
    {
        std::string srcName(correlatedSourcesListBox->text(i).latin1());
        dbs.push_back(shortToLong[srcName]);
    }
    if(dbs.size() < 1)
    {
        QString msg;
        msg.sprintf("A database correlation must have at least one correlated "
            "source. You must add a correlated source before you can %s "
            "this database correlation.", (createMode ? "create" : "alter"));
        Warning(msg);
    }

    //
    // Do the right thing in the viewer.
    //
    if(createMode)
    {
        GetViewerMethods()->CreateDatabaseCorrelation(name, dbs, method);
    }
    else
    {
        GetViewerMethods()->AlterDatabaseCorrelation(name, dbs, method);
    }

    cancelClicked();
}

// ****************************************************************************
// Method: QvisDatabaseCorrelationWindow::cancelClicked
//
// Purpose: 
//   This is a Qt slot function that is called when the window is getting
//   dismissed.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 12:31:28 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisDatabaseCorrelationWindow::cancelClicked()
{
    hide();
    emit deleteMe(this);
}
