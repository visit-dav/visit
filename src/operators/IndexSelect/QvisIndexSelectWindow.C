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

#include "QvisIndexSelectWindow.h"

#include <IndexSelectAttributes.h>
#include <ViewerProxy.h>
#include <avtSIL.h>
#include <SILRestrictionAttributes.h>
#include <QvisSILSetSelector.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QTimer>
#include <QWidget>
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
//   Cyrus Harrison, Tue Aug 19 16:23:19 PDT 2008
//   Qt4 Port. 
//
// ****************************************************************************

void
QvisIndexSelectWindow::CreateWindowContents()
{
    QGridLayout *wholeLayout = new QGridLayout();
    topLayout->addLayout(wholeLayout);
    QGridLayout *mainLayout = new QGridLayout();
    wholeLayout->addLayout(mainLayout, 0, 0, 1, 4);


    mainLayout->addWidget(new QLabel(tr("Dimension"), central),0,0);
    QWidget *dimWidget = new QWidget(central);
    dim = new QButtonGroup(dimWidget);
    QHBoxLayout *dimLayout = new QHBoxLayout(dimWidget);
    dimLayout->setSpacing(2);
    QRadioButton *dimDimensionOneD = new QRadioButton(tr("1D"), dimWidget);
    dim->addButton(dimDimensionOneD,0);
    dimLayout->addWidget(dimDimensionOneD);
    QRadioButton *dimDimensionTwoD = new QRadioButton(tr("2D"), dimWidget);
    dimLayout->addWidget(dimDimensionTwoD);
    dim->addButton(dimDimensionTwoD,1);
    QRadioButton *dimDimensionThreeD = new QRadioButton(tr("3D"), dimWidget);
    dimLayout->addWidget(dimDimensionThreeD);
    dim->addButton(dimDimensionThreeD,2);
    connect(dim, SIGNAL(buttonClicked(int)),
            this, SLOT(dimChanged(int)));
    mainLayout->addWidget(dimWidget, 0,1,1,2);

    QGridLayout *labelLayout = new QGridLayout();

    QLabel *minLabel = new QLabel(tr("Min"), central);
    minLabel->setAlignment(Qt::AlignCenter);
    labelLayout->addWidget(minLabel, 0, 1); 
    QLabel *maxLabel = new QLabel(tr("Max"), central);
    maxLabel->setAlignment(Qt::AlignCenter);
    labelLayout->addWidget(maxLabel, 0, 2); 
    QLabel *incrLabel = new QLabel(tr("Incr"), central);
    incrLabel->setAlignment(Qt::AlignCenter);
    labelLayout->addWidget(incrLabel, 0, 3); 
    wholeLayout->addLayout(labelLayout, 1, 0, 1,4 );

    // 
    // Create the oneD spinBoxes
    // 
    oneDWidget = new QWidget(central); 
    QGridLayout *oneDLayout = new QGridLayout(oneDWidget);
    oneDLabel = new QLabel(tr("I"), oneDWidget);
    oneDLabel->setAlignment(Qt::AlignCenter);
    oneDLayout->addWidget(oneDLabel, 0, 0);

    // Set Up Min
    oneDMin = new QSpinBox(oneDWidget);
    oneDMin->setRange(0,MAX_VAL);
    oneDMin->setSingleStep(1);
    connect(oneDMin, SIGNAL(valueChanged(int)),
             this, SLOT(oneDMinChanged(int)));
    oneDLayout->addWidget(oneDMin, 0, 1);
    oneDLayout->addItem(new QSpacerItem(5, 5), 0, 2);

    // Set Up Max
    oneDMax = new QSpinBox(oneDWidget);
    oneDMax->setRange(-1,MAX_VAL);
    oneDMax->setSingleStep(1);
    oneDMax->setSpecialValueText(tr("max"));
    oneDMax->setValue(-1);
    connect(oneDMax, SIGNAL(valueChanged(int)),
             this, SLOT(oneDMaxChanged(int)));
    oneDLayout->addWidget(oneDMax, 0, 3);
    oneDLayout->addItem(new QSpacerItem(5, 5), 0, 4);

    // Set Up Incr
    oneDIncr = new QSpinBox(oneDWidget);
    oneDIncr->setRange(1,MAX_VAL);
    oneDIncr->setSingleStep(1);
    connect(oneDIncr, SIGNAL(valueChanged(int)),
             this, SLOT(oneDIncrChanged(int)));
    oneDLayout->addWidget(oneDIncr, 0, 5);

    wholeLayout->addWidget(oneDWidget, 2,0, 1,4);

    // 
    // Create the twoD spinBoxes
    // 
    twoDWidget = new QWidget(central); 
    QGridLayout *twoDLayout = new QGridLayout(twoDWidget);
    twoDLabel = new QLabel(tr("J"), twoDWidget);
    twoDLabel->setAlignment(Qt::AlignCenter);
    twoDLayout->addWidget(twoDLabel, 0, 0);

    // Set Up Min
    twoDMin = new QSpinBox(twoDWidget);
    twoDMin->setRange(0,MAX_VAL);
    twoDMin->setSingleStep(1);
    connect(twoDMin, SIGNAL(valueChanged(int)),
             this, SLOT(twoDMinChanged(int)));
    twoDLayout->addWidget(twoDMin, 0, 1);
    twoDLayout->addItem(new QSpacerItem(5, 5), 0, 2);

    // Set Up Max
    twoDMax = new QSpinBox(twoDWidget);
    twoDMax->setRange(-1,MAX_VAL);
    twoDMax->setSingleStep(1);
    twoDMax->setSpecialValueText(tr("max"));
    twoDMax->setValue(-1);
    connect(twoDMax, SIGNAL(valueChanged(int)),
             this, SLOT(twoDMaxChanged(int)));
    twoDLayout->addWidget(twoDMax, 0, 3);
    twoDLayout->addItem(new QSpacerItem(5, 5), 0, 4);

    // Set Up Incr
    twoDIncr = new QSpinBox(twoDWidget);
    twoDIncr->setRange(1,MAX_VAL);
    twoDIncr->setSingleStep(1);
    connect(twoDIncr, SIGNAL(valueChanged(int)),
             this, SLOT(twoDIncrChanged(int)));
    twoDLayout->addWidget(twoDIncr, 0, 5);

    wholeLayout->addWidget(twoDWidget, 3,0, 1,4);

    // 
    // Create the threeD spinBoxes
    // 
    threeDWidget = new QWidget(central); 

    QGridLayout *threeDLayout = new QGridLayout(threeDWidget);
    threeDLabel = new QLabel(tr("K"), threeDWidget);
    threeDLabel->setAlignment(Qt::AlignCenter);
    threeDLayout->addWidget(threeDLabel, 0, 0);

    // Set Up Min
    threeDMin = new QSpinBox(threeDWidget);
    threeDMin->setRange(0,MAX_VAL);
    threeDMin->setSingleStep(1);
    connect(threeDMin, SIGNAL(valueChanged(int)),
             this, SLOT(threeDMinChanged(int)));
    threeDLayout->addWidget(threeDMin, 0, 1);
    threeDLayout->addItem(new QSpacerItem(5, 5), 0, 2);

    // Set Up Max
    threeDMax = new QSpinBox(threeDWidget);
    threeDMax->setRange(-1,MAX_VAL);
    threeDMax->setSingleStep(1);
    threeDMax->setSpecialValueText(tr("max"));
    threeDMax->setValue(-1);
    connect(threeDMax, SIGNAL(valueChanged(int)),
             this, SLOT(threeDMaxChanged(int)));
    threeDLayout->addWidget(threeDMax, 0, 3);
    threeDLayout->addItem(new QSpacerItem(5, 5), 0, 4);

    // Set Up Incr
    threeDIncr = new QSpinBox(threeDWidget);
    threeDIncr->setRange(1,MAX_VAL);
    threeDIncr->setSingleStep(1);
    connect(threeDIncr, SIGNAL(valueChanged(int)),
             this, SLOT(threeDIncrChanged(int)));
    threeDLayout->addWidget(threeDIncr, 0, 5);

    wholeLayout->addWidget(threeDWidget, 4,0, 1,4);


    useWholeCollection = new QCheckBox(tr("Use Whole Collection"), central);
    useWholeCollection->setChecked(false);
    connect(useWholeCollection, SIGNAL(toggled(bool)),
            this, SLOT(useWholeCollectionToggled(bool)));
    wholeLayout->addWidget(useWholeCollection, 5,0, 1,1);

    //
    // silSet (category/subset)
    //
    intVector roles;
    roles.push_back(SIL_DOMAIN);
    roles.push_back(SIL_BLOCK);
    silSet = new QvisSILSetSelector(central,
        GetViewerState()->GetSILRestrictionAttributes(), roles);
    connect(silSet, SIGNAL(categoryChanged(const QString &)),
            this, SLOT(categoryChanged(const QString &)));
    connect(silSet, SIGNAL(subsetChanged(const QString &)),
            this, SLOT(subsetChanged(const QString &)));
    wholeLayout->addWidget(silSet, 5,2, 1,2);
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
//   Cyrus Harrison, Tue Aug 19 16:23:19 PDT 2008
//   Qt4 Port. 
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
          case IndexSelectAttributes::ID_dim:
            if (atts->GetDim() == IndexSelectAttributes::TwoD ||
                atts->GetDim() == IndexSelectAttributes::ThreeD)
                twoDWidget->setEnabled(true);
            else
                twoDWidget->setEnabled(false);
            if (atts->GetDim() == IndexSelectAttributes::ThreeD)
                threeDWidget->setEnabled(true);
            else
                threeDWidget->setEnabled(false);
            dim->button(atts->GetDim())->setChecked(true);
            break;
          case IndexSelectAttributes::ID_xMin:
            oneDMin->blockSignals(true);
            oneDMin->setValue(atts->GetXMin());
            oneDMin->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_xMax:
            oneDMax->blockSignals(true);
            oneDMax->setValue(atts->GetXMax());
            oneDMax->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_xIncr:
            oneDIncr->blockSignals(true);
            oneDIncr->setValue(atts->GetXIncr());
            oneDIncr->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_yMin:
            twoDMin->blockSignals(true);
            twoDMin->setValue(atts->GetYMin());
            twoDMin->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_yMax:
            twoDMax->blockSignals(true);
            twoDMax->setValue(atts->GetYMax());
            twoDMax->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_yIncr:
            twoDIncr->blockSignals(true);
            twoDIncr->setValue(atts->GetYIncr());
            twoDIncr->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_zMin:
            threeDMin->blockSignals(true);
            threeDMin->setValue(atts->GetZMin());
            threeDMin->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_zMax:
            threeDMax->blockSignals(true);
            threeDMax->setValue(atts->GetZMax());
            threeDMax->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_zIncr:
            threeDIncr->blockSignals(true);
            threeDIncr->setValue(atts->GetZIncr());
            threeDIncr->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_useWholeCollection:
            useWholeCollection->blockSignals(true);
            useWholeCollection->setChecked(atts->GetUseWholeCollection());
            silSet->setEnabled(!atts->GetUseWholeCollection());
            useWholeCollection->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_categoryName:
            silSet->blockSignals(true);
            silSet->SetCategoryName(atts->GetCategoryName().c_str());
            silSet->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_subsetName:
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
//   Cyrus Harrison, Tue Aug 19 16:23:19 PDT 2008
//   Qt4 Port. 
//
// ****************************************************************************

void
QvisIndexSelectWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    if(which_widget == IndexSelectAttributes::ID_xMin || doAll)
    {
        if (atts->GetXMin() != oneDMin->value())
            atts->SetXMin(oneDMin->value());
    }
    if(which_widget == IndexSelectAttributes::ID_xMax || doAll)
    {
        if (atts->GetXMax() != oneDMax->value())
            atts->SetXMax(oneDMax->value());
    }
    if(which_widget == IndexSelectAttributes::ID_xIncr || doAll)
    {
        if (atts->GetXIncr() != oneDIncr->value())
            atts->SetXIncr(oneDIncr->value());
    }
    if(which_widget == IndexSelectAttributes::ID_yMin || doAll)
    {
        if (atts->GetYMin() != twoDMin->value())
            atts->SetYMin(twoDMin->value());
    }
    if(which_widget == IndexSelectAttributes::ID_yMax || doAll)
    {
        if (atts->GetYMax() != twoDMax->value())
            atts->SetYMax(twoDMax->value());
    }
    if(which_widget == IndexSelectAttributes::ID_yIncr || doAll)
    {
        if (atts->GetYIncr() != twoDIncr->value())
            atts->SetYIncr(twoDIncr->value());
    }
    if(which_widget == IndexSelectAttributes::ID_zMin || doAll)
    {
        if (atts->GetZMin() != threeDMin->value())
            atts->SetZMin(threeDMin->value());
    }
    if(which_widget == IndexSelectAttributes::ID_zMax || doAll)
    {
        if (atts->GetZMax() != threeDMax->value())
            atts->SetZMax(threeDMax->value());
    }
    if(which_widget == IndexSelectAttributes::ID_zIncr || doAll)
    {
        if (atts->GetZIncr() != threeDIncr->value())
            atts->SetZIncr(threeDIncr->value());
    }

    // Do categoryName and subsetName
    if(doAll)
    {
        atts->SetCategoryName(silSet->GetCategoryName().toStdString());
        atts->SetSubsetName(silSet->GetSubsetName().toStdString());
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
    GetCurrentValues(IndexSelectAttributes::ID_xMin);
    Apply();
}

void
QvisIndexSelectWindow::oneDMaxChanged(int)
{
    GetCurrentValues(IndexSelectAttributes::ID_xMax);
    Apply();
}

void
QvisIndexSelectWindow::oneDIncrChanged(int)
{
    GetCurrentValues(IndexSelectAttributes::ID_xIncr);
    Apply();
}

void
QvisIndexSelectWindow::twoDMinChanged(int)
{
    GetCurrentValues(IndexSelectAttributes::ID_yMin);
    Apply();
}

void
QvisIndexSelectWindow::twoDMaxChanged(int)
{
    GetCurrentValues(IndexSelectAttributes::ID_yMax);
    Apply();
}

void
QvisIndexSelectWindow::twoDIncrChanged(int)
{
    GetCurrentValues(IndexSelectAttributes::ID_yIncr);
    Apply();
}

void
QvisIndexSelectWindow::threeDMinChanged(int)
{
    GetCurrentValues(IndexSelectAttributes::ID_zMin);
    Apply();
}

void
QvisIndexSelectWindow::threeDMaxChanged(int )
{
    GetCurrentValues(IndexSelectAttributes::ID_zMax);
    Apply();
}

void
QvisIndexSelectWindow::threeDIncrChanged(int)
{
    GetCurrentValues(IndexSelectAttributes::ID_zIncr);
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
    atts->SetCategoryName(cname.toStdString());
    if (AutoUpdate())
        QTimer::singleShot(100, this, SLOT(delayedApply()));
    else
        Apply();
}

void
QvisIndexSelectWindow::subsetChanged(const QString &sname)
{
    atts->SetSubsetName(sname.toStdString());
    if (AutoUpdate())
        QTimer::singleShot(100, this, SLOT(delayedApply()));
    else
        Apply();
}

