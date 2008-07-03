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

#include "QvisIndexSelectWindow.h"

#include <IndexSelectAttributes.h>
#include <ViewerProxy.h>
#include <avtSIL.h>
#include <SILRestrictionAttributes.h>
#include <QvisSILSetSelector.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qtimer.h>
#include <qvbox.h>
#include <vectortypes.h>
#include <stdio.h>
#include <string>


using std::string;


#define MAX_VAL 100000000

// ****************************************************************************
// Method: QvisIndexSelectWindow::QvisIndexSelectWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Thu Jun 6 17:02:08 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisIndexSelectWindow::QvisIndexSelectWindow(const int type,
                         IndexSelectAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisIndexSelectWindow::~QvisIndexSelectWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Thu Jun 6 17:02:08 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisIndexSelectWindow::~QvisIndexSelectWindow()
{
}


// ****************************************************************************
// Method: QvisIndexSelectWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Thu Jun 6 17:02:08 PST 2002
//
// Modifications:
//   
//   Hank Childs, Sat Jun 29 16:17:41 PDT 2002
//   Re-enable coding for the Groups widget.  Modify layout so everything
//   spaces correctly.
//
//   Hank Childs, Fri Jul 12 15:47:43 PDT 2002
//   Fix typo (AI->All).
//
//   Kathleen Bonnell, Thu Aug 26 16:55:59 PDT 2004
//   Changed Min/Max/Incr from LineEdit to SpinBox for usability, added
//   labels and group boxes for each dim.
//
//   Kathleen Bonnell, Thu Jun  7 12:53:47 PDT 2007 
//   Added QvisSILSetSelector to replace domainIndex and groupIndex. 
//   Changed whichData button group to useWholeCollection checkbox.
// 
//   Brad Whitlock, Fri Apr 25 09:08:02 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

