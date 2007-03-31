#include <QvisRenderingWindow.h>
#include <float.h>
#include <limits.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qslider.h>

#include <RenderingAttributes.h>
#include <ViewerProxy.h>
#include <WindowInformation.h>
#include <QvisOpacitySlider.h>

// ****************************************************************************
// Method: QvisRenderingWindow::QvisRenderingWindow
//
// Purpose: 
//   Constructor for the QvisRenderingWindow class.
//
// Arguments:
//   caption : The window caption.
//   shortName : The name shown in the notepad.
//   n         : The notepad area.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:46:40 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisRenderingWindow::QvisRenderingWindow(const char *caption,
    const char *shortName, QvisNotepadArea *n) :
    QvisPostableWindowSimpleObserver(caption, shortName, n)
{
    renderAtts = 0;
    windowInfo = 0;

    objectRepresentation = 0;
    stereoType = 0;
    scalableThreshold = 0;
}

// ****************************************************************************
// Method: QvisRenderingWindow::~QvisRenderingWindow
//
// Purpose: 
//   This is the destructor for the QvisRenderingWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:47:29 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisRenderingWindow::~QvisRenderingWindow()
{
    delete objectRepresentation;
    delete stereoType;
    delete scalableThreshold;

    if(renderAtts)
        renderAtts->Detach(this);

    if(windowInfo)
        windowInfo->Detach(this);
}

// ****************************************************************************
// Method: QvisRenderingWindow::CreateWindowContents
//
// Purpose: 
//   This method creates all of the window's widgets.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:47:57 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Oct 24 13:33:31 PST 2002
//   I made the stereo radio buttons class members.
//   
//   Kathleen Bonnell, Wed Dec  4 18:42:48 PST 2002 
//   Removed antialiasingQuality slider, no longer needed. 
//   
//   Jeremy Meredith, Fri Nov 14 17:47:19 PST 2003
//   Added specular options.
//
// ****************************************************************************

