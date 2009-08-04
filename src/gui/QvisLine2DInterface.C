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

#include <QvisLine2DInterface.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QSpinBox>
#include <QToolTip>

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
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Mon Jul 21 10:39:10 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisLine2DInterface::QvisLine2DInterface(QWidget *parent) :
    QvisAnnotationObjectInterface(parent)
{
    // Set the title of the group box.
    this->setTitle(GetName());

    QGridLayout *cLayout = new QGridLayout(0);
    topLayout->addLayout(cLayout);
    cLayout->setSpacing(10);

    // Add controls for the start position
    positionStartEdit = new QvisScreenPositionEdit(this);
    connect(positionStartEdit, SIGNAL(screenPositionChanged(double, double)),
            this, SLOT(positionStartChanged(double, double)));
    QLabel *startLabel = new QLabel(tr("Start"), this);
    QString startTip(tr("Start of line in screen coordinates [0,1]"));
    startLabel->setToolTip(startTip);
    cLayout->addWidget(positionStartEdit, 0, 1, 1, 3);
    cLayout->addWidget(startLabel, 0, 0);

    // Add controls for the end position
    positionEndEdit = new QvisScreenPositionEdit(this);
    connect(positionEndEdit, SIGNAL(screenPositionChanged(double, double)),
            this, SLOT(positionEndChanged(double, double)));
    QLabel *endLabel = new QLabel(tr("End"), this);
    QString endTip(tr("End of line in screen coordinates [0,1]"));
    endLabel->setToolTip(endTip);
    cLayout->addWidget(positionEndEdit, 1, 1, 1, 3);
    cLayout->addWidget(endLabel, 1, 0);
   
    // Add controls for width.
    widthSpinBox = new QSpinBox(this);
    widthSpinBox->setMinimum(1);
    widthSpinBox->setMaximum(100);
    widthSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
    connect(widthSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(widthChanged(int)));
    cLayout->addWidget(widthSpinBox, 2, 1);
    cLayout->addWidget(new QLabel(tr("Width"), this), 2, 0);

    // Add controls for the line color.
    colorButton = new QvisColorButton(this);
    connect(colorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(colorChanged(const QColor &)));
    cLayout->addWidget(new QLabel(tr("Line color"), this),
                       3, 0, Qt::AlignLeft);
    cLayout->addWidget(colorButton, 3, 1);

    // Add controls for the line opacity.
    opacitySlider = new QvisOpacitySlider(0, 255, 10, 0, this);
    connect(opacitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(opacityChanged(int)));
    cLayout->addWidget(opacitySlider, 3, 2, 1, 2);

    // Beginning arrow control.
    beginArrowComboBox = new QComboBox(this);
    beginArrowComboBox->addItem(tr("None"));
    beginArrowComboBox->addItem(tr("Line"));
    beginArrowComboBox->addItem(tr("Solid"));
    beginArrowComboBox->setEditable(false);
    connect(beginArrowComboBox, SIGNAL(activated(int)),
            this, SLOT(beginArrowChanged(int)));
    cLayout->addWidget(beginArrowComboBox, 4, 1, 1, 3);
    cLayout->addWidget(new QLabel(tr("Begin arrow"), this), 4, 0);

    // Beginning arrow control.
    endArrowComboBox = new QComboBox(this);
    endArrowComboBox->addItem(tr("None"));
    endArrowComboBox->addItem(tr("Line"));
    endArrowComboBox->addItem(tr("Solid"));
    endArrowComboBox->setEditable(false);
    connect(endArrowComboBox, SIGNAL(activated(int)),
            this, SLOT(endArrowChanged(int)));
    cLayout->addWidget(endArrowComboBox, 5, 1, 1, 3);
    cLayout->addWidget(new QLabel(tr("End arrow"), this), 5, 0);

    // Added a visibility toggle
    visibleCheckBox = new QCheckBox(tr("Visible"), this);
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
//   Brad Whitlock, Mon Jul 21 10:45:24 PDT 2008
//   Qt 4.
//
// ****************************************************************************
QString
QvisLine2DInterface::GetMenuText(const AnnotationObject &annot) const
{
    QString retval;
    if(annot.GetText().size() > 0)
        retval = QString("%1 - %2").arg(GetName()).arg(annot.GetText()[0].c_str());
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
//   Brad Whitlock, Mon Jul 21 10:46:14 PDT 2008
//   Qt 4.
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
    beginArrowComboBox->setCurrentIndex(annot->GetColor2().Green());
    endArrowComboBox->setCurrentIndex(annot->GetColor2().Blue());
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
        GetScreenPosition(positionStartEdit, tr("Start"));
    }

    if(which_widget == 1 || doAll)
    {
        // Get the new position
        GetScreenPosition2(positionEndEdit, tr("End"));
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
