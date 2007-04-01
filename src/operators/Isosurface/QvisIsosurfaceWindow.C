#include "QvisIsosurfaceWindow.h"

#include <IsosurfaceAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <QvisColorTableButton.h>
#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisIsosurfaceWindow::QvisIsosurfaceWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Apr 16 17:41:29 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisIsosurfaceWindow::QvisIsosurfaceWindow(const int type,
                         IsosurfaceAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisIsosurfaceWindow::~QvisIsosurfaceWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Apr 16 17:41:29 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisIsosurfaceWindow::~QvisIsosurfaceWindow()
{
}


// ****************************************************************************
// Method: QvisIsosurfaceWindow::CreateWindow
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Apr 16 17:41:29 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Sep 8 17:03:40 PST 2003
//   I changed some menu labels to plural.
//
// ****************************************************************************

void
QvisIsosurfaceWindow::CreateWindowContents()
{
    topLayout->addSpacing(5);
    QGridLayout *limitsLayout = new QGridLayout(topLayout, 4, 3);
    limitsLayout->setSpacing(10);

    // Add the select by combo box.
    selectByComboBox = new QComboBox(false, central, "selectByComboBox");
    selectByComboBox->insertItem("N levels");
    selectByComboBox->insertItem("Value(s)");
    selectByComboBox->insertItem("Percent(s)");
    connect(selectByComboBox, SIGNAL(activated(int)),
           this, SLOT(selectByChanged(int)));
    QLabel *selectByLabel = new QLabel(selectByComboBox, "Select by",
                                       central, "selectByLabel");
    limitsLayout->addWidget(selectByLabel, 0, 0);
    limitsLayout->addWidget(selectByComboBox, 0, 1);
 
    // Add the select by text field.
    selectByLineEdit = new QLineEdit(central, "selectByLineEdit");
    connect(selectByLineEdit, SIGNAL(returnPressed()),
           this, SLOT(processSelectByText()));
    limitsLayout->addWidget(selectByLineEdit, 0, 2);
 
    //
    // Create the scale radio buttons
    //
    QHBoxLayout *scaleLayout = new QHBoxLayout(topLayout);
 
    // Create a group of radio buttons
    scalingButtons = new QButtonGroup( central, "scaleRadioGroup" );
    scalingButtons->setFrameStyle(QFrame::NoFrame);
    QLabel *scaleLabel = new QLabel(scalingButtons, "Scale", central,
        "scaleLabel");
    scaleLayout->addWidget(scaleLabel);
 
    QHBoxLayout *scaleButtonsLayout = new QHBoxLayout(scalingButtons);
    scaleButtonsLayout->setSpacing(10);
    QRadioButton *rb = new QRadioButton("Linear", scalingButtons );
    rb->setChecked( TRUE );
    scaleButtonsLayout->addWidget(rb);
    rb = new QRadioButton( scalingButtons );
    rb->setText( "Log" );
    scaleButtonsLayout->addWidget(rb);
    scaleLayout->addWidget( scalingButtons );
    scaleLayout->addStretch(0);
    // Each time a radio button is clicked, call the scaleClicked slot.
    connect(scalingButtons, SIGNAL(clicked(int)),
            this, SLOT(scaleClicked(int)));
 
    //
    // Create the Limits stuff
    //
    QLabel *limitsLabel = new QLabel("Limits", central, "limitsLabel");
    limitsLayout->addWidget(limitsLabel, 1, 0);
    // Create the min toggle and line edit
    minToggle = new QCheckBox("Min", central, "minToggle");
    limitsLayout->addWidget(minToggle, 1, 1);
    connect(minToggle, SIGNAL(toggled(bool)),
            this, SLOT(minToggled(bool)));
    minLineEdit = new QLineEdit(central, "minLineEdit");
    connect(minLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMinLimitText()));
    limitsLayout->addWidget(minLineEdit, 1, 2);
 
    // Create the max toggle and line edit
    maxToggle = new QCheckBox("Max", central, "maxToggle");
    limitsLayout->addWidget(maxToggle, 2, 1);
    connect(maxToggle, SIGNAL(toggled(bool)),
            this, SLOT(maxToggled(bool)));
    maxLineEdit = new QLineEdit(central, "maxLineEdit");
    connect(maxLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMaxLimitText()));
    limitsLayout->addWidget(maxLineEdit, 2, 2);

    limitsLayout->addWidget(new QLabel("variable", central, "variableLabel"),9,0);
    variable = new QLineEdit(central, "variable");
    connect(variable, SIGNAL(returnPressed()),
            this, SLOT(variableProcessText()));
    limitsLayout->addWidget(variable, 9,1);
}


