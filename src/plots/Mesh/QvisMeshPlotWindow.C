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

#include <QvisMeshPlotWindow.h>

#include <qgrid.h> 
#include <qlayout.h> 
#include <qpushbutton.h> 
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include <QvisColorButton.h>
#include <QvisPointControl.h>
#include <QvisColorManagerWidget.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>

#include <MeshAttributes.h>
#include <ViewerProxy.h>


// ****************************************************************************
// Method: QvisMeshPlotWindow::QvisMeshPlotWindow
//
// Purpose: 
//   Constructor for the QvisMeshPlotWindow class.
//
// Arguments:
//   _meshAtts : A pointer to the MeshAttributes that the window will observe.
//  caption    : The caption of the window.
//  shortName  : The name used in the notepad area.
//  notepad    : A pointer to the notepad to which the window will post.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 16:43:05 PST 2001
//
// Modifications:
//
// ****************************************************************************

QvisMeshPlotWindow::QvisMeshPlotWindow(const int type,
    MeshAttributes *_meshAtts, const char *caption, const char *shortName,
    QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(_meshAtts, caption, shortName, notepad)
{
    plotType = type;
    meshAtts = _meshAtts;
}

// ****************************************************************************
// Method: QvisMeshPlotWindow::~QvisMeshPlotWindow
//
// Purpose: 
//   Destructor for the QvisMeshPlotWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 16:43:05 PST 2001
//
// Modifications:
//   
// ****************************************************************************

QvisMeshPlotWindow::~QvisMeshPlotWindow()
{
    meshAtts = 0;
}

// ****************************************************************************
// Method: QvisMeshPlotWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets that are in the window and sets
//   up their signals/slots.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 16:43:05 PST 2001
//
// Modifications:
//   
//   Kathleen Bonnell, Wed Sep  5 17:16:17 PDT 2001
//   Added support for opaqueColor.
//   
//   Kathleen Bonnell, Wed Sep 26 15:19:31 PDT 2001
//   Added foregroundToggle, backgroundToggle.
//
//   Jeremy Meredith, Tue Dec 10 10:27:34 PST 2002
//   Added smoothing options.
//
//   Jeremy Meredith, Fri Dec 20 11:36:03 PST 2002
//   Added scaling of point variables by a scalar field.
//
//   Hank Childs, Thu Aug 21 23:09:59 PDT 2003
//   Added support for more types of point glyphs.
//
//   Kathleen Bonnell, Thu Sep  4 11:15:30 PDT 2003 
//   Changed opaqueToggle checkbox to opaqueMode radio button group,
//   to support 'Auto', 'On' and 'Off' modes.  Moved outlineOnly
//   to same line of layout as errorTolerance. 
//
//   Kathleen Bonnell, Thu Feb  5 11:48:48 PST 2004 
//   Added showInternalToggle. 
// 
//   Jeremy Meredith, Tue May  4 13:23:10 PDT 2004
//   Added support for a new (Point) type of glyphing for point meshes.
//
//   Kathleen Bonnell, Fri Nov 12 10:51:59 PST 2004 
//   Replaced point-related control widgets with QvisPointControl. 
//
//   Brad Whitlock, Wed Jul 20 14:27:00 PST 2005
//   Added pointSizePixelsChanged slot.
//
// ****************************************************************************

void
QvisMeshPlotWindow::CreateWindowContents()
{
    //
    // Create the layout that we'll use.
    //
    QGridLayout *theLayout = new QGridLayout(topLayout, 9, 4);
    theLayout->setSpacing(10);

    // Create the lineSyle widget.
    lineStyle = new QvisLineStyleWidget(0, central, "lineStyle");
    theLayout->addWidget(lineStyle, 0, 1);
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    lineStyleLabel = new QLabel(lineStyle, "Line style",
                                central, "lineStyleLabel");
    theLayout->addWidget(lineStyleLabel, 0, 0);

    // Create the lineSyle widget.
    lineWidth = new QvisLineWidthWidget(0, central, "lineWidth");
    theLayout->addWidget(lineWidth, 0, 3);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    lineWidthLabel = new QLabel(lineWidth, "Line width",
                                central, "lineWidthLabel");
    theLayout->addWidget(lineWidthLabel, 0, 2);

    // Create the mesh color button.
    meshColor = new QvisColorButton(central, "meshColorButton");
    connect(meshColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(meshColorChanged(const QColor &)));
    theLayout->addWidget(meshColor, 1, 1);
    meshColorLabel = new QLabel(meshColor, "Mesh color",
                                central, "meshColorLabel");
    theLayout->addWidget(meshColorLabel, 1, 0);

    // Create the foreground toggle
    foregroundToggle = new QCheckBox("Use foreground", 
                                     central, "foregroundToggle");
    connect(foregroundToggle, SIGNAL(toggled(bool)),
            this, SLOT(foregroundToggled(bool)));
    theLayout->addMultiCellWidget(foregroundToggle, 1, 1, 2, 3);


    // Create the opaque color button.
    opaqueColor = new QvisColorButton(central, "opaqueColorButton");
    connect(opaqueColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(opaqueColorChanged(const QColor &)));
    theLayout->addWidget(opaqueColor, 2, 1);
    opaqueColorLabel = new QLabel(opaqueColor, "Opaque color",
                                  central, "opaqueColorLabel");
    theLayout->addWidget(opaqueColorLabel, 2, 0);

    // Create the background toggle
    backgroundToggle = new QCheckBox("Use background", 
                                     central, "backgroundToggle");
    connect(backgroundToggle, SIGNAL(toggled(bool)),
            this, SLOT(backgroundToggled(bool)));
    theLayout->addMultiCellWidget(backgroundToggle, 2, 2, 2, 3);

    // Create the opaque mode buttons
    opaqueMode = new QButtonGroup(0, "opaqueMode");
    connect(opaqueMode, SIGNAL(clicked(int)), this,
            SLOT(opaqueModeChanged(int)));
    QGridLayout *opaqueModeLayout = new QGridLayout(1, 5);
    opaqueModeLayout->setSpacing(10);
    opaqueModeLayout->setColStretch(4, 1000);
    opaqueModeLayout->addWidget(new QLabel("Opaque mode", central), 0,0);
    QRadioButton *rb = new QRadioButton("Auto", central, "auto");
    opaqueMode->insert(rb);
    opaqueModeLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton("On", central, "on");
    opaqueMode->insert(rb);
    opaqueModeLayout->addWidget(rb, 0, 3);
    rb = new QRadioButton("Off", central, "off");
    opaqueMode->insert(rb);
    opaqueModeLayout->addWidget(rb, 0, 4);
    theLayout->addMultiCellLayout(opaqueModeLayout, 3, 3, 0, 3);

    // Create the showInternal toggle
    showInternalToggle = new QCheckBox("Show Internal Zones", central, 
                                       "showInternalToggle");
    connect(showInternalToggle, SIGNAL(toggled(bool)),
            this, SLOT(showInternalToggled(bool)));
    theLayout->addMultiCellWidget(showInternalToggle, 4,4, 0,1);

    // Create the outline only toggle
    outlineOnlyToggle = new QCheckBox("Outline only", central,
                                      "OutlineOnlyToggle");
    connect(outlineOnlyToggle, SIGNAL(toggled(bool)),
            this, SLOT(outlineOnlyToggled(bool)));
    theLayout->addWidget(outlineOnlyToggle, 5, 0);

    // Create the error tolerance line edit
    errorToleranceLineEdit = new QLineEdit(central, "errorToleranceLineEdit");
    connect(errorToleranceLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processErrorToleranceText()));
    theLayout->addMultiCellWidget(errorToleranceLineEdit, 5, 5, 2, 3);
    errorToleranceLabel = new QLabel(errorToleranceLineEdit, "Tolerance",
                                        central, "errorToleranceLabel");
    theLayout->addWidget(errorToleranceLabel, 5, 1);
    //errorToleranceLineEdit->setEnabled(false); 
    //errorToleranceLabel->setEnabled(false); 

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
    theLayout->addMultiCellWidget(pointControl, 6, 6, 0, 3);
 

    // Create the legend toggle
    legendToggle = new QCheckBox("Legend", central, "legendToggle");
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    theLayout->addWidget(legendToggle, 7, 0);

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
    theLayout->addMultiCellLayout(smoothingLayout, 8,8 , 0,3);
}

