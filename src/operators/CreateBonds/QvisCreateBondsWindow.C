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

#include "QvisCreateBondsWindow.h"

#include <CreateBondsAttributes.h>
#include <AtomicProperties.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qlistbox.h>
#include <qheader.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <QvisColorTableButton.h>
#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisVariableButton.h>
#include <ImproperUseException.h>
#include <QvisElementButton.h>

#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisCreateBondsWindow::QvisCreateBondsWindow
//
// Purpose: 
//   Constructor
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Modifications:
//   
// ****************************************************************************

QvisCreateBondsWindow::QvisCreateBondsWindow(const int type,
                         CreateBondsAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisCreateBondsWindow::~QvisCreateBondsWindow
//
// Purpose: 
//   Destructor
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Modifications:
//   
// ****************************************************************************

QvisCreateBondsWindow::~QvisCreateBondsWindow()
{
}


// ****************************************************************************
// Method: QvisCreateBondsWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisCreateBondsWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 3,2,  10, "mainLayout");

    elementVariableLabel = new QLabel("Variable for atomic number", central, "elementVariableLabel");
    mainLayout->addWidget(elementVariableLabel,0,0);
    int elementVariableMask = QvisVariableButton::Scalars;
    elementVariable = new QvisVariableButton(true, true, true, elementVariableMask, central, "elementVariable");
    connect(elementVariable, SIGNAL(activated(const QString&)),
            this, SLOT(elementVariableChanged(const QString&)));
    mainLayout->addWidget(elementVariable, 0,1);

    maxBondsLabel = new QLabel("Maximum bonds per atom", central, "maxBondsLabel");
    mainLayout->addWidget(maxBondsLabel,1,0);
    maxBonds = new QLineEdit(central, "maxBonds");
    mainLayout->addWidget(maxBonds,1,1);
    connect(maxBonds, SIGNAL(returnPressed()),
            this, SLOT(maxBondsReturnPressed()));

    // ------------------------------------------------------------------------

    QGroupBox *bondsListGroup = new QGroupBox("Bonds", central);
    mainLayout->addMultiCellWidget(bondsListGroup, 2,2, 0,1);
    mainLayout->setRowStretch(2, 1000);

    QVBoxLayout *listLayout = new QVBoxLayout(bondsListGroup, 10);
    listLayout->addSpacing(10);

    bondsList = new QListView(bondsListGroup);
    bondsList->setResizeMode(QListView::AllColumns);
    bondsList->addColumn("1st");
    bondsList->addColumn("2nd");
    bondsList->addColumn("Min");
    bondsList->addColumn("Max");
    bondsList->header()->setClickEnabled(false);
    bondsList->header()->setMovingEnabled(false);
    bondsList->setAllColumnsShowFocus(true);
    bondsList->setSorting(-1);
    listLayout->addWidget(bondsList);


    QHBoxLayout *listButtonLayout = new QHBoxLayout(listLayout);
    //listLayout->add

    newButton = new QPushButton("New", bondsListGroup);
    listButtonLayout->addWidget(newButton);

    delButton = new QPushButton("Del", bondsListGroup);
    listButtonLayout->addWidget(delButton);

    connect(newButton, SIGNAL(pressed()),
            this, SLOT(bondsListNew()));
    connect(delButton, SIGNAL(pressed()),
            this, SLOT(bondsListDel()));


    QGroupBox *bondsDetailsGroup = new QGroupBox("Details", bondsListGroup);
    listLayout->addWidget(bondsDetailsGroup);

    QVBoxLayout *bondDetailsTopLayout = new QVBoxLayout(bondsDetailsGroup, 10);
    bondDetailsTopLayout->addSpacing(10);

    QHBoxLayout *bondDetailsLayout = new QHBoxLayout(bondDetailsTopLayout);
    firstElement = new QvisElementButton(bondsDetailsGroup, "firstElement");
    secondElement = new QvisElementButton(bondsDetailsGroup, "secondElement");
    bondDetailsLayout->addWidget(new QLabel("1st:", bondsDetailsGroup));
    bondDetailsLayout->addWidget(firstElement);
    bondDetailsLayout->addWidget(new QLabel("2nd:", bondsDetailsGroup));
    bondDetailsLayout->addWidget(secondElement);

    QHBoxLayout *bondDetailsLayout2 = new QHBoxLayout(bondDetailsTopLayout);
    bondDetailsLayout2->addWidget(new QLabel("Min:", bondsDetailsGroup));
    minDist = new QLineEdit(bondsDetailsGroup);
    bondDetailsLayout2->addWidget(minDist);
    
    bondDetailsLayout2->addWidget(new QLabel("Max:", bondsDetailsGroup));
    maxDist = new QLineEdit(bondsDetailsGroup);
    bondDetailsLayout2->addWidget(maxDist);

    connect(minDist, SIGNAL(textChanged(const QString&)),
            this, SLOT(minDistTextChanged(const QString&)));
    connect(maxDist, SIGNAL(textChanged(const QString&)),
            this, SLOT(maxDistTextChanged(const QString&)));

    connect(minDist, SIGNAL(returnPressed()),
            this, SLOT(minDistReturnPressed()));
    connect(maxDist, SIGNAL(returnPressed()),
            this, SLOT(maxDistReturnPressed()));

    connect(firstElement, SIGNAL(selectedElement(int)),
            this, SLOT(firstElementChanged(int)));
    connect(secondElement, SIGNAL(selectedElement(int)),
            this, SLOT(secondElementChanged(int)));

    connect(bondsList, SIGNAL(selectionChanged()),
            this, SLOT(UpdateWindowSingleItem()));

    // ------------------------------------------------------------------------

    
}


