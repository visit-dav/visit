/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include "QvisSILSetSelector.h"

#include <ViewerProxy.h>
#include <avtSIL.h>
#include <SILRestrictionAttributes.h>

#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <stdio.h>
#include <string>
#include <algorithm>


using std::string;

// ****************************************************************************
// Method: QvisSILSetSelector::QvisSILSetSelector
//
// Purpose:    Constructor
//
// Programmer: Kathleen Bonnell 
// Creation:   June 6, 2007 
//
// Modifications:
//   Kathleen Bonnell, Thu Jun 14 12:13:03 PDT 2007
//   Added userCategory and userSubset.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Thu Jul 17 16:41:18 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisSILSetSelector::QvisSILSetSelector(QWidget *parent, 
        SILRestrictionAttributes *silra, intVector &r) :
    QWidget(parent), SimpleObserver(), GUIBase(), silUseSet()
{
    defaultItem = "Whole";
    lastGoodCategory = defaultItem;
    lastGoodSubset = defaultItem;
    userCategory = defaultItem;
    userSubset = defaultItem;
    silTopSet = -1;
    silNumSets = -1;
    silNumCollections = -1;
    silAtts = silra;
    silAtts->Attach(this);
    allowedCategories = r;

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setSpacing(5);
    mainLayout->setMargin(0);

    //
    // Category
    //
    categoryLabel = new QLabel(tr("Category"), this);
    mainLayout->addWidget(categoryLabel,0,0);

    categoryName = new QComboBox(this);
    categoryName->setAutoCompletion(true);
    categoryName->addItem(defaultItem);
    categoryName->setCurrentIndex(0);
    categoryName->setEditText(defaultItem);
    connect(categoryName, SIGNAL(activated(int)),
            this, SLOT(categoryNameChanged()));
    mainLayout->addWidget(categoryName, 0,1);

    //
    // Set
    //
    subsetLabel = new QLabel(tr("Set"), this);
    mainLayout->addWidget(subsetLabel,1,0);
    subsetName = new QComboBox(this);
    subsetName->setAutoCompletion(true);
    subsetName->addItem(defaultItem);
    subsetName->setCurrentIndex(0);
    connect(subsetName, SIGNAL(activated(int)),
            this, SLOT(subsetNameChanged()));
    mainLayout->addWidget(subsetName, 1,1);
    mainLayout->setColumnStretch(1, 10);

    UpdateComboBoxes();
}


// ****************************************************************************
// Method: QvisSILSetSelector::~QvisSILSetSelector
//
// Purpose:    Destructor
//
// Programmer: Kathleen Bonnell
// Creation:   June 6, 2007 
//
// Modifications:
//   
// ****************************************************************************

QvisSILSetSelector::~QvisSILSetSelector()
{
    if (silAtts)
        silAtts->Detach(this);
}


// ****************************************************************************
// Method: QvisSILSetSelector::Update
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Kathleen Bonnell
// Creation:   June 6, 2007 
//
// Modifications:
//   
// ****************************************************************************

void
QvisSILSetSelector::Update(Subject *TheChangedSubject)
{
    if (silAtts == 0)
        return;

    if (TheChangedSubject == silAtts)
        UpdateComboBoxes();
}


// ****************************************************************************
// Method: QvisSILSetSelector::SubjectRemoved
//
// Purpose: 
//   Detaches from the atts when they are going to be removed. 
//
// Programmer: Kathleen Bonnell
// Creation:   June 6, 2007 
//
// Modifications:
//   
// ****************************************************************************

void
QvisSILSetSelector::SubjectRemoved(Subject *TheRemovedSubject)
{
    if (TheRemovedSubject == silAtts)
        silAtts = 0; 
}


