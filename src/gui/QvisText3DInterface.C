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

#include <QvisText3DInterface.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <QvisColorButton.h>
#include <QvisOpacitySlider.h>
#include <QvisScreenPositionEdit.h>

#include <AnnotationObject.h>

#define GetFacesCamera GetFontBold
#define SetFacesCamera SetFontBold

#define GetRotations GetPosition2
#define SetRotations SetPosition2

// The flag that lets us switch between relative and fixed heights.
#define GetRelativeHeightMode GetFontItalic
#define SetRelativeHeightMode SetFontItalic

// Relative scale will scale the text as a percentage of the bbox diagonal
#define GetRelativeHeight GetIntAttribute1
#define SetRelativeHeight SetIntAttribute1

// Fixed height in world coordinates.
#define GetFixedHeight   GetDoubleAttribute1
#define SetFixedHeight   SetDoubleAttribute1

// ****************************************************************************
// Method: QvisText3DInterface::QvisText3DInterface
//
// Purpose: 
//   Constructor for the QvisText3DInterface class.
//
// Arguments:
//   parent : This widget's parent widget.
//   name   : The name of this widget.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 11:46:42 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisText3DInterface::QvisText3DInterface(QWidget *parent,
    const char *name) : QvisAnnotationObjectInterface(parent, name)
{
    // Set the title of the group box.
    this->setTitle(GetName());

    QGridLayout *cLayout = new QGridLayout(topLayout, 11, 4);
    cLayout->setSpacing(10);

    int row = 0;

    // Add controls for entering the text
    textLineEdit = new QLineEdit(this, "textLineEdit");
    connect(textLineEdit, SIGNAL(returnPressed()),
            this, SLOT(textChanged()));
    cLayout->addMultiCellWidget(textLineEdit, row, row, 1, 3);
    cLayout->addWidget(new QLabel(textLineEdit, "Text",
        this), row, 0);
    ++row;

    // Add controls for the position
    positionEdit = new QLineEdit(this, "positionEdit");
    connect(positionEdit, SIGNAL(returnPressed()),
            this, SLOT(positionChanged()));
    cLayout->addMultiCellWidget(positionEdit, row, row, 1, 3);
    cLayout->addWidget(new QLabel(positionEdit, "Position",
        this), row, 0);
    ++row;

    // Add controls for the height.
    heightMode = new QButtonGroup(0, "heightMode");
    connect(heightMode, SIGNAL(clicked(int)),
            this, SLOT(heightModeChanged(int)));
    cLayout->addWidget(new QLabel("Height", this), row, 0);
    QRadioButton *rb = new QRadioButton("Relative", this, "relative");
    heightMode->insert(rb, 0);
    cLayout->addWidget(rb, row, 1);
    // Add controls for relative height
    relativeHeightSpinBox = new QSpinBox(1, 100, 1, this, "relativeHeightSpinBox");
    relativeHeightSpinBox->setSuffix("%");
    relativeHeightSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
    connect(relativeHeightSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(relativeHeightChanged(int)));
    cLayout->addWidget(relativeHeightSpinBox, row, 2);
    ++row;

    // Add controls for fixed height.
    rb = new QRadioButton("Fixed", this, "Fixed");
    heightMode->insert(rb, 1);
    cLayout->addWidget(rb, row, 1);
    fixedHeightEdit = new QLineEdit(this, "fixedHeightEdit");
    connect(fixedHeightEdit, SIGNAL(returnPressed()),
            this, SLOT(fixedHeightChanged()));
    cLayout->addWidget(fixedHeightEdit, row, 2);
    ++row;

    QFrame *splitter1 = new QFrame(this, "splitter");
    splitter1->setFrameStyle(QFrame::HLine + QFrame::Raised);
    cLayout->addMultiCellWidget(splitter1, row, row, 0, 3);  
    ++row;

    facesCameraCheckBox = new QCheckBox("Preserve orientation when view changes", this, "facesCameraCheckBox");
    connect(facesCameraCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(facesCameraToggled(bool)));
    cLayout->addMultiCellWidget(facesCameraCheckBox, row, row, 0, 3);
    ++row;

    rotateZ = new QSpinBox(-360, 360, 1, this, "rotateZ");
    rotateZ->setButtonSymbols(QSpinBox::PlusMinus);
    rotateZ->setSuffix(" deg");
    connect(rotateZ, SIGNAL(valueChanged(int)),
            this, SLOT(rotateZChanged(int)));
    QLabel *rotateZLabel = new QLabel(rotateZ, "Rotate Z", this, "rotateZLabel");
    rotateX = new QSpinBox(-360, 360, 1, this, "rotateX");
    rotateX->setButtonSymbols(QSpinBox::PlusMinus);
    rotateX->setSuffix(" deg");
    connect(rotateX, SIGNAL(valueChanged(int)),
            this, SLOT(rotateXChanged(int)));
    QLabel *rotateXLabel = new QLabel(rotateX, "Rotate X", this, "rotateXLabel");
    rotateY = new QSpinBox(-360, 360, 1, this, "rotateY");
    rotateY->setButtonSymbols(QSpinBox::PlusMinus);
    rotateY->setSuffix(" deg");
    connect(rotateY, SIGNAL(valueChanged(int)),
            this, SLOT(rotateYChanged(int)));
    QLabel *rotateYLabel = new QLabel(rotateY, "Rotate Y", this, "rotateYLabel");
    QGridLayout *rLayout = new QGridLayout(2, 3, 5, "rLayout");
    cLayout->addMultiCellLayout(rLayout, row, row, 0, 3);
    rLayout->addWidget(rotateYLabel, 0, 0);
    rLayout->addWidget(rotateXLabel, 0, 1);
    rLayout->addWidget(rotateZLabel, 0, 2);
    rLayout->addWidget(rotateY, 1, 0);
    rLayout->addWidget(rotateX, 1, 1);
    rLayout->addWidget(rotateZ, 1, 2);
    ++row;

    QFrame *splitter2 = new QFrame(this, "splitter2");
    splitter2->setFrameStyle(QFrame::HLine + QFrame::Raised);
    cLayout->addMultiCellWidget(splitter2, row, row, 0, 3);  
    ++row;

    // Add controls for the text color.
    textColorButton = new QvisColorButton(this, "textColorButton");
    connect(textColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(textColorChanged(const QColor &)));
    cLayout->addWidget(new QLabel(textColorButton, "Text color", this),
        row, 0, Qt::AlignLeft);
    cLayout->addWidget(textColorButton, row, 1);
    textColorOpacity = new QvisOpacitySlider(0, 255, 10, 0, this,
        "textColorOpacity");
    connect(textColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(textOpacityChanged(int)));
    cLayout->addMultiCellWidget(textColorOpacity, row, row, 2, 3);
    ++row;

    // Added a use foreground toggle
    useForegroundColorCheckBox = new QCheckBox("Use foreground color", this,
        "useForegroundColorCheckBox");
    connect(useForegroundColorCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(useForegroundColorToggled(bool)));
    cLayout->addMultiCellWidget(useForegroundColorCheckBox, row, row, 0, 3);
    ++row;

    // Added a visibility toggle
    visibleCheckBox = new QCheckBox("Visible", this, "visibleCheckBox");
    connect(visibleCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(visibilityToggled(bool)));
    cLayout->addWidget(visibleCheckBox, row, 0);
}

