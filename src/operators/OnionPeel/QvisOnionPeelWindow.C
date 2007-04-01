#include "QvisOnionPeelWindow.h"

#include <OnionPeelAttributes.h>
#include <ViewerProxy.h>
#include <avtSIL.h>
#include <SILRestrictionAttributes.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qspinbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <stdio.h>
#include <string>


using std::string;
using std::vector;

// ****************************************************************************
// Method: QvisOnionPeelWindow::QvisOnionPeelWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Thu Aug 8 14:29:46 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Thu Feb 26 13:19:40 PST 2004
//   Added observation of SILRestrictionAttributes, data member silUseSet.
//   
// ****************************************************************************

QvisOnionPeelWindow::QvisOnionPeelWindow(const int type,
                         OnionPeelAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad) ,silUseSet()
{
    atts = subj;
    defaultItem = "Whole";
    silTopSet = -1;
    silNumSets = -1;
    silNumCollections = -1;
    silAtts = viewer->GetSILRestrictionAttributes();
    silAtts->Attach(this);
}


// ****************************************************************************
// Method: QvisOnionPeelWindow::~QvisOnionPeelWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Thu Aug 8 14:29:46 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisOnionPeelWindow::~QvisOnionPeelWindow()
{
}


// ****************************************************************************
// Method: QvisOnionPeelWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Thu Aug 8 14:29:46 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 10 14:28:14 PST 2004
//   Added useGlobalId checkbox.
//   
//   Kathleen Bonnell, Tue Jan 18 19:37:46 PST 2005 
//   Changed maximum for requestedLayer spin box. 
//   
//   Kathleen Bonnell, Wed Jan 19 15:45:38 PST 2005 
//   Added 'seedType' button group. 
//
// ****************************************************************************

void
QvisOnionPeelWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 7,3,  10, "mainLayout");


    //
    // Adjacency
    //
    mainLayout->addWidget(new QLabel("Adjacency", central, "adjacencyTypeLabel"),0,0);
    adjacencyType = new QButtonGroup(central, "adjacencyType");
    adjacencyType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *adjacencyTypeLayout = new QHBoxLayout(adjacencyType);
    adjacencyTypeLayout->setSpacing(10);
    QRadioButton *adjacencyTypeNodeFaceNode = new QRadioButton("Node", adjacencyType);
    adjacencyTypeLayout->addWidget(adjacencyTypeNodeFaceNode);
    QRadioButton *adjacencyTypeNodeFaceFace = new QRadioButton("Face", adjacencyType);
    adjacencyTypeLayout->addWidget(adjacencyTypeNodeFaceFace);
    connect(adjacencyType, SIGNAL(clicked(int)),
            this, SLOT(adjacencyTypeChanged(int)));
    mainLayout->addWidget(adjacencyType, 0,1);

    //
    // Category
    //
    categoryLabel = new QLabel("Category", central, "categoryNameLabel");
    mainLayout->addWidget(categoryLabel,1,0);

    categoryName = new QComboBox(true, central, "categoryName");
    categoryName->setAutoCompletion(true);
    categoryName->setInsertionPolicy(QComboBox::NoInsertion);
    categoryName->insertItem(defaultItem);
    categoryName->setCurrentItem(0);
    categoryName->setEditText(defaultItem);
    connect(categoryName, SIGNAL(activated(int)),
            this, SLOT(categoryNameChanged()));
    mainLayout->addMultiCellWidget(categoryName, 1,1, 1,2);

    //
    // Set
    //
    subsetLabel = new QLabel("Set", central, "subsetNameLabel");
    mainLayout->addWidget(subsetLabel,2,0);
    subsetName = new QComboBox(true, central, "subsetName");
    subsetName->setAutoCompletion(true);
    subsetName->setInsertionPolicy(QComboBox::NoInsertion);
    subsetName->insertItem(defaultItem);
    subsetName->setCurrentItem(0);
    subsetName->setEditText(defaultItem);
    connect(subsetName, SIGNAL(activated(int)),
            this, SLOT(subsetNameChanged()));
    mainLayout->addMultiCellWidget(subsetName, 2,2, 1,2);

    //
    // Seed
    //
    mainLayout->addWidget(new QLabel("Seed", central, "seedTypeLabel"),3,0);
    seedType = new QButtonGroup(central, "seedType");
    seedType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *seedTypeLayout = new QHBoxLayout(seedType);
    seedTypeLayout->setSpacing(10);
    QRadioButton *rb = new QRadioButton("Cell", seedType);
    seedTypeLayout->addWidget(rb);
    rb = new QRadioButton("Node", seedType);
    seedTypeLayout->addWidget(rb);
    connect(seedType, SIGNAL(clicked(int)),
            this, SLOT(seedTypeChanged(int)));
    mainLayout->addWidget(seedType, 3,1);
    
    //
    // Index
    //
    mainLayout->addMultiCellWidget(new QLabel("Seed # or i j [k]", 
                central, "indexLabel"),4,4,0,1);
    index = new QLineEdit(central, "index");
    index->setText(QString("1"));
    connect(index, SIGNAL(returnPressed()),
            this, SLOT(indexChanged()));
    mainLayout->addWidget(index, 4,2);

    //
    // UseGlobalId
    //
    useGlobalId = new QCheckBox("Seed # is Global", central, "useGlobalId");
    useGlobalId->setChecked(false);
    connect(useGlobalId, SIGNAL(toggled(bool)),
            this, SLOT(useGlobalIdToggled(bool)));
    mainLayout->addMultiCellWidget(useGlobalId, 5,5, 0,2);

    //
    // Layers
    //
    mainLayout->addWidget(new QLabel("Layers", central, "requestedLayerLabel"),6,0);
    requestedLayer = new QSpinBox(0, 10000, 1, central, "requestedLayer");
    connect(requestedLayer, SIGNAL(valueChanged(int)), 
            this, SLOT(requestedLayerChanged(int)));
    mainLayout->addWidget(requestedLayer, 6,1);
}