void
QvisRenderingWindow::CreateWindowContents()
{
    //
    // Create the renderer options group.
    //
    QGroupBox *options = new QGroupBox(central, "options");
    options->setTitle("Options");
    topLayout->addWidget(options);

    QVBoxLayout *spacer = new QVBoxLayout(options);
    spacer->addSpacing(10);
    QGridLayout *oLayout = new QGridLayout(spacer, 12, 4);
    oLayout->setSpacing(5);
    oLayout->setMargin(10);

    // Create the antialiasing widgets.
    antialiasingToggle = new QCheckBox("Antialiasing", options,
        "antialiasingToggle");
    connect(antialiasingToggle, SIGNAL(toggled(bool)),
            this, SLOT(antialiasingToggled(bool)));
    oLayout->addMultiCellWidget(antialiasingToggle, 0, 0, 0, 1);

    // Create the surface rep widgets.
    QLabel *drawObjLabel = new QLabel("Draw objects as", options, "drawObjLabel");
    oLayout->addMultiCellWidget(drawObjLabel, 2, 2, 0, 3);
    objectRepresentation = new QButtonGroup(0, "objectRepresentation");
    connect(objectRepresentation, SIGNAL(clicked(int)),
            this, SLOT(objectRepresentationChanged(int)));
    QRadioButton *surfaces = new QRadioButton("Surfaces", options, "surfaces");
    objectRepresentation->insert(surfaces);
    oLayout->addWidget(surfaces, 3, 1);
    QRadioButton *wires = new QRadioButton("Wireframe", options,
        "wires");
    objectRepresentation->insert(wires);
    oLayout->addWidget(wires, 3, 2);
    QRadioButton *points = new QRadioButton("Points", options,
        "points");
    objectRepresentation->insert(points);
    oLayout->addWidget(points, 3, 3);

    // Create the display list toggle.
    dislayListToggle = new QCheckBox("Use display lists", options,
        "dislayListToggle");
    connect(dislayListToggle, SIGNAL(toggled(bool)),
            this, SLOT(displayListToggled(bool)));
    oLayout->addMultiCellWidget(dislayListToggle, 4, 4, 0, 3);

    // Create the stereo widgets.
    stereoToggle = new QCheckBox("Stereo", options,
        "stereoToggle");
    connect(stereoToggle, SIGNAL(toggled(bool)),
            this, SLOT(stereoToggled(bool)));
    oLayout->addMultiCellWidget(stereoToggle, 5, 5, 0, 3);
    stereoType = new QButtonGroup(0, "stereoType");
    connect(stereoType, SIGNAL(clicked(int)),
            this, SLOT(stereoTypeChanged(int)));
    redblue = new QRadioButton("Red/Blue", options, "redblue");
    stereoType->insert(redblue);
    oLayout->addWidget(redblue, 6, 1);
    interlace = new QRadioButton("Interlace", options,
        "interlace");
    stereoType->insert(interlace);
    oLayout->addWidget(interlace, 6, 2);
    crystalEyes = new QRadioButton("Crystal Eyes", options,
        "crystalEyes");
    stereoType->insert(crystalEyes);
    oLayout->addWidget(crystalEyes, 6, 3);

    // Create the scalable rendering widgets.
    QLabel *scalrenLabel = new QLabel("Use scalable rendering", options,"scalrenLabel");
    oLayout->addMultiCellWidget(scalrenLabel, 7, 7, 0, 3);
    scalableThreshold = new QButtonGroup(0, "scalableThreshold");
    connect(scalableThreshold, SIGNAL(clicked(int)),
            this, SLOT(scalableThresholdChanged(int)));
    scalrenAuto = new QRadioButton("Auto", options, "auto");
    scalableThreshold->insert(scalrenAuto);
    oLayout->addWidget(scalrenAuto, 8, 1);
    scalrenAlways = new QRadioButton("Always", options, "always");
    scalableThreshold->insert(scalrenAlways);
    oLayout->addWidget(scalrenAlways, 8, 2);
    scalrenNever = new QRadioButton("Never", options, "never");
    scalableThreshold->insert(scalrenNever);
    oLayout->addWidget(scalrenNever, 8, 3);

    // Create the scalable rendering options
    specularToggle = new QCheckBox("Specular lighting", options,
                                   "specularToggle");
    connect(specularToggle, SIGNAL(toggled(bool)),
            this, SLOT(specularToggled(bool)));
    oLayout->addMultiCellWidget(specularToggle, 9,9, 0,3);

    specularStrengthSlider = new QvisOpacitySlider(0, 100, 10, 60, options,
                                             "specularStrengthSlider", NULL);
    specularStrengthSlider->setTickInterval(25);
    connect(specularStrengthSlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(specularStrengthChanged(int, const void*)));
    specularStrengthLabel = new QLabel(specularStrengthSlider, "Strength",
                                       options, "specularStrengthLabel");
    oLayout->addWidget(specularStrengthLabel, 10,1);
    oLayout->addMultiCellWidget(specularStrengthSlider, 10,10, 2,3);

    specularPowerSlider = new QvisOpacitySlider(0, 1000, 100, 100, options,
                                                "specularPowerSlider", NULL);
    specularPowerSlider->setTickInterval(100);
    connect(specularPowerSlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(specularPowerChanged(int, const void*)));
    specularPowerLabel = new QLabel(specularPowerSlider, "Sharpness",
                                    options, "specularPowerLabel");
    oLayout->addWidget(specularPowerLabel, 11,1);
    oLayout->addMultiCellWidget(specularPowerSlider, 11,11, 2,3);


    //
    // Create the renderer information group.
    //
    QGroupBox *info = new QGroupBox(central, "info");
    info->setTitle("Information");
    topLayout->addWidget(info);

    QVBoxLayout *vLayout = new QVBoxLayout(info);
    vLayout->addSpacing(10);
    vLayout->setMargin(10);

    renderNotifyToggle = new QCheckBox("Query after each render", info,
        "renderNotifyToggle");
    connect(renderNotifyToggle, SIGNAL(toggled(bool)),
            this, SLOT(renderNotifyToggled(bool)));
    vLayout->addWidget(renderNotifyToggle);
    vLayout->addSpacing(5);

    QGridLayout *scalrenLayout = new QGridLayout(vLayout, 2, 4);
    scalrenLayout->setSpacing(5);
    QLabel *scalrenLabel2 = new QLabel("Using Scalable Rendering: ", info, "scalrenLabel2");
    scalrenLayout->addWidget(scalrenLabel2, 0, 0);
    scalrenUsingLabel = new QLabel("N/A", info, "scalrenUsingLabel");
    scalrenLayout->addWidget(scalrenUsingLabel, 0, 1);
    vLayout->addSpacing(5);

    QGridLayout *iLayout = new QGridLayout(vLayout, 2, 4);
    iLayout->setSpacing(5);
    QLabel *fpsLabel2 = new QLabel("Frames per second:", info, "fpsLabel2");
    iLayout->addWidget(fpsLabel2, 0, 0);
    fpsMinLabel = new QLabel("0.", info, "fpsMinLabel");
    iLayout->addWidget(fpsMinLabel, 0, 1);
    fpsAvgLabel = new QLabel("0.", info, "fpsAvgLabel");
    iLayout->addWidget(fpsAvgLabel, 0, 2);
    fpsMaxLabel = new QLabel("0.", info, "fpsMaxLabel");
    iLayout->addWidget(fpsMaxLabel, 0, 3);

    QLabel *ntri = new QLabel("Approximate triangle count:", info, "ntri");
    iLayout->addWidget(ntri, 1, 0);
    approxNumTriangles = new QLabel("0.", info, "approxNumTriangles");
    iLayout->addWidget(approxNumTriangles, 1, 1);
    vLayout->addSpacing(5);

    QGridLayout *eLayout = new QGridLayout(vLayout, 3, 3);
    eLayout->setSpacing(5);
    QLabel *xExtentsLabel = new QLabel("X Extents:", info, "xExtentsLabel");
    eLayout->addWidget(xExtentsLabel, 3, 0);
    extents[0] = new QLabel("0.", info, "extents_0");
    eLayout->addWidget(extents[0], 3, 1);
    extents[1] = new QLabel("0.", info, "extents_1");
    eLayout->addWidget(extents[1], 3, 2);

    QLabel *yExtentsLabel = new QLabel("Y Extents:", info, "yExtentsLabel");
    eLayout->addWidget(yExtentsLabel, 4, 0);
    extents[2] = new QLabel("0.", info, "extents_2");
    eLayout->addWidget(extents[2], 4, 1);
    extents[3] = new QLabel("0.", info, "extents_3");
    eLayout->addWidget(extents[3], 4, 2);

    QLabel *zExtentsLabel = new QLabel("Z Extents:", info, "zExtentsLabel");
    eLayout->addWidget(zExtentsLabel, 5, 0);
    extents[4] = new QLabel("0.", info, "extents_4");
    eLayout->addWidget(extents[4], 5, 1);
    extents[5] = new QLabel("0.", info, "extents_5");
    eLayout->addWidget(extents[5], 5, 2);
}

