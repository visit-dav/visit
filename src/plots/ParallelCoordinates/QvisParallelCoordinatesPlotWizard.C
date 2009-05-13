/*****************************************************************************
*
* Copyright (c) 2000 - 2008, The Regents of the University of California
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

#include <QvisParallelCoordinatesPlotWizard.h>
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

#define parAxisAtts ((ParallelCoordinatesAttributes *)localCopy)


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::QvisParallelCoordinatesPlotWizard
//
// Purpose: 
//   Constructor for the ParallelCoordinates plot wizard.
//
// Note: This is intended to emulate the style of the QvisScatterPlotWizard
//       for the Scatter plot, which came first.
//
// Arguments:
//   s       : Subject to use when communicating with the viewer
//   parent  : Wizard's parent widget
//   varNeme : Name of the plot's scalar variable
//   name    : Wizard's name
//
// Programmer: Mark Blair
// Creation:   Mon Jun 19 15:16:00 PDT 2006
//
// Modifications:
//    Jeremy Meredith, Thu Feb  7 12:58:15 EST 2008
//    A wizard is needed because you can't reset the default plot attributes
//    without a wizard's accept action having been called.  If you don't, then
//    you'll have the wrong number of axes defined in the plot attributes.
//    As such, I extended the wizard to support a "no-op" mode.
//
//    Brad Whitlock, Wed Apr 23 10:12:44 PDT 2008
//    Added tr()
//  
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port. 
//
//    Cyrus Harrison, Wed May 13 08:30:56 PDT 2009
//    Overhauled to create a simpler list widget based interface.
//
// ****************************************************************************

QvisParallelCoordinatesPlotWizard::QvisParallelCoordinatesPlotWizard(
                                   AttributeSubject *s, 
                                   QWidget *parent,
                                   const std::string &varName,
                                   const avtDatabaseMetaData *md,
                                   const ExpressionList *expList,
                                   bool doNothing) 
: QvisWizard(s, parent) 
{
    this->varName = varName;

    if (doNothing)
    {
        parAxisAtts->ResetAxes();
        SetupFinishPage(tr("Click Finish"),
                        tr("The plot has successfully been "
                           "defined by use of an array variable."), false);
        numPages = 1;
        return;    
    }

    // Set the wizard title.
    topLevelWidget()->setWindowTitle(tr("ParallelCoordinates Plot Wizard"));
    
    InitScalarVarNames(md,expList);
    usedVars[QString(varName.c_str())] = true;
    
    
    SetupAxisVariableSelectionPage();
    SetupFinishPage(tr("Click Finish"),
        tr("Click the Finish button to create a new ParallelCoordinates plot."),
        true);
    SetParallelCoordinatesAttributes();
    numPages = 2;
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::~QvisParallelCoordinatesPlotWizard
//
// Purpose: Destructor for the QvisParallelCoordinatesPlotWizard class.
//
// Programmer: Mark Blair
// Creation:   Mon Jun 19 15:16:00 PDT 2006
//
// Modifications:
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port. 
// 
//    Cyrus Harrison, Wed May 13 08:30:56 PDT 2009
//    Added parent widget to all wizard pages, so we no longer need to 
//    explicitly delete these wizard page widgets.
//
// ****************************************************************************

QvisParallelCoordinatesPlotWizard::~QvisParallelCoordinatesPlotWizard()
{

}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::validateCurrentPage
//
// Purpose: 
//   Validates the current wizard page's inputs before moving 
//   to the next. 
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jul 21 12:32:18 PDT 2008
//
//  
// Modifications:
//  Cyrus Harrison,Thu May  7 15:21:33 PDT 2009
//  Changed b/c of wizard overhaul.
//
// ****************************************************************************

bool
QvisParallelCoordinatesPlotWizard::validateCurrentPage()
{
    int id = currentId();
    
    if(id == 0 && numPages == 2)
    {
        // make sure we have at least two vars
        if(axisVarsList->count() < 2)
        {
            warnLbl->setText(tr("Please select at least 2 variables."));
            return false;
        }

        warnLbl->setText(" ");

        UpdatePreview(finalPreview);
        button(QWizard::NextButton)->setEnabled(false);
    }
    else if(id == 1)
    {   
        // we are finished, set the plot atts.
        SetParallelCoordinatesAttributes();
    }
    
    return true;
}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::GetScalarVarNames
//
// Purpose: Prepares the list of avaliable scalar variables.
//
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 15:21:33 PDT 2009
//
// Arguments:
//      md       Metadata for the current active database.
//      exprList The current expression list.
//  
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::InitScalarVarNames(
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
// Method: QvisParallelCoordinatesPlotWizard::SetupAxisVariableSelectionPage
//
// Purpose: 
//   Creates a single wizard page to select and arrange the axis varibles
//   for a parallel corrdinates plot.
//
// Programmer: Cyrus Harrison
// Creation:   Wed May  6 13:48:44 PDT 2009
//  
// Modifications:
//
// ****************************************************************************

void 
QvisParallelCoordinatesPlotWizard::SetupAxisVariableSelectionPage()
{
    QWizardPage *page = new QWizardPage(this);
    page->setTitle(tr("Select axis variables"));
    
    QVBoxLayout *page_layout = new QVBoxLayout(page);
    QHBoxLayout *sel_layout = new QHBoxLayout();
    QHBoxLayout *warn_layout = new QHBoxLayout();
    QVBoxLayout *button_layout = new QVBoxLayout();
    
    varsList = new QListWidget(page);
    axisVarsList = new QListWidget(page);
    
    varsList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    axisVarsList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    
    warnLbl = new QLabel(" ",this);
    QPalette palette;
    palette.setColor(QPalette::Foreground, QColor(Qt::red));
    warnLbl->setPalette(palette);
    
    warn_layout->addStretch(2);
    warn_layout->addWidget(warnLbl);
    
    addButton = new QPushButton(tr("Add"),page);
    upButton = new QPushButton(tr("Move Up"),page);
    downButton = new QPushButton(tr("Move Down"),page);
    removeButton = new QPushButton(tr("Remove"),page);
    
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
    selectionPreview = new QvisParallelCoordinatesWidget(page);
    
    UpdateVarsList();
    UpdateAxisVarsList();
    UpdatePreview(selectionPreview);
    
    sel_layout->addWidget(varsList);
    sel_layout->addLayout(button_layout);
    sel_layout->addWidget(axisVarsList);
    page_layout->addWidget(selectionPreview);
    page_layout->addLayout(warn_layout);
    page_layout->addLayout(sel_layout);
    
    addPage(page);
    
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
// Method: QvisParallelCoordinatesPlotWizard::OnScalarVarSelectionChanged
//
// Purpose: 
//   Call to update the add button based on selection from the avalaible 
//   scalar vars list widget.
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
QvisParallelCoordinatesPlotWizard::OnScalarVarSelectionChanged()
{
    QList<QListWidgetItem *> selected = varsList->selectedItems();
    if(selected.count() > 0)
        addButton->setEnabled(true);
    else
        addButton->setEnabled(false);
}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::OnAxisVarSelectionChanged
//
// Purpose: 
//   Call to update the up,down & remove buttons based on selection from the 
//   axis vars list widget.
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
QvisParallelCoordinatesPlotWizard::OnAxisVarSelectionChanged()
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
// Method: QvisParallelCoordinatesPlotWizard::OnAddButtonPressed
//
// Purpose: 
//  Slot for press of the "add" button.
// 
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 09:09:14 PDT 2009
//
//  
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::OnAddButtonPressed()
{
    // take items from 
    QList<QListWidgetItem *> selected = varsList->selectedItems();
     
    foreach(QListWidgetItem *itm,selected)
    {
        axisVarsList->addItem(varsList->takeItem(varsList->row(itm)));
        usedVars[itm->text()] = true;
    }
    
    if(axisVarsList->count() >1)
        warnLbl->setText(" ");
    
    UpdatePreview(selectionPreview);
}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::OnUpButtonPressed
//
// Purpose: 
//  Slot for press of the "up" button.
// 
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 09:09:14 PDT 2009
//
//  
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::OnUpButtonPressed()
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
// Method: QvisParallelCoordinatesPlotWizard::OnDownButtonPressed
//
// Purpose: 
//  Slot for press of the "down" button.
// 
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 09:09:14 PDT 2009
//
//  
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::OnDownButtonPressed()
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
// Method: QvisParallelCoordinatesPlotWizard::OnRemoveButtonPressed
//
// Purpose: 
//  Slot for press of the "remove" button.
// 
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 09:09:14 PDT 2009
//
//  
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::OnRemoveButtonPressed()
{
    // take items from 
    QList<QListWidgetItem *> selected = axisVarsList->selectedItems();
     
    foreach(QListWidgetItem *itm,selected)
    {
        usedVars[itm->text()] = false;
        axisVarsList->takeItem(axisVarsList->row(itm));
        delete itm;
    }
    
    UpdateVarsList();
    UpdatePreview(selectionPreview);
}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::UpdateVarsList
//
// Purpose: 
//  Used to fill the vars list in a consistent manner when items are removed
//  from the axisVarsList.
// 
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 09:09:14 PDT 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::UpdateVarsList()
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
// Method: QvisParallelCoordinatesPlotWizard::UpdateAxisVarsList
//
// Purpose: 
//  Updates the axis vars list box using the "usedVars" map.
// 
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 09:09:14 PDT 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::UpdateAxisVarsList()
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
// Method: QvisParallelCoordinatesPlotWizard::UpdatePreview
//
// Purpose: 
//  Refreshes a parallel coordinates preview widget.
// 
// Programmer: Cyrus Harrison
// Creation:   Thu May  7 09:09:14 PDT 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::UpdatePreview
(QvisParallelCoordinatesWidget *preview)
{
    stringVector axisVars;
    
    int naxisvars = axisVarsList->count();
    for(int row=0; row < naxisvars; row++)
        axisVars.push_back(axisVarsList->item(row)->text().toStdString());
    
    while(axisVars.size() < 2)
        axisVars.push_back("?");
        
    preview->setNumberOfAxes(axisVars.size());
    preview->setAxisTitles(axisVars);
    preview->redrawAllAxes(true);
}



// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::SetupFinishPage
//
// Purpose: 
//   Creates final wizard page
//
// Arguments:
//   title       : Page title.
//   promptText  : User prompt to display.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jul 21 12:32:18 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::SetupFinishPage(const QString &title,
                                                   const QString &promptText,
                                                   bool preview)
{
    QWizardPage *page= new QWizardPage(this);
    page->setTitle(title);
    page->setFinalPage(true);
    
    QVBoxLayout *page_layout = new QVBoxLayout(page);
    
    
    if(preview)
    {
        finalPreview = new QvisParallelCoordinatesWidget(page);
        page_layout->addWidget(finalPreview);
    }

    if (!promptText.isEmpty())
    {
        QLabel *prompt = new QLabel(promptText, page);
        page_layout->addWidget(prompt);
        page_layout->addStretch(2);
    }
    addPage(page);
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::SetParallelCoordinatesAttributes
//
// Purpose: Sets local copy of plot's attributes
//
// Arguments:
//
// Programmer: Jeremy Meredith
// Creation:   January 31, 2008
//
// Modifications:
//    Jeremy Meredith, Fri Feb  1 17:56:59 EST 2008
//    Made limits be in terms of actual values, not normalized 0..1.
//
//    Jeremy Meredith, Mon Feb  4 16:06:57 EST 2008
//    Drop the axis extents in the plot attributes -- they were not used.
//
//    Jeremy Meredith, Fri Feb 15 13:16:46 EST 2008
//    Renamed orderedAxisNames to scalarAxisNames to distinguish these
//    as names of actual scalars instead of just display names.  Added
//    visualAxisNames.
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::SetParallelCoordinatesAttributes()
{
    stringVector saxisNames;
    stringVector vaxisNames;
    doubleVector extMins;
    doubleVector extMaxs;
    
    if(numPages == 1)
    {
        saxisNames.push_back(varName);
        vaxisNames.push_back(varName);
        extMins.push_back(-1e+37);
        extMaxs.push_back(+1e+37);
    }
    else if(numPages == 2)
    {
        int nvars = axisVarsList->count();
    
        for(int i=0;i < nvars;i++)
        {   
            std::string vname = axisVarsList->item(i)->text().toStdString();
            saxisNames.push_back(vname);
            vaxisNames.push_back(vname);
            extMins.push_back(-1e+37);
            extMaxs.push_back(+1e+37);
        }    
    }
    
    parAxisAtts->SetScalarAxisNames(saxisNames);
    parAxisAtts->SetVisualAxisNames(vaxisNames);
    parAxisAtts->SetExtentMinima(extMins);
    parAxisAtts->SetExtentMaxima(extMaxs);
}

