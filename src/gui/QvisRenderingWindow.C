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
#include <qspinbox.h>

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
//   Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//   Changed name of scalableThreshold to scalrenActivationMode
//   
//   Hank Childs, Sun May  9 15:54:29 PDT 2004
//   Initialize dlMode.
//
// ****************************************************************************

QvisRenderingWindow::QvisRenderingWindow(const char *caption,
    const char *shortName, QvisNotepadArea *n) :
    QvisPostableWindowSimpleObserver(caption, shortName, n, ApplyButton)
{
    renderAtts = 0;
    windowInfo = 0;

    objectRepresentation = 0;
    dlMode = 0;
    stereoType = 0;
    scalrenActivationMode = 0;
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
//   Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//   Changed name of scalableThreshold to scalrenActivationMode
//
//   Hank Childs, Sun May  9 15:54:29 PDT 2004
//   Delete dlMode.
//
// ****************************************************************************

QvisRenderingWindow::~QvisRenderingWindow()
{
    delete objectRepresentation;
    delete dlMode;
    delete stereoType;
    delete scalrenActivationMode;

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
//   Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//   Added scalable threshold spinbox
//
//   Hank Childs, Sun May  9 15:54:29 PDT 2004
//   Add support for multiple display list modes.
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
    QGridLayout *oLayout = new QGridLayout(spacer, 14, 4);
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

    // Create the display list widgets.
    QLabel *displayListLabel = new QLabel("Use display lists", options, "displayListLabel");
    oLayout->addMultiCellWidget(displayListLabel, 4, 4, 0, 3);
    dlMode = new QButtonGroup(0, "displayList");
    connect(dlMode, SIGNAL(clicked(int)),
            this, SLOT(displayListModeChanged(int)));
    QRadioButton *dl_auto = new QRadioButton("Auto", options, "dl_auto");
    dlMode->insert(dl_auto);
    oLayout->addWidget(dl_auto, 5, 1);
    QRadioButton *dl_always = new QRadioButton("Always", options,
        "dl_always");
    dlMode->insert(dl_always);
    oLayout->addWidget(dl_always, 5, 2);
    QRadioButton *dl_never = new QRadioButton("Never", options,
        "dl_never");
    dlMode->insert(dl_never);
    oLayout->addWidget(dl_never, 5, 3);

    // Create the stereo widgets.
    stereoToggle = new QCheckBox("Stereo", options,
        "stereoToggle");
    connect(stereoToggle, SIGNAL(toggled(bool)),
            this, SLOT(stereoToggled(bool)));
    oLayout->addMultiCellWidget(stereoToggle, 6, 6, 0, 3);
    stereoType = new QButtonGroup(0, "stereoType");
    connect(stereoType, SIGNAL(clicked(int)),
            this, SLOT(stereoTypeChanged(int)));
    redblue = new QRadioButton("Red/Blue", options, "redblue");
    stereoType->insert(redblue);
    oLayout->addWidget(redblue, 7, 1);
    interlace = new QRadioButton("Interlace", options,
        "interlace");
    stereoType->insert(interlace);
    oLayout->addWidget(interlace, 7, 2);
    crystalEyes = new QRadioButton("Crystal Eyes", options,
        "crystalEyes");
    stereoType->insert(crystalEyes);
    oLayout->addWidget(crystalEyes, 7, 3);

    // Create the scalable rendering widgets.
    QLabel *scalrenLabel = new QLabel("Use scalable rendering", options,"scalrenLabel");
    oLayout->addMultiCellWidget(scalrenLabel, 8, 8, 0, 3);
    scalrenActivationMode = new QButtonGroup(0, "scalrenActivationMode");
    connect(scalrenActivationMode, SIGNAL(clicked(int)),
            this, SLOT(scalrenActivationModeChanged(int)));
    scalrenAuto = new QRadioButton("Auto", options, "auto");
    scalrenActivationMode->insert(scalrenAuto);
    oLayout->addWidget(scalrenAuto, 9, 1);
    scalrenAlways = new QRadioButton("Always", options, "always");
    scalrenActivationMode->insert(scalrenAlways);
    oLayout->addWidget(scalrenAlways, 9, 2);
    scalrenNever = new QRadioButton("Never", options, "never");
    scalrenActivationMode->insert(scalrenNever);
    oLayout->addWidget(scalrenNever, 9, 3);

    // Create the polygon count spin box for scalable rendering threshold
    scalrenGeometryLabel =  new QLabel("When polygon count exceeds", options, "scalrenGeometryLabel");
    oLayout->addMultiCellWidget(scalrenGeometryLabel, 10, 10, 1, 2);
    scalrenAutoThreshold = new QSpinBox(0, 10000, 500, options, "scalrenAutoThreshold");
    scalrenAutoThreshold->setValue(RenderingAttributes::DEFAULT_SCALABLE_THRESHOLD);
    scalrenAutoThresholdChanged(RenderingAttributes::DEFAULT_SCALABLE_THRESHOLD);
    connect(scalrenAutoThreshold, SIGNAL(valueChanged(int)),
            this, SLOT(scalrenAutoThresholdChanged(int)));
    oLayout->addWidget(scalrenAutoThreshold, 10, 3);

    // Create the specular lighting options
    specularToggle = new QCheckBox("Specular lighting", options,
                                   "specularToggle");
    connect(specularToggle, SIGNAL(toggled(bool)),
            this, SLOT(specularToggled(bool)));
    oLayout->addMultiCellWidget(specularToggle, 11, 11, 0,3);

    specularStrengthSlider = new QvisOpacitySlider(0, 100, 10, 60, options,
                                             "specularStrengthSlider", NULL);
    specularStrengthSlider->setTickInterval(25);
    connect(specularStrengthSlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(specularStrengthChanged(int, const void*)));
    specularStrengthLabel = new QLabel(specularStrengthSlider, "Strength",
                                       options, "specularStrengthLabel");
    oLayout->addWidget(specularStrengthLabel, 12,1);
    oLayout->addMultiCellWidget(specularStrengthSlider, 12,12, 2,3);

    specularPowerSlider = new QvisOpacitySlider(0, 1000, 100, 100, options,
                                                "specularPowerSlider", NULL);
    specularPowerSlider->setTickInterval(100);
    connect(specularPowerSlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(specularPowerChanged(int, const void*)));
    specularPowerLabel = new QLabel(specularPowerSlider, "Sharpness",
                                    options, "specularPowerLabel");
    oLayout->addWidget(specularPowerLabel, 13,1);
    oLayout->addMultiCellWidget(specularPowerSlider, 13,13, 2,3);


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
//   Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//   Changed name of scalableThreshld to scalrenActivationMode
//
//   Hank Childs, Sun May  9 15:54:29 PDT 2004
//   Add support for multiple display list modes.
//
// ****************************************************************************

void
QvisRenderingWindow::UpdateOptions(bool doAll)
{
    QString tmp;
    int itmp, itmp2;

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
            itmp = (int) renderAtts->GetDisplayListMode();
            if (itmp == 2) // Auto for atts's enum type order
                itmp2 = 0; // Order of Auto in window
            else if (itmp == 1) // Always for atts' enum type order
                itmp2 = 1; // Order of Always in window.
            else           // Never for atts' enum type order
                itmp2 = 2; // Order of Never in window.
            dlMode->blockSignals(true);
            dlMode->setButton(itmp2);
            dlMode->blockSignals(false);
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
        case 7: //scalrenActivationMode
            itmp = (int)renderAtts->GetScalableThreshold();
            scalrenActivationMode->blockSignals(true);
            if (itmp == 0)
               scalrenActivationMode->setButton(1);
            else if (itmp == INT_MAX)
               scalrenActivationMode->setButton(2);
            else
               scalrenActivationMode->setButton(0);
            scalrenActivationMode->blockSignals(false);
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
// Method: QvisRenderingWindow::displayListModeChanged
//
// Purpose: 
//   This Qt slot function is called when we change the display list mode.
//
// Arguments:
//   mode : The new display list mode.
//
// Programmer: Hank Childs
// Creation:   May 9, 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisRenderingWindow::displayListModeChanged(int mode)
{
    int itmp = 0;
    if (mode == 0)      // Auto in Window
        itmp = 2;       // Auto for atts' enum type
    else if (mode == 1) // Always in window
        itmp = 1;       // Always for atts' enum type
    else                // Never in window.
        itmp = 0;       // Never for atts' enum type

    renderAtts->SetDisplayListMode((RenderingAttributes::DisplayListMode)itmp);
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
// Method: QvisRenderingWindow::scalrenActivationModeChanged
//
// Purpose: 
//   This Qt slot function is called when the scalable activation mode changes.
//
// Arguments:
//   val : The new scalable activation mode flag.
//         0 = Auto, 1 = Always, 2 = Never
//
// Programmer: Mark C. Miller 
// Creation:   Wed Nov 20 17:09:59 PST 2002 
//
// Modifications:
//
//   Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//   Added scalrenAutoThreshold spinbox and geometry label
//   
// ****************************************************************************

void
QvisRenderingWindow::scalrenActivationModeChanged(int val)
{
    if (val == 0)
    {
        scalrenAutoThreshold->setEnabled(1);
        scalrenGeometryLabel->setEnabled(1);
        scalrenAutoThresholdChanged(scalrenAutoThreshold->value());
    }
    else if (val == 1)
    {
        scalrenAutoThreshold->setEnabled(0);
        scalrenGeometryLabel->setEnabled(0);
        renderAtts->SetScalableThreshold(0);
    }
    else
    {
        scalrenAutoThreshold->setEnabled(0);
        scalrenGeometryLabel->setEnabled(0);
        renderAtts->SetScalableThreshold(INT_MAX);
    }
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::scalrenAutoThresholdChanged
//
// Purpose: 
//   This Qt slot function is called when the scalable rendering automatic
//   polygon count threshold changes.
//
// Arguments:
//   val : The new polygon count threshold.
//
// Programmer: Mark C. Miller 
// Creation:   Wed Apr 21 22:42:57 PDT 2004 
//
// Modifications:
//   
// ****************************************************************************
void
QvisRenderingWindow::scalrenAutoThresholdChanged(int val)
{
    // determine the actual value from the given value and the current suffix
    int actualVal;
    if (scalrenAutoThreshold->suffix() == " GPolys")
        actualVal = val * (int) 1e9;
    else if (scalrenAutoThreshold->suffix() == " MPolys")
        actualVal = val * (int) 1e6;
    else if (scalrenAutoThreshold->suffix() == " KPolys")
        actualVal = val * (int) 1e3;
    else if (scalrenAutoThreshold->suffix() == "  Polys")
        actualVal = val;
    else
        actualVal = val;

    // deal with stepping backwards
    int step = scalrenAutoThreshold->lineStep();
    if ((val == 500) && (step == 500) && (scalrenAutoThreshold->suffix() == "  Polys"))
        actualVal = 950;
    else if ((val == 5) && (step == 5) && (scalrenAutoThreshold->suffix() == " KPolys"))
        actualVal = 9500;
    else if ((val == 50) && (step == 50) && (scalrenAutoThreshold->suffix() == " KPolys"))
        actualVal = 95000;
    else if ((val == 500) && (step == 500) && (scalrenAutoThreshold->suffix() == " KPolys"))
        actualVal = 950000;
    else if ((val == 5) && (step == 5) && (scalrenAutoThreshold->suffix() == " MPolys"))
        actualVal = 9500000;
    else if ((val == 50) && (step == 50) && (scalrenAutoThreshold->suffix() == " MPolys"))
        actualVal = 95000000;
    else if ((val == 0) && (step == 1) && (scalrenAutoThreshold->suffix() == " GPolys"))
        actualVal = 950000000;

    // compute the step size and suffix for the displayed value in the GUI
    QString suffix;
    if      (actualVal < 1e3) { step = 50;  suffix = "  Polys"; }
    else if (actualVal < 1e4) { step = 500; suffix = "  Polys"; }
    else if (actualVal < 1e5) { step = 5;   suffix = " KPolys"; }
    else if (actualVal < 1e6) { step = 50;  suffix = " KPolys"; }
    else if (actualVal < 1e7) { step = 500; suffix = " KPolys"; }
    else if (actualVal < 1e8) { step = 5;   suffix = " MPolys"; }
    else if (actualVal < 1e9) { step = 50;  suffix = " MPolys"; }
    else                      { step = 1;   suffix = " GPolys"; }

    // compute the divisor for the displayed value in the GUI
    int div;
    if      (suffix == " KPolys") div = (int) 1e3;
    else if (suffix == " MPolys") div = (int) 1e6; 
    else if (suffix == " GPolys") div = (int) 1e9;
    else                          div = 1;

    // set the new GUI value
    scalrenAutoThreshold->setLineStep(step);
    scalrenAutoThreshold->setSuffix(suffix);
    scalrenAutoThreshold->setValue(actualVal / div);
    renderAtts->SetScalableThreshold(actualVal);

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
