// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisTransformWindow.h"

#include <TransformAttributes.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QTabWidget>
#include <QWidget>


// ****************************************************************************
// Method: QvisTransformWindow::QvisTransformWindow
//
// Purpose: 
//   Constructor
//
// Programmer: Jeremy Meredith
// Creation:   Fri Apr 12 13:17:34 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisTransformWindow::QvisTransformWindow(const int type,
                         TransformAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type, subj, caption, shortName, notepad)
{
    atts = subj;

    inputCoord = 0;
    outputCoord = 0;
}

// ****************************************************************************
// Method: QvisTransformWindow::~QvisTransformWindow
//
// Purpose: 
//   Destructor
//
// Programmer: Jeremy Meredith
// Creation:   Fri Apr 12 13:17:34 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisTransformWindow::~QvisTransformWindow()
{
}

// ****************************************************************************
//  Method:  QvisTransformWindow::CreateWindowContents
//
//  Purpose:
//    Create the transform window.  Modified from the generated version.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 24, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 25 11:43:56 PDT 2001
//    Made it use my new QNarrowLineEdit for some of the text fields so the
//    default window width is thinner.
//
//    Jeremy Meredith, Fri Feb  4 17:48:04 PST 2005
//    Added support for coordinate transforms.
//
//    Jeremy Meredith, Tue Apr 15 13:42:44 EDT 2008
//    Added support for linear transforms.
//
//    Brad Whitlock, Thu Apr 24 15:50:49 PDT 2008
//    Added tr()'s
//
//    Brad Whitlock, Wed Aug 13 20:51:47 PDT 2008
//    Qt 4.
//
//    Jeremy Meredith, Fri Aug  7 14:42:15 EDT 2009
//    Renamed Spherical coordinate help text to match ISO 31-11 and our
//    new interpretation of them.  Added vector transform method
//    selection to coordinate system conversions.
//
//    Dave Pugmire, Fri May 14 08:04:43 EDT 2010
//    Flag for vector transformations.
//
//    Jeremy Meredith, Fri Jun 18 11:29:03 EDT 2010
//    Fixed a widget parent bug (1st page widget pointing to 3rd page parent).
//
//    Tom Fogal, Tue Jul 27 11:15:25 MDT 2010
//    Initialize elements for 4x4 matrix modification.
//    Pull second checkbox up to horizontally align them.
//
//    Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//    Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisTransformWindow::CreateWindowContents()
{
    transformTypeTabs = new QTabWidget(central);
    topLayout->addWidget(transformTypeTabs);

    // ----------------------------------------------------------------------
    // First page
    // ----------------------------------------------------------------------
    firstPage = new QWidget(central);
    transformTypeTabs->addTab(firstPage, tr("Arbitrary"));

    QGridLayout *mainLayout = new QGridLayout(firstPage);

    // ---------------
    doRotate = new QCheckBox(tr("Rotate"), firstPage);
    connect(doRotate, SIGNAL(toggled(bool)),
            this, SLOT(doRotateChanged(bool)));
    mainLayout->addWidget(doRotate, 0, 0, 1, 3);

    rotateOriginLabel = new QLabel(tr("Origin"), firstPage);
    mainLayout->addWidget(rotateOriginLabel,1,1, Qt::AlignRight);
    rotateOrigin = new QLineEdit(firstPage);
    connect(rotateOrigin, SIGNAL(returnPressed()),
            this, SLOT(rotateOriginProcessText()));
    mainLayout->addWidget(rotateOrigin, 1, 2, 1, 5);

    rotateAxisLabel = new QLabel(tr("Axis"), firstPage);
    mainLayout->addWidget(rotateAxisLabel,2,1, Qt::AlignRight);
    rotateAxis = new QLineEdit(firstPage);
    connect(rotateAxis, SIGNAL(returnPressed()),
            this, SLOT(rotateAxisProcessText()));
    mainLayout->addWidget(rotateAxis, 2, 2, 1, 5);

    rotateAmountLabel = new QLabel(tr("Amount"), firstPage);
    mainLayout->addWidget(rotateAmountLabel,3,1, Qt::AlignRight);
    rotateAmount = new QNarrowLineEdit(firstPage);
    connect(rotateAmount, SIGNAL(returnPressed()),
            this, SLOT(rotateAmountProcessText()));
    mainLayout->addWidget(rotateAmount, 3, 2, 1, 2);

    
    rotateTypeWidget = new QWidget(firstPage);
    rotateType = new QButtonGroup(rotateTypeWidget);
    QHBoxLayout *rotateTypeLayout = new QHBoxLayout(rotateTypeWidget);
    rotateTypeLayout->setContentsMargins(0,0,0,0);
    rotateTypeLayout->setSpacing(10);
    QRadioButton *rotateTypeAngleTypeDeg = new QRadioButton(tr("Deg"),rotateTypeWidget);
    rotateType->addButton(rotateTypeAngleTypeDeg, 0);
    rotateTypeLayout->addWidget(rotateTypeAngleTypeDeg);
    QRadioButton *rotateTypeAngleTypeRad = new QRadioButton(tr("Rad"),rotateTypeWidget);
    rotateType->addButton(rotateTypeAngleTypeRad, 1);
    rotateTypeLayout->addWidget(rotateTypeAngleTypeRad);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(rotateType, SIGNAL(buttonClicked(int)),
            this, SLOT(rotateTypeChanged(int)));
#else
    connect(rotateType, SIGNAL(idClicked(int)),
            this, SLOT(rotateTypeChanged(int)));
#endif
    mainLayout->addWidget(rotateTypeWidget, 3, 4, 1, 3);

    // ---------------
    doScale = new QCheckBox(tr("Scale"), firstPage);
    connect(doScale, SIGNAL(toggled(bool)),
            this, SLOT(doScaleChanged(bool)));
    mainLayout->addWidget(doScale, 4, 0, 1, 3);

    scaleOriginLabel = new QLabel(tr("Origin"), firstPage);
    mainLayout->addWidget(scaleOriginLabel,5,1, Qt::AlignRight);
    scaleOrigin = new QLineEdit(firstPage);
    connect(scaleOrigin, SIGNAL(returnPressed()),
            this, SLOT(scaleOriginProcessText()));
    mainLayout->addWidget(scaleOrigin, 5, 2, 1, 5);

    scaleXLabel = new QLabel(tr("X"), firstPage);
    mainLayout->addWidget(scaleXLabel,6,1, Qt::AlignRight);
    scaleX = new QNarrowLineEdit(firstPage);
    connect(scaleX, SIGNAL(returnPressed()),
            this, SLOT(scaleXProcessText()));
    mainLayout->addWidget(scaleX, 6,2);

    scaleYLabel = new QLabel(tr("Y"), firstPage);
    mainLayout->addWidget(scaleYLabel,6,3, Qt::AlignRight);
    scaleY = new QNarrowLineEdit(firstPage);
    connect(scaleY, SIGNAL(returnPressed()),
            this, SLOT(scaleYProcessText()));
    mainLayout->addWidget(scaleY, 6,4);

    scaleZLabel = new QLabel(tr("Z"), firstPage);
    mainLayout->addWidget(scaleZLabel,6,5, Qt::AlignRight);
    scaleZ = new QNarrowLineEdit(firstPage);
    connect(scaleZ, SIGNAL(returnPressed()),
            this, SLOT(scaleZProcessText()));
    mainLayout->addWidget(scaleZ, 6,6);

    // ---------------
    doTranslate = new QCheckBox(tr("Translate"), firstPage);
    connect(doTranslate, SIGNAL(toggled(bool)),
            this, SLOT(doTranslateChanged(bool)));
    mainLayout->addWidget(doTranslate, 7, 0, 1, 3);

    translateXLabel = new QLabel(tr("X"), firstPage);
    mainLayout->addWidget(translateXLabel,8,1, Qt::AlignRight);
    translateX = new QNarrowLineEdit(firstPage);
    connect(translateX, SIGNAL(returnPressed()),
            this, SLOT(translateXProcessText()));
    mainLayout->addWidget(translateX, 8,2);

    translateYLabel = new QLabel(tr("Y"), firstPage);
    mainLayout->addWidget(translateYLabel,8,3, Qt::AlignRight);
    translateY = new QNarrowLineEdit(firstPage);
    connect(translateY, SIGNAL(returnPressed()),
            this, SLOT(translateYProcessText()));
    mainLayout->addWidget(translateY, 8,4);

    translateZLabel = new QLabel(tr("Z"), firstPage);
    mainLayout->addWidget(translateZLabel,8,5, Qt::AlignRight);
    translateZ = new QNarrowLineEdit(firstPage);
    connect(translateZ, SIGNAL(returnPressed()),
            this, SLOT(translateZProcessText()));
    mainLayout->addWidget(translateZ, 8,6);
    
    transformVectors1 = new QCheckBox(tr("Transform vectors"), firstPage);
    mainLayout->addWidget(transformVectors1, 9, 0);
    connect(transformVectors1, SIGNAL(toggled(bool)),
            this, SLOT(transformVectorsChanged(bool)));
    mainLayout->setRowStretch(9, 100);

    // ----------------------------------------------------------------------
    // Second page
    // ----------------------------------------------------------------------
    secondPage = new QWidget(central);
    transformTypeTabs->addTab(secondPage, "Coordinate");

    QVBoxLayout *secondPageLayout = new QVBoxLayout(secondPage);

    QGroupBox *inputFrame = new QGroupBox(tr("Input coordinates"), secondPage);
    secondPageLayout->addWidget(inputFrame);

    inputCoord = new QButtonGroup(inputFrame);
    QVBoxLayout *inputCoordLayout = new QVBoxLayout(inputFrame);
    inputCoordLayout->setSpacing(10);
    QRadioButton *iCart = new QRadioButton(tr("Cartesian (x,y,z)"), inputFrame);
    inputCoord->addButton(iCart, 0);
    inputCoordLayout->addWidget(iCart);
    QRadioButton *iCyl  = new QRadioButton(tr("Cylindrical (r,phi,z)"), inputFrame);
    inputCoord->addButton(iCyl, 1);
    inputCoordLayout->addWidget(iCyl);
    QRadioButton *iSph  = new QRadioButton(tr("Spherical (r,theta,phi)"), inputFrame);
    inputCoord->addButton(iSph, 2);
    inputCoordLayout->addWidget(iSph);

    QGroupBox *outputFrame = new QGroupBox(tr("Output coordinates"), secondPage);
    secondPageLayout->addWidget(outputFrame);

    outputCoord = new QButtonGroup(outputFrame);
    QGridLayout *outputCoordLayout = new QGridLayout(outputFrame);
    outputCoordLayout->setSpacing(10);
    QRadioButton *oCart = new QRadioButton(tr("Cartesian (x,y,z)"), outputFrame);
    outputCoord->addButton(oCart, 0);
    outputCoordLayout->addWidget(oCart, 0, 0);
    QRadioButton *oCyl  = new QRadioButton(tr("Cylindrical (r,phi,z)"), outputFrame);
    outputCoord->addButton(oCyl, 1);
    outputCoordLayout->addWidget(oCyl, 1, 0);
    QRadioButton *oSph  = new QRadioButton(tr("Spherical (r,theta,phi)"), outputFrame);
    outputCoord->addButton(oSph, 2);
    outputCoordLayout->addWidget(oSph, 2, 0);

    continuousPhi = new QCheckBox(tr("phi on a semi-infinite domain"),
                                  outputFrame);
    outputCoordLayout->addWidget(continuousPhi, 1, 1);
    connect(continuousPhi, SIGNAL(toggled(bool)),
            this, SLOT(continuousPhiChanged(bool)));

    continuousPhiLabel = new QLabel(tr("     (lines only)"), secondPage);
    outputCoordLayout->addWidget(continuousPhiLabel, 2, 1);   


    QFrame *vectorMethodFrame = new QFrame(secondPage);
    secondPageLayout->addWidget(vectorMethodFrame);
    QHBoxLayout *vectorMethodLayout = new QHBoxLayout(vectorMethodFrame);
    QLabel *vectorMethodLabel = new QLabel(tr("Vector transform method:"),
                                           secondPage);
    vectorMethodCombo = new QComboBox(secondPage);
    vectorMethodCombo->addItem(tr("No transformation"));
    vectorMethodCombo->addItem(tr("Treat as point coordinates"));
    vectorMethodCombo->addItem(tr("Treat as coordinate displacements"));
    vectorMethodCombo->addItem(tr("Treat as instantaneous directions"));
    connect(vectorMethodCombo, SIGNAL(activated(int)),
           this, SLOT(vectorMethodChanged(int)));
    vectorMethodLayout->addWidget(vectorMethodLabel);
    vectorMethodLayout->addWidget(vectorMethodCombo);

    secondPageLayout->addStretch(100);

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(inputCoord, SIGNAL(buttonClicked(int)),
            this, SLOT(inputCoordChanged(int)));
    connect(outputCoord, SIGNAL(buttonClicked(int)),
            this, SLOT(outputCoordChanged(int)));
#else
    connect(inputCoord, SIGNAL(idClicked(int)),
            this, SLOT(inputCoordChanged(int)));
    connect(outputCoord, SIGNAL(idClicked(int)),
            this, SLOT(outputCoordChanged(int)));
#endif

    // ----------------------------------------------------------------------
    // Third page
    // ----------------------------------------------------------------------
    thirdPage = new QWidget(central);
    transformTypeTabs->addTab(thirdPage, tr("Linear"));

    QGridLayout *thirdPageLayout = new QGridLayout(thirdPage);
    thirdPageLayout->addWidget(new QLabel(tr("Matrix elements")),
                               0, 0, 1, 3);
    m00 = new QNarrowLineEdit(thirdPage);
    m01 = new QNarrowLineEdit(thirdPage);
    m02 = new QNarrowLineEdit(thirdPage);
    m03 = new QNarrowLineEdit(thirdPage);
    m10 = new QNarrowLineEdit(thirdPage);
    m11 = new QNarrowLineEdit(thirdPage);
    m12 = new QNarrowLineEdit(thirdPage);
    m13 = new QNarrowLineEdit(thirdPage);
    m20 = new QNarrowLineEdit(thirdPage);
    m21 = new QNarrowLineEdit(thirdPage);
    m22 = new QNarrowLineEdit(thirdPage);
    m23 = new QNarrowLineEdit(thirdPage);
    m30 = new QNarrowLineEdit(thirdPage);
    m31 = new QNarrowLineEdit(thirdPage);
    m32 = new QNarrowLineEdit(thirdPage);
    m33 = new QNarrowLineEdit(thirdPage);
    thirdPageLayout->addWidget(m00, 1, 0);
    thirdPageLayout->addWidget(m01, 1, 1);
    thirdPageLayout->addWidget(m02, 1, 2);
    thirdPageLayout->addWidget(m03, 1, 3);
    thirdPageLayout->addWidget(m10, 2, 0);
    thirdPageLayout->addWidget(m11, 2, 1);
    thirdPageLayout->addWidget(m12, 2, 2);
    thirdPageLayout->addWidget(m13, 2, 3);
    thirdPageLayout->addWidget(m20, 3, 0);
    thirdPageLayout->addWidget(m21, 3, 1);
    thirdPageLayout->addWidget(m22, 3, 2);
    thirdPageLayout->addWidget(m23, 3, 3);
    thirdPageLayout->addWidget(m30, 4, 0);
    thirdPageLayout->addWidget(m31, 4, 1);
    thirdPageLayout->addWidget(m32, 4, 2);
    thirdPageLayout->addWidget(m33, 4, 3);
    connect(m00, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m01, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m02, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m03, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m10, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m11, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m12, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m13, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m20, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m21, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m22, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m23, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m30, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m31, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m32, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m33, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    linearInvert = new QCheckBox(tr("Invert linear transform"),
                                 thirdPage);
    thirdPageLayout->addWidget(linearInvert, 5, 0, 1, 2);
    connect(linearInvert, SIGNAL(toggled(bool)),
            this, SLOT(linearInvertChanged(bool)));

    transformVectors3 = new QCheckBox(tr("Transform vectors"), thirdPage);
    thirdPageLayout->addWidget(transformVectors3, 5, 2, 1, 2);
    connect(transformVectors3, SIGNAL(toggled(bool)),
            this, SLOT(transformVectorsChanged(bool)));

    thirdPageLayout->setRowStretch(6, 100);

    connect(transformTypeTabs, SIGNAL(currentChanged(int)),
            this, SLOT(pageTurned(int)));
}

// ****************************************************************************
//  Method:  QvisTransformWindow::UpdateWindow
//
//  Purpose:
//    Updates the widgets in the window when the subject changes.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 24, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//    Replaced simple QString::sprintf's with a setNum because there seems
//    to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//    Jeremy Meredith, Fri Feb  4 17:48:04 PST 2005
//    Added support for coordinate transforms.
//
//    Jeremy Meredith, Tue Apr 15 13:20:52 EDT 2008
//    Added support for linear transforms.
//
//    Brad Whitlock, Wed Aug 13 21:09:50 PDT 2008
//    Qt 4
//
//    Jeremy Meredith, Fri Aug  7 14:42:15 EDT 2009
//    Added vector transform method selection to coordinate system conversions.
//
//    Brad Whitlock, Wed Jul 14 15:18:44 PDT 2010
//    I added some more blockSignals calls to prevent slots from being called
//    as a result of setting widget values.
//
//    Tom Fogal, Tue Jul 27 11:06:46 MDT 2010
//    Add cases for 4x4 matrix elements.
//
//   Kathleen Biagas, Thu Apr 9 07:19:54 MST 2015
//   Use helper function DoubleToQString for consistency in formatting across
//   all windows.
//
// ****************************************************************************

void
QvisTransformWindow::UpdateWindow(bool doAll)
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
          case TransformAttributes::ID_doRotate:
            if (atts->GetDoRotate() == true)
            {
                rotateOrigin->setEnabled(true);
                rotateOriginLabel->setEnabled(true);
                rotateAxis->setEnabled(true);
                rotateAxisLabel->setEnabled(true);
                rotateAmount->setEnabled(true);
                rotateAmountLabel->setEnabled(true);
                rotateTypeWidget->setEnabled(true);
            }
            else
            {
                rotateOrigin->setEnabled(false);
                rotateOriginLabel->setEnabled(false);
                rotateAxis->setEnabled(false);
                rotateAxisLabel->setEnabled(false);
                rotateAmount->setEnabled(false);
                rotateAmountLabel->setEnabled(false);
                rotateTypeWidget->setEnabled(false);
            }
            doRotate->blockSignals(true);
            doRotate->setChecked(atts->GetDoRotate());
            doRotate->blockSignals(false);
            break;
          case TransformAttributes::ID_rotateOrigin:
            rotateOrigin->setText(DoublesToQString(atts->GetRotateOrigin(),3));
            break;
          case TransformAttributes::ID_rotateAxis:
            rotateAxis->setText(DoublesToQString(atts->GetRotateAxis(),3));
            break;
          case TransformAttributes::ID_rotateAmount:
            rotateAmount->setText(DoubleToQString(atts->GetRotateAmount()));
            break;
          case TransformAttributes::ID_rotateType:
            rotateType->blockSignals(true);
            rotateType->button(atts->GetRotateType())->setChecked(true);
            rotateType->blockSignals(false);
            break;
          case TransformAttributes::ID_doScale:
            if (atts->GetDoScale() == true)
            {
                scaleOrigin->setEnabled(true);
                scaleOriginLabel->setEnabled(true);
                scaleX->setEnabled(true);
                scaleXLabel->setEnabled(true);
                scaleY->setEnabled(true);
                scaleYLabel->setEnabled(true);
                scaleZ->setEnabled(true);
                scaleZLabel->setEnabled(true);
            }
            else
            {
                scaleOrigin->setEnabled(false);
                scaleOriginLabel->setEnabled(false);
                scaleX->setEnabled(false);
                scaleXLabel->setEnabled(false);
                scaleY->setEnabled(false);
                scaleYLabel->setEnabled(false);
                scaleZ->setEnabled(false);
                scaleZLabel->setEnabled(false);
            }
            doScale->blockSignals(true);
            doScale->setChecked(atts->GetDoScale());
            doScale->blockSignals(false);
            break;
          case TransformAttributes::ID_scaleOrigin:
            scaleOrigin->setText(DoublesToQString(atts->GetScaleOrigin(),3));
            break;
          case TransformAttributes::ID_scaleX:
            scaleX->setText(DoubleToQString(atts->GetScaleX()));
            break;
          case TransformAttributes::ID_scaleY:
            scaleY->setText(DoubleToQString(atts->GetScaleY()));
            break;
          case TransformAttributes::ID_scaleZ:
            scaleZ->setText(DoubleToQString(atts->GetScaleZ()));
            break;
          case TransformAttributes::ID_doTranslate:
            if (atts->GetDoTranslate() == true)
            {
                translateX->setEnabled(true);
                translateXLabel->setEnabled(true);
                translateY->setEnabled(true);
                translateYLabel->setEnabled(true);
                translateZ->setEnabled(true);
                translateZLabel->setEnabled(true);
            }
            else
            {
                translateX->setEnabled(false);
                translateXLabel->setEnabled(false);
                translateY->setEnabled(false);
                translateYLabel->setEnabled(false);
                translateZ->setEnabled(false);
                translateZLabel->setEnabled(false);
            }
            doTranslate->blockSignals(true);
            doTranslate->setChecked(atts->GetDoTranslate());
            doTranslate->blockSignals(false);
            break;
          case TransformAttributes::ID_translateX:
            translateX->setText(DoubleToQString(atts->GetTranslateX()));
            break;
          case TransformAttributes::ID_translateY:
            translateY->setText(DoubleToQString(atts->GetTranslateY()));
            break;
          case TransformAttributes::ID_translateZ:
            translateZ->setText(DoubleToQString(atts->GetTranslateZ()));
            break;
          case TransformAttributes::ID_transformType:
            if (atts->GetTransformType() == TransformAttributes::Similarity)
            {
                transformTypeTabs->setCurrentIndex(0);
            }
            else if (atts->GetTransformType() == TransformAttributes::Coordinate)
            {
                transformTypeTabs->setCurrentIndex(1);
            }
            else
            {
                transformTypeTabs->setCurrentIndex(2);
            }
            break;
          case TransformAttributes::ID_inputCoordSys:
            inputCoord->blockSignals(true);
            inputCoord->button(atts->GetInputCoordSys())->setChecked(true);
            inputCoord->blockSignals(false);

            continuousPhi->blockSignals(true);
            continuousPhi->setEnabled( (atts->GetInputCoordSys()  ==
                                        TransformAttributes::Cartesian &&
                                        atts->GetOutputCoordSys() ==
                                        TransformAttributes::Cylindrical) );
            continuousPhi->blockSignals(false);
            continuousPhiLabel->setEnabled( (atts->GetInputCoordSys()  ==
                                             TransformAttributes::Cartesian &&
                                             atts->GetOutputCoordSys() ==
                                             TransformAttributes::Cylindrical) );
            break;
          case TransformAttributes::ID_outputCoordSys:
            outputCoord->blockSignals(true);
            outputCoord->button(atts->GetOutputCoordSys())->setChecked(true);
            outputCoord->blockSignals(false);

            continuousPhi->blockSignals(true);
            continuousPhi->setEnabled( (atts->GetInputCoordSys()  ==
                                        TransformAttributes::Cartesian &&
                                        atts->GetOutputCoordSys() ==
                                        TransformAttributes::Cylindrical) );
            continuousPhi->blockSignals(false);
            continuousPhiLabel->setEnabled( (atts->GetInputCoordSys()  ==
                                             TransformAttributes::Cartesian &&
                                             atts->GetOutputCoordSys() ==
                                             TransformAttributes::Cylindrical) );
            break;
          case TransformAttributes::ID_continuousPhi:
            continuousPhi->blockSignals(true);
            continuousPhi->setChecked(atts->GetContinuousPhi());
            continuousPhi->blockSignals(false);
            break;
          case TransformAttributes::ID_m00:
            m00->setText(DoubleToQString(atts->GetM00()));
            break;
          case TransformAttributes::ID_m01:
            m01->setText(DoubleToQString(atts->GetM01()));
            break;
          case TransformAttributes::ID_m02:
            m02->setText(DoubleToQString(atts->GetM02()));
            break;
          case TransformAttributes::ID_m03:
            m03->setText(DoubleToQString(atts->GetM03()));
            break;

          case TransformAttributes::ID_m10:
            m10->setText(DoubleToQString(atts->GetM10()));
            break;
          case TransformAttributes::ID_m11:
            m11->setText(DoubleToQString(atts->GetM11()));
            break;
          case TransformAttributes::ID_m12:
            m12->setText(DoubleToQString(atts->GetM12()));
            break;
          case TransformAttributes::ID_m13:
            m13->setText(DoubleToQString(atts->GetM13()));
            break;

          case TransformAttributes::ID_m20:
            m20->setText(DoubleToQString(atts->GetM20()));
            break;
          case TransformAttributes::ID_m21:
            m21->setText(DoubleToQString(atts->GetM21()));
            break;
          case TransformAttributes::ID_m22:
            m22->setText(DoubleToQString(atts->GetM22()));
            break;
          case TransformAttributes::ID_m23:
            m23->setText(DoubleToQString(atts->GetM23()));
            break;

          case TransformAttributes::ID_m30:
            m30->setText(DoubleToQString(atts->GetM30()));
            break;
          case TransformAttributes::ID_m31:
            m31->setText(DoubleToQString(atts->GetM31()));
            break;
          case TransformAttributes::ID_m32:
            m32->setText(DoubleToQString(atts->GetM32()));
            break;
          case TransformAttributes::ID_m33:
            m33->setText(DoubleToQString(atts->GetM33()));
            break;

          case TransformAttributes::ID_invertLinearTransform:
            linearInvert->blockSignals(true);
            linearInvert->setChecked(atts->GetInvertLinearTransform());
            linearInvert->blockSignals(false);
            break;

          case TransformAttributes::ID_vectorTransformMethod:
            vectorMethodCombo->blockSignals(true);
            vectorMethodCombo->setCurrentIndex(
                                       int(atts->GetVectorTransformMethod()));
            vectorMethodCombo->blockSignals(false);

          case TransformAttributes::ID_transformVectors:
            transformVectors1->blockSignals(true);
            transformVectors3->blockSignals(true);
            transformVectors1->setChecked(atts->GetTransformVectors());
            transformVectors3->setChecked(atts->GetTransformVectors());
            transformVectors1->blockSignals(false);
            transformVectors3->blockSignals(false);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisTransformWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values for one or all of the lineEdit widgets.
//
// Arguments:
//   which_widget : The number of the widget to update. If -1 is passed,
//                  the routine gets the current values for all widgets.
//
// Modifications:
//    Kathleen Bonnell, Wed May 21 11:06:13 PDT 2003 
//    Disallow (0, 0, 0) as the rotation axis.
//
//    Jeremy Meredith, Tue Apr 15 13:42:44 EDT 2008
//    Added support for linear transforms.
//
//    Brad Whitlock, Thu Apr 24 15:53:37 PDT 2008
//    Added tr()'s
//
//    Brad Whitlock, Wed Aug 13 21:34:35 PDT 2008
//    Qt 4.
//
//    Gunther H. Weber, Mon Apr 30 16:32:24 PDT 2012
//    Copy all matrix elements (fourth row and fourth column were missing).
//
// ****************************************************************************

void
QvisTransformWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg;

    // Do rotateOrigin
    if(which_widget == TransformAttributes::ID_rotateOrigin || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(rotateOrigin, val, 3))
            atts->SetRotateOrigin(val);
        else
        {
            ResettingError(tr("rotate origin"), 
                DoublesToQString(atts->GetRotateOrigin(), 3));
            atts->SetRotateOrigin(atts->GetRotateOrigin());
        }
    }

    // Do rotateAxis
    if(which_widget == TransformAttributes::ID_rotateAxis || doAll)
    {
        double val[3];
        if((okay = LineEditGetDoubles(rotateAxis, val, 3)) == true)
        {
            okay = (val[0] != 0. || val[1] != 0. || val[2] != 0);
            if (okay)
                atts->SetRotateAxis(val);
        }

        if(!okay)
        {
            ResettingError(tr("rotate axis"),
                DoublesToQString(atts->GetRotateAxis(), 3));
            atts->SetRotateAxis(atts->GetRotateAxis());
        }
    }

    // Do rotateAmount
    if(which_widget == TransformAttributes::ID_rotateAmount || doAll)
    {
        double val;
        if(LineEditGetDouble(rotateAmount, val))
            atts->SetRotateAmount(val);
        else
        {
            ResettingError(tr("rotate amount"),
                DoubleToQString(atts->GetRotateAmount()));
            atts->SetRotateAmount(atts->GetRotateAmount());
        }
    }

    // Do scaleOrigin
    if(which_widget == TransformAttributes::ID_scaleOrigin || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(scaleOrigin, val, 3))
            atts->SetScaleOrigin(val);
        else
        {
            ResettingError(tr("scale origin"),
                DoublesToQString(atts->GetScaleOrigin(), 3));
            atts->SetScaleOrigin(atts->GetScaleOrigin());
        }
    }

    // Do scaleX
    if(which_widget == TransformAttributes::ID_scaleX || doAll)
    {
        double val;
        if(LineEditGetDouble(scaleX, val))
            atts->SetScaleX(val);
        else
        {
            ResettingError(tr("X scale"), DoubleToQString(atts->GetScaleX()));
            atts->SetScaleX(atts->GetScaleX());
        }
    }

    // Do scaleY
    if(which_widget == TransformAttributes::ID_scaleY || doAll)
    {
        double val;
        if(LineEditGetDouble(scaleY, val))
            atts->SetScaleY(val);
        else
        {
            ResettingError(tr("Y scale"), DoubleToQString(atts->GetScaleY()));
            atts->SetScaleY(atts->GetScaleY());
        }
    }

    // Do scaleZ
    if(which_widget == TransformAttributes::ID_scaleZ || doAll)
    {
        double val;
        if(LineEditGetDouble(scaleZ, val))
            atts->SetScaleZ(val);
        else
        {
            ResettingError(tr("Z scale"), DoubleToQString(atts->GetScaleZ()));
            atts->SetScaleZ(atts->GetScaleZ());
        }
    }

    // Do translateX
    if(which_widget == TransformAttributes::ID_translateX || doAll)
    {
        double val;
        if(LineEditGetDouble(translateX, val))
            atts->SetTranslateX(val);
        else
        {
            ResettingError(tr("translation in X"), DoubleToQString(atts->GetTranslateX()));
            atts->SetTranslateX(atts->GetTranslateX());
        }
    }

    // Do translateY
    if(which_widget == TransformAttributes::ID_translateY || doAll)
    {
        double val;
        if(LineEditGetDouble(translateY, val))
            atts->SetTranslateY(val);
        else
        {
            ResettingError(tr("translation in Y"), DoubleToQString(atts->GetTranslateY()));
            atts->SetTranslateY(atts->GetTranslateY());
        }
    }

    // Do translateZ
    if(which_widget == TransformAttributes::ID_translateZ || doAll)
    {
        double val;
        if(LineEditGetDouble(translateZ, val))
            atts->SetTranslateZ(val);
        else
        {
            ResettingError(tr("translation in Z"), DoubleToQString(atts->GetTranslateZ()));
            atts->SetTranslateZ(atts->GetTranslateZ());
        }
    }

    // Do linear transform matrix elements
    if (which_widget == 100 || doAll)
    {
        double val;
        if(LineEditGetDouble(m00, val))
            atts->SetM00(val);
        else
        {
            ResettingError(tr("Matrix 00"), DoubleToQString(atts->GetM00()));
            atts->SetM00(atts->GetM00());
        }
        if(LineEditGetDouble(m01, val))
            atts->SetM01(val);
        else
        {
            ResettingError(tr("Matrix 01"), DoubleToQString(atts->GetM01()));
            atts->SetM01(atts->GetM01());
        }
        if(LineEditGetDouble(m02, val))
            atts->SetM02(val);
        else
        {
            ResettingError(tr("Matrix 02"), DoubleToQString(atts->GetM02()));
            atts->SetM02(atts->GetM02());
        }
        if(LineEditGetDouble(m03, val))
            atts->SetM03(val);
        else
        {
            ResettingError(tr("Matrix 03"), DoubleToQString(atts->GetM03()));
            atts->SetM03(atts->GetM03());
        }

        if(LineEditGetDouble(m10, val))
            atts->SetM10(val);
        else
        {
            ResettingError(tr("Matrix 10"), DoubleToQString(atts->GetM10()));
            atts->SetM10(atts->GetM10());
        }
        if(LineEditGetDouble(m11, val))
            atts->SetM11(val);
        else
        {
            ResettingError(tr("Matrix 11"), DoubleToQString(atts->GetM11()));
            atts->SetM11(atts->GetM11());
        }
        if(LineEditGetDouble(m12, val))
            atts->SetM12(val);
        else
        {
            ResettingError(tr("Matrix 12"), DoubleToQString(atts->GetM12()));
            atts->SetM12(atts->GetM12());
        }
        if(LineEditGetDouble(m13, val))
            atts->SetM13(val);
        else
        {
            ResettingError(tr("Matrix 13"), DoubleToQString(atts->GetM13()));
            atts->SetM13(atts->GetM13());
        }

        if(LineEditGetDouble(m20, val))
            atts->SetM20(val);
        else
        {
            ResettingError(tr("Matrix 20"), DoubleToQString(atts->GetM20()));
            atts->SetM20(atts->GetM20());
        }
        if(LineEditGetDouble(m21, val))
            atts->SetM21(val);
        else
        {
            ResettingError(tr("Matrix 21"), DoubleToQString(atts->GetM21()));
            atts->SetM21(atts->GetM21());
        }
        if(LineEditGetDouble(m22, val))
            atts->SetM22(val);
        else
        {
            ResettingError(tr("Matrix 22"), DoubleToQString(atts->GetM22()));
            atts->SetM22(atts->GetM22());
        }
        if(LineEditGetDouble(m23, val))
            atts->SetM23(val);
        else
        {
            ResettingError(tr("Matrix 23"), DoubleToQString(atts->GetM23()));
            atts->SetM23(atts->GetM23());
        }

        if(LineEditGetDouble(m30, val))
            atts->SetM30(val);
        else
        {
            ResettingError(tr("Matrix 30"), DoubleToQString(atts->GetM30()));
            atts->SetM30(atts->GetM30());
        }
        if(LineEditGetDouble(m31, val))
            atts->SetM31(val);
        else
        {
            ResettingError(tr("Matrix 31"), DoubleToQString(atts->GetM31()));
            atts->SetM31(atts->GetM31());
        }
        if(LineEditGetDouble(m32, val))
            atts->SetM32(val);
        else
        {
            ResettingError(tr("Matrix 32"), DoubleToQString(atts->GetM32()));
            atts->SetM32(atts->GetM32());
        }
        if(LineEditGetDouble(m33, val))
            atts->SetM33(val);
        else
        {
            ResettingError(tr("Matrix 33"), DoubleToQString(atts->GetM33()));
            atts->SetM33(atts->GetM33());
        }
    }
}