// ****************************************************************************
// Method: QvisOnionPeelWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Thu Aug 8 14:29:46 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Thu Feb 26 13:19:40 PST 2004
//   Only update the ComboBoxes if the SILRestrictionAttributes have changed.
//   
//   Kathleen Bonnell, Tue May  4 17:47:50 PDT 2004 
//   Also update the ComboBoxes if doAll is true. 
//    
//   Kathleen Bonnell, Fri Dec 10 14:28:14 PST 2004
//   Added useGlobalId checkbox.
//   
//   Kathleen Bonnell, Wed Jan 19 15:45:38 PST 2005 
//   Added 'seedType' button group. 
//
// ****************************************************************************

void
QvisOnionPeelWindow::UpdateWindow(bool doAll)
{
    QString temp;
    int i, j;
    std::vector<int> ivec;

    if (selectedSubject == silAtts)
    {
        UpdateComboBoxes();
        return;
    }

    if (doAll)
        UpdateComboBoxes();

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case 0: // Adjacency 
            adjacencyType->blockSignals(true);
            adjacencyType->setButton(atts->GetAdjacencyType());
            adjacencyType->blockSignals(false);
            break;
        case 1: //useGlobalId 
            useGlobalId->blockSignals(true);
            useGlobalId->setChecked(atts->GetUseGlobalId());
            if (atts->GetUseGlobalId())
            {
                categoryName->setEnabled(false);
                categoryLabel->setEnabled(false);
                subsetName->setEnabled(false);
                subsetLabel->setEnabled(false);
            }
            useGlobalId->blockSignals(false);
            break;
        case 2: //Category 
            break;
        case 3: // Subset 
            break;
        case 4: // index 
            index->blockSignals(true);
            ivec = atts->GetIndex(); 
            char buff[80];
            temp = "";
            for (j = 0; j < ivec.size(); j++) 
            {
                sprintf(buff, "%d ", ivec[j]);
                temp += buff;
            }
            index->setText(temp); 
            index->blockSignals(false);
            break;
        case 5: // Logical 
            break;
        case 6: // Layers 
            requestedLayer->blockSignals(true);
            requestedLayer->setValue(atts->GetRequestedLayer());
            requestedLayer->blockSignals(false);
            break;
        case 7: // SeedType 
            seedType->blockSignals(true);
            seedType->setButton(atts->GetSeedType());
            seedType->blockSignals(false);
            break;
        }
    } // end for
    if (!doAll)
        UpdateComboBoxesEnabledState(2);
}

