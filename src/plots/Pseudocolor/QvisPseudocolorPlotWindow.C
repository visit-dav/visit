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

#include <QWidget> 
#include <QLayout> 
#include <QPushButton> 
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox> 
#include <QLineEdit>

#include <QvisPseudocolorPlotWindow.h>
#include <PseudocolorAttributes.h>
#include <ViewerProxy.h>
#include <QvisOpacitySlider.h>
#include <QvisColorTableButton.h>
#include <QvisPointControl.h>

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::QvisPseudocolorPlotWindow
//
// Purpose: 
//   Constructor for the QvisPseudocolorPlotWindow class.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 1 17:05:42 PST 2000
//
// Modifications:
//    Eric Brugger, Wed Mar 14 06:59:33 PST 2001
//    I added the argument type.
//   
// ****************************************************************************

QvisPseudocolorPlotWindow::QvisPseudocolorPlotWindow(const int type,
    PseudocolorAttributes *_pcAtts, const QString &caption, const QString &shortName,
    QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(_pcAtts, caption, shortName, notepad)
{
    plotType = type;
    pcAtts   = _pcAtts;
    centeringButtons = 0;
    scalingButtons = 0;
    smoothingLevelButtons = 0;
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::~QvisPseudocolorPlotWindow
//
// Purpose: 
//   Destructor for the QvisPseudocolorPlotWindow class.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 1 17:06:01 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisPseudocolorPlotWindow::~QvisPseudocolorPlotWindow()
{
    pcAtts = 0;
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets that are in the window and sets
//   up their signals/slots.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 1 17:06:30 PST 2000
//
// Modifications:
//   Kathleen Bonnell, Wed Dec 13 13:17:26 PST 2000
//   Added opacity slider.
//
//   Brad Whitlock, Sat Jun 16 12:43:40 PDT 2001
//   Added the color table button.
//
//   Kathleen Bonnell, Thu Oct  4 16:28:16 PDT 2001 
//   Added the limits selection combo box. 
//
//   Kathleen Bonnell, Wed May 29 13:40:22 PDT 2002 
//   Removed 'Specify' from limitsSelect. 
//
//   Jeremy Meredith, Tue Dec 10 10:23:07 PST 2002
//   Added smoothing options.
//
//   Jeremy Meredith, Fri Dec 20 11:36:03 PST 2002
//   Added scaling of point variables by a scalar field.
//
//   Hank Childs, Thu Aug 21 21:36:38 PDT 2003
//   Added point type options.
//
//   Jeremy Meredith, Tue May  4 13:23:10 PDT 2004
//   Added support for a new (Point) type of glyphing for point meshes.
//
//   Kathleen Bonnell, Fri Nov 12 11:25:23 PST 2004 
//   Replace individual point-size related widgets with QvisPointControl 
//
//   Brad Whitlock, Wed Jul 20 14:27:00 PST 2005
//   Added pointSizePixelsChanged slot.
//
//   Brad Whitlock, Wed Apr 23 10:16:57 PDT 2008
//   Added tr()'s
//
//   Brad Whitlock, Tue May 27 14:52:08 PDT 2008
//   Qt 4.
//
//   Dave Pugmire, Wed Oct 29 16:00:48 EDT 2008
//   Swap the min/max in the gui.
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::CreateWindowContents()
{
    //
    // Create the centering radio buttons
    //
    QHBoxLayout *centeringLayout = new QHBoxLayout(0);
    centeringLayout->setMargin(0);
    topLayout->addLayout(centeringLayout);

    // Create a group of radio buttons
    centeringButtons = new QButtonGroup(central);
    QLabel *centeringLabel = new QLabel(tr("Centering"), central);
    centeringLayout->addWidget(centeringLabel);
    
    // Create the radio buttons
    QRadioButton *rb= new QRadioButton(tr("Natural"), central );
    rb->setChecked(true);
    centeringButtons->addButton(rb, 0);
    centeringLayout->addWidget(rb);
    rb = new QRadioButton(tr("Nodal"), central );
    centeringLayout->addWidget(rb);
    centeringButtons->addButton(rb, 1);
    rb = new QRadioButton(tr("Zonal"), central );
    centeringLayout->addWidget(rb);
    centeringButtons->addButton(rb, 2);
    centeringLayout->addStretch(0);
    // Each time a radio button is clicked, call the centeringClicked slot.
    connect(centeringButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(centeringClicked(int)));
    
    //
    // Create the Limits stuff
    //
    QGridLayout *limitsLayout = new QGridLayout(0);
    limitsLayout->setMargin(0);
    topLayout->addLayout(limitsLayout);

    limitsSelect = new QComboBox(central);
    limitsSelect->addItem(tr("Use Original Data"));
    limitsSelect->addItem(tr("Use Current Plot"));
    connect(limitsSelect, SIGNAL(activated(int)),
            this, SLOT(limitsSelectChanged(int))); 
    QLabel *limitsLabel = new QLabel(tr("Limits"), central);
    limitsLabel->setBuddy(limitsSelect);
    limitsLayout->addWidget(limitsLabel, 0, 0);
    limitsLayout->addWidget(limitsSelect, 0, 1, 1, 2, Qt::AlignLeft);

    // Create the max toggle and line edit
    maxToggle = new QCheckBox(tr("Maximum"), central);
    limitsLayout->addWidget(maxToggle, 1, 1);
    connect(maxToggle, SIGNAL(toggled(bool)),
            this, SLOT(maxToggled(bool)));
    maxLineEdit = new QLineEdit(central);
    connect(maxLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMaxLimitText())); 
    limitsLayout->addWidget(maxLineEdit, 1, 2);

    // Create the min toggle and line edit
    minToggle = new QCheckBox(tr("Minimum"), central);
    limitsLayout->addWidget(minToggle, 2, 1);
    connect(minToggle, SIGNAL(toggled(bool)),
            this, SLOT(minToggled(bool)));
    minLineEdit = new QLineEdit(central);
    connect(minLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMinLimitText())); 
    limitsLayout->addWidget(minLineEdit, 2, 2);

    //
    // Create the scale radio buttons
    //
    QHBoxLayout *scaleLayout = new QHBoxLayout(0);
    scaleLayout->setMargin(0);
    topLayout->addLayout(scaleLayout);

    // Create a group of radio buttons
    scalingButtons = new QButtonGroup(central);
    QLabel *scaleLabel = new QLabel(tr("Scale"), central);
    scaleLayout->addWidget(scaleLabel);
    
    rb = new QRadioButton(tr("Linear"), central);
    rb->setChecked(true);
    scalingButtons->addButton(rb, 0);
    scaleLayout->addWidget(rb);
    rb = new QRadioButton(tr("Log"), central);
    scalingButtons->addButton(rb, 1);
    scaleLayout->addWidget(rb);
    rb = new QRadioButton(tr("Skew"), central);
    scalingButtons->addButton(rb, 2);
    scaleLayout->addWidget(rb);
    scaleLayout->addStretch(0);
    // Each time a radio button is clicked, call the scalelicked slot.
    connect(scalingButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(scaleClicked(int)));

    //
    // Create the rest of the window in a grid layout.
    //
    QGridLayout *gLayout = new QGridLayout(0);
    gLayout->setMargin(0);
    topLayout->addLayout(gLayout);

    // Create the skew factor line edit    
    skewLineEdit = new QLineEdit(central);
    connect(skewLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processSkewText())); 
    gLayout->addWidget(skewLineEdit, 0, 1);
    skewLabel = new QLabel(tr("Skew factor"), central);
    skewLabel->setBuddy(skewLineEdit);
    skewLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gLayout->addWidget(skewLabel, 0, 0);

    // Create the point control
    pointControl = new QvisPointControl(central);
    connect(pointControl, SIGNAL(pointSizeChanged(double)),
            this, SLOT(pointSizeChanged(double)));
    connect(pointControl, SIGNAL(pointSizePixelsChanged(int)),
            this, SLOT(pointSizePixelsChanged(int)));
    connect(pointControl, SIGNAL(pointSizeVarChanged(const QString &)),
            this, SLOT(pointSizeVarChanged(const QString &)));
    connect(pointControl, SIGNAL(pointSizeVarToggled(bool)),
            this, SLOT(pointSizeVarToggled(bool)));
    connect(pointControl, SIGNAL(pointTypeChanged(int)),
            this, SLOT(pointTypeChanged(int)));
    gLayout->addWidget(pointControl, 1, 0, 1, 2);
 
    //
    // Create the opacity slider
    //
    opacitySlider = new QvisOpacitySlider(0, 255, 25, 255, central);
    opacitySlider->setTickInterval(64);
    opacitySlider->setGradientColor(QColor(0, 0, 0));
    connect(opacitySlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(changedOpacity(int, const void*)));
    gLayout->addWidget(opacitySlider, 2, 1);

    QLabel *opacityLabel = new QLabel(tr("Opacity"), central);
    opacityLabel->setBuddy(opacitySlider);
    opacityLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gLayout->addWidget(opacityLabel, 2, 0);

    // Create the color table widgets
    colorTableButton = new QvisColorTableButton(central);
    connect(colorTableButton, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    gLayout->addWidget(colorTableButton, 3, 1, Qt::AlignLeft | Qt::AlignVCenter);
    QLabel *colorTableLabel = new QLabel(tr("Color table"), central);
    colorTableLabel->setBuddy(colorTableButton);
    colorTableLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gLayout->addWidget(colorTableLabel, 3, 0);

    // Create the legend toggle
    legendToggle = new QCheckBox(tr("Legend"), central);
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    gLayout->addWidget(legendToggle, 4, 0);

    // Create the lighting toggle
    lightingToggle = new QCheckBox(tr("Lighting"), central);
    connect(lightingToggle, SIGNAL(toggled(bool)),
            this, SLOT(lightingToggled(bool)));
    gLayout->addWidget(lightingToggle, 4, 1);

    // Create the smoothing level buttons
    smoothingLevelButtons = new QButtonGroup(central);
    connect(smoothingLevelButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(smoothingLevelChanged(int)));
    QGridLayout *smoothingLayout = new QGridLayout(0);
    smoothingLayout->setMargin(0);
    smoothingLayout->setSpacing(10);
    smoothingLayout->setColumnStretch(4, 1000);
    smoothingLayout->addWidget(new QLabel(tr("Geometry smoothing"), central), 0,0);
    rb = new QRadioButton(tr("None"), central);
    smoothingLevelButtons->addButton(rb, 0);
    smoothingLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("Fast"), central);
    smoothingLevelButtons->addButton(rb, 1);
    smoothingLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton(tr("High"), central);
    smoothingLevelButtons->addButton(rb, 2);
    smoothingLayout->addWidget(rb, 0, 3);
    gLayout->addLayout(smoothingLayout, 5,0, 1,2);
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::UpdateWindow
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
// Programmer: Brad Whitlock
// Creation:   Tue Aug 1 17:07:27 PST 2000
//
// Modifications:
//   Kathleen Bonnell, Wed Dec 13 13:17:26 PST 2000
//   Added opacity.
//
//   Brad Whitlock, Sat Jun 16 15:32:18 PST 2001
//   Added code to set the color table.
//
//   Kathleen Bonnell, Thu Oct  4 16:28:16 PDT 2001 
//   Added code for limits selection. 
//
//   Brad Whitlock, Wed Nov 7 16:05:44 PST 2001
//   Modified the code a little so it can use the generated attributes.
//
//   Brad Whitlock, Fri Feb 15 09:37:51 PDT 2002
//   Altered the code to set the opacity slider's opacity.
//
//   Kathleen Bonnell, Wed May 29 13:40:22 PDT 2002 
//   Removed dependency of min/max upon LimitsMode. 
//
//   Jeremy Meredith, Tue Dec 10 10:23:07 PST 2002
//   Added smoothing options.
//
//   Jeremy Meredith, Fri Dec 20 11:36:03 PST 2002
//   Added scaling of point variables by a scalar field.
//
//   Hank Childs, Thu Aug 21 21:59:56 PDT 2003
//   Account for point type.
//
//   Jeremy Meredith, Tue May  4 13:23:10 PDT 2004
//   Added support for a new (Point) type of glyphing for point meshes.
//   When doing GL_POINT, we ignore point size, so also disable it.
//
//   Kathleen Bonnell, Fri Nov 12 11:25:23 PST 2004 
//   Replace point-size related cases with QvisPointControl 
//
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Mark C. Miller, Mon Dec  6 13:30:51 PST 2004
//   Fixed SGI compiler error with string conversion to QString
//
//   Brad Whitlock, Thu Jun 26 16:58:00 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < pcAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!pcAtts->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case PseudocolorAttributes::ID_legendFlag:
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(legendToggle, SIGNAL(toggled(bool)),
                       this, SLOT(legendToggled(bool)));
            legendToggle->setChecked(pcAtts->GetLegendFlag());
            connect(legendToggle, SIGNAL(toggled(bool)),
                    this, SLOT(legendToggled(bool)));
            break;
        case PseudocolorAttributes::ID_lightingFlag:
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(lightingToggle, SIGNAL(toggled(bool)),
                       this, SLOT(lightingToggled(bool)));
            lightingToggle->setChecked(pcAtts->GetLightingFlag());
            connect(lightingToggle, SIGNAL(toggled(bool)),
                    this, SLOT(lightingToggled(bool)));
            break;
        case PseudocolorAttributes::ID_minFlag:
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(minToggle, SIGNAL(toggled(bool)),
                       this, SLOT(minToggled(bool)));
            minToggle->setChecked(pcAtts->GetMinFlag());
            minLineEdit->setEnabled(pcAtts->GetMinFlag());
            connect(minToggle, SIGNAL(toggled(bool)),
                    this, SLOT(minToggled(bool)));
            break;
        case PseudocolorAttributes::ID_maxFlag:
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(maxToggle, SIGNAL(toggled(bool)),
                       this, SLOT(maxToggled(bool)));
            maxToggle->setChecked(pcAtts->GetMaxFlag());
            maxLineEdit->setEnabled(pcAtts->GetMaxFlag());
            connect(maxToggle, SIGNAL(toggled(bool)),
                    this, SLOT(maxToggled(bool)));
            break;
        case PseudocolorAttributes::ID_centering:
            centeringButtons->blockSignals(true);
            centeringButtons->button(pcAtts->GetCentering())->setChecked(true);
            centeringButtons->blockSignals(false);
            break;
        case PseudocolorAttributes::ID_scaling:
            scalingButtons->blockSignals(true);
            scalingButtons->button(pcAtts->GetScaling())->setChecked(true);
            scalingButtons->blockSignals(false);
            skewLineEdit->setEnabled(pcAtts->GetScaling() ==
                PseudocolorAttributes::Skew);
            skewLabel->setEnabled(pcAtts->GetScaling() ==
                PseudocolorAttributes::Skew);
            break;
        case PseudocolorAttributes::ID_limitsMode:
            limitsSelect->blockSignals(true);
            limitsSelect->setCurrentIndex(pcAtts->GetLimitsMode());
            limitsSelect->blockSignals(false);
            break;
        case PseudocolorAttributes::ID_min:
            temp.setNum(pcAtts->GetMin());
            minLineEdit->setText(temp);
            break;
        case PseudocolorAttributes::ID_max:
            temp.setNum(pcAtts->GetMax());
            maxLineEdit->setText(temp);
            break;
        case PseudocolorAttributes::ID_pointSize:
            pointControl->blockSignals(true);
            pointControl->SetPointSize(pcAtts->GetPointSize());
            pointControl->blockSignals(false);
            break;
        case PseudocolorAttributes::ID_pointType:
            pointControl->blockSignals(true);
            pointControl->SetPointType(pcAtts->GetPointType());
            pointControl->blockSignals(false);
            break;
        case PseudocolorAttributes::ID_skewFactor:
            temp.setNum(pcAtts->GetSkewFactor());
            skewLineEdit->setText(temp);
            break;
        case PseudocolorAttributes::ID_opacity:
            opacitySlider->blockSignals(true);
            opacitySlider->setValue(int((float)pcAtts->GetOpacity() * 255.f));
            opacitySlider->blockSignals(false);
            break;
        case PseudocolorAttributes::ID_colorTableName:
            colorTableButton->setColorTable(pcAtts->GetColorTableName().c_str());
            break;
        case PseudocolorAttributes::ID_smoothingLevel:
            smoothingLevelButtons->blockSignals(true);
            smoothingLevelButtons->button(pcAtts->GetSmoothingLevel())->setChecked(true);
            smoothingLevelButtons->blockSignals(false);
            break;
        case PseudocolorAttributes::ID_pointSizeVarEnabled:
            pointControl->blockSignals(true);
            pointControl->SetPointSizeVarChecked(pcAtts->GetPointSizeVarEnabled());
            pointControl->blockSignals(false);
            break;
        case PseudocolorAttributes::ID_pointSizeVar:
            pointControl->blockSignals(true);
            temp = QString(pcAtts->GetPointSizeVar().c_str());
            pointControl->SetPointSizeVar(temp);
            pointControl->blockSignals(false);
            break;
        case PseudocolorAttributes::ID_pointSizePixels:
            pointControl->blockSignals(true);
            pointControl->SetPointSizePixels(pcAtts->GetPointSizePixels());
            pointControl->blockSignals(false);
            break;
        }
    } // end for
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values for one or all of the lineEdit widgets.
//
// Arguments:
//   which_widget : The number of the widget to update. If -1 is passed,
//                  the routine gets the current values for all widgets.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 15:11:42 PST 2000
//
// Modifications:
//   Jeremy Meredith, Fri Dec 20 11:36:03 PST 2002
//   Added scaling of point variables by a scalar field.
//
//   Kathleen Bonnell, Fri Nov 12 11:25:23 PST 2004 
//   Replace pointSizeLineEdit and pointSizeVarLineEdit with pointControl. 
//
//   Brad Whitlock, Wed Jul 20 14:25:04 PST 2005
//   Added PointSizePixels.
//
//   Brad Whitlock, Wed Apr 23 10:20:14 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Wed Jul  9 14:52:14 PDT 2008
//   Use helper methods.
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do the minimum value.
    if(which_widget == PseudocolorAttributes::ID_min || doAll)
    {
        double val;
        if(LineEditGetDouble(minLineEdit, val))
            pcAtts->SetMin(val);
        else
        {
            ResettingError(tr("minimum value"), DoubleToQString(pcAtts->GetMin()));
            pcAtts->SetMin(pcAtts->GetMin());
        }
    }

    // Do the maximum value
    if(which_widget == PseudocolorAttributes::ID_max || doAll)
    {
        double val;
        if(LineEditGetDouble(maxLineEdit, val))
            pcAtts->SetMax(val);
        else
        {
            ResettingError(tr("maximum value"), DoubleToQString(pcAtts->GetMax()));
            pcAtts->SetMax(pcAtts->GetMax());
        }
    }

    // Do the skew factor value
    if(which_widget == PseudocolorAttributes::ID_skewFactor || doAll)
    {
        double val;
        if(LineEditGetDouble(skewLineEdit, val))
            pcAtts->SetSkewFactor(val);
        else
        {
            ResettingError(tr("maximum value"), DoubleToQString(pcAtts->GetSkewFactor()));
            pcAtts->SetSkewFactor(pcAtts->GetSkewFactor());
        }
    }

    // Do the point size value and point size var value.
    if(doAll)
    {
        pcAtts->SetPointSize(pointControl->GetPointSize());
        pcAtts->SetPointSizePixels(pointControl->GetPointSizePixels());
        pcAtts->SetPointSizeVar(pointControl->GetPointSizeVar().toStdString());
    }
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::Apply
//
// Purpose: 
//   This method applies the pc attributes and optionally tells
//   the viewer to apply them.
//
// Arguments:
//   ignore : This flag, when true, tells the code to ignore the
//            AutoUpdate function and tell the viewer to apply the
//            pc attributes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 15:22:16 PST 2000
//
// Modifications:
//    Eric Brugger, Wed Mar 14 06:59:33 PST 2001
//    I modified the routine to pass to the viewer proxy the plot
//    type stored within the class instead of the one hardwired from
//    an include file.
//   
// ****************************************************************************

