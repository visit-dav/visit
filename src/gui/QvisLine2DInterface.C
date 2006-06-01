/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#include <QvisLine2DInterface.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qtooltip.h>

#include <AnnotationObject.h>

#include <QvisColorButton.h>
#include <QvisOpacitySlider.h>
#include <QvisScreenPositionEdit.h>

// ****************************************************************************
// Method: QvisLine2DInterface::QvisLine2DInterface
//
// Purpose: 
//   Constructor for the QvisLine2DInterface class.
//
// Arguments:
//   parent : This widget's parent widget.
//   name   : The name of this widget.
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:31:16 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Jun 28 13:36:41 PST 2005
//   Added code to make tool tips for the start, end coordinates.
//
// ****************************************************************************

QvisLine2DInterface::QvisLine2DInterface(QWidget *parent,
    const char *name) : QvisAnnotationObjectInterface(parent, name)
{
    // Set the title of the group box.
    this->setTitle(GetName());

    QGridLayout *cLayout = new QGridLayout(topLayout, 7, 4);
    cLayout->setSpacing(10);

    // Add controls for the start position
    positionStartEdit = new QvisScreenPositionEdit(this, "positionStartEdit");
    connect(positionStartEdit, SIGNAL(screenPositionChanged(double, double)),
            this, SLOT(positionStartChanged(double, double)));
    QLabel *startLabel = new QLabel(positionStartEdit, "Start", this);
    QString startTip("Start of line in screen coordinates [0,1]");
    QToolTip::add(startLabel, startTip);
    cLayout->addMultiCellWidget(positionStartEdit, 0, 0, 1, 3);
    cLayout->addWidget(startLabel, 0, 0);

    // Add controls for the end position
    positionEndEdit = new QvisScreenPositionEdit(this, "positionEndEdit");
    connect(positionEndEdit, SIGNAL(screenPositionChanged(double, double)),
            this, SLOT(positionEndChanged(double, double)));
    QLabel *endLabel = new QLabel(positionEndEdit, "End", this);
    QString endTip("End of line in screen coordinates [0,1]");
    QToolTip::add(endLabel, endTip);
    cLayout->addMultiCellWidget(positionEndEdit, 1, 1, 1, 3);
    cLayout->addWidget(endLabel, 1, 0);
   
    // Add controls for width.
    widthSpinBox = new QSpinBox(1, 100, 1, this, "widthSpinBox");
    widthSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
    connect(widthSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(widthChanged(int)));
    cLayout->addWidget(widthSpinBox, 2, 1);
    cLayout->addWidget(new QLabel(widthSpinBox, "Width", this), 2, 0);

    // Add controls for the line color.
    colorButton = new QvisColorButton(this, "colorButton");
    connect(colorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(colorChanged(const QColor &)));
    cLayout->addWidget(new QLabel(colorButton, "Line color", this),
                       3, 0, Qt::AlignLeft);
    cLayout->addWidget(colorButton, 3, 1);

    // Add controls for the line opacity.
    opacitySlider = new QvisOpacitySlider(0, 255, 10, 0, this,
                                          "opacitySlider");
    connect(opacitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(opacityChanged(int)));
    cLayout->addMultiCellWidget(opacitySlider, 3, 3, 2, 3);

    // Beginning arrow control.
    beginArrowComboBox = new QComboBox(this, "beginArrowControl");
    beginArrowComboBox->insertItem("None", 0);
    beginArrowComboBox->insertItem("Line", 1);
    beginArrowComboBox->insertItem("Solid", 2);
    beginArrowComboBox->setEditable(false);
    connect(beginArrowComboBox, SIGNAL(activated(int)),
            this, SLOT(beginArrowChanged(int)));
    cLayout->addMultiCellWidget(beginArrowComboBox, 4, 4, 1, 3);
    cLayout->addWidget(new QLabel("Begin Arrow", this), 4, 0);

    // Beginning arrow control.
    endArrowComboBox = new QComboBox(this, "endArrowControl");
    endArrowComboBox->insertItem("None", 0);
    endArrowComboBox->insertItem("Line", 1);
    endArrowComboBox->insertItem("Solid", 2);
    endArrowComboBox->setEditable(false);
    connect(endArrowComboBox, SIGNAL(activated(int)),
            this, SLOT(endArrowChanged(int)));
    cLayout->addMultiCellWidget(endArrowComboBox, 5, 5, 1, 3);
    cLayout->addWidget(new QLabel("End Arrow", this), 5, 0);

    // Added a visibility toggle
    visibleCheckBox = new QCheckBox("Visible", this, "visibleCheckBox");
    connect(visibleCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(visibilityToggled(bool)));
    cLayout->addWidget(visibleCheckBox, 6, 0);
}

