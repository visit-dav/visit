/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "QvisThresholdWindow.h"

#include <ThresholdAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbitmap.h>
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
// Method: QvisThresholdWindow::QvisThresholdWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   
//   Mark Blair, Tue Mar  7 13:25:00 PST 2006
//   Upgraded to support multiple threshold variables.
//
//   Mark Blair, Wed Sep  6 19:33:00 PDT 2006
//   Removed problematic mechanism for accommodating ExtentsAttributes from
//   extents tool.
//
//   Mark Blair, Thu Sep 21 15:16:27 PDT 2006
//   Added support for input from Extents tool.  Certain Threshold changes are
//   preserved when viewer sends attributes that do not know about the changes.
//   (See full explanation in header of RestoreAppropriateUnappliedAttributes.)
//
//   Mark Blair, Tue Oct 31 20:18:10 PST 2006
//   Previous change undone.  Too much conflict when multiple vis windows in
//   use.  All unapplied Threshold GUI changes are now lost if user dismisses
//   then reopens GUI, or if user moves an arrowhead in Extents tool of a second
//   vis window tool-locked to the Threshold operator's vis window.  Too bad.
//
// ****************************************************************************

QvisThresholdWindow::QvisThresholdWindow(const int type,
                         ThresholdAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisThresholdWindow::~QvisThresholdWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   
//   Mark Blair, Thu Sep 21 15:16:27 PDT 2006
//   Need to delete GUI variable list object.
//
//   Mark Blair, Tue Oct 31 20:18:10 PST 2006
//   Previous change undone.
//
// ****************************************************************************

QvisThresholdWindow::~QvisThresholdWindow()
{
    // Nothing here.
}


// ****************************************************************************
// Method: QvisThresholdWindow::CreateWindow
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Dec 10 09:40:53 PDT 2004
//   Changed so it uses a variable button. I also improved the widget spacing.
//
//   Hank Childs, Tue Sep 13 09:25:35 PDT 2005
//   Add support for "PointsOnly".
//
//   Mark Blair, Tue Mar  7 13:25:00 PST 2006
//   Upgraded to support multiple threshold variables.
//
//   Mark Blair, Tue Aug  8 17:47:00 PDT 2006
//   Now accommodates an empty list of threshold variables.
//
// ****************************************************************************

void
QvisThresholdWindow::CreateWindowContents()
{
    static unsigned char leftArrow[8] =
        { 0x00, 0x01, 0x07, 0x1f, 0x7f, 0x1f, 0x07, 0x01 };
    static unsigned char rightArrow[8] =
        { 0x00, 0x80, 0xe0, 0xf8, 0xfe, 0xf8, 0xe0, 0x80 };

    QGroupBox *shownVarBox = new QGroupBox(central, "shownVarBox");
    shownVarBox->setTitle("For the currently selected variable");
    topLayout->addWidget(shownVarBox);

    QGridLayout *shownVarLayout = new QGridLayout(shownVarBox, 6, 5, 15, 5);

    shownVarLayout->addRowSpacing(0, 15);

    zonePortionLabel =
        new QLabel("Include zone if", shownVarBox, "zonePortionLabel");
    shownVarLayout->addMultiCellWidget(zonePortionLabel, 1, 1, 0, 1);
    zonePortion = new QButtonGroup(shownVarBox, "zonePortion");
    zonePortion->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *zonePortionLayout = new QHBoxLayout(zonePortion, 0, 10);
    QRadioButton *entirelyInRange =
        new QRadioButton("Entirely in range", zonePortion);
    zonePortionLayout->addWidget(entirelyInRange);
    QRadioButton *anyPartInRange =
        new QRadioButton("Any part in range", zonePortion);
    zonePortionLayout->addWidget(anyPartInRange);
    connect(zonePortion, SIGNAL(clicked(int)),
            this, SLOT(zonePortionChanged(int)));
    shownVarLayout->addMultiCellWidget(zonePortion, 1, 1, 2, 5);

    lowerBoundLabel = new QLabel("Lower bound", shownVarBox, "lowerBoundLabel");
    shownVarLayout->addMultiCellWidget(lowerBoundLabel, 2, 2, 0, 1);
    lowerBound = new QLineEdit(shownVarBox, "lowerBound");
    connect(lowerBound, SIGNAL(returnPressed()), this, SLOT(lowerBoundChanged()));
    connect(lowerBound, SIGNAL(lostFocus()), this, SLOT(lowerBoundChanged()));
    shownVarLayout->addMultiCellWidget(lowerBound, 2, 2, 2, 5);

    upperBoundLabel = new QLabel("Upper bound", shownVarBox, "upperBoundLabel");
    shownVarLayout->addMultiCellWidget(upperBoundLabel, 3, 3, 0, 1);
    upperBound = new QLineEdit(shownVarBox, "upperBound");
    connect(upperBound, SIGNAL(returnPressed()), this, SLOT(upperBoundChanged()));
    connect(upperBound, SIGNAL(lostFocus()), this, SLOT(upperBoundChanged()));
    shownVarLayout->addMultiCellWidget(upperBound, 3, 3, 2, 5);

    shownVarLayout->addMultiCellWidget(new QLabel("Variable selected:",
        shownVarBox, "varShownLabel"), 4, 4, 0, 1);
    shownVariable = new QLabel("default", shownVarBox, "shownVariable");
    shownVarLayout->addMultiCellWidget(shownVariable, 4, 4, 2, 4);

    QButtonGroup *prevVarOrNext = new QButtonGroup(shownVarBox, "prevVarOrNext");
    prevVarOrNext->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *prevOrNextLayout = new QHBoxLayout(prevVarOrNext);
    prevOrNextLayout->setSpacing(0);
    showPrevVariable = new QPushButton(prevVarOrNext);
    leftArrowBitmap = new QBitmap(8, 8, leftArrow);
    showPrevVariable->setPixmap(*leftArrowBitmap);
    prevOrNextLayout->addWidget(showPrevVariable);
    connect(showPrevVariable, SIGNAL(clicked()), this, SLOT(prevVarClicked()));
    showNextVariable = new QPushButton(prevVarOrNext);
    rightArrowBitmap = new QBitmap(8, 8, rightArrow);
    showNextVariable->setPixmap(*rightArrowBitmap);
    prevOrNextLayout->addWidget(showNextVariable);
    connect(showNextVariable, SIGNAL(clicked()), this, SLOT(nextVarClicked()));
    shownVarLayout->addWidget(prevVarOrNext, 4, 5);

    QGroupBox *varsInListBox = new QGroupBox(central, "varsInListBox");
    varsInListBox->setTitle("For the list of threshold variables");
    topLayout->addWidget(varsInListBox);

    QGridLayout *varsInListLayout = new QGridLayout(varsInListBox, 6, 3, 15, 5);

    varsInListLayout->addRowSpacing(0, 15);

    varsInListLayout->addMultiCellWidget(new QLabel("Output mesh is",
        varsInListBox, "outputMeshLabel"), 1, 1, 0, 1);
    outputMeshType = new QButtonGroup(varsInListBox, "outputMeshType");
    outputMeshType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *outputMeshTypeLayout = new QHBoxLayout(outputMeshType, 0, 10);
    QRadioButton *zonesFromInput =
        new QRadioButton("Zones from input", outputMeshType);
    outputMeshTypeLayout->addWidget(zonesFromInput);
    QRadioButton *pointMesh = new QRadioButton("Point mesh", outputMeshType);
    outputMeshTypeLayout->addWidget(pointMesh);
    connect(outputMeshType, SIGNAL(clicked(int)),
            this, SLOT(outputMeshTypeChanged(int)));
    varsInListLayout->addMultiCellWidget(outputMeshType, 1, 1, 2, 5);

    addVariable = new QvisVariableButton(false, true, true,
        QvisVariableButton::Scalars, varsInListBox, "addVariable");
    addVariable->setText("Add variable");
    addVariable->setChangeTextOnVariableChange(false);
    connect(addVariable, SIGNAL(activated(const QString &)),
            this, SLOT(variableAdded(const QString &)));
    varsInListLayout->addMultiCellWidget(addVariable, 2, 2, 0, 1);
    deleteVariable = new QvisVariableButton(false, true, true,
        QvisVariableButton::Scalars, varsInListBox, "deleteVariable");
    deleteVariable->setText("Delete variable");
    deleteVariable->setChangeTextOnVariableChange(false);
    connect(deleteVariable, SIGNAL(activated(const QString &)),
            this, SLOT(variableDeleted(const QString &)));
    varsInListLayout->addMultiCellWidget(deleteVariable, 2, 2, 2, 3);
    swapVariable = new QvisVariableButton(false, true, true,
        QvisVariableButton::Scalars, varsInListBox, "swapVariable");
    swapVariable->setText("Swap variable");
    swapVariable->setChangeTextOnVariableChange(false);
    connect(swapVariable, SIGNAL(activated(const QString &)),
            this, SLOT(variableSwapped(const QString &)));
    varsInListLayout->addMultiCellWidget(swapVariable, 2, 2, 4, 5);
}


// ****************************************************************************
// Method: QvisThresholdWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   
//   Hank Childs, Thu Sep 25 09:16:09 PDT 2003
//   Allow for "min" and "max" to be valid values in the lbound and ubound
//   windows.
//   
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Hank Childs, Thu Sep 15 15:31:34 PDT 2005
//   Add support for meshType.
//
//   Mark Blair, Tue Mar  7 13:25:00 PST 2006
//   Upgraded to support multiple threshold variables.
//
//   Mark Blair, Tue Aug  8 17:47:00 PDT 2006
//   Now accommodates an empty list of threshold variables.
//
//   Eric Brugger, Fri Sep  8 11:10:09 PDT 2006
//   Changed the way a conversion between std::string and QString was done
//   to eliminate a runtime link error on the ibm.
//
//   Mark Blair, Thu Sep 21 15:16:27 PDT 2006
//   Restore appropriate unapplied changes when Threshold attributes are
//   received from the viewer that do not yet know about these changes.
//
//   Mark Blair, Tue Oct  3 13:19:11 PDT 2006
//   Handle default scalar variable flag, display default variable as "default"
//   once again.
//
//   Mark Blair, Tue Oct 31 20:18:10 PST 2006
//   Second change back undone.  Too much conflict when multiple vis windows in
//   use.  All unapplied Threshold GUI changes are now lost if user dismisses
//   then reopens GUI, or if user moves an arrowhead in Extents tool of a second
//   vis window tool-locked to the Threshold operator's vis window.  Too bad.
//
// ****************************************************************************

void
QvisThresholdWindow::UpdateWindow(bool doAll)
{
    QString fieldString;
    std::string shownVarName = atts->GetShownVariable();
    bool varListIsEmpty =
        (shownVarName == std::string("(no variables in list)"));
    bool enableZonePortion = (!varListIsEmpty &&
        (atts->GetOutputMeshType() == ThresholdAttributes::InputZones));

    for (int attIndex = 0; attIndex < atts->NumAttributes(); attIndex++)
    {
        switch (attIndex)
        {
            case 0:  // outputMeshType
                outputMeshType->setButton((int)atts->GetOutputMeshType());

                break;

            case 1:  // listedVarNames
                break;

            case 2:  // shownVarPosition
                shownVariable->setText(QString(shownVarName.c_str()));

                break;

            case 3:  // zonePortions
                zonePortion->setButton((int)atts->GetZonePortion());

                zonePortion->setEnabled(enableZonePortion);
                zonePortionLabel->setEnabled(enableZonePortion);
                
                break;

            case 4:  // lowerBounds
                if (atts->GetLowerBound() < -9e+36)
                    fieldString = "min";
                else
                    fieldString.setNum(atts->GetLowerBound());
                    
                lowerBound->setText(fieldString);

                lowerBound->setReadOnly(varListIsEmpty);
                lowerBoundLabel->setEnabled(!varListIsEmpty);

                break;

            case 5:  // upperBounds
                if (atts->GetUpperBound() > +9e+36)
                    fieldString = "max";
                else
                    fieldString.setNum(atts->GetUpperBound());
                    
                upperBound->setText(fieldString);

                upperBound->setReadOnly(varListIsEmpty);
                upperBoundLabel->setEnabled(!varListIsEmpty);

                break;
                
            case 6:   // defaultVarName
                break;
                
            case 7:   // defaultVarIsScalar
                break;
        }
    }
}


// ****************************************************************************
// Method: QvisThresholdWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   Hank Childs, Thu Sep 25 09:16:09 PDT 2003
//   Allow for "min" and "max" to be valid values in the lbound and ubound
//   windows.
//
//   Jeremy Meredith, Wed Mar  3 16:02:43 PST 2004
//   Fixed a type with using "min".
//
//   Brad Whitlock, Fri Dec 10 09:43:19 PDT 2004
//   Removed code to get the variable.
//
//   Mark Blair, Tue Mar  7 13:25:00 PST 2006
//   Added support for multiple threshold variables.
//
//   Mark Blair, Wed Sep  6 19:33:00 PDT 2006
//   Removed problematic mechanism for accommodating ExtentsAttributes from
//   extents tool.
//
//   Mark Blair, Thu Sep 21 15:16:27 PDT 2006
//   Added support for input from Extents tool.  Save pending GUI changes.
//
//   Mark Blair, Tue Oct 31 20:18:10 PST 2006
//   Previous change undone.
//
// ****************************************************************************

void
QvisThresholdWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;
    
    // Do amount
    if ((which_widget == 0) || doAll) {
        // Nothing for amount
    }

    // Do lowerBound
    if ((which_widget == 1) || doAll) {
        temp = lowerBound->displayText().simplifyWhiteSpace();

        if (temp.latin1() == QString("min")) atts->ChangeLowerBound(-1e+37);
        else {
            okay = !temp.isEmpty();

            if (okay)
            {
                double val = temp.toDouble(&okay);
                atts->ChangeLowerBound(val);
            }

            if (!okay)
            {
                msg.sprintf ("The value of lbound was invalid. "
                    "Resetting to the last good value of %g.",
                    atts->GetLowerBound());
                Message(msg);
                atts->ChangeLowerBound(atts->GetLowerBound());
            }
        }
    }

    // Do upperbound
    if ((which_widget == 2) || doAll) {
        temp = upperBound->displayText().simplifyWhiteSpace();

        if (temp.latin1() == QString("max")) atts->ChangeUpperBound(+1e+37);
        else {
            okay = !temp.isEmpty();

            if (okay)
            {
                double val = temp.toDouble(&okay);
                atts->ChangeUpperBound(val);
            }
    
            if (!okay)
            {
                msg.sprintf("The value of ubound was invalid. "
                    "Resetting to the last good value of %g.",
                    atts->GetUpperBound());
                Message(msg);
                atts->ChangeUpperBound(atts->GetUpperBound());
            }
        }
    }
}