void
QvisPseudocolorPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current aslice attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        pcAtts->Notify();

        // Tell the viewer to set the pc attributes.
        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        pcAtts->Notify();
}

//
// Qt Slot functions...
//

void
QvisPseudocolorPlotWindow::apply()
{
    Apply(true);
}

void
QvisPseudocolorPlotWindow::makeDefault()
{
    // Tell the viewer to set the default pc attributes.
    GetCurrentValues(-1);
    pcAtts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}

void
QvisPseudocolorPlotWindow::reset()
{
    // Tell the viewer to reset the aslice attributes to the last
    // applied values.
    GetViewerMethods()->ResetPlotOptions(plotType);
}

void
QvisPseudocolorPlotWindow::centeringClicked(int button)
{
    // Only do it if it changed.
    if(button != pcAtts->GetCentering())
    {
        pcAtts->SetCentering(PseudocolorAttributes::Centering(button));
        Apply();
    }
}

void
QvisPseudocolorPlotWindow::scaleClicked(int scale)
{
    // Only do it if it changed.
    if(scale != pcAtts->GetScaling())
    {
        pcAtts->SetScaling(PseudocolorAttributes::Scaling(scale));
        Apply();
    }
}

void
QvisPseudocolorPlotWindow::limitsSelectChanged(int mode)
{
    // Only do it if it changed.
    if(mode != pcAtts->GetLimitsMode())
    {
        pcAtts->SetLimitsMode(PseudocolorAttributes::LimitsMode(mode));
        Apply();
    }
}


