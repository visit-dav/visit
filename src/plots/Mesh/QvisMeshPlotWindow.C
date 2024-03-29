// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisMeshPlotWindow.h>

#include <QWidget> 
#include <QLayout> 
#include <QGroupBox> 
#include <QLabel>
#include <QCheckBox>
#include <QButtonGroup>
#include <QRadioButton>

#include <QvisColorButton.h>
#include <QvisPointControl.h>
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
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Brad Whitlock, Wed Mar 24 16:42:50 PDT 2010
//   Subtle layout tweaks.
//
//   Kathleen Biagas, Thu Apr 23 13:13:16 PDT 2015
//   Removed never used outlineOnly and errorTolerance widgets.
//
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisMeshPlotWindow::CreateWindowContents()
{
    //
    // Create the plot options
    //
    QGroupBox * zoneGroup = new QGroupBox(central);
    zoneGroup->setTitle(tr("Zone"));
    topLayout->addWidget(zoneGroup);

    QGridLayout *zoneLayout = new QGridLayout(zoneGroup);
    zoneLayout->setContentsMargins(5,5,5,5);
    zoneLayout->setSpacing(10);
 
    // Create the showInternal toggle
    showInternalToggle = new QCheckBox(tr("Show internal zones"), central);
    connect(showInternalToggle, SIGNAL(toggled(bool)),
            this, SLOT(showInternalToggled(bool)));
    zoneLayout->addWidget(showInternalToggle, 0, 0, 1, 2);

    //
    // Create the color stuff
    //
    QGroupBox * colorGroup = new QGroupBox(central);
    colorGroup->setTitle(tr("Color"));
    topLayout->addWidget(colorGroup);

    QGridLayout *colorLayout = new QGridLayout(colorGroup);
    colorLayout->setContentsMargins(5,5,5,5);
    colorLayout->setSpacing(10); 
    
    // Create the radio buttons for mesh color source
    colorLayout->addWidget(new QLabel(tr("Mesh color"), central), 0, 0);

    meshColorButtons = new QButtonGroup(central);

    QRadioButton * rb = new QRadioButton(tr("Foreground"), central);
    rb->setChecked(true);
    meshColorButtons->addButton(rb, 0);
    colorLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("Random"), central);
    meshColorButtons->addButton(rb, 2);
    colorLayout->addWidget(rb, 0, 2, Qt::AlignRight | Qt::AlignVCenter);
    rb = new QRadioButton(tr("Custom"), central);
    meshColorButtons->addButton(rb, 1);
    colorLayout->addWidget(rb, 0, 3, Qt::AlignRight | Qt::AlignVCenter);

    // Each time a radio button is clicked, call the scale clicked slot.
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(meshColorButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(meshColorClicked(int)));
#else
    connect(meshColorButtons, SIGNAL(idClicked(int)),
            this, SLOT(meshColorClicked(int)));
#endif

    // Create the mesh color button.
    meshColor = new QvisColorButton(central);
    connect(meshColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(meshColorChanged(const QColor &)));
    colorLayout->addWidget(meshColor, 0, 4);

    // Create the radio buttons for opaque color source
    opaqueColorLabel = new QLabel(tr("Opaque color"), central);
    colorLayout->addWidget(opaqueColorLabel, 1, 0);

    opaqueColorButtons = new QButtonGroup(central);

    rb = new QRadioButton(tr("Background"), central);
    rb->setChecked(true);
    opaqueColorButtons->addButton(rb, 0);
    colorLayout->addWidget(rb, 1, 1);
    rb = new QRadioButton(tr("Random"), central);
    opaqueColorButtons->addButton(rb, 2);
    colorLayout->addWidget(rb, 1, 2, Qt::AlignRight | Qt::AlignVCenter);
    rb = new QRadioButton(tr("Custom"), central);
    opaqueColorButtons->addButton(rb, 1);
    colorLayout->addWidget(rb, 1, 3, Qt::AlignRight | Qt::AlignVCenter);

    // Each time a radio button is clicked, call the scale clicked slot.
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(opaqueColorButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(opaqueColorClicked(int)));
#else
    connect(opaqueColorButtons, SIGNAL(idClicked(int)),
            this, SLOT(opaqueColorClicked(int)));
#endif

    // Create the opaque color button.
    opaqueColor = new QvisColorButton(central);
    connect(opaqueColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(opaqueColorChanged(const QColor &)));
    colorLayout->addWidget(opaqueColor, 1, 4);

    // Create the opaque mode buttons
    colorLayout->addWidget(new QLabel(tr("Opaque mode"), central), 2, 0);
    opaqueModeGroup = new QButtonGroup(central);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(opaqueModeGroup, SIGNAL(buttonClicked(int)), this,
            SLOT(opaqueModeChanged(int)));