// ****************************************************************************
// Method: QvisLine2DInterface::~QvisLine2DInterface
//
// Purpose: 
//   Destructor for the QvisLine2DInterface class.
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:31:23 PDT 2004
//
// Modifications:
//   
// ****************************************************************************
QvisLine2DInterface::~QvisLine2DInterface()
{
}

// ****************************************************************************
// Method: QvisLine2DInterface::GetMenuText
//
// Purpose: 
//   Returns the text to use in the annotation list box.
//
// Arguments:
//   annot : The annotation object to use for extra information.
//
// Returns:    The text to use in the annotation list box.
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:31:41 PDT 2004
//
// Modifications:
//   
// ****************************************************************************
QString
QvisLine2DInterface::GetMenuText(const AnnotationObject &annot) const
{
    QString retval;
    if(annot.GetText().size() > 0)
        retval.sprintf("%s - %s",
                       GetName().latin1(),
                       annot.GetText()[0].c_str());
    else
        retval = GetName();

    return retval;
}

// ****************************************************************************
// Method: QvisLine2DInterface::UpdateControls
//
// Purpose: 
//   Updates the controls in the interface using the data in the Annotation
//   object pointed to by the annot pointer.
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:31:46 PDT 2004
//
// Modifications:
//   
// ****************************************************************************
void
QvisLine2DInterface::UpdateControls()
{
    // Set the start position.
    positionStartEdit->setPosition(annot->GetPosition()[0],
                                   annot->GetPosition()[1]);
    
    // Set the end position.
    positionEndEdit->setPosition(annot->GetPosition2()[0],
                                 annot->GetPosition2()[1]);

    // Set the spinbox values for the width and height 
    widthSpinBox->blockSignals(true);
    widthSpinBox->setValue(annot->GetColor2().Red());
    widthSpinBox->blockSignals(false);

    // Set the begin and end arrow styles.
    beginArrowComboBox->blockSignals(true);
    endArrowComboBox->blockSignals(true);
    beginArrowComboBox->setCurrentItem(annot->GetColor2().Green());
    endArrowComboBox->setCurrentItem(annot->GetColor2().Blue());
    beginArrowComboBox->blockSignals(false);
    endArrowComboBox->blockSignals(false);

    // Change color and opacity.
    colorButton->blockSignals(true);
    opacitySlider->blockSignals(true);
    QColor tc(annot->GetColor1().Red(),
              annot->GetColor1().Green(),
              annot->GetColor1().Blue());
    colorButton->setButtonColor(tc);
    opacitySlider->setGradientColor(tc);
    opacitySlider->setValue(annot->GetColor1().Alpha());
    opacitySlider->blockSignals(false);
    colorButton->blockSignals(false);

    // Set the visible check box.
    visibleCheckBox->blockSignals(true);
    visibleCheckBox->setChecked(annot->GetVisible());
    visibleCheckBox->blockSignals(false);
}

// ****************************************************************************
// Method: QvisLine2DInterface::GetCurrentValues
//
// Purpose: 
//   Gets the current values for the text fields.
//
// Arguments:
//   which_widget : The widget for which we're getting the values. -1 for all.
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:32:04 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Mar 6 11:08:06 PDT 2006
//   I added code to make sure that the end points get recorded.
//
// ****************************************************************************

