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

#include "QvisIsosurfaceWindow.h"

#include <IsosurfaceAttributes.h>
#include <ViewerProxy.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>
#include <QButtonGroup>
#include <QRadioButton>
#include <QvisColorTableButton.h>
#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisVariableButton.h>

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
                         const QString &caption,
                         const QString &shortName,
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
//   Cyrus Harrison, Tue Aug 19 09:38:07 PDT 2008
//   Qt4 Port.
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
//   Brad Whitlock, Thu Dec 9 17:45:51 PST 2004
//   I made it use a variable button.
//
//   Brad Whitlock, Fri Apr 25 09:05:45 PDT 2008
//   Added tr()'s
//
//   Cyrus Harrison, Tue Aug 19 09:38:07 PDT 2008
//   Qt4 Port.
//
//   Dave Pugmire, Thu Oct 30 08:40:26 EDT 2008
//   Swapped the min/max fields.
//
// ****************************************************************************

void
QvisIsosurfaceWindow::CreateWindowContents()
{
    topLayout->addSpacing(5);
    QGridLayout *limitsLayout = new QGridLayout();
    topLayout->addLayout(limitsLayout);
    limitsLayout->setSpacing(10);

    // Add the select by combo box.
    selectByComboBox = new QComboBox(central);
    selectByComboBox->addItem(tr("N levels"));
    selectByComboBox->addItem(tr("Value(s)"));
    selectByComboBox->addItem(tr("Percent(s)"));
    connect(selectByComboBox, SIGNAL(activated(int)),
           this, SLOT(selectByChanged(int)));
    QLabel *selectByLabel = new QLabel(tr("Select by"),central);
    limitsLayout->addWidget(selectByLabel, 0, 0);
    limitsLayout->addWidget(selectByComboBox, 0, 1);
 
    // Add the select by text field.
    selectByLineEdit = new QLineEdit(central);
    connect(selectByLineEdit, SIGNAL(returnPressed()),
           this, SLOT(processSelectByText()));
    limitsLayout->addWidget(selectByLineEdit, 0, 2);
 
    //
    // Create the Limits stuff
    //
    QLabel *limitsLabel = new QLabel(tr("Limits"), central);
    limitsLayout->addWidget(limitsLabel, 1, 0);

    // Create the max toggle and line edit
    maxToggle = new QCheckBox(tr("Maximum"), central);
    limitsLayout->addWidget(maxToggle, 1, 1);
    connect(maxToggle, SIGNAL(toggled(bool)),
            this, SLOT(maxToggled(bool)));
    maxLineEdit = new QLineEdit(central);
    connect(maxLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMaxLimitText()));
    limitsLayout->addWidget(maxLineEdit, 1, 2);

    // Create the min toggle and line edit
    minToggle = new QCheckBox(tr("Minimum"), central);
    limitsLayout->addWidget(minToggle, 2, 1);
    connect(minToggle, SIGNAL(toggled(bool)),
            this, SLOT(minToggled(bool)));
    minLineEdit = new QLineEdit(central);
    connect(minLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMinLimitText()));
    limitsLayout->addWidget(minLineEdit, 2, 2);
 
    //
    // Create the variable stuff
    //
    QLabel *variableLabel = new QLabel(tr("Variable"), central, "variableLabel");
    limitsLayout->addWidget(variableLabel,3,0);

    variable = new QvisVariableButton(true, true, true, 
                                      QvisVariableButton::Scalars, central);
    connect(variable, SIGNAL(activated(const QString &)),
            this, SLOT(variableChanged(const QString &)));
    limitsLayout->addWidget(variable, 3, 1, 1, 2);
    
    //
    // Create the scale radio buttons
    //
    
    scalingButtons = new QButtonGroup(central);
    QHBoxLayout *scaleButtonsLayout = new QHBoxLayout();
    topLayout->addLayout(scaleButtonsLayout);
    
    QLabel *scaleLabel = new QLabel(tr("Scale"), central);
    scaleButtonsLayout->addWidget(scaleLabel);
    scaleButtonsLayout->setSpacing(10);
    
    QRadioButton *rb = new QRadioButton(tr("Linear"), central);
    scalingButtons->addButton(rb,0);
    scaleButtonsLayout->addWidget(rb);
    
    rb = new QRadioButton(tr("Log"),central);
    scalingButtons->addButton(rb,1);
    scaleButtonsLayout->addWidget(rb);
    scaleButtonsLayout->addStretch(5);
    
    scalingButtons->button(0)->setChecked(true);
    
    // Each time a radio button is clicked, call the scaleClicked slot.
    connect(scalingButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(scaleClicked(int)));

    //
    // Create the scale radio buttons
    //
    QHBoxLayout *scaleLayout = new QHBoxLayout(topLayout);
 
    // Create a group of radio buttons
    scalingButtons = new QButtonGroup( central, "scaleRadioGroup" );
    scalingButtons->setFrameStyle(QFrame::NoFrame);
    QLabel *scaleLabel = new QLabel(scalingButtons, tr("Scale"), central,
        "scaleLabel");
    scaleLayout->addWidget(scaleLabel);
 
    QHBoxLayout *scaleButtonsLayout = new QHBoxLayout(scalingButtons);
    scaleButtonsLayout->setSpacing(10);
    QRadioButton *rb = new QRadioButton(tr("Linear"), scalingButtons );
    rb->setChecked( TRUE );
    scaleButtonsLayout->addWidget(rb);
    rb = new QRadioButton( scalingButtons );
    rb->setText( tr("Log") );
    scaleButtonsLayout->addWidget(rb);
    scaleLayout->addWidget( scalingButtons );
    scaleLayout->addStretch(0);
    // Each time a radio button is clicked, call the scaleClicked slot.
    connect(scalingButtons, SIGNAL(clicked(int)),
            this, SLOT(scaleClicked(int)));
 
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
//   Cyrus Harrison, Tue Aug 19 09:38:07 PDT 2008
//   Qt4 Port.
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
          case IsosurfaceAttributes::ID_contourNLevels: 
            if(atts->GetContourMethod() == IsosurfaceAttributes::Level)
                UpdateSelectByText();
            break;
          case IsosurfaceAttributes::ID_contourValue: 
            if(atts->GetContourMethod() == IsosurfaceAttributes::Value)
                UpdateSelectByText();
            break;
          case IsosurfaceAttributes::ID_contourPercent:
            if(atts->GetContourMethod() == IsosurfaceAttributes::Percent)
                UpdateSelectByText();
            break;
          case IsosurfaceAttributes::ID_contourMethod:
            selectByComboBox->blockSignals(true);
            selectByComboBox->setCurrentIndex(atts->GetContourMethod());
            selectByComboBox->blockSignals(false);
            if (atts->GetContourMethod() == IsosurfaceAttributes::Value)
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
          case IsosurfaceAttributes::ID_minFlag:
            minToggle->blockSignals(true);
            minToggle->setChecked(atts->GetMinFlag());
            minLineEdit->setEnabled(atts->GetMinFlag());
            minToggle->blockSignals(false);
            break;
          case IsosurfaceAttributes::ID_min:
            minLineEdit->setText(DoubleToQString(atts->GetMin()));
            break;
          case IsosurfaceAttributes::ID_maxFlag:
            maxToggle->blockSignals(true);
            maxToggle->setChecked(atts->GetMaxFlag());
            maxLineEdit->setEnabled(atts->GetMaxFlag());
            maxToggle->blockSignals(false);
            break;
          case IsosurfaceAttributes::ID_max: 
            maxLineEdit->setText(DoubleToQString(atts->GetMax()));
            break;
          case IsosurfaceAttributes::ID_scaling: 
            scalingButtons->button(atts->GetScaling())->setChecked(true);
            break;
          case IsosurfaceAttributes::ID_variable:
            variable->blockSignals(true);
            variable->setText(QString(atts->GetVariable().c_str()));
            variable->blockSignals(false);
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
//   Cyrus Harrison, Tue Aug 19 09:38:07 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************
 