void
QvisPseudocolorPlotWindow::processMinLimitText()
{
    GetCurrentValues(PseudocolorAttributes::ID_min);
    Apply();
}

void
QvisPseudocolorPlotWindow::processMaxLimitText()
{
    GetCurrentValues(PseudocolorAttributes::ID_max);
    Apply();
}

void
QvisPseudocolorPlotWindow::minToggled(bool val)
{
    pcAtts->SetMinFlag(val);
    Apply();
}

void
QvisPseudocolorPlotWindow::maxToggled(bool val)
{
    pcAtts->SetMaxFlag(val);
    Apply();
}

void
QvisPseudocolorPlotWindow::legendToggled(bool val)
{
    pcAtts->SetLegendFlag(val);
    Apply();
}

void
QvisPseudocolorPlotWindow::lightingToggled(bool val)
{
    pcAtts->SetLightingFlag(val);
    Apply();
}

void
QvisPseudocolorPlotWindow::processSkewText()
{
    GetCurrentValues(PseudocolorAttributes::ID_skewFactor);
    Apply();
}

void
QvisPseudocolorPlotWindow::changedOpacity(int opacity, const void*)
{
    pcAtts->SetOpacity((float)opacity/255.);
    Apply();
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::colorTableClicked
//
// Purpose: 
//   This is a Qt slot function that sets the desired color table into the
//   pseudocolor plot attributes.
//
// Arguments:
//   useDefault : Whether or not to use the default color table.
//   ctName     : The name of the color table to use.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 18:58:03 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisPseudocolorPlotWindow::colorTableClicked(bool useDefault,
    const QString &ctName)
{
    pcAtts->SetColorTableName(ctName.toStdString());
    Apply();
}

