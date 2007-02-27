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

#include "QvisMoleculePlotWindow.h"

#include <MoleculeAttributes.h>
#include <ViewerProxy.h>

#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcombobox.h>
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
// Method: QvisMoleculePlotWindow::QvisMoleculePlotWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Wed Mar 22 16:56:07 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisMoleculePlotWindow::QvisMoleculePlotWindow(const int type,
                         MoleculeAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad)
{
    plotType = type;
    atts = subj;
}


// ****************************************************************************
// Method: QvisMoleculePlotWindow::~QvisMoleculePlotWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Wed Mar 22 16:56:07 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisMoleculePlotWindow::~QvisMoleculePlotWindow()
{
}


// ****************************************************************************
// Method: QvisMoleculePlotWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Jeremy Meredith
// Creation:   March 23, 2006
//
// Modifications:
//   Brad Whitlock, Mon Mar 27 17:35:07 PST 2006
//   Added sphere imposters.
//
// ****************************************************************************

void
QvisMoleculePlotWindow::CreateWindowContents()
{
    QTabWidget *topTab = new QTabWidget(central, "topTab");
    topLayout->addWidget(topTab);


    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    //                            Atoms Tab
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    QGroupBox *atomsGroup = new QGroupBox(central, "atomsGroup");
    atomsGroup->setFrameStyle(QFrame::NoFrame);
    topTab->addTab(atomsGroup, "Atoms");

    QGridLayout *atomsLayout = new QGridLayout(atomsGroup, 5,3,  5,10, "atomsLayout");

    drawAtomsAsLabel = new QLabel("Draw atoms as", atomsGroup, "drawAtomsAsLabel");
    atomsLayout->addMultiCellWidget(drawAtomsAsLabel, 0,0, 0,1);
    drawAtomsAs = new QComboBox(atomsGroup, "drawAtomsAs");
    drawAtomsAs->insertItem("None");
    drawAtomsAs->insertItem("Spheres");
    drawAtomsAs->insertItem("Sphere imposters");
    connect(drawAtomsAs, SIGNAL(activated(int)),
            this, SLOT(drawAtomsAsChanged(int)));
    atomsLayout->addWidget(drawAtomsAs, 0,2);
    

    atomSphereQualityLabel = new QLabel("Atom sphere quality", atomsGroup, "atomSphereQualityLabel");
    atomsLayout->addMultiCellWidget(atomSphereQualityLabel,1,1, 0,1);
    atomSphereQuality = new QComboBox(atomsGroup, "atomSphereQuality");
    atomSphereQuality->insertItem("Low");
    atomSphereQuality->insertItem("Medium");
    atomSphereQuality->insertItem("High");
    atomSphereQuality->insertItem("Super");
    connect(atomSphereQuality, SIGNAL(activated(int)),
            this, SLOT(atomSphereQualityChanged(int)));
    atomsLayout->addWidget(atomSphereQuality, 1,2);

    scaleRadiusByLabel = new QLabel("Radius based on", atomsGroup, "scaleRadiusByLabel");
    atomsLayout->addMultiCellWidget(scaleRadiusByLabel, 2,2, 0,1);
    scaleRadiusBy = new QComboBox(atomsGroup, "scaleRadiusBy");
    scaleRadiusBy->insertItem("Fixed value");
    scaleRadiusBy->insertItem("Covalent radius");
    scaleRadiusBy->insertItem("Atomic radius");
    scaleRadiusBy->insertItem("Scalar variable");
    connect(scaleRadiusBy, SIGNAL(activated(int)),
            this, SLOT(scaleRadiusByChanged(int)));
    atomsLayout->addWidget(scaleRadiusBy, 2,2);

    radiusVariableLabel = new QLabel("Variable for atom radius", atomsGroup, "radiusVariableLabel");
    atomsLayout->addWidget(radiusVariableLabel,3,1);
    int radiusVariableMask = QvisVariableButton::Scalars;
    radiusVariable = new QvisVariableButton(true, true, true, radiusVariableMask, atomsGroup, "radiusVariable");
    connect(radiusVariable, SIGNAL(activated(const QString&)),
            this, SLOT(radiusVariableChanged(const QString&)));
    atomsLayout->addWidget(radiusVariable, 3,2);

    radiusScaleFactorLabel = new QLabel("Atom radius scale factor", atomsGroup, "radiusScaleFactorLabel");
    atomsLayout->addWidget(radiusScaleFactorLabel,4,1);
    radiusScaleFactor = new QLineEdit(atomsGroup, "radiusScaleFactor");
    connect(radiusScaleFactor, SIGNAL(returnPressed()),
            this, SLOT(radiusScaleFactorProcessText()));
    atomsLayout->addWidget(radiusScaleFactor, 4,2);

    radiusFixedLabel = new QLabel("Fixed atom radius", atomsGroup, "radiusFixedLabel");
    atomsLayout->addWidget(radiusFixedLabel,5,1);
    radiusFixed = new QLineEdit(atomsGroup, "radiusFixed");
    connect(radiusFixed, SIGNAL(returnPressed()),
            this, SLOT(radiusFixedProcessText()));
    atomsLayout->addWidget(radiusFixed, 5,2);

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    //                            Bonds Tab
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    QGroupBox *bondsGroup = new QGroupBox(central, "bondsGroup");
    bondsGroup->setFrameStyle(QFrame::NoFrame);
    topTab->addTab(bondsGroup, "Bonds");

    QGridLayout *bondsLayout = new QGridLayout(bondsGroup, 6,3,  5,10, "bondsLayout");

    int row = 0;
    drawBondsAsLabel = new QLabel("Draw bonds as", bondsGroup, "drawBondsAsLabel");
    bondsLayout->addMultiCellWidget(drawBondsAsLabel, row,row, 0,1);
    drawBondsAs = new QComboBox(bondsGroup, "drawBondsAs");
    drawBondsAs->insertItem("None");
    drawBondsAs->insertItem("Lines");
    drawBondsAs->insertItem("Cylinders");
    connect(drawBondsAs, SIGNAL(activated(int)),
            this, SLOT(drawBondsAsChanged(int)));
    bondsLayout->addWidget(drawBondsAs, row,2);
    row++;

    bondCylinderQualityLabel = new QLabel("Bond cylinder quality", bondsGroup, "bondCylinderQualityLabel");
    bondsLayout->addWidget(bondCylinderQualityLabel, row,1);
    bondCylinderQuality = new QComboBox(bondsGroup, "bondCylinderQuality");
    bondCylinderQuality->insertItem("Low");
    bondCylinderQuality->insertItem("Medium");
    bondCylinderQuality->insertItem("High");
    bondCylinderQuality->insertItem("Super");
    connect(bondCylinderQuality, SIGNAL(activated(int)),
            this, SLOT(bondCylinderQualityChanged(int)));
    bondsLayout->addWidget(bondCylinderQuality, row,2);
    row++;

    bondRadiusLabel = new QLabel("Bond radius", bondsGroup, "bondRadiusLabel");
    bondsLayout->addWidget(bondRadiusLabel, row,1);
    bondRadius = new QLineEdit(bondsGroup, "bondRadius");
    connect(bondRadius, SIGNAL(returnPressed()),
            this, SLOT(bondRadiusProcessText()));
    bondsLayout->addWidget(bondRadius, row,2);
    row++;

    bondLineWidthLabel = new QLabel("Bond line width", bondsGroup, "bondLineWidthLabel");
    bondsLayout->addWidget(bondLineWidthLabel, row,1);
    bondLineWidth = new QvisLineWidthWidget(0, bondsGroup, "bondLineWidth");
    connect(bondLineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(bondLineWidthChanged(int)));
    bondsLayout->addWidget(bondLineWidth, row,2);
    row++;

    bondLineStyleLabel = new QLabel("Bond line style", bondsGroup, "bondLineStyleLabel");
    bondsLayout->addWidget(bondLineStyleLabel, row,1);
    bondLineStyle = new QvisLineStyleWidget(0, bondsGroup, "bondLineStyle");
    connect(bondLineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(bondLineStyleChanged(int)));
    bondsLayout->addWidget(bondLineStyle, row,2);
    row++;

    colorBondsLabel = new QLabel("Color bonds by", bondsGroup, "colorBondsLabel");
    bondsLayout->addMultiCellWidget(colorBondsLabel, row,row, 0,1);
 
    colorBonds = new QButtonGroup(NULL, "colorBonds");
    colorBonds->setFrameStyle(QFrame::NoFrame);
    QGridLayout *colorBondsLayout = new QGridLayout(2,2);
    colorBondsLayout->setSpacing(10);
    QRadioButton *colorBondsBondColoringModeColorByAtom = new QRadioButton("Adjacent atom color", bondsGroup);
    colorBondsLayout->addMultiCellWidget(colorBondsBondColoringModeColorByAtom, 0,0, 0,1);
    QRadioButton *colorBondsBondColoringModeSingleColor = new QRadioButton("Single color", bondsGroup);
    colorBondsLayout->addWidget(colorBondsBondColoringModeSingleColor, 1,0);
    colorBonds->insert(colorBondsBondColoringModeColorByAtom, 0);
    colorBonds->insert(colorBondsBondColoringModeSingleColor, 1);
    connect(colorBonds, SIGNAL(clicked(int)),
            this, SLOT(colorBondsChanged(int)));
    bondSingleColor = new QvisColorButton(bondsGroup, "bondSingleColor");
    connect(bondSingleColor, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(bondSingleColorChanged(const QColor&)));
    colorBondsLayout->addWidget(bondSingleColor, 1,1);
    bondsLayout->addLayout(colorBondsLayout, row,2);
    row++;



    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    //                            Colors Tab
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    QGroupBox *colorsGroup = new QGroupBox(central, "colorsGroup");
    colorsGroup->setFrameStyle(QFrame::NoFrame);
    topTab->addTab(colorsGroup, "Colors");

    QVBoxLayout *colorsLayout = new QVBoxLayout(colorsGroup, 10, 2);
    

    QGroupBox *discreteGroup = new QGroupBox(colorsGroup, "discreteGroup");
    discreteGroup->setTitle("Discrete colors");
    colorsLayout->addWidget(discreteGroup);
    QVBoxLayout *discreteTopLayout = new QVBoxLayout(discreteGroup, 10, 2);
    discreteTopLayout->addSpacing(15);

    QLabel *discreteColorTableLabel = new QLabel("Color table for:", discreteGroup, "discreteColorTableLabel");
    discreteTopLayout->addWidget(discreteColorTableLabel);

    QHBoxLayout *discreteColorTablesLayout = new QHBoxLayout(discreteTopLayout, 10);
    discreteColorTablesLayout->addSpacing(15);

    QGridLayout *discreteLayout = new QGridLayout(discreteColorTablesLayout, 3,2,  10, "colorsLayout");
    discreteLayout->setSpacing(5);

    elementColorTableLabel = new QLabel("Element types", discreteGroup, "elementColorTableLabel");
    discreteLayout->addWidget(elementColorTableLabel, 0,0);
    elementColorTable = new QvisColorTableButton(discreteGroup, "elementColorTable");
    connect(elementColorTable, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(elementColorTableChanged(bool, const QString&)));
    discreteLayout->addWidget(elementColorTable, 0,1);

    residueTypeColorTableLabel = new QLabel("Residue types", discreteGroup, "residueTypeColorTableLabel");
    discreteLayout->addWidget(residueTypeColorTableLabel, 1,0);
    residueTypeColorTable = new QvisColorTableButton(discreteGroup, "residueTypeColorTable");
    connect(residueTypeColorTable, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(residueTypeColorTableChanged(bool, const QString&)));
    discreteLayout->addWidget(residueTypeColorTable, 1,1);

    residueSequenceColorTableLabel = new QLabel("Other discrete fields", discreteGroup, "residueSequenceColorTableLabel");
    discreteLayout->addWidget(residueSequenceColorTableLabel, 2,0);
    residueSequenceColorTable = new QvisColorTableButton(discreteGroup, "residueSequenceColorTable");
    connect(residueSequenceColorTable, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(residueSequenceColorTableChanged(bool, const QString&)));
    discreteLayout->addWidget(residueSequenceColorTable, 2,1);

    QGroupBox *continuousGroup = new QGroupBox(colorsGroup, "continuousGroup");
    continuousGroup->setTitle("Continuous colors");
    colorsLayout->addWidget(continuousGroup);
    QVBoxLayout *continuousTopLayout = new QVBoxLayout(continuousGroup, 10, 2);
    continuousTopLayout->addSpacing(15);
    QGridLayout *continuousLayout = new QGridLayout(continuousTopLayout, 3,2,  10, "colorsLayout");
    continuousLayout->setSpacing(5);


    continuousColorTableLabel = new QLabel("Color table for scalars", continuousGroup, "continuousColorTableLabel");
    continuousLayout->addMultiCellWidget(continuousColorTableLabel, 0,0, 0,0);
    continuousColorTable = new QvisColorTableButton(continuousGroup, "continuousColorTable");
    connect(continuousColorTable, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(continuousColorTableChanged(bool, const QString&)));
    continuousLayout->addWidget(continuousColorTable, 0,1);

    minFlag = new QCheckBox("Clamp minimum", continuousGroup, "minFlag");
    connect(minFlag, SIGNAL(toggled(bool)),
            this, SLOT(minFlagChanged(bool)));
    continuousLayout->addWidget(minFlag, 1,0);

    scalarMin = new QLineEdit(continuousGroup, "scalarMin");
    connect(scalarMin, SIGNAL(returnPressed()),
            this, SLOT(scalarMinProcessText()));
    continuousLayout->addWidget(scalarMin, 1,1);

    maxFlag = new QCheckBox("Clamp maximum", continuousGroup, "maxFlag");
    connect(maxFlag, SIGNAL(toggled(bool)),
            this, SLOT(maxFlagChanged(bool)));
    continuousLayout->addWidget(maxFlag, 2,0);

    scalarMax = new QLineEdit(continuousGroup, "scalarMax");
    connect(scalarMax, SIGNAL(returnPressed()),
            this, SLOT(scalarMaxProcessText()));
    continuousLayout->addWidget(scalarMax, 2,1);

    // ------------------------------------------------------------------

    legendFlag = new QCheckBox("Legend", central, "legendFlag");
    connect(legendFlag, SIGNAL(toggled(bool)),
            this, SLOT(legendFlagChanged(bool)));
    topLayout->addWidget(legendFlag);

}


