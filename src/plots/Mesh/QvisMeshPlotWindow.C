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

#include <QvisMeshPlotWindow.h>

#include <QWidget> 
#include <QLayout> 
#include <QPushButton> 
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QButtonGroup>
#include <QRadioButton>

#include <QvisColorButton.h>
#include <QvisPointControl.h>
#include <QvisColorManagerWidget.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisOpacitySlider.h>

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
    MeshAttributes *_meshAtts, const QString &caption, const QString &shortName,
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
//   Brad Whitlock, Wed Apr 23 09:58:27 PDT 2008
//   Added tr()'s
//
//   Cyrus Harrison, Fri Jul 18 14:44:51 PDT 2008
//   Initial Qt4 Port. 
//
//   Jeremy Meredith, Fri Feb 20 17:28:17 EST 2009
//   Added per-plot alpha (opacity) support.
//
// ****************************************************************************

void
QvisMeshPlotWindow::CreateWindowContents()
{
    //
    // Create the layout that we'll use.
    //
    QGridLayout *theLayout = new QGridLayout();
    topLayout->addLayout(theLayout);

    // Create the lineSyle widget.
    lineStyle = new QvisLineStyleWidget(0, central);
    theLayout->addWidget(lineStyle, 0, 1);
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    lineStyleLabel = new QLabel(tr("Line style"), central);
    theLayout->addWidget(lineStyleLabel, 0, 0);

    // Create the lineSyle widget.
    lineWidth = new QvisLineWidthWidget(0, central);
    theLayout->addWidget(lineWidth, 0, 3);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    lineWidthLabel = new QLabel(tr("Line width"),central);
    theLayout->addWidget(lineWidthLabel, 0, 2);

    // Create the mesh color button.
    meshColor = new QvisColorButton(central);
    connect(meshColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(meshColorChanged(const QColor &)));
    theLayout->addWidget(meshColor, 1, 1);
    meshColorLabel = new QLabel( tr("Mesh color"),central);
    theLayout->addWidget(meshColorLabel, 1, 0);

    // Create the foreground toggle
    foregroundToggle = new QCheckBox(tr("Use foreground"),central);
    connect(foregroundToggle, SIGNAL(toggled(bool)),
            this, SLOT(foregroundToggled(bool)));
    theLayout->addWidget(foregroundToggle, 1, 2, 1, 2);


    // Create the opaque color button.
    opaqueColor = new QvisColorButton(central);
    connect(opaqueColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(opaqueColorChanged(const QColor &)));
    theLayout->addWidget(opaqueColor, 2, 1);
    opaqueColorLabel = new QLabel(tr("Opaque color"),central);
    theLayout->addWidget(opaqueColorLabel, 2, 0);

    // Create the background toggle
    backgroundToggle = new QCheckBox(tr("Use background"), central);
    connect(backgroundToggle, SIGNAL(toggled(bool)),
            this, SLOT(backgroundToggled(bool)));
    theLayout->addWidget(backgroundToggle, 2, 2, 1, 2);

    // Create the opaque mode buttons
    opaqueModeGroup = new QButtonGroup(central);
    connect(opaqueModeGroup, SIGNAL(buttonClicked(int)), this,
            SLOT(opaqueModeChanged(int)));

    QGridLayout *opaqueModeLayout = new QGridLayout();
    opaqueModeLayout->setColumnStretch(4,1000);
    
    opaqueModeLayout->addWidget(new QLabel(tr("Opaque mode"), central), 0,0);
    
    QRadioButton *rb = new QRadioButton(tr("Auto"), central);
    opaqueModeGroup->addButton(rb,0);
    opaqueModeLayout->addWidget(rb, 0, 2);
    
    rb = new QRadioButton(tr("On"), central);
    opaqueModeGroup->addButton(rb,1);
    opaqueModeLayout->addWidget(rb, 0, 3);
    
    rb = new QRadioButton(tr("Off"), central);
    opaqueModeGroup->addButton(rb,2);
    opaqueModeLayout->addWidget(rb, 0, 4);
    
    theLayout->addLayout(opaqueModeLayout, 3, 0,1, 4);

    //
    // Create the opacity slider
    //
    opacityLabel = new QLabel(tr("Opacity"), central);
    theLayout->addWidget(opacityLabel, 4, 0);

    opacitySlider = new QvisOpacitySlider(0, 255, 25, 255, central);
    opacitySlider->setTickInterval(64);
    opacitySlider->setGradientColor(QColor(0, 0, 0));
    connect(opacitySlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(changedOpacity(int, const void*)));
    theLayout->addWidget(opacitySlider, 4, 1, 1, 3);
    opacityLabel->setBuddy(opacitySlider);


    // Create the showInternal toggle
    showInternalToggle = new QCheckBox(tr("Show Internal Zones"), central);
    connect(showInternalToggle, SIGNAL(toggled(bool)),
            this, SLOT(showInternalToggled(bool)));
    theLayout->addWidget(showInternalToggle, 5,0, 1,2);

    // Create the outline only toggle
    outlineOnlyToggle = new QCheckBox(tr("Outline only"), central);
    connect(outlineOnlyToggle, SIGNAL(toggled(bool)),
            this, SLOT(outlineOnlyToggled(bool)));
    theLayout->addWidget(outlineOnlyToggle, 6, 0);

    // Create the error tolerance line edit
    errorToleranceLineEdit = new QLineEdit(central);
    connect(errorToleranceLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processErrorToleranceText()));
    theLayout->addWidget(errorToleranceLineEdit, 6, 2, 1, 2);
    errorToleranceLabel = new QLabel(tr("Tolerance"),central);
    theLayout->addWidget(errorToleranceLabel, 6, 1);

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
    theLayout->addWidget(pointControl, 7, 0, 1, 4);
 

    // Create the legend toggle
    legendToggle = new QCheckBox(tr("Legend"), central);
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    theLayout->addWidget(legendToggle, 8, 0);

    // Create the smoothing level buttons
    smoothingLevelGroup = new QButtonGroup(central);
    connect(smoothingLevelGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(smoothingLevelChanged(int)));
            
    QGridLayout *smoothingLayout = new QGridLayout();
    smoothingLayout->setColumnStretch(4, 1000);
    smoothingLayout->addWidget(new QLabel(tr("Geometry smoothing"), central), 0,0);
    rb = new QRadioButton(tr("None"), central);
    smoothingLevelGroup->addButton(rb,0);
    smoothingLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("Fast"), central);
    smoothingLevelGroup->addButton(rb,1);
    smoothingLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton(tr("High"), central);
    smoothingLevelGroup->addButton(rb,1);
    smoothingLayout->addWidget(rb, 0, 3);
    theLayout->addLayout(smoothingLayout, 9,0 , 1,4);
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
//   Cyrus Harrison, Fri Jul 18 14:44:51 PDT 2008
//   Initial Qt4 Port. 
//
//   Jeremy Meredith, Fri Feb 20 17:28:17 EST 2009
//   Added per-plot alpha (opacity) support.
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
        case MeshAttributes::ID_legendFlag:
            legendToggle->blockSignals(true);
            legendToggle->setChecked(meshAtts->GetLegendFlag());
            legendToggle->blockSignals(false);
            break;
        case MeshAttributes::ID_lineStyle:
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(meshAtts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;
        case MeshAttributes::ID_lineWidth:
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(meshAtts->GetLineWidth());
            lineWidth->blockSignals(false);
        case MeshAttributes::ID_meshColor:
            { // new scope
            QColor temp(meshAtts->GetMeshColor().Red(),
                        meshAtts->GetMeshColor().Green(),
                        meshAtts->GetMeshColor().Blue());
            meshColor->blockSignals(true);
            meshColor->setButtonColor(temp);
            meshColor->blockSignals(false);
            }
            break;
        case MeshAttributes::ID_outlineOnlyFlag :
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
        case MeshAttributes::ID_errorTolerance:
            temp.setNum(meshAtts->GetErrorTolerance());
            errorToleranceLineEdit->setText(temp);
            break;
        case MeshAttributes::ID_opaqueMode:
            opaqueModeGroup->blockSignals(true);
            opaqueModeGroup->button((int)meshAtts->GetOpaqueMode())->setChecked(true);

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
            opaqueModeGroup->blockSignals(false);
            break;
        case MeshAttributes::ID_pointSize:
            pointControl->blockSignals(true);
            pointControl->SetPointSize(meshAtts->GetPointSize());
            pointControl->blockSignals(false);
            break;
        case MeshAttributes::ID_opaqueColor:
            { // new scope
            QColor temp(meshAtts->GetOpaqueColor().Red(),
                        meshAtts->GetOpaqueColor().Green(),
                        meshAtts->GetOpaqueColor().Blue());
            opaqueColor->blockSignals(true);
            opaqueColor->setButtonColor(temp);
            opaqueColor->blockSignals(false);
            }
            break;
        case MeshAttributes::ID_backgroundFlag:
            backgroundToggle->blockSignals(true);
            backgroundToggle->setChecked(meshAtts->GetBackgroundFlag());
            backgroundToggle->blockSignals(false);

            opaqueColor->setEnabled(!meshAtts->GetBackgroundFlag());
            opaqueColorLabel->setEnabled(!meshAtts->GetBackgroundFlag());
            break;
        case MeshAttributes::ID_foregroundFlag:
            foregroundToggle->blockSignals(true);
            foregroundToggle->setChecked(meshAtts->GetForegroundFlag());
            foregroundToggle->blockSignals(false);

            meshColor->setEnabled(!meshAtts->GetForegroundFlag());
            meshColorLabel->setEnabled(!meshAtts->GetForegroundFlag());
            break;
        case MeshAttributes::ID_smoothingLevel:
            smoothingLevelGroup->blockSignals(true);
            smoothingLevelGroup->button((int)meshAtts->GetSmoothingLevel())->setChecked(true);
            smoothingLevelGroup->blockSignals(false);
            break;
        case MeshAttributes::ID_pointSizeVarEnabled:
            pointControl->blockSignals(true);
            pointControl->SetPointSizeVarChecked(meshAtts->GetPointSizeVarEnabled());
            pointControl->blockSignals(false);
            break;
        case MeshAttributes::ID_pointSizeVar:
            pointControl->blockSignals(true);
            temp = QString(meshAtts->GetPointSizeVar().c_str());
            pointControl->SetPointSizeVar(temp);
            pointControl->blockSignals(false);
            break;
        case MeshAttributes::ID_pointType:
            pointControl->blockSignals(true);
            pointControl->SetPointType (meshAtts->GetPointType());
            pointControl->blockSignals(false);
            break;
        case MeshAttributes::ID_opaqueMeshIsAppropriate:
            // If in AUTO mode:  if OpaqueMesh is appropriate, handle 
            // opaqueColor and backgroundToggle according to the
            // appropriate values in meshAtts, otherwise disable them.
            if (meshAtts->GetOpaqueMode() == MeshAttributes::Auto)
            {
                opaqueModeGroup->blockSignals(true);
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
                opaqueModeGroup->blockSignals(false);
            }
            break;
        case MeshAttributes::ID_showInternal:
            showInternalToggle->blockSignals(true);
            showInternalToggle->setChecked(meshAtts->GetShowInternal());
            showInternalToggle->blockSignals(false);
            break;
        case MeshAttributes::ID_pointSizePixels:
            pointControl->blockSignals(true);
            pointControl->SetPointSizePixels(meshAtts->GetPointSizePixels());
            pointControl->blockSignals(false);
            break;
        case MeshAttributes::ID_opacity:
            opacitySlider->blockSignals(true);
            opacitySlider->setValue(int((float)meshAtts->GetOpacity() * 255.f));
            opacitySlider->blockSignals(false);
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
//   Brad Whitlock, Wed Apr 23 10:00:10 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Mon Aug 11 11:13:36 PDT 2008
//   Changed to new style.
//
// ****************************************************************************

void
QvisMeshPlotWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do the error tolerance.
    if(which_widget == MeshAttributes::ID_errorTolerance || doAll)
    {
        double val;
        if(LineEditGetDouble(errorToleranceLineEdit, val))
            meshAtts->SetErrorTolerance(val);
        else
        {
            ResettingError(tr("error tolerance"),
                DoubleToQString(meshAtts->GetErrorTolerance()));
            meshAtts->SetErrorTolerance(meshAtts->GetErrorTolerance());
        }
    }

    if(doAll)
    {
        meshAtts->SetPointSize(pointControl->GetPointSize());
        meshAtts->SetPointSizePixels(pointControl->GetPointSizePixels());
        meshAtts->SetPointSizeVar(pointControl->GetPointSizeVar().toStdString());
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
    GetCurrentValues(MeshAttributes::ID_errorTolerance);
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
    meshAtts->SetPointSizeVar(var.toStdString()); 
    Apply();
}

// ****************************************************************************
//  Method:  QvisMeshPlotWindow::changedOpacity
//
//  Purpose:
//    Callback function when opacity slider moves.
//
//  Arguments:
//    opacity    the new value
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 20, 2009
//
// ****************************************************************************
void
QvisMeshPlotWindow::changedOpacity(int opacity, const void*)
{
    meshAtts->SetOpacity((float)opacity/255.);
    Apply();
}