// ****************************************************************************
// Method: QvisMeshPlotWindow::UpdateWindow
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
// Creation:   Fri Mar 9 17:10:54 PST 2001
//
// Modifications:
//
//   Kathleen Bonnell, Thu Jun 21 16:33:54 PDT 2001
//   Enable lineStyle, lineStyle label.
//   
//   Kathleen Bonnell, Fri Jun 29 14:37:18 PDT 2001 
//   Enable opaqueToggle.
//   
//   Kathleen Bonnell, Wed Sep  5 17:16:17 PDT 2001
//   Added support for opaqueColor.
//   
//   Kathleen Bonnell, Wed Sep 26 15:19:31 PDT 2001
//   Added foregroundToggle, backgroundToggle.
//
//   Jeremy Meredith, Tue Dec 10 10:27:44 PST 2002
//   Added smoothing options.
//
//   Jeremy Meredith, Fri Dec 20 11:36:03 PST 2002
//   Added scaling of point variables by a scalar field.
//
//   Hank Childs, Thu Aug 21 23:17:17 PDT 2003
//   Added support for point type.
//
//   Kathleen Bonnell, Wed Sep  3 08:46:25 PDT 2003 
//   Added support for opaqueMeshIsAppropriate. 
//
//   Kathleen Bonnell, Thu Sep  4 11:15:30 PDT 2003 
//   MeshAtts' 'opaqueFlag' bool is now an 'opaqueMode' enum, support it. 
//
//   Kathleen Bonnell, Thu Feb  5 11:48:48 PST 2004 
//   Added 'showInternalToggle'. 
//
//   Jeremy Meredith, Tue May  4 13:23:10 PDT 2004
//   Added support for a new (Point) type of glyphing for point meshes.
//   When doing GL_POINT, we ignore point size, so also disable it.
//
//   Kathleen Bonnell, Fri Nov 12 10:51:59 PST 2004 
//   Replaced point-related widgets with  pointControl. 
//
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Mark C. Miller, Mon Dec  6 13:30:51 PST 2004
//   Fixed SGI compiler error with string conversion to QString
//
//   Brad Whitlock, Wed Jul 20 15:01:21 PST 2005
//   Added pointSizePixels.
//
// ****************************************************************************