// ****************************************************************************
//
// Qt Slot functions
//
// ****************************************************************************


void
QvisThresholdWindow::apply()
{
    QvisOperatorWindow::apply();
}


void
QvisThresholdWindow::outputMeshTypeChanged(int buttonID)
{
    ThresholdAttributes::OutputMeshType newOutputMeshType =
        ThresholdAttributes::OutputMeshType(buttonID);

    if (newOutputMeshType != atts->GetOutputMeshType())
    {
        atts->SetOutputMeshType(newOutputMeshType);
        
        bool enableZonePortion =
            ((newOutputMeshType == ThresholdAttributes::InputZones) &&
             (atts->GetShownVariable() != std::string("(no variables in list)")));
             
        zonePortionLabel->setEnabled(enableZonePortion);
        zonePortion->setEnabled(enableZonePortion);
    
        Apply();
    }
}


void
QvisThresholdWindow::zonePortionChanged(int buttonID)
{
    ThresholdAttributes::ZonePortion newZonePortion =
        ThresholdAttributes::ZonePortion(buttonID);

    if (newZonePortion != atts->GetZonePortion())
    {
        atts->ChangeZonePortion(newZonePortion);
        Apply();
    }
}


void
QvisThresholdWindow::lowerBoundChanged()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisThresholdWindow::upperBoundChanged()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisThresholdWindow::prevVarClicked()
{
    atts->ShowPreviousVariable();
    UpdateShownFields();
}


