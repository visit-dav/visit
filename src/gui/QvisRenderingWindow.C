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

#include <QvisRenderingWindow.h>
#include <float.h>
#include <limits.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QLineEdit>
#include <QTabWidget>

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
//   Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//   Added compression controls
//
//   Brad Whitlock, Wed Apr  9 11:52:02 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisRenderingWindow::QvisRenderingWindow(const QString &caption,
    const QString &shortName, QvisNotepadArea *n) :
    QvisPostableWindowSimpleObserver(caption, shortName, n, ApplyButton)
{
    renderAtts = 0;
    windowInfo = 0;

    objectRepresentation = 0;
    dlMode = 0;
    stereoType = 0;
    scalrenActivationMode = 0;
    scalrenCompressMode = 0;
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
//   Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//   Added compression controls
//
// ****************************************************************************

QvisRenderingWindow::~QvisRenderingWindow()
{
    if(renderAtts)
        renderAtts->Detach(this);

    if(windowInfo)
        windowInfo->Detach(this);
}

// ****************************************************************************
// Method: QvisRenderingWindow::CreateBasicPage
//
// Purpose: 
//   Creates the widgets on the basic page.
//
// Arguments:
//
// Returns:    The basic page widget.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 12:10:45 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisRenderingWindow::CreateBasicPage()
{
    int row = 0;
    QWidget *basicOptions = new QWidget(central);
    QGridLayout *basicLayout = new QGridLayout(basicOptions);
    basicLayout->setSpacing(5);
    basicLayout->setMargin(10);

    // Create the antialiasing widgets.
    antialiasingToggle = new QCheckBox(tr("Antialiasing"), basicOptions);
    connect(antialiasingToggle, SIGNAL(toggled(bool)),
            this, SLOT(antialiasingToggled(bool)));
    basicLayout->addWidget(antialiasingToggle, row, 0, 1, 3);
    row++;

    // Create the surface rep widgets.
    QLabel *drawObjLabel = new QLabel(tr("Draw objects as"), basicOptions);
    basicLayout->addWidget(drawObjLabel, row, 0, 1, 3);
    objectRepresentation = new QButtonGroup(basicOptions);
    connect(objectRepresentation, SIGNAL(buttonClicked(int)),
            this, SLOT(objectRepresentationChanged(int)));
    row++;

    QRadioButton *surfaces = new QRadioButton(tr("Surfaces"), basicOptions);
    objectRepresentation->addButton(surfaces, 0);
    basicLayout->addWidget(surfaces, row, 1);
    QRadioButton *wires = new QRadioButton(tr("Wireframe"), basicOptions);
    objectRepresentation->addButton(wires, 1);
    basicLayout->addWidget(wires, row, 2);
    QRadioButton *points = new QRadioButton(tr("Points"), basicOptions);
    objectRepresentation->addButton(points, 2);
    basicLayout->addWidget(points, row, 3);
    row++;

    // Create the display list widgets.
    QLabel *displayListLabel = new QLabel(tr("Use display lists"), basicOptions);
    basicLayout->addWidget(displayListLabel, row, 0, 1, 3);
    dlMode = new QButtonGroup(basicOptions);
    connect(dlMode, SIGNAL(buttonClicked(int)),
            this, SLOT(displayListModeChanged(int)));
    row++;

    QRadioButton *dl_auto = new QRadioButton(tr("Auto"), basicOptions);
    dlMode->addButton(dl_auto, 0);
    basicLayout->addWidget(dl_auto, row, 1);
    QRadioButton *dl_always = new QRadioButton(tr("Always"), basicOptions);
    dlMode->addButton(dl_always, 1);
    basicLayout->addWidget(dl_always, row, 2);
    QRadioButton *dl_never = new QRadioButton(tr("Never"), basicOptions);
    dlMode->addButton(dl_never, 2);
    basicLayout->addWidget(dl_never, row, 3);
    row++;

    // Create the stereo widgets.
    stereoToggle = new QCheckBox(tr("Stereo"), basicOptions);
    connect(stereoToggle, SIGNAL(toggled(bool)),
            this, SLOT(stereoToggled(bool)));
    basicLayout->addWidget(stereoToggle, row, 0, 1, 3);
    row++;

    stereoType = new QButtonGroup(basicOptions);
    connect(stereoType, SIGNAL(buttonClicked(int)),
            this, SLOT(stereoTypeChanged(int)));
    redblue = new QRadioButton(tr("Red/Blue"), basicOptions);
    stereoType->addButton(redblue, 0);
    basicLayout->addWidget(redblue, row, 1);
    interlace = new QRadioButton(tr("Interlace"), basicOptions);
    stereoType->addButton(interlace, 1);
    basicLayout->addWidget(interlace, row, 2);
    row++;
    crystalEyes = new QRadioButton(tr("Crystal Eyes"), basicOptions);
    stereoType->addButton(crystalEyes, 2);
    basicLayout->addWidget(crystalEyes, row, 1);
    redgreen = new QRadioButton(tr("Red/Green"), basicOptions);
    stereoType->addButton(redgreen, 3);
    basicLayout->addWidget(redgreen, row, 2);
    row++;

    // Create the specular lighting options
    specularToggle = new QCheckBox(tr("Specular lighting"), basicOptions);
    connect(specularToggle, SIGNAL(toggled(bool)),
            this, SLOT(specularToggled(bool)));
    basicLayout->addWidget(specularToggle, row, 0, 1, 3);
    row++;

    specularStrengthSlider = new QvisOpacitySlider(0, 100, 10, 60, basicOptions);
    specularStrengthSlider->setTickInterval(25);
    connect(specularStrengthSlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(specularStrengthChanged(int, const void*)));
    specularStrengthLabel = new QLabel(tr("Strength"), basicOptions);
    specularStrengthLabel->setBuddy(specularStrengthSlider);
    basicLayout->addWidget(specularStrengthLabel, row, 1);
    basicLayout->addWidget(specularStrengthSlider, row, 2, 1, 2);
    row++;

    specularPowerSlider = new QvisOpacitySlider(0, 1000, 100, 100, basicOptions);
    specularPowerSlider->setTickInterval(100);
    connect(specularPowerSlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(specularPowerChanged(int, const void*)));
    specularPowerLabel = new QLabel(tr("Sharpness"), basicOptions);
    specularPowerLabel->setBuddy(specularPowerSlider);
    basicLayout->addWidget(specularPowerLabel, row,1);
    basicLayout->addWidget(specularPowerSlider, row, 2, 1, 2);
    row++;

    return basicOptions;
}

// ****************************************************************************
// Method: QvisRenderingWindow::CreateAdvancedPage
//
// Purpose: 
//   Creates the advanced page widgets
//
// Arguments:
//
// Returns:    
//
// Note:       Moved from CreateWindowContents.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 13:18:25 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisRenderingWindow::CreateAdvancedPage()
{
    int row = 0;

    QWidget *advancedOptions = new QWidget(central);
    QGridLayout *advLayout = new QGridLayout(advancedOptions);
    advLayout->setSpacing(5);
    advLayout->setMargin(10);

    // Create the scalable rendering widgets.
    QLabel *scalrenLabel = new QLabel(tr("Use scalable rendering"), advancedOptions);
    advLayout->addWidget(scalrenLabel, row, 0, 1, 3);
    scalrenActivationMode = new QButtonGroup(advancedOptions);
    connect(scalrenActivationMode, SIGNAL(buttonClicked(int)),
            this, SLOT(scalrenActivationModeChanged(int)));
    row++;

    scalrenAuto = new QRadioButton(tr("Auto"), advancedOptions);
    scalrenActivationMode->addButton(scalrenAuto, 0);
    advLayout->addWidget(scalrenAuto, row, 1);
    scalrenAlways = new QRadioButton(tr("Always"), advancedOptions);
    scalrenActivationMode->addButton(scalrenAlways, 1);
    advLayout->addWidget(scalrenAlways, row, 2);
    scalrenNever = new QRadioButton(tr("Never"), advancedOptions);
    scalrenActivationMode->addButton(scalrenNever, 2);
    advLayout->addWidget(scalrenNever, row, 3);
    row++;

    // Create the polygon count spin box for scalable rendering threshold
    scalrenGeometryLabel =  new QLabel(tr("When polygon count exceeds"), advancedOptions);
    advLayout->addWidget(scalrenGeometryLabel, row, 1, 1, 2);
    scalrenAutoThreshold = new QSpinBox(advancedOptions);
    scalrenAutoThreshold->setMinimum(0);
    scalrenAutoThreshold->setMaximum(10000);
    scalrenAutoThreshold->setValue(RenderingAttributes::DEFAULT_SCALABLE_AUTO_THRESHOLD);
    scalrenAutoThresholdChanged(RenderingAttributes::DEFAULT_SCALABLE_AUTO_THRESHOLD);
    connect(scalrenAutoThreshold, SIGNAL(valueChanged(int)),
            this, SLOT(scalrenAutoThresholdChanged(int)));
    advLayout->addWidget(scalrenAutoThreshold, row, 3);
    row++;

    // Create the compress mode widgets.
    scalrenCompressLabel = new QLabel(tr("Compress images (geometry too) from engine"),
                                      advancedOptions);
    advLayout->addWidget(scalrenCompressLabel, row, 0, 1, 3);
    scalrenCompressMode = new QButtonGroup(advancedOptions);
    connect(scalrenCompressMode, SIGNAL(buttonClicked(int)),
            this, SLOT(scalrenCompressModeChanged(int)));
    row++;

    QRadioButton *cmp_auto = new QRadioButton(tr("Auto"), advancedOptions);
    cmp_auto->setEnabled(0); // disabled until implemented
    scalrenCompressMode->addButton(cmp_auto, 0);
    advLayout->addWidget(cmp_auto, row, 1);
    QRadioButton *cmp_always = new QRadioButton(tr("Always"), advancedOptions);
    scalrenCompressMode->addButton(cmp_always, 1);
    advLayout->addWidget(cmp_always, row, 2);
    QRadioButton *cmp_never = new QRadioButton(tr("Never"), advancedOptions);
    scalrenCompressMode->addButton(cmp_never, 2);
    advLayout->addWidget(cmp_never, row, 3);
    row++;

    // Create the shadow lighting options
    shadowToggle = new QCheckBox(tr("Shadows (scalable rendering only)"), advancedOptions);
    connect(shadowToggle, SIGNAL(toggled(bool)),
            this, SLOT(shadowToggled(bool)));
    advLayout->addWidget(shadowToggle, row, 0, 1, 3);
    row++;

    shadowStrengthSlider = new QvisOpacitySlider(0, 100, 10, 60, advancedOptions);
    shadowStrengthSlider->setTickInterval(25);
    connect(shadowStrengthSlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(shadowStrengthChanged(int, const void*)));
    shadowStrengthLabel = new QLabel(tr("Strength"), advancedOptions);
    shadowStrengthLabel->setBuddy(shadowStrengthSlider);
    advLayout->addWidget(shadowStrengthLabel, row,1);
    advLayout->addWidget(shadowStrengthSlider, row, 2, 1, 2);
    row++;

    // Create the depth cueing options
    depthCueingToggle = new QCheckBox(tr("Depth Cueing (scalable rendering only)"), advancedOptions);
    connect(depthCueingToggle, SIGNAL(toggled(bool)),
            this, SLOT(depthCueingToggled(bool)));
    advLayout->addWidget(depthCueingToggle, row, 0, 1, 3);
    row++;

    depthCueingStartLabel = new QLabel(tr("Start point"));
    advLayout->addWidget(depthCueingStartLabel, row, 1);
    depthCueingStartEdit = new QLineEdit(advancedOptions);
    advLayout->addWidget(depthCueingStartEdit, row, 2, 1, 2);
    connect(depthCueingStartEdit, SIGNAL(returnPressed()),
            this, SLOT(depthCueingStartChanged()));
    row++;

    depthCueingEndLabel = new QLabel(tr("End point"));
    advLayout->addWidget(depthCueingEndLabel, row, 1);
    depthCueingEndEdit = new QLineEdit(advancedOptions);
    advLayout->addWidget(depthCueingEndEdit, row, 2, 1, 2);
    connect(depthCueingEndEdit, SIGNAL(returnPressed()),
            this, SLOT(depthCueingEndChanged()));
    row++;
    
    // Create color texturing options.
    colorTexturingToggle = new QCheckBox(tr("Apply color using textures"), advancedOptions);
    connect(colorTexturingToggle, SIGNAL(toggled(bool)),
            this, SLOT(colorTexturingToggled(bool)));
    advLayout->addWidget(colorTexturingToggle, row, 0, 1, 3);
    row++;

    return advancedOptions;
}

// ****************************************************************************
// Method: QvisRenderingWindow::CreateInformationPage
//
// Purpose: 
//   Creates the information page.
//
// Arguments:
//
// Returns:    The information page.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 13:18:51 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisRenderingWindow::CreateInformationPage()
{
    QWidget *info = new QWidget(central);
    QVBoxLayout *vLayout = new QVBoxLayout(info);
    vLayout->addSpacing(10);
    vLayout->setMargin(10);

    renderNotifyToggle = new QCheckBox(tr("Query after each render"), info);
    connect(renderNotifyToggle, SIGNAL(toggled(bool)),
            this, SLOT(renderNotifyToggled(bool)));
    vLayout->addWidget(renderNotifyToggle);
    vLayout->addSpacing(5);

    QGridLayout *iLayout = new QGridLayout(0);
    iLayout->setSpacing(5);
    vLayout->addLayout(iLayout);
    QLabel *scalrenLabel2 = new QLabel(tr("Using Scalable Rendering: "), info);
    iLayout->addWidget(scalrenLabel2, 0, 0);
    scalrenUsingLabel = new QLabel(tr("N/A"), info);
    iLayout->addWidget(scalrenUsingLabel, 0, 1, 1, 3);
    vLayout->addSpacing(5);

    fpsLabel = new QLabel(tr("Frames per second:"), info);
    iLayout->addWidget(fpsLabel, 1, 0);
    fpsMinLabel = new QLabel("0.", info);
    iLayout->addWidget(fpsMinLabel, 1, 1);
    fpsAvgLabel = new QLabel("0.", info);
    iLayout->addWidget(fpsAvgLabel, 1, 2);
    fpsMaxLabel = new QLabel("0.", info);
    iLayout->addWidget(fpsMaxLabel, 1, 3);

    QLabel *ntri = new QLabel(tr("Approximate polygon count:"), info);
    iLayout->addWidget(ntri, 2, 0);
    approxNumPrimitives = new QLabel("0.", info);
    iLayout->addWidget(approxNumPrimitives, 2, 1);
    vLayout->addSpacing(5);

    QGridLayout *eLayout = new QGridLayout(0);
    eLayout->setSpacing(5);
    vLayout->addLayout(eLayout);
    QLabel *xExtentsLabel = new QLabel(tr("X Extents:"), info);
    eLayout->addWidget(xExtentsLabel, 0, 0);
    extents[0] = new QLabel("0.", info);
    eLayout->addWidget(extents[0], 0, 1);
    extents[1] = new QLabel("0.", info);
    eLayout->addWidget(extents[1], 0, 2);

    QLabel *yExtentsLabel = new QLabel(tr("Y Extents:"), info);
    eLayout->addWidget(yExtentsLabel, 1, 0);
    extents[2] = new QLabel("0.", info);
    eLayout->addWidget(extents[2], 1, 1);
    extents[3] = new QLabel("0.", info);
    eLayout->addWidget(extents[3], 1, 2);

    QLabel *zExtentsLabel = new QLabel(tr("Z Extents:"), info);
    eLayout->addWidget(zExtentsLabel, 2, 0);
    extents[4] = new QLabel("0.", info);
    eLayout->addWidget(extents[4], 2, 1);
    extents[5] = new QLabel("0.", info);
    eLayout->addWidget(extents[5], 2, 2);
    vLayout->addStretch(1000);

    return info;
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
//   Brad Whitlock, Thu Jun 19 11:57:46 PDT 2008
//   Moved code to helper functions.
//
// ****************************************************************************

void
QvisRenderingWindow::CreateWindowContents()
{
    int row = 0;

    QTabWidget *topTab = new QTabWidget(central);
    topLayout->addWidget(topTab);

    //
    // Create the basic renderer options page.
    //
    topTab->addTab(CreateBasicPage(), tr("Basic"));


    //
    // Create the advanced renderer options group.
    //
    topTab->addTab(CreateAdvancedPage(), tr("Advanced"));

    //
    // Create the renderer information group.
    //
    topTab->addTab(CreateInformationPage(), tr("Information"));
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
//   Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//   Changed scalable rendering controls to use activation mode and auto
//   threshold
//
//   Hank Childs, Sun Oct 24 07:36:18 PDT 2004
//   Added shadow options.
//
//   Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//   Fixed problem with updating scalable auto threshold
//
//   Kathleen Bonnell, Thu Jun 30 15:29:55 PDT 2005 
//   Added redgreen radiobutton.
//
//   Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//   Added compression controls
//
//   Brad Whitlock, Mon Sep 18 10:49:51 PDT 2006
//   Added colorTexturingFlag.
//
//   Jeremy Meredith, Wed Aug 29 15:27:16 EDT 2007
//   Added depth cueing.  Simplified and corrected window sensitivity.
//
//   Brad Whitlock, Mon Dec 17 10:25:23 PST 2007
//   Made it use ids.
//
//   Brad Whitlock, Thu Jun 19 13:26:50 PDT 2008
//   Qt 4.
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
        case RenderingAttributes::ID_antialiasing:
            antialiasingToggle->blockSignals(true);
            antialiasingToggle->setChecked(renderAtts->GetAntialiasing());
            antialiasingToggle->blockSignals(false);
            break;
        case RenderingAttributes::ID_geometryRepresentation:
            itmp = (int)renderAtts->GetGeometryRepresentation();
            objectRepresentation->blockSignals(true);
            objectRepresentation->button(itmp)->setChecked(true);
            objectRepresentation->blockSignals(false);
            break;
        case RenderingAttributes::ID_displayListMode:
            itmp = (int) renderAtts->GetDisplayListMode();
            if (itmp == 2) // Auto for atts's enum type order
                itmp2 = 0; // Order of Auto in window
            else if (itmp == 1) // Always for atts' enum type order
                itmp2 = 1; // Order of Always in window.
            else           // Never for atts' enum type order
                itmp2 = 2; // Order of Never in window.
            dlMode->blockSignals(true);
            dlMode->button(itmp2)->setChecked(true);
            dlMode->blockSignals(false);
            break;
        case RenderingAttributes::ID_stereoRendering:
            stereoToggle->blockSignals(true);
            stereoToggle->setChecked(renderAtts->GetStereoRendering());
            stereoToggle->blockSignals(false);
            break;
        case RenderingAttributes::ID_stereoType:
            stereoType->blockSignals(true);
            stereoType->button((int)renderAtts->GetStereoType())->setChecked(true);
            stereoType->blockSignals(false);
            break;
        case RenderingAttributes::ID_notifyForEachRender:
            renderNotifyToggle->blockSignals(true);
            renderNotifyToggle->setChecked(renderAtts->GetNotifyForEachRender());
            renderNotifyToggle->blockSignals(false);
            break;
        case RenderingAttributes::ID_scalableActivationMode:
            { // new scope
            RenderingAttributes::TriStateMode rtmp;
            rtmp = renderAtts->GetScalableActivationMode();
            scalrenActivationMode->blockSignals(true);
            if (rtmp == RenderingAttributes::Always)
               scalrenActivationMode->button(1)->setChecked(true);
            else if (rtmp == RenderingAttributes::Never)
               scalrenActivationMode->button(2)->setChecked(true);
            else
               scalrenActivationMode->button(0)->setChecked(true);
            scalrenActivationMode->blockSignals(false);
            }
            break;
        case RenderingAttributes::ID_scalableAutoThreshold:
            { // new scope
            QString suffix;           
            int step, widgetVal;
            int actualVal = renderAtts->GetScalableAutoThreshold();
            InterpretScalableAutoThreshold(actualVal, &step, &suffix, &widgetVal);
            scalrenAutoThreshold->blockSignals(true);
            scalrenAutoThreshold->setSuffix(suffix);
            scalrenAutoThreshold->setSingleStep(step);
            scalrenAutoThreshold->setValue(widgetVal);
            scalrenAutoThreshold->blockSignals(false);
            break;
            }
        case RenderingAttributes::ID_specularFlag:
            specularToggle->blockSignals(true);
            specularToggle->setChecked(renderAtts->GetSpecularFlag());
            specularToggle->blockSignals(false);
            break;
        case RenderingAttributes::ID_specularCoeff:
            specularStrengthSlider->blockSignals(true);
            specularStrengthSlider->setValue(int(renderAtts->GetSpecularCoeff()*100.));
            specularStrengthSlider->blockSignals(false);
            break;
        case RenderingAttributes::ID_specularPower:
            specularPowerSlider->blockSignals(true);
            specularPowerSlider->setValue(int(renderAtts->GetSpecularPower()*10.));
            specularPowerSlider->blockSignals(false);
            break;
        case RenderingAttributes::ID_specularColor:
            // Not user-modifiable at this time
            break;
        case RenderingAttributes::ID_doShadowing:
            shadowToggle->blockSignals(true);
            shadowToggle->setChecked(renderAtts->GetDoShadowing());
            shadowToggle->blockSignals(false);
            break;
        case RenderingAttributes::ID_shadowStrength:
            shadowStrengthSlider->blockSignals(true);
            shadowStrengthSlider->setValue(int(renderAtts->GetShadowStrength()*100.));
            shadowStrengthSlider->blockSignals(false);
            break;
        case RenderingAttributes::ID_doDepthCueing:
            depthCueingToggle->blockSignals(true);
            depthCueingToggle->setChecked(renderAtts->GetDoDepthCueing());
            depthCueingToggle->blockSignals(false);
            break;
        case RenderingAttributes::ID_startCuePoint:
            depthCueingStartEdit->blockSignals(true);
            tmp = DoublesToQString(renderAtts->GetStartCuePoint(), 3);
            depthCueingStartEdit->setText(tmp);
            depthCueingStartEdit->blockSignals(false);
            break;
        case RenderingAttributes::ID_endCuePoint:
            depthCueingEndEdit->blockSignals(true);
            tmp = DoublesToQString(renderAtts->GetEndCuePoint(), 3);
            depthCueingEndEdit->setText(tmp);
            depthCueingEndEdit->blockSignals(false);
            break;
        case RenderingAttributes::ID_compressionActivationMode:
            itmp = (int) renderAtts->GetCompressionActivationMode();
            if (itmp == 2) // Auto for atts's enum type order
                itmp2 = 0; // Order of Auto in window
            else if (itmp == 1) // Always for atts' enum type order
                itmp2 = 1; // Order of Always in window.
            else           // Never for atts' enum type order
                itmp2 = 2; // Order of Never in window.
            scalrenCompressMode->blockSignals(true);
            scalrenCompressMode->button(itmp2)->setChecked(true);
            scalrenCompressMode->blockSignals(false);
            break;
        case RenderingAttributes::ID_colorTexturingFlag:
            colorTexturingToggle->blockSignals(true);
            colorTexturingToggle->setChecked(renderAtts->GetColorTexturingFlag());
            colorTexturingToggle->blockSignals(false);
            break;
        }
    }

    UpdateWindowSensitivity();
}