void
QvisMeshPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < meshAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!meshAtts->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case 0: // legendFlag
            legendToggle->blockSignals(true);
            legendToggle->setChecked(meshAtts->GetLegendFlag());
            legendToggle->blockSignals(false);
            break;
        case 1: // lineStyle
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(meshAtts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;
        case 2: // lineWidth
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(meshAtts->GetLineWidth());
            lineWidth->blockSignals(false);
        case 3: // meshColor
            { // new scope
            QColor temp(meshAtts->GetMeshColor().Red(),
                        meshAtts->GetMeshColor().Green(),
                        meshAtts->GetMeshColor().Blue());
            meshColor->blockSignals(true);
            meshColor->setButtonColor(temp);
            meshColor->blockSignals(false);
            }
            break;
        case 4: // outlineOnlyFlag 
            // disabled until implemented
            outlineOnlyToggle->setEnabled(false);
            errorToleranceLineEdit->setEnabled(false);
            errorToleranceLabel->setEnabled(false);
            // 
            // Uncomment following and delete above when implemented
            // 
            //outlineOnlyToggle->blockSignals(true);
            //outlineOnlyToggle->setChecked(meshAtts->GetOutlineOnlyFlag());
            //outlineOnlyToggle->blockSignals(false);
        
            //errorToleranceLineEdit->setEnabled(meshAtts->GetOutlineOnlyFlag());
            //errorToleranceLabel->setEnabled(meshAtts->GetOutlineOnlyFlag());
            break;
        case 5: // errorTolerance
            temp.setNum(meshAtts->GetErrorTolerance());
            errorToleranceLineEdit->setText(temp);
            break;
        case 6: // opaqueMode
            opaqueMode->blockSignals(true);
            opaqueMode->setButton((int)meshAtts->GetOpaqueMode());

            switch(meshAtts->GetOpaqueMode())   
            {
                 case MeshAttributes::Auto:
                     if (meshAtts->GetOpaqueMeshIsAppropriate())
                     {
                         opaqueColor->setEnabled(!meshAtts->GetBackgroundFlag());
                         opaqueColorLabel->setEnabled(!meshAtts->GetBackgroundFlag());
                         backgroundToggle->setEnabled(true);
                     }
                     else 
                     {
                         opaqueColor->setEnabled(false);
                         opaqueColorLabel->setEnabled(false);
                         backgroundToggle->setEnabled(false);
                     }
                     break;
                 case MeshAttributes::On:
                     opaqueColor->setEnabled(!meshAtts->GetBackgroundFlag());
                     opaqueColorLabel->setEnabled(!meshAtts->GetBackgroundFlag());
                     backgroundToggle->setEnabled(true);
                     break;
                 case MeshAttributes::Off:
                     opaqueColor->setEnabled(false);
                     opaqueColorLabel->setEnabled(false);
                     backgroundToggle->setEnabled(false);
                     break;
            }
            opaqueMode->blockSignals(false);
            break;
        case 7: // pointSize
            pointControl->blockSignals(true);
            pointControl->SetPointSize(meshAtts->GetPointSize());
            pointControl->blockSignals(false);
            break;
        case 8: // opaqueColor
            { // new scope
            QColor temp(meshAtts->GetOpaqueColor().Red(),
                        meshAtts->GetOpaqueColor().Green(),
                        meshAtts->GetOpaqueColor().Blue());
            opaqueColor->blockSignals(true);
            opaqueColor->setButtonColor(temp);
            opaqueColor->blockSignals(false);
            }
            break;
        case 9: // backgroundFlag
            backgroundToggle->blockSignals(true);
            backgroundToggle->setChecked(meshAtts->GetBackgroundFlag());
            backgroundToggle->blockSignals(false);

            opaqueColor->setEnabled(!meshAtts->GetBackgroundFlag());
            opaqueColorLabel->setEnabled(!meshAtts->GetBackgroundFlag());
            break;
        case 10: // foregroundFlag
            foregroundToggle->blockSignals(true);
            foregroundToggle->setChecked(meshAtts->GetForegroundFlag());
            foregroundToggle->blockSignals(false);

            meshColor->setEnabled(!meshAtts->GetForegroundFlag());
            meshColorLabel->setEnabled(!meshAtts->GetForegroundFlag());
            break;
        case 11: // smoothingLevel
            smoothingLevelButtons->blockSignals(true);
            smoothingLevelButtons->setButton((int)meshAtts->GetSmoothingLevel());
            smoothingLevelButtons->blockSignals(false);
            break;
        case 12: // pointSizeVarEnabled
            pointControl->blockSignals(true);
            pointControl->SetPointSizeVarChecked(meshAtts->GetPointSizeVarEnabled());
            pointControl->blockSignals(false);
            break;
        case 13: // pointSizeVar
            pointControl->blockSignals(true);
            temp = QString(meshAtts->GetPointSizeVar().c_str());
            pointControl->SetPointSizeVar(temp);
            pointControl->blockSignals(false);
            break;
        case 14: // pointType
            pointControl->blockSignals(true);
            pointControl->SetPointType (meshAtts->GetPointType());
            pointControl->blockSignals(false);
            break;
        case 15: // opaqueMeshIsAppropriate
            // If in AUTO mode:  if OpaqueMesh is appropriate, handle 
            // opaqueColor and backgroundToggle according to the
            // appropriate values in meshAtts, otherwise disable them.
            if (meshAtts->GetOpaqueMode() == MeshAttributes::Auto)
            {
                opaqueMode->blockSignals(true);
                if (meshAtts->GetOpaqueMeshIsAppropriate())
                {
                    opaqueColor->setEnabled(!meshAtts->GetBackgroundFlag());
                    opaqueColorLabel->setEnabled(!meshAtts->GetBackgroundFlag());
                    backgroundToggle->setEnabled(true);
                }
                else 
                {
                    opaqueColor->setEnabled(false);
                    opaqueColorLabel->setEnabled(false);
                    backgroundToggle->setEnabled(false);
                }
                opaqueMode->blockSignals(false);
            }
            break;
        case 16: // showInternal
            showInternalToggle->blockSignals(true);
            showInternalToggle->setChecked(meshAtts->GetShowInternal());
            showInternalToggle->blockSignals(false);
            break;
        case 17: // pointSizePixels
            pointControl->blockSignals(true);
            pointControl->SetPointSizePixels(meshAtts->GetPointSizePixels());
            pointControl->blockSignals(false);
            break;
        }
    } // end for
}