// ****************************************************************************
//  Method:  QvisPseudocolorPlotWindow::smoothingLevelChanged
//
//  Purpose:
//    Qt slot function that is called when one of the smoothing buttons
//    is clicked.
//
//  Arguments:
//    level  :   The new level.
//
//  Programmer:  Jeremy Meredith
//  Creation:    December  9, 2002
//
//  Modifications:
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::smoothingLevelChanged(int level)
{
    pcAtts->SetSmoothingLevel(level);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisPseudocolorPlotWindow::pointTypeChanged
//
//  Purpose:
//    Qt slot function that is called when one of the point types is clicked.
//
//  Arguments:
//    type   :   The new type
//
//  Programmer:  Hank Childs
//  Creation:    August 21, 2003
//
//  Modifications:
//   Jeremy Meredith, Tue May  4 13:23:10 PDT 2004
//   Added support for a new (Point) type of glyphing for point meshes.
//   When doing GL_POINT, we ignore point size, so also disable it.
//
// ****************************************************************************

void
QvisPseudocolorPlotWindow::pointTypeChanged(int type)
{
    pcAtts->SetPointType((PseudocolorAttributes::PointType) type);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::pointSizeVarToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the pointSizeVar toggle
//   button is toggled.
//
// Arguments:
//   val : The new state of the pointSizeVar toggle.
//
// Programmer: Jeremy Meredith
// Creation:   December 20, 2002
//   
// ****************************************************************************

void
QvisPseudocolorPlotWindow::pointSizeVarToggled(bool val)
{
    pcAtts->SetPointSizeVarEnabled(val);
    Apply();
}


// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::pointSizeChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   point size text and presses the Enter key.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 12, 2004 
//
// Modifications:
//   
// ****************************************************************************
void
QvisPseudocolorPlotWindow::pointSizeChanged(double size)
{
    pcAtts->SetPointSize(size); 
    Apply();
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::pointSizePixelsChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   point size text and presses the Enter key.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 20 14:25:58 PST 2005
//
// Modifications:
//   
// ****************************************************************************
void
QvisPseudocolorPlotWindow::pointSizePixelsChanged(int size)
{
    pcAtts->SetPointSizePixels(size); 
    Apply();
}

// ****************************************************************************
// Method: QvisPseudocolorPlotWindow::pointSizeVarChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   point size variable text and presses the Enter key.
//
// Programmer: Jeremy Meredith
// Creation:   December 20, 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisPseudocolorPlotWindow::pointSizeVarChanged(const QString &var)
{
    pcAtts->SetPointSizeVar(var.toStdString());
    Apply();
}