// ****************************************************************************
// Method: QvisRenderingWindow::UpdateWindow
//
// Purpose: 
//   This method updates the window's widgets when its subjects update.
//
// Arguments:
//   doAll : Whether or not to ignore field selection.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:48:24 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisRenderingWindow::UpdateWindow(bool doAll)
{
    if(SelectedSubject() == renderAtts || doAll)
        UpdateOptions(doAll);
    if(SelectedSubject() == windowInfo || doAll)
        UpdateInformation(doAll);
}

// ****************************************************************************
// Method: QvisRenderingWindow::UpdateOptions
//
// Purpose: 
//   Updates the widgets that control the rendering options.
//
// Arguments:
//   doAll : Whether or not to ignore selection and update all widgets.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 24 11:28:53 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Oct 24 13:35:22 PST 2002
//   I added code to enable/disable the stereo rendering buttons.
//
//   Kathleen Bonnell, Wed Dec  4 18:42:48 PST 2002 
//   Renumber switch cases, to reflect antialisingFrames removed from atts. 
//
//   Jeremy Meredith, Fri Nov 14 17:44:35 PST 2003
//   Added specular options.
//
// ****************************************************************************

void
QvisRenderingWindow::UpdateOptions(bool doAll)
{
    QString tmp;
    int itmp;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < renderAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!renderAtts->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case 0: //antialiasing
            antialiasingToggle->blockSignals(true);
            antialiasingToggle->setChecked(renderAtts->GetAntialiasing());
            antialiasingToggle->blockSignals(false);
            break;
        case 1: //geometryRepresentation
            itmp = (int)renderAtts->GetGeometryRepresentation();
            objectRepresentation->blockSignals(true);
            objectRepresentation->setButton(itmp);
            objectRepresentation->blockSignals(false);
            break;
        case 2: //displayLists
            dislayListToggle->blockSignals(true);
            dislayListToggle->setChecked(renderAtts->GetDisplayLists());
            dislayListToggle->blockSignals(false);
            break;
        case 3: //stereoRendering
            stereoToggle->blockSignals(true);
            stereoToggle->setChecked(renderAtts->GetStereoRendering());
            stereoToggle->blockSignals(false);
            stereoType->setEnabled(renderAtts->GetStereoRendering());
            redblue->setEnabled(renderAtts->GetStereoRendering());
            interlace->setEnabled(renderAtts->GetStereoRendering());
            crystalEyes->setEnabled(renderAtts->GetStereoRendering());
            break;
        case 4: //stereoType
            stereoType->blockSignals(true);
            stereoType->setButton((int)renderAtts->GetStereoType());
            stereoType->blockSignals(false);
            break;
        case 5: //notifyForEachRender
            renderNotifyToggle->blockSignals(true);
            renderNotifyToggle->setChecked(renderAtts->GetNotifyForEachRender());
            renderNotifyToggle->blockSignals(false);
            break;
        case 6: //scalableRendering
            break;
        case 7: //scalableThreshold
            itmp = (int)renderAtts->GetScalableThreshold();
            scalableThreshold->blockSignals(true);
            if (itmp == 0)
               scalableThreshold->setButton(1);
            else if (itmp == INT_MAX)
               scalableThreshold->setButton(2);
            else
               scalableThreshold->setButton(0);
            scalableThreshold->blockSignals(false);
            break;
        case 8: //specularFlag
            specularToggle->blockSignals(true);
            specularToggle->setChecked(renderAtts->GetSpecularFlag());
            specularToggle->blockSignals(false);
            specularStrengthSlider->setEnabled(renderAtts->GetSpecularFlag());
            specularPowerSlider->setEnabled(renderAtts->GetSpecularFlag());
            specularStrengthLabel->setEnabled(renderAtts->GetSpecularFlag());
            specularPowerLabel->setEnabled(renderAtts->GetSpecularFlag());
            break;
        case 9: //specularCoeff
            specularStrengthSlider->blockSignals(true);
            specularStrengthSlider->setValue(int(renderAtts->GetSpecularCoeff()*100.));
            specularStrengthSlider->blockSignals(false);
            break;
        case 10: //specularPower
            specularPowerSlider->blockSignals(true);
            specularPowerSlider->setValue(int(renderAtts->GetSpecularPower()*10.));
            specularPowerSlider->blockSignals(false);
            break;
        case 11: //specularColor
            // Not user-modifiable at this time
            break;
        }
    }
}