// ****************************************************************************
// Method: QvisMeshPlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values for one or all of the lineEdit widgets.
//
// Arguments:
//   which_widget : The number of the widget to update. If -1 is passed,
//                  the routine gets the current values for all widgets.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 17:18:54 PST 2001
//
// Modifications:
//   Jeremy Meredith, Fri Dec 20 11:36:03 PST 2002
//   Added scaling of point variables by a scalar field.
//
//   Kathleen Bonnell, Fri Nov 12 10:58:37 PST 2004 
//   Replace separate pointSize and pointSizeVar widgets with single
//   pointControl. 
//
// ****************************************************************************

void
QvisMeshPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do the error tolerance.
    if(which_widget == 0 || doAll)
    {
        temp = errorToleranceLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            meshAtts->SetErrorTolerance(val);
        }

        if(!okay)
        {
            msg.sprintf("The error tolerance was invalid. "
                "Resetting to the last good value of %g.",
                meshAtts->GetErrorTolerance());
            Message(msg);
            meshAtts->SetErrorTolerance(meshAtts->GetErrorTolerance());
        }
    }

    if(doAll)
    {
        meshAtts->SetPointSize(pointControl->GetPointSize());
        meshAtts->SetPointSizePixels(pointControl->GetPointSizePixels());
        meshAtts->SetPointSizeVar(pointControl->GetPointSizeVar().latin1());
    }
}

