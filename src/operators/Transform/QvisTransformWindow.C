/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qvgroupbox.h>
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
// ****************************************************************************

void
QvisTransformWindow::CreateWindowContents()
{
    transformTypeTabs = new QTabWidget(central, "transformTypeTabs");
    topLayout->addWidget(transformTypeTabs);

    // ----------------------------------------------------------------------
    // First page
    // ----------------------------------------------------------------------
    firstPage = new QFrame(central, "firstPage");
    transformTypeTabs->addTab(firstPage, tr("Arbitrary"));

    QGridLayout *mainLayout = new QGridLayout(firstPage, 9,7, 10, 10, "mainLayout");
    mainLayout->addColSpacing(0, 15);

    // ---------------
    doRotate = new QCheckBox(tr("Rotate"), firstPage, "doRotate");
    connect(doRotate, SIGNAL(toggled(bool)),
            this, SLOT(doRotateChanged(bool)));
    mainLayout->addMultiCellWidget(doRotate, 0,0, 0,2);

    rotateOriginLabel = new QLabel(tr("Origin"), firstPage, "rotateOriginLabel");
    mainLayout->addWidget(rotateOriginLabel,1,1, Qt::AlignRight);
    rotateOrigin = new QLineEdit(firstPage, "rotateOrigin");
    connect(rotateOrigin, SIGNAL(returnPressed()),
            this, SLOT(rotateOriginProcessText()));
    mainLayout->addMultiCellWidget(rotateOrigin, 1,1, 2,6);

    rotateAxisLabel = new QLabel(tr("Axis"), firstPage, "rotateAxisLabel");
    mainLayout->addWidget(rotateAxisLabel,2,1, Qt::AlignRight);
    rotateAxis = new QLineEdit(firstPage, "rotateAxis");
    connect(rotateAxis, SIGNAL(returnPressed()),
            this, SLOT(rotateAxisProcessText()));
    mainLayout->addMultiCellWidget(rotateAxis, 2,2 ,2,6);

    rotateAmountLabel = new QLabel(tr("Amount"), firstPage, "rotateAmountLabel");
    mainLayout->addWidget(rotateAmountLabel,3,1, Qt::AlignRight);
    rotateAmount = new QNarrowLineEdit(firstPage, "rotateAmount");
    connect(rotateAmount, SIGNAL(returnPressed()),
            this, SLOT(rotateAmountProcessText()));
    mainLayout->addMultiCellWidget(rotateAmount, 3,3, 2,3);

    rotateType = new QButtonGroup(firstPage, "rotateType");
    rotateType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *rotateTypeLayout = new QHBoxLayout(rotateType);
    rotateTypeLayout->setSpacing(10);
    QRadioButton *rotateTypeAngleTypeDeg = new QRadioButton(tr("Deg"), rotateType);
    rotateTypeLayout->addWidget(rotateTypeAngleTypeDeg);
    QRadioButton *rotateTypeAngleTypeRad = new QRadioButton(tr("Rad"), rotateType);
    rotateTypeLayout->addWidget(rotateTypeAngleTypeRad);
    connect(rotateType, SIGNAL(clicked(int)),
            this, SLOT(rotateTypeChanged(int)));
    mainLayout->addMultiCellWidget(rotateType, 3,3, 4,6);

    // ---------------
    doScale = new QCheckBox(tr("Scale"), firstPage, "doScale");
    connect(doScale, SIGNAL(toggled(bool)),
            this, SLOT(doScaleChanged(bool)));
    mainLayout->addMultiCellWidget(doScale, 4,4, 0,2);

    scaleOriginLabel = new QLabel(tr("Origin"), firstPage, "scaleOriginLabel");
    mainLayout->addWidget(scaleOriginLabel,5,1, Qt::AlignRight);
    scaleOrigin = new QLineEdit(firstPage, "scaleOrigin");
    connect(scaleOrigin, SIGNAL(returnPressed()),
            this, SLOT(scaleOriginProcessText()));
    mainLayout->addMultiCellWidget(scaleOrigin, 5,5, 2,6);

    scaleXLabel = new QLabel(tr("X"), firstPage, "scaleXLabel");
    mainLayout->addWidget(scaleXLabel,6,1, Qt::AlignRight);
    scaleX = new QNarrowLineEdit(firstPage, "scaleX");
    connect(scaleX, SIGNAL(returnPressed()),
            this, SLOT(scaleXProcessText()));
    mainLayout->addWidget(scaleX, 6,2);

    scaleYLabel = new QLabel(tr("Y"), firstPage, "scaleYLabel");
    mainLayout->addWidget(scaleYLabel,6,3, Qt::AlignRight);
    scaleY = new QNarrowLineEdit(firstPage, "scaleY");
    connect(scaleY, SIGNAL(returnPressed()),
            this, SLOT(scaleYProcessText()));
    mainLayout->addWidget(scaleY, 6,4);

    scaleZLabel = new QLabel(tr("Z"), firstPage, "scaleZLabel");
    mainLayout->addWidget(scaleZLabel,6,5, Qt::AlignRight);
    scaleZ = new QNarrowLineEdit(firstPage, "scaleZ");
    connect(scaleZ, SIGNAL(returnPressed()),
            this, SLOT(scaleZProcessText()));
    mainLayout->addWidget(scaleZ, 6,6);

    // ---------------
    doTranslate = new QCheckBox(tr("Translate"), firstPage, "doTranslate");
    connect(doTranslate, SIGNAL(toggled(bool)),
            this, SLOT(doTranslateChanged(bool)));
    mainLayout->addMultiCellWidget(doTranslate, 7,7, 0,2);

    translateXLabel = new QLabel(tr("X"), firstPage, "translateXLabel");
    mainLayout->addWidget(translateXLabel,8,1, Qt::AlignRight);
    translateX = new QNarrowLineEdit(firstPage, "translateX");
    connect(translateX, SIGNAL(returnPressed()),
            this, SLOT(translateXProcessText()));
    mainLayout->addWidget(translateX, 8,2);

    translateYLabel = new QLabel(tr("Y"), firstPage, "translateYLabel");
    mainLayout->addWidget(translateYLabel,8,3, Qt::AlignRight);
    translateY = new QNarrowLineEdit(firstPage, "translateY");
    connect(translateY, SIGNAL(returnPressed()),
            this, SLOT(translateYProcessText()));
    mainLayout->addWidget(translateY, 8,4);

    translateZLabel = new QLabel(tr("Z"), firstPage, "translateZLabel");
    mainLayout->addWidget(translateZLabel,8,5, Qt::AlignRight);
    translateZ = new QNarrowLineEdit(firstPage, "translateZ");
    connect(translateZ, SIGNAL(returnPressed()),
            this, SLOT(translateZProcessText()));
    mainLayout->addWidget(translateZ, 8,6);

    // ----------------------------------------------------------------------
    // Second page
    // ----------------------------------------------------------------------
    secondPage = new QFrame(central, "secondPage");
    transformTypeTabs->addTab(secondPage, "Coordinate");

    QVBoxLayout *secondPageLayout = new QVBoxLayout(secondPage, 10, 10, "secondPageLayout");

    QVGroupBox *inputFrame = new QVGroupBox(tr("Input coordinates"), secondPage, "inputFrame");
    secondPageLayout->addWidget(inputFrame);
    inputFrame->setFrameStyle(QFrame::Box | QFrame::Sunken );

    inputCoord = new QButtonGroup(inputFrame, "inputCoord");
    inputCoord->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *inputCoordLayout = new QVBoxLayout(inputCoord,0,0);
    inputCoordLayout->setSpacing(10);
    QRadioButton *iCart = new QRadioButton(tr("Cartesian (x,y,z)"), inputCoord);
    inputCoordLayout->addWidget(iCart);
    QRadioButton *iCyl  = new QRadioButton(tr("Cylindrical (r,phi,z)"), inputCoord);
    inputCoordLayout->addWidget(iCyl);
    QRadioButton *iSph  = new QRadioButton(tr("Spherical (r,phi,theta)"), inputCoord);
    inputCoordLayout->addWidget(iSph);

    QVGroupBox *outputFrame = new QVGroupBox(tr("Output coordinates"), secondPage, "outputFrame");
    secondPageLayout->addWidget(outputFrame);
    outputFrame->setFrameStyle(QFrame::Box | QFrame::Sunken );

    outputCoord = new QButtonGroup(outputFrame, "outputCoord");
    outputCoord->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *outputCoordLayout = new QVBoxLayout(outputCoord,0,0);
    outputCoordLayout->setSpacing(10);
    QRadioButton *oCart = new QRadioButton(tr("Cartesian (x,y,z)"), outputCoord);
    outputCoordLayout->addWidget(oCart);
    QRadioButton *oCyl  = new QRadioButton(tr("Cylindrical (r,phi,z)"), outputCoord);
    outputCoordLayout->addWidget(oCyl);
    QRadioButton *oSph  = new QRadioButton(tr("Spherical (r,phi,theta)"), outputCoord);
    outputCoordLayout->addWidget(oSph);

    secondPageLayout->addStretch(100);

    connect(inputCoord, SIGNAL(clicked(int)),
            this, SLOT(inputCoordChanged(int)));
    connect(outputCoord, SIGNAL(clicked(int)),
            this, SLOT(outputCoordChanged(int)));

    // ----------------------------------------------------------------------
    // Third page
    // ----------------------------------------------------------------------
    thirdPage = new QFrame(central, "thirdPage");
    transformTypeTabs->addTab(thirdPage, tr("Linear"));

    QGridLayout *thirdPageLayout = new QGridLayout(thirdPage, 6,3, 10, 10, "thirdPageLayout");
    thirdPageLayout->addMultiCellWidget(new QLabel(tr("Matrix elements"),thirdPage),
                                        0,0, 0,2);
    m00 = new QNarrowLineEdit(thirdPage, tr("m00"));
    m01 = new QNarrowLineEdit(thirdPage, tr("m01"));
    m02 = new QNarrowLineEdit(thirdPage, tr("m02"));
    m10 = new QNarrowLineEdit(thirdPage, tr("m10"));
    m11 = new QNarrowLineEdit(thirdPage, tr("m11"));
    m12 = new QNarrowLineEdit(thirdPage, tr("m12"));
    m20 = new QNarrowLineEdit(thirdPage, tr("m20"));
    m21 = new QNarrowLineEdit(thirdPage, tr("m21"));
    m22 = new QNarrowLineEdit(thirdPage, tr("m22"));
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
                                 thirdPage, "linearInvert");
    thirdPageLayout->addMultiCellWidget(linearInvert, 4,4, 0,2);
    connect(linearInvert, SIGNAL(toggled(bool)),
            this, SLOT(linearInvertChanged(bool)));


    connect(transformTypeTabs, SIGNAL(currentChanged(QWidget*)),
            this, SLOT(pageTurned(QWidget*)));
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

        const float          *fptr;
        switch(i)
        {
          case 0: //doRotate
            if (atts->GetDoRotate() == true)
            {
                rotateOrigin->setEnabled(true);
                rotateOriginLabel->setEnabled(true);
                rotateAxis->setEnabled(true);
                rotateAxisLabel->setEnabled(true);
                rotateAmount->setEnabled(true);
                rotateAmountLabel->setEnabled(true);
                rotateType->setEnabled(true);
            }
            else
            {
                rotateOrigin->setEnabled(false);
                rotateOriginLabel->setEnabled(false);
                rotateAxis->setEnabled(false);
                rotateAxisLabel->setEnabled(false);
                rotateAmount->setEnabled(false);
                rotateAmountLabel->setEnabled(false);
                rotateType->setEnabled(false);
            }
            doRotate->setChecked(atts->GetDoRotate());
            break;
          case 1: //rotateOrigin
            fptr = atts->GetRotateOrigin();
            temp.sprintf("%g %g %g", fptr[0], fptr[1], fptr[2]);
            rotateOrigin->setText(temp);
            break;
          case 2: //rotateAxis
            fptr = atts->GetRotateAxis();
            temp.sprintf("%g %g %g", fptr[0], fptr[1], fptr[2]);
            rotateAxis->setText(temp);
            break;
          case 3: //rotateAmount
            temp.setNum(atts->GetRotateAmount());
            rotateAmount->setText(temp);
            break;
          case 4: //rotateType
            rotateType->setButton(atts->GetRotateType());
            break;
          case 5: //doScale
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
          case 6: //scaleOrigin
            fptr = atts->GetScaleOrigin();
            temp.sprintf("%g %g %g", fptr[0], fptr[1], fptr[2]);
            scaleOrigin->setText(temp);
            break;
          case 7: //scaleX
            temp.setNum(atts->GetScaleX());
            scaleX->setText(temp);
            break;
          case 8: //scaleY
            temp.setNum(atts->GetScaleY());
            scaleY->setText(temp);
            break;
          case 9: //scaleZ
            temp.setNum(atts->GetScaleZ());
            scaleZ->setText(temp);
            break;
          case 10: //doTranslate
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
          case 11: //translateX
            temp.setNum(atts->GetTranslateX());
            translateX->setText(temp);
            break;
          case 12: //translateY
            temp.setNum(atts->GetTranslateY());
            translateY->setText(temp);
            break;
          case 13: //translateZ
            temp.setNum(atts->GetTranslateZ());
            translateZ->setText(temp);
            break;
          case 14: // transformType
            if (atts->GetTransformType() == TransformAttributes::Similarity)
            {
                transformTypeTabs->showPage(firstPage);
            }
            else if (atts->GetTransformType() == TransformAttributes::Coordinate)
            {
                transformTypeTabs->showPage(secondPage);
            }
            else
            {
                transformTypeTabs->showPage(thirdPage);
            }
            break;
          case 15: // inputCoordSys
            inputCoord->setButton(atts->GetInputCoordSys());
            break;
          case 16: // outputCoordSys
            outputCoord->setButton(atts->GetOutputCoordSys());
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
            linearInvert->setChecked(atts->GetInvertLinearTransform());
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
//    Jeremy Meredith, Thu Aug  7 15:36:20 EDT 2008
//    Corrected an if(a=b) to if(a==b).
//
// ****************************************************************************

void
QvisTransformWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do doRotate
    if(which_widget == 0 || doAll)
    {
        // Nothing for doRotate
    }

    // Do rotateOrigin
    if(which_widget == 1 || doAll)
    {
        temp = rotateOrigin->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val[3];
            okay = (sscanf(temp.latin1(), "%g %g %g", 
                           &val[0], &val[1], &val[2]) == 3);
            if (okay)
                atts->SetRotateOrigin(val);
        }

        if(!okay)
        {
            const float *val = atts->GetRotateOrigin();
            QString num; num.sprintf("<%g %g %g>", val[0], val[1], val[2]);
            msg = tr("The value of rotateOrigin was invalid. "
                     "Resetting to the last good value of %1.").arg(num);
            Message(msg);
            atts->SetRotateOrigin(atts->GetRotateOrigin());
        }
    }

    // Do rotateAxis
    if(which_widget == 2 || doAll)
    {
        temp = rotateAxis->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val[3];
            okay = (sscanf(temp.latin1(), "%g %g %g", 
                           &val[0], &val[1], &val[2]) == 3);
            if (okay)
            {
                okay = (val[0] != 0. || val[1] != 0. || val[2] != 0);
                if (okay)
                    atts->SetRotateAxis(val);
            }
        }

        if(!okay)
        {
            const float *val = atts->GetRotateAxis();
            QString num; num.sprintf("<%g %g %g>", val[0], val[1], val[2]);
            msg = tr("The value of rotateAxis was invalid. "
                     "Resetting to the last good value of %1.").arg(num);
            Message(msg);
            atts->SetRotateAxis(atts->GetRotateAxis());
        }
    }

    // Do rotateAmount
    if(which_widget == 3 || doAll)
    {
        temp = rotateAmount->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            if(okay)
                atts->SetRotateAmount(val);
        }

        if(!okay)
        {
            msg = tr("The value of rotateAmount was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetRotateAmount());
            Message(msg);
            atts->SetRotateAmount(atts->GetRotateAmount());
        }
    }

    // Do rotateType
    if(which_widget == 4 || doAll)
    {
        // Nothing for rotateType
    }

    // Do doScale
    if(which_widget == 5 || doAll)
    {
        // Nothing for doScale
    }

    // Do scaleOrigin
    if(which_widget == 6 || doAll)
    {
        temp = scaleOrigin->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val[3];
            okay = (sscanf(temp.latin1(), "%g %g %g", 
                    &val[0], &val[1], &val[2]) == 3);
            if (okay)
                atts->SetScaleOrigin(val);
        }

        if(!okay)
        {
            const float *val = atts->GetScaleOrigin();
            QString num; num.sprintf("<%g %g %g>", val[0], val[1], val[2]);
            msg = tr("The value of scaleOrigin was invalid. "
                     "Resetting to the last good value of %1.").arg(num);
            Message(msg);
            atts->SetScaleOrigin(atts->GetScaleOrigin());
        }
    }

    // Do scaleX
    if(which_widget == 7 || doAll)
    {
        temp = scaleX->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetScaleX(val);
        }

        if(!okay)
        {
            msg = tr("The value of scaleX was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetScaleX());
            Message(msg);
            atts->SetScaleX(atts->GetScaleX());
        }
    }

    // Do scaleY
    if(which_widget == 8 || doAll)
    {
        temp = scaleY->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetScaleY(val);
        }

        if(!okay)
        {
            msg = tr("The value of scaleY was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetScaleY());
            Message(msg);
            atts->SetScaleY(atts->GetScaleY());
        }
    }

    // Do scaleZ
    if(which_widget == 9 || doAll)
    {
        temp = scaleZ->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetScaleZ(val);
        }

        if(!okay)
        {
            msg = tr("The value of scaleZ was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetScaleZ());
            Message(msg);
            atts->SetScaleZ(atts->GetScaleZ());
        }
    }

    // Do doTranslate
    if(which_widget == 10 || doAll)
    {
        // Nothing for doTranslate
    }

    // Do translateX
    if(which_widget == 11 || doAll)
    {
        temp = translateX->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetTranslateX(val);
        }

        if(!okay)
        {
            msg = tr("The value of translateX was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetTranslateX());
            Message(msg);
            atts->SetTranslateX(atts->GetTranslateX());
        }
    }

    // Do translateY
    if(which_widget == 12 || doAll)
    {
        temp = translateY->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetTranslateY(val);
        }

        if(!okay)
        {
            msg = tr("The value of translateY was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetTranslateY());
            Message(msg);
            atts->SetTranslateY(atts->GetTranslateY());
        }
    }

    // Do translateZ
    if(which_widget == 13 || doAll)
    {
        temp = translateZ->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            atts->SetTranslateZ(val);
        }

        if(!okay)
        {
            msg = tr("The value of translateZ was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetTranslateZ());
            Message(msg);
            atts->SetTranslateZ(atts->GetTranslateZ());
        }
    }

    // Do linear transform matrix elements
    if (which_widget == 100 || doAll)
    {
        atts->SetM00(m00->displayText().simplifyWhiteSpace().toFloat());
        atts->SetM01(m01->displayText().simplifyWhiteSpace().toFloat());
        atts->SetM02(m02->displayText().simplifyWhiteSpace().toFloat());

        atts->SetM10(m10->displayText().simplifyWhiteSpace().toFloat());
        atts->SetM11(m11->displayText().simplifyWhiteSpace().toFloat());
        atts->SetM12(m12->displayText().simplifyWhiteSpace().toFloat());

        atts->SetM20(m20->displayText().simplifyWhiteSpace().toFloat());
        atts->SetM21(m21->displayText().simplifyWhiteSpace().toFloat());
        atts->SetM22(m22->displayText().simplifyWhiteSpace().toFloat());
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
    GetCurrentValues(1);
    Apply();
}


void
QvisTransformWindow::rotateAxisProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisTransformWindow::rotateAmountProcessText()
{
    GetCurrentValues(3);
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
    GetCurrentValues(6);
    Apply();
}


void
QvisTransformWindow::scaleXProcessText()
{
    GetCurrentValues(7);
    Apply();
}


void
QvisTransformWindow::scaleYProcessText()
{
    GetCurrentValues(8);
    Apply();
}


void
QvisTransformWindow::scaleZProcessText()
{
    GetCurrentValues(9);
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
    GetCurrentValues(11);
    Apply();
}


void
QvisTransformWindow::translateYProcessText()
{
    GetCurrentValues(12);
    Apply();
}


void
QvisTransformWindow::translateZProcessText()
{
    GetCurrentValues(13);
    Apply();
}

void
QvisTransformWindow::pageTurned(QWidget *page)
{
    if (page == firstPage)
    {
        atts->SetTransformType(TransformAttributes::Similarity);
        Apply();
    }
    else if (page == secondPage)
    {
        atts->SetTransformType(TransformAttributes::Coordinate);
        Apply();
    }
    else if (page == thirdPage)
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