void
QvisIndexSelectWindow::CreateWindowContents()
{
    QGridLayout *wholeLayout = new QGridLayout(topLayout, 7, 4, 10,
                                              "wholeLayout");
    QGridLayout *mainLayout = new QGridLayout(4,2,  10, "mainLayout");
    wholeLayout->addMultiCellLayout(mainLayout, 0, 0, 0, 3);


    mainLayout->addWidget(new QLabel(tr("Dimension"), central, "dimLabel"),0,0);
    dim = new QButtonGroup(central, "dim");
    dim->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *dimLayout = new QHBoxLayout(dim);
    dimLayout->setSpacing(2);
    QRadioButton *dimDimensionOneD = new QRadioButton(tr("1D"), dim);
    dimLayout->addWidget(dimDimensionOneD);
    QRadioButton *dimDimensionTwoD = new QRadioButton(tr("2D"), dim);
    dimLayout->addWidget(dimDimensionTwoD);
    QRadioButton *dimDimensionThreeD = new QRadioButton(tr("3D"), dim);
    dimLayout->addWidget(dimDimensionThreeD);
    connect(dim, SIGNAL(clicked(int)),
            this, SLOT(dimChanged(int)));
    mainLayout->addMultiCellWidget(dim, 0,0,1,2);


    QGridLayout *labelLayout = new QGridLayout(1, 3, 10, "labelLayout");
    wholeLayout->addMultiCellLayout(labelLayout, 1, 1, 0,3 );

    QLabel *minLabel = new QLabel(tr("Min"), central, "minLabel");
    minLabel->setAlignment(AlignCenter);
    labelLayout->addWidget(minLabel, 0, 0); 
    QLabel *maxLabel = new QLabel(tr("Max"), central, "maxLabel");
    maxLabel->setAlignment(AlignCenter);
    labelLayout->addWidget(maxLabel, 0, 1); 
    QLabel *incrLabel = new QLabel(tr("Incr"), central, "incrLabel");
    incrLabel->setAlignment(AlignCenter);
    labelLayout->addWidget(incrLabel, 0, 2); 


    // 
    // Create the oneD spinBoxes
    // 
    oneDWidgetGroup = new QGroupBox(central, "oneDWidgetGroup"); 
    oneDWidgetGroup->setFrameShape(QFrame::NoFrame);
    QGridLayout *oneDLayout = new QGridLayout(oneDWidgetGroup, 1, 6);
    oneDLabel = new QLabel(central, tr("I"), oneDWidgetGroup);
    oneDLabel->setAlignment(AlignCenter);
    oneDLayout->addWidget(oneDLabel, 0, 0);

    // Set Up Min
    oneDMin = new QSpinBox(0, MAX_VAL, 1, oneDWidgetGroup, "oneDMin");
    connect(oneDMin, SIGNAL(valueChanged(int)),
             this, SLOT(oneDMinChanged(int)));
    oneDLayout->addWidget(oneDMin, 0, 1);
    oneDLayout->addItem(new QSpacerItem(5, 5), 0, 2);

    // Set Up Max
    oneDMax = new QSpinBox(-1, MAX_VAL, 1, oneDWidgetGroup, "oneDMax");
    oneDMax->setSpecialValueText(tr("max"));
    oneDMax->setValue(-1);
    connect(oneDMax, SIGNAL(valueChanged(int)),
             this, SLOT(oneDMaxChanged(int)));
    oneDLayout->addWidget(oneDMax, 0, 3);
    oneDLayout->addItem(new QSpacerItem(5, 5), 0, 4);

    // Set Up Incr
    oneDIncr = new QSpinBox(1, MAX_VAL, 1, oneDWidgetGroup, "oneDIncr");
    connect(oneDIncr, SIGNAL(valueChanged(int)),
             this, SLOT(oneDIncrChanged(int)));
    oneDLayout->addWidget(oneDIncr, 0, 5);

    wholeLayout->addMultiCellWidget(oneDWidgetGroup, 2,2, 0,3);

    // 
    // Create the twoD spinBoxes
    // 
    twoDWidgetGroup = new QGroupBox(central, "twoDWidgetGroup"); 
    twoDWidgetGroup->setFrameShape(QFrame::NoFrame);
    QGridLayout *twoDLayout = new QGridLayout(twoDWidgetGroup, 1, 6);
    twoDLabel = new QLabel(central, tr("J"), twoDWidgetGroup);
    twoDLabel->setAlignment(AlignCenter);
    twoDLayout->addWidget(twoDLabel, 0, 0);

    // Set Up Min
    twoDMin = new QSpinBox(0, MAX_VAL, 1, twoDWidgetGroup, "twoDMin");
    connect(twoDMin, SIGNAL(valueChanged(int)),
             this, SLOT(twoDMinChanged(int)));
    twoDLayout->addWidget(twoDMin, 0, 1);
    twoDLayout->addItem(new QSpacerItem(5, 5), 0, 2);

    // Set Up Max
    twoDMax = new QSpinBox(-1, MAX_VAL, 1, twoDWidgetGroup, "twoDMax");
    twoDMax->setSpecialValueText(tr("max"));
    twoDMax->setValue(-1);
    connect(twoDMax, SIGNAL(valueChanged(int)),
             this, SLOT(twoDMaxChanged(int)));
    twoDLayout->addWidget(twoDMax, 0, 3);
    twoDLayout->addItem(new QSpacerItem(5, 5), 0, 4);

    // Set Up Incr
    twoDIncr = new QSpinBox(1, MAX_VAL, 1, twoDWidgetGroup, "twoDIncr");
    connect(twoDIncr, SIGNAL(valueChanged(int)),
             this, SLOT(twoDIncrChanged(int)));
    twoDLayout->addWidget(twoDIncr, 0, 5);

    wholeLayout->addMultiCellWidget(twoDWidgetGroup, 3,3, 0,3);

    // 
    // Create the threeD spinBoxes
    // 
    threeDWidgetGroup = new QGroupBox(central, "threeDWidgetGroup"); 
    threeDWidgetGroup->setFrameShape(QFrame::NoFrame);

    QGridLayout *threeDLayout = new QGridLayout(threeDWidgetGroup, 1, 6);
    threeDLabel = new QLabel(central, tr("K"), threeDWidgetGroup);
    threeDLabel->setAlignment(AlignCenter);
    threeDLayout->addWidget(threeDLabel, 0, 0);

    // Set Up Min
    threeDMin = new QSpinBox(0, MAX_VAL, 1, threeDWidgetGroup, "threeDMin");
    connect(threeDMin, SIGNAL(valueChanged(int)),
             this, SLOT(threeDMinChanged(int)));
    threeDLayout->addWidget(threeDMin, 0, 1);
    threeDLayout->addItem(new QSpacerItem(5, 5), 0, 2);

    // Set Up Max
    threeDMax = new QSpinBox(-1, MAX_VAL, 1, threeDWidgetGroup, "threeDMax");
    threeDMax->setSpecialValueText(tr("max"));
    threeDMax->setValue(-1);
    connect(threeDMax, SIGNAL(valueChanged(int)),
             this, SLOT(threeDMaxChanged(int)));
    threeDLayout->addWidget(threeDMax, 0, 3);
    threeDLayout->addItem(new QSpacerItem(5, 5), 0, 4);

    // Set Up Incr
    threeDIncr = new QSpinBox(1, MAX_VAL, 1, threeDWidgetGroup, "threeDIncr");
    connect(threeDIncr, SIGNAL(valueChanged(int)),
             this, SLOT(threeDIncrChanged(int)));
    threeDLayout->addWidget(threeDIncr, 0, 5);

    wholeLayout->addMultiCellWidget(threeDWidgetGroup, 4,4, 0,3);


    useWholeCollection = new QCheckBox(tr("Use Whole Collection"), central, "useWholeCollection");
    useWholeCollection->setChecked(false);
    connect(useWholeCollection, SIGNAL(toggled(bool)),
            this, SLOT(useWholeCollectionToggled(bool)));
    wholeLayout->addMultiCellWidget(useWholeCollection, 5,5, 0,1);

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
    wholeLayout->addMultiCellWidget(silSet, 5,5, 2,3);
}