// ****************************************************************************
// Method: QvisMoleculePlotWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Jeremy Meredith
// Creation:   March 23, 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisMoleculePlotWindow::UpdateWindow(bool doAll)
{
    QString temp;
    double r;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }

        const double         *dptr;
        const float          *fptr;
        const int            *iptr;
        const char           *cptr;
        const unsigned char  *uptr;
        const string         *sptr;
        QColor                tempcolor;
        switch(i)
        {
          case 0: //drawAtomsAs
            if (atts->GetDrawAtomsAs() != MoleculeAttributes::NoAtoms)
            {
                atomSphereQualityLabel->setEnabled(true);
                atomSphereQuality->setEnabled(true);
                scaleRadiusByLabel->setEnabled(true);
                scaleRadiusBy->setEnabled(true);
                radiusVariableLabel->setEnabled(true);
                radiusVariable->setEnabled(true);
                radiusScaleFactorLabel->setEnabled(true);
                radiusScaleFactor->setEnabled(true);
                radiusFixedLabel->setEnabled(true);
                radiusFixed->setEnabled(true);
            }
            else
            {
                atomSphereQualityLabel->setEnabled(false);
                atomSphereQuality->setEnabled(false);
                scaleRadiusByLabel->setEnabled(false);
                scaleRadiusBy->setEnabled(false);
                radiusVariableLabel->setEnabled(false);
                radiusVariable->setEnabled(false);
                radiusScaleFactorLabel->setEnabled(false);
                radiusScaleFactor->setEnabled(false);
                radiusFixedLabel->setEnabled(false);
                radiusFixed->setEnabled(false);
            }
            drawAtomsAs->blockSignals(true);
            drawAtomsAs->setCurrentItem(atts->GetDrawAtomsAs());
            drawAtomsAs->blockSignals(false);
            break;
          case 1: //scaleRadiusBy
            if (atts->GetScaleRadiusBy() == MoleculeAttributes::Variable)
            {
                radiusVariable->setEnabled(true);
                radiusVariableLabel->setEnabled(true);
            }
            else
            {
                radiusVariable->setEnabled(false);
                radiusVariableLabel->setEnabled(false);
            }
            if (atts->GetScaleRadiusBy() == MoleculeAttributes::Atomic || atts->GetScaleRadiusBy() == MoleculeAttributes::Covalent || atts->GetScaleRadiusBy() == MoleculeAttributes::Variable)
            {
                radiusScaleFactor->setEnabled(true);
                radiusScaleFactorLabel->setEnabled(true);
            }
            else
            {
                radiusScaleFactor->setEnabled(false);
                radiusScaleFactorLabel->setEnabled(false);
            }
            if (atts->GetScaleRadiusBy() == MoleculeAttributes::Fixed)
            {
                radiusFixed->setEnabled(true);
                radiusFixedLabel->setEnabled(true);
            }
            else
            {
                radiusFixed->setEnabled(false);
                radiusFixedLabel->setEnabled(false);
            }
            scaleRadiusBy->blockSignals(true);
            scaleRadiusBy->setCurrentItem(atts->GetScaleRadiusBy());
            scaleRadiusBy->blockSignals(false);
            break;
          case 2: //drawBondsAs
            if (atts->GetDrawBondsAs() == MoleculeAttributes::LineBonds || atts->GetDrawBondsAs() == MoleculeAttributes::CylinderBonds)
            {
                colorBonds->setEnabled(true);
                colorBondsLabel->setEnabled(true);
            }
            else
            {
                colorBonds->setEnabled(false);
                colorBondsLabel->setEnabled(false);
            }
            if (atts->GetDrawBondsAs() == MoleculeAttributes::CylinderBonds)
            {
                bondRadius->setEnabled(true);
                bondRadiusLabel->setEnabled(true);
            }
            else
            {
                bondRadius->setEnabled(false);
                bondRadiusLabel->setEnabled(false);
            }
            if (atts->GetDrawBondsAs() == MoleculeAttributes::CylinderBonds)
            {
                bondCylinderQuality->setEnabled(true);
                bondCylinderQualityLabel->setEnabled(true);
            }
            else
            {
                bondCylinderQuality->setEnabled(false);
                bondCylinderQualityLabel->setEnabled(false);
            }
            if (atts->GetDrawBondsAs() == MoleculeAttributes::LineBonds)
            {
                bondLineWidth->setEnabled(true);
                bondLineWidthLabel->setEnabled(true);
            }
            else
            {
                bondLineWidth->setEnabled(false);
                bondLineWidthLabel->setEnabled(false);
            }
            if (atts->GetDrawBondsAs() == MoleculeAttributes::LineBonds)
            {
                bondLineStyle->setEnabled(true);
                bondLineStyleLabel->setEnabled(true);
            }
            else
            {
                bondLineStyle->setEnabled(false);
                bondLineStyleLabel->setEnabled(false);
            }
            if (atts->GetColorBonds() == MoleculeAttributes::SingleColor &&
                atts->GetDrawBondsAs() != MoleculeAttributes::NoBonds)
            {
                bondSingleColor->setEnabled(true);
            }
            else
            {
                bondSingleColor->setEnabled(false);
            }
            drawBondsAs->blockSignals(true);
            drawBondsAs->setCurrentItem(atts->GetDrawBondsAs());
            drawBondsAs->blockSignals(false);
            break;
          case 3: //colorBonds
            if (atts->GetColorBonds() == MoleculeAttributes::SingleColor &&
                atts->GetDrawBondsAs() != MoleculeAttributes::NoBonds)
            {
                bondSingleColor->setEnabled(true);
            }
            else
            {
                bondSingleColor->setEnabled(false);
            }
            colorBonds->setButton(atts->GetColorBonds());
            break;
          case 4: //bondSingleColor
            tempcolor = QColor(atts->GetBondSingleColor().Red(),
                               atts->GetBondSingleColor().Green(),
                               atts->GetBondSingleColor().Blue());
            bondSingleColor->setButtonColor(tempcolor);
            break;
          case 5: //radiusVariable
            radiusVariable->setText(atts->GetRadiusVariable().c_str());
            break;
          case 6: //radiusScaleFactor
            temp.setNum(atts->GetRadiusScaleFactor());
            radiusScaleFactor->setText(temp);
            break;
          case 7: //radiusFixed
            temp.setNum(atts->GetRadiusFixed());
            radiusFixed->setText(temp);
            break;
          case 8: //atomSphereQuality
            atomSphereQuality->blockSignals(true);
            atomSphereQuality->setCurrentItem(atts->GetAtomSphereQuality());
            atomSphereQuality->blockSignals(false);
            break;
          case 9: //bondCylinderQuality
            bondCylinderQuality->blockSignals(true);
            bondCylinderQuality->setCurrentItem(atts->GetBondCylinderQuality());
            bondCylinderQuality->blockSignals(false);
            break;
          case 10: //bondRadius
            temp.setNum(atts->GetBondRadius());
            bondRadius->setText(temp);
            break;
          case 11: //bondLineWidth
            bondLineWidth->blockSignals(true);
            bondLineWidth->SetLineWidth(atts->GetBondLineWidth());
            bondLineWidth->blockSignals(false);
            break;
          case 12: //bondLineStyle
            bondLineStyle->blockSignals(true);
            bondLineStyle->SetLineStyle(atts->GetBondLineStyle());
            bondLineStyle->blockSignals(false);
            break;
          case 13: //elementColorTable
            elementColorTable->setColorTable(atts->GetElementColorTable().c_str());
            break;
          case 14: //residueTypeColorTable
            residueTypeColorTable->setColorTable(atts->GetResidueTypeColorTable().c_str());
            break;
          case 15: //residueSequenceColorTable
            residueSequenceColorTable->setColorTable(atts->GetResidueSequenceColorTable().c_str());
            break;
          case 16: //continuousColorTable
            continuousColorTable->setColorTable(atts->GetContinuousColorTable().c_str());
            break;
          case 17: //legendFlag
            legendFlag->blockSignals(true);
            legendFlag->setChecked(atts->GetLegendFlag());
            legendFlag->blockSignals(false);
            break;
          case 18: //minFlag
            if (atts->GetMinFlag() == true)
            {
                scalarMin->setEnabled(true);
            }
            else
            {
                scalarMin->setEnabled(false);
            }
            minFlag->blockSignals(true);
            minFlag->setChecked(atts->GetMinFlag());
            minFlag->blockSignals(false);
            break;
          case 19: //scalarMin
            temp.setNum(atts->GetScalarMin());
            scalarMin->setText(temp);
            break;
          case 20: //maxFlag
            if (atts->GetMaxFlag() == true)
            {
                scalarMax->setEnabled(true);
            }
            else
            {
                scalarMax->setEnabled(false);
            }
            maxFlag->blockSignals(true);
            maxFlag->setChecked(atts->GetMaxFlag());
            maxFlag->blockSignals(false);
            break;
          case 21: //scalarMax
            temp.setNum(atts->GetScalarMax());
            scalarMax->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisMoleculePlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Wed Mar 22 16:56:08 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisMoleculePlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do drawAtomsAs
    if(which_widget == 0 || doAll)
    {
        // Nothing for drawAtomsAs
    }

    // Do scaleRadiusBy
    if(which_widget == 1 || doAll)
    {
        // Nothing for scaleRadiusBy
    }

    // Do drawBondsAs
    if(which_widget == 2 || doAll)
    {
        // Nothing for drawBondsAs
    }

    // Do colorBonds
    if(which_widget == 3 || doAll)
    {
        // Nothing for colorBonds
    }

    // Do bondSingleColor
    if(which_widget == 4 || doAll)
    {
        // Nothing for bondSingleColor
    }

    // Do radiusVariable
    if(which_widget == 5 || doAll)
    {
        // Nothing for radiusVariable
    }

    // Do radiusScaleFactor
    if(which_widget == 6 || doAll)
    {
        temp = radiusScaleFactor->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetRadiusScaleFactor(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of radiusScaleFactor was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetRadiusScaleFactor());
            Message(msg);
            atts->SetRadiusScaleFactor(atts->GetRadiusScaleFactor());
        }
    }

    // Do radiusFixed
    if(which_widget == 7 || doAll)
    {
        temp = radiusFixed->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetRadiusFixed(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of radiusFixed was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetRadiusFixed());
            Message(msg);
            atts->SetRadiusFixed(atts->GetRadiusFixed());
        }
    }

    // Do atomSphereQuality
    if(which_widget == 8 || doAll)
    {
        // Nothing for atomSphereQuality
    }

    // Do bondCylinderQuality
    if(which_widget == 9 || doAll)
    {
        // Nothing for bondCylinderQuality
    }

    // Do bondRadius
    if(which_widget == 10 || doAll)
    {
        temp = bondRadius->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetBondRadius(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of bondRadius was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetBondRadius());
            Message(msg);
            atts->SetBondRadius(atts->GetBondRadius());
        }
    }

    // Do bondLineWidth
    if(which_widget == 11 || doAll)
    {
        // Nothing for bondLineWidth
    }

    // Do bondLineStyle
    if(which_widget == 12 || doAll)
    {
        // Nothing for bondLineStyle
    }

    // Do elementColorTable
    if(which_widget == 13 || doAll)
    {
        // Nothing for elementColorTable
    }

    // Do residueTypeColorTable
    if(which_widget == 14 || doAll)
    {
        // Nothing for residueTypeColorTable
    }

    // Do residueSequenceColorTable
    if(which_widget == 15 || doAll)
    {
        // Nothing for residueSequenceColorTable
    }

    // Do continuousColorTable
    if(which_widget == 16 || doAll)
    {
        // Nothing for continuousColorTable
    }

    // Do legendFlag
    if(which_widget == 17 || doAll)
    {
        // Nothing for legendFlag
    }

    // Do minFlag
    if(which_widget == 18 || doAll)
    {
        // Nothing for minFlag
    }

    // Do scalarMin
    if(which_widget == 19 || doAll)
    {
        temp = scalarMin->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetScalarMin(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of scalarMin was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetScalarMin());
            Message(msg);
            atts->SetScalarMin(atts->GetScalarMin());
        }
    }

    // Do maxFlag
    if(which_widget == 20 || doAll)
    {
        // Nothing for maxFlag
    }

    // Do scalarMax
    if(which_widget == 21 || doAll)
    {
        temp = scalarMax->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetScalarMax(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of scalarMax was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetScalarMax());
            Message(msg);
            atts->SetScalarMax(atts->GetScalarMax());
        }
    }

}


