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

#include "QvisOnionPeelWindow.h"

#include <OnionPeelAttributes.h>
#include <ViewerProxy.h>
#include <avtSIL.h>
#include <SILRestrictionAttributes.h>
#include <QvisSILSetSelector.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <stdio.h>
#include <vectortypes.h>



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
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Removed defaultItem, silTopSet, silNumSets, silNumCollections, silAtts.
//
// ****************************************************************************

QvisOnionPeelWindow::QvisOnionPeelWindow(const int type,
                         OnionPeelAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad) 
{
    atts = subj;
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
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Replace categoryName/subsetName combo boxes with QvisSILSetSelector.
//
//   Brad Whitlock, Fri Apr 25 08:54:57 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

void
QvisOnionPeelWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 7,3,  10, "mainLayout");

    //
    // Adjacency
    //
    mainLayout->addWidget(new QLabel(tr("Adjacency"), central, "adjacencyTypeLabel"),0,0);
    adjacencyType = new QButtonGroup(central, "adjacencyType");
    adjacencyType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *adjacencyTypeLayout = new QHBoxLayout(adjacencyType);
    adjacencyTypeLayout->setSpacing(10);
    QRadioButton *adjacencyTypeNodeFaceNode = new QRadioButton(tr("Node"), adjacencyType);
    adjacencyTypeLayout->addWidget(adjacencyTypeNodeFaceNode);
    QRadioButton *adjacencyTypeNodeFaceFace = new QRadioButton(tr("Face"), adjacencyType);
    adjacencyTypeLayout->addWidget(adjacencyTypeNodeFaceFace);
    connect(adjacencyType, SIGNAL(clicked(int)),
            this, SLOT(adjacencyTypeChanged(int)));
    mainLayout->addWidget(adjacencyType, 0,1);

    //
    // silSet (category/subset)
    //
    intVector roles;
    roles.push_back(SIL_DOMAIN);
    roles.push_back(SIL_BLOCK);
    silSet = new QvisSILSetSelector(central, "silSet", 
        GetViewerState()->GetSILRestrictionAttributes(), roles);
    connect(silSet, SIGNAL(categoryChanged(const QString &)),
            this, SLOT(categoryChanged(const QString &)));
    connect(silSet, SIGNAL(subsetChanged(const QString &)),
            this, SLOT(subsetChanged(const QString &)));
    mainLayout->addMultiCellWidget(silSet, 1,1, 0,2);

    //
    // Seed
    //
    mainLayout->addWidget(new QLabel(tr("Seed"), central, "seedTypeLabel"),2,0);
    seedType = new QButtonGroup(central, "seedType");
    seedType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *seedTypeLayout = new QHBoxLayout(seedType);
    seedTypeLayout->setSpacing(10);
    QRadioButton *rb = new QRadioButton(tr("Cell"), seedType);
    seedTypeLayout->addWidget(rb);
    rb = new QRadioButton(tr("Node"), seedType);
    seedTypeLayout->addWidget(rb);
    connect(seedType, SIGNAL(clicked(int)),
            this, SLOT(seedTypeChanged(int)));
    mainLayout->addWidget(seedType, 2,1);
    
    //
    // Index
    //
#if 0
    mainLayout->addMultiCellWidget(new QLabel(tr("Seed # or i j [k]"), 
                central, "indexLabel"),3,3,0,1);
#endif
    mainLayout->addWidget(new QLabel(tr("Seed # or i j [k]"), 
                central, "indexLabel"),3,0);
    index = new QLineEdit(central, "index");
    index->setText(QString("1"));
    connect(index, SIGNAL(returnPressed()),
            this, SLOT(indexChanged()));
    mainLayout->addWidget(index, 3,1);

    //
    // UseGlobalId
    //
    useGlobalId = new QCheckBox(tr("Seed # is Global"), central, "useGlobalId");
    useGlobalId->setChecked(false);
    connect(useGlobalId, SIGNAL(toggled(bool)),
            this, SLOT(useGlobalIdToggled(bool)));
    mainLayout->addMultiCellWidget(useGlobalId, 4,4, 0,1);

    //
    // Layers
    //
    mainLayout->addWidget(new QLabel(tr("Layers"), central, "requestedLayerLabel"),5,0);
    requestedLayer = new QSpinBox(0, 10000, 1, central, "requestedLayer");
    connect(requestedLayer, SIGNAL(valueChanged(int)), 
            this, SLOT(requestedLayerChanged(int)));
    mainLayout->addWidget(requestedLayer, 5,1);
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
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Removed calls to UpdateComboBoxes, replace category/subset with silSet.
//
// ****************************************************************************

void
QvisOnionPeelWindow::UpdateWindow(bool doAll)
{
    QString temp;
    int i, j;
    intVector ivec;


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
            silSet->setEnabled(!atts->GetUseGlobalId());
            useGlobalId->blockSignals(false);
            break;
        case 2: //Category 
            silSet->blockSignals(true);
            silSet->SetCategoryName(atts->GetCategoryName().c_str());
            silSet->blockSignals(false);
            break;
        case 3: // Subset 
            silSet->blockSignals(true);
            silSet->SetSubsetName(atts->GetSubsetName().c_str());
            silSet->blockSignals(false);
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
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Replaced category/subset with silSet.
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

    // Do categoryName && subsetName
    if(doAll)
    {
        atts->SetCategoryName(silSet->GetCategoryName().latin1());
        atts->SetSubsetName(silSet->GetSubsetName().latin1());
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
            msg = tr("The values for the index were invalid. "
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


// ****************************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Changed name from 'categoryNameChanged' to 'categoryChanged'. 
//   SetCategory name directly instead of calling GetCurrentValues. 
//   
// ****************************************************************************

void
QvisOnionPeelWindow::categoryChanged(const QString &cname)
{
    atts->SetCategoryName(cname.latin1());
    if (AutoUpdate())
        QTimer::singleShot(100, this, SLOT(delayedApply()));
    else
        Apply();
}


// ****************************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Changed name from 'subsetNameChanged' to 'subsetChanged'. SetSubsetname
//   directly instead of calling GetCurrentValues. 
//   
// ****************************************************************************

void
QvisOnionPeelWindow::subsetChanged(const QString &sname)
{
    atts->SetSubsetName(sname.latin1());
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

// ****************************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Replaced 'UpdateComboBoxesEnabled' with 'silSet->setEnabled'.
//   
// ****************************************************************************

void
QvisOnionPeelWindow::useGlobalIdToggled(bool val)
{
    if(val != atts->GetUseGlobalId())
    {
        atts->SetUseGlobalId(val);
        silSet->setEnabled(!val);
        if (AutoUpdate())
            QTimer::singleShot(100, this, SLOT(delayedApply()));
        else
            Apply();
    }
}

