#include "QvisRemoveCellsWindow.h"

#include <RemoveCellsAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qlistbox.h>
#include <qradiobutton.h>
#include <QvisColorTableButton.h>
#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <stdio.h>
#include <string>

using std::string;

static const char * formatString = "%d (%d)";

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
//   
// ****************************************************************************

QvisRemoveCellsWindow::QvisRemoveCellsWindow(const int type,
                         RemoveCellsAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
    buildingList = false;
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
//   
// ****************************************************************************

void
QvisRemoveCellsWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 6,2,  10, "mainLayout");

    QGridLayout *midLayout = new QGridLayout(mainLayout);
    
    cellList = new QListBox(central);
    connect(cellList, SIGNAL(highlighted(int)),
            this, SLOT(listSelected(int)));
    midLayout->addWidget(cellList, 0,0);
    mainLayout->addMultiCell(midLayout, 0, 2, 0, 0);
    
    changeButton = new QPushButton("Change", central, "Change");
    connect(changeButton, SIGNAL(clicked()),
            this, SLOT(changeButtonClicked()));
    mainLayout->addWidget(changeButton, 0, 1);
    
    removeButton = new QPushButton("Remove", central, "Remove");
    connect(removeButton, SIGNAL(clicked()),
            this, SLOT(removeButtonClicked()));
    mainLayout->addWidget(removeButton, 1, 1);

    addButton = new QPushButton("Add", central, "Add");
    connect(addButton, SIGNAL(clicked()),
            this, SLOT(addButtonClicked()));
    mainLayout->addWidget(addButton, 2,1);
    
    const int fieldWidth = 90;
    
    mainLayout->addWidget(new QLabel("cell", central, "cell"),3,0);
    cell = new QLineEdit("0.0", central, "cell");
    cell->setMaximumWidth(fieldWidth);
    connect(cell, SIGNAL(returnPressed()),
            this, SLOT(cellProcessText()));
    mainLayout->addWidget(cell, 3,1);

    mainLayout->addWidget(new QLabel("domain", central, "domain"),4,0);
    domain = new QLineEdit("0.0", central, "domain");
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
//   
// ****************************************************************************

void
QvisRemoveCellsWindow::UpdateWindow(bool doAll)
{
    QString temp;
    double r;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }
        else if (i == 3) // skip domainList if we've done cellList
            continue;

        const double         *dptr;
        const float          *fptr;
        const int            *iptr;
        const char           *cptr;
        const unsigned char  *uptr;
        const string         *sptr;
        QColor                tempcolor;
        switch(i)
        {
          case 0: //cell
            temp.sprintf("%d", atts->GetCell());
            cell->setText(temp);
            break;
          case 1: //domain
            temp.sprintf("%d", atts->GetDomain());
            domain->setText(temp);
            break;
          case 2: //cellList
          case 3: //domainList
            buildingList = true;
            
            int currentItem = cellList->currentItem();
            // Destroy the list
            cellList->clear();

            // And build it back up from scratch
            const vector<int> &cV = atts->GetCellList();
            const vector<int> &dV = atts->GetDomainList();
            
            int j;
            for (j = 0; j < cV.size(); ++j)
            {
                QString qstr;
                qstr.sprintf(formatString, cV[j], dV[j]);

                cellList->insertItem(qstr);
            }
            
            if (currentItem > 0 && currentItem < cellList->count())
                cellList->setCurrentItem(currentItem);

            buildingList = false;
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
//   
// ****************************************************************************

void
QvisRemoveCellsWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do cell
    if(which_widget == 0 || doAll)
    {
        temp = cell->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetCell(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of cell was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetCell());
            Message(msg);
            atts->SetCell(atts->GetCell());
        }
    }

    // Do domain
    if(which_widget == 1 || doAll)
    {
        temp = domain->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetDomain(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of domain was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetDomain());
            Message(msg);
            atts->SetDomain(atts->GetDomain());
        }
    }

    // Do cellList / domainList
    if(which_widget == 2 || which_widget == 3 || doAll)
    {
        int i;
        vector<int> cV = atts->GetCellList();
        vector<int> dV = atts->GetDomainList();

        cV.resize(cellList->count());
        dV.resize(cellList->count());
        
        for (i = 0; i < cellList->count(); ++i)
        {
            const char *str = cellList->text(i).latin1();
            sscanf(str, formatString, &(cV[i]), &(dV[i]));
        }

        atts->SetCellList(cV);
        atts->SetDomainList(dV);
    }
}


//
// Qt Slot functions
//


void
QvisRemoveCellsWindow::cellProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisRemoveCellsWindow::domainProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisRemoveCellsWindow::listSelected(int index)
{
    // 
    // We don't want to do anything if nothing was really selected,
    // or if this signal came because we were regenerating the list.
    // 
    if (index < 0 || buildingList)
        return;
    const char *str = cellList->text(index).latin1();
    int cell, domain;
    sscanf(str, formatString, &cell, &domain);

    atts->SetCell(cell);
    atts->SetDomain(domain);

    UpdateWindow(false);
}


void
QvisRemoveCellsWindow::addButtonClicked()
{
    int cell, domain;
    
    GetCurrentValues(0);
    GetCurrentValues(1);
    
    cell = atts->GetCell();
    domain = atts->GetDomain();
    
    vector<int> cV = atts->GetCellList();
    vector<int> dV = atts->GetDomainList();

    cV.push_back(cell);
    dV.push_back(domain);
    
    atts->SetCellList(cV);
    atts->SetDomainList(dV);
    
    QString qstr;

    qstr.sprintf(formatString, cell, domain);
    cellList->insertItem(qstr);

    Apply();
}

void
QvisRemoveCellsWindow::removeButtonClicked()
{
    int index = cellList->currentItem();
    if (index < 0)
        return;
    if (index == 0)
    {
        if (cellList->count() == 1)
            cellList->clear();
        else
        {
            cellList->setCurrentItem(1);
            cellList->removeItem(index);
        }
    }
    else
    {
        cellList->setCurrentItem(index - 1);
        cellList->removeItem(index);
    }
    
    GetCurrentValues(3);
}


void
QvisRemoveCellsWindow::changeButtonClicked()
{
    int index = cellList->currentItem();
    if (index < 0)
        return;

    int cell, domain;

    GetCurrentValues(0);
    GetCurrentValues(1);
    
    cell = atts->GetCell();
    domain = atts->GetDomain();
    
    vector<int> cV = atts->GetCellList();
    vector<int> dV = atts->GetDomainList();
    
    cV[index] = cell;
    dV[index] = domain;

    atts->SetCellList(cV);
    atts->SetDomainList(dV);
    
    QString qstr;

    qstr.sprintf(formatString, cell, domain);
    cellList->changeItem(qstr, index);  

    Apply();
}