void
QvisLine2DInterface::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    if(which_widget == 0 || doAll)
    {
        // Get the new position
        GetScreenPosition(positionStartEdit, "Start");
    }

    if(which_widget == 1 || doAll)
    {
        // Get the new position
        GetScreenPosition2(positionEndEdit, "End");
    }

    if(which_widget == 2 || doAll)
    {
        // Get its new current value and store it in the atts.
        ForceSpinBoxUpdate(widthSpinBox);
        ColorAttribute ca;
        ca.SetRgb(widthSpinBox->value(),
                  annot->GetColor2().Green(),
                  annot->GetColor2().Blue());
        annot->SetColor2(ca);
    }
}

//
// Qt Slot functions
//

// ****************************************************************************
// Method: QvisLine2DInterface::positionChanged
//
// Purpose: 
//   This is a Qt slot function that is called when return is pressed in the 
//   position line edit.
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:32:19 PDT 2004
//
// Modifications:
//   
// ****************************************************************************
void
QvisLine2DInterface::positionStartChanged(double x, double y)
{
    double pos[] = {x, y, 0.};
    annot->SetPosition(pos);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLine2DInterface::positionChanged
//
// Purpose: 
//   This is a Qt slot function that is called when return is pressed in the 
//   position line edit.
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:32:25 PDT 2004
//
// Modifications:
//   
// ****************************************************************************
void
QvisLine2DInterface::positionEndChanged(double x, double y)
{
    double pos[] = {x, y, 0.};
    annot->SetPosition2(pos);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLine2DInterface::beginArrowChanged
//
// Purpose:
//   Called when the begin arrow is changed.
//
// Arguments:
//   i:    The type of arrow to use.
//
// Returns:
//
// Note:
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:32:34 PDT 2004
//
// Modifications:
//
// ****************************************************************************
void
QvisLine2DInterface::beginArrowChanged(int i)
{
    ColorAttribute ca;
    ca.SetRgb(annot->GetColor2().Red(),
              i,
              annot->GetColor2().Blue());
    annot->SetColor2(ca);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLine2DInterface::endArrowChanged
//
// Purpose:
//   Called when the end arrow is changed.
//
// Arguments:
//   i:    The type of arrow to use.
//
// Returns:
//
// Note:
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:32:34 PDT 2004
//
// Modifications:
//
// ****************************************************************************
void
QvisLine2DInterface::endArrowChanged(int i)
{
    ColorAttribute ca;
    ca.SetRgb(annot->GetColor2().Red(),
              annot->GetColor2().Green(),
              i);
    annot->SetColor2(ca);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLine2DInterface::widthChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the value of the width
//   spin box changes.
//
// Arguments:
//   w : The new width in percent.
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:33:26 PDT 2004
//
// Modifications:
//   
// ****************************************************************************
void
QvisLine2DInterface::widthChanged(int w)
{
    ColorAttribute ca;
    ca.SetRgb(w,
              annot->GetColor2().Green(),
              annot->GetColor2().Blue());
    annot->SetColor2(ca);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLine2DInterface::olorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new color is
//   selected.
//
// Arguments:
//   c : The new start color.
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:33:35 PDT 2004
//
// Modifications:
//   
// ****************************************************************************
void
QvisLine2DInterface::colorChanged(const QColor &c)
{
    int a = annot->GetColor1().Alpha();
    ColorAttribute tc(c.red(), c.green(), c.blue(), a);
    annot->SetColor1(tc);
    Apply();
}

// ****************************************************************************
// Method: QvisLine2DInterface::opacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new opacity is
//   selected.
//
// Arguments:
//   opacity : The new start opacity.
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:33:47 PDT 2004
//
// Modifications:
//   
// ****************************************************************************
void
QvisLine2DInterface::opacityChanged(int opacity)
{
    ColorAttribute tc(annot->GetColor1());
    tc.SetAlpha(opacity);
    annot->SetColor1(tc);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLine2DInterface::visibilityToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the visibility toggle is
//   changed.
//
// Arguments:
//   val : The visibility flag.
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:34:03 PDT 2004
//
// Modifications:
//   
// ****************************************************************************
void
QvisLine2DInterface::visibilityToggled(bool val)
{
    annot->SetVisible(val);
    SetUpdate(false);
    Apply();
}