//
// Qt Slot functions.
//

void
QvisTransformWindow::doRotateChanged(bool val)
{
    atts->SetDoRotate(val);
    Apply();
}


void
QvisTransformWindow::rotateOriginProcessText()
{
    GetCurrentValues(TransformAttributes::ID_rotateOrigin);
    Apply();
}


void
QvisTransformWindow::rotateAxisProcessText()
{
    GetCurrentValues(TransformAttributes::ID_rotateAxis);
    Apply();
}


void
QvisTransformWindow::rotateAmountProcessText()
{
    GetCurrentValues(TransformAttributes::ID_rotateAmount);
    Apply();
}


void
QvisTransformWindow::rotateTypeChanged(int val)
{
    TransformAttributes::AngleType val2;
    val2 = (val == 0) ? TransformAttributes::Deg : TransformAttributes::Rad;
    if(val2 != atts->GetRotateType())
    {
        atts->SetRotateType(val2);
        Apply();
    }
}


void
QvisTransformWindow::doScaleChanged(bool val)
{
    atts->SetDoScale(val);
    Apply();
}


void
QvisTransformWindow::scaleOriginProcessText()
{
    GetCurrentValues(TransformAttributes::ID_scaleOrigin);
    Apply();
}


void
QvisTransformWindow::scaleXProcessText()
{
    GetCurrentValues(TransformAttributes::ID_scaleX);
    Apply();
}