// ****************************************************************************
// Method: QvisOnionPeelWindow::UpdateComboBoxes
//
// Purpose: 
//   Determnes which combo box needs to be updated. 
//
// Programmer: Kathleen Bonnell 
//
// Modifications:
//   Kathleen Bonnell, Thu Feb 26 13:19:40 PST 2004
//   Only update the box whose information has changed. 
//   
// ****************************************************************************

void
QvisOnionPeelWindow::UpdateComboBoxes()
{
    QString cn = categoryName->currentText();

    if (silAtts->GetTopSet() != silTopSet ||
        silAtts->GetSilAtts().GetNCollections() != silNumCollections)
    {
        silTopSet = silAtts->GetTopSet();
        silNumCollections = silAtts->GetSilAtts().GetNCollections();
        FillCategoryBox();
    }

    if (cn != categoryName->currentText() || silUseSet != silAtts->GetUseSet())
    {
        silUseSet = silAtts->GetUseSet();
        FillSubsetBox();
    }
}

// ****************************************************************************
// Method: QvisOnionPeelWindow::UpdateComboBoxesEnabledState
//
// Purpose: 
//   Updates the enabled state for category and subset combo boxes. 
//
// Arguments:
//   which     Which combo box should be updated. 0 == Category
//             1 == Subset, 2 == Both.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 10, 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisOnionPeelWindow::UpdateComboBoxesEnabledState(int which)
{
    bool doAll = which == 2;
    if (which == 0 || doAll)
    {
        if (categoryName->count() == 0)
        {
            categoryName->insertItem(defaultItem);
            categoryName->setCurrentItem(0);
            categoryName->setEnabled(false);
            categoryLabel->setEnabled(false);
        }
        else if (categoryName->count() == 1)
        {
            categoryName->setEnabled(false);
            categoryLabel->setEnabled(false);
        }
        else if (atts->GetUseGlobalId())
        {
            categoryName->setEnabled(false);
            categoryLabel->setEnabled(false);
        }
        else 
        {
            categoryName->setEnabled(true);
            categoryLabel->setEnabled(true);
        }
    }
    if (which == 1 || doAll)
    {
        if (subsetName->count() == 0)
        {
            subsetName->insertItem(defaultItem);
            subsetName->setCurrentItem(0);
            subsetName->setEnabled(false);
            subsetLabel->setEnabled(false);
        }
        else if (subsetName->count() == 1)
        {
            subsetName->setEnabled(false);
            subsetLabel->setEnabled(false);
        }
        else if (atts->GetUseGlobalId())
        {
            subsetName->setEnabled(false);
            subsetLabel->setEnabled(false);
        }
        else
        {
            subsetName->setEnabled(true);
            subsetLabel->setEnabled(true);
        }
        categoryName->blockSignals(false);
        subsetName->blockSignals(false);
    }
}


// ****************************************************************************
// Method: QvisOnionPeelWindow::FillCategoryBox
//
// Purpose: 
//   Reads the current SILRestriction and updates the category list. 
//
// Programmer: Kathleen Bonnell 
//
// Modifications:
//   Kathleen Bonnell, Thu Feb 26 13:19:40 PST 2004
//   Only update the box whose information has changed. 
//   
//   Kathleen Bonnell, Thu Sep  2 15:54:03 PDT 2004 
//   Disable if zero or 1 names.
//   
// ****************************************************************************

