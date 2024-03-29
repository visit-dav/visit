// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisIndexSelectWindow.h"

#include <IndexSelectAttributes.h>
#include <ViewerProxy.h>
#include <avtSIL.h>
#include <SILRestrictionAttributes.h>
#include <QvisSILSetSelector.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QTimer>
#include <QWidget>
#include <vectortypes.h>


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
    : QvisOperatorWindow(type, subj, caption, shortName, notepad)
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
//   Kathleen Biagas, Wed Jun  8 17:10:30 PDT 2016
//   Set keyboard tracking to false for spin boxes so that 'valueChanged'
//   signal will only emit when 'enter' is pressed or spinbox loses focus.
//
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisIndexSelectWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout();
    topLayout->addLayout(mainLayout);

    int row = 0;

    mainLayout->addWidget(new QLabel(tr("Dimension"), central), row ,0);
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
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(dim, SIGNAL(buttonClicked(int)),
            this, SLOT(dimChanged(int)));
#else
    connect(dim, SIGNAL(idClicked(int)),
            this, SLOT(dimChanged(int)));
#endif
    mainLayout->addWidget(dimWidget, row, 1, 1, 3);

    ++row;

    QLabel *indexLabel = new QLabel(tr("  "), central);
    mainLayout->addWidget(indexLabel, row, 0); 
    QLabel *minLabel = new QLabel(tr("Min"), central);
    mainLayout->addWidget(minLabel, row, 1); 
    QLabel *maxLabel = new QLabel(tr("Max"), central);
    mainLayout->addWidget(maxLabel, row, 2); 
    QLabel *incrLabel = new QLabel(tr("Incr"), central);
    mainLayout->addWidget(incrLabel, row, 3); 
    QLabel *wrapLabel = new QLabel(tr("Wrap"), central);
    mainLayout->addWidget(wrapLabel, row, 4); 

    ++row;

    // 
    // Create the oneD spinBoxes
    //
    oneDLabel = new QLabel(tr("I"), central);
    oneDLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(oneDLabel, row, 0);

    // Set Up Min
    oneDMin = new QSpinBox(central);
    oneDMin->setKeyboardTracking(false);
    if( atts->GetXAbsMax() == -1 )
      oneDMin->setRange(0, MAX_VAL);
    else
      oneDMin->setRange(0, atts->GetXAbsMax());
    oneDMin->setSingleStep(1);
    connect(oneDMin, SIGNAL(valueChanged(int)),
             this, SLOT(oneDMinChanged(int)));
    mainLayout->addWidget(oneDMin, row, 1);

    // Set Up Max
    oneDMax = new QSpinBox(central);
    oneDMin->setKeyboardTracking(false);
    oneDMax->setSingleStep(1);

    if( atts->GetXAbsMax() == -1 )
    {
      oneDMax->setRange(-1, MAX_VAL);
      oneDMax->setSpecialValueText(tr("max"));
      oneDMax->setValue(atts->GetXMax());
      connect(oneDMax, SIGNAL(editingFinished()),
              this, SLOT(oneDMaxChanged()));
    }
    else
    {
      oneDMax->setRange(0, atts->GetXAbsMax());
      oneDMax->setSpecialValueText(tr(""));
      oneDMax->setValue(atts->GetXMax());
      connect(oneDMax, SIGNAL(valueChanged(int)),
              this, SLOT(oneDMaxChanged(int)));
    }
    mainLayout->addWidget(oneDMax, row, 2);

    // Set Up Incr
    oneDIncr = new QSpinBox(central);
    oneDIncr->setKeyboardTracking(false);
    if( atts->GetXAbsMax() == -1 )
      oneDIncr->setRange(1, MAX_VAL);
    else
      oneDIncr->setRange(1, atts->GetXAbsMax()+1);
    oneDIncr->setSingleStep(1);
    connect(oneDIncr, SIGNAL(valueChanged(int)),
             this, SLOT(oneDIncrChanged(int)));
    mainLayout->addWidget(oneDIncr, row, 3);

    // Set up wrap
    oneDWrap = new QCheckBox(tr(""), central);
    oneDWrap->setChecked(false);
    connect(oneDWrap, SIGNAL(toggled(bool)),
            this, SLOT(oneDWrapToggled(bool)));
    mainLayout->addWidget(oneDWrap, row, 4);

    ++row;

    // 
    // Create the twoD spinBoxes
    // 
    twoDLabel = new QLabel(tr("J"), central);
    twoDLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(twoDLabel, row, 0);

    // Set Up Min
    twoDMin = new QSpinBox(central);
    twoDMin->setKeyboardTracking(false);
    if( atts->GetZAbsMax() == -1 )
      twoDMin->setRange(0, MAX_VAL);
    else
      twoDMin->setRange(0, atts->GetYAbsMax());
    twoDMin->setSingleStep(1);
    connect(twoDMin, SIGNAL(valueChanged(int)),
             this, SLOT(twoDMinChanged(int)));
    mainLayout->addWidget(twoDMin, row, 1);

    // Set Up Max
    twoDMax = new QSpinBox(central);
    twoDMax->setKeyboardTracking(false);
    twoDMax->setSingleStep(1);
    if( atts->GetZAbsMax() == -1 )
    {
      twoDMax->setRange(-1, MAX_VAL);
      twoDMax->setSpecialValueText(tr("max"));
      twoDMax->setValue(atts->GetYMax());
      connect(twoDMax, SIGNAL(editingFinished()),
              this, SLOT(twoDMaxChanged()));
    }
    else
    {
      twoDMax->setRange(0, atts->GetYAbsMax());
      twoDMax->setSpecialValueText(tr(""));
      twoDMax->setValue(atts->GetYMax());
      connect(twoDMax, SIGNAL(valueChanged(int)),
              this, SLOT(twoDMaxChanged(int)));
    }
    mainLayout->addWidget(twoDMax, row, 2);

    // Set Up Incr
    twoDIncr = new QSpinBox(central);
    twoDIncr->setKeyboardTracking(false);
    if( atts->GetZAbsMax() == -1 )
      twoDIncr->setRange(1,MAX_VAL);
    else
      twoDIncr->setRange(1,atts->GetYAbsMax()+1);
    twoDIncr->setSingleStep(1);
    connect(twoDIncr, SIGNAL(valueChanged(int)),
             this, SLOT(twoDIncrChanged(int)));
    mainLayout->addWidget(twoDIncr, row, 3);

    // Set up wrap
    twoDWrap = new QCheckBox(tr(""), central);
    twoDWrap->setChecked(false);
    connect(twoDWrap, SIGNAL(toggled(bool)),
            this, SLOT(twoDWrapToggled(bool)));
    mainLayout->addWidget(twoDWrap, row, 4);

    ++row;

    // 
    // Create the threeD spinBoxes
    // 
    threeDLabel = new QLabel(tr("K"), central);
    threeDLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(threeDLabel, row, 0);

    // Set Up Min
    threeDMin = new QSpinBox(central);
    threeDMin->setKeyboardTracking(false);
    if( atts->GetZAbsMax() == -1 )
      threeDMin->setRange(0, MAX_VAL);
    else
      threeDMin->setRange(0, atts->GetZAbsMax());
    threeDMin->setSingleStep(1);
    connect(threeDMin, SIGNAL(valueChanged(int)),
             this, SLOT(threeDMinChanged(int)));
    mainLayout->addWidget(threeDMin, row, 1);

    // Set Up Max
    threeDMax = new QSpinBox(central);
    threeDMax->setKeyboardTracking(false);
    threeDMax->setSingleStep(1);
    if( atts->GetZAbsMax() == -1 )
    {
      threeDMax->setRange(-1, MAX_VAL);
      threeDMax->setSpecialValueText(tr("max"));
      threeDMax->setValue(atts->GetZMax());
      connect(threeDMax, SIGNAL(editingFinished()),
              this, SLOT(threeDMaxChanged()));
    }
    else
    {
      threeDMax->setRange(0, atts->GetZAbsMax());
      threeDMax->setSpecialValueText(tr(""));
      threeDMax->setValue(atts->GetZMax());
      connect(threeDMax, SIGNAL(valueChanged(int)),
              this, SLOT(threeDMaxChanged(int)));
    }
    mainLayout->addWidget(threeDMax, row, 2);

    // Set Up Incr
    threeDIncr = new QSpinBox(central);
    threeDIncr->setKeyboardTracking(false);
    if( atts->GetZAbsMax() == -1 )
      threeDIncr->setRange(1, MAX_VAL);
    else
      threeDIncr->setRange(1, atts->GetZAbsMax()+1);
    threeDIncr->setSingleStep(1);
    connect(threeDIncr, SIGNAL(valueChanged(int)),
             this, SLOT(threeDIncrChanged(int)));
    mainLayout->addWidget(threeDIncr, row, 3);

    // Set up wrap
    threeDWrap = new QCheckBox(tr(""), central);
    threeDWrap->setChecked(false);
    connect(threeDWrap, SIGNAL(toggled(bool)),
            this, SLOT(threeDWrapToggled(bool)));
    mainLayout->addWidget(threeDWrap, row, 4);

    ++row;

    QGridLayout *wholeLayout = new QGridLayout();
    topLayout->addLayout(wholeLayout);

    // Set up whole collection
    useWholeCollection = new QCheckBox(tr("Use Whole Collection"), central);
    useWholeCollection->setChecked(false);
    connect(useWholeCollection, SIGNAL(toggled(bool)),
            this, SLOT(useWholeCollectionToggled(bool)));
    wholeLayout->addWidget(useWholeCollection, 5,0);

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
    wholeLayout->addWidget(silSet, 5,1);
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
            {
              bool twoD = (atts->GetDim() == IndexSelectAttributes::TwoD ||
                           atts->GetDim() == IndexSelectAttributes::ThreeD);

              twoDLabel->setEnabled(twoD);
              twoDMin->setEnabled(twoD);
              twoDMax->setEnabled(twoD);
              twoDIncr->setEnabled(twoD);
              twoDWrap->setEnabled(twoD);

              bool threeD = (atts->GetDim() == IndexSelectAttributes::ThreeD);

              threeDLabel->setEnabled(threeD);
              threeDMin->setEnabled(threeD);
              threeDMax->setEnabled(threeD);
              threeDIncr->setEnabled(threeD);
              threeDWrap->setEnabled(threeD);

              dim->button(atts->GetDim())->setChecked(true);
            }
            break;
          case IndexSelectAttributes::ID_xMin:
            oneDMin->blockSignals(true);
            oneDMin->setValue(atts->GetXMin());
            oneDMin->blockSignals(false);

            oneDMax->blockSignals(true);
            if( atts->GetXAbsMax() != -1)
              oneDMax->setMinimum(atts->GetXMin());
            else
              oneDMax->setMaximum(MAX_VAL);
            oneDMax->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_xMax:
            oneDMax->blockSignals(true);
            if( atts->GetXMax() == -1 && atts->GetXAbsMax() != -1)
              atts->SetXMax(atts->GetXAbsMax());
            oneDMax->setValue(atts->GetXMax());
            oneDMax->blockSignals(false);

            oneDMin->blockSignals(true);
            if( atts->GetXMax() == -1 && atts->GetXAbsMax() == -1)
              oneDMin->setMaximum(MAX_VAL);
            else
              oneDMin->setMaximum(atts->GetXMax());
            oneDMin->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_xAbsMax:
            oneDMin->blockSignals(true);
            if( atts->GetXAbsMax() == -1 )
              oneDMin->setRange(0,MAX_VAL);
            else
              oneDMax->setRange(0,atts->GetXAbsMax());
            oneDMin->blockSignals(false);
            
            oneDMax->blockSignals(true);
            if( atts->GetXAbsMax() == -1 )
            {
              oneDMax->setRange(-1,MAX_VAL);
              oneDMax->setSpecialValueText(tr("max"));
            }
            else
            {
              oneDMax->setRange(0,atts->GetXAbsMax());
              oneDMax->setSpecialValueText(tr(""));
            }
            oneDMax->setValue(atts->GetXMax());
            oneDMax->blockSignals(false);

            oneDIncr->blockSignals(true);
            if( atts->GetXAbsMax() == -1 )
              oneDIncr->setRange(1,MAX_VAL);
            else
              oneDIncr->setRange(1,atts->GetXAbsMax()+1);
            oneDIncr->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_xIncr:
            oneDIncr->blockSignals(true);
            oneDIncr->setValue(atts->GetXIncr());
            oneDIncr->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_xWrap:
            oneDWrap->blockSignals(true);
            oneDWrap->setChecked(atts->GetXWrap());
            oneDWrap->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_yMin:
            twoDMin->blockSignals(true);
            twoDMin->setValue(atts->GetYMin());
            twoDMin->blockSignals(false);

            twoDMax->blockSignals(true);
            if( atts->GetYAbsMax() != -1)
              twoDMax->setMinimum(atts->GetYMin());
            else
              twoDMax->setMaximum(MAX_VAL);
            twoDMax->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_yMax:
            twoDMax->blockSignals(true);
            if( atts->GetYMax() == -1 && atts->GetYAbsMax() != -1)
              atts->SetYMax(atts->GetYAbsMax());
            twoDMax->setValue(atts->GetYMax());
            twoDMax->blockSignals(false);

            twoDMin->blockSignals(true);
            if( atts->GetYMax() == -1 && atts->GetYAbsMax() == -1)
              twoDMin->setMaximum(MAX_VAL);
            else
              twoDMin->setMaximum(atts->GetYMax());
            twoDMin->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_yAbsMax:
            twoDMin->blockSignals(true);
            if( atts->GetYAbsMax() == -1 )
              twoDMin->setRange(0,MAX_VAL);
            else
              twoDMax->setRange(0,atts->GetYAbsMax());
            twoDMin->blockSignals(false);
            
            twoDMax->blockSignals(true);
            if( atts->GetYAbsMax() == -1 )
            {
              twoDMax->setRange(-1,MAX_VAL);
              twoDMax->setSpecialValueText(tr("max"));
            }
            else
            {
              twoDMax->setRange(0,atts->GetYAbsMax());
              twoDMax->setSpecialValueText(tr(""));
            }
            twoDMax->setValue(atts->GetYMax());
            twoDMax->blockSignals(false);

            twoDIncr->blockSignals(true);
            if( atts->GetYAbsMax() == -1 )
              twoDIncr->setRange(1,MAX_VAL);
            else
              twoDIncr->setRange(1,atts->GetYAbsMax()+1);
            twoDIncr->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_yIncr:
            twoDIncr->blockSignals(true);
            twoDIncr->setValue(atts->GetYIncr());
            twoDIncr->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_yWrap:
            twoDWrap->blockSignals(true);
            twoDWrap->setChecked(atts->GetYWrap());
            twoDWrap->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_zMin:
            threeDMin->blockSignals(true);
            threeDMin->setValue(atts->GetZMin());
            threeDMin->blockSignals(false);

            threeDMax->blockSignals(true);
            if( atts->GetYAbsMax() != -1)
              threeDMax->setMinimum(atts->GetZMin());
            else
              threeDMax->setMaximum(MAX_VAL);
            threeDMax->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_zMax:
            threeDMax->blockSignals(true);
            if( atts->GetZMax() == -1 && atts->GetZAbsMax() != -1)
              atts->SetZMax(atts->GetZAbsMax());
            threeDMax->setValue(atts->GetZMax());
            threeDMax->blockSignals(false);

            threeDMin->blockSignals(true);
            if( atts->GetZMax() == -1 && atts->GetZAbsMax() == -1)
              threeDMin->setMaximum(MAX_VAL);
            else
              threeDMin->setMaximum(atts->GetZMax());
            threeDMin->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_zAbsMax:
            threeDMin->blockSignals(true);
            if( atts->GetZAbsMax() == -1 )
              threeDMin->setRange(0,MAX_VAL);
            else
              threeDMax->setRange(0,atts->GetZAbsMax());
            threeDMin->blockSignals(false);
            
            threeDMax->blockSignals(true);
            if( atts->GetZAbsMax() == -1 )
            {
              threeDMax->setRange(-1,MAX_VAL);
              threeDMax->setSpecialValueText(tr("max"));
            }
            else
            {
              threeDMax->setRange(0,atts->GetZAbsMax());
              threeDMax->setSpecialValueText(tr(""));
            }
            threeDMax->setValue(atts->GetZMax());
            threeDMax->blockSignals(false);

            threeDIncr->blockSignals(true);
            if( atts->GetZAbsMax() == -1 )
              threeDIncr->setRange(1,MAX_VAL);
            else
              threeDIncr->setRange(1,atts->GetZAbsMax()+1);
            threeDIncr->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_zIncr:
            threeDIncr->blockSignals(true);
            threeDIncr->setValue(atts->GetZIncr());
            threeDIncr->blockSignals(false);
            break;
          case IndexSelectAttributes::ID_zWrap:
            threeDWrap->blockSignals(true);
            threeDWrap->setChecked(atts->GetZWrap());
            threeDWrap->blockSignals(false);
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
    if(which_widget == IndexSelectAttributes::ID_xWrap || doAll)
    {
        if (atts->GetXWrap() != oneDWrap->isChecked())
            atts->SetXWrap(oneDWrap->isChecked());
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
    if(which_widget == IndexSelectAttributes::ID_yWrap || doAll)
    {
        if (atts->GetYWrap() != twoDWrap->isChecked())
            atts->SetYWrap(twoDWrap->isChecked());
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
    if(which_widget == IndexSelectAttributes::ID_zWrap || doAll)
    {
        if (atts->GetZWrap() != threeDWrap->isChecked())
            atts->SetZWrap(threeDWrap->isChecked());
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
    if(val != atts->GetDim() && val <= atts->GetMaxDim() )
    {
        atts->SetDim(IndexSelectAttributes::Dimension(val));
        Apply();
    }
    else if( val > atts->GetMaxDim() )
    {
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
QvisIndexSelectWindow::oneDWrapToggled(bool)
{
    GetCurrentValues(IndexSelectAttributes::ID_xWrap);
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
QvisIndexSelectWindow::twoDWrapToggled(bool)
{
    GetCurrentValues(IndexSelectAttributes::ID_yWrap);
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
QvisIndexSelectWindow::threeDWrapToggled(bool)
{
    GetCurrentValues(IndexSelectAttributes::ID_zWrap);
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
