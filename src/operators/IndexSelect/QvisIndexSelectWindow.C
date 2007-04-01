#include "QvisIndexSelectWindow.h"

#include <IndexSelectAttributes.h>
#include <ViewerProxy.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <stdio.h>
#include <string>

using std::string;


#define MAX_VAL 1000

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
                         const char *caption,
                         const char *shortName,
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
// ****************************************************************************

void
QvisIndexSelectWindow::CreateWindowContents()
{
    QGridLayout *wholeLayout = new QGridLayout(topLayout, 5, 4, 10,
                                              "wholeLayout");
    QGridLayout *mainLayout = new QGridLayout(4,2,  10, "mainLayout");
    wholeLayout->addMultiCellLayout(mainLayout, 0, 0, 0, 3);


    mainLayout->addWidget(new QLabel("Dimension", central, "dimLabel"),0,0);
    dim = new QButtonGroup(central, "dim");
    dim->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *dimLayout = new QHBoxLayout(dim);
    dimLayout->setSpacing(2);
    QRadioButton *dimDimensionOneD = new QRadioButton("1D", dim);
    dimLayout->addWidget(dimDimensionOneD);
    QRadioButton *dimDimensionTwoD = new QRadioButton("2D", dim);
    dimLayout->addWidget(dimDimensionTwoD);
    QRadioButton *dimDimensionThreeD = new QRadioButton("3D", dim);
    dimLayout->addWidget(dimDimensionThreeD);
    connect(dim, SIGNAL(clicked(int)),
            this, SLOT(dimChanged(int)));
    mainLayout->addMultiCellWidget(dim, 0,0,1,2);


    QGridLayout *labelLayout = new QGridLayout(1, 3, 10, "labelLayout");
    wholeLayout->addMultiCellLayout(labelLayout, 1, 1, 0,3 );

    QLabel *minLabel = new QLabel("Min", central, "minLabel");
    minLabel->setAlignment(AlignCenter);
    labelLayout->addWidget(minLabel, 0, 0); 
    QLabel *maxLabel = new QLabel("Max", central, "maxLabel");
    maxLabel->setAlignment(AlignCenter);
    labelLayout->addWidget(maxLabel, 0, 1); 
    QLabel *incrLabel = new QLabel("Incr", central, "incrLabel");
    incrLabel->setAlignment(AlignCenter);
    labelLayout->addWidget(incrLabel, 0, 2); 


    // 
    // Create the oneD spinBoxes
    // 
    oneDWidgetGroup = new QGroupBox(central, "oneDWidgetGroup"); 
    oneDWidgetGroup->setFrameShape(QFrame::NoFrame);
    QGridLayout *oneDLayout = new QGridLayout(oneDWidgetGroup, 1, 6);
    oneDLabel = new QLabel(central, "I", oneDWidgetGroup);
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
    oneDMax->setSpecialValueText("max");
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
    twoDLabel = new QLabel(central, "J", twoDWidgetGroup);
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
    twoDMax->setSpecialValueText("max");
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
    threeDLabel = new QLabel(central, "K", threeDWidgetGroup);
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
    threeDMax->setSpecialValueText("max");
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


    whichData = new QButtonGroup(central, "whichData");
    whichData->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *whichDataLayout = new QHBoxLayout(whichData);
    whichDataLayout->setSpacing(5);
    QRadioButton *whichDataDataTypeAllDomains = new QRadioButton("All", whichData);
    whichDataLayout->addWidget(whichDataDataTypeAllDomains);
    QRadioButton *whichDataDataTypeOneDomain = new QRadioButton("Block", whichData);
    whichDataLayout->addWidget(whichDataDataTypeOneDomain);

    domainIndex = new QLineEdit(whichData, "domainIndex");
    domainIndex->setMaximumWidth(30);
    connect(domainIndex, SIGNAL(returnPressed()),
            this, SLOT(domainIndexProcessText()));
    whichDataLayout->addWidget(domainIndex);

    QRadioButton *whichDataDataTypeOneGroup = new QRadioButton("Group", whichData);
    whichDataLayout->addWidget(whichDataDataTypeOneGroup);
    connect(whichData, SIGNAL(clicked(int)),
            this, SLOT(whichDataChanged(int)));

    groupIndex = new QLineEdit(whichData, "groupIndex");
    groupIndex->setMaximumWidth(30);
    connect(groupIndex, SIGNAL(returnPressed()),
            this, SLOT(groupIndexProcessText()));
    whichDataLayout->addWidget(groupIndex);

    wholeLayout->addMultiCellWidget(whichData, 5,5,0,3);
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
// ****************************************************************************

void
QvisIndexSelectWindow::UpdateWindow(bool doAll)
{
    QString temp;

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
          case 10: //whichData
            whichData->setButton(atts->GetWhichData());
            if (atts->GetWhichData() == IndexSelectAttributes::OneDomain)
            {
                domainIndex->setEnabled(true);
                groupIndex->setEnabled(false);
            }
            else if (atts->GetWhichData() == IndexSelectAttributes::OneGroup)
            {
                domainIndex->setEnabled(false);
                groupIndex->setEnabled(true);
            }
            else
            {
                domainIndex->setEnabled(false);
                groupIndex->setEnabled(false);
            }
            break;
          case 11: //domainIndex
            temp.sprintf("%d", atts->GetDomainIndex());
            domainIndex->setText(temp);
            break;
          case 12: //groupIndex
            temp.sprintf("%d", atts->GetGroupIndex());
            groupIndex->setText(temp);
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
// ****************************************************************************

void
QvisIndexSelectWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

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

    // Do whichData
    if(which_widget == 10 || doAll)
    {
        // Nothing for whichData
    }

    // Do domainIndex
    if(which_widget == 11 || doAll)
    {
        temp = domainIndex->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetDomainIndex(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of domainIndex was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetDomainIndex());
            Message(msg);
            atts->SetDomainIndex(atts->GetDomainIndex());
        }
    }

    // Do groupIndex
    if(which_widget == 12 || doAll)
    {
        temp = groupIndex->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetGroupIndex(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of groupIndex was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetGroupIndex());
            Message(msg);
            atts->SetGroupIndex(atts->GetGroupIndex());
        }
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
QvisIndexSelectWindow::whichDataChanged(int val)
{
    if(val != atts->GetWhichData())
    {
        atts->SetWhichData(IndexSelectAttributes::DataType(val));
        Apply();
    }
}


void
QvisIndexSelectWindow::domainIndexProcessText()
{
    GetCurrentValues(11);
    Apply();
}


void
QvisIndexSelectWindow::groupIndexProcessText()
{
    GetCurrentValues(12);
    Apply();
}


