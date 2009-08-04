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

#include "QvisTransformWindow.h"

#include <TransformAttributes.h>
#include <ViewerProxy.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QTabWidget>
#include <QWidget>

#include <QvisColorTableButton.h>
#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <stdio.h>

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
    rotateTypeLayout->setMargin(0);
    rotateTypeLayout->setSpacing(10);
    QRadioButton *rotateTypeAngleTypeDeg = new QRadioButton(tr("Deg"),rotateTypeWidget);
    rotateType->addButton(rotateTypeAngleTypeDeg, 0);
    rotateTypeLayout->addWidget(rotateTypeAngleTypeDeg);
    QRadioButton *rotateTypeAngleTypeRad = new QRadioButton(tr("Rad"),rotateTypeWidget);
    rotateType->addButton(rotateTypeAngleTypeRad, 1);
    rotateTypeLayout->addWidget(rotateTypeAngleTypeRad);
    connect(rotateType, SIGNAL(buttonClicked(int)),
            this, SLOT(rotateTypeChanged(int)));
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
    QRadioButton *iSph  = new QRadioButton(tr("Spherical (r,phi,theta)"), inputFrame);
    inputCoord->addButton(iSph, 2);
    inputCoordLayout->addWidget(iSph);

    QGroupBox *outputFrame = new QGroupBox(tr("Output coordinates"), secondPage);
    secondPageLayout->addWidget(outputFrame);

    outputCoord = new QButtonGroup(outputFrame);
    QVBoxLayout *outputCoordLayout = new QVBoxLayout(outputFrame);
    outputCoordLayout->setSpacing(10);
    QRadioButton *oCart = new QRadioButton(tr("Cartesian (x,y,z)"), outputFrame);
    outputCoord->addButton(oCart, 0);
    outputCoordLayout->addWidget(oCart);
    QRadioButton *oCyl  = new QRadioButton(tr("Cylindrical (r,phi,z)"), outputFrame);
    outputCoord->addButton(oCyl, 1);
    outputCoordLayout->addWidget(oCyl);
    QRadioButton *oSph  = new QRadioButton(tr("Spherical (r,phi,theta)"), outputFrame);
    outputCoord->addButton(oSph, 2);
    outputCoordLayout->addWidget(oSph);

    secondPageLayout->addStretch(100);

    connect(inputCoord, SIGNAL(buttonClicked(int)),
            this, SLOT(inputCoordChanged(int)));
    connect(outputCoord, SIGNAL(buttonClicked(int)),
            this, SLOT(outputCoordChanged(int)));

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
    m10 = new QNarrowLineEdit(thirdPage);
    m11 = new QNarrowLineEdit(thirdPage);
    m12 = new QNarrowLineEdit(thirdPage);
    m20 = new QNarrowLineEdit(thirdPage);
    m21 = new QNarrowLineEdit(thirdPage);
    m22 = new QNarrowLineEdit(thirdPage);
    thirdPageLayout->addWidget(m00, 1, 0);
    thirdPageLayout->addWidget(m01, 1, 1);
    thirdPageLayout->addWidget(m02, 1, 2);
    thirdPageLayout->addWidget(m10, 2, 0);
    thirdPageLayout->addWidget(m11, 2, 1);
    thirdPageLayout->addWidget(m12, 2, 2);
    thirdPageLayout->addWidget(m20, 3, 0);
    thirdPageLayout->addWidget(m21, 3, 1);
    thirdPageLayout->addWidget(m22, 3, 2);
    connect(m00, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m01, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m02, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m10, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m11, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m12, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m20, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m21, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    connect(m22, SIGNAL(returnPressed()), this, SLOT(ltElementtChanged()));
    linearInvert = new QCheckBox(tr("Invert linear transform"),
                                 thirdPage);
    thirdPageLayout->addWidget(linearInvert, 4, 0, 1, 3);
    connect(linearInvert, SIGNAL(toggled(bool)),
            this, SLOT(linearInvertChanged(bool)));
    thirdPageLayout->setRowStretch(5, 100);

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
// ****************************************************************************
void
QvisTransformWindow::UpdateWindow(bool doAll)
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
            doRotate->setChecked(atts->GetDoRotate());
            break;
          case TransformAttributes::ID_rotateOrigin:
            rotateOrigin->setText(FloatsToQString(atts->GetRotateOrigin(),3));
            break;
          case TransformAttributes::ID_rotateAxis:
            rotateAxis->setText(FloatsToQString(atts->GetRotateAxis(),3));
            break;
          case TransformAttributes::ID_rotateAmount:
            temp.setNum(atts->GetRotateAmount());
            rotateAmount->setText(temp);
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
            doScale->setChecked(atts->GetDoScale());
            break;
          case TransformAttributes::ID_scaleOrigin:
            scaleOrigin->setText(FloatsToQString(atts->GetScaleOrigin(),3));
            break;
          case TransformAttributes::ID_scaleX:
            temp.setNum(atts->GetScaleX());
            scaleX->setText(temp);
            break;
          case TransformAttributes::ID_scaleY:
            temp.setNum(atts->GetScaleY());
            scaleY->setText(temp);
            break;
          case TransformAttributes::ID_scaleZ:
            temp.setNum(atts->GetScaleZ());
            scaleZ->setText(temp);
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
            doTranslate->setChecked(atts->GetDoTranslate());
            break;
          case TransformAttributes::ID_translateX:
            temp.setNum(atts->GetTranslateX());
            translateX->setText(temp);
            break;
          case TransformAttributes::ID_translateY:
            temp.setNum(atts->GetTranslateY());
            translateY->setText(temp);
            break;
          case TransformAttributes::ID_translateZ:
            temp.setNum(atts->GetTranslateZ());
            translateZ->setText(temp);
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
            break;
          case TransformAttributes::ID_outputCoordSys:
            outputCoord->blockSignals(true);
            outputCoord->button(atts->GetOutputCoordSys())->setChecked(true);
            outputCoord->blockSignals(false);
            break;

          case TransformAttributes::ID_m00:
            temp.setNum(atts->GetM00());
            m00->setText(temp);
            break;
          case TransformAttributes::ID_m01:
            temp.setNum(atts->GetM01());
            m01->setText(temp);
            break;
          case TransformAttributes::ID_m02:
            temp.setNum(atts->GetM02());
            m02->setText(temp);
            break;

          case TransformAttributes::ID_m10:
            temp.setNum(atts->GetM10());
            m10->setText(temp);
            break;
          case TransformAttributes::ID_m11:
            temp.setNum(atts->GetM11());
            m11->setText(temp);
            break;
          case TransformAttributes::ID_m12:
            temp.setNum(atts->GetM12());
            m12->setText(temp);
            break;

          case TransformAttributes::ID_m20:
            temp.setNum(atts->GetM20());
            m20->setText(temp);
            break;
          case TransformAttributes::ID_m21:
            temp.setNum(atts->GetM21());
            m21->setText(temp);
            break;
          case TransformAttributes::ID_m22:
            temp.setNum(atts->GetM22());
            m22->setText(temp);
            break;

          case TransformAttributes::ID_invertLinearTransform:
            linearInvert->blockSignals(true);
            linearInvert->setChecked(atts->GetInvertLinearTransform());
            linearInvert->blockSignals(false);
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
// ****************************************************************************

void
QvisTransformWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do rotateOrigin
    if(which_widget == TransformAttributes::ID_rotateOrigin || doAll)
    {
        float val[3];
        if(LineEditGetFloats(rotateOrigin, val, 3))
            atts->SetRotateOrigin(val);
        else
        {
            ResettingError(tr("rotate origin"), 
                FloatsToQString(atts->GetRotateOrigin(), 3));
            atts->SetRotateOrigin(atts->GetRotateOrigin());
        }
    }

    // Do rotateAxis
    if(which_widget == TransformAttributes::ID_rotateAxis || doAll)
    {
        float val[3];
        if((okay = LineEditGetFloats(rotateAxis, val, 3)) == true)
        {
            okay = (val[0] != 0. || val[1] != 0. || val[2] != 0);
            if (okay)
                atts->SetRotateAxis(val);
        }

        if(!okay)
        {
            ResettingError(tr("rotate axis"),
                FloatsToQString(atts->GetRotateAxis(), 3));
            atts->SetRotateAxis(atts->GetRotateAxis());
        }
    }

    // Do rotateAmount
    if(which_widget == TransformAttributes::ID_rotateAmount || doAll)
    {
        float val;
        if(LineEditGetFloat(rotateAmount, val))
            atts->SetRotateAmount(val);
        else
        {
            ResettingError(tr("rotate amount"),
                FloatToQString(atts->GetRotateAmount()));
            atts->SetRotateAmount(atts->GetRotateAmount());
        }
    }

    // Do scaleOrigin
    if(which_widget == TransformAttributes::ID_scaleOrigin || doAll)
    {
        float val[3];
        if(LineEditGetFloats(scaleOrigin, val, 3))
            atts->SetScaleOrigin(val);
        else
        {
            ResettingError(tr("scale origin"),
                FloatsToQString(atts->GetScaleOrigin(), 3));
            atts->SetScaleOrigin(atts->GetScaleOrigin());
        }
    }

    // Do scaleX
    if(which_widget == TransformAttributes::ID_scaleX || doAll)
    {
        float val;
        if(LineEditGetFloat(scaleX, val))
            atts->SetScaleX(val);
        else
        {
            ResettingError(tr("X scale"), FloatToQString(atts->GetScaleX()));
            atts->SetScaleX(atts->GetScaleX());
        }
    }

    // Do scaleY
    if(which_widget == TransformAttributes::ID_scaleY || doAll)
    {
        float val;
        if(LineEditGetFloat(scaleY, val))
            atts->SetScaleY(val);
        else
        {
            ResettingError(tr("Y scale"), FloatToQString(atts->GetScaleY()));
            atts->SetScaleY(atts->GetScaleY());
        }
    }

    // Do scaleZ
    if(which_widget == TransformAttributes::ID_scaleZ || doAll)
    {
        float val;
        if(LineEditGetFloat(scaleZ, val))
            atts->SetScaleZ(val);
        else
        {
            ResettingError(tr("Z scale"), FloatToQString(atts->GetScaleZ()));
            atts->SetScaleZ(atts->GetScaleZ());
        }
    }

    // Do translateX
    if(which_widget == TransformAttributes::ID_translateX || doAll)
    {
        float val;
        if(LineEditGetFloat(translateX, val))
            atts->SetTranslateX(val);
        else
        {
            ResettingError(tr("translation in X"), FloatToQString(atts->GetTranslateX()));
            atts->SetTranslateX(atts->GetTranslateX());
        }
    }

    // Do translateY
    if(which_widget == TransformAttributes::ID_translateY || doAll)
    {
        float val;
        if(LineEditGetFloat(translateY, val))
            atts->SetTranslateY(val);
        else
        {
            ResettingError(tr("translation in Y"), FloatToQString(atts->GetTranslateY()));
            atts->SetTranslateY(atts->GetTranslateY());
        }
    }

    // Do translateZ
    if(which_widget == TransformAttributes::ID_translateZ || doAll)
    {
        float val;
        if(LineEditGetFloat(translateZ, val))
            atts->SetTranslateZ(val);
        else
        {
            ResettingError(tr("translation in Z"), FloatToQString(atts->GetTranslateZ()));
            atts->SetTranslateZ(atts->GetTranslateZ());
        }
    }

    // Do linear transform matrix elements
    if (which_widget == 100 || doAll)
    {
        atts->SetM00(m00->displayText().trimmed().toFloat());
        atts->SetM01(m01->displayText().trimmed().toFloat());
        atts->SetM02(m02->displayText().trimmed().toFloat());

        atts->SetM10(m10->displayText().trimmed().toFloat());
        atts->SetM11(m11->displayText().trimmed().toFloat());
        atts->SetM12(m12->displayText().trimmed().toFloat());

        atts->SetM20(m20->displayText().trimmed().toFloat());
        atts->SetM21(m21->displayText().trimmed().toFloat());
        atts->SetM22(m22->displayText().trimmed().toFloat());
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
}

void QvisTransformWindow::outputCoordChanged(int v)
{
    atts->SetOutputCoordSys(TransformAttributes::CoordinateSystem(v));
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

