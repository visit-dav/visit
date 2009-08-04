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

#include <QvisTensorPlotWindow.h>
#include <QLayout> 
#include <QButtonGroup>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>

#include <TensorAttributes.h>
#include <ViewerProxy.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisColorButton.h>
#include <QvisColorTableButton.h>

// ****************************************************************************
// Method: QvisTensorPlotWindow::QvisTensorPlotWindow
//
// Purpose: 
//   Constructor for the QvisTensorPlotWindow class.
//
// Arguments:
//   type      : An identifier used to identify the plot type in the viewer.
//   _vecAtts  : The tensor attributes that the window observes.
//   caption   : The caption displayed in the window decorations.
//   shortName : The name used in the notepad.
//   notepad   : The notepad area where the window posts itself.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

QvisTensorPlotWindow::QvisTensorPlotWindow(const int type,
    TensorAttributes *_vecAtts, const QString &caption, const QString &shortName,
    QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(_vecAtts, caption, shortName, notepad)
{
    plotType = type;
    tensorAtts = _vecAtts;

    // Initialize parentless widgets.
    reduceButtonGroup = 0;
    colorButtonGroup = 0;
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::~QvisTensorPlotWindow
//
// Purpose: 
//   Destructor for the QvisTensorPlotWindow class.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// Modifications:
//   Cyrus Harrison, Wed Aug 27 08:54:49 PDT 2008
//   Made sure a button groups have parents, so we don't need to explicitly
//   delete.
//
// ****************************************************************************

QvisTensorPlotWindow::~QvisTensorPlotWindow()
{
    tensorAtts = 0;
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets that are in the window and sets
//   up their signals/slots.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// Modifications:
//   Eric Brugger, Wed Nov 24 11:43:13 PST 2004
//   Added scaleByMagnitude and autoScale.
//
//   Brad Whitlock, Wed Apr 23 12:04:43 PDT 2008
//   Added tr()'s
//
//   Brad Whitlock, Tue Aug 8 20:12:23 PST 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisTensorPlotWindow::CreateWindowContents()
{
    //
    // Create the color-related widgets.
    //
    colorGroupBox = new QGroupBox(central);
    colorGroupBox->setTitle(tr("Tensor color"));
    topLayout->addWidget(colorGroupBox);
    QGridLayout *cgLayout = new QGridLayout(colorGroupBox);
    cgLayout->setSpacing(10);
    cgLayout->setColumnStretch(1, 10);

    // Add the tensor color label.
    colorButtonGroup = new QButtonGroup(colorGroupBox);
    connect(colorButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(colorModeChanged(int)));
    QRadioButton *rb = new QRadioButton(tr("Eigenvalues"), colorGroupBox);
    colorButtonGroup->addButton(rb, 0);
    cgLayout->addWidget(rb, 0, 0);
    rb = new QRadioButton(tr("Constant"), colorGroupBox);
    rb->setChecked(true);
    colorButtonGroup->addButton(rb, 1);
    cgLayout->addWidget(rb, 1, 0);

    // Create the color-by-eigenvalues button.
    colorTableButton = new QvisColorTableButton(colorGroupBox);
    connect(colorTableButton, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    cgLayout->addWidget(colorTableButton, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);

    // Create the tensor color button.
    tensorColor = new QvisColorButton(colorGroupBox);
    tensorColor->setButtonColor(QColor(255, 0, 0));
    connect(tensorColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(tensorColorChanged(const QColor &)));
    cgLayout->addWidget(tensorColor, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

    //
    // Create the scale-related widgets.
    //
    scaleGroupBox = new QGroupBox(central);
    scaleGroupBox->setTitle(tr("Tensor scale"));
    topLayout->addWidget(scaleGroupBox);
    QGridLayout *sgLayout = new QGridLayout(scaleGroupBox);
    sgLayout->setSpacing(10);
    sgLayout->setColumnStretch(1, 10);

    // Add the scale line edit.
    scaleLineEdit = new QLineEdit(scaleGroupBox);
    connect(scaleLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processScaleText()));
    sgLayout->addWidget(scaleLineEdit, 0, 1);
    QLabel *scaleLabel = new QLabel(tr("Scale"), scaleGroupBox);
    scaleLabel->setBuddy(scaleLineEdit);
    sgLayout->addWidget(scaleLabel, 0, 0, Qt::AlignRight | Qt::AlignVCenter);

    // Add the scale by magnitude toggle button.
    scaleByMagnitudeToggle = new QCheckBox(tr("Scale by magnitude"), scaleGroupBox);
    connect(scaleByMagnitudeToggle, SIGNAL(clicked()), 
            this, SLOT(scaleByMagnitudeToggled()));
    sgLayout->addWidget(scaleByMagnitudeToggle, 1, 0, 1, 2);

    // Add the auto scale toggle button.
    autoScaleToggle = new QCheckBox(tr("Auto scale"), scaleGroupBox);
    connect(autoScaleToggle, SIGNAL(clicked()), this, SLOT(autoScaleToggled()));
    sgLayout->addWidget(autoScaleToggle, 2, 0, 1, 2);

    //
    // Create the reduce-related widgets.
    //
    reduceGroupBox = new QGroupBox(central);
    reduceGroupBox->setTitle(tr("Reduce by"));
    topLayout->addWidget(reduceGroupBox);
    QGridLayout *rgLayout = new QGridLayout(reduceGroupBox);
    rgLayout->setSpacing(10);
    rgLayout->setColumnStretch(1, 10);

    // Create the reduce button group.
    reduceButtonGroup = new QButtonGroup(reduceGroupBox);
    connect(reduceButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(reduceMethodChanged(int)));
    rb= new QRadioButton(tr("N tensors"), reduceGroupBox);
    rb->setChecked(true);
    reduceButtonGroup->addButton(rb, 0);
    rgLayout->addWidget(rb, 0, 0);
    rb = new QRadioButton(tr("Stride"), reduceGroupBox);
    reduceButtonGroup->addButton(rb, 1);
    rgLayout->addWidget(rb, 1, 0);

    // Add the N tensors line edit.
    nTensorsLineEdit = new QLineEdit(reduceGroupBox);
    connect(scaleLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processNTensorsText()));
    rgLayout->addWidget(nTensorsLineEdit, 0, 1);

    // Add the stride line edit.
    strideLineEdit = new QLineEdit(reduceGroupBox);
    connect(strideLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processStrideText()));
    rgLayout->addWidget(strideLineEdit, 1, 1);

    //
    // Add the toggle buttons
    //

    // Add the legend toggle button.
    legendToggle = new QCheckBox(tr("Legend"), central);
    connect(legendToggle, SIGNAL(clicked()), this, SLOT(legendToggled()));
    topLayout->addWidget(legendToggle);
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the window's subject is changed. The
//   subject tells this window what attributes changed and we put the
//   new values into those widgets.
//
// Arguments:
//   doAll : If this flag is true, update all the widgets regardless
//           of whether or not they are selected.
//
// Returns:    
//
// Note:       
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Eric Brugger, Wed Nov 24 11:43:13 PST 2004
//   Added scaleByMagnitude and autoScale.
//
//   Brad Whitlock, Tue Aug 8 20:12:23 PST 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisTensorPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < tensorAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!tensorAtts->IsSelected(i))
                continue;
        }

        switch(i)
        {
        case TensorAttributes::ID_useStride:
            reduceButtonGroup->blockSignals(true);
            reduceButtonGroup->button(tensorAtts->GetUseStride()?1:0)->setChecked(true);
            reduceButtonGroup->blockSignals(false);

            nTensorsLineEdit->setEnabled(!tensorAtts->GetUseStride());
            strideLineEdit->setEnabled(tensorAtts->GetUseStride());
            break;
        case TensorAttributes::ID_stride:
            strideLineEdit->setText(IntToQString(tensorAtts->GetStride()));
            break;
        case TensorAttributes::ID_nTensors:
            nTensorsLineEdit->setText(IntToQString(tensorAtts->GetNTensors()));
            break;
        case TensorAttributes::ID_scale:
            temp.setNum(tensorAtts->GetScale());
            scaleLineEdit->setText(temp);
            break;
        case TensorAttributes::ID_scaleByMagnitude:
            scaleByMagnitudeToggle->blockSignals(true);
            scaleByMagnitudeToggle->setChecked(tensorAtts->GetScaleByMagnitude());
            scaleByMagnitudeToggle->blockSignals(false);
            break;
        case TensorAttributes::ID_autoScale:
            autoScaleToggle->blockSignals(true);
            autoScaleToggle->setChecked(tensorAtts->GetAutoScale());
            autoScaleToggle->blockSignals(false);
            break;
        case TensorAttributes::ID_colorByEigenvalues:
            colorButtonGroup->blockSignals(true);
            colorButtonGroup->button(tensorAtts->GetColorByEigenvalues() ? 0 : 1)->setChecked(true);
            colorButtonGroup->blockSignals(false);
            break;
        case TensorAttributes::ID_useLegend:
            legendToggle->blockSignals(true);
            legendToggle->setChecked(tensorAtts->GetUseLegend());
            legendToggle->blockSignals(false);
            break;
        case TensorAttributes::ID_tensorColor:
            { // new scope
            QColor temp(tensorAtts->GetTensorColor().Red(),
                        tensorAtts->GetTensorColor().Green(),
                        tensorAtts->GetTensorColor().Blue());
            tensorColor->blockSignals(true);
            tensorColor->setButtonColor(temp);
            tensorColor->blockSignals(false);
            }
            break;
        case TensorAttributes::ID_colorTableName:
            colorTableButton->setColorTable(tensorAtts->GetColorTableName().c_str());
            break;
        }
    } // end for
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values for one or all of the lineEdit widgets.
//
// Arguments:
//   which_widget : The number of the widget to update. If -1 is passed,
//                  the routine gets the current values for all widgets.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// Modifications:
//   Brad Whitlock, Wed Apr 23 12:05:54 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Tue Aug 8 20:12:23 PST 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisTensorPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do the scale value.
    if(which_widget == TensorAttributes::ID_scale || doAll)
    {
        double val;
        if(LineEditGetDouble(scaleLineEdit, val))
            tensorAtts->SetScale(val);
        else
        {
            ResettingError("scale value", 
                DoubleToQString(tensorAtts->GetScale()));
            tensorAtts->SetScale(tensorAtts->GetScale());
        }
    }

    // Do the N tensors value.
    if(which_widget == TensorAttributes::ID_nTensors || doAll)
    {
        int val;
        if(LineEditGetInt(nTensorsLineEdit, val))
            tensorAtts->SetNTensors(val);
        else
        {
            ResettingError("number of tensors",
                IntToQString(tensorAtts->GetNTensors()));
            tensorAtts->SetNTensors(tensorAtts->GetNTensors());
        }
    }

    // Do the stride value.
    if(which_widget == TensorAttributes::ID_stride || doAll)
    {
        int val;
        if(LineEditGetInt(strideLineEdit, val))
            tensorAtts->SetStride(val);
        else
        {
            ResettingError("stride", IntToQString(tensorAtts->GetStride()));
            tensorAtts->SetStride(tensorAtts->GetStride());
        }
    }
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::Apply
//
// Purpose: 
//   This method applies the tensor attributes and optionally tells the viewer
//   to apply them to the plot.
//
// Arguments:
//   ignore : This flag, when true, tells the code to ignore the
//            AutoUpdate function and tell the viewer to apply the
//            tensor attributes.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current aslice attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        tensorAtts->Notify();

        // Tell the viewer to set the tensor attributes.
        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        tensorAtts->Notify();
}

