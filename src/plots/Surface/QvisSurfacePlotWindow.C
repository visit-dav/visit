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

#include <QvisSurfacePlotWindow.h>

#include <math.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

#include <QvisColorButton.h>
#include <QvisColorManagerWidget.h>
#include <QvisColorTableButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisOpacitySlider.h>
#include <SurfaceAttributes.h>
#include <ViewerProxy.h>

#include <DebugStream.h>

// ****************************************************************************
// Method: QvisSurfacePlotWindow::QvisSurfacePlotWindow
//
// Purpose: 
//   Constructor for the QvisSurfacePlotWindow class.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001 
//
// Modifications:
//    Eric Brugger, Fri Mar 16 16:17:18 PST 2001
//    I added the argument type.
//
// ****************************************************************************

QvisSurfacePlotWindow::QvisSurfacePlotWindow(const int type,
    SurfaceAttributes *surfaceAtts_, const char *caption,
    const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(surfaceAtts_, caption, shortName, notepad)
{
    plotType    = type;
    surfaceAtts = surfaceAtts_;
}

// ****************************************************************************
// Method: QvisSurfacePlotWindow::~QvisSurfacePlotWindow
//
// Purpose: 
//   Destructor for the QvisSurfacePlotWindow class.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001 
//
// Modifications:
//   
// ****************************************************************************

QvisSurfacePlotWindow::~QvisSurfacePlotWindow()
{
    surfaceAtts = 0;
}

// ****************************************************************************
// Method: QvisSurfacePlotWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets that are in the window and sets
//   up their signals/slots.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001 
//
// Modifications:
//   Brad Whitlock, Sat Jun 16 18:28:35 PST 2001
//   Added a color table button.
//
//   Jeremy Meredith, Fri Sep 14 17:02:31 PDT 2001
//   Swapped the creation order of the wireframe label and button so the
//   button was created by the time the label wanted to buddy it.
//
//   Kathleen Bonnell, Thu Oct 11 12:45:30 PDT 2001 
//   Added limitsSelect button and associated colorinMin/Max.  Changed old
//   min/max to scalingMin/max. 
//
//   Brad Whitlock, Fri Oct 26 16:47:44 PST 2001
//   Enabled wireframe color.
//
//   Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002
//   Once again use the same limits for coloring and for scaling.  Removed
//   coloringMin/Max.  Renamed scalingMin/Max to simply min/max.
//   Moved limitsSelect to same window area as limits line edits. 
//
//   Kathleen Bonnell, Wed May 29 13:40:22 PDT 2002  
//   Remove 'Specify' from limitSelect. 
//
//   Brad Whitlock, Wed Apr 23 12:01:18 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

void
QvisSurfacePlotWindow::CreateWindowContents()
{
    //
    //  Create the rendering mode layout
    //
    QGridLayout *renderLayout = new QGridLayout(topLayout, 4, 2);
    renderLayout->addColSpacing(0, 2);
   
    //
    // Create the surface toggle
    //
    surfaceToggle = new QCheckBox(tr("Surface"), central, "surfaceToggle");
    connect(surfaceToggle, SIGNAL(toggled(bool)),
            this, SLOT(surfaceToggled(bool)));
    renderLayout->addMultiCellWidget(surfaceToggle, 0, 0, 0, 1);


    //
    // Create the wireframe toggle
    //
    wireframeToggle = new QCheckBox(tr("Wireframe"), central,"wireframeToggle");
    connect(wireframeToggle, SIGNAL(toggled(bool)),
            this, SLOT(wireframeToggled(bool)));
    renderLayout->addMultiCellWidget(wireframeToggle, 2, 2, 0, 1); 


    //
    // Create the mode buttons that determine if the surface is 
    // colored by z-value or single-color.
    //
    colorModeGroup = new QGroupBox(central, "colorModeGroup");
    QVBoxLayout *modeLayout = new QVBoxLayout(colorModeGroup);
    modeLayout->setMargin(10);
    modeLayout->addSpacing(5);

    colorModeButtons = new QButtonGroup(colorModeGroup, "colorModeButtons");
    colorModeButtons->setFrameStyle(QFrame::NoFrame);
    connect(colorModeButtons, SIGNAL(clicked(int)),
            this, SLOT(colorModeChanged(int)));
    QGridLayout *colorModeButtonLayout = 
            new QGridLayout(colorModeButtons, 3, 2);
    colorModeButtonLayout->setSpacing(5);
    QLabel *colorModeLabel = 
           new QLabel(tr("Surface color"), colorModeButtons, "colorModeLabel");
    colorModeButtonLayout->addWidget(colorModeLabel, 0, 0);


    QRadioButton *rb = new QRadioButton(tr("Z Value"),colorModeButtons,"colorByZ");
    colorModeButtonLayout->addWidget(rb, 2, 0);
    rb = new QRadioButton(tr("Constant"), colorModeButtons, "colorBySurface");
    colorModeButtonLayout->addWidget(rb, 1, 0);

    // Create the surface color button.
    surfaceColor = new QvisColorButton(colorModeButtons, "surfaceColor");
    surfaceColor->setButtonColor(QColor(255, 0, 0));
    connect(surfaceColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(surfaceColorChanged(const QColor &)));
    colorModeButtonLayout->addWidget(surfaceColor, 1, 1, 
                                     AlignLeft | AlignVCenter);

    // Create the surface color-by-z button.
    colorTableButton = new QvisColorTableButton(colorModeButtons, 
                                                "colorTableButton");
    connect(colorTableButton, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    colorModeButtonLayout->addWidget(colorTableButton, 2, 1, 
                                     AlignLeft | AlignVCenter);

    modeLayout->addWidget(colorModeButtons);

    renderLayout->addWidget(colorModeGroup, 1, 1);



    // Create the wireframe color button
    QGroupBox *wireModeGroup = new QGroupBox(central, "wireModeGroup");

    QVBoxLayout *wireModeLayout = new QVBoxLayout(wireModeGroup);
    wireModeLayout->setMargin(10);

    QButtonGroup * wireModeButtons = 
                   new QButtonGroup(wireModeGroup, "wireModeButtons");
    wireModeButtons->setFrameStyle(QFrame::NoFrame);
    QGridLayout *wireModeButtonLayout =
                   new QGridLayout(wireModeButtons, 2, 4);
    wireModeButtonLayout->setSpacing(10);

    wireframeColor = new QvisColorButton(wireModeButtons, "wireframeColor");
    wireframeColor->setButtonColor(QColor(0, 0, 0));
    connect(wireframeColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(wireframeColorChanged(const QColor &)));
    wireModeButtonLayout->addWidget(wireframeColor, 1, 1);

    wireframeLabel = new QLabel(wireframeColor, tr("Wire Color"),
                                wireModeButtons, "wireframeLabel");
    wireModeButtonLayout->addWidget(wireframeLabel, 1, 0);

    //
    // Create the lineWidth widget.
    //
    lineWidth = new QvisLineWidthWidget(0, wireModeButtons, "lineWidth");
    wireModeButtonLayout->addWidget(lineWidth, 0, 3);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    lineWidthLabel = new QLabel(lineWidth, tr("Line width"),
                                wireModeButtons, "lineWidthLabel");
    wireModeButtonLayout->addWidget(lineWidthLabel, 0, 2);

    //
    // Create the lineStyle widget.
    //
    lineStyle = new QvisLineStyleWidget(0, wireModeButtons, "lineStyle");
    wireModeButtonLayout->addWidget(lineStyle, 0, 1);
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    lineStyleLabel = new QLabel(lineStyle, tr("Line style"),
                                wireModeButtons, "lineStyleLabel");
    wireModeButtonLayout->addWidget(lineStyleLabel, 0, 0);

    wireModeLayout->addWidget(wireModeButtons);
    renderLayout->addWidget(wireModeGroup, 3, 1);

    topLayout->addSpacing(5);

    //
    // Create the scale radio buttons
    //
    QHBoxLayout *scaleLayout = new QHBoxLayout(topLayout);
    // Create a group of radio buttons
    scalingButtons = new QButtonGroup( central, "scaleRadioGroup" );
    scalingButtons->setFrameStyle(QFrame::NoFrame);
    QLabel *scaleLabel = new QLabel(scalingButtons, tr("Scale  "), central,
        "scaleLabel");
    scaleLayout->addWidget(scaleLabel);

    QHBoxLayout *scaleButtonsLayout = new QHBoxLayout(scalingButtons);
    scaleButtonsLayout->setSpacing(10);
    rb = new QRadioButton(tr("Linear"), scalingButtons );
    rb->setChecked( TRUE );
    scaleButtonsLayout->addWidget(rb);
    rb = new QRadioButton( scalingButtons );
    rb->setText( tr("Log") );
    scaleButtonsLayout->addWidget(rb);
    rb = new QRadioButton( scalingButtons );
    rb->setText( tr("Skew") );
    scaleButtonsLayout->addWidget(rb);
    scaleLayout->addWidget( scalingButtons );
    scaleLayout->addStretch(0);
    // Each time a radio button is clicked, call the scaleClicked slot.
    connect(scalingButtons, SIGNAL(clicked(int)),
            this, SLOT(scaleClicked(int)));

    //
    // Create the layout for skew information.
    //
  
    QGridLayout *skewLayout = new QGridLayout(topLayout, 1, 2);
    // Create the skew factor line edit
    skewLineEdit = new QLineEdit(central, "skewLineEdit");
    connect(skewLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processSkewText()));
    skewLayout->addColSpacing(0, 50);
    skewLayout->addWidget(skewLineEdit, 0, 2);
    skewLabel = new QLabel(skewLineEdit, tr("Skew factor"), central, "skewFactor");
    skewLabel->setAlignment(AlignRight | AlignVCenter);
    skewLayout->addWidget(skewLabel, 0, 1);

    topLayout->addSpacing(5);

    //
    // Create Limits stuff .
    //
    //QGroupBox *limitsGroup = new QGroupBox(central, "limitsGroup");
    //limitsGroup->setFrameStyle(QFrame::NoFrame);

    QGridLayout *limitsLayout = new QGridLayout(topLayout, 3, 4);
    limitsLayout->setMargin(10);
    limitsLayout->setSpacing(5);
                                                     
    limitsSelect = new QComboBox(false, central, "limitsSelect");
    limitsSelect->insertItem(tr("Use Original Data"));
    limitsSelect->insertItem(tr("Use Current Plot"));
    connect(limitsSelect, SIGNAL(activated(int)),
            this, SLOT(limitsSelectChanged(int)));
    QLabel *limitsLabel = new QLabel(limitsSelect, "Limits",
                                     central, "limitsLabel");
    limitsLayout->addWidget(limitsLabel, 0, 0);
    limitsLayout->addColSpacing(1, 20);
    limitsLayout->addMultiCellWidget(limitsSelect, 0, 0, 2, 3, AlignLeft);

    // Create the min toggle and line edit
    minToggle = new QCheckBox(tr("Min"), central, "minToggle");
    limitsLayout->addWidget(minToggle, 1, 2);
    connect(minToggle, SIGNAL(toggled(bool)),
            this, SLOT(minToggled(bool)));
    minLineEdit = new QLineEdit(central, "minLineEdit");
    connect(minLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMinLimitText())); 
    limitsLayout->addWidget(minLineEdit, 1, 3);

    // Create the max toggle and line edit
    maxToggle = new QCheckBox(tr("Max"), central, "maxToggle");
    limitsLayout->addWidget(maxToggle, 2, 2);
    connect(maxToggle, SIGNAL(toggled(bool)),
            this, SLOT(maxToggled(bool)));
    maxLineEdit = new QLineEdit(central, "maxLineEdit");
    connect(maxLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMaxLimitText())); 
    limitsLayout->addWidget(maxLineEdit, 2, 3);
    //limitsLayout->addColSpacing(4, 75);
    //renderLayout->addWidget(limitsGroup, 2, 0);


    topLayout->addSpacing(5);
    // Create toggle buttons for various flags

    QGridLayout *toggleLayout = new QGridLayout(topLayout, 2, 2);
    toggleLayout->setSpacing(10);

    // Create the legend toggle
    legendToggle = new QCheckBox(tr("Legend"), central, "legendToggle");
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    toggleLayout->addWidget(legendToggle, 0, 0, Qt::AlignHCenter);

    // Create the lighting toggle
    lightingToggle = new QCheckBox(tr("Lighting"), central, "lightingToggle");
    connect(lightingToggle, SIGNAL(toggled(bool)),
            this, SLOT(lightingToggled(bool)));
    toggleLayout->addWidget(lightingToggle, 0, 1, Qt::AlignHCenter);

}

