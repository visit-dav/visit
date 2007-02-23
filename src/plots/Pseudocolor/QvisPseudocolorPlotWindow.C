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

#include <qgrid.h> 
#include <qlayout.h> 
#include <qpushbutton.h> 
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h> 
#include <qlineedit.h>

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
    PseudocolorAttributes *_pcAtts, const char *caption, const char *shortName,
    QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(_pcAtts, caption, shortName, notepad)
{
    plotType = type;
    pcAtts   = _pcAtts;
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
// ****************************************************************************

void
QvisPseudocolorPlotWindow::CreateWindowContents()
{
    //
    // Create the centering radio buttons
    //
    QHBoxLayout *centeringLayout = new QHBoxLayout(topLayout);

    // Create a group of radio buttons
    centeringButtons = new QButtonGroup( central, "radioGroup" );
    centeringButtons->setFrameStyle(QFrame::NoFrame);
    QLabel *centeringLabel = new QLabel(centeringButtons, "Centering",
        central, "centerLabel");
    centeringLayout->addWidget(centeringLabel);
    
    // Create a layout for the radio buttons
    QHBoxLayout *centeringButtonLayout = new QHBoxLayout(centeringButtons);
    centeringButtonLayout->setSpacing(10);
    // Create the radio buttons
    QRadioButton *rb= new QRadioButton("Natural", centeringButtons );
    rb->setChecked( TRUE );
    centeringButtonLayout->addWidget(rb);
    rb = new QRadioButton("Nodal", centeringButtons );
    centeringButtonLayout->addWidget(rb);
    rb = new QRadioButton("Zonal", centeringButtons );
    centeringButtonLayout->addWidget(rb);
    centeringLayout->addWidget( centeringButtons );
    centeringLayout->addStretch(0);
    // Each time a radio button is clicked, call the centeringClicked slot.
    connect(centeringButtons, SIGNAL(clicked(int)),
            this, SLOT(centeringClicked(int)));
    
    //
    // Create the Limits stuff
    //
    QGridLayout *limitsLayout = new QGridLayout(topLayout, 3, 3);

    limitsSelect = new QComboBox(false, central, "limitsSelect");
    limitsSelect->insertItem("Use Original Data");
    limitsSelect->insertItem("Use Current Plot");
    connect(limitsSelect, SIGNAL(activated(int)),
            this, SLOT(limitsSelectChanged(int))); 
    QLabel *limitsLabel = new QLabel(limitsSelect, "Limits", 
                                     central, "limitsLabel");
    limitsLayout->addWidget(limitsLabel, 0, 0);
    limitsLayout->addMultiCellWidget(limitsSelect, 0, 0, 1, 2, AlignLeft);

    // Create the min toggle and line edit
    minToggle = new QCheckBox("Min", central, "minToggle");
    limitsLayout->addWidget(minToggle, 1, 1);
    connect(minToggle, SIGNAL(toggled(bool)),
            this, SLOT(minToggled(bool)));
    minLineEdit = new QLineEdit(central, "minLineEdit");
    connect(minLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMinLimitText())); 
    limitsLayout->addWidget(minLineEdit, 1, 2);

    // Create the max toggle and line edit
    maxToggle = new QCheckBox("Max", central, "maxToggle");
    limitsLayout->addWidget(maxToggle, 2, 1);
    connect(maxToggle, SIGNAL(toggled(bool)),
            this, SLOT(maxToggled(bool)));
    maxLineEdit = new QLineEdit(central, "maxLineEdit");
    connect(maxLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMaxLimitText())); 
    limitsLayout->addWidget(maxLineEdit, 2, 2);

    //
    // Create the scale radio buttons
    //
    QHBoxLayout *scaleLayout = new QHBoxLayout(topLayout);

    // Create a group of radio buttons
    scalingButtons = new QButtonGroup( central, "scaleRadioGroup" );
    scalingButtons->setFrameStyle(QFrame::NoFrame);
    QLabel *scaleLabel = new QLabel(scalingButtons, "Scale", central,
        "scaleLabel");
    scaleLayout->addWidget(scaleLabel);
    
    QHBoxLayout *scaleButtonsLayout = new QHBoxLayout(scalingButtons);
    scaleButtonsLayout->setSpacing(10);
    rb = new QRadioButton("Linear", scalingButtons );
    rb->setChecked( TRUE );
    scaleButtonsLayout->addWidget(rb);
    rb = new QRadioButton( scalingButtons );
    rb->setText( "Log" );
    scaleButtonsLayout->addWidget(rb);
    rb = new QRadioButton( scalingButtons );
    rb->setText( "Skew" );
    scaleButtonsLayout->addWidget(rb);
    scaleLayout->addWidget( scalingButtons );
    scaleLayout->addStretch(0);
    // Each time a radio button is clicked, call the scalelicked slot.
    connect(scalingButtons, SIGNAL(clicked(int)),
            this, SLOT(scaleClicked(int)));

    //
    // Create the rest of the window in a grid layout.
    //
    QGridLayout *gLayout = new QGridLayout(topLayout, 6, 2);

    // Create the skew factor line edit    
    skewLineEdit = new QLineEdit(central, "skewLineEdit");
    connect(skewLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processSkewText())); 
    gLayout->addWidget(skewLineEdit, 0, 1);
    skewLabel = new QLabel(skewLineEdit, "Skew factor", central, "skewFactor");
    skewLabel->setAlignment(AlignRight | AlignVCenter);
    gLayout->addWidget(skewLabel, 0, 0);

    // Create the point control
    pointControl = new QvisPointControl(central, "pointControl");
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
    gLayout->addMultiCellWidget(pointControl, 1, 1, 0, 1);
 
    //
    // Create the opacity slider
    //
    opacitySlider = new QvisOpacitySlider(0, 255, 25, 255, central, 
                    "opacitySlider", NULL);
    opacitySlider->setTickInterval(64);
    opacitySlider->setGradientColor(QColor(0, 0, 0));
    connect(opacitySlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(changedOpacity(int, const void*)));
    gLayout->addWidget(opacitySlider, 2, 1);

    QLabel *opacityLabel = new QLabel(opacitySlider, "Opacity", 
                                      central, "opacityLabel"); 
    opacityLabel->setAlignment(AlignRight | AlignVCenter);
    gLayout->addWidget(opacityLabel, 2, 0);

    // Create the color table widgets
    colorTableButton = new QvisColorTableButton(central, "colorTableButton");
    connect(colorTableButton, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    gLayout->addWidget(colorTableButton, 3, 1, AlignLeft | AlignVCenter);
    QLabel *colorTableLabel = new QLabel(colorTableButton, "Color table",
                                         central, "colorTableLabel");
    colorTableLabel->setAlignment(AlignRight | AlignVCenter);
    gLayout->addWidget(colorTableLabel, 3, 0);

    // Create the legend toggle
    legendToggle = new QCheckBox("Legend", central, "legendToggle");
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    gLayout->addWidget(legendToggle, 4, 0);

    // Create the lighting toggle
    lightingToggle = new QCheckBox("Lighting", central, "lightingToggle");
    connect(lightingToggle, SIGNAL(toggled(bool)),
            this, SLOT(lightingToggled(bool)));
    gLayout->addWidget(lightingToggle, 4, 1);

    // Create the smoothing level buttons
    smoothingLevelButtons = new QButtonGroup(0, "smoothingButtons");
    connect(smoothingLevelButtons, SIGNAL(clicked(int)),
            this, SLOT(smoothingLevelChanged(int)));
    QGridLayout *smoothingLayout = new QGridLayout(1, 5);
    smoothingLayout->setSpacing(10);
    smoothingLayout->setColStretch(4, 1000);
    smoothingLayout->addWidget(new QLabel("Geometry smoothing", central), 0,0);
    rb = new QRadioButton("None", central, "NoSmoothing");
    smoothingLevelButtons->insert(rb);
    smoothingLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton("Fast", central, "LowSmoothing");
    smoothingLevelButtons->insert(rb);
    smoothingLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton("High", central, "HighSmoothing");
    smoothingLevelButtons->insert(rb);
    smoothingLayout->addWidget(rb, 0, 3);
    gLayout->addMultiCellLayout(smoothingLayout, 5,5 , 0,1);
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
        case 0: // legendFlag
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(legendToggle, SIGNAL(toggled(bool)),
                       this, SLOT(legendToggled(bool)));
            legendToggle->setChecked(pcAtts->GetLegendFlag());
            connect(legendToggle, SIGNAL(toggled(bool)),
                    this, SLOT(legendToggled(bool)));
            break;
        case 1: // lightingFlag
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(lightingToggle, SIGNAL(toggled(bool)),
                       this, SLOT(lightingToggled(bool)));
            lightingToggle->setChecked(pcAtts->GetLightingFlag());
            connect(lightingToggle, SIGNAL(toggled(bool)),
                    this, SLOT(lightingToggled(bool)));
            break;
        case 2: // minFlag
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(minToggle, SIGNAL(toggled(bool)),
                       this, SLOT(minToggled(bool)));
            minToggle->setChecked(pcAtts->GetMinFlag());
            minLineEdit->setEnabled(pcAtts->GetMinFlag());
            connect(minToggle, SIGNAL(toggled(bool)),
                    this, SLOT(minToggled(bool)));
            break;
        case 3: // maxFlag
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(maxToggle, SIGNAL(toggled(bool)),
                       this, SLOT(maxToggled(bool)));
            maxToggle->setChecked(pcAtts->GetMaxFlag());
            maxLineEdit->setEnabled(pcAtts->GetMaxFlag());
            connect(maxToggle, SIGNAL(toggled(bool)),
                    this, SLOT(maxToggled(bool)));
            break;
        case 4: // centering
            centeringButtons->setButton(pcAtts->GetCentering());
            break;
        case 5: // scaling
            scalingButtons->setButton(pcAtts->GetScaling());
            skewLineEdit->setEnabled(pcAtts->GetScaling() ==
                PseudocolorAttributes::Skew);
            skewLabel->setEnabled(pcAtts->GetScaling() ==
                PseudocolorAttributes::Skew);
            break;
        case 6: // limitsMode
            limitsSelect->blockSignals(true);
            limitsSelect->setCurrentItem(pcAtts->GetLimitsMode());
            limitsSelect->blockSignals(false);
            break;
        case 7: // min
            temp.setNum(pcAtts->GetMin());
            minLineEdit->setText(temp);
            break;
        case 8: // max
            temp.setNum(pcAtts->GetMax());
            maxLineEdit->setText(temp);
            break;
        case 9: // pointSize
            pointControl->blockSignals(true);
            pointControl->SetPointSize(pcAtts->GetPointSize());
            pointControl->blockSignals(false);
            break;
        case 10: // pointType
            pointControl->blockSignals(true);
            pointControl->SetPointType(pcAtts->GetPointType());
            pointControl->blockSignals(false);
            break;
        case 11: // skewFactor
            temp.setNum(pcAtts->GetSkewFactor());
            skewLineEdit->setText(temp);
            break;
        case 12: // opacity
            opacitySlider->blockSignals(true);
            opacitySlider->setValue(int((float)pcAtts->GetOpacity() * 255.f));
            opacitySlider->blockSignals(false);
            break;
        case 13: // colorTableName
            colorTableButton->setColorTable(pcAtts->GetColorTableName().c_str());
            break;
        case 14: // smoothingLevel
            smoothingLevelButtons->blockSignals(true);
            smoothingLevelButtons->setButton(pcAtts->GetSmoothingLevel());
            smoothingLevelButtons->blockSignals(false);
            break;
        case 15: // pointSizeVarEnabled
            pointControl->blockSignals(true);
            pointControl->SetPointSizeVarChecked(pcAtts->GetPointSizeVarEnabled());
            pointControl->blockSignals(false);
            break;
        case 16: // pointSizeVar
            pointControl->blockSignals(true);
            temp = QString(pcAtts->GetPointSizeVar().c_str());
            pointControl->SetPointSizeVar(temp);
            pointControl->blockSignals(false);
            break;
        case 17: // pointSizePixels
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
// ****************************************************************************

void
QvisPseudocolorPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do the minimum value.
    if(which_widget == 0 || doAll)
    {
        temp = minLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            pcAtts->SetMin(val);
        }

        if(!okay)
        {
            msg.sprintf("The minimum value was invalid. "
                "Resetting to the last good value of %g.", pcAtts->GetMin());
            Message(msg);
            pcAtts->SetMin(pcAtts->GetMin());
        }
    }

    // Do the maximum value
    if(which_widget == 1 || doAll)
    {
        temp = maxLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            pcAtts->SetMax(val);
        }

        if(!okay)
        {
            msg.sprintf("The maximum value was invalid. "
                "Resetting to the last good value of %g.", pcAtts->GetMax());
            Message(msg);
            pcAtts->SetMax(pcAtts->GetMax());
        }
    }

    // Do the skew factor value
    if(which_widget == 2 || doAll)
    {
        temp = skewLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            pcAtts->SetSkewFactor(val);
        }

        if(!okay)
        {
            msg.sprintf("The skew factor was invalid. "
                "Resetting to the last good value of %g.",
                pcAtts->GetSkewFactor());
            Message(msg);
            pcAtts->SetSkewFactor(pcAtts->GetSkewFactor());
        }
    }

    // Do the point size value and point size var value.
    if(doAll)
    {
        pcAtts->SetPointSize(pointControl->GetPointSize());
        pcAtts->SetPointSizePixels(pointControl->GetPointSizePixels());
        pcAtts->SetPointSizeVar(pointControl->GetPointSizeVar().latin1());
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
    GetCurrentValues(0);
    Apply();
}

void
QvisPseudocolorPlotWindow::processMaxLimitText()
{
    GetCurrentValues(1);
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
    GetCurrentValues(2);
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
    pcAtts->SetColorTableName(ctName.latin1());
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
    pcAtts->SetPointSizeVar(var.latin1());
    Apply();
}