void
QvisIsosurfaceWindow::UpdateSelectByText()
{
     if(atts->GetContourMethod() == IsosurfaceAttributes::Level)
    {
        selectByLineEdit->setText(IntToQString(atts->GetContourNLevels()));
    }
    else if(atts->GetContourMethod() == IsosurfaceAttributes::Value)
    {
        selectByLineEdit->setText(DoublesToQString(atts->GetContourValue()));
    }
    else if(atts->GetContourMethod() == IsosurfaceAttributes::Percent)
    {
        selectByLineEdit->setText(DoublesToQString(atts->GetContourPercent()));
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
//   Cyrus Harrison, Tue Aug 19 09:38:07 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************
 
void
QvisIsosurfaceWindow::ProcessSelectByText()
{
    doubleVector temp;
 
    // Try converting the line edit to a double vector
    LineEditGetDoubles(selectByLineEdit, temp);
    
    if(atts->GetContourMethod() == IsosurfaceAttributes::Level)
    {
        // take the first element as the number of levels.
        // If there were elements in the list use the first one, else use 10.
        int nlevels = (temp.size() > 0) ? int(temp[0]) : 10;
 
        // Prevent less than one contour
        if(nlevels < 1)
        {
            Warning(tr("VisIt requires at least one contour."));
            nlevels = 1;
        }
 
        atts->SetContourNLevels(nlevels);
    }
    else if(atts->GetContourMethod() == IsosurfaceAttributes::Value)
    {
        // store values in contour's value vector.
        atts->SetContourValue(temp);
    }
    else if(atts->GetContourMethod() == IsosurfaceAttributes::Percent)
    {
        // store values contour's percent vector.
        atts->SetContourPercent(temp);
    }
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
//   Brad Whitlock, Thu Dec 9 17:47:34 PST 2004
//   I removed the coding to get the variable since it's now in a widget that
//   does not need that.
//
//   Cyrus Harrison, Tue Aug 19 09:38:07 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

void
QvisIsosurfaceWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
 
    // Dp the steby line edit.
    if(which_widget == 0 || doAll)
        ProcessSelectByText();
 
    // Do the minimum value.
    if(which_widget == IsosurfaceAttributes::ID_min || doAll)
    {   
        double val;
        if(LineEditGetDouble(minLineEdit, val))
            atts->SetMin(val);
        else
        {
            ResettingError(tr("minimum value"),
                DoubleToQString(atts->GetMin()));
            atts->SetMin(atts->GetMin());
        }
    }
 
    // Do the maximum value
    if(which_widget == IsosurfaceAttributes::ID_max || doAll)
    {
        double val;
        if(LineEditGetDouble(maxLineEdit, val))
            atts->SetMax(val);
        else
        {
            ResettingError(tr("maximum value"),
                DoubleToQString(atts->GetMax()));
            atts->SetMax(atts->GetMax());
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
//   Cyrus Harrison, Tue Aug 19 09:38:07 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************
 
void
QvisIsosurfaceWindow::processMinLimitText()
{
    GetCurrentValues(IsosurfaceAttributes::ID_min);
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
//   Cyrus Harrison, Tue Aug 19 09:38:07 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************
 
void
QvisIsosurfaceWindow::processMaxLimitText()
{
    GetCurrentValues(IsosurfaceAttributes::ID_max);
    Apply();
}

// ****************************************************************************
// Method: QvisIsosurfaceWindow::variableChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user selects a new
//   variable.
//
// Arguments:
//   var : The name of the selected variable.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 17:48:30 PST 2004
//
// Modifications:
//   Cyrus Harrison, Tue Aug 19 09:38:07 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

void
QvisIsosurfaceWindow::variableChanged(const QString &var)
{
    atts->SetVariable(var.toStdString());
    SetUpdate(false);
    Apply();
}


