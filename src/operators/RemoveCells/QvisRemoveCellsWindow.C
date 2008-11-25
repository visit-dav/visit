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

#include "QvisRemoveCellsWindow.h"

#include <RemoveCellsAttributes.h>
#include <ViewerProxy.h>

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>
#include <QButtonGroup>
#include <QListWidget>
#include <QRadioButton>
#include <QvisColorTableButton.h>
#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <stdio.h>
#include <string>
#include <vector>

using std::string;
using std::vector;


// ****************************************************************************
// Method: QvisRemoveCellsWindow::QvisRemoveCellsWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Thu Jul 17 15:33:35 PST 2003
//
// Modifications:
//   Cyrus Harrison, Thu Aug 21 16:11:16 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

QvisRemoveCellsWindow::QvisRemoveCellsWindow(const int type,
                         RemoveCellsAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisRemoveCellsWindow::~QvisRemoveCellsWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Thu Jul 17 15:33:35 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisRemoveCellsWindow::~QvisRemoveCellsWindow()
{
}


// ****************************************************************************
// Method: QvisRemoveCellsWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Thu Jul 17 15:33:35 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Apr 24 16:52:40 PDT 2008
//   Added tr()'s
//
//   Cyrus Harrison, Thu Aug 21 16:11:16 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

void
QvisRemoveCellsWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout();
    topLayout->addLayout(mainLayout);

    QGridLayout *midLayout = new QGridLayout();
    
    cellList = new QListWidget(central);
    midLayout->addWidget(cellList, 0,0);
    mainLayout->addLayout(midLayout, 0, 0, 3, 1);
    connect(cellList,SIGNAL(currentRowChanged(int)),
            this,SLOT(selectionChanged(int)));
    
    changeButton = new QPushButton(tr("Change"), central);
    connect(changeButton, SIGNAL(clicked()),
            this, SLOT(changeButtonClicked()));
    mainLayout->addWidget(changeButton, 0, 1);
    
    removeButton = new QPushButton(tr("Remove"), central);
    connect(removeButton, SIGNAL(clicked()),
            this, SLOT(removeButtonClicked()));
    mainLayout->addWidget(removeButton, 1, 1);

    addButton = new QPushButton(tr("Add"), central);
    connect(addButton, SIGNAL(clicked()),
            this, SLOT(addButtonClicked()));
    mainLayout->addWidget(addButton, 2,1);
    
    const int fieldWidth = 90;
    
    mainLayout->addWidget(new QLabel(tr("Cell"), central),3,0);
    cell = new QLineEdit("0", central);
    cell->setMaximumWidth(fieldWidth);
    connect(cell, SIGNAL(returnPressed()),
            this, SLOT(cellProcessText()));
    mainLayout->addWidget(cell, 3,1);

    mainLayout->addWidget(new QLabel(tr("Domain"), central),4,0);
    domain = new QLineEdit("0", central);
    domain->setMaximumWidth(fieldWidth);
    connect(domain, SIGNAL(returnPressed()),
            this, SLOT(domainProcessText()));
    mainLayout->addWidget(domain, 4,1);
}



// ****************************************************************************
// Method: QvisRemoveCellsWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Thu Jul 17 15:33:35 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Mon Jun 30 15:15:02 PDT 2008
//   Removed unreferenced variables.
//
//   Cyrus Harrison, Thu Aug 21 16:11:16 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

void
QvisRemoveCellsWindow::UpdateWindow(bool doAll)
{
    QString temp;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        } // skip domainList if we've done cellList
        else if (i == RemoveCellsAttributes::ID_domainList) 
            continue;

        switch(i)
        {
          case RemoveCellsAttributes::ID_cellList:
          case RemoveCellsAttributes::ID_domainList:
            cellList->blockSignals(true);
            int row = cellList->currentRow();
            
            // Destroy the list
            cellList->clear();

            // And build it back up from scratch
            const vector<int> &cV = atts->GetCellList();
            const vector<int> &dV = atts->GetDomainList();
            
            for (int j = 0; j < cV.size(); ++j)
                cellList->addItem(CreateEntryString(cV[j],dV[j]));
            
            if (row > 0 && row < cellList->count())
                cellList->setCurrentRow(row);
            cellList->blockSignals(false);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisRemoveCellsWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Thu Jul 17 15:33:35 PST 2003
//
// Modifications:   
//   Cyrus Harrison, Thu Aug 21 16:11:16 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

void
QvisRemoveCellsWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do cellList / domainList
    if(which_widget == RemoveCellsAttributes::ID_cellList    || 
       which_widget == RemoveCellsAttributes::ID_domainList  || doAll)
    {
        vector<int> cV;
        vector<int> dV;

        cV.resize(cellList->count());
        dV.resize(cellList->count());
        
        for (int i = 0; i < cellList->count(); ++i)
        {
            ParseEntryString(cellList->item(i)->text(),cV[i],dV[i]);
        }

        atts->SetCellList(cV);
        atts->SetDomainList(dV);
    }
}