void
QvisThresholdWindow::nextVarClicked()
{
    atts->ShowNextVariable();
    UpdateShownFields();
}


void
QvisThresholdWindow::variableAdded(const QString &variableToAdd)
{
    atts->InsertVariable(variableToAdd.latin1());
    UpdateShownFields();
}


void
QvisThresholdWindow::variableDeleted(const QString &variableToDelete)
{
    atts->DeleteVariable(variableToDelete.latin1());
    UpdateShownFields();
}


void
QvisThresholdWindow::variableSwapped(const QString &variableToSwapIn)
{
    atts->SwapVariable(variableToSwapIn.latin1());
    UpdateShownFields();
}


// ****************************************************************************
// Method: QvisThresholdWindow::UpdateShownFields
//
// Purpose: Updates all widgets that display data for the variable whose data
//          is currently being shown, which is one variable in the list of
//          currently selected threshold variables.
//
// Programmer: Mark Blair
// Creation:   Tue Mar  7 13:25:00 PST 2006
//
// Modifications:
//   
//   Mark Blair, Tue Aug  8 17:47:00 PDT 2006
//   Now accommodates an empty list of threshold variables.
//
//   Mark Blair, Wed Sep  6 19:33:00 PDT 2006
//   Removed problematic mechanism for accommodating ExtentsAttributes from
//   extents tool.
//
//   Eric Brugger, Fri Sep  8 11:10:09 PDT 2006
//   Changed the way a conversion between std::string and QString was done
//   to eliminate a runtime link error on the ibm.
//
//   Mark Blair, Thu Sep 21 15:16:27 PDT 2006
//   Added support for input from Extents tool.  Save pending GUI changes.
//
//   Mark Blair, Tue Oct  3 13:19:11 PDT 2006
//   Display default variable as "default" once again.
//
//   Mark Blair, Tue Oct 31 20:18:10 PST 2006
//   Second change back undone.
//
// ****************************************************************************