#else
    connect(opaqueModeGroup, SIGNAL(idClicked(int)), this,
            SLOT(opaqueModeChanged(int)));
#endif

    QHBoxLayout *opaqueModeLayout = new QHBoxLayout();
    opaqueModeLayout->setContentsMargins(0,0,0,0);
    opaqueModeLayout->setSpacing(10);
   
    rb = new QRadioButton(tr("Auto"), central);
    opaqueModeGroup->addButton(rb,0);
    opaqueModeLayout->addWidget(rb);
    
    rb = new QRadioButton(tr("On"), central);
    opaqueModeGroup->addButton(rb,1);
    opaqueModeLayout->addWidget(rb);
    
    rb = new QRadioButton(tr("Off"), central);
    opaqueModeGroup->addButton(rb,2);
    opaqueModeLayout->addWidget(rb);
    
    colorLayout->addLayout(opaqueModeLayout, 2, 1, 1, 3);

    //
    // Create the opacity slider
    //
    opacityLabel = new QLabel(tr("Opacity"), central);
    colorLayout->addWidget(opacityLabel, 3, 0);

    opacitySlider = new QvisOpacitySlider(0, 255, 25, 255, central);
    opacitySlider->setTickInterval(64);
    opacitySlider->setGradientColor(QColor(0, 0, 0));
    connect(opacitySlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(changedOpacity(int, const void*)));
    colorLayout->addWidget(opacitySlider, 3, 1, 1, 3);
    opacityLabel->setBuddy(opacitySlider);


    //
    // Create the style stuff
    //

    QGroupBox * styleGroup = new QGroupBox(central);
    styleGroup->setTitle(tr("Point / Line Style"));
    topLayout->addWidget(styleGroup);

    QGridLayout *styleLayout = new QGridLayout(styleGroup);
    styleLayout->setContentsMargins(5,5,5,5);
    styleLayout->setSpacing(10);
 
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
    styleLayout->addWidget(pointControl, 0, 0, 1, 4);


    // Create the lineWidth widget.
    styleLayout->addWidget(new QLabel(tr("Line width"), central), 1, 0);

    lineWidth = new QvisLineWidthWidget(0, central);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    styleLayout->addWidget(lineWidth, 1, 1);

    //
    // Create the geometry group
    //
    QGroupBox * smoothingGroup = new QGroupBox(central);
    smoothingGroup->setTitle(tr("Geometry"));
    topLayout->addWidget(smoothingGroup);

    QGridLayout *smoothingLayout = new QGridLayout(smoothingGroup);
    smoothingLayout->setContentsMargins(5,5,5,5);
    smoothingLayout->setSpacing(10);
    
    smoothingLayout->addWidget(new QLabel(tr("Smoothing"), central), 0,0);

    // Create the smoothing level buttons
    smoothingLevelButtons = new QButtonGroup(central);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(smoothingLevelButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(smoothingLevelChanged(int)));
#else
    connect(smoothingLevelButtons, SIGNAL(idClicked(int)),
            this, SLOT(smoothingLevelChanged(int)));
#endif

    rb = new QRadioButton(tr("None"), central);
    smoothingLevelButtons->addButton(rb, 0);
    smoothingLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("Fast"), central);
    smoothingLevelButtons->addButton(rb, 1);
    smoothingLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton(tr("High"), central);
    smoothingLevelButtons->addButton(rb, 2);
    smoothingLayout->addWidget(rb, 0, 3);


    //
    // Create the misc stuff
    //
    QGroupBox * miscGroup = new QGroupBox(central);
    miscGroup->setTitle(tr("Misc"));
    topLayout->addWidget(miscGroup);

    QGridLayout *miscLayout = new QGridLayout(miscGroup);
    miscLayout->setContentsMargins(5,5,5,5);
    miscLayout->setSpacing(10);
 
    // Create the legend toggle
    legendToggle = new QCheckBox(tr("Legend"), central);
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    miscLayout->addWidget(legendToggle, 0, 0);

    // Create the lighting toggle
//     lightingToggle = new QCheckBox(tr("Lighting"), central);
//     connect(lightingToggle, SIGNAL(toggled(bool)),
//             this, SLOT(lightingToggled(bool)));
//     miscLayout->addWidget(lightingToggle, 0, 1);

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
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Kathleen Biagas, Thu Apr 23 13:13:16 PDT 2015
//   Removed never used outlineOnly and errorTolerance widgets.
//
// ****************************************************************************