// ****************************************************************************
// Method: QvisIndexSelectWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Thu Jun 6 17:02:08 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Thu Aug 26 16:55:59 PDT 2004
//   Changed Min/Max/Incr from LineEdit to SpinBox.
//   
//   Kathleen Bonnell, Thu Jun  7 12:53:47 PDT 2007 
//   Added QvisSILSetSelector to replace domainIndex and groupIndex. 
//   Changed whichData button group to useWholeCollection checkbox.
//
//   Kathleen Bonnell,  Tue Jul 1 11:47:51 PDT 2008
//   Removed unreferenced variables.
//
// ****************************************************************************

void
QvisIndexSelectWindow::UpdateWindow(bool doAll)
{
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
          case 0: //dim
            if (atts->GetDim() == IndexSelectAttributes::TwoD ||
                atts->GetDim() == IndexSelectAttributes::ThreeD)
                twoDWidgetGroup->setEnabled(true);
            else
                twoDWidgetGroup->setEnabled(false);
            if (atts->GetDim() == IndexSelectAttributes::ThreeD)
                threeDWidgetGroup->setEnabled(true);
            else
                threeDWidgetGroup->setEnabled(false);
            dim->setButton(atts->GetDim());
            break;
          case 1: //xMin
            oneDMin->blockSignals(true);
            oneDMin->setValue(atts->GetXMin());
            oneDMin->blockSignals(false);
            break;
          case 2: //xMax
            oneDMax->blockSignals(true);
            oneDMax->setValue(atts->GetXMax());
            oneDMax->blockSignals(false);
            break;
          case 3: //xIncr
            oneDIncr->blockSignals(true);
            oneDIncr->setValue(atts->GetXIncr());
            oneDIncr->blockSignals(false);
            break;
          case 4: //yMin
            twoDMin->blockSignals(true);
            twoDMin->setValue(atts->GetYMin());
            twoDMin->blockSignals(false);
            break;
          case 5: //yMax
            twoDMax->blockSignals(true);
            twoDMax->setValue(atts->GetYMax());
            twoDMax->blockSignals(false);
            break;
          case 6: //yIncr
            twoDIncr->blockSignals(true);
            twoDIncr->setValue(atts->GetYIncr());
            twoDIncr->blockSignals(false);
            break;
          case 7: //zMin
            threeDMin->blockSignals(true);
            threeDMin->setValue(atts->GetZMin());
            threeDMin->blockSignals(false);
            break;
          case 8: //zMax
            threeDMax->blockSignals(true);
            threeDMax->setValue(atts->GetZMax());
            threeDMax->blockSignals(false);
            break;
          case 9: //zIncr
            threeDIncr->blockSignals(true);
            threeDIncr->setValue(atts->GetZIncr());
            threeDIncr->blockSignals(false);
            break;
          case 10: //useWholeCollection
            useWholeCollection->blockSignals(true);
            useWholeCollection->setChecked(atts->GetUseWholeCollection());
            silSet->setEnabled(!atts->GetUseWholeCollection());
            useWholeCollection->blockSignals(false);
            break;
          case 11: //categoryName
            silSet->blockSignals(true);
            silSet->SetCategoryName(atts->GetCategoryName().c_str());
            silSet->blockSignals(false);
            break;
          case 12: // Subset Name
            silSet->blockSignals(true);
            silSet->SetSubsetName(atts->GetSubsetName().c_str());
            silSet->blockSignals(false);
            break;
 
        }
    }
}


