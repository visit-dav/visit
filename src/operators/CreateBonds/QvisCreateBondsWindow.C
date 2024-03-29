// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisCreateBondsWindow.h"

#include <CreateBondsAttributes.h>
#include <AtomicProperties.h>
#include <ViewerProxy.h>

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QPushButton>
#include <QTabWidget>
#include <QGroupBox>
#include <QvisVariableButton.h>
#include <QvisElementButton.h>
#include <ImproperUseException.h>
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
//   Cyrus Harrison, Wed Aug 20 08:27:03 PDT 2008
//   Qt4 Port.
//
//   Jeremy Meredith, Wed Jan 27 10:39:48 EST 2010
//   Added periodic bond matching support.
//
// ****************************************************************************

void
QvisCreateBondsWindow::CreateWindowContents()
{
    QTabWidget *tabWidget = new QTabWidget(central);
    topLayout->addWidget(tabWidget);

    // ------------------------------------------------------------------------

    QGroupBox *bondsTreeGroup = new QGroupBox(central);
    bondsTreeGroup->setFlat(true);
    tabWidget->addTab(bondsTreeGroup,tr("Bonding parameters"));

    QVBoxLayout *listLayout = new QVBoxLayout(bondsTreeGroup);

    QLabel *noteLabel = new QLabel(tr("Note: first match is taken, so "
                                      "order is significant"),
                                   bondsTreeGroup);
    QFont ital(noteLabel->font());
    ital.setItalic(true);
    noteLabel->setFont(ital);
    listLayout->addWidget(noteLabel);

    bondsTree = new QTreeWidget(bondsTreeGroup);

    QTreeWidgetItem *header = new QTreeWidgetItem();
    header->setText(0,tr("1st"));
    header->setText(1,tr("2nd"));
    header->setText(2,tr("Min"));
    header->setText(3,tr("Max"));
    bondsTree->setHeaderItem(header);

    bondsTree->setSelectionMode(QAbstractItemView::SingleSelection);

    bondsTree->setColumnWidth(0, 50);
    bondsTree->setColumnWidth(1, 50);

    bondsTree->setRootIsDecorated(false);
    bondsTree->header()->setSectionsClickable(false);
    bondsTree->header()->setSectionsMovable(false);
    bondsTree->setAllColumnsShowFocus(true);

    listLayout->addWidget(bondsTree);


    QHBoxLayout *listButtonLayout = new QHBoxLayout();
    listLayout->addLayout(listButtonLayout);

    newButton = new QPushButton(tr("New"), bondsTreeGroup);
    listButtonLayout->addWidget(newButton);

    delButton = new QPushButton(tr("Del"), bondsTreeGroup);
    listButtonLayout->addWidget(delButton);

    upButton = new QPushButton(tr("Up"), bondsTreeGroup);
    listButtonLayout->addWidget(upButton);

    downButton = new QPushButton(tr("Down"), bondsTreeGroup);
    listButtonLayout->addWidget(downButton);

    connect(newButton, SIGNAL(clicked()),
            this, SLOT(bondsTreeNew()));
    connect(delButton, SIGNAL(clicked()),
            this, SLOT(bondsTreeDel()));
    connect(upButton, SIGNAL(clicked()),
            this, SLOT(bondsTreeUp()));
    connect(downButton, SIGNAL(clicked()),
            this, SLOT(bondsTreeDown()));


    QGroupBox *bondsDetailsGroup = new QGroupBox(tr("Details"), bondsTreeGroup);
    listLayout->addWidget(bondsDetailsGroup);

    QVBoxLayout *bondDetailsTopLayout = new QVBoxLayout(bondsDetailsGroup);
    QHBoxLayout *bondDetailsLayout = new QHBoxLayout();
    bondDetailsTopLayout->addLayout(bondDetailsLayout);

    firstElement = new QvisElementButton(bondsDetailsGroup);
    secondElement = new QvisElementButton(bondsDetailsGroup);
    bondDetailsLayout->addWidget(new QLabel(tr("1st:"), bondsDetailsGroup));
    bondDetailsLayout->addWidget(firstElement);
    bondDetailsLayout->addWidget(new QLabel(tr("2nd:"), bondsDetailsGroup));
    bondDetailsLayout->addWidget(secondElement);

    QHBoxLayout *bondDetailsLayout2 = new QHBoxLayout();
    bondDetailsTopLayout->addLayout(bondDetailsLayout2);
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

    connect(bondsTree, SIGNAL(itemSelectionChanged()),
            this, SLOT(UpdateWindowSingleItem()));

    // ------------------------------------------------------------------------

    QGroupBox *advTab = new QGroupBox(tabWidget);
    advTab->setFlat(true);
    tabWidget->addTab(advTab, tr("Advanced settings"));

    QGridLayout *advLayout = new QGridLayout(advTab);


    elementVariableLabel = new QLabel(tr("Variable for atomic number"),
                                      advTab);
    advLayout->addWidget(elementVariableLabel,0,0);
    int elementVariableMask = QvisVariableButton::Scalars;
    elementVariable = new QvisVariableButton(true, true, true,
                                             elementVariableMask, advTab);
    connect(elementVariable, SIGNAL(activated(const QString&)),
            this, SLOT(elementVariableChanged(const QString&)));
    advLayout->addWidget(elementVariable, 0,1);

    maxBondsLabel = new QLabel(tr("Maximum bonds per atom"), advTab);
    advLayout->addWidget(maxBondsLabel,1,0);
    maxBonds = new QLineEdit(advTab);
    advLayout->addWidget(maxBonds,1,1);
    connect(maxBonds, SIGNAL(returnPressed()),
            this, SLOT(maxBondsReturnPressed()));

    addPeriodicBonds = new QGroupBox(tr("Add periodic bonds"),advTab);
    addPeriodicBonds->setCheckable(true);
    connect(addPeriodicBonds, SIGNAL(toggled(bool)),
            this, SLOT(addPeriodicBondsToggled(bool)));
    advLayout->addWidget(addPeriodicBonds, 2,0, 1,2);

    int row;
    row = 0;
    QGridLayout *vecLayout = new QGridLayout(addPeriodicBonds);

    QLabel *periodicLabel = new QLabel(tr("Periodic in"), addPeriodicBonds);
    vecLayout->addWidget(periodicLabel, row,0);
    xPeriodic = new QCheckBox(tr("X"), addPeriodicBonds);
    yPeriodic = new QCheckBox(tr("Y"), addPeriodicBonds);
    zPeriodic = new QCheckBox(tr("Z"), addPeriodicBonds);
    vecLayout->addWidget(xPeriodic, row,1);
    vecLayout->addWidget(yPeriodic, row,2);
    vecLayout->addWidget(zPeriodic, row,3);
    connect(xPeriodic, SIGNAL(toggled(bool)),
            this, SLOT(xPeriodicToggled(bool)));
    connect(yPeriodic, SIGNAL(toggled(bool)),
            this, SLOT(yPeriodicToggled(bool)));
    connect(zPeriodic, SIGNAL(toggled(bool)),
            this, SLOT(zPeriodicToggled(bool)));
    row++;

    useUnitCellVectors = new QCheckBox(tr("Use provided unit cell vectors"),
                                       addPeriodicBonds);
    connect(useUnitCellVectors, SIGNAL(toggled(bool)),
            this, SLOT(useUnitCellVectorsChanged(bool)));
    vecLayout->addWidget(useUnitCellVectors, row,0, 1,4);
    row++;

    xVectorLabel = new QLabel(tr("Vector for X"), addPeriodicBonds);
    vecLayout->addWidget(xVectorLabel,row,0);
    xVector = new QLineEdit(addPeriodicBonds);
    connect(xVector, SIGNAL(returnPressed()),
            this, SLOT(xVectorProcessText()));
    vecLayout->addWidget(xVector, row,1, 1,3);
    row++;

    yVectorLabel = new QLabel(tr("Vector for Y"), addPeriodicBonds);
    vecLayout->addWidget(yVectorLabel,row,0);
    yVector = new QLineEdit(addPeriodicBonds);
    connect(yVector, SIGNAL(returnPressed()),
            this, SLOT(yVectorProcessText()));
    vecLayout->addWidget(yVector, row,1, 1,3);
    row++;

    zVectorLabel = new QLabel(tr("Vector for Z"), addPeriodicBonds);
    vecLayout->addWidget(zVectorLabel,row,0);
    zVector = new QLineEdit(addPeriodicBonds);
    connect(zVector, SIGNAL(returnPressed()),
            this, SLOT(zVectorProcessText()));
    vecLayout->addWidget(zVector, row,1, 1,3);
    row++;

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
//    Cyrus Harrison, Wed Aug 20 08:27:03 PDT 2008
//    Qt4 Port.
//
//    Jeremy Meredith, Wed Jan 27 10:39:48 EST 2010
//    Added periodic bond matching support.
//
//    Jeremy Meredith, Wed Aug 11 10:19:31 EDT 2010
//    Disallow unknown element (atomic id==0); use "*" in that case.
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
            maxBonds->setText(QString("%1").arg(atts->GetMaxBondsClamp()));
            break;
          case CreateBondsAttributes::ID_addPeriodicBonds:
            addPeriodicBonds->blockSignals(true);
            addPeriodicBonds->setChecked(atts->GetAddPeriodicBonds());
            addPeriodicBonds->blockSignals(false);
            break;
          case CreateBondsAttributes::ID_periodicInX:
            xPeriodic->blockSignals(true);
            xPeriodic->setChecked(atts->GetPeriodicInX());
            xPeriodic->blockSignals(false);
            break;
          case CreateBondsAttributes::ID_periodicInY:
            yPeriodic->blockSignals(true);
            yPeriodic->setChecked(atts->GetPeriodicInY());
            yPeriodic->blockSignals(false);
            break;
          case CreateBondsAttributes::ID_periodicInZ:
            zPeriodic->blockSignals(true);
            zPeriodic->setChecked(atts->GetPeriodicInZ());
            zPeriodic->blockSignals(false);
            break;
          case CreateBondsAttributes::ID_useUnitCellVectors:
            useUnitCellVectors->blockSignals(true);
            useUnitCellVectors->setChecked(atts->GetUseUnitCellVectors());
            useUnitCellVectors->blockSignals(false);
            if (atts->GetUseUnitCellVectors() == false)
            {
                xVector->setEnabled(true);
                xVectorLabel->setEnabled(true);
                yVector->setEnabled(true);
                yVectorLabel->setEnabled(true);
                zVector->setEnabled(true);
                zVectorLabel->setEnabled(true);
            }
            else
            {
                xVector->setEnabled(false);
                xVectorLabel->setEnabled(false);
                yVector->setEnabled(false);
                yVectorLabel->setEnabled(false);
                zVector->setEnabled(false);
                zVectorLabel->setEnabled(false);
            }
            break;
          case CreateBondsAttributes::ID_xVector:
            xVector->setText(DoublesToQString(atts->GetXVector(), 3));
            break;
          case CreateBondsAttributes::ID_yVector:
            yVector->setText(DoublesToQString(atts->GetYVector(), 3));
            break;
          case CreateBondsAttributes::ID_zVector:
            zVector->setText(DoublesToQString(atts->GetZVector(), 3));
            break;
        default:
            break;
        }
    }

    if (update_bonds_list)
    {
        int old_index = bondsTree->indexOfTopLevelItem(bondsTree->currentItem());
        QTreeWidgetItem *new_item = NULL;
        bondsTree->clear();
        int n = GetListLength();
        for (int i=0; i<n; i++)
        {
            QString el1 = "*";
            QString el2 = "*";
            if (atts->GetAtomicNumber1()[i] >= 1)
                el1 = element_names[atts->GetAtomicNumber1()[i]];
            if (atts->GetAtomicNumber2()[i] >= 1)
                el2 = element_names[atts->GetAtomicNumber2()[i]];

            QTreeWidgetItem *item = new QTreeWidgetItem(bondsTree);
            item->setText(0,el1);
            item->setText(1,el2);
            item->setText(2,QString("%1").arg(atts->GetMinDist()[i],0,'f',4));
            item->setText(3,QString("%1").arg(atts->GetMaxDist()[i],0,'f',4));
            if (old_index == i)
                new_item = item;
        }
        if (new_item)
        {
            bondsTree->setCurrentItem(new_item);
        }
        else
        {
            bondsTree->clearSelection();
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
//   Kathleen Bonnell, Tue Jul 1 11:39:22 PDT 2008
//   Removed unreferenced variables.
//
//   Cyrus Harrison, Wed Aug 20 08:27:03 PDT 2008
//   Qt4 Port.
//
//   Jeremy Meredith, Wed Jan 27 10:39:48 EST 2010
//   Added periodic bond matching support.
//
// ****************************************************************************

void
QvisCreateBondsWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do maxBondsClamp
    if(which_widget == CreateBondsAttributes::ID_maxBondsClamp || doAll)
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

    // Do xVector
    if(which_widget == CreateBondsAttributes::ID_xVector || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(xVector, val, 3))
            atts->SetXVector(val);
        else
        {
            ResettingError(tr("Vector for X"),
                DoublesToQString(atts->GetXVector(),3));
            atts->SetXVector(atts->GetXVector());
        }
    }

    // Do yVector
    if(which_widget == CreateBondsAttributes::ID_yVector || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(yVector, val, 3))
            atts->SetYVector(val);
        else
        {
            ResettingError(tr("Vector for Y"),
                DoublesToQString(atts->GetYVector(),3));
            atts->SetYVector(atts->GetYVector());
        }
    }

    // Do zVector
    if(which_widget == CreateBondsAttributes::ID_zVector || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(zVector, val, 3))
            atts->SetZVector(val);
        else
        {
            ResettingError(tr("Vector for Z"),
                DoublesToQString(atts->GetZVector(),3));
            atts->SetZVector(atts->GetZVector());
        }
    }

}


