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
                         const char *caption,
                         const char *shortName,
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
    transformTypeTabs->addTab(firstPage, "Arbitrary");

    QGridLayout *mainLayout = new QGridLayout(firstPage, 9,7, 10, 10, "mainLayout");
    mainLayout->addColSpacing(0, 15);

    // ---------------
    doRotate = new QCheckBox("Rotate", firstPage, "doRotate");
    connect(doRotate, SIGNAL(toggled(bool)),
            this, SLOT(doRotateChanged(bool)));
    mainLayout->addMultiCellWidget(doRotate, 0,0, 0,2);

    rotateOriginLabel = new QLabel("Origin", firstPage, "rotateOriginLabel");
    mainLayout->addWidget(rotateOriginLabel,1,1, Qt::AlignRight);
    rotateOrigin = new QLineEdit(firstPage, "rotateOrigin");
    connect(rotateOrigin, SIGNAL(returnPressed()),
            this, SLOT(rotateOriginProcessText()));
    mainLayout->addMultiCellWidget(rotateOrigin, 1,1, 2,6);

    rotateAxisLabel = new QLabel("Axis", firstPage, "rotateAxisLabel");
    mainLayout->addWidget(rotateAxisLabel,2,1, Qt::AlignRight);
    rotateAxis = new QLineEdit(firstPage, "rotateAxis");
    connect(rotateAxis, SIGNAL(returnPressed()),
            this, SLOT(rotateAxisProcessText()));
    mainLayout->addMultiCellWidget(rotateAxis, 2,2 ,2,6);

    rotateAmountLabel = new QLabel("Amount", firstPage, "rotateAmountLabel");
    mainLayout->addWidget(rotateAmountLabel,3,1, Qt::AlignRight);
    rotateAmount = new QNarrowLineEdit(firstPage, "rotateAmount");
    connect(rotateAmount, SIGNAL(returnPressed()),
            this, SLOT(rotateAmountProcessText()));
    mainLayout->addMultiCellWidget(rotateAmount, 3,3, 2,3);

    rotateType = new QButtonGroup(firstPage, "rotateType");
    rotateType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *rotateTypeLayout = new QHBoxLayout(rotateType);
    rotateTypeLayout->setSpacing(10);
    QRadioButton *rotateTypeAngleTypeDeg = new QRadioButton("Deg", rotateType);
    rotateTypeLayout->addWidget(rotateTypeAngleTypeDeg);
    QRadioButton *rotateTypeAngleTypeRad = new QRadioButton("Rad", rotateType);
    rotateTypeLayout->addWidget(rotateTypeAngleTypeRad);
    connect(rotateType, SIGNAL(clicked(int)),
            this, SLOT(rotateTypeChanged(int)));
    mainLayout->addMultiCellWidget(rotateType, 3,3, 4,6);

    // ---------------
    doScale = new QCheckBox("Scale", firstPage, "doScale");
    connect(doScale, SIGNAL(toggled(bool)),
            this, SLOT(doScaleChanged(bool)));
    mainLayout->addMultiCellWidget(doScale, 4,4, 0,2);

    scaleOriginLabel = new QLabel("Origin", firstPage, "scaleOriginLabel");
    mainLayout->addWidget(scaleOriginLabel,5,1, Qt::AlignRight);
    scaleOrigin = new QLineEdit(firstPage, "scaleOrigin");
    connect(scaleOrigin, SIGNAL(returnPressed()),
            this, SLOT(scaleOriginProcessText()));
    mainLayout->addMultiCellWidget(scaleOrigin, 5,5, 2,6);

    scaleXLabel = new QLabel("X", firstPage, "scaleXLabel");
    mainLayout->addWidget(scaleXLabel,6,1, Qt::AlignRight);
    scaleX = new QNarrowLineEdit(firstPage, "scaleX");
    connect(scaleX, SIGNAL(returnPressed()),
            this, SLOT(scaleXProcessText()));
    mainLayout->addWidget(scaleX, 6,2);

    scaleYLabel = new QLabel("Y", firstPage, "scaleYLabel");
    mainLayout->addWidget(scaleYLabel,6,3, Qt::AlignRight);
    scaleY = new QNarrowLineEdit(firstPage, "scaleY");
    connect(scaleY, SIGNAL(returnPressed()),
            this, SLOT(scaleYProcessText()));
    mainLayout->addWidget(scaleY, 6,4);

    scaleZLabel = new QLabel("Z", firstPage, "scaleZLabel");
    mainLayout->addWidget(scaleZLabel,6,5, Qt::AlignRight);
    scaleZ = new QNarrowLineEdit(firstPage, "scaleZ");
    connect(scaleZ, SIGNAL(returnPressed()),
            this, SLOT(scaleZProcessText()));
    mainLayout->addWidget(scaleZ, 6,6);

    // ---------------
    doTranslate = new QCheckBox("Translate", firstPage, "doTranslate");
    connect(doTranslate, SIGNAL(toggled(bool)),
            this, SLOT(doTranslateChanged(bool)));
    mainLayout->addMultiCellWidget(doTranslate, 7,7, 0,2);

    translateXLabel = new QLabel("X", firstPage, "translateXLabel");
    mainLayout->addWidget(translateXLabel,8,1, Qt::AlignRight);
    translateX = new QNarrowLineEdit(firstPage, "translateX");
    connect(translateX, SIGNAL(returnPressed()),
            this, SLOT(translateXProcessText()));
    mainLayout->addWidget(translateX, 8,2);

    translateYLabel = new QLabel("Y", firstPage, "translateYLabel");
    mainLayout->addWidget(translateYLabel,8,3, Qt::AlignRight);
    translateY = new QNarrowLineEdit(firstPage, "translateY");
    connect(translateY, SIGNAL(returnPressed()),
            this, SLOT(translateYProcessText()));
    mainLayout->addWidget(translateY, 8,4);

    translateZLabel = new QLabel("Z", firstPage, "translateZLabel");
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

    QVGroupBox *inputFrame = new QVGroupBox("Input coordinates", secondPage, "inputFrame");
    secondPageLayout->addWidget(inputFrame);
    inputFrame->setFrameStyle(QFrame::Box | QFrame::Sunken );

    inputCoord = new QButtonGroup(inputFrame, "inputCoord");
    inputCoord->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *inputCoordLayout = new QVBoxLayout(inputCoord,0,0);
    inputCoordLayout->setSpacing(10);
    QRadioButton *iCart = new QRadioButton("Cartesian", inputCoord);
    inputCoordLayout->addWidget(iCart);
    QRadioButton *iCyl  = new QRadioButton("Cylindrical", inputCoord);
    inputCoordLayout->addWidget(iCyl);
    QRadioButton *iSph  = new QRadioButton("Spherical", inputCoord);
    inputCoordLayout->addWidget(iSph);

    QVGroupBox *outputFrame = new QVGroupBox("Output coordinates", secondPage, "outputFrame");
    secondPageLayout->addWidget(outputFrame);
    outputFrame->setFrameStyle(QFrame::Box | QFrame::Sunken );

    outputCoord = new QButtonGroup(outputFrame, "outputCoord");
    outputCoord->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *outputCoordLayout = new QVBoxLayout(outputCoord,0,0);
    outputCoordLayout->setSpacing(10);
    QRadioButton *oCart = new QRadioButton("Cartesian", outputCoord);
    outputCoordLayout->addWidget(oCart);
    QRadioButton *oCyl  = new QRadioButton("Cylindrical", outputCoord);
    outputCoordLayout->addWidget(oCyl);
    QRadioButton *oSph  = new QRadioButton("Spherical", outputCoord);
    outputCoordLayout->addWidget(oSph);

    secondPageLayout->addStretch(100);

    connect(inputCoord, SIGNAL(clicked(int)),
            this, SLOT(inputCoordChanged(int)));
    connect(outputCoord, SIGNAL(clicked(int)),
            this, SLOT(outputCoordChanged(int)));

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
            else
            {
                transformTypeTabs->showPage(secondPage);
            }
            break;
          case 15: // inputCoordSys
            inputCoord->setButton(atts->GetInputCoordSys());
            break;
          case 16: // outputCoordSys
            outputCoord->setButton(atts->GetOutputCoordSys());
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
//   Kathleen Bonnell, Wed May 21 11:06:13 PDT 2003 
//   Disallow (0, 0, 0) as the rotation axis.
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
            msg.sprintf("The value of rotateOrigin was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
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
            msg.sprintf("The value of rotateAxis was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
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
            atts->SetRotateAmount(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of rotateAmount was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetRotateAmount());
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
            msg.sprintf("The value of scaleOrigin was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
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
            msg.sprintf("The value of scaleX was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetScaleX());
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
            msg.sprintf("The value of scaleY was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetScaleY());
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
            msg.sprintf("The value of scaleZ was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetScaleZ());
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
            msg.sprintf("The value of translateX was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetTranslateX());
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
            msg.sprintf("The value of translateY was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetTranslateY());
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
            msg.sprintf("The value of translateZ was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetTranslateZ());
            Message(msg);
            atts->SetTranslateZ(atts->GetTranslateZ());
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
}

void QvisTransformWindow::inputCoordChanged(int v)
{
    atts->SetInputCoordSys(TransformAttributes::CoordinateSystem(v));
}

void QvisTransformWindow::outputCoordChanged(int v)
{
    atts->SetOutputCoordSys(TransformAttributes::CoordinateSystem(v));
}