// ****************************************************************************
// Method: QvisIndexSelectWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Thu Jun 6 17:02:08 PST 2002
//
// Modifications:
//   
//   Hank Childs, Wed Jun 19 10:00:03 PDT 2002
//   If a tuple does not parse, make sure not to use those values.
//
//   Kathleen Bonnell, Thu Aug 26 16:55:59 PDT 2004 
//   Removed code associated with Min/Max/Incr, they are no longer line edits,
//   but spin boxes.
//
//   Kathleen Bonnell, Thu Oct  7 10:29:36 PDT 2004 
//   Added back in code associated with Min/Max/Incr, so that they get updated
//   correctly on Apply.
//
//   Kathleen Bonnell, Thu Jun  7 12:53:47 PDT 2007 
//   Added QvisSILSetSelector to replace domainIndex and groupIndex. 
//   Changed whichData button group to useWholeCollection checkbox.
//
//   Kathleen Bonnell,  Tue Jul 1 11:47:51 PDT 2008
//   Removed unreferenced variables.
//
// ****************************************************************************

void
QvisIndexSelectWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do dim
    if(which_widget == 0 || doAll)
    {
        // Nothing for dim
    }
    if(which_widget == 1 || doAll)
    {
        if (atts->GetXMin() != oneDMin->value())
            atts->SetXMin(oneDMin->value());
    }
    if(which_widget == 2 || doAll)
    {
        if (atts->GetXMax() != oneDMax->value())
            atts->SetXMax(oneDMax->value());
    }
    if(which_widget == 3 || doAll)
    {
        if (atts->GetXIncr() != oneDIncr->value())
            atts->SetXIncr(oneDIncr->value());
    }
    if(which_widget == 4 || doAll)
    {
        if (atts->GetYMin() != twoDMin->value())
            atts->SetYMin(twoDMin->value());
    }
    if(which_widget == 5 || doAll)
    {
        if (atts->GetYMax() != twoDMax->value())
            atts->SetYMax(twoDMax->value());
    }
    if(which_widget == 6 || doAll)
    {
        if (atts->GetYIncr() != twoDIncr->value())
            atts->SetYIncr(twoDIncr->value());
    }
    if(which_widget == 7 || doAll)
    {
        if (atts->GetZMin() != threeDMin->value())
            atts->SetZMin(threeDMin->value());
    }
    if(which_widget == 8 || doAll)
    {
        if (atts->GetZMax() != threeDMax->value())
            atts->SetZMax(threeDMax->value());
    }
    if(which_widget == 9 || doAll)
    {
        if (atts->GetZIncr() != threeDIncr->value())
            atts->SetZIncr(threeDIncr->value());
    }

    // Do categoryName and subsetName
    if(doAll)
    {
        atts->SetCategoryName(silSet->GetCategoryName().latin1());
        atts->SetSubsetName(silSet->GetSubsetName().latin1());
    }
}

//
// Qt Slot functions
//


void
QvisIndexSelectWindow::dimChanged(int val)
{
    if(val != atts->GetDim())
    {
        atts->SetDim(IndexSelectAttributes::Dimension(val));
        Apply();
    }
}


void
QvisIndexSelectWindow::oneDMinChanged(int)
{
    GetCurrentValues(1);
    Apply();
}

void
QvisIndexSelectWindow::oneDMaxChanged(int)
{
    GetCurrentValues(2);
    Apply();
}

void
QvisIndexSelectWindow::oneDIncrChanged(int)
{
    GetCurrentValues(3);
    Apply();
}

void
QvisIndexSelectWindow::twoDMinChanged(int)
{
    GetCurrentValues(4);
    Apply();
}

void
QvisIndexSelectWindow::twoDMaxChanged(int)
{
    GetCurrentValues(5);
    Apply();
}

void
QvisIndexSelectWindow::twoDIncrChanged(int)
{
    GetCurrentValues(6);
    Apply();
}

void
QvisIndexSelectWindow::threeDMinChanged(int)
{
    GetCurrentValues(7);
    Apply();
}

void
QvisIndexSelectWindow::threeDMaxChanged(int )
{
    GetCurrentValues(8);
    Apply();
}

void
QvisIndexSelectWindow::threeDIncrChanged(int)
{
    GetCurrentValues(9);
    Apply();
}


void
QvisIndexSelectWindow::useWholeCollectionToggled(bool val)
{
    if(val != atts->GetUseWholeCollection())
    {
        atts->SetUseWholeCollection(val);
        silSet->setEnabled(!val);
        if (AutoUpdate())
            QTimer::singleShot(100, this, SLOT(delayedApply()));
        else
            Apply();
    }
}


void
QvisIndexSelectWindow::categoryChanged(const QString &cname)
{
    atts->SetCategoryName(cname.latin1());
    if (AutoUpdate())
        QTimer::singleShot(100, this, SLOT(delayedApply()));
    else
        Apply();
}

void
QvisIndexSelectWindow::subsetChanged(const QString &sname)
{
    atts->SetSubsetName(sname.latin1());
    if (AutoUpdate())
        QTimer::singleShot(100, this, SLOT(delayedApply()));
    else
        Apply();
}