// ****************************************************************************
// Method: QvisCreateBondsWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisCreateBondsWindow::UpdateWindow(bool doAll)
{
    bool update_bonds_list = false;
    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }

        switch(i)
        {
          case 0: //elementVariable
            elementVariable->setText(atts->GetElementVariable().c_str());
            break;
          case 1: //atomicNumber1
          case 2: //atomicNumber2
          case 3: //minDist
          case 4: //maxDist
            update_bonds_list = true;
            break;
          case 10: //maxBondsClamp
            maxBonds->setText(QString().sprintf("%d",atts->GetMaxBondsClamp()));
            break;
        default:
            break;
        }
    }

    if (update_bonds_list)
    {
        int old_index = GetItemIndex(bondsList->currentItem());
        QListViewItem *new_item = NULL;
        bondsList->clear();
        int n = GetListLength();
        //for (int i=0; i<n; i++)
        for (int i=n-1; i>=0; i--)
        {
            QListViewItem *item =
                new QListViewItem(bondsList,
                                  element_names[atts->GetAtomicNumber1()[i]],
                                  element_names[atts->GetAtomicNumber2()[i]],
                                  QString().sprintf("%.4f",atts->GetMinDist()[i]),
                                  QString().sprintf("%.4f",atts->GetMaxDist()[i]));
            if (old_index == i)
                new_item = item;
        }
        if (new_item)
        {
            bondsList->setSelected(new_item, true);
        }
    }
    UpdateWindowSingleItem();
}


// ****************************************************************************
// Method: QvisCreateBondsWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisCreateBondsWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do elementVariable
    if(which_widget == 0 || doAll)
    {
        // Nothing for elementVariable
    }

    // Do atomicNumber1
    if(which_widget == 1 || doAll)
    {
        //writeSourceGetCurrent unknown for intVector (variable atomicNumber1)
    }

    // Do atomicNumber2
    if(which_widget == 2 || doAll)
    {
        //writeSourceGetCurrent unknown for intVector (variable atomicNumber2)
    }

    // Do minDist
    if(which_widget == 3 || doAll)
    {
        //writeSourceGetCurrent unknown for doubleVector (variable minDist)
    }

    // Do maxDist
    if(which_widget == 4 || doAll)
    {
        //writeSourceGetCurrent unknown for doubleVector (variable maxDist)
    }

    // Do maxBondsClamp
    if(which_widget == 10 || doAll)
    {
        int oldval = atts->GetMaxBondsClamp();
        int newval = maxBonds->displayText().toInt();
        if (newval < 1 || newval > 100)
        {
            Message("The value for maxBonds must be between 1 and 100.");
            atts->SetMaxBondsClamp(oldval);
        }
        else
        {
            atts->SetMaxBondsClamp(newval);
        }
    }

}


//
// Qt Slot functions
//


void
QvisCreateBondsWindow::elementVariableChanged(const QString &varName)
{
    atts->SetElementVariable(varName.latin1());
    SetUpdate(false);
    Apply();
    cerr << "atts.size = "<<atts->GetMinDist().size()<<endl;
}


//writeSourceCallback unknown for intVector (variable atomicNumber1)


//writeSourceCallback unknown for intVector (variable atomicNumber2)


//writeSourceCallback unknown for doubleVector (variable minDist)


//writeSourceCallback unknown for doubleVector (variable maxDist)
void QvisCreateBondsWindow::UpdateWindowSingleItem()
{
    int index = GetItemIndex(bondsList->currentItem());

    int n = GetListLength();
    if (index < 0 || index >= n)
    {
        firstElement->setEnabled(false);
        secondElement->setEnabled(false);
        minDist->setEnabled(false);
        maxDist->setEnabled(false);
        return;
    }

    firstElement->setEnabled(true);
    secondElement->setEnabled(true);
    minDist->setEnabled(true);
    maxDist->setEnabled(true);

    firstElement->setElementNumber(atts->GetAtomicNumber1()[index]);
    secondElement->setElementNumber(atts->GetAtomicNumber2()[index]);
    minDist->setText(QString().sprintf("%.4f",atts->GetMinDist()[index]));
    maxDist->setText(QString().sprintf("%.4f",atts->GetMaxDist()[index]));
}