void
QvisMeshPlotWindow::UpdateWindow(bool doAll)
{
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
        case MeshAttributes::ID_opaqueMode:
            opaqueModeGroup->blockSignals(true);
            opaqueModeGroup->button((int)meshAtts->GetOpaqueMode())->setChecked(true);

            switch(meshAtts->GetOpaqueMode())   
            {
                 case MeshAttributes::Auto:
                     if (meshAtts->GetOpaqueMeshIsAppropriate())
                     {
                         opaqueColorLabel->setEnabled(true);
                         opaqueColorButtons->button(0)->setEnabled(true);
                         opaqueColorButtons->button(1)->setEnabled(true);

                         opaqueColor->setEnabled(meshAtts->GetOpaqueColorSource() == MeshAttributes::OpaqueCustom);
                     }
                     else 
                     {
                         opaqueColorLabel->setEnabled(false);
                         opaqueColorButtons->button(0)->setEnabled(false);
                         opaqueColorButtons->button(1)->setEnabled(false);

                         opaqueColor->setEnabled(false);
                     }
                     break;
                 case MeshAttributes::On:
                     opaqueColorLabel->setEnabled(true);
                     opaqueColorButtons->button(0)->setEnabled(true);
                     opaqueColorButtons->button(1)->setEnabled(true);
                     opaqueColor->setEnabled(meshAtts->GetOpaqueColorSource() == MeshAttributes::OpaqueCustom);
                     break;
                 case MeshAttributes::Off:
                     opaqueColorLabel->setEnabled(false);
                     opaqueColorButtons->button(0)->setEnabled(false);
                     opaqueColorButtons->button(1)->setEnabled(false);
                     opaqueColor->setEnabled(meshAtts->GetOpaqueColorSource() == MeshAttributes::OpaqueCustom);
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
        case MeshAttributes::ID_meshColorSource:
            meshColorButtons->blockSignals(true);
            meshColorButtons->button(meshAtts->GetMeshColorSource())->setChecked(true);
            meshColorButtons->blockSignals(false);

            meshColor->setEnabled(meshAtts->GetMeshColorSource() == MeshAttributes::MeshCustom);

            break;
        case MeshAttributes::ID_opaqueColorSource:
            opaqueColorButtons->blockSignals(true);
            opaqueColorButtons->button(meshAtts->GetOpaqueColorSource())->setChecked(true);
            opaqueColorButtons->blockSignals(false);

            opaqueColor->setEnabled(meshAtts->GetOpaqueColorSource() == MeshAttributes::OpaqueCustom);

            break;
        case MeshAttributes::ID_smoothingLevel:
            smoothingLevelButtons->blockSignals(true);
            smoothingLevelButtons->button((int)meshAtts->GetSmoothingLevel())->setChecked(true);
            smoothingLevelButtons->blockSignals(false);
            break;
        case MeshAttributes::ID_pointSizeVarEnabled:
            pointControl->blockSignals(true);
            pointControl->SetPointSizeVarChecked(meshAtts->GetPointSizeVarEnabled());
            pointControl->blockSignals(false);
            break;
        case MeshAttributes::ID_pointSizeVar:
            {
            pointControl->blockSignals(true);
            QString temp(meshAtts->GetPointSizeVar().c_str());
            pointControl->SetPointSizeVar(temp);
            pointControl->blockSignals(false);
            }
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
                    opaqueColorLabel->setEnabled(true);
                    opaqueColorButtons->button(0)->setEnabled(true);
                    opaqueColorButtons->button(1)->setEnabled(true);
                    opaqueColor->setEnabled(meshAtts->GetOpaqueColorSource() == MeshAttributes::OpaqueCustom);
                }
                else 
                {
                    opaqueColorLabel->setEnabled(false);
                    opaqueColorButtons->button(0)->setEnabled(false);
                    opaqueColorButtons->button(1)->setEnabled(false);
                    opaqueColor->setEnabled(false);
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
//   Kathleen Biagas, Thu Apr 23 13:13:16 PDT 2015
//   Removed never used outlineOnly and errorTolerance widgets.
//
// ****************************************************************************

void
QvisMeshPlotWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

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
// Method: QvisMeshPlotWindow::opaqueColorClicked
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
// Modifications:
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
// ****************************************************************************

void
QvisMeshPlotWindow::opaqueColorClicked(int  val)
{
    // Only do it if it changed.
    if(val != meshAtts->GetOpaqueColorSource())
    {
        meshAtts->SetOpaqueColorSource(MeshAttributes::OpaqueColor(val));
        opaqueColor->setEnabled(meshAtts->GetOpaqueColorSource() == MeshAttributes::OpaqueCustom);
        Apply();
    }
}


// ****************************************************************************
// Method: QvisMeshPlotWindow::meshColorClicked
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
// Modifications:
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
// ****************************************************************************

void
QvisMeshPlotWindow::meshColorClicked(int val)
{
    // Only do it if it changed.
    if(val != meshAtts->GetMeshColorSource())
    {
        meshAtts->SetMeshColorSource(MeshAttributes::MeshColor(val));
        meshColor->setEnabled(meshAtts->GetMeshColorSource() == MeshAttributes::MeshCustom);
        Apply();
    }
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
    meshAtts->SetPointType((GlyphType) type);
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