// ****************************************************************************
// Method: QvisRenderingWindow::UpdateInformation
//
// Purpose: 
//   Updates the information labels with statistics from the viewer.
//
// Arguments:
//   doAll : Whether or not to ignore selection and update all widgets.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 24 11:29:35 PDT 2002
//
// Modifications:
//   Eric Brugger, Fri Apr 18 11:52:33 PDT 2003
//   I removed auto center view.
//   
// ****************************************************************************

void
QvisRenderingWindow::UpdateInformation(bool doAll)
{
    QString tmp;
    float fps;
    int j;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < windowInfo->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!windowInfo->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case 0: //windowMode
        case 1: //boundingBoxNavigate
        case 2: //spin
        case 3: //perspective
        case 4: //lockView
        case 5: //viewExtentsType
        case 6: //viewDimension
            break;
        case 7: //lastRenderMin
            // Determine the fps.
            if(windowInfo->GetLastRenderMin() > 0. &&
               windowInfo->GetLastRenderMin() < 1.e05)
                fps = 1. / windowInfo->GetLastRenderMin();
            else
                fps = 0.;
            tmp.sprintf("%1.3g", fps);
            fpsMaxLabel->setText(tmp);
            break;
        case 8: //lastRenderAvg
            // Determine the fps.
            if(windowInfo->GetLastRenderAvg() > 0.)
                fps = 1. / windowInfo->GetLastRenderAvg();
            else
                fps = 0.;
            tmp.sprintf("%1.3g", fps);
            fpsAvgLabel->setText(tmp);
            break;
        case 9: //lastRenderMax
            // Determine the fps.
            if(windowInfo->GetLastRenderMax() > 0.)
                fps = 1. / windowInfo->GetLastRenderMax();
            else
                fps = 0.;
            tmp.sprintf("%1.3g", fps);
            fpsMinLabel->setText(tmp);
            break;
        case 10: //numTriangles
            tmp.sprintf("%d", windowInfo->GetNumTriangles());
            approxNumTriangles->setText(tmp);
            break;
        case 11: //extents
            for(j = 0; j < 6; ++j)
            {
                double d = windowInfo->GetExtents()[j];
                if(d == DBL_MAX || d == -DBL_MAX)
                    tmp = "not set";
                else
                    tmp.sprintf("%g", windowInfo->GetExtents()[j]);
                extents[j]->setText(tmp);
            }
            break;
        case 12: // lockTools
        case 13: // lockTime
        case 14: // cameraViewMode
        case 15: // fullFrame
            break;
        case 16: // usingScalableRendering
            scalrenUsingLabel->setText(windowInfo->GetUsingScalableRendering() ? "yes" : "no");
            break;
        }            
    }
}