// ****************************************************************************
// Method: QvisSILSetSelector::UpdateComboBoxes
//
// Purpose: 
//   Determnes which combo box needs to be updated. 
//
// Programmer: Kathleen Bonnell 
// Creation:   June 6, 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisSILSetSelector::UpdateComboBoxes()
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
// Method: QvisSILSetSelector::FillCategoryBox
//
// Purpose: 
//   Reads the current SILRestriction and updates the category list. 
//
// Programmer: Kathleen Bonnell 
// Creation:   June 6, 2007
//
// Modifications:
//   Kathleen Bonnell, Thu Jun 14 12:13:03 PDT 2007
//   Added userCategory.
//
//   Brad Whitlock, Fri Jul 18 08:36:21 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSILSetSelector::FillCategoryBox()
{
    categoryName->blockSignals(true);
    categoryName->clear();

    if (silTopSet > -1)
    {
        avtSILRestriction_p restriction = GetViewerProxy()->GetPlotSILRestriction();
        avtSILSet_p current = restriction->GetSILSet(silTopSet);
        const intVector &mapsOut = current->GetMapsOut();
        for (int j = 0; j < mapsOut.size(); ++j)
        {
            int cIndex = mapsOut[j];
            avtSILCollection_p collection =restriction->GetSILCollection(cIndex);
            QString collectionName(collection->GetCategory().c_str());
            if (std::find(allowedCategories.begin(), allowedCategories.end(),
                        (int)collection->GetRole()) != allowedCategories.end())
            {
                categoryName->addItem(collectionName);    
            }
        }

        if (categoryName->count() != 0)
        {
            //
            // Set the current item for the category
            //
            if (userCategory == defaultItem && lastGoodCategory == defaultItem)
            {
                categoryName->setCurrentIndex(0);
            }
            else 
            {
                int idx = categoryName->findText(userCategory, Qt::MatchExactly);
                if (idx == -1)
                    idx = categoryName->findText(lastGoodCategory, Qt::MatchExactly);
                idx = (idx == -1 ? 0 : idx);
                categoryName->setCurrentIndex(idx);
            }
        } /* if category has items in it */
    }

    if (categoryName->count() == 0)
    {
        categoryName->addItem(defaultItem);
        categoryName->setCurrentIndex(0);
        categoryName->setEnabled(false);
        categoryLabel->setEnabled(false);
    }
    else if (categoryName->count() == 1)
    {
        categoryName->setEnabled(false);
        categoryLabel->setEnabled(false);
    }
    else 
    {
        categoryName->setEnabled(true);
        categoryLabel->setEnabled(true);
    }
    categoryName->blockSignals(false);
}


// ****************************************************************************
// Method: QvisSILSetSelector::FillSubsetBox
//
// Purpose: 
//   Reads the current SILRestriction and updates the subset list. 
//
// Programmer: Kathleen Bonnell 
// Creation:   June 6, 2007 
//
// Modifications:
//   Kathleen Bonnell, Thu Jun 14 12:13:03 PDT 2007
//   Added userSubset.
//
//   Brad Whitlock, Fri Jul 18 08:43:42 PDT 2008
//   Qt 4.
//
//   Hank Childs, Fri Dec 11 11:37:48 PST 2009
//   Adapt to changes in the SIL interface.
//
// ****************************************************************************

void
QvisSILSetSelector::FillSubsetBox()
{
    subsetName->blockSignals(true);
    subsetName->clear();

    QString cn = categoryName->currentText();

    if (cn != defaultItem)
    {
        avtSILRestriction_p restriction = GetViewerProxy()->GetPlotSILRestriction();

        //
        // Fill  with sets under the currently selected category.
        //
        int colIndex = restriction->GetCollectionIndex(cn.toStdString(), silTopSet);
        avtSILCollection_p collection =restriction->GetSILCollection(colIndex); 
        if (*collection != NULL)
        {
            int numSubsets = collection->GetNumberOfSubsets();
            for (int i = 0; i < numSubsets; ++i)
            {
                int setIdx = collection->GetSubset(i);
                if (silUseSet[setIdx] != 0)
                {
                    avtSILSet_p set = restriction->GetSILSet(setIdx);
                    subsetName->addItem(QString(set->GetName().c_str()));
                } 
            } 
            //
            // Set the current item for the subset 
            //
            if (subsetName->count() != 0)
            {
                if (userSubset == defaultItem && lastGoodSubset == defaultItem) 
                {
                    subsetName->setCurrentIndex(0);
                }
                else 
                {
                    int idx = subsetName->findText(userSubset, Qt::MatchExactly);
                    if (idx == -1)
                        idx = subsetName->findText(lastGoodSubset, Qt::MatchExactly);
                    idx = (idx == -1 ? 0 : idx);
                    subsetName->setCurrentIndex(idx);
                }
            }
        } /*if collection!=NULL */
    }
    if (subsetName->count() == 0)
    {
        subsetName->addItem(defaultItem);
        subsetName->setCurrentIndex(0);
        subsetName->setEnabled(false);
        subsetLabel->setEnabled(false);
    }
    else if (subsetName->count() == 1)
    {
        subsetName->setEnabled(false);
        subsetLabel->setEnabled(false);
    }
    else
    {
        subsetName->setEnabled(true);
        subsetLabel->setEnabled(true);
    }
    subsetName->blockSignals(false);
}