// ****************************************************************************
// Method: QvisText3DInterface::~QvisText3DInterface
//
// Purpose: 
//   Destructor for the QvisText3DInterface class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 11:47:58 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisText3DInterface::~QvisText3DInterface()
{
    delete heightMode;
}

// ****************************************************************************
// Method: QvisText3DInterface::GetMenuText
//
// Purpose: 
//   Returns the text to use in the annotation list box.
//
// Arguments:
//   annot : The annotation object to use for extra information.
//
// Returns:    The text to use in the annotation list box.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 16:06:47 PST 2007
//
// Modifications:
//   
// ****************************************************************************

QString
QvisText3DInterface::GetMenuText(const AnnotationObject &annot) const
{
    QString retval;
    if(annot.GetText().size() > 0)
        retval.sprintf("%s - %s", GetName().latin1(), annot.GetText()[0].c_str());
    else
        retval = GetName();

    return retval;
}

// ****************************************************************************
// Method: QvisText3DInterface::UpdateControls
//
// Purpose: 
//   Updates the controls in the interface using the data in the Annotation
//   object pointed to by the annot pointer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 11:48:15 PDT 2007
//
// Modifications:
//
// ****************************************************************************

void
QvisText3DInterface::UpdateControls()
{
    // Set the text
    const stringVector &annotText = annot->GetText();
    if(annotText.size() > 0)
        textLineEdit->setText(annotText[0].c_str());
    else
        textLineEdit->setText("");

    // Set the position.
    QString pos;
    pos.sprintf("%lg %lg %lg", 
        annot->GetPosition()[0],
        annot->GetPosition()[1],
        annot->GetPosition()[2]);
    positionEdit->setText(pos);

    // Set the height mode
    heightMode->blockSignals(true);
    heightMode->setButton(annot->GetRelativeHeightMode()?0:1);
    heightMode->blockSignals(false);
    relativeHeightSpinBox->setEnabled(annot->GetRelativeHeightMode());
    fixedHeightEdit->setEnabled(!annot->GetRelativeHeightMode());

    // Set the spinbox value for relative height.
    relativeHeightSpinBox->blockSignals(true);
    relativeHeightSpinBox->setValue(annot->GetRelativeHeight());
    relativeHeightSpinBox->blockSignals(false);

    // Set the value for the fixed height.
    QString tmp;
    tmp.sprintf("%lg", annot->GetFixedHeight());
    fixedHeightEdit->setText(tmp);

    // Set the faces camera check box.
    facesCameraCheckBox->blockSignals(true);
    facesCameraCheckBox->setChecked(annot->GetFacesCamera());
    facesCameraCheckBox->blockSignals(false);

    // Set the rotation values.
    int rx = (int)annot->GetRotations()[0];
    int ry = (int)annot->GetRotations()[1];
    int rz = (int)annot->GetRotations()[2];
    rotateX->blockSignals(true); rotateX->setValue(rx); rotateX->blockSignals(false); 
    rotateY->blockSignals(true); rotateY->setValue(ry); rotateY->blockSignals(false); 
    rotateZ->blockSignals(true); rotateZ->setValue(rz); rotateZ->blockSignals(false); 

    //
    // Set the text color. If we're using the foreground color for the text
    // color then make the button be white and only let the user change the 
    // opacity.
    //
    textColorOpacity->blockSignals(true);
    if(annot->GetUseForegroundForTextColor())
    {
        QColor tmp(255,255,255);
        textColorButton->setButtonColor(tmp);
        textColorButton->setEnabled(false);
        textColorOpacity->setGradientColor(tmp);
    }
    else
    {
        QColor tc(annot->GetTextColor().Red(),
                  annot->GetTextColor().Green(),
                  annot->GetTextColor().Blue());
        textColorButton->setButtonColor(tc);
        textColorButton->setEnabled(true);
        textColorOpacity->setGradientColor(tc);
    }
    textColorOpacity->setValue(annot->GetTextColor().Alpha());
    textColorOpacity->blockSignals(false);

    // Set the use foreground color check box.
    useForegroundColorCheckBox->blockSignals(true);
    useForegroundColorCheckBox->setChecked(annot->GetUseForegroundForTextColor());
    useForegroundColorCheckBox->blockSignals(false);

    // Set the visible check box.
    visibleCheckBox->blockSignals(true);
    visibleCheckBox->setChecked(annot->GetVisible());
    visibleCheckBox->blockSignals(false);
}

