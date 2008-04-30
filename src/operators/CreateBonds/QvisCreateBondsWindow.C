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
                         const QString &caption,
                         const QString &shortName,
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
//   Jeremy Meredith, Mon Feb 11 16:52:06 EST 2008
//   Support wildcards in matches.  This added a need for an up/down button
//   (since order is now significant).
//
//   Brad Whitlock, Fri Apr 25 09:21:43 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

void
QvisCreateBondsWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 3,2,  10, "mainLayout");

    elementVariableLabel = new QLabel(tr("Variable for atomic number"), central, "elementVariableLabel");
    mainLayout->addWidget(elementVariableLabel,0,0);
    int elementVariableMask = QvisVariableButton::Scalars;
    elementVariable = new QvisVariableButton(true, true, true, elementVariableMask, central, "elementVariable");
    connect(elementVariable, SIGNAL(activated(const QString&)),
            this, SLOT(elementVariableChanged(const QString&)));
    mainLayout->addWidget(elementVariable, 0,1);

    maxBondsLabel = new QLabel(tr("Maximum bonds per atom"), central, "maxBondsLabel");
    mainLayout->addWidget(maxBondsLabel,1,0);
    maxBonds = new QLineEdit(central, "maxBonds");
    mainLayout->addWidget(maxBonds,1,1);
    connect(maxBonds, SIGNAL(returnPressed()),
            this, SLOT(maxBondsReturnPressed()));

    // ------------------------------------------------------------------------

    QGroupBox *bondsListGroup = new QGroupBox(tr("Bonds"), central);
    mainLayout->addMultiCellWidget(bondsListGroup, 2,2, 0,1);
    mainLayout->setRowStretch(2, 1000);

    QVBoxLayout *listLayout = new QVBoxLayout(bondsListGroup, 10);
    listLayout->addSpacing(15);

    QLabel *noteLabel = new QLabel(tr("Note: first match is taken, so "
                                      "order is significant"),
                                   bondsListGroup);
    QFont ital(noteLabel->font());
    ital.setItalic(true);
    noteLabel->setFont(ital);
    //noteLabel->font().setItalic(true);
    listLayout->addWidget(noteLabel);

    bondsList = new QListView(bondsListGroup);
    bondsList->addColumn(tr("1st"));
    bondsList->addColumn(tr("2nd"));
    bondsList->addColumn(tr("Min"));
    bondsList->addColumn(tr("Max"));
    bondsList->setColumnWidth(0, 50);
    bondsList->setColumnWidth(1, 50);
    bondsList->setResizeMode(QListView::AllColumns);
    bondsList->header()->setClickEnabled(false);
    bondsList->header()->setMovingEnabled(false);
    bondsList->setAllColumnsShowFocus(true);
    bondsList->setSorting(-1);
    listLayout->addWidget(bondsList);


    QHBoxLayout *listButtonLayout = new QHBoxLayout(listLayout);
    //listLayout->add

    newButton = new QPushButton(tr("New"), bondsListGroup);
    listButtonLayout->addWidget(newButton);

    delButton = new QPushButton(tr("Del"), bondsListGroup);
    listButtonLayout->addWidget(delButton);

    upButton = new QPushButton(tr("Up"), bondsListGroup);
    listButtonLayout->addWidget(upButton);

    downButton = new QPushButton(tr("Down"), bondsListGroup);
    listButtonLayout->addWidget(downButton);

    connect(newButton, SIGNAL(pressed()),
            this, SLOT(bondsListNew()));
    connect(delButton, SIGNAL(pressed()),
            this, SLOT(bondsListDel()));
    connect(upButton, SIGNAL(pressed()),
            this, SLOT(bondsListUp()));
    connect(downButton, SIGNAL(pressed()),
            this, SLOT(bondsListDown()));


    QGroupBox *bondsDetailsGroup = new QGroupBox(tr("Details"), bondsListGroup);
    listLayout->addWidget(bondsDetailsGroup);

    QVBoxLayout *bondDetailsTopLayout = new QVBoxLayout(bondsDetailsGroup, 10);
    bondDetailsTopLayout->addSpacing(10);

    QHBoxLayout *bondDetailsLayout = new QHBoxLayout(bondDetailsTopLayout);
    firstElement = new QvisElementButton(bondsDetailsGroup, "firstElement");
    secondElement = new QvisElementButton(bondsDetailsGroup, "secondElement");
    bondDetailsLayout->addWidget(new QLabel(tr("1st:"), bondsDetailsGroup));
    bondDetailsLayout->addWidget(firstElement);
    bondDetailsLayout->addWidget(new QLabel(tr("2nd:"), bondsDetailsGroup));
    bondDetailsLayout->addWidget(secondElement);

    QHBoxLayout *bondDetailsLayout2 = new QHBoxLayout(bondDetailsTopLayout);
    bondDetailsLayout2->addWidget(new QLabel(tr("Min:"), bondsDetailsGroup));
    minDist = new QLineEdit(bondsDetailsGroup);
    bondDetailsLayout2->addWidget(minDist);
    
    bondDetailsLayout2->addWidget(new QLabel(tr("Max:"), bondsDetailsGroup));
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
//    Jeremy Meredith, Mon Feb 11 16:52:55 EST 2008
//    Changed to use IDs.  Added support for atomic number of -1 means "any".
//    Clear selection after regenerating list if nothing used to be selected.
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
          case CreateBondsAttributes::ID_elementVariable:
            elementVariable->setText(atts->GetElementVariable().c_str());
            break;
          case CreateBondsAttributes::ID_atomicNumber1:
          case CreateBondsAttributes::ID_atomicNumber2:
          case CreateBondsAttributes::ID_minDist:
          case CreateBondsAttributes::ID_maxDist:
            update_bonds_list = true;
            break;
          case CreateBondsAttributes::ID_maxBondsClamp:
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
            QString el1 = "*";
            QString el2 = "*";
            if (atts->GetAtomicNumber1()[i] >= 0)
                el1 = element_names[atts->GetAtomicNumber1()[i]];
            if (atts->GetAtomicNumber2()[i] >= 0)
                el2 = element_names[atts->GetAtomicNumber2()[i]];

            QListViewItem *item =
                new QListViewItem(bondsList,
                                  el1, el2,
                                  QString().sprintf("%.4f",atts->GetMinDist()[i]),
                                  QString().sprintf("%.4f",atts->GetMaxDist()[i]));
            if (old_index == i)
                new_item = item;
        }
        if (new_item)
        {
            bondsList->setSelected(new_item, true);
        }
        else
        {
            bondsList->clearSelection();
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
            Message(tr("The value for maxBonds must be between 1 and 100."));
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
}