void
QvisOnionPeelWindow::FillCategoryBox()
{
    categoryName->blockSignals(true);
    categoryName->clear();

    if (silTopSet > -1)
    {
        avtSILRestriction_p restriction = viewer->GetPlotSILRestriction();
        avtSILSet_p current = restriction->GetSILSet(silTopSet);
        const std::vector<int> &mapsOut = current->GetMapsOut();
        for (int j = 0; j < mapsOut.size(); ++j)
        {
            int cIndex = mapsOut[j];
            avtSILCollection_p collection =restriction->GetSILCollection(cIndex);
            QString collectionName(collection->GetCategory().c_str());
            if ((collection->GetRole() == SIL_DOMAIN) ||
                (collection->GetRole() == SIL_BLOCK)) 
            {
                categoryName->insertItem(collectionName);    
            }
        }

        if (categoryName->count() != 0)
        {
            //
            // Set the current item for the category
            //
            QString cn(atts->GetCategoryName().c_str());
 
            if (cn == defaultItem)
            {
                categoryName->setCurrentItem(0);
            }
            else 
            {
                QListBox *lb = categoryName->listBox();
                int idx = lb->index(lb->findItem(cn));
                idx = (idx == -1 ? 0 : idx);
                categoryName->setCurrentItem(idx);
            }
        } /* if category has items in it */
    }

    UpdateComboBoxesEnabledState(0);
    categoryName->setEditText(categoryName->currentText());
    categoryName->blockSignals(false);
}


// ****************************************************************************
// Method: QvisOnionPeelWindow::FillSubsetBox
//
// Purpose: 
//   Reads the current SILRestriction and updates the subset list. 
//
// Programmer: Kathleen Bonnell 
//
// Modifications:
//   Kathleen Bonnell, Thu Feb 26 13:19:40 PST 2004
//   Check the silUseSet for names to include. 
//   
//   Kathleen Bonnell, Thu Sep  2 15:54:03 PDT 2004 
//   Disable if zero or 1 names.
//   
// ****************************************************************************

void
QvisOnionPeelWindow::FillSubsetBox()
{
    subsetName->blockSignals(true);
    subsetName->clear();

    QString cn = categoryName->currentText();

    if (cn != defaultItem) 
    {
        avtSILRestriction_p restriction = viewer->GetPlotSILRestriction();

        //
        // Fill  with sets under the currently selected category.
        //
        int colIndex = restriction->GetCollectionIndex(cn.latin1());
        avtSILCollection_p collection =restriction->GetSILCollection(colIndex); 
        if (*collection != NULL)
        {
            std::vector<int> sets = collection->GetSubsetList();
            for (int i = 0; i < sets.size(); ++i)
            {
                if (silUseSet[sets[i]] != 0)
                {
                    avtSILSet_p set = restriction->GetSILSet(sets[i]);
                    subsetName->insertItem(QString(set->GetName().c_str()));
                } 
            } 
            //
            // Set the current item for the subset 
            //
            if (subsetName->count() != 0)
            {
                QString sn(atts->GetSubsetName().c_str());
                if (sn == defaultItem) 
                {
                    subsetName->setCurrentItem(0);
                }
                else 
                {
                    QListBox *lb = subsetName->listBox();
                    int idx = lb->index(lb->findItem(sn));
                    idx = (idx == -1 ? 0 : idx);
                    subsetName->setCurrentItem(idx);
                }
            }
        } /*if collection!=NULL */
    }
    UpdateComboBoxesEnabledState(1);
    subsetName->setEditText(subsetName->currentText());
    subsetName->blockSignals(false);
}

// ****************************************************************************
// Method: QvisOnionPeelWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Thu Aug 8 14:29:46 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Tue Jan 18 19:37:46 PST 2005
//   Added logic for requestedLayer.
//   
// ****************************************************************************