// ****************************************************************************
// Method: QvisMoleculePlotWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: xml2window
// Creation:   Wed Mar 22 16:56:08 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisMoleculePlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        atts->Notify();

        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        atts->Notify();
}


//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisMoleculePlotWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: xml2window
// Creation:   Wed Mar 22 16:56:08 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisMoleculePlotWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisMoleculePlotWindow::makeDefault
//
// Purpose: 
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: xml2window
// Creation:   Wed Mar 22 16:56:08 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisMoleculePlotWindow::makeDefault()
{
    GetCurrentValues(-1);
    atts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisMoleculePlotWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: xml2window
// Creation:   Wed Mar 22 16:56:08 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisMoleculePlotWindow::reset()
{
    GetViewerMethods()->ResetPlotOptions(plotType);
}


void
QvisMoleculePlotWindow::drawAtomsAsChanged(int val)
{
    if(val != atts->GetDrawAtomsAs())
    {
        atts->SetDrawAtomsAs(MoleculeAttributes::AtomRenderingMode(val));
        Apply();
    }
}


void
QvisMoleculePlotWindow::scaleRadiusByChanged(int val)
{
    if(val != atts->GetScaleRadiusBy())
    {
        atts->SetScaleRadiusBy(MoleculeAttributes::RadiusType(val));
        Apply();
    }
}


void
QvisMoleculePlotWindow::drawBondsAsChanged(int val)
{
    if(val != atts->GetDrawBondsAs())
    {
        atts->SetDrawBondsAs(MoleculeAttributes::BondRenderingMode(val));
        Apply();
    }
}


void
QvisMoleculePlotWindow::colorBondsChanged(int val)
{
    if(val != atts->GetColorBonds())
    {
        atts->SetColorBonds(MoleculeAttributes::BondColoringMode(val));
        Apply();
    }
}


void
QvisMoleculePlotWindow::bondSingleColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    atts->SetBondSingleColor(temp);
    Apply();
}


