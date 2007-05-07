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

#include <qtable.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
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
//   Mark Blair, Tue Apr 17 16:24:42 PDT 2007
//   Rewritten to support new Threshold GUI.
//
// ****************************************************************************

void
QvisThresholdWindow::CreateWindowContents()
{
    static unsigned char leftArrow[8] =
        { 0x00, 0x01, 0x07, 0x1f, 0x7f, 0x1f, 0x07, 0x01 };
    static unsigned char rightArrow[8] =
        { 0x00, 0x80, 0xe0, 0xf8, 0xfe, 0xf8, 0xe0, 0x80 };

    QGroupBox *threshVarsBox = new QGroupBox(central, "threshVarsBox");
    threshVarsBox->setTitle("For individual threshold variables");
    topLayout->addWidget(threshVarsBox);

    QGridLayout *threshVarsLayout = new QGridLayout(threshVarsBox, 2, 4, 15, 5);

    threshVarsLayout->addRowSpacing(0, 15);

    threshVarsList = new QTable(0, 4, threshVarsBox, "threshVarsList");
    threshVarsLayout->addMultiCellWidget(threshVarsList, 1, 1, 0, 1);

    threshVarsList->setSelectionMode(QTable::Single);
    threshVarsList->setLeftMargin(0);
    threshVarsList->setColumnReadOnly(0, true);
    threshVarsList->setColumnWidth(0, 170);
    threshVarsList->setColumnWidth(1, 110);
    threshVarsList->setColumnWidth(2, 110);
    threshVarsList->setColumnWidth(3, 110);

    QStringList columnLabels;
    columnLabels << "Variable" << "Lower bound" << "Upper bound" << "Show zone if";
    threshVarsList->setColumnLabels(columnLabels);
    
    threshVarsLayout->addRowSpacing(2, 10);

    QvisVariableButton *addVarToList = new QvisVariableButton(false, true, true,
        QvisVariableButton::Scalars, threshVarsBox, "addVarToList");
    addVarToList->setText("Add variable");
    addVarToList->setChangeTextOnVariableChange(false);
    connect(addVarToList, SIGNAL(activated(const QString &)),
            this, SLOT(variableAddedToList(const QString &)));
    threshVarsLayout->addWidget(addVarToList, 3, 0);
    QPushButton *deleteSelectedVar = new QPushButton(
        QString("Delete selected variable"), threshVarsBox, "deleteSelectedVar");
    connect(deleteSelectedVar, SIGNAL(clicked()),
        this, SLOT(selectedVariableDeleted()));
    threshVarsLayout->addWidget(deleteSelectedVar, 3, 1);

    QGroupBox *forAllVarsBox = new QGroupBox(central, "forAllVarsBox");
    forAllVarsBox->setTitle("For all threshold variables");
    topLayout->addWidget(forAllVarsBox);

    QGridLayout *forAllVarsLayout = new QGridLayout(forAllVarsBox, 6, 2, 15, 5);

    forAllVarsLayout->addRowSpacing(0, 15);

    forAllVarsLayout->addMultiCellWidget(new QLabel("Output mesh is",
        forAllVarsBox, "outputMeshLabel"), 1, 1, 0, 1);
    outputMeshType = new QButtonGroup(forAllVarsBox, "outputMeshType");
    outputMeshType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *outputMeshTypeLayout = new QHBoxLayout(outputMeshType, 0, 10);
    QRadioButton *zonesFromInput =
        new QRadioButton("Zones from input", outputMeshType);
    outputMeshTypeLayout->addWidget(zonesFromInput);
    QRadioButton *pointMesh = new QRadioButton("Point mesh", outputMeshType);
    outputMeshTypeLayout->addWidget(pointMesh);
    connect(outputMeshType, SIGNAL(clicked(int)),
            this, SLOT(outputMeshTypeChanged(int)));
    forAllVarsLayout->addMultiCellWidget(outputMeshType, 1, 1, 2, 5);
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
//   Mark Blair, Tue Mar 13 19:51:29 PDT 2007
//   Now forces attribute consistency if inconsistent, which can occur, for
//   instance, if user specifies an invalid attribute combination in the CLI.
//
//   Mark Blair, Tue Apr 17 16:24:42 PDT 2007
//   Rewritten to support new Threshold GUI.
//
// ****************************************************************************

void
QvisThresholdWindow::UpdateWindow(bool doAll)
{
    intVector curZonePortions;
    doubleVector curBounds;
    QComboTableItem *zoneShowSelector;
    int varNum;
    QString fieldString;

    atts->ForceAttributeConsistency();
    
    for (int attIndex = 0; attIndex < atts->NumAttributes(); attIndex++)
    {
        switch (attIndex)
        {
            case 0:  // outputMeshType
                outputMeshType->setButton((int)atts->GetOutputMeshType());

                break;

            case 1:  // listedVarNames
                PopulateThresholdVariablesList();

                break;

            case 2:  // shownVarPosition
                break;

            case 3:  // zonePortions
                curZonePortions = atts->GetZonePortions();
                
                for (varNum = 0; varNum < curZonePortions.size(); varNum++ )
                {
                    zoneShowSelector =
                       (QComboTableItem *)threshVarsList->item(varNum, 3);
                    zoneShowSelector->setCurrentItem(curZonePortions[varNum]);
                }
                
                threshVarsList->setColumnReadOnly(3,
                    (atts->GetOutputMeshType() != ThresholdAttributes::InputZones));
                
                break;

            case 4:  // lowerBounds
                curBounds = atts->GetLowerBounds();
                
                for (varNum = 0; varNum < curBounds.size(); varNum++ )
                {
                    if (curBounds[varNum] < -9e+36)
                        fieldString = "min";
                    else
                        fieldString.setNum(curBounds[varNum]);

                    threshVarsList->setText(varNum, 1, fieldString);
                }

                break;

            case 5:  // upperBounds
                curBounds = atts->GetUpperBounds();
                
                for (varNum = 0; varNum < curBounds.size(); varNum++ )
                {
                    if (curBounds[varNum] > +9e+36)
                        fieldString = "max";
                    else
                        fieldString.setNum(curBounds[varNum]);

                    threshVarsList->setText(varNum, 2, fieldString);
                }

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
//   Mark Blair, Tue Apr 17 16:24:42 PDT 2007
//   Rewritten to support new Threshold GUI.
//
// ****************************************************************************

void
QvisThresholdWindow::GetCurrentValues(int which_widget)
{
    if (which_widget != -1) return;
    
    stringVector curVarNames;
    doubleVector curLowerBounds;
    doubleVector curUpperBounds;
    intVector    curZonePortions;

    if (threshVarsList->numRows() != guiFullVarNames.size()) // Just in case
    {
        debug3 << "QTW/GCV/1: Threshold GUI out of sync with internal data."
               << endl;
               
        curVarNames.push_back(std::string("default"));
        curLowerBounds.push_back(-1e+37);
        curUpperBounds.push_back(+1e+37);
        curZonePortions.push_back((int)ThresholdAttributes::PartOfZone);
    
        atts->SetListedVarNames(curVarNames);
        atts->SetLowerBounds(curLowerBounds);
        atts->SetUpperBounds(curUpperBounds);
        atts->SetZonePortions(curZonePortions);
        
        threshVarsList->setNumRows(0);
        AddNewRowToVariablesList(QString("default"));

        guiFullVarNames.clear();
        guiFullVarNames.push_back(std::string("default"));
        
        return;
    }

    int listRowCount = threshVarsList->numRows();
    bool valueIsValid;
    double lowerBound, upperBound, bound;
    QString lowerBoundText, upperBoundText, errMsg;
    QComboTableItem *zoneShowSelector;
    
    for (int rowNum = 0; rowNum < listRowCount; rowNum++ )
    {
        curVarNames.push_back(guiFullVarNames[rowNum]);
        
        lowerBoundText = threshVarsList->text(rowNum,1).simplifyWhiteSpace();
        upperBoundText = threshVarsList->text(rowNum,2).simplifyWhiteSpace();
        
        if (lowerBoundText == QString("min")) lowerBound = -1e+37;
        else
        {
            valueIsValid = !lowerBoundText.isEmpty();
            if (valueIsValid) lowerBound = lowerBoundText.toDouble(&valueIsValid);
            
            if (!valueIsValid)
            {
                errMsg.sprintf ("Invalid lower bound; will reset to min.");
                Message(errMsg);

                lowerBound = -1e+37;
            }
        }

        if (upperBoundText == QString("max")) upperBound = +1e+37;
        else
        {
            valueIsValid = !upperBoundText.isEmpty();
            if (valueIsValid) upperBound = upperBoundText.toDouble(&valueIsValid);
            
            if (!valueIsValid)
            {
                errMsg.sprintf("Invalid upper bound; will reset to max.");
                Message(errMsg);

                upperBound = +1e+37;
            }
        }
        
        if (lowerBound > upperBound)
        {
            errMsg.sprintf("Lower bound exceeds upper bound; will reverse them.");
            Message(errMsg);
            
            bound = lowerBound; lowerBound = upperBound; upperBound = bound;
        }
        
        curLowerBounds.push_back(lowerBound);
        curUpperBounds.push_back(upperBound);
        
        zoneShowSelector = (QComboTableItem *)threshVarsList->item(rowNum, 3);
        curZonePortions.push_back(zoneShowSelector->currentItem());
    }
    
    atts->SetListedVarNames(curVarNames);
    atts->SetLowerBounds(curLowerBounds);
    atts->SetUpperBounds(curUpperBounds);
    atts->SetZonePortions(curZonePortions);
}


// ****************************************************************************
//
// Qt Slot functions
//
// ****************************************************************************

void
QvisThresholdWindow::variableAddedToList(const QString &variableToAdd)
{
    if (threshVarsList->numRows() != guiFullVarNames.size())
    {
        debug3 << "QTW/vATL/1: Threshold GUI out of sync with internal data."
               << endl;
        return;
    }

    for (int varNum = 0; varNum < guiFullVarNames.size(); varNum++ )
    {
        if (guiFullVarNames[varNum] == variableToAdd) return;
    }
    
    char listVarText[21];
    
    MakeDisplayableVariableNameText(listVarText, variableToAdd, 20);
    AddNewRowToVariablesList(QString(listVarText));
    
    guiFullVarNames.push_back(variableToAdd);
}


void
QvisThresholdWindow::selectedVariableDeleted()
{
    if (threshVarsList->numRows() != guiFullVarNames.size())
    {
        debug3 << "QTW/sVD/1: Threshold GUI out of sync with internal data."
               << endl;
        return;
    }

    if (guiFullVarNames.size() == 0)
        return;
        
    int selectedVarNum = threshVarsList->currentRow();

    threshVarsList->removeRow(selectedVarNum);

    guiFullVarNames.erase(guiFullVarNames.begin() + selectedVarNum);
}


void
QvisThresholdWindow::outputMeshTypeChanged(int buttonID)
{
    ThresholdAttributes::OutputMeshType newOutputMeshType =
        ThresholdAttributes::OutputMeshType(buttonID);

    if (newOutputMeshType != atts->GetOutputMeshType())
    {
        threshVarsList->setColumnReadOnly(3,
            (newOutputMeshType != ThresholdAttributes::InputZones));
        atts->SetOutputMeshType(newOutputMeshType);
    }
}


void
QvisThresholdWindow::apply()
{
    threshVarsList->setCurrentCell(threshVarsList->currentRow(), 0);
    QvisOperatorWindow::apply();
}


// ****************************************************************************
// Method: QvisThresholdWindow::PopulateThresholdVariableTable
//
// Purpose: Adds a new row to the table of current threshold variables for each
//          variable not already in the table and deletes any row corresponding
//          to a variable no longer in effect.
//
// Programmer: Mark Blair
// Creation:   Tue Apr 10 17:59:47 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisThresholdWindow::PopulateThresholdVariablesList()
{
    stringVector curVarNames = atts->GetListedVarNames();
    std::string curVarName, guiVarName;
    int curVarCount = curVarNames.size();
    int guiVarCount = guiFullVarNames.size();
    int guiVarNum = 0;
    int curVarNum;
    
    char listVarText[21];
    
    if (threshVarsList->numRows() != guiVarCount)
    {
        debug3 << "QTW/PTVL/1: Threshold GUI out of sync with internal data."
               << endl;
        return;
    }

    while (guiVarNum < guiVarCount)
    {
        guiVarName = guiFullVarNames[guiVarNum];
        
        for (curVarNum = 0; curVarNum < curVarCount; curVarNum++ )
        {
            if (curVarNames[curVarNum] == guiVarName) break;
        }
        
        if (curVarNum < curVarCount)
            guiVarNum++;
        else
        {
            threshVarsList->removeRow(guiVarNum);
            guiFullVarNames.erase(guiFullVarNames.begin() + guiVarNum);

            guiVarCount--;
        }
    }

    for (curVarNum = 0; curVarNum < curVarCount; curVarNum++ )
    {
        curVarName = curVarNames[curVarNum];

        for (guiVarNum = 0; guiVarNum < guiVarCount; guiVarNum++ )
        {
            if (guiFullVarNames[guiVarNum] == curVarName) break;
        }
        
        if (guiVarNum >= guiVarCount)   // guiVarCount is NOT incremented.
        {
            MakeDisplayableVariableNameText(listVarText, curVarName, 20);
            AddNewRowToVariablesList(QString(listVarText));

            guiFullVarNames.push_back(curVarName);
        }
    }
}


// ****************************************************************************
// Method: QvisThresholdWindow::AddNewRowToVariablesList
//
// Purpose: Adds a new row for a selected variable, or for no variable, to the
//          list of current threshold variables and their attributes.
//
// Programmer: Mark Blair
// Creation:   Tue Apr 10 17:59:47 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisThresholdWindow::AddNewRowToVariablesList(const QString &listVarName)
{
    int listRowCount = threshVarsList->numRows();

    threshVarsList->setNumRows(listRowCount + 1);

    threshVarsList->setText(listRowCount, 0, listVarName);
    threshVarsList->setText(listRowCount, 1, QString("min"));
    threshVarsList->setText(listRowCount, 2, QString("max"));

    QComboTableItem *zoneShowSelector;
    QStringList zoneShowLabels;
    zoneShowLabels << "All in range" << "Part in range";
    
    zoneShowSelector = new QComboTableItem(threshVarsList, zoneShowLabels);
    zoneShowSelector->setCurrentItem(1);
    threshVarsList->setItem(listRowCount, 3, zoneShowSelector);
    
    if (listRowCount == 0) threshVarsList->setCurrentCell(0, 0);
}


// *****************************************************************************
//  Method: QvisThresholdWindow::MakeDisplayableVariableNameText
//
//  Purpose: Creates a displayable version of a threshold variable name.  Long
//           variable names and compound names are shortened in a meaningful way.
//
//  Programmer: Mark Blair
//  Creation:   Tue Apr 10 17:59:47 PDT 2007
//
//  Modifications:
//
// *****************************************************************************

void QvisThresholdWindow::MakeDisplayableVariableNameText(
    char displayVarText[], const std::string &variableName, int maxDisplayChars)
{
    int rawVarNameLen;
    char rawVarName[121];
    
    strncpy(rawVarName, variableName.c_str(), 120);
    
    if ((rawVarNameLen = strlen(rawVarName)) <= maxDisplayChars)
        strcpy(displayVarText, rawVarName);
    else
    {
        rawVarName[maxDisplayChars-3] = '\0';
        sprintf(displayVarText,"%s..%s",rawVarName,&rawVarName[rawVarNameLen-2]);
    }
}