void
QvisTransformWindow::scaleYProcessText()
{
    GetCurrentValues(TransformAttributes::ID_scaleY);
    Apply();
}


void
QvisTransformWindow::scaleZProcessText()
{
    GetCurrentValues(TransformAttributes::ID_scaleZ);
    Apply();
}


void
QvisTransformWindow::doTranslateChanged(bool val)
{
    atts->SetDoTranslate(val);
    Apply();
}


void
QvisTransformWindow::translateXProcessText()
{
    GetCurrentValues(TransformAttributes::ID_translateX);
    Apply();
}


void
QvisTransformWindow::translateYProcessText()
{
    GetCurrentValues(TransformAttributes::ID_translateY);
    Apply();
}


void
QvisTransformWindow::translateZProcessText()
{
    GetCurrentValues(TransformAttributes::ID_translateZ);
    Apply();
}

void
QvisTransformWindow::pageTurned(int page)
{
    if (page == 0)
    {
        atts->SetTransformType(TransformAttributes::Similarity);
        Apply();
    }
    else if (page == 1)
    {
        atts->SetTransformType(TransformAttributes::Coordinate);
        Apply();
    }
    else if (page == 2)
    {
        atts->SetTransformType(TransformAttributes::Linear);
        Apply();
    }
}

void QvisTransformWindow::inputCoordChanged(int v)
{
    atts->SetInputCoordSys(TransformAttributes::CoordinateSystem(v));
    Apply();
}

void QvisTransformWindow::outputCoordChanged(int v)
{
    atts->SetOutputCoordSys(TransformAttributes::CoordinateSystem(v));
    Apply();
}

void
QvisTransformWindow::continuousPhiChanged(bool val)
{
    atts->SetContinuousPhi(val);
    Apply();
}


void
QvisTransformWindow::ltElementtChanged()
{
    GetCurrentValues(100);
    Apply();
}

void
QvisTransformWindow::linearInvertChanged(bool val)
{
    atts->SetInvertLinearTransform(val);
    Apply();
}

void
QvisTransformWindow::transformVectorsChanged(bool val)
{
    atts->SetTransformVectors(val);
    Apply();
}

void QvisTransformWindow::vectorMethodChanged(int v)
{
    atts->SetVectorTransformMethod(
                                TransformAttributes::VectorTransformMethod(v));
}