void
QvisMoleculePlotWindow::radiusVariableChanged(const QString &varName)
{
    atts->SetRadiusVariable(varName.latin1());
    SetUpdate(false);
    Apply();
}


void
QvisMoleculePlotWindow::radiusScaleFactorProcessText()
{
    GetCurrentValues(6);
    Apply();
}


void
QvisMoleculePlotWindow::radiusFixedProcessText()
{
    GetCurrentValues(7);
    Apply();
}


void
QvisMoleculePlotWindow::atomSphereQualityChanged(int val)
{
    if(val != atts->GetAtomSphereQuality())
    {
        atts->SetAtomSphereQuality(MoleculeAttributes::DetailLevel(val));
        Apply();
    }
}


void
QvisMoleculePlotWindow::bondCylinderQualityChanged(int val)
{
    if(val != atts->GetBondCylinderQuality())
    {
        atts->SetBondCylinderQuality(MoleculeAttributes::DetailLevel(val));
        Apply();
    }
}


void
QvisMoleculePlotWindow::bondRadiusProcessText()
{
    GetCurrentValues(10);
    Apply();
}


void
QvisMoleculePlotWindow::bondLineWidthChanged(int style)
{
    atts->SetBondLineWidth(style);
    Apply();
}


void
QvisMoleculePlotWindow::bondLineStyleChanged(int style)
{
    atts->SetBondLineStyle(style);
    Apply();
}