// ****************************************************************************
// Method: QvisIsosurfaceWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue Apr 16 17:41:29 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Thu Oct  3 13:31:41 PDT 2002  
//   Disable min/max when contourMethod changes to "value", otherwise ensure
//   it is enabled.
//   
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisIsosurfaceWindow::UpdateWindow(bool doAll)
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
          case 0: //contourNLevels
            if(atts->GetContourMethod() 
               == IsosurfaceAttributes::Level)
                UpdateSelectByText();
            break;
          case 1: //contourValue
            if(atts->GetContourMethod()
               == IsosurfaceAttributes::Value)
                UpdateSelectByText();
            break;
          case 2: //contourPercent
            if(atts->GetContourMethod()
               == IsosurfaceAttributes::Percent)
                UpdateSelectByText();
            break;
          case 3: //contourMethod
            selectByComboBox->blockSignals(true);
            selectByComboBox->setCurrentItem(atts->GetContourMethod());
            selectByComboBox->blockSignals(false);
            if (atts->GetContourMethod() == 
                IsosurfaceAttributes::Value)
            {
                minToggle->setEnabled(false);
                maxToggle->setEnabled(false);
                minLineEdit->setEnabled(false);
                maxLineEdit->setEnabled(false);
            }
            else
            {
                minToggle->setEnabled(true);
                maxToggle->setEnabled(true);
                minLineEdit->setEnabled(atts->GetMinFlag());
                maxLineEdit->setEnabled(atts->GetMaxFlag());
            }
            UpdateSelectByText();
            break;
          case 4: //minFlag
            minToggle->blockSignals(true);
            minToggle->setChecked(atts->GetMinFlag());
            minLineEdit->setEnabled(atts->GetMinFlag());
            minToggle->blockSignals(false);
            break;
          case 5: //min
            temp.setNum(atts->GetMin());
            minLineEdit->setText(temp);
            break;
          case 6: //maxFlag
            maxToggle->blockSignals(true);
            maxToggle->setChecked(atts->GetMaxFlag());
            maxLineEdit->setEnabled(atts->GetMaxFlag());
            maxToggle->blockSignals(false);
            break;
          case 7: //max
            temp.setNum(atts->GetMax());
            maxLineEdit->setText(temp);
            break;
          case 8: //scaling
            scalingButtons->setButton(atts->GetScaling());
            break;
          case 9: //variable
            temp = atts->GetVariable().c_str();
            variable->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisIsosurfaceWindow::UpdateSelectByText
//
// Purpose:
//   This method updates the selectBy line edit when the value, percent, or
//   NLevels change.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 12:18:33 PDT 2001
//
// Modifications:
//
// ****************************************************************************
 
void
QvisIsosurfaceWindow::UpdateSelectByText()
{
    QString temp, temp2;
    int     i;
 
    if(atts->GetContourMethod() == IsosurfaceAttributes::Level)
    {
        temp.sprintf("%d", atts->GetContourNLevels());
        selectByLineEdit->setText(temp);
    }
    else if(atts->GetContourMethod() 
            == IsosurfaceAttributes::Value)
    {
        for(i = 0; i < atts->GetContourValue().size(); ++i)
        {
            temp2.sprintf("%g ", atts->GetContourValue()[i]);
            temp += temp2;
        }
        selectByLineEdit->setText(temp);
    }
    else if(atts->GetContourMethod()
            == IsosurfaceAttributes::Percent)
    {
        for(i = 0; i < atts->GetContourPercent().size(); ++i)
        {
            temp2.sprintf("%g ", atts->GetContourPercent()[i]);
            temp += temp2;
        }
        selectByLineEdit->setText(temp);
    }
}