//
// Qt Slot functions
//

// ****************************************************************************
// Method: QvisSILSetSelector::categoryNameChanged
//
// Purpose: 
//   A Qt slot called when a new subset name is chosen, emits a signal
//   with the new subset name when not blocked.
//
// Programmer: Kathleen Bonnell 
// Creation:   June 6, 2007 
//
// Modifications:
//   
// ****************************************************************************

void
QvisSILSetSelector::categoryNameChanged()
{
    QString temp = categoryName->currentText();
    if (!temp.isEmpty())
    {
        if (temp != lastGoodCategory)
        {
            FillSubsetBox();
            lastGoodCategory = temp;
        }
        if (!signalsBlocked())
            emit categoryChanged(temp);
    }
}


// ****************************************************************************
// Method: QvisSILSetSelector::subsetNameChanged
//
// Purpose: 
//   A Qt slot called when a new subset name is chosen, emits a signal
//   with the new subset name when not blocked.
//
// Programmer: Kathleen Bonnell 
// Creation:   June 6, 2007 
//
// Modifications:
//   
// ****************************************************************************

void
QvisSILSetSelector::subsetNameChanged()
{
    QString temp = subsetName->currentText();
    if (!temp.isEmpty())
    {
        if (!signalsBlocked())
            emit subsetChanged(temp);
        lastGoodSubset = temp;
    }
}


// ****************************************************************************
// Method: QvisSILSetSelector::SetCategoryName
//
// Purpose: 
//   Sets the currently selected category name, if it exists in the list. 
//
// Arguments:
//   name      The category name to make current.
//
// Programmer: Kathleen Bonnell 
// Creation:   June 6, 2007 
//
// Modifications:
//   Kathleen Bonnell, Thu Jun 14 12:13:03 PDT 2007
//   Added userCategory.
//   
//   Hank Childs, Wed Jan  9 17:29:03 PST 2008
//   Call method categoryNameChanged, so the subset names will be set up.
//
//   Brad Whitlock, Fri Jul 18 08:45:12 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void 
QvisSILSetSelector::SetCategoryName(const QString &name)
{
    userCategory = name;
    blockSignals(true);
    if (categoryName->count() != 0)
    {
        int idx = categoryName->findText(name, Qt::MatchExactly);
        idx = (idx == -1 ? 0 : idx);
        categoryName->setCurrentIndex(idx);
        categoryNameChanged();
        lastGoodCategory = categoryName->currentText();
    }
    blockSignals(false);
}

// ****************************************************************************
// Method: QvisSILSetSelector::GetCategoryName
//
// Purpose: 
//   Returns the currently selected categoryName.
//
// Programmer: Kathleen Bonnell 
// Creation:   June 6, 2007 
//
// Modifications:
//   
// ****************************************************************************

QString 
QvisSILSetSelector::GetCategoryName() const
{
    return categoryName->currentText();
}


// ****************************************************************************
// Method: QvisSILSetSelector::SetSubsetName
//
// Purpose: 
//   Sets the currently selected subset name, if it exists in the list. 
//
// Arguments:
//   name      The subset name to make current.
//
// Programmer: Kathleen Bonnell 
// Creation:   June 6, 2007 
//
// Modifications:
//   Kathleen Bonnell, Thu Jun 14 12:13:03 PDT 2007
//   Added userSubset.
//
//   Brad Whitlock, Fri Jul 18 08:46:58 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void 
QvisSILSetSelector::SetSubsetName(const QString &name)
{
    userSubset = name;
    blockSignals(true);
    if (subsetName->count() != 0)
    {
        int idx = subsetName->findText(name, Qt::MatchExactly);
        idx = (idx == -1 ? 0 : idx);
        subsetName->setCurrentIndex(idx);
        lastGoodSubset = subsetName->currentText();
    }
    blockSignals(false);
}


// ****************************************************************************
// Method: QvisSILSetSelector::GetSubsetName
//
// Purpose: 
//   Returns the currently selected subset name. 
//
// Programmer: Kathleen Bonnell 
// Creation:   June 6, 2007 
//
// Modifications:
//   
// ****************************************************************************

QString 
QvisSILSetSelector::GetSubsetName() const
{
    return subsetName->currentText();
}
