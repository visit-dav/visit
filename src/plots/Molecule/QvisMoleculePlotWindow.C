// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisMoleculePlotWindow.h"

#include <MoleculeAttributes.h>
#include <ViewerProxy.h>

#include <QTabWidget>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QWidget>
#include <QButtonGroup>
#include <QRadioButton>
#include <QComboBox>
#include <QGroupBox>
#include <QvisColorTableButton.h>
#include <QvisColorButton.h>
#include <QvisLineWidthWidget.h>
#include <QvisVariableButton.h>


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
                         const QString &caption,
                         const QString &shortName,
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
//   Brad Whitlock, Wed Apr 23 10:01:15 PDT 2008
//   Added tr()'s
//
//   Cyrus Harrison, Fri Jul 18 14:38:14 PDT 2008
//   Initial Qt4 Port.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisMoleculePlotWindow::CreateWindowContents()
{
    QTabWidget *topTab = new QTabWidget(central);
    topLayout->addWidget(topTab);


    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    //                            Atoms Tab
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    QWidget *atomsGroup = new QWidget(central);
    topTab->addTab(atomsGroup, tr("Atoms"));

    QGridLayout *atomsLayout = new QGridLayout(atomsGroup);

    drawAtomsAsLabel = new QLabel(tr("Draw atoms as"), atomsGroup);
    atomsLayout->addWidget(drawAtomsAsLabel, 0,0, 1,2);
    drawAtomsAs = new QComboBox(atomsGroup);
    drawAtomsAs->addItem(tr("None"));
    drawAtomsAs->addItem(tr("Spheres"));
    drawAtomsAs->addItem(tr("Sphere imposters"));
    connect(drawAtomsAs, SIGNAL(activated(int)),
            this, SLOT(drawAtomsAsChanged(int)));
    atomsLayout->addWidget(drawAtomsAs, 0,2);
    

    atomSphereQualityLabel = new QLabel(tr("Atom sphere quality"), atomsGroup);
    atomsLayout->addWidget(atomSphereQualityLabel,1,0, 1,2);
    atomSphereQuality = new QComboBox(atomsGroup);
    atomSphereQuality->addItem(tr("Low"));
    atomSphereQuality->addItem(tr("Medium"));
    atomSphereQuality->addItem(tr("High"));
    atomSphereQuality->addItem(tr("Super"));
    connect(atomSphereQuality, SIGNAL(activated(int)),
            this, SLOT(atomSphereQualityChanged(int)));
    atomsLayout->addWidget(atomSphereQuality, 1,2);

    scaleRadiusByLabel = new QLabel(tr("Radius based on"), atomsGroup);
    atomsLayout->addWidget(scaleRadiusByLabel, 2,0, 1,2);
    scaleRadiusBy = new QComboBox(atomsGroup);
    scaleRadiusBy->addItem(tr("Fixed value"));
    scaleRadiusBy->addItem(tr("Covalent radius"));
    scaleRadiusBy->addItem(tr("Atomic radius"));
    scaleRadiusBy->addItem(tr("Scalar variable"));
    connect(scaleRadiusBy, SIGNAL(activated(int)),
            this, SLOT(scaleRadiusByChanged(int)));
    atomsLayout->addWidget(scaleRadiusBy, 2,2);

    radiusVariableLabel = new QLabel(tr("Variable for atom radius"), atomsGroup);
    atomsLayout->addWidget(radiusVariableLabel,3,1);
    int radiusVariableMask = QvisVariableButton::Scalars;
    radiusVariable = new QvisVariableButton(true, true, true, radiusVariableMask, atomsGroup);
    connect(radiusVariable, SIGNAL(activated(const QString&)),
            this, SLOT(radiusVariableChanged(const QString&)));
    atomsLayout->addWidget(radiusVariable, 3,2);

    radiusScaleFactorLabel = new QLabel(tr("Atom radius scale factor"), atomsGroup);
    atomsLayout->addWidget(radiusScaleFactorLabel,4,1);
    radiusScaleFactor = new QLineEdit(atomsGroup);
    connect(radiusScaleFactor, SIGNAL(returnPressed()),
            this, SLOT(radiusScaleFactorProcessText()));
    atomsLayout->addWidget(radiusScaleFactor, 4,2);

    radiusFixedLabel = new QLabel(tr("Fixed atom radius"), atomsGroup);
    atomsLayout->addWidget(radiusFixedLabel,5,1);
    radiusFixed = new QLineEdit(atomsGroup);
    connect(radiusFixed, SIGNAL(returnPressed()),
            this, SLOT(radiusFixedProcessText()));
    atomsLayout->addWidget(radiusFixed, 5,2);
    atomsLayout->setRowStretch(6,2);

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    //                            Bonds Tab
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    QWidget *bondsGroup = new QWidget(central);
    topTab->addTab(bondsGroup, tr("Bonds"));

    QGridLayout *bondsLayout = new QGridLayout(bondsGroup);

    int row = 0;
    drawBondsAsLabel = new QLabel(tr("Draw bonds as"), bondsGroup);
    bondsLayout->addWidget(drawBondsAsLabel, row,0, 1,2);
    drawBondsAs = new QComboBox(bondsGroup);
    drawBondsAs->addItem(tr("None"));
    drawBondsAs->addItem(tr("Lines"));
    drawBondsAs->addItem(tr("Cylinders"));
    connect(drawBondsAs, SIGNAL(activated(int)),
            this, SLOT(drawBondsAsChanged(int)));
    bondsLayout->addWidget(drawBondsAs, row,2);
    row++;

    bondCylinderQualityLabel = new QLabel(tr("Bond cylinder quality"), bondsGroup);
    bondsLayout->addWidget(bondCylinderQualityLabel, row,1);
    bondCylinderQuality = new QComboBox(bondsGroup);
    bondCylinderQuality->addItem(tr("Low"));
    bondCylinderQuality->addItem(tr("Medium"));
    bondCylinderQuality->addItem(tr("High"));
    bondCylinderQuality->addItem(tr("Super"));
    connect(bondCylinderQuality, SIGNAL(activated(int)),
            this, SLOT(bondCylinderQualityChanged(int)));
    bondsLayout->addWidget(bondCylinderQuality, row,2);
    row++;

    bondRadiusLabel = new QLabel(tr("Bond radius"), bondsGroup);
    bondsLayout->addWidget(bondRadiusLabel, row,1);
    bondRadius = new QLineEdit(bondsGroup);
    connect(bondRadius, SIGNAL(returnPressed()),
            this, SLOT(bondRadiusProcessText()));
    bondsLayout->addWidget(bondRadius, row,2);
    row++;

    bondLineWidthLabel = new QLabel(tr("Bond line width"), bondsGroup);
    bondsLayout->addWidget(bondLineWidthLabel, row,1);
    bondLineWidth = new QvisLineWidthWidget(0, bondsGroup);
    connect(bondLineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(bondLineWidthChanged(int)));
    bondsLayout->addWidget(bondLineWidth, row,2);
    row++;

    colorBondsLabel = new QLabel(tr("Color bonds by"), bondsGroup);
    bondsLayout->addWidget(colorBondsLabel, row,0, 1,2);
 
    colorBondsWidget = new QWidget(bondsGroup);
    colorBondsGroup = new QButtonGroup(colorBondsWidget);
    
    QGridLayout *colorBondsLayout = new QGridLayout(colorBondsWidget);
    colorBondsLayout->setContentsMargins(0,0,0,0);
    
    QRadioButton *colorBondsBondColoringModeColorByAtom = new QRadioButton(tr("Adjacent atom color"),
                                                                           colorBondsWidget);
    QRadioButton *colorBondsBondColoringModeSingleColor = new QRadioButton(tr("Single color"), 
                                                                           colorBondsWidget);
    
    colorBondsLayout->addWidget(colorBondsBondColoringModeColorByAtom, 0,0, 1,2);
    colorBondsLayout->addWidget(colorBondsBondColoringModeSingleColor, 1,0);
    
    colorBondsGroup->addButton(colorBondsBondColoringModeColorByAtom, 0);
    colorBondsGroup->addButton(colorBondsBondColoringModeSingleColor, 1);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(colorBondsGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(colorBondsChanged(int)));
#else
    connect(colorBondsGroup, SIGNAL(idClicked(int)),
            this, SLOT(colorBondsChanged(int)));
#endif
    bondSingleColor = new QvisColorButton(bondsGroup);
    connect(bondSingleColor, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(bondSingleColorChanged(const QColor&)));
    colorBondsLayout->addWidget(bondSingleColor, 1,1);
    bondsLayout->addWidget(colorBondsWidget, row,2);
    row++;
    bondsLayout->setRowStretch(row,2);



    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    //                            Colors Tab
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    QWidget *colorsGroup = new QWidget(central);
    topTab->addTab(colorsGroup, tr("Colors"));

    QVBoxLayout *colorsLayout = new QVBoxLayout(colorsGroup);
    

    QGroupBox *discreteGroup = new QGroupBox(colorsGroup);
    discreteGroup->setTitle(tr("Discrete colors"));
    colorsLayout->addWidget(discreteGroup);
    
    QGridLayout *discreteLayout = new QGridLayout(discreteGroup);

    QLabel *discreteColorTableLabel = new QLabel(tr("Color table for:"), discreteGroup);
    discreteLayout->addWidget(discreteColorTableLabel,0,0,1,2);
    
    elementColorTableLabel = new QLabel(tr("Element types"), discreteGroup);
    discreteLayout->addWidget(elementColorTableLabel, 1,0);
    elementColorTable = new QvisColorTableButton(discreteGroup);
    connect(elementColorTable, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(elementColorTableChanged(bool, const QString&)));
    discreteLayout->addWidget(elementColorTable, 1,1);

    residueTypeColorTableLabel = new QLabel(tr("Residue types"), discreteGroup);
    discreteLayout->addWidget(residueTypeColorTableLabel, 2,0);
    residueTypeColorTable = new QvisColorTableButton(discreteGroup);
    connect(residueTypeColorTable, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(residueTypeColorTableChanged(bool, const QString&)));
    discreteLayout->addWidget(residueTypeColorTable, 2,1);

    residueSequenceColorTableLabel = new QLabel(tr("Other discrete fields"), discreteGroup);
    discreteLayout->addWidget(residueSequenceColorTableLabel, 3,0);
    residueSequenceColorTable = new QvisColorTableButton(discreteGroup);
    connect(residueSequenceColorTable, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(residueSequenceColorTableChanged(bool, const QString&)));
    discreteLayout->addWidget(residueSequenceColorTable, 3,1);

    QGroupBox *continuousGroup = new QGroupBox(colorsGroup);
    continuousGroup->setTitle(tr("Continuous colors"));
    colorsLayout->addWidget(continuousGroup);
    QVBoxLayout *continuousTopLayout = new QVBoxLayout(continuousGroup);

    QGridLayout *continuousLayout = new QGridLayout();
    continuousTopLayout->addLayout(continuousLayout);


    continuousColorTableLabel = new QLabel(tr("Color table for scalars"), continuousGroup);
    continuousLayout->addWidget(continuousColorTableLabel, 0,0);
    continuousColorTable = new QvisColorTableButton(continuousGroup);
    connect(continuousColorTable, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(continuousColorTableChanged(bool, const QString&)));
    continuousLayout->addWidget(continuousColorTable, 0,1);

    minFlag = new QCheckBox(tr("Clamp minimum"), continuousGroup);
    connect(minFlag, SIGNAL(toggled(bool)),
            this, SLOT(minFlagChanged(bool)));
    continuousLayout->addWidget(minFlag, 1,0);

    scalarMin = new QLineEdit(continuousGroup);
    connect(scalarMin, SIGNAL(returnPressed()),
            this, SLOT(scalarMinProcessText()));
    continuousLayout->addWidget(scalarMin, 1,1);

    maxFlag = new QCheckBox(tr("Clamp maximum"), continuousGroup);
    connect(maxFlag, SIGNAL(toggled(bool)),
            this, SLOT(maxFlagChanged(bool)));
    continuousLayout->addWidget(maxFlag, 2,0);

    scalarMax = new QLineEdit(continuousGroup);
    connect(scalarMax, SIGNAL(returnPressed()),
            this, SLOT(scalarMaxProcessText()));
    continuousLayout->addWidget(scalarMax, 2,1);
    colorsLayout->addStretch(2);

    // ------------------------------------------------------------------

    //
    // Create the misc stuff
    //
    QGroupBox * miscGroup = new QGroupBox(central);
    miscGroup->setTitle(tr("Misc"));
    topLayout->addWidget(miscGroup);

    QGridLayout *miscLayout = new QGridLayout(miscGroup);
    miscLayout->setContentsMargins(5,5,5,5);
    miscLayout->setSpacing(10);
 
    // Create the legend toggle
    legendToggle = new QCheckBox(tr("Legend"), central);
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    miscLayout->addWidget(legendToggle, 0, 0);
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
//   Kathleen Bonnell, Wed Jun 4 07:59:02 PDT 2008
//   Removed unused varaibles.
//
//   Cyrus Harrison, Fri Jul 18 14:38:14 PDT 2008
//   Initial Qt4 Port.
//
//   Kathleen Biagas, Thu Apr 9 07:19:54 MST 2015
//   Use helper function FloatToQString for consistency in formatting across
//   all windows.
//
//   Kathleen Biagas, Fri Jun 18 2021
//   Moved atom related widget enable/disable to end of method, for easier
//   coding due to complex dependencies.
//
// ****************************************************************************

void
QvisMoleculePlotWindow::UpdateWindow(bool doAll)
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

        QColor                tempcolor;
        switch(i)
        {
          case MoleculeAttributes::ID_drawAtomsAs:
            drawAtomsAs->blockSignals(true);
            drawAtomsAs->setCurrentIndex(atts->GetDrawAtomsAs());
            drawAtomsAs->blockSignals(false);
            break;
          case MoleculeAttributes::ID_scaleRadiusBy:
            scaleRadiusBy->blockSignals(true);
            scaleRadiusBy->setCurrentIndex(atts->GetScaleRadiusBy());
            scaleRadiusBy->blockSignals(false);
            break;
          case MoleculeAttributes::ID_drawBondsAs:
            if (atts->GetDrawBondsAs() == MoleculeAttributes::LineBonds || atts->GetDrawBondsAs() == MoleculeAttributes::CylinderBonds)
            {
                colorBondsWidget->setEnabled(true);
                colorBondsLabel->setEnabled(true);
            }
            else
            {
                colorBondsWidget->setEnabled(false);
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
            drawBondsAs->setCurrentIndex(atts->GetDrawBondsAs());
            drawBondsAs->blockSignals(false);
            break;
          case MoleculeAttributes::ID_colorBonds:
            if (atts->GetColorBonds() == MoleculeAttributes::SingleColor &&
                atts->GetDrawBondsAs() != MoleculeAttributes::NoBonds)
            {
                bondSingleColor->setEnabled(true);
            }
            else
            {
                bondSingleColor->setEnabled(false);
            }
            colorBondsGroup->button(atts->GetColorBonds())->setChecked(true);
            break;
          case MoleculeAttributes::ID_bondSingleColor:
            tempcolor = QColor(atts->GetBondSingleColor().Red(),
                               atts->GetBondSingleColor().Green(),
                               atts->GetBondSingleColor().Blue());
            bondSingleColor->setButtonColor(tempcolor);
            break;
          case MoleculeAttributes::ID_radiusVariable:
            radiusVariable->setText(atts->GetRadiusVariable().c_str());
            break;
          case MoleculeAttributes::ID_radiusScaleFactor:
            radiusScaleFactor->setText(FloatToQString(atts->GetRadiusScaleFactor()));
            break;
          case MoleculeAttributes::ID_radiusFixed:
            radiusFixed->setText(FloatToQString(atts->GetRadiusFixed()));
            break;
          case MoleculeAttributes::ID_atomSphereQuality:
            atomSphereQuality->blockSignals(true);
            atomSphereQuality->setCurrentIndex(atts->GetAtomSphereQuality());
            atomSphereQuality->blockSignals(false);
            break;
          case MoleculeAttributes::ID_bondCylinderQuality:
            bondCylinderQuality->blockSignals(true);
            bondCylinderQuality->setCurrentIndex(atts->GetBondCylinderQuality());
            bondCylinderQuality->blockSignals(false);
            break;
          case MoleculeAttributes::ID_bondRadius:
            bondRadius->setText(FloatToQString(atts->GetBondRadius()));
            break;
          case MoleculeAttributes::ID_bondLineWidth:
            bondLineWidth->blockSignals(true);
            bondLineWidth->SetLineWidth(atts->GetBondLineWidth());
            bondLineWidth->blockSignals(false);
            break;
          case MoleculeAttributes::ID_elementColorTable:
            elementColorTable->setColorTable(atts->GetElementColorTable().c_str());
            break;
          case MoleculeAttributes::ID_residueTypeColorTable:
            residueTypeColorTable->setColorTable(atts->GetResidueTypeColorTable().c_str());
            break;
          case MoleculeAttributes::ID_residueSequenceColorTable:
            residueSequenceColorTable->setColorTable(atts->GetResidueSequenceColorTable().c_str());
            break;
          case MoleculeAttributes::ID_continuousColorTable:
            continuousColorTable->setColorTable(atts->GetContinuousColorTable().c_str());
            break;
          case MoleculeAttributes::ID_legendFlag:
            legendToggle->blockSignals(true);
            legendToggle->setChecked(atts->GetLegendFlag());
            legendToggle->blockSignals(false);
            break;
          case MoleculeAttributes::ID_minFlag:
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
          case MoleculeAttributes::ID_scalarMin:
            scalarMin->setText(FloatToQString(atts->GetScalarMin()));
            break;
          case MoleculeAttributes::ID_maxFlag:
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
          case MoleculeAttributes::ID_scalarMax:
            scalarMax->setText(FloatToQString(atts->GetScalarMax()));
            break;
        }
    }
    if (atts->GetDrawAtomsAs() == MoleculeAttributes::NoAtoms)
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
    else
    {
        scaleRadiusByLabel->setEnabled(true);
        scaleRadiusBy->setEnabled(true);

        if (atts->GetDrawAtomsAs() == MoleculeAttributes::SphereAtoms)
        {
            atomSphereQualityLabel->setEnabled(true);
            atomSphereQuality->setEnabled(true);
        }
        else
        {
            atomSphereQualityLabel->setEnabled(false);
            atomSphereQuality->setEnabled(false);
        }

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

        if (atts->GetScaleRadiusBy() != MoleculeAttributes::Fixed)
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
//   Cyrus Harrison, Fri Jul 18 14:38:14 PDT 2008
//   Initial Qt4 Port.
//
//   Brad Whitlock, Mon Aug 11 11:05:49 PDT 2008
//   Updated to new style.
//
// ****************************************************************************

void
QvisMoleculePlotWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
    QString msg, temp;

    // Do radiusScaleFactor
    if(which_widget == MoleculeAttributes::ID_radiusScaleFactor || doAll)
    {
        float val;
        if(LineEditGetFloat(radiusScaleFactor, val))
            atts->SetRadiusScaleFactor(val);
        else
        {
            ResettingError(tr("radius scale factor"),
                FloatToQString(atts->GetRadiusScaleFactor()));
            atts->SetRadiusScaleFactor(atts->GetRadiusScaleFactor());
        }
    }

    // Do radiusFixed
    if(which_widget == MoleculeAttributes::ID_radiusFixed || doAll)
    {
        float val;
        if(LineEditGetFloat(radiusFixed, val))
            atts->SetRadiusFixed(val);
        else
        {
            ResettingError(tr("fixed radius"),
                FloatToQString(atts->GetRadiusFixed()));
            atts->SetRadiusFixed(atts->GetRadiusFixed());
        }
    }

    // Do bondRadius
    if(which_widget == MoleculeAttributes::ID_bondRadius || doAll)
    {
        float val;
        if(LineEditGetFloat(bondRadius, val))
            atts->SetBondRadius(val);
        else
        {
            ResettingError(tr("bond radius"), 
                FloatToQString(atts->GetBondRadius()));
            atts->SetBondRadius(atts->GetBondRadius());
        }
    }

    // Do scalarMin
    if(which_widget == MoleculeAttributes::ID_scalarMin || doAll)
    {
        float val;
        if(LineEditGetFloat(scalarMin, val))
            atts->SetScalarMin(val);
        else
        {
            ResettingError(tr("minimum"),
                FloatToQString(atts->GetScalarMin()));
            atts->SetScalarMin(atts->GetScalarMin());
        }
    }

    // Do scalarMax
    if(which_widget == MoleculeAttributes::ID_scalarMax || doAll)
    {
        float val;
        if(LineEditGetFloat(scalarMax, val))
            atts->SetScalarMax(val);
        else
        {
            ResettingError(tr("maximum"),
                FloatToQString(atts->GetScalarMax()));
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
    atts->SetRadiusVariable(varName.toStdString());
    SetUpdate(false);
    Apply();
}


void
QvisMoleculePlotWindow::radiusScaleFactorProcessText()
{
    GetCurrentValues(MoleculeAttributes::ID_radiusScaleFactor);
    Apply();
}


void
QvisMoleculePlotWindow::radiusFixedProcessText()
{
    GetCurrentValues(MoleculeAttributes::ID_radiusFixed);
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
    GetCurrentValues(MoleculeAttributes::ID_bondRadius);
    Apply();
}


void
QvisMoleculePlotWindow::bondLineWidthChanged(int style)
{
    atts->SetBondLineWidth(style);
    Apply();
}


void
QvisMoleculePlotWindow::elementColorTableChanged(bool useDefault, const QString &ctName)
{
    atts->SetElementColorTable(ctName.toStdString());
    Apply();
}


void
QvisMoleculePlotWindow::residueTypeColorTableChanged(bool useDefault, const QString &ctName)
{
    atts->SetResidueTypeColorTable(ctName.toStdString());
    Apply();
}


void
QvisMoleculePlotWindow::residueSequenceColorTableChanged(bool useDefault, const QString &ctName)
{
    atts->SetResidueSequenceColorTable(ctName.toStdString());
    Apply();
}


void
QvisMoleculePlotWindow::continuousColorTableChanged(bool useDefault, const QString &ctName)
{
    atts->SetContinuousColorTable(ctName.toStdString());
    Apply();
}


void
QvisMoleculePlotWindow::legendToggled(bool val)
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
    GetCurrentValues(MoleculeAttributes::ID_scalarMin);
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
    GetCurrentValues(MoleculeAttributes::ID_scalarMax);
    Apply();
}