// ****************************************************************************
// Method: QvisMeshPlotWindow::Apply
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
//   
// ****************************************************************************

void
QvisMeshPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current aslice attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        meshAtts->Notify();

        // Tell the viewer to set the mesh attributes.
        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        meshAtts->Notify();
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisMeshPlotWindow::apply
//
// Purpose: 
//   This a Qt slot function that is called when the window's Apply button is
//   clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 17:29:03 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisMeshPlotWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisMeshPlotWindow::makeDefault
//
// Purpose: 
//   This is a Qt slot function that is called when the window's "Make default"
//   button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 17:29:32 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisMeshPlotWindow::makeDefault()
{
    // Tell the viewer to set the default mesh attributes.
    GetCurrentValues(-1);
    meshAtts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}

// ****************************************************************************
// Method: QvisMeshPlotWindow::reset
//
// Purpose: 
//   This is a Qt slot function that is called when the window's "reset"
//   button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 17:30:07 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisMeshPlotWindow::reset()
{
    // Tell the viewer to reset the mesh attributes to the last
    // applied values.
    GetViewerMethods()->ResetPlotOptions(plotType);
}

// ****************************************************************************
// Method: QvisMeshPlotWindow::lineStyleChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the line style changes.
//
// Arguments:
//   newStyle : The new line style.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 17:30:54 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisMeshPlotWindow::lineStyleChanged(int newStyle)
{
    meshAtts->SetLineStyle(newStyle);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisMeshPlotWindow::lineWidthChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the line width changes.
//
// Arguments:
//   newWidth : The new line width.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 17:31:14 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisMeshPlotWindow::lineWidthChanged(int newWidth)
{
    meshAtts->SetLineWidth(newWidth);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisMeshPlotWindow::legendToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the legend toggle is
//   toggled.
//
// Arguments:
//   val : The new legend toggle state.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 17:31:32 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisMeshPlotWindow::legendToggled(bool val)
{
    meshAtts->SetLegendFlag(val);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisMeshPlotWindow::showInternalToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the showInternal toggle is
//   toggled.
//
// Arguments:
//   val : The new showInternal toggle state.
//
// Programmer: Kathleen Bonnell 
// Creation:   February 5, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
QvisMeshPlotWindow::showInternalToggled(bool val)
{
    meshAtts->SetShowInternal(val);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisMeshPlotWindow::meshColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the mesh color changes.
//
// Arguments:
//   color : The new mesh color.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 17:32:39 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisMeshPlotWindow::meshColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    meshAtts->SetMeshColor(temp);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisMeshPlotWindow::opaqueColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the opaque color changes.
//
// Arguments:
//   color      The new opaque color.
//
// Programmer:  Kathleen Bonnell 
// Creation:    Wed Sep  5 16:20:52 PDT 2001 
//
// ****************************************************************************

void
QvisMeshPlotWindow::opaqueColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    meshAtts->SetOpaqueColor(temp);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisMeshPlotWindow::outlineOnlyToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the outline only toggle
//   button is toggled.
//
// Arguments:
//   val : The new state of the "outline only" toggle.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 17:33:10 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisMeshPlotWindow::outlineOnlyToggled(bool val)
{
    meshAtts->SetOutlineOnlyFlag(val);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisMeshPlotWindow::opaqueModeChanged
//
//  Purpose:
//    Qt slot function that is called when one of the opaque mode buttons
//    is clicked.
//
//  Arguments:
//    val   :   The new mode
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 17:33:10 PST 2001
//
// Modifications:
//   Kathleen Bonnell, Thu Sep  4 11:15:30 PDT 2003
//   Modified to support radio buttons instead of check box. 
//   
// ****************************************************************************

void
QvisMeshPlotWindow::opaqueModeChanged(int val)
{
    meshAtts->SetOpaqueMode((MeshAttributes::OpaqueMode) val);
    Apply();
}


// ****************************************************************************
// Method: QvisMeshPlotWindow::backgroundToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the background toggle
//   button is toggled.
//
// Arguments:
//   val : The new state of the background toggle.
//
// Programmer: Kathleen Bonnell 
// Creation:   September 26, 2001 
//
// ****************************************************************************

void
QvisMeshPlotWindow::backgroundToggled(bool val)
{
    meshAtts->SetBackgroundFlag(val);
    Apply();
}


// ****************************************************************************
// Method: QvisMeshPlotWindow::foregroundToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the foreground toggle
//   button is toggled.
//
// Arguments:
//   val : The new state of the foreground toggle.
//
// Programmer: Kathleen Bonnell 
// Creation:   September 26, 2001 
//   
// ****************************************************************************

void
QvisMeshPlotWindow::foregroundToggled(bool val)
{
    meshAtts->SetForegroundFlag(val);
    Apply();
}

// ****************************************************************************
// Method: QvisMeshPlotWindow::processErrorToleranceText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   error tolerance text and pressed the Enter key.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 17:34:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisMeshPlotWindow::processErrorToleranceText()
{
    GetCurrentValues(0);
    Apply();
}


// ****************************************************************************
//  Method:  QvisMeshPlotWindow::smoothingLevelChanged
//
//  Purpose:
//    Qt slot function that is called when one of the smoothing buttons
//    is clicked.
//
//  Arguments:
//    level  :   The new level.
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2002
//
//  Modifications:
//
// ****************************************************************************

void
QvisMeshPlotWindow::smoothingLevelChanged(int level)
{
    meshAtts->SetSmoothingLevel((MeshAttributes::SmoothingLevel) level);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
//  Method:  QvisMeshPlotWindow::pointTypeChanged
//
//  Purpose:
//    Qt slot function that is called when one of the point type buttons
//    is clicked.
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
//   Kathleen Bonnell, Fri Nov 12 11:04:56 PST 2004 
//   Setting enabled state of pointSizeLineEdit handled by QvisPointControl. 
//
// ****************************************************************************

void
QvisMeshPlotWindow::pointTypeChanged(int type)
{
    meshAtts->SetPointType((MeshAttributes::PointType) type);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
//  Method:  QvisMeshPlotWindow::pointSizeChanged
//
//  Purpose:
//   This is a Qt slot function that is called when the user changes the
//   point size text and presses the Enter key.
//
//  Arguments:
//    size       The new point size
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 12, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
QvisMeshPlotWindow::pointSizeChanged(double size)
{
    meshAtts->SetPointSize(size); 
    Apply();
}

// ****************************************************************************
// Method: QvisMeshPlotWindow::pointSizePixelsChanged
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
QvisMeshPlotWindow::pointSizePixelsChanged(int size)
{
    meshAtts->SetPointSizePixels(size); 
    Apply();
}

// ****************************************************************************
// Method: QvisMeshPlotWindow::pointSizeVarToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the pointSizeVar toggle
//   button is toggled.
//
// Arguments:
//   val : The new state of the pointSizeVar toggle.
//
// Programmer: Jeremy Meredith
// Creation:   December 19, 2002
//   
// ****************************************************************************

void
QvisMeshPlotWindow::pointSizeVarToggled(bool val)
{
    meshAtts->SetPointSizeVarEnabled(val);
    Apply();
}


// ****************************************************************************
// Method: QvisMeshPlotWindow::pointSizeVarChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   point size variable text and presses the Enter key.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 12, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
QvisMeshPlotWindow::pointSizeVarChanged(const QString &var)
{
    meshAtts->SetPointSizeVar(var.latin1()); 
    Apply();
}