//
// Qt Slot functions
//


void
QvisCreateBondsWindow::elementVariableChanged(const QString &varName)
{
    atts->SetElementVariable(varName.toStdString());
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
//    Cyrus Harrison, Wed Aug 20 08:27:03 PDT 2008
//    Qt4 Port.
//
//    Hank Childs, Mon Dec 14 16:43:16 PST 2009
//    Support new SIL interface.
//
// ****************************************************************************
void QvisCreateBondsWindow::UpdateWindowSingleItem()
{
    int index = bondsTree->indexOfTopLevelItem(bondsTree->currentItem());

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
    minDist->setText(QString("%1").arg(atts->GetMinDist()[index],0,'f',4));
    maxDist->setText(QString("%1").arg(atts->GetMaxDist()[index],0,'f',4));

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
                int nSets = col->GetNumberOfSubsets();
                if (nSets < 100)
                {
                    for (int j = 0; j < nSets; j++)
                    {
                        avtSILSet_p set = sil->GetSILSet(col->GetSubset(j));
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
    QTreeWidgetItem *item = bondsTree->currentItem();
    int n = GetListLength();
    int index = bondsTree->indexOfTopLevelItem(item);
    if (index < 0 || index >= n)
        return;

    atts->GetMinDist()[index] = minDist->displayText().toFloat();
    item->setText(2, QString("%1").arg(atts->GetMinDist()[index],0,'f',4));
    atts->SelectMinDist();
}

void QvisCreateBondsWindow::maxDistTextChanged(const QString &txt)
{
    QTreeWidgetItem *item = bondsTree->currentItem();
    int n = GetListLength();
    int index = bondsTree->indexOfTopLevelItem(item);
    if (index < 0 || index >= n)
        return;

    atts->GetMaxDist()[index] = maxDist->displayText().toFloat();
    item->setText(3, QString("%1").arg(atts->GetMaxDist()[index],0,'f',4));
    atts->SelectMaxDist();
}



void
QvisCreateBondsWindow::addPeriodicBondsToggled(bool val)
{
    atts->SetAddPeriodicBonds(val);
    Apply();
}


void
QvisCreateBondsWindow::useUnitCellVectorsChanged(bool val)
{
    atts->SetUseUnitCellVectors(val);
    Apply();
}


void
QvisCreateBondsWindow::xPeriodicToggled(bool val)
{
    atts->SetPeriodicInX(val);
    Apply();
}

void
QvisCreateBondsWindow::yPeriodicToggled(bool val)
{
    atts->SetPeriodicInY(val);
    Apply();
}

void
QvisCreateBondsWindow::zPeriodicToggled(bool val)
{
    atts->SetPeriodicInZ(val);
    Apply();
}


void
QvisCreateBondsWindow::xVectorProcessText()
{
    GetCurrentValues(CreateBondsAttributes::ID_xVector);
    Apply();
}


void
QvisCreateBondsWindow::yVectorProcessText()
{
    GetCurrentValues(CreateBondsAttributes::ID_yVector);
    Apply();
}


void
QvisCreateBondsWindow::zVectorProcessText()
{
    GetCurrentValues(CreateBondsAttributes::ID_zVector);
    Apply();
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
//    Cyrus Harrison, Wed Aug 20 08:27:03 PDT 2008
//    Qt4 Port.
//
// ****************************************************************************
void QvisCreateBondsWindow::firstElementChanged(int element)
{
    QTreeWidgetItem *item = bondsTree->currentItem();
    int n = GetListLength();
    int index = bondsTree->indexOfTopLevelItem(bondsTree->currentItem());
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
//    Cyrus Harrison, Wed Aug 20 08:27:03 PDT 2008
//    Qt4 Port.
//
// ****************************************************************************
void QvisCreateBondsWindow::secondElementChanged(int element)
{
    QTreeWidgetItem *item = bondsTree->currentItem();
    int n = GetListLength();
    int index = bondsTree->indexOfTopLevelItem(bondsTree->currentItem());
    if (index < 0 || index >= n)
        return;

    atts->GetAtomicNumber2()[index] = element;
    item->setText(1, secondElement->text());
    Apply();
    atts->SelectAtomicNumber2();
}

int QvisCreateBondsWindow::GetListLength()
{
    size_t n1 = atts->GetAtomicNumber1().size();
    size_t n2 = atts->GetAtomicNumber2().size();
    size_t n3 = atts->GetMinDist().size();
    size_t n4 = atts->GetMaxDist().size();
    if (n1 != n2 || n1 != n3 || n1 != n4)
    {
        EXCEPTION1(ImproperUseException,
                   "Bond list data arrays were not all the same length.");
    }
    return (int)n1;
}

// ****************************************************************************
//  Method:  QvisCreateBondsWindow::bondsTreeNew
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
//    Cyrus Harrison, Wed Aug 20 08:27:03 PDT 2008
//    Qt4 Port.
//
// ****************************************************************************
void QvisCreateBondsWindow::bondsTreeNew()
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
    int newindex = bondsTree->topLevelItemCount() -1;
    bondsTree->setCurrentItem(bondsTree->topLevelItem(newindex));
}

// ****************************************************************************
//  Method:  QvisCreateBondsWindow::bondsTreeDel
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
//  Modifications:
//    Cyrus Harrison, Wed Aug 20 08:27:03 PDT 2008
//    Qt4 Port.
//
// ****************************************************************************
void QvisCreateBondsWindow::bondsTreeDel()
{
    int n = GetListLength();
    int index = bondsTree->indexOfTopLevelItem(bondsTree->currentItem());
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

    // if we deleted the last one, we want to select the new last element.
    if(n-1 > 0 && index == n-1)
        bondsTree->setCurrentItem(bondsTree->topLevelItem(index-1));
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
//  Method:  QvisCreateBondsWindow::bondsTreeUp
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
//  Modifications:
//    Cyrus Harrison, Wed Aug 20 08:27:03 PDT 2008
//    Qt4 Port.
//
// ****************************************************************************
void QvisCreateBondsWindow::bondsTreeUp()
{
    int n = GetListLength();
    int index = bondsTree->indexOfTopLevelItem(bondsTree->currentItem());
    if (index < 1 || index >= n)
        return;

    int newindex = index-1;
    SwapIndex(atts, index, newindex);
    Apply();

    bondsTree->setCurrentItem(bondsTree->topLevelItem(newindex));
}

// ****************************************************************************
//  Method:  QvisCreateBondsWindow::bondsTreeDown
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
//  Modifications:
//    Cyrus Harrison, Wed Aug 20 08:27:03 PDT 2008
//    Qt4 Port.
//
// ****************************************************************************
void QvisCreateBondsWindow::bondsTreeDown()
{
    int n = GetListLength();
    int index = bondsTree->indexOfTopLevelItem(bondsTree->currentItem());
    if (index < 0 || index >= n-1)
        return;

    int newindex = index+1;
    SwapIndex(atts, index, newindex);
    Apply();

    bondsTree->setCurrentItem(bondsTree->topLevelItem(newindex));
}