void
QvisThresholdWindow::UpdateShownFields()
{
    QString fieldString;
    std::string shownVarName = atts->GetShownVariable();
    bool varListIsEmpty =
        (shownVarName == std::string("(no variables in list)"));
    bool enableZonePortion = (!varListIsEmpty &&
        (atts->GetOutputMeshType() == ThresholdAttributes::InputZones));
    
    shownVariable->setText(QString(shownVarName.c_str()));

    zonePortion->setButton((int)atts->GetZonePortion());

    zonePortion->setEnabled(enableZonePortion);
    zonePortionLabel->setEnabled(enableZonePortion);

    if (atts->GetLowerBound() < -9e+36)
        fieldString = "min";
    else
        fieldString.setNum(atts->GetLowerBound());

    lowerBound->setText(fieldString);

    lowerBound->setReadOnly(varListIsEmpty);
    lowerBoundLabel->setEnabled(!varListIsEmpty);

    if (atts->GetUpperBound() > +9e+36)
        fieldString = "max";
    else
        fieldString.setNum(atts->GetUpperBound());

    upperBound->setText(fieldString);

    upperBound->setReadOnly(varListIsEmpty);
    upperBoundLabel->setEnabled(!varListIsEmpty);
    
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisThresholdWindow::RestoreAppropriateUnappliedAttributes
//
// Purpose: All changes the user has made in the Threshold GUI since its Apply
//          button was last clicked will be restored, EXCEPT for minima and
//          maxima of variables that were not newly selected since then.  Those
//          values are left untouched because they may have been changed more
//          recently by an Extents tool in a different vis window whose tools
//          are locked to the tools in this Threshold operator's vis window.
//
// Programmer: Mark Blair
// Creation:   Thu Sep 21 15:16:27 PDT 2006
//
// Modifications:
//
//   Mark Blair, Tue Oct  3 13:19:11 PDT 2006
//   Handles "default" as a threshold variable name.
//
//   Mark Blair, Thu Oct  5 18:24:43 PDT 2006
//   Do not restore if current applied attributes look like default attributes;
//   probably reinitializing.
//
//   Mark Blair, Tue Oct 31 20:18:10 PST 2006
//   Method no longer used.  Too much conflict when multiple vis windows in
//   use.  All unapplied Threshold GUI changes are now lost if user dismisses
//   then reopens GUI, or if user moves an arrowhead in Extents tool of a second
//   vis window tool-locked to the Threshold operator's vis window.  Too bad.
//
// ****************************************************************************

/*
void
QvisThresholdWindow::RestoreAppropriateUnappliedAttributes()
{
    stringVector viewerVarNames  = atts->GetListedVarNames();
    doubleVector viewerVarMinima = atts->GetLowerBounds();
    doubleVector viewerVarMaxima = atts->GetUpperBounds();

    stringVector guiVarNames  = guiVarAtts->GetListedVarNames();
    doubleVector guiVarMinima = guiVarAtts->GetLowerBounds();
    doubleVector guiVarMaxima = guiVarAtts->GetUpperBounds();
    
    int viewerVarCount = viewerVarNames.size();
    int guiVarCount    = guiVarNames.size();
    int viewerVarNum, guiVarNum;
    std::string viewerVarName;
    
    for (viewerVarNum = 0; viewerVarNum < viewerVarCount; viewerVarNum++)
    {
        if (viewerVarMinima[viewerVarNum] > -9e+36) break;
        if (viewerVarMaxima[viewerVarNum] < +9e+36) break;
    }
    
    if (viewerVarNum >= viewerVarCount)
    {
        *guiVarAtts = *atts;
        return;
    }
    
    for (viewerVarNum = 0; viewerVarNum < viewerVarCount; viewerVarNum++)
    {
        viewerVarName = viewerVarNames[viewerVarNum];
        
        for (guiVarNum = 0; guiVarNum < guiVarCount; guiVarNum++)
        {
            if (guiVarNames[guiVarNum] == viewerVarName) break;
            
            if (viewerVarName == std::string("default"))
            {
                if (guiVarNames[guiVarNum] == atts->GetDefaultVarName()) break;
            }
        }
        
        if (guiVarNum < guiVarCount)
        {
            guiVarMinima[guiVarNum] = viewerVarMinima[viewerVarNum];
            guiVarMaxima[guiVarNum] = viewerVarMaxima[viewerVarNum];
        }
    }
    
    guiVarAtts->SetLowerBounds(guiVarMinima);
    guiVarAtts->SetUpperBounds(guiVarMaxima);
    
    *atts = *guiVarAtts;
}
*/
