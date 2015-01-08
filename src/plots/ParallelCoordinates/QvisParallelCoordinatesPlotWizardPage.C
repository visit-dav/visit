/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <QvisParallelCoordinatesPlotWizardPage.h>
#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QListWidget>

#include <QvisParallelCoordinatesWidget.h>
#include <QvisVariableButton.h>
#include <ParallelCoordinatesAttributes.h>

#include <avtDatabaseMetaData.h>
#include <ExpressionList.h>
#include <QDebug>


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizardPage::QvisParallelCoordinatesPlotWizardPage
//
// Purpose:
//   Constructor for main wizard page for Parallel Coordinates Plot setup.
//
//
// Arguments:
//   s       : Subject to use when communicating with the viewer
//   parent  : Wizard's parent widget
//   varName : Name of the plot's scalar variable
//   md      : Metadata for current database
//   expList : Current set of expressions
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 19 15:16:00 PDT 2006
//
// Note: Refactored from QvisParallelCoordinatesPlotWizard.
//
// Modifications:
//
// ****************************************************************************

QvisParallelCoordinatesPlotWizardPage::QvisParallelCoordinatesPlotWizardPage(
                                   AttributeSubject *s,
                                   QWidget *parent,
                                   const std::string &varName,
                                   const avtDatabaseMetaData *md,
                                   const ExpressionList *expList)