// ****************************************************************************
// Method: QvisText3DInterface::GetCurrentValues
//
// Purpose: 
//   Gets the current values for the text fields.
//
// Arguments:
//   which_widget : The widget for which we're getting the values. -1 for all.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 11:49:08 PDT 2007
//
// Modifications:
//
// ****************************************************************************

void
QvisText3DInterface::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    if(which_widget == 0 || doAll)
    {
        stringVector sv;
        sv.push_back(textLineEdit->text().latin1());
        annot->SetText(sv);
    }

    if(which_widget == 1 || doAll)
    {
        // Get the new position
        QString temp = positionEdit->displayText().stripWhiteSpace();
        bool okay = !temp.isEmpty();
        if (okay)
        {
            double v[3];
            if (sscanf(temp.latin1(), "%lg %lg %lg", &v[0], &v[1], &v[2]) == 3)
            {
                annot->SetPosition(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            QString msg;
            msg.sprintf("The position must be specified as a 3D coordinate. "
                "Resetting to the last good value of %lg %lg %lg.",
                annot->GetPosition()[0],annot->GetPosition()[1],annot->GetPosition()[2]);
            Error(msg);
            annot->SetPosition(annot->GetPosition());
        }  
    }

    if(which_widget == 2 || doAll)
    {
        // Get its new current value and store it in the atts.
        ForceSpinBoxUpdate(relativeHeightSpinBox);
        int h = relativeHeightSpinBox->value();
        annot->SetRelativeHeight(h);
    }

    if(which_widget == 3 || doAll)
    {
         // Get the new fixedHeightEdit
        QString temp = fixedHeightEdit->displayText().stripWhiteSpace();
        bool okay = !temp.isEmpty();
        if (okay)
        {
            double v;
            if (sscanf(temp.latin1(), "%lg", &v) == 1)
            {
                okay = v > 0.;
                if(okay)
                    annot->SetFixedHeight(v);
            }
            else
                okay = false;
        }

        if (!okay)
        {
            QString msg;
            msg.sprintf("The fixed height is a floating point number greater than zero. "
                "Resetting to the last good value of %lg.",
                annot->GetFixedHeight());
            Error(msg);
            annot->SetFixedHeight(annot->GetFixedHeight());
        }
    }

    if(which_widget == 4 || doAll)
    {
        // Get its new current value and store it in the atts.
        ForceSpinBoxUpdate(rotateZ);
        int r = rotateZ->value();
        double rotations[3];
        rotations[0] = annot->GetRotations()[0];
        rotations[1] = annot->GetRotations()[1];
        rotations[2] = (double)r;
        annot->SetRotations(rotations);
    }

    if(which_widget == 5 || doAll)
    {
        // Get its new current value and store it in the atts.
        ForceSpinBoxUpdate(rotateX);
        int r = rotateX->value();
        double rotations[3];
        rotations[0] = (double)r;
        rotations[1] = annot->GetRotations()[1];
        rotations[2] = annot->GetRotations()[2];
        annot->SetRotations(rotations);
    }

    if(which_widget == 6 || doAll)
    {
        // Get its new current value and store it in the atts.
        ForceSpinBoxUpdate(rotateY);
        int r = rotateY->value();
        double rotations[3];
        rotations[0] = annot->GetRotations()[0];
        rotations[1] = (double)r;
        rotations[2] = annot->GetRotations()[2];
        annot->SetRotations(rotations);
    }
}

//
// Qt Slot functions
//

// ****************************************************************************
// Method: QvisText3DInterface::textChanged
//
// Purpose: 
//   This is a Qt slot function that is called when return is pressed in the 
//   text line edit.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 11:49:46 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisText3DInterface::textChanged()
{
    GetCurrentValues(0);
    Apply();
}

// ****************************************************************************
// Method: QvisText3DInterface::positionChanged
//
// Purpose: 
//   This is a Qt slot function that is called when return is pressed in the 
//   position line edit.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 11:49:46 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisText3DInterface::positionChanged()
{
    GetCurrentValues(1);
    Apply();
}

void
QvisText3DInterface::heightModeChanged(int mode)
{
    bool doRelative = mode == 0;
    annot->SetRelativeHeightMode(doRelative);
    Apply();
}

void
QvisText3DInterface::relativeHeightChanged(int h)
{
    annot->SetRelativeHeight(h);
    SetUpdate(false);
    Apply();
}

void
QvisText3DInterface::fixedHeightChanged()
{
    GetCurrentValues(3);
    Apply();
}

// ****************************************************************************
// Method: QvisText3DInterface::facesCameraToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the faces camera checkbox
//   is toggled.
//
// Arguments:
//   val : The new bold flag.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 8 16:01:03 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisText3DInterface::facesCameraToggled(bool val)
{
    annot->SetFacesCamera(val);
    SetUpdate(false);
    Apply();
}

void
QvisText3DInterface::rotateZChanged(int)
{
    GetCurrentValues(4);
    Apply();
}

void
QvisText3DInterface::rotateXChanged(int)
{
    GetCurrentValues(5);
    Apply();
}

void
QvisText3DInterface::rotateYChanged(int)
{
    GetCurrentValues(6);
    Apply();
}

// ****************************************************************************
// Method: QvisText3DInterface::textColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new start color is
//   selected.
//
// Arguments:
//   c : The new start color.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 11:49:46 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisText3DInterface::textColorChanged(const QColor &c)
{
    int a = annot->GetTextColor().Alpha();
    ColorAttribute tc(c.red(), c.green(), c.blue(), a);
    annot->SetTextColor(tc);
    Apply();
}

// ****************************************************************************
// Method: QvisText3DInterface::textOpacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new start opacity is
//   selected.
//
// Arguments:
//   opacity : The new start opacity.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 11:49:46 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisText3DInterface::textOpacityChanged(int opacity)
{
    ColorAttribute tc(annot->GetTextColor());
    tc.SetAlpha(opacity);
    annot->SetTextColor(tc);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisText3DInterface::useForegroundColorToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the useForegroundColor
//   check box is clicked.
//
// Arguments:
//   val : The new setting for useForegroundColor
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 12:34:48 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisText3DInterface::useForegroundColorToggled(bool val)
{
    annot->SetUseForegroundForTextColor(val);
    Apply();
}

// ****************************************************************************
// Method: QvisText3DInterface::visibilityToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the visibility toggle is
//   changed.
//
// Arguments:
//   val : The visibility flag.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 11:49:46 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisText3DInterface::visibilityToggled(bool val)
{
    annot->SetVisible(val);
    SetUpdate(false);
    Apply();
}