// ****************************************************************************
// Method: QvisSurfacePlotWindow::UpdateWindow
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
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001 
//
// Modifications:
//   Kathleen Bonnell, Mon Mar 26 18:17:53 PST 2001
//   Disable lineStyle and lineStyleLabel until we have vtk version
//   in which line stippling is available.
//
//   Brad Whitlock, Sat Jun 16 18:29:08 PST 2001
//   Added code to set the color table.
//
//   Kathleen Bonnell, Thu Jun 21 16:33:54 PDT 2001
//   Enable lineStyle and lineStyleLabel.
//
//   Kathleen Bonnell, Thu Oct 11 12:45:30 PDT 2001 
//   Added limitsSelect button and associated colorinMin/Max.  Changed old
//   min/max to scalingMin/max. 
//
//   Brad Whitlock, Fri Oct 26 17:07:55 PST 2001
//   Enabled the wireframe widgets.
//
//   Brad Whitlock, Fri Nov 9 11:31:26 PDT 2001
//   Changed a constant name so it conforms to the name given by xml2atts.
//
//   Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002 
//   Reflect change in atts ordering.
//
//   Kathleen Bonnell, Wed May 29 13:40:22 PDT 2002  
//   Remove MinMaxEnabled as user setting minmax no longer dependent upon
//   limits mode. 
//
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisSurfacePlotWindow::UpdateWindow(bool doAll)
{
    QString temp;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < surfaceAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!surfaceAtts->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case 0: // legendFlag
            legendToggle->blockSignals(true);
            legendToggle->setChecked(surfaceAtts->GetLegendFlag());
            legendToggle->blockSignals(false);
            break;

        case 1: // lightingFlag
            lightingToggle->blockSignals(true);
            lightingToggle->setChecked(surfaceAtts->GetLightingFlag());
            lightingToggle->blockSignals(false);
            break;

        case 2: // surfaceFlag
            surfaceToggle->blockSignals(true);
            surfaceToggle->setChecked(surfaceAtts->GetSurfaceFlag());
            surfaceToggle->blockSignals(false);

            // set sensitivity of other widgets.
            surfaceColor->setEnabled(surfaceAtts->GetSurfaceFlag());
            colorModeGroup->setEnabled(surfaceAtts->GetSurfaceFlag()); 
            colorModeButtons->setEnabled(surfaceAtts->GetSurfaceFlag()); 
            break;

        case 3: // wireframeFlag
            wireframeToggle->blockSignals(true);
            wireframeToggle->setChecked(surfaceAtts->GetWireframeFlag());
            wireframeToggle->blockSignals(false);

            // set sensitivity of other widgets.
            wireframeColor->setEnabled(surfaceAtts->GetWireframeFlag());
            wireframeLabel->setEnabled(surfaceAtts->GetWireframeFlag());

            lineStyleLabel->setEnabled(surfaceAtts->GetWireframeFlag());
            lineStyle->setEnabled(surfaceAtts->GetWireframeFlag());
            lineWidthLabel->setEnabled(surfaceAtts->GetWireframeFlag());
            lineWidth->setEnabled(surfaceAtts->GetWireframeFlag());
            break;


        case 4: // limitsMode
            limitsSelect->blockSignals(true);
            limitsSelect->setCurrentItem(surfaceAtts->GetLimitsMode());
            limitsSelect->blockSignals(false);
            break;

        case 5: // minFlag
            minToggle->blockSignals(true);
            minToggle->setChecked(surfaceAtts->GetMinFlag());
            minLineEdit->setEnabled(surfaceAtts->GetMinFlag());
            minToggle->blockSignals(false);
            break;

        case 6: // maxFlag
            maxToggle->blockSignals(true);
            maxToggle->setChecked(surfaceAtts->GetMaxFlag());
            maxLineEdit->setEnabled(surfaceAtts->GetMaxFlag());
            maxToggle->blockSignals(false);
            break;

        case 7: // colorByZFlag 
            colorModeButtons->setButton(surfaceAtts->GetColorByZFlag() ? 0:1);
            break;

        case 8: // scaling
            scalingButtons->setButton(surfaceAtts->GetScaling());
            skewLineEdit->setEnabled(surfaceAtts->GetScaling() ==
                SurfaceAttributes::Skew);
            skewLabel->setEnabled(surfaceAtts->GetScaling() ==
                SurfaceAttributes::Skew);
            break;

        case  9: // lineStyle
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(surfaceAtts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;

        case 10: // lineWidth
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(surfaceAtts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;

        case 11: // surfaceColor
            { // new scope
            QColor temp(surfaceAtts->GetSurfaceColor().Red(),
                        surfaceAtts->GetSurfaceColor().Green(),
                        surfaceAtts->GetSurfaceColor().Blue());
            surfaceColor->blockSignals(true);
            surfaceColor->setButtonColor(temp);
            surfaceColor->blockSignals(false);
            }
            break;

        case 12: // wireframeColor
            { // new scope
            QColor temp(surfaceAtts->GetWireframeColor().Red(),
                        surfaceAtts->GetWireframeColor().Green(),
                        surfaceAtts->GetWireframeColor().Blue());
            wireframeColor->blockSignals(true);
            wireframeColor->setButtonColor(temp);
            wireframeColor->blockSignals(false);
            }
            break;

        case 13: // skewFactor
            temp.setNum(surfaceAtts->GetSkewFactor());
            skewLineEdit->setText(temp);
            break;

        case 14: // min
            temp.setNum(surfaceAtts->GetMin());
            minLineEdit->setText(temp);
            break;

        case 15: // max
            temp.setNum(surfaceAtts->GetMax());
            maxLineEdit->setText(temp);
            break;

        case 16: // colorTableName
            colorTableButton->setColorTable(
                              surfaceAtts->GetColorTableName().c_str());
            break;
        }

    } // end for
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values from the text fields and puts the values in the
//   surfaceAtts.
//
// Arguments:
//   which_widget : A number indicating which line edit for which to get
//                  the value. An index of -1 gets them all.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001 
//
// Modifications:
//   Kathleen Bonnell, Thu Oct 11 12:45:30 PDT 2001 
//   Changed old min/max to scalingMin/Max. Added coloringMin/Max.
//
//   Brad Whitlock, Fri Feb 15 11:51:09 PDT 2002
//   Fixed format strings.
//
//   Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002 
//   Changed back to min/max, removed scalingMin/Max and coloringMin/Max.
//
//   Brad Whitlock, Wed Apr 23 12:03:20 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisSurfacePlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do the skew factor.
    if(which_widget == 0 || doAll)
    {
        temp = skewLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                surfaceAtts->SetSkewFactor(val);
        }

        if(!okay)
        {
            msg = tr("The skew factor was invalid. "
                    "Resetting to the last good value of %1.").
                  arg(surfaceAtts->GetSkewFactor());
            Message(msg);
            surfaceAtts->SetSkewFactor(surfaceAtts->GetSkewFactor());
        }
    }

    // Do the coloring minimum value.
    if(which_widget == 1 || doAll)
    {
        temp = minLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            surfaceAtts->SetMin(val);
        }

        if(!okay)
        {
            msg = tr("The minimum value was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(surfaceAtts->GetMin());
            Message(msg);
            surfaceAtts->SetMin(surfaceAtts->GetMin());
        }
    }

    // Do the coloring maximum value
    if(which_widget == 2 || doAll)
    {
        temp = maxLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            surfaceAtts->SetMax(val);
        }

        if(!okay)
        {
            msg = tr("The maximum value was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(surfaceAtts->GetMax());
            Message(msg);
            surfaceAtts->SetMax(surfaceAtts->GetMax());
        }
    }

}