: QWizardPage(parent)
{

    this->varName = varName;

    InitScalarVarNames(md,expList);
    usedVars[QString(varName.c_str())] = true;
    SetupAxisVariableSelectionPage();
    ready = false;
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizardPage::~QvisParallelCoordinatesPlotWizardPage
//
// Purpose: Destructor for the QvisParallelCoordinatesPlotWizardPage class.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Jul  8 10:21:45 PDT 2010
// 
//
// Modifications:
//
//
// ****************************************************************************
QvisParallelCoordinatesPlotWizardPage::~QvisParallelCoordinatesPlotWizardPage()
{

}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizardPage::GetAxisNames
//
// Purpose:
//   Returns the selected axis names.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Jul  8 09:10:49 PDT 2010
//
//
// Modifications:
//
// ****************************************************************************
void
QvisParallelCoordinatesPlotWizardPage::GetAxisNames(stringVector &res)
{
    res.clear();
    int naxisvars = axisVarsList->count();
    for(int row=0; row < naxisvars; row++)
        res.push_back(axisVarsList->item(row)->text().toStdString());

}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizardPage::isComplete
//
// Purpose:
//   Checks to that enables the 'Finish' button.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Jul  8 09:10:49 PDT 2010
//
//
// Modifications:
//
// ****************************************************************************

bool QvisParallelCoordinatesPlotWizardPage::isComplete() const
{
    return ready;
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizardPage::InitScalarVarNames
//
// Purpose: Prepares the list of avaliable scalar variables.
//
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 15:21:33 PDT 2009
//
// Note: Refactored from QvisParallelCoordinatesPlotWizard.
//
// Arguments:
//      md       Metadata for the current active database.
//      exprList The current expression list.
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizardPage::InitScalarVarNames(
                                   const avtDatabaseMetaData *md,
                                   const ExpressionList *exprList)
{
    scalarVarNames.clear();
    scalarExprNames.clear();
    scalarDBExprNames.clear();
    QString ename="";

    for (int i = 0; i < md->GetNumScalars(); ++i)
    {
        const avtScalarMetaData &smd = md->GetScalars(i);
        if (!smd.hideFromGUI && smd.validVariable)
        {
            ename = QString(smd.name.c_str());
            usedVars[ename] = false;
            scalarVarNames.append(ename);
        }
    }

    // Get the user-defined expressions.
    for(int i = 0; i < exprList->GetNumExpressions(); ++i)
    {
        const Expression &e = exprList->GetExpressions(i);
        if(!e.GetHidden() && 
           !e.GetFromDB() && 
           e.GetType() == Expression::ScalarMeshVar)
        {
            ename = QString(e.GetName().c_str());
            usedVars[ename] = false;
            scalarExprNames.append(ename);
        }
    }

    // Get the expressions from the metadata.
    for(int i = 0; i < md->GetNumberOfExpressions(); ++i)
    {
        const Expression *e = md->GetExpression(i);
        if( e != 0 && 
           !e->GetHidden() && 
            e->GetType() == Expression::ScalarMeshVar)
        {
            ename = QString(e->GetName().c_str());
            usedVars[ename] = false;
            scalarDBExprNames.append(ename);
        }
    }
}



// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizardPage::SetupAxisVariableSelectionPage
//
// Purpose:
//   Sets up the wizard page used to select and arrange the axis varibles
//   for a parallel corrdinates plot.
//
// Note: Refactored from QvisParallelCoordinatesPlotWizard.
//
// Programmer: Cyrus Harrison
// Creation:   Wed May  6 13:48:44 PDT 2009
//
// Modifications:
//
// ****************************************************************************

void 
QvisParallelCoordinatesPlotWizardPage::SetupAxisVariableSelectionPage()
{
    setTitle(tr("Select axis variables"));

    QVBoxLayout *page_layout = new QVBoxLayout(this);
    QHBoxLayout *sel_layout = new QHBoxLayout();
    QHBoxLayout *info_layout = new QHBoxLayout();
    QVBoxLayout *button_layout = new QVBoxLayout();

    varsList = new QListWidget(this);
    axisVarsList = new QListWidget(this);

    varsList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    axisVarsList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    infoLbl = new QLabel(
                   tr("Please select at least 2 variables to setup your plot."),
                   this);

    info_layout->addWidget(infoLbl);
    info_layout->addStretch(2);

    addButton = new QPushButton(tr("Add"),this);
    upButton = new QPushButton(tr("Move Up"),this);
    downButton = new QPushButton(tr("Move Down"),this);
    removeButton = new QPushButton(tr("Remove"),this);

    addButton->setEnabled(false);
    upButton->setEnabled(false);
    downButton->setEnabled(false);
    removeButton->setEnabled(false);

    button_layout->addStretch(2);
    button_layout->addWidget(addButton);
    button_layout->addWidget(upButton);
    button_layout->addWidget(downButton);
    button_layout->addWidget(removeButton);
    button_layout->addStretch(2);
    selectionPreview = new QvisParallelCoordinatesWidget(this);


    UpdateVarsList();
    UpdateAxisVarsList();
    UpdatePreview(selectionPreview);

    sel_layout->addWidget(varsList);
    sel_layout->addLayout(button_layout);
    sel_layout->addWidget(axisVarsList);
    page_layout->addWidget(selectionPreview);
    page_layout->addLayout(info_layout);
    page_layout->addLayout(sel_layout);


    connect(varsList, SIGNAL(itemSelectionChanged()), 
            this,SLOT(OnScalarVarSelectionChanged()));

    connect(addButton, SIGNAL(pressed()), 
            this,SLOT(OnAddButtonPressed()));

    connect(upButton, SIGNAL(pressed()), 
            this,SLOT(OnUpButtonPressed()));

    connect(downButton, SIGNAL(pressed()), 
            this,SLOT(OnDownButtonPressed()));

    connect(removeButton, SIGNAL(pressed()), 
            this,SLOT(OnRemoveButtonPressed()));

    connect(axisVarsList, SIGNAL(itemSelectionChanged()), 
            this,SLOT(OnAxisVarSelectionChanged()));
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizardPage::OnScalarVarSelectionChanged
//
// Purpose:
//   Call to update the add button based on selection from the avalaible 
//   scalar vars list widget.
//
// Note: Refactored from QvisParallelCoordinatesPlotWizard.
//
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jul 21 12:32:18 PDT 2008
//
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizardPage::OnScalarVarSelectionChanged()
{
    QList<QListWidgetItem *> selected = varsList->selectedItems();
    if(selected.count() > 0)
        addButton->setEnabled(true);
    else
        addButton->setEnabled(false);
}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizardPage::OnAxisVarSelectionChanged
//
// Purpose:
//   Call to update the up,down & remove buttons based on selection from the 
//   axis vars list widget.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jul 21 12:32:18 PDT 2008
//
// Note: Refactored from QvisParallelCoordinatesPlotWizard.
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizardPage::OnAxisVarSelectionChanged()
{
    QList<QListWidgetItem *> selected = axisVarsList->selectedItems();
    int nitems = axisVarsList->count();
    int nsel = selected.count();
    int min_idx = nitems;
    int max_idx = -1;

    foreach(QListWidgetItem *itm,selected)
    {
        int row = axisVarsList->row(itm);
        if(row < min_idx) min_idx = row;
        if(row > max_idx) max_idx = row;
    }

    if(nsel > 0 && min_idx > 0)
        upButton->setEnabled(true);
    else
        upButton->setEnabled(false);

    if(nsel > 0 && max_idx < nitems -1 )
        downButton->setEnabled(true);
    else
        downButton->setEnabled(false);

    if(nsel > 0)
        removeButton->setEnabled(true);
    else
        removeButton->setEnabled(false);

}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizardPage::OnAddButtonPressed
//
// Purpose:
//  Slot for press of the "add" button.
//
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 09:09:14 PDT 2009
//
// Note: Refactored from QvisParallelCoordinatesPlotWizard.
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizardPage::OnAddButtonPressed()
{

    // take items from
    QList<QListWidgetItem *> selected = varsList->selectedItems();

    foreach(QListWidgetItem *itm,selected)
    {
        axisVarsList->addItem(varsList->takeItem(varsList->row(itm)));
        usedVars[itm->text()] = true;
    }

    if(axisVarsList->count() >1)
    {
        ready = true;
        emit completeChanged();
    }

    UpdatePreview(selectionPreview);
}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizardPage::OnUpButtonPressed
//
// Purpose: 
//  Slot for press of the "up" button.
//
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 09:09:14 PDT 2009
//
// Note: Refactored from QvisParallelCoordinatesPlotWizard.
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizardPage::OnUpButtonPressed()
{
    int nitems = axisVarsList->count();
    QStringList tmp_names;
    QStringList final_names;
    QList<bool> final_selected;
    for(int i= nitems -1; i >=0; i--)
    {
        QListWidgetItem *itm = axisVarsList->item(i);
        if(itm->isSelected())
            tmp_names.append(itm->text());
        else
        {
            final_names.append(itm->text());
            final_selected.append(false);

            foreach(QString name,tmp_names)
            {
                final_names.append(name);
                final_selected.append(true);
            }
            tmp_names.clear();
        }
    }

    foreach(QString name,tmp_names)
    {
        final_names.append(name);
        final_selected.append(true);
    }
    tmp_names.clear();
    axisVarsList->clear();

    for(int i= nitems-1; i >= 0; i--)
    {
        QListWidgetItem *itm = new QListWidgetItem(final_names[i],axisVarsList);
        axisVarsList->addItem(itm);
        if(final_selected[i])
            itm->setSelected(true);

    }
    OnAxisVarSelectionChanged();
    UpdatePreview(selectionPreview);
}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizardPage::OnDownButtonPressed
//
// Purpose:
//  Slot for press of the "down" button.
//
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 09:09:14 PDT 2009
//
// Note: Refactored from QvisParallelCoordinatesPlotWizard.
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizardPage::OnDownButtonPressed()
{
    int nitems = axisVarsList->count();
    QStringList tmp_names;
    QStringList final_names;
    QList<bool> final_selected;
    for(int i=0; i < nitems; i++)
    {
        QListWidgetItem *itm = axisVarsList->item(i);
        if(itm->isSelected())
            tmp_names.append(itm->text());
        else
        {
            final_names.append(itm->text());
            final_selected.append(false);

            foreach(QString name,tmp_names)
            {
                final_names.append(name);
                final_selected.append(true);
            }
            tmp_names.clear();
        }
    }

    foreach(QString name,tmp_names)
    {
        final_names.append(name);
        final_selected.append(true);
    }
    tmp_names.clear();
    axisVarsList->clear();

    for(int i=0;i < nitems; i++)
    {
        QListWidgetItem *itm = new QListWidgetItem(final_names[i],axisVarsList);
        axisVarsList->addItem(itm);
        if(final_selected[i])
            itm->setSelected(true);
    }
    OnAxisVarSelectionChanged();
    UpdatePreview(selectionPreview);
}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizardPage::OnRemoveButtonPressed
//
// Purpose:
//  Slot for press of the "remove" button.
//
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 09:09:14 PDT 2009
//
// Note: Refactored from QvisParallelCoordinatesPlotWizard.
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizardPage::OnRemoveButtonPressed()
{
    // take items from 
    QList<QListWidgetItem *> selected = axisVarsList->selectedItems();

    foreach(QListWidgetItem *itm,selected)
    {
        usedVars[itm->text()] = false;
        axisVarsList->takeItem(axisVarsList->row(itm));
        delete itm;
    }

    if(axisVarsList->count() <=1)
    {
        emit completeChanged();
        ready = false;
    }

    UpdateVarsList();
    UpdatePreview(selectionPreview);
}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizardPage::UpdateVarsList
//
// Purpose:
//  Used to fill the vars list in a consistent manner when items are removed
//  from the axisVarsList.
//
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 09:09:14 PDT 2009
//
// Note: Refactored from QvisParallelCoordinatesPlotWizard.
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizardPage::UpdateVarsList()
{
    varsList->clear();

    foreach(QString var,scalarVarNames)
    {
        if(!usedVars[var])
            varsList->addItem(var);
    }

    if(scalarExprNames.count() > 0 ) // && show_exprs
    {
        foreach(QString var,scalarExprNames)
        {
            if(!usedVars[var])
                varsList->addItem(var);
        }
    }

    if(scalarDBExprNames.count() > 0 )  // && show_db_exprs
    {
        foreach(QString var,scalarDBExprNames)
        {
            if(!usedVars[var])
                varsList->addItem(var);
        }
    }
}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizardPage::UpdateAxisVarsList
//
// Purpose:
//  Updates the axis vars list box using the "usedVars" map.
//
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 09:09:14 PDT 2009
//
// Note: Refactored from QvisParallelCoordinatesPlotWizard.
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizardPage::UpdateAxisVarsList()
{
    axisVarsList->clear();

    QMapIterator<QString,bool> itr(usedVars);

    while(itr.hasNext())
    {
        itr.next();
        if(itr.value() == true)
            axisVarsList->addItem(itr.key());
    }
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizardPage::UpdatePreview
//
// Purpose:
//  Refreshes a parallel coordinates preview widget.
//
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 09:09:14 PDT 2009
//
// Note: Refactored from QvisParallelCoordinatesPlotWizard.
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizardPage::UpdatePreview
(QvisParallelCoordinatesWidget *preview)
{
    stringVector axisVars;

    int naxisvars = axisVarsList->count();
    for(int row=0; row < naxisvars; row++)
        axisVars.push_back(axisVarsList->item(row)->text().toStdString());

    while(axisVars.size() < 2)
        axisVars.push_back("?");

    preview->setNumberOfAxes((int)axisVars.size());
    preview->setAxisTitles(axisVars);
    preview->redrawAllAxes(true);
}