// ****************************************************************************
// Method: QvisIsosurfaceWindow::ProcessSelectByText
//
// Purpose:
//   Processes the string in the selectByLineEdit and sets the results into
//   the contour attributes.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 12:50:35 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 15:20:33 PST 2002
//   Added a code to prevent more than MAX_CONTOURS contours.
//
// ****************************************************************************
 
void
QvisIsosurfaceWindow::ProcessSelectByText()
{
    doubleVector temp;
 
    if(atts->GetContourMethod() == IsosurfaceAttributes::Level)
    {
        // Try converting the line edit to a double vector so we can take
        // the first element as the number of levels.
        StringToDoubleList(selectByLineEdit->displayText().latin1(), temp);
 
        // If there were elements in the list use the first one, else use 10.
        int nlevels = (temp.size() > 0) ? int(temp[0]) : 10;
 
        // Prevent less than one contour
        if(nlevels < 1)
        {
            Warning("VisIt requires at least one contour.");
            nlevels = 1;
        }
 
        atts->SetContourNLevels(nlevels);
    }
    else if(atts->GetContourMethod() == IsosurfaceAttributes::Value)
    {
        // Convert the text fo a list of doubles and store them in the
        // contour's value vector.
        StringToDoubleList(selectByLineEdit->displayText().latin1(), temp);
        atts->SetContourValue(temp);
    }
    else if(atts->GetContourMethod() == IsosurfaceAttributes::Percent)
    {
        // Convert the text to a list of doubles and store them in the
        // contour's percent vector.
        StringToDoubleList(selectByLineEdit->displayText().latin1(), temp);
        atts->SetContourPercent(temp);
    }
}

// ****************************************************************************
// Method: QvisIsosurfaceWindow::StringToDoubleList
//
// Purpose:
//   Dissects a string into a list of doubles and stores it in the passed-in
//   doubleVector.
//
// Arguments:
//   str : The string to be searched for doubles.
//   dv  : The return vector to contain the doubles.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:05:15 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 15:18:35 PST 2002
//   Added code to prevent adding any more than MAX_CONTOURS elements to
//   the dv vector.
//
// ****************************************************************************
 
void
QvisIsosurfaceWindow::StringToDoubleList(const char *str, doubleVector &dv)
{
    int length, offset = 0;
 
    // Clear the vector.
    dv.clear();
 
    // Get out if the input string is nothing.
    if(str == NULL || ((length = strlen(str)) == 0))
    {
        return;
    }
 
    do {
        // Skip any preceding spaces, stop at end of string too.
        for(; str[offset] == ' ' || str[offset] == '\0'; ++offset);
 
        if(offset < length)
        {
            char buf[30];
            sscanf(str + offset, "%s", buf);
            offset += strlen(buf);
 
            // Only add if the token was something.
            if(strlen(buf) > 0)
            {
                double dtemp = (double)atof(buf);
                dv.push_back(dtemp);
            }
        }
    } while(offset < length);
}