// ****************************************************************************
//  Method:  QvisCreateBondsWindow::UpdateWindowSingleItem
//
//  Purpose:
//    Update the contents specific to the selected item in the list.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 30, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Feb 11 16:55:15 EST 2008
//    Set the enabled state of the up/down/del buttons.
//    Also, use selectedItem instead of currentItem; the behavior is
//    slightly preferable because it's possible to have a currentItem but
//    nothing selected.
//
//    Jeremy Meredith, Tue Feb 12 14:00:26 EST 2008
//    Added support for hinting selectable elements from what's in the SIL.
//
// ****************************************************************************
void QvisCreateBondsWindow::UpdateWindowSingleItem()
{
    int index = GetItemIndex(bondsList->selectedItem());

    int n = GetListLength();
    if (index < 0 || index >= n)
    {
        firstElement->setEnabled(false);
        secondElement->setEnabled(false);
        minDist->setEnabled(false);
        maxDist->setEnabled(false);
        delButton->setEnabled(false);
        upButton->setEnabled(false);
        downButton->setEnabled(false);
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

    delButton->setEnabled(index>=0 && index<=n-1);
    upButton->setEnabled(index>0 && index<=n-1);
    downButton->setEnabled(index>=0 && index<n-1);

    //
    // If we have a SIL collection called "elements", and it's not
    // fully populated (i.e. less than 100 sets in the collection),
    // then we can help the user by hinting which elements are
    // used in the current database.
    //
    std::vector<int> hints;
    avtSILRestriction_p sil = GetViewerProxy()->GetPlotSILRestriction();
    if (sil->GetNumCollections() > 0 &&
        sil->GetTopSet() != -1)
    {
        for (int i=0; i<sil->GetNumCollections(); i++)
        {
            avtSILCollection_p col = sil->GetSILCollection(i);
            if (col->GetCategory() == "element")
            {
                const std::vector<int> &sets = col->GetSubsetList();
                if (sets.size() < 100)
                {
                    for (int j = 0; j < sets.size(); j++)
                    {
                        avtSILSet_p set = sil->GetSILSet(sets[j]);
                        const string &name = set->GetName();
                        int element = ElementNameToAtomicNumber(name.c_str());
                        if (element != -1)
                            hints.push_back(element);
                    }
                }
                break;
            }
        }
    }
    firstElement->setHintedElements(hints);
    secondElement->setHintedElements(hints);
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

// ****************************************************************************
//  Method:  QvisCreateBondsWindow::firstElementChanged
//
//  Purpose:
//    callback for when the first element changed
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 30, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Feb 11 16:57:16 EST 2008
//    Use the text instead of the atomic number to support wildcards.
//
// ****************************************************************************
void QvisCreateBondsWindow::firstElementChanged(int element)
{
    QListViewItem *item = bondsList->currentItem();
    int n = GetListLength();
    int index = GetItemIndex(bondsList->currentItem());
    if (index < 0 || index >= n)
        return;

    atts->GetAtomicNumber1()[index] = element;
    item->setText(0, firstElement->text());
    Apply();
    atts->SelectAtomicNumber1();
}

// ****************************************************************************
//  Method:  QvisCreateBondsWindow::secondElementChanged
//
//  Purpose:
//    callback for when the second element changed
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 30, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Feb 11 16:57:16 EST 2008
//    Use the text instead of the atomic number to support wildcards.
//
// ****************************************************************************
void QvisCreateBondsWindow::secondElementChanged(int element)
{
    QListViewItem *item = bondsList->currentItem();
    int n = GetListLength();
    int index = GetItemIndex(bondsList->currentItem());
    if (index < 0 || index >= n)
        return;

    atts->GetAtomicNumber2()[index] = element;
    item->setText(1, secondElement->text());
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

// ****************************************************************************
//  Method:  QvisCreateBondsWindow::bondsListNew
//
//  Purpose:
//    insert a new item item in the bonds list
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 30, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Feb 11 16:57:16 EST 2008
//    Changed default to be wildcard atom types.
//
// ****************************************************************************
void QvisCreateBondsWindow::bondsListNew()
{
    atts->GetAtomicNumber1().push_back(-1);
    atts->GetAtomicNumber2().push_back(-1);
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

// ****************************************************************************
//  Method:  QvisCreateBondsWindow::bondsListDel
//
//  Purpose:
//    delete the currently selected item in the bonds list
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 30, 2006
//
// ****************************************************************************
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

// ****************************************************************************
//  Method:  SwapIndex
//
//  Purpose:
//    Swaps two entries in the CreateBondsAttributes.
//
//  Arguments:
//    atts                      the atts
//    oldindex, newindex        the indices to swap
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 11, 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Feb 12 13:58:12 EST 2008
//    Forget to select vectors after making changes.  Fixed now.
//
// ****************************************************************************
static void
SwapIndex(CreateBondsAttributes *atts, int oldindex, int newindex)
{
    int    oldAN1 = atts->GetAtomicNumber1()[oldindex];
    int    oldAN2 = atts->GetAtomicNumber2()[oldindex];
    double oldMin = atts->GetMinDist()[oldindex];
    double oldMax = atts->GetMaxDist()[oldindex];

    atts->GetAtomicNumber1()[oldindex] = atts->GetAtomicNumber1()[newindex];
    atts->GetAtomicNumber2()[oldindex] = atts->GetAtomicNumber2()[newindex];
    atts->GetMinDist()[oldindex]       = atts->GetMinDist()[newindex];
    atts->GetMaxDist()[oldindex]       = atts->GetMaxDist()[newindex];

    atts->GetAtomicNumber1()[newindex] = oldAN1;
    atts->GetAtomicNumber2()[newindex] = oldAN2;
    atts->GetMinDist()[newindex]       = oldMin;
    atts->GetMaxDist()[newindex]       = oldMax;

    atts->SelectAtomicNumber1();
    atts->SelectAtomicNumber2();
    atts->SelectMinDist();
    atts->SelectMaxDist();
}

// ****************************************************************************
//  Method:  QvisCreateBondsWindow::bondsListUp
//
//  Purpose:
//    Move the currently selected bond entry up in the list
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 11, 2008
//
// ****************************************************************************
void QvisCreateBondsWindow::bondsListUp()
{
    int n = GetListLength();
    int index = GetItemIndex(bondsList->currentItem());
    if (index < 1 || index >= n)
        return;

    int newindex = index-1;
    SwapIndex(atts, index, newindex);
    Apply();

    QListViewItem *item = bondsList->firstChild();
    for (int i=0; item != NULL; i++, item = item->nextSibling())
    {
        if (i==newindex)
        {
            bondsList->setSelected(item, true);
            break;
        }
    }
}

// ****************************************************************************
//  Method:  QvisCreateBondsWindow::bondsListDown
//
//  Purpose:
//    Move the currently selected bond entry down in the list
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 11, 2008
//
// ****************************************************************************
void QvisCreateBondsWindow::bondsListDown()
{
    int n = GetListLength();
    int index = GetItemIndex(bondsList->currentItem());
    if (index < 0 || index >= n-1)
        return;

    int newindex = index+1;
    SwapIndex(atts, index, newindex);
    Apply();

    QListViewItem *item = bondsList->firstChild();
    for (int i=0; item != NULL; i++, item = item->nextSibling())
    {
        if (i==newindex)
        {
            bondsList->setSelected(item, true);
            break;
        }
    }
}