// ****************************************************************************
// Method: QvisSurfacePlotWindow::Apply
//
// Purpose:
//   This method applies the surface plot attributes and optionally
//   tells the viewer to apply them.
//
// Arguments:
//   ignore : This flag, when true, tells the code to ignore the
//            AutoUpdate function and tell the viewer to apply the
//            surface plot attributes.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001
//
// Modifications:
//   Eric Brugger, Fri Mar 16 16:17:18 PST 2001
//   I modified the routine to pass to the viewer proxy the plot
//   type stored within the class instead of the one hardwired from
//   an include file.
//
//   Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001
//   Removed error checking of var limits for log scaling, error checking
//   now performed at plot level.
// ****************************************************************************

void
QvisSurfacePlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current surface plot attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);

        surfaceAtts->Notify();

        // Tell the viewer to set the surface plot attributes.
        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        surfaceAtts->Notify();
}


//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisSurfacePlotWindow::apply
//
// Purpose:
//   This is a Qt slot function that is called when the window's Apply
//   button is clicked.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::apply()
{
    Apply(true);
}



// ****************************************************************************
// Method: QvisSurfacePlotWindow::makeDefault
//
// Purpose:
//   This is a Qt slot function that is called when the window's
//   "Make default" button is clicked.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001
//
// Modifications:
//    Eric Brugger, Fri Mar 16 16:17:18 PST 2001
//    I modified the routine to pass to the viewer proxy the plot
//    type stored within the class instead of the one hardwired from
//    an include file.
//
// ****************************************************************************