// ****************************************************************************
//  Method:  QvisRenderingWindow::UpdateWindowSensitivity
//
//  Purpose:
//    Set the sensitivity of all widgets in this window based on
//    values in the attributes.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2007
//
//  Modifications:
//    Brad Whitlock, Thu Jun 19 13:44:11 PDT 2008
//    Qt 4.
//
//    Cyrus Harrison, Mon Jul 28 15:23:05 PDT 2008
//    I added code to enable/disable the scalable auto threshold spin box based
//    on the scalable rendering mode. 
//
// ****************************************************************************

void
QvisRenderingWindow::UpdateWindowSensitivity()
{
    bool scalableAlways =
        renderAtts->GetScalableActivationMode() == RenderingAttributes::Always;
    bool scalableAuto =
        renderAtts->GetScalableActivationMode() == RenderingAttributes::Auto;
    bool shadowOn = renderAtts->GetDoShadowing();
    bool depthCueingOn = renderAtts->GetDoDepthCueing();
    bool stereoOn = renderAtts->GetStereoRendering();
    bool specularOn = renderAtts->GetSpecularFlag();

    scalrenAutoThreshold->setEnabled(scalableAuto);
    shadowToggle->setEnabled(scalableAlways);
    shadowStrengthSlider->setEnabled(scalableAlways && shadowOn);
    shadowStrengthLabel->setEnabled(scalableAlways && shadowOn);

    depthCueingToggle->setEnabled(scalableAlways);
    depthCueingStartEdit->setEnabled(scalableAlways && depthCueingOn);
    depthCueingStartLabel->setEnabled(scalableAlways && depthCueingOn);
    depthCueingEndEdit->setEnabled(scalableAlways && depthCueingOn);
    depthCueingEndLabel->setEnabled(scalableAlways && depthCueingOn);

    redblue->setEnabled(stereoOn);
    interlace->setEnabled(stereoOn);
    crystalEyes->setEnabled(stereoOn);
    redgreen->setEnabled(stereoOn);

    specularStrengthSlider->setEnabled(specularOn);
    specularPowerSlider->setEnabled(specularOn);
    specularStrengthLabel->setEnabled(specularOn);
    specularPowerLabel->setEnabled(specularOn);
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
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Mark C. Miller, Wed Jan  5 16:23:01 PST 2005
//   Fixed problem with case lables not matching Select statements
//
//   Mark C. Miller, Fri Mar  4 13:05:02 PST 2005
//   Changed GetNumTriangles to GetNumPrimitives
//
//   Mark C. Miller, Thu Jul 21 20:16:42 PDT 2005
//   Added a break; statement after case label 16
//
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Added seconds per frame for < 1 fps cases 
//
//   Brad Whitlock, Mon Dec 17 10:30:33 PST 2007
//   Made it use ids.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
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
        case WindowInformation::ID_activeSource:
        case WindowInformation::ID_activeTimeSlider:
        case WindowInformation::ID_timeSliders:
        case WindowInformation::ID_timeSliderCurrentStates:
        case WindowInformation::ID_animationMode:
        case WindowInformation::ID_interactionMode:
        case WindowInformation::ID_boundingBoxNavigate:
        case WindowInformation::ID_spin:
        case WindowInformation::ID_fullFrame:
        case WindowInformation::ID_perspective:
        case WindowInformation::ID_lockView:
        case WindowInformation::ID_lockTools:
        case WindowInformation::ID_lockTime:
        case WindowInformation::ID_viewExtentsType:
        case WindowInformation::ID_viewDimension:
        case WindowInformation::ID_viewKeyframes:
        case WindowInformation::ID_cameraViewMode:
            break;
        case WindowInformation::ID_usingScalableRendering:
            scalrenUsingLabel->setText(windowInfo->GetUsingScalableRendering() ? tr("yes") : tr("no"));
            break;
        case WindowInformation::ID_lastRenderMin:
            // Determine the fps.
            if(windowInfo->GetLastRenderMin() > 0. &&
               windowInfo->GetLastRenderMin() < 1.e05)
                fps = 1. / windowInfo->GetLastRenderMin();
            else
                fps = 0.;
            tmp.sprintf("%1.3g", fps);
            fpsMaxLabel->setText(tmp);
            break;
        case WindowInformation::ID_lastRenderAvg:
            // Determine the fps.
            if(windowInfo->GetLastRenderAvg() > 0.)
                fps = 1. / windowInfo->GetLastRenderAvg();
            else
                fps = 0.;
            if (fps > 0.0 && fps < 1.0)
            {
                fps = 1.0 / fps;
                fpsLabel->setText(tr("Seconds per frame:"));
            }
            else
            {
                fpsLabel->setText(tr("Frames per second:"));
            }
            tmp.sprintf("%1.3g", fps);
            fpsAvgLabel->setText(tmp);
            break;
        case WindowInformation::ID_lastRenderMax:
            // Determine the fps.
            if(windowInfo->GetLastRenderMax() > 0.)
                fps = 1. / windowInfo->GetLastRenderMax();
            else
                fps = 0.;
            tmp.sprintf("%1.3g", fps);
            fpsMinLabel->setText(tmp);
            break;
        case WindowInformation::ID_numPrimitives:
            tmp.sprintf("%d", windowInfo->GetNumPrimitives());
            approxNumPrimitives->setText(tmp);
            break;
        case WindowInformation::ID_extents:
            for(j = 0; j < 6; ++j)
            {
                double d = windowInfo->GetExtents()[j];
                if(d == DBL_MAX || d == -DBL_MAX)
                    tmp = tr("not set");
                else
                    tmp.setNum(windowInfo->GetExtents()[j]);
                extents[j]->setText(tmp);
            }
            break;
        case WindowInformation::ID_windowSize:
        case WindowInformation::ID_winMode:
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
        GetViewerMethods()->SetRenderingAttributes();
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
//   Jeremy Meredith, Wed Aug 29 15:27:16 EDT 2007
//   Added call to GetCurrentValues.
//   
// ****************************************************************************

void
QvisRenderingWindow::apply()
{
    GetCurrentValues();
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

    renderAtts->SetDisplayListMode((RenderingAttributes::TriStateMode)itmp);
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
//   Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//   Changed scalable rendering controls to use activation mode and auto
//   threshold
//
//   Hank Childs, Sun Dec  4 18:41:39 PST 2005
//   Automatically update the shadow toggle ['5596].
//
//   Jeremy Meredith, Wed Aug 29 15:28:05 EDT 2007
//   Moved window sensitivity handling to its own function.
//
// ****************************************************************************

void
QvisRenderingWindow::scalrenActivationModeChanged(int val)
{
    if (val == 0)
    {
        renderAtts->SetScalableActivationMode(RenderingAttributes::Auto);
        scalrenAutoThresholdChanged(scalrenAutoThreshold->value());
    }
    else if (val == 1)
    {
        renderAtts->SetScalableActivationMode(RenderingAttributes::Always);
    }
    else
    {
        renderAtts->SetScalableActivationMode(RenderingAttributes::Never);
    }
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisRenderingWindow::InterpretScalableAutoThreshold
//
// Purpose: Determine widget controls given scalable auto threshold 
//
// Programmer: Mark C. Miller 
// Creation:   January 4, 2005 
//
// ****************************************************************************
void
QvisRenderingWindow::InterpretScalableAutoThreshold(int actualVal,
    int *step, QString *suffix, int *widgetVal) const
{
    // compute the step size and suffix for the displayed value in the GUI
    if      (actualVal < 1e3) { *step = 50;  *suffix = "  Polys"; }
    else if (actualVal < 1e4) { *step = 500; *suffix = "  Polys"; }
    else if (actualVal < 1e5) { *step = 5;   *suffix = " KPolys"; }
    else if (actualVal < 1e6) { *step = 50;  *suffix = " KPolys"; }
    else if (actualVal < 1e7) { *step = 500; *suffix = " KPolys"; }
    else if (actualVal < 1e8) { *step = 5;   *suffix = " MPolys"; }
    else if (actualVal < 1e9) { *step = 50;  *suffix = " MPolys"; }
    else                      { *step = 1;   *suffix = " GPolys"; }

    // compute the divisor for the displayed value in the GUI
    int div;
    if      (*suffix == " KPolys") div = (int) 1e3;
    else if (*suffix == " MPolys") div = (int) 1e6; 
    else if (*suffix == " GPolys") div = (int) 1e9;
    else                          div = 1;

    *widgetVal = actualVal / div;
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
//   Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//   Changed scalable rendering controls to use activation mode and auto
//   threshold
//
//   Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//   Changed to use InterpretScalableAutoThreshold
//
//   Brad Whitlock, Thu Jun 19 13:45:24 PDT 2008
//   Qt 4.
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
    int step = scalrenAutoThreshold->singleStep();
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

    int widgetVal;
    QString suffix;
    InterpretScalableAutoThreshold(actualVal, &step, &suffix, &widgetVal);

    // set the new GUI value
    scalrenAutoThreshold->setSingleStep(step);
    scalrenAutoThreshold->setSuffix(suffix);
    scalrenAutoThreshold->setValue(widgetVal);

    renderAtts->SetScalableAutoThreshold(actualVal);

    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisRenderingWindow::scalrenCompressModeChanged
//
// Programmer: Mark C. Miller
// Creation:   November 2, 2005 
//
// ****************************************************************************

void
QvisRenderingWindow::scalrenCompressModeChanged(int mode)
{
    int itmp = 0;
    if (mode == 0)      // Auto in Window
        itmp = 2;       // Auto for atts' enum type
    else if (mode == 1) // Always in window
        itmp = 1;       // Always for atts' enum type
    else                // Never in window.
        itmp = 0;       // Never for atts' enum type

    renderAtts->SetCompressionActivationMode((RenderingAttributes::TriStateMode)itmp);
    SetUpdate(false);
    Apply();
}
// ****************************************************************************
//  Method:  QvisRenderingWindow::shadowToggled
//
//  Purpose:
//    Callback for the shadow toggle button
//
//  Arguments:
//    val        true to enable shadow
//
//  Programmer:  Hank Childs
//  Creation:    October 24, 2004
//
//  Modifications:
//    Jeremy Meredith, Wed Aug 29 15:28:05 EDT 2007
//    Moved window sensitivity handling to its own function.
//
// ****************************************************************************

void
QvisRenderingWindow::shadowToggled(bool val)
{
    renderAtts->SetDoShadowing(val);
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::shadowStrengthChanged
//
//  Purpose:
//    Callback for the shadow coefficient slider
//
//  Arguments:
//    val        the new coefficient
//
//  Programmer:  Hank Childs
//  Creation:    October 24, 2004
//
// ****************************************************************************

void
QvisRenderingWindow::shadowStrengthChanged(int val, const void*)
{
    renderAtts->SetShadowStrength(float(val)/100.);
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
//  Modifications:
//    Jeremy Meredith, Wed Aug 29 15:28:05 EDT 2007
//    Moved window sensitivity handling to its own function.
//
// ****************************************************************************

void
QvisRenderingWindow::specularToggled(bool val)
{
    renderAtts->SetSpecularFlag(val);
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
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

// ****************************************************************************
// Method: QvisRenderingWindow::colorTexturingToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the colorTexturing check
//   box is toggled.
//
// Arguments:
//   val : The new on/off value for the widget.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 18 10:52:30 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisRenderingWindow::colorTexturingToggled(bool val)
{
    renderAtts->SetColorTexturingFlag(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::depthCueingToggled
//
//  Purpose:
//    Triggered when depth cueing is toggled.
//
//  Arguments:
//    val        the new value for the flag
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2007
//
// ****************************************************************************
void
QvisRenderingWindow::depthCueingToggled(bool val)
{
    renderAtts->SetDoDepthCueing(val);
    UpdateWindowSensitivity();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::depthCueingStartChanged
//
//  Purpose:
//    Triggered when return is pressed in the depth cueing start point widget.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2007
//
// ****************************************************************************
void
QvisRenderingWindow::depthCueingStartChanged()
{
    GetCurrentValues();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisRenderingWindow::depthCueingEndChanged
//
//  Purpose:
//    Triggered when return is pressed in the depth cueing end point widget.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2007
//
// ****************************************************************************
void
QvisRenderingWindow::depthCueingEndChanged()
{
    GetCurrentValues();
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
//  Method:  QvisRenderingWindow::GetCurrentValues
//
//  Purpose:
//    Fill the values in the attributes from the ones in widgets which
//    do not update continuously (like line edits).
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2007
//
//  Modifications:
//    Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//    Support for internationalization.
//
//    Brad Whitlock, Thu Jun 19 13:34:38 PDT 2008
//    Use convenience methods.
//
// ****************************************************************************

void
QvisRenderingWindow::GetCurrentValues()
{
    double v[3];

    if(LineEditGetDoubles(depthCueingStartEdit, v, 3))
        renderAtts->SetStartCuePoint(v);
    else
    {
        ResettingError(tr("start point"),
                       DoublesToQString(renderAtts->GetStartCuePoint(), 3));
        renderAtts->SetStartCuePoint(renderAtts->GetStartCuePoint());
    }

    if(LineEditGetDoubles(depthCueingStartEdit, v, 3))
        renderAtts->SetEndCuePoint(v);
    else
    {
        ResettingError(tr("end point"),
                       DoublesToQString(renderAtts->GetEndCuePoint(), 3));
        renderAtts->SetEndCuePoint(renderAtts->GetEndCuePoint());
    }
}