// ****************************************************************************
// Method: QvisRemoveCellsWindow::ParseEntryString
//
// Purpose: 
//   Helper that parses cell/domain from an entry string.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Jul 17 15:33:35 PST 2003
//
// Modifications:   
//
// ****************************************************************************
void 
QvisRemoveCellsWindow::ParseEntryString(const QString &str, int &cell, int &domain)
{
    QString s = str;
    s.replace("(","");
    s.replace(")","");
    QStringList tok = s.split(" ");  
    cell   = tok[0].toInt();
    domain = tok[1].toInt();
}

// ****************************************************************************
// Method: QvisRemoveCellsWindow::GetCurrentCell
//
// Purpose: 
//   Helper that obtains the current values from the cell/domain line edits.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Jul 17 15:33:35 PST 2003
//
// Modifications:   
//
// ****************************************************************************
void 
QvisRemoveCellsWindow::GetCurrentCell(int &cell, int &domain)
{
    LineEditGetInt(this->cell,cell);
    LineEditGetInt(this->domain,domain);
}

// ****************************************************************************
// Method: QvisRemoveCellsWindow::CreateEntryString
//
// Purpose: 
//   Helper that creates an entry string - "Cell (Domain)"
//
// Programmer: Cyrus Harrison
// Creation:   Thu Jul 17 15:33:35 PST 2003
//
// Modifications:   
//
// ****************************************************************************
QString
QvisRemoveCellsWindow::CreateEntryString(int cell, int domain)
{
    return QString("%1 (%2)").arg(cell).arg(domain);
}

//
// Qt Slot functions
//

void
QvisRemoveCellsWindow::selectionChanged(int row)
{
    if(row <0)
        return;
    int cell_id,domain_id;
    ParseEntryString(cellList->item(row)->text(),cell_id,domain_id);
    cell->setText(IntToQString(cell_id));
    domain->setText(IntToQString(domain_id));
}


void
QvisRemoveCellsWindow::cellProcessText()
{
    int val;
    if(!LineEditGetInt(cell, val))
    {
        ResettingError(tr("Cell"),0);
        cell->setText(0);
    }
        
}


void
QvisRemoveCellsWindow::domainProcessText()
{
    int val;
    if(!LineEditGetInt(domain, val))
    {
        ResettingError(tr("Domain"),0);
        domain->setText(0);        
    }
}


void
QvisRemoveCellsWindow::addButtonClicked()
{
    int cell, domain;
    
    GetCurrentCell(cell,domain);
    
    vector<int> cV = atts->GetCellList();
    vector<int> dV = atts->GetDomainList();

    cV.push_back(cell);
    dV.push_back(domain);
    
    atts->SetCellList(cV);
    atts->SetDomainList(dV);
    
    cellList->addItem(CreateEntryString(cell,domain));

    Apply();
}

void
QvisRemoveCellsWindow::removeButtonClicked()
{
    if(cellList->currentItem())
        delete cellList->currentItem();
    GetCurrentValues(RemoveCellsAttributes::ID_cellList);
}


void
QvisRemoveCellsWindow::changeButtonClicked()
{
    int index = cellList->currentRow();
    if (index < 0)
        return;

    int cell, domain;

    GetCurrentCell(cell,domain);
    
    vector<int> cV = atts->GetCellList();
    vector<int> dV = atts->GetDomainList();
    
    cV[index] = cell;
    dV[index] = domain;

    atts->SetCellList(cV);
    atts->SetDomainList(dV);
    
    cellList->item(index)->setText(CreateEntryString(cell,domain));

    Apply();
}