void
QvisSurfacePlotWindow::makeDefault()
{
    // Tell the viewer to set the default surface plot attributes.
    GetCurrentValues(-1);
    surfaceAtts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}



// ****************************************************************************
// Method: QvisSurfacePlotWindow::reset
//
// Purpose:
//   This is a Qt slot function that is called when the window's
//   Reset button is clicked.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::reset()
{
    // Tell the viewer to reset the surface plot attributes to the last
    // applied values.
    GetViewerMethods()->ResetPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::lineStyleChanged
//
// Purpose:
//   This is a Qt slot function that is called when the window's
//   line style is changed.
//
// Arguments:
//   newStyle : The new line style.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::lineStyleChanged(int newStyle)
{
    surfaceAtts->SetLineStyle(newStyle);
    Apply();
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::lineWidthChanged
//
// Purpose:
//   This is a Qt slot function that is called when the window's
//   line width widget is changed.
//
// Arguments:
//   newWidth : The new line width.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::lineWidthChanged(int newWidth)
{
    surfaceAtts->SetLineWidth(newWidth);
    Apply();
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::legendToggled
//
// Purpose:
//   This is a Qt slot function that is called when the window's
//   legend toggle button is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::legendToggled(bool val)
{
    surfaceAtts->SetLegendFlag(val);
    Apply();
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::lightingToggled
//
// Purpose:
//   This is a Qt slot function that is called when the window's
//   lighting toggle button is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::lightingToggled(bool val)
{
    surfaceAtts->SetLightingFlag(val);
    Apply();
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::scaleClicked
//
// Purpose:
//   This is a Qt slot function that is called when a scale button is clicked.
//
// Arguments:
//   button : Which scaling button was selected
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::scaleClicked(int button)
{
    if (button != surfaceAtts->GetScaling())
    {
        if(button == 1)
            surfaceAtts->SetScaling(SurfaceAttributes::Log);
        else if(button == 2)
            surfaceAtts->SetScaling(SurfaceAttributes::Skew);
        else
            surfaceAtts->SetScaling(SurfaceAttributes::Linear);
        Apply();
    }
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::surfaceToggled
//
// Purpose:
//   This is a Qt slot function that is called when the window's
//   surface toggle button is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::surfaceToggled(bool val)
{
    surfaceAtts->SetSurfaceFlag(val);
    Apply();
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::wireframeToggled
//
// Purpose:
//   This is a Qt slot function that is called when the window's
//   wireframe toggle button is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::wireframeToggled(bool val)
{
    surfaceAtts->SetWireframeFlag(val);
    Apply();
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::surfaceColorChanged
//
// Purpose:
//   This is a Qt slot function that is called when the surface color button's
//   color changes.
//
// Arguments:
//   color : The new surface color.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001
//
// Modifications:
//   Brad Whitlock, Sat Jun 16 18:39:59 PST 2001
//   Added a line of code to set the ColorByZ flag to false if we've selected
//   a new color for the flat-colored surface.
//
// ****************************************************************************

void
QvisSurfacePlotWindow::surfaceColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    surfaceAtts->SetSurfaceColor(temp);
    surfaceAtts->SetColorByZFlag(false);
    Apply();
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::wireframeColorChanged
//
// Purpose:
//   This is a Qt slot function that is called when the wireframe
//   color button's color changes.
//
// Arguments:
//   color : The new wireframe color.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::wireframeColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    surfaceAtts->SetWireframeColor(temp);
    Apply();
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::colorModeChanged
//
// Purpose:
//   This is a Qt slot function that is called when a
//   colorBy button is clicked.
//
// Arguments:
//   button  :  Which colorMode button was selected.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::colorModeChanged(int button)
{
    // Only do it if it changed.
    if (0 == button)
        surfaceAtts->SetColorByZFlag(true);
    else
        surfaceAtts->SetColorByZFlag(false);
    Apply();
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::processSkewText
//
// Purpose:
//   This is a Qt slot function that is called when the skew factor changes.
//
// Programmer: Kathleen Bonnell
// Creation:   March 06, 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::processSkewText()
{
    GetCurrentValues(0);
    Apply();
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::minToggled
//
// Purpose:
//   This is a Qt slot function that is called when the min toggle is clicked.
//
// Arguments:
//   val : The toggle's new value.
//
// Programmer: Kathleen Bonnell
// Creation:   October 11, 2001 
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::minToggled(bool val)
{
    surfaceAtts->SetMinFlag(val);
    Apply();
}

// ****************************************************************************
// Method: QvisSurfacePlotWindow::processMinLimitText
//
// Purpose:
//   This is a Qt slot function that is called when the min limit changes.
//
// Programmer: Kathleen Bonnell
// Creation:   October 11, 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::processMinLimitText()
{
    GetCurrentValues(1);
    Apply();
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::maxToggled
//
// Purpose:
//   This is a Qt slot function that is called when the max toggle is clicked.
//
// Arguments:
//   val : The toggle's new value.
//
// Programmer: Kathleen Bonnell
// Creation:   October 11, 2001 
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::maxToggled(bool val)
{
    surfaceAtts->SetMaxFlag(val);
    Apply();
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::processMaxLimitText
//
// Purpose:
//   This is a Qt slot function that is called when the max limit changes.
//
// Programmer: Kathleen Bonnell
// Creation:   October 11, 2001 
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::processMaxLimitText()
{
    GetCurrentValues(2);
    Apply();
}


// ****************************************************************************
// Method: QvisSurfacePlotWindow::colorTableClicked
//
// Purpose:
//   This is a Qt slot function that sets the desired color table name into
//   the surface plot attributes.
//
// Arguments:
//   <unnamed>  : If this is true, we want to use the default color table.
//   ctName     : The name of the color table to use if we're not going to
//                use the default.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 18:30:51 PST 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisSurfacePlotWindow::colorTableClicked(bool , const QString &ctName)
{
    surfaceAtts->SetColorByZFlag(true);
    surfaceAtts->SetColorTableName(ctName.latin1());
    Apply();
}

// ****************************************************************************
// Method: QvisSurfacePlotWindow::limitsSelectChanged
//
// Purpose:
//   This is a Qt slot function that sets 
//
// Arguments:
//   limitsMode : Which type of limits should be used for coloring. 
//              
//
// Programmer: Kathleen Bonnell 
// Creation:   October 11, 2001
//
// ****************************************************************************

void
QvisSurfacePlotWindow::limitsSelectChanged(int limitsMode)
{
    if (surfaceAtts->GetLimitsMode() != limitsMode)
    {
        if(limitsMode == 0)
            surfaceAtts->SetLimitsMode(SurfaceAttributes::OriginalData);
        else
            surfaceAtts->SetLimitsMode(SurfaceAttributes::CurrentPlot);
        Apply();
    }
}