void QvisCreateBondsWindow::bondsListNew()
{
    atts->GetAtomicNumber1().push_back(0);
    atts->GetAtomicNumber2().push_back(0);
    atts->GetMinDist().push_back(0.4);
    atts->GetMaxDist().push_back(1.2);
    atts->SelectAtomicNumber1();
    atts->SelectAtomicNumber2();
    atts->SelectMinDist();
    atts->SelectMaxDist();
    Apply();
    bondsList->setSelected(bondsList->lastItem(), true);
    //UpdateWindow(true);
}

int QvisCreateBondsWindow::GetItemIndex(QListViewItem *item1)
{
    int index = 0;

    QListViewItem *item2 = bondsList->firstChild();
    while (item1 != item2 && item2)
    {
        index++;
        item2 = item2->nextSibling();
    }

    if (!item2)
        return -1;
    else
        return index;
}

void QvisCreateBondsWindow::minDistReturnPressed()
{
    Apply();
}

void QvisCreateBondsWindow::maxDistReturnPressed()
{
    Apply();
}

void QvisCreateBondsWindow::maxBondsReturnPressed()
{
    GetCurrentValues(10);
    Apply();
}

void QvisCreateBondsWindow::minDistTextChanged(const QString &txt)
{
    QListViewItem *item = bondsList->currentItem();
    int n = GetListLength();
    int index = GetItemIndex(item);
    if (index < 0 || index >= n)
        return;

    atts->GetMinDist()[index] = minDist->displayText().toFloat();
    item->setText(2, QString().sprintf("%.4f",atts->GetMinDist()[index]));
    atts->SelectMinDist();
}

void QvisCreateBondsWindow::maxDistTextChanged(const QString &txt)
{
    QListViewItem *item = bondsList->currentItem();
    int n = GetListLength();
    int index = GetItemIndex(item);
    if (index < 0 || index >= n)
        return;
    
    atts->GetMaxDist()[index] = maxDist->displayText().toFloat();
    item->setText(3, QString().sprintf("%.4f",atts->GetMaxDist()[index]));
    atts->SelectMaxDist();
}

void QvisCreateBondsWindow::firstElementChanged(int element)
{
    QListViewItem *item = bondsList->currentItem();
    int n = GetListLength();
    int index = GetItemIndex(bondsList->currentItem());
    if (index < 0 || index >= n)
        return;

    atts->GetAtomicNumber1()[index] = element;
    item->setText(0, element_names[element]);
    Apply();
    atts->SelectAtomicNumber1();
}

void QvisCreateBondsWindow::secondElementChanged(int element)
{
    QListViewItem *item = bondsList->currentItem();
    int n = GetListLength();
    int index = GetItemIndex(bondsList->currentItem());
    if (index < 0 || index >= n)
        return;

    atts->GetAtomicNumber2()[index] = element;
    item->setText(1, element_names[element]);
    Apply();
    atts->SelectAtomicNumber2();
}

int QvisCreateBondsWindow::GetListLength()
{
    int n1 = atts->GetAtomicNumber1().size();
    int n2 = atts->GetAtomicNumber2().size();
    int n3 = atts->GetMinDist().size();
    int n4 = atts->GetMaxDist().size();
    if (n1 != n2 || n1 != n3 || n1 != n4)
    {
        EXCEPTION1(ImproperUseException,
                   "Bond list data arrays were not all the same length.");
    }
    return n1;
}

void QvisCreateBondsWindow::bondsListDel()
{
    int n = GetListLength();
    int index = GetItemIndex(bondsList->currentItem());
    if (index < 0 || index >= n)
        return;


    for (int i=index; i<n-1; i++)
    {
        atts->GetAtomicNumber1()[i] = atts->GetAtomicNumber1()[i+1];
        atts->GetAtomicNumber2()[i] = atts->GetAtomicNumber2()[i+1];
        atts->GetMinDist()[i] = atts->GetMinDist()[i+1];
        atts->GetMaxDist()[i] = atts->GetMaxDist()[i+1];
    }
    atts->GetAtomicNumber1().resize(n-1);
    atts->GetAtomicNumber2().resize(n-1);
    atts->GetMinDist().resize(n-1);
    atts->GetMaxDist().resize(n-1);
    UpdateWindow(true);
    atts->SelectAtomicNumber1();
    atts->SelectAtomicNumber2();
    atts->SelectMinDist();
    atts->SelectMaxDist();
    if (bondsList->currentItem())
        bondsList->setSelected(bondsList->currentItem(), true);
}