void
QvisMoleculePlotWindow::elementColorTableChanged(bool useDefault, const QString &ctName)
{
    atts->SetElementColorTable(ctName.latin1());
    Apply();
}


void
QvisMoleculePlotWindow::residueTypeColorTableChanged(bool useDefault, const QString &ctName)
{
    atts->SetResidueTypeColorTable(ctName.latin1());
    Apply();
}


void
QvisMoleculePlotWindow::residueSequenceColorTableChanged(bool useDefault, const QString &ctName)
{
    atts->SetResidueSequenceColorTable(ctName.latin1());
    Apply();
}


void
QvisMoleculePlotWindow::continuousColorTableChanged(bool useDefault, const QString &ctName)
{
    atts->SetContinuousColorTable(ctName.latin1());
    Apply();
}


void
QvisMoleculePlotWindow::legendFlagChanged(bool val)
{
    atts->SetLegendFlag(val);
    Apply();
}


void
QvisMoleculePlotWindow::minFlagChanged(bool val)
{
    atts->SetMinFlag(val);
    Apply();
}


void
QvisMoleculePlotWindow::scalarMinProcessText()
{
    GetCurrentValues(19);
    Apply();
}


void
QvisMoleculePlotWindow::maxFlagChanged(bool val)
{
    atts->SetMaxFlag(val);
    Apply();
}


void
QvisMoleculePlotWindow::scalarMaxProcessText()
{
    GetCurrentValues(21);
    Apply();
}