// ****************************************************************************
// Method: QvisIsosurfaceWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Apr 16 17:41:29 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisIsosurfaceWindow::GetCurrentValues(int which_widget)
{
   bool okay, doAll = (which_widget == -1);
    QString msg, temp;
 
    // Dp the steby line edit.
    if(which_widget == 0 || doAll)
        ProcessSelectByText();
 
    // Do the minimum value.
    if(which_widget == 1 || doAll)
    {
        temp = minLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMin(val);
        }
 
        if(!okay)
        {
            msg.sprintf("The minimum value was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMin());
            Message(msg);
            atts->SetMin(atts->GetMin());
        }
    }
 
    // Do the maximum value
    if(which_widget == 2 || doAll)
    {
        temp = maxLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMax(val);
        }
 
        if(!okay)
        {
            msg.sprintf("The maximum value was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMax());
            Message(msg);
            atts->SetMax(atts->GetMax());
        }
    }

    // Do variable
    if(which_widget == 3 || doAll)
    {
        temp = variable->displayText();
        okay = !temp.isEmpty();
        if(okay)
        {
            atts->SetVariable(temp.latin1());
        }
 
        if(!okay)
        {
            msg.sprintf("The value of variable was invalid. "
                "Resetting to the last good value of %s.",
                atts->GetVariable().c_str());
            Message(msg);
            atts->SetVariable(atts->GetVariable());
        }
    }
}


//
// Qt Slot functions
//


void
QvisIsosurfaceWindow::selectByChanged(int mode)
{
    atts->SetContourMethod(IsosurfaceAttributes::Select_by(mode));
    Apply();
}
 
// ****************************************************************************
// Method: QvisIsosurfaceWindow::processSelectByText
//
// Purpose:
//   This is a Qt slot function that is called then the selectbyLineEdit
//   changes values.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 10:04:18 PDT 2001
//
// Modifications:
//
// ****************************************************************************
 
void
QvisIsosurfaceWindow::processSelectByText()
{
    GetCurrentValues(0);
    Apply();
}
 
// ****************************************************************************
// Method: QvisIsosurfaceWindow::minToggled
//
// Purpose:
//   This is a Qt slot function that is called when the min toggle is clicked.
//
// Arguments:
//   val : The toggle's new value.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 10:05:28 PDT 2001
//
// Modifications:
//
// ****************************************************************************
 
void
QvisIsosurfaceWindow::minToggled(bool val)
{
    atts->SetMinFlag(val);
    Apply();
}
 
// ****************************************************************************
// Method: QvisIsosurfaceWindow::scaleClicked
//
// Purpose:
//   This is a Qt slot function that is called when a scale button is clicked.
//
// Arguments:
//   button  :  Which scaling button was selected.
//
// Programmer: Kathleen Bonnell
// Creation:   February 27, 2001
//
// Modifications:
//
// ****************************************************************************
 
void
QvisIsosurfaceWindow::scaleClicked(int button)
{
    // Only do it if it changed.
    IsosurfaceAttributes::Scaling val = IsosurfaceAttributes::Scaling(button);
    if (val != atts->GetScaling())
    {
        atts->SetScaling(val);
        Apply();
    }
}
 
// ****************************************************************************
// Method: QvisIsosurfaceWindow::processMinLimitText
//
// Purpose:
//   This is a Qt slot function that is called when the min limit changes.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 10:11:38 PDT 2001
//
// Modifications:
//
// ****************************************************************************
 
void
QvisIsosurfaceWindow::processMinLimitText()
{
    GetCurrentValues(1);
    Apply();
}
 
// ****************************************************************************
// Method: QvisIsosurfaceWindow::maxToggled
//
// Purpose:
//   This is a Qt slot function that is called when the max toggle is clicked.
//
// Arguments:
//   val : The toggle's new value.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 10:05:28 PDT 2001
//
// Modifications:
//
// ****************************************************************************
 
void
QvisIsosurfaceWindow::maxToggled(bool val)
{
    atts->SetMaxFlag(val);
    Apply();
}
 
// ****************************************************************************
// Method: QvisIsosurfaceWindow::processMaxLimitText
//
// Purpose:
//   This is a Qt slot function that is called when the max limit changes.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 10:11:38 PDT 2001
//
// Modifications:
//
// ****************************************************************************
 
void
QvisIsosurfaceWindow::processMaxLimitText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisIsosurfaceWindow::variableProcessText()
{
    GetCurrentValues(3);
    Apply();
}