// ****************************************************************************
// Method: QvisRenderingWindow::Apply
//
// Purpose: 
//   Tells the viewer to apply the rendering attributes.
//
// Arguments:
//   ignore : A flag that causes us to unconditionally apply the
//            rendering attributes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:49:19 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisRenderingWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        renderAtts->Notify();
        viewer->SetRenderingAttributes();
    }
    else
        renderAtts->Notify();
}

// ****************************************************************************
// Method: QvisRenderingWindow::SubjectRemoved
//
// Purpose: 
//   This method makes sure that we don't Detach from the subject if it is
//   destroyed first.
//
// Arguments:
//   TheRemovedSubject : The subject being deleted.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:50:04 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisRenderingWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(TheRemovedSubject == renderAtts)
        renderAtts = 0;
    else if(TheRemovedSubject == windowInfo)
        windowInfo = 0;
}

// ****************************************************************************
// Method: QvisRenderingWindow::ConnectRenderingAttributes
//
// Purpose: 
//   Makes this window observe the rendering attributes.
//
// Arguments:
//   w : The rendering attributes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:51:11 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisRenderingWindow::ConnectRenderingAttributes(RenderingAttributes *w)
{
    renderAtts = w;
    renderAtts->Attach(this);
}

// ****************************************************************************
// Method: QvisRenderingWindow::ConnectWindowInformation
//
// Purpose: 
//   Makes this window observe the window information.
//
// Arguments:
//   w : The window information.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:51:11 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisRenderingWindow::ConnectWindowInformation(WindowInformation *w)
{
    windowInfo = w;
    windowInfo->Attach(this);
}

//
// Qt slot functions
//