//
// Qt Slot functions...
//

void
QvisTensorPlotWindow::apply()
{
    Apply(true);
}

void
QvisTensorPlotWindow::makeDefault()
{
    // Tell the viewer to set the default tensor attributes.
    GetCurrentValues(-1);
    tensorAtts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}

void
QvisTensorPlotWindow::reset()
{
    // Tell the viewer to reset the aslice attributes to the last
    // applied values.
    GetViewerMethods()->ResetPlotOptions(plotType);
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::tensorColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   tensor color.
//
// Arguments:
//   color : The new tensor color.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::tensorColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    tensorAtts->SetTensorColor(temp);
    tensorAtts->SetColorByEigenvalues(false);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::processScaleText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   scale line edit.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::processScaleText()
{
    GetCurrentValues(TensorAttributes::ID_scale);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::scaleByMagnitudeToggled
//
// Purpose:
//   This is a Qt slot function that is called when the user toggles the
//   window's scale by magnitude toggle button.
//
// Programmer: Eric Brugger
// Creation:   November 24, 2004
//
// ****************************************************************************

void
QvisTensorPlotWindow::scaleByMagnitudeToggled()
{
    tensorAtts->SetScaleByMagnitude(!tensorAtts->GetScaleByMagnitude());
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::autoScaleToggled
//
// Purpose:
//   This is a Qt slot function that is called when the user toggles the
//   window's auto scale toggle button.
//
// Programmer: Eric Brugger
// Creation:   November 24, 2004
//
// ****************************************************************************

void
QvisTensorPlotWindow::autoScaleToggled()
{
    tensorAtts->SetAutoScale(!tensorAtts->GetAutoScale());
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::reduceMethodChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   method used to reduce the number of tensors.
//
// Arguments:
//   index : The reduction method.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::reduceMethodChanged(int index)
{
    tensorAtts->SetUseStride(index != 0);
    Apply();   
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::processNTensorsText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   N tensors line edit.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::processNTensorsText()
{
    GetCurrentValues(TensorAttributes::ID_nTensors);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::processStrideText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   stride line edit.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::processStrideText()
{
    GetCurrentValues(TensorAttributes::ID_stride);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::legendToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the user toggles the
//   window's legend toggle button.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::legendToggled()
{
    tensorAtts->SetUseLegend(!tensorAtts->GetUseLegend());
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::colorByEigenvaluesToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the user toggles the
//   window's "color by eigenvalues" toggle button.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::colorModeChanged(int index)
{
    tensorAtts->SetColorByEigenvalues(index == 0);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::colorTableClicked
//
// Purpose: 
//   This is a Qt slot function that sets the desired color table name into
//   the tensor plot attributes.
//
// Arguments:
//   useDefault : If this is true, we want to use the default color table.
//   ctName     : The name of the color table to use if we're not going to
//                use the default.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::colorTableClicked(bool useDefault,
    const QString &ctName)
{
    tensorAtts->SetColorByEigenvalues(true);
    tensorAtts->SetColorTableName(ctName.toStdString());
    Apply();
}