void
QvisOnionPeelWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do adjacencyType
    if(which_widget == 0 || doAll)
    {
        // Nothing for adjacencyType
    }

    // Do categoryName
    if(which_widget == 1 || doAll)
    {
        temp = categoryName->currentText();
        okay = !temp.isEmpty();
        if(okay)
        {
            atts->SetCategoryName(temp.latin1());
        }

        if(!okay)
        {
            msg.sprintf("The value of categoryName was invalid. "
                "Resetting to the last good value of %s.",
                atts->GetCategoryName().c_str());
            Message(msg);
            atts->SetCategoryName(atts->GetCategoryName());
        }
    }

    // Do subsetName
    if(which_widget == 2 || doAll)
    {
        temp = subsetName->currentText();
        okay = !temp.isEmpty();
        if(okay)
        {
            atts->SetSubsetName(temp.latin1());
        }

        if(!okay)
        {
            msg.sprintf("The value of subsetName was invalid. "
                "Resetting to the last good value of %s.",
                atts->GetSubsetName().c_str());
            Message(msg);
            atts->SetSubsetName(atts->GetSubsetName());
        }
    }

    // Do index
    if(which_widget == 3 || doAll)
    {
        intVector ivec;
        temp = index->displayText().simplifyWhiteSpace();
        QStringList lst(QStringList::split(QString(" "), temp));
 
        QStringList::Iterator it;
        int val;
        for (it = lst.begin(); it != lst.end(); ++it)
        { 
            sscanf((*it).latin1(), "%d", &val);
            ivec.push_back(val);
        }
        okay = ((ivec.size() > 0) && (ivec.size() <= 3));
        if (okay)
        {
            atts->SetLogical(ivec.size() != 1 && !atts->GetUseGlobalId());
            atts->SetIndex(ivec);
        } 
        else 
        {
            msg.sprintf("The values for the index were invalid. "
                "Resetting to the last good value."); 
            Message(msg);
            atts->SetLogical(atts->GetLogical());
            atts->SetIndex(atts->GetIndex());
        }
    }

    // Do requestedLayer
    if(which_widget == 4 || doAll)
    {
        if (atts->GetRequestedLayer() != requestedLayer->value())
            atts->SetRequestedLayer(requestedLayer->value());
    }

}


//
// Qt Slot functions
//

// ****************************************************************************
// Method: QvisOnionPeelWindow::delayedApply
//
// Purpose: 
//   This is a Qt slot function that delays the Apply when autoupdate is used.
//   We do this because there is an internal timer in the spin boxes that
//   keeps adding signals for the spin boxes when we press the arrows. This
//   causes problems when the plot has no OnionPeel operator and we want to
//   add one with the "Add operator" dialog. Once the operator is added, the
//   pent up signals are processed causing the operator window to go nuts. By
//   having this slot called on a delayed timer, we avoid the problem.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 26 17:41:47 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisOnionPeelWindow::delayedApply()
{
    Apply(true);
}

void
QvisOnionPeelWindow::adjacencyTypeChanged(int val)
{
    if(val != atts->GetAdjacencyType())
    {
        atts->SetAdjacencyType(OnionPeelAttributes::NodeFace(val));
        if (AutoUpdate())
            QTimer::singleShot(100, this, SLOT(delayedApply()));
        else
            Apply();
    }
}

void
QvisOnionPeelWindow::seedTypeChanged(int val)
{
    if(val != atts->GetSeedType())
    {
        atts->SetSeedType(OnionPeelAttributes::SeedIdType(val));
        if (AutoUpdate())
            QTimer::singleShot(100, this, SLOT(delayedApply()));
        else
            Apply();
    }
}


void
QvisOnionPeelWindow::categoryNameChanged()
{
    if (strcmp(categoryName->currentText().latin1(),
               atts->GetCategoryName().c_str()) != 0)
    {
        FillSubsetBox();
    }
    GetCurrentValues(1);
    if (AutoUpdate())
        QTimer::singleShot(100, this, SLOT(delayedApply()));
    else
        Apply();
}


void
QvisOnionPeelWindow::subsetNameChanged()
{
    GetCurrentValues(2);
    if (AutoUpdate())
        QTimer::singleShot(100, this, SLOT(delayedApply()));
    else
        Apply();
}

void
QvisOnionPeelWindow::indexChanged()
{
    GetCurrentValues(3);
    if (AutoUpdate())
        QTimer::singleShot(100, this, SLOT(delayedApply()));
    else
        Apply();
}


// ****************************************************************************
// Modifications:
//   Kathleen Bonnell, Tue Jan 18 19:37:46 PST 2005
//   Added call to GetCurrentValues. 
//   
// ****************************************************************************
void
QvisOnionPeelWindow::requestedLayerChanged(int val)
{
    GetCurrentValues(4);
    if (AutoUpdate())
        QTimer::singleShot(100, this, SLOT(delayedApply()));
    else
        Apply();
}


void
QvisOnionPeelWindow::useGlobalIdToggled(bool val)
{
    if(val != atts->GetUseGlobalId())
    {
        atts->SetUseGlobalId(val);
        UpdateComboBoxesEnabledState(2);
        if (AutoUpdate())
            QTimer::singleShot(100, this, SLOT(delayedApply()));
        else
            Apply();
    }
}