// ****************************************************************************
// Method: QvisRenderingWindow::apply
//
// Purpose: 
//   This Qt slot function is called by clicking the apply button.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:51:45 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisRenderingWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisRenderingWindow::antialiasingToggled
//
// Purpose: 
//   This Qt slot function is called when the antialiasing checkbox is clicked.
//
// Arguments:
//   val : The new AA value.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:52:07 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisRenderingWindow::antialiasingToggled(bool val)
{
    renderAtts->SetAntialiasing(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::objectRepresentationChanged
//
// Purpose: 
//   This Qt slot function is called when we change surface representations.
//
// Arguments:
//   val : The new surface representation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:53:28 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisRenderingWindow::objectRepresentationChanged(int val)
{
    renderAtts->SetGeometryRepresentation(
        (RenderingAttributes::GeometryRepresentation)val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::displayListToggled
//
// Purpose: 
//   This Qt slot function is called when we click the display list toggle.
//
// Arguments:
//   val : The new display list flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:54:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisRenderingWindow::displayListToggled(bool val)
{
    renderAtts->SetDisplayLists(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::stereoToggled
//
// Purpose: 
//   This Qt slot function is called when we change turn stereo on/off.
//
// Arguments:
//   val : The new stereo enabled mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:54:52 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisRenderingWindow::stereoToggled(bool val)
{
    renderAtts->SetStereoRendering(val);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::stereoTypeChanged
//
// Purpose: 
//   This Qt slot function is called when the stereo type changes.
//
// Arguments:
//   val : The new stereo type.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:55:32 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisRenderingWindow::stereoTypeChanged(int val)
{
    renderAtts->SetStereoType((RenderingAttributes::StereoTypes)val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::renderNotifyToggled
//
// Purpose: 
//   This Qt slot function is called when the "Query after render" toggle
//   button is clicked.
//
// Arguments:
//   val : The new query mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:56:12 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisRenderingWindow::renderNotifyToggled(bool val)
{
    renderAtts->SetNotifyForEachRender(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::scalableThresholdChanged
//
// Purpose: 
//   This Qt slot function is called when the scalable threshold changes.
//
// Arguments:
//   val : The new scalable threshold flag.
//         0 = Auto, 1 = Always, 2 = Never
//
// Programmer: Mark C. Miller 
// Creation:   Wed Nov 20 17:09:59 PST 2002 
//
// Modifications:
//   
// ****************************************************************************

void
QvisRenderingWindow::scalableThresholdChanged(int val)
{
    if (val == 0)
       renderAtts->SetScalableThreshold(
          RenderingAttributes::DEFAULT_SCALABLE_THRESHOLD);
    else if (val == 1)
       renderAtts->SetScalableThreshold(0);
    else
       renderAtts->SetScalableThreshold(INT_MAX);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::specularToggled
//
//  Purpose:
//    Callback for the specular toggle button
//
//  Arguments:
//    val        true to enable specular
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 14, 2003
//
// ****************************************************************************

void
QvisRenderingWindow::specularToggled(bool val)
{
    renderAtts->SetSpecularFlag(val);
    specularStrengthSlider->setEnabled(renderAtts->GetSpecularFlag());
    specularPowerSlider->setEnabled(renderAtts->GetSpecularFlag());
    specularStrengthLabel->setEnabled(renderAtts->GetSpecularFlag());
    specularPowerLabel->setEnabled(renderAtts->GetSpecularFlag());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::specularStrengthChanged
//
//  Purpose:
//    Callback for the specular coefficient slider
//
//  Arguments:
//    val        the new coefficient
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 14, 2003
//
// ****************************************************************************

void
QvisRenderingWindow::specularStrengthChanged(int val, const void*)
{
    renderAtts->SetSpecularCoeff(float(val)/100.);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::specularPowerChanged
//
//  Purpose:
//    Callback for the specular exponent slider.
//
//  Arguments:
//    val        the new value
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 14, 2003
//
// ****************************************************************************

void
QvisRenderingWindow::specularPowerChanged(int val, const void*)
{
    renderAtts->SetSpecularPower(float(val)/10.);
    SetUpdate(false);
    Apply();
}
