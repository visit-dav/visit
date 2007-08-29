/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
#include <qlineedit.h>
#include <qtabwidget.h>

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
// ****************************************************************************

QvisRenderingWindow::~QvisRenderingWindow()
{
    delete objectRepresentation;
    delete dlMode;
    delete stereoType;
    delete scalrenActivationMode;
    delete scalrenCompressMode;

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
//   Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//   Changed scalable rendering controls to use activation mode and auto
//   threshold
//
//   Hank Childs, Sun Oct 24 07:36:18 PDT 2004
//   Added shadow options.
//
//   Mark C. Miller, Fri Mar  4 13:05:02 PST 2005
//   Changed approxNumTriangles to approxNumPrimitives
//
//   Kathleen Bonnell, Thu Jun 30 15:29:55 PDT 2005 
//   Added redgreen radiobutton.
//
//   Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//   Added compression controls
//
//   Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//   Changed label string for compression 
//
//   Brad Whitlock, Mon Sep 18 10:46:02 PDT 2006
//   Added colorTexturing check box.
//
//   Jeremy Meredith, Wed Aug 29 15:26:26 EDT 2007
//   Split options into basic and advanced.
//   Added tab support since the window was too tall.
//   Added row counter for grid widget layout.
//   Added depth cueing.
//
// ****************************************************************************
#define TABS
void
QvisRenderingWindow::CreateWindowContents()
{
    int row;

#ifdef TABS
    QTabWidget *topTab = new QTabWidget(central, "topTab");
    topLayout->addWidget(topTab);
#endif

    //
    // Create the basic renderer options group.
    //
    row = 0;
    QGroupBox *basicOptions = new QGroupBox(central, "basicOptions");
#ifdef TABS
    topTab->addTab(basicOptions, "Basic");
    basicOptions->setFrameStyle(QFrame::NoFrame);
#else
    basicOptions->setTitle("Basic Options");
    topLayout->addWidget(basicOptions);
#endif

    QVBoxLayout *spacer1 = new QVBoxLayout(basicOptions);
    spacer1->addSpacing(10);
    QGridLayout *o1Layout = new QGridLayout(spacer1, 12, 4);
    o1Layout->setSpacing(5);
    o1Layout->setMargin(10);

    // Create the antialiasing widgets.
    antialiasingToggle = new QCheckBox("Antialiasing", basicOptions,
        "antialiasingToggle");
    connect(antialiasingToggle, SIGNAL(toggled(bool)),
            this, SLOT(antialiasingToggled(bool)));
    o1Layout->addMultiCellWidget(antialiasingToggle, row, row, 0, 1);
    row++;

    // Create the surface rep widgets.
    QLabel *drawObjLabel = new QLabel("Draw objects as", basicOptions, "drawObjLabel");
    o1Layout->addMultiCellWidget(drawObjLabel, row, row, 0, 3);
    objectRepresentation = new QButtonGroup(0, "objectRepresentation");
    connect(objectRepresentation, SIGNAL(clicked(int)),
            this, SLOT(objectRepresentationChanged(int)));
    row++;

    QRadioButton *surfaces = new QRadioButton("Surfaces", basicOptions, "surfaces");
    objectRepresentation->insert(surfaces);
    o1Layout->addWidget(surfaces, row, 1);
    QRadioButton *wires = new QRadioButton("Wireframe", basicOptions,
        "wires");
    objectRepresentation->insert(wires);
    o1Layout->addWidget(wires, row, 2);
    QRadioButton *points = new QRadioButton("Points", basicOptions,
        "points");
    objectRepresentation->insert(points);
    o1Layout->addWidget(points, row, 3);
    row++;

    // Create the display list widgets.
    QLabel *displayListLabel = new QLabel("Use display lists", basicOptions, "displayListLabel");
    o1Layout->addMultiCellWidget(displayListLabel, row, row, 0, 3);
    dlMode = new QButtonGroup(0, "displayList");
    connect(dlMode, SIGNAL(clicked(int)),
            this, SLOT(displayListModeChanged(int)));
    row++;

    QRadioButton *dl_auto = new QRadioButton("Auto", basicOptions, "dl_auto");
    dlMode->insert(dl_auto);
    o1Layout->addWidget(dl_auto, row, 1);
    QRadioButton *dl_always = new QRadioButton("Always", basicOptions,
        "dl_always");
    dlMode->insert(dl_always);
    o1Layout->addWidget(dl_always, row, 2);
    QRadioButton *dl_never = new QRadioButton("Never", basicOptions,
        "dl_never");
    dlMode->insert(dl_never);
    o1Layout->addWidget(dl_never, row, 3);
    row++;

    // Create the stereo widgets.
    stereoToggle = new QCheckBox("Stereo", basicOptions,
        "stereoToggle");
    connect(stereoToggle, SIGNAL(toggled(bool)),
            this, SLOT(stereoToggled(bool)));
    o1Layout->addMultiCellWidget(stereoToggle, row, row, 0, 3);
    row++;

    stereoType = new QButtonGroup(0, "stereoType");
    connect(stereoType, SIGNAL(clicked(int)),
            this, SLOT(stereoTypeChanged(int)));
    redblue = new QRadioButton("Red/Blue", basicOptions, "redblue");
    stereoType->insert(redblue);
    o1Layout->addWidget(redblue, row, 1);
    interlace = new QRadioButton("Interlace", basicOptions,
        "interlace");
    stereoType->insert(interlace);
    o1Layout->addWidget(interlace, row, 2);
    row++;
    crystalEyes = new QRadioButton("Crystal Eyes", basicOptions,
        "crystalEyes");
    stereoType->insert(crystalEyes);
    o1Layout->addWidget(crystalEyes, row, 1);
    redgreen = new QRadioButton("Red/Green", basicOptions, "redgreen");
    stereoType->insert(redgreen);
    o1Layout->addWidget(redgreen, row, 2);
    row++;

    // Create the specular lighting options
    specularToggle = new QCheckBox("Specular lighting", basicOptions,
                                   "specularToggle");
    connect(specularToggle, SIGNAL(toggled(bool)),
            this, SLOT(specularToggled(bool)));
    o1Layout->addMultiCellWidget(specularToggle, row, row, 0,3);
    row++;

    specularStrengthSlider = new QvisOpacitySlider(0, 100, 10, 60, basicOptions,
                                             "specularStrengthSlider", NULL);
    specularStrengthSlider->setTickInterval(25);
    connect(specularStrengthSlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(specularStrengthChanged(int, const void*)));
    specularStrengthLabel = new QLabel(specularStrengthSlider, "Strength",
                                       basicOptions, "specularStrengthLabel");
    o1Layout->addWidget(specularStrengthLabel, row,1);
    o1Layout->addMultiCellWidget(specularStrengthSlider, row, row, 2,3);
    row++;

    specularPowerSlider = new QvisOpacitySlider(0, 1000, 100, 100, basicOptions,
                                                "specularPowerSlider", NULL);
    specularPowerSlider->setTickInterval(100);
    connect(specularPowerSlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(specularPowerChanged(int, const void*)));
    specularPowerLabel = new QLabel(specularPowerSlider, "Sharpness",
                                    basicOptions, "specularPowerLabel");
    o1Layout->addWidget(specularPowerLabel, row,1);
    o1Layout->addMultiCellWidget(specularPowerSlider, row, row, 2,3);
    row++;

    //
    // Create the advanced renderer options group.
    //
    row = 0;
    QGroupBox *advancedOptions = new QGroupBox(central, "advancedOptions");
#ifdef TABS
    topTab->addTab(advancedOptions, "Advanced");
    advancedOptions->setFrameStyle(QFrame::NoFrame);
#else
    advancedOptions->setTitle("Advanced Options");
    topLayout->addWidget(advancedOptions);
#endif

    QVBoxLayout *spacer2 = new QVBoxLayout(advancedOptions);
    spacer2->addSpacing(10);
    QGridLayout *o2Layout = new QGridLayout(spacer2, 12, 4);
    o2Layout->setSpacing(5);
    o2Layout->setMargin(10);


    // Create the scalable rendering widgets.
    QLabel *scalrenLabel = new QLabel("Use scalable rendering", advancedOptions,"scalrenLabel");
    o2Layout->addMultiCellWidget(scalrenLabel, row, row, 0, 3);
    scalrenActivationMode = new QButtonGroup(0, "scalrenActivationMode");
    connect(scalrenActivationMode, SIGNAL(clicked(int)),
            this, SLOT(scalrenActivationModeChanged(int)));
    row++;

    scalrenAuto = new QRadioButton("Auto", advancedOptions, "auto");
    scalrenActivationMode->insert(scalrenAuto);
    o2Layout->addWidget(scalrenAuto, row, 1);
    scalrenAlways = new QRadioButton("Always", advancedOptions, "always");
    scalrenActivationMode->insert(scalrenAlways);
    o2Layout->addWidget(scalrenAlways, row, 2);
    scalrenNever = new QRadioButton("Never", advancedOptions, "never");
    scalrenActivationMode->insert(scalrenNever);
    o2Layout->addWidget(scalrenNever, row, 3);
    row++;

    // Create the polygon count spin box for scalable rendering threshold
    scalrenGeometryLabel =  new QLabel("When polygon count exceeds", advancedOptions, "scalrenGeometryLabel");
    o2Layout->addMultiCellWidget(scalrenGeometryLabel, row, row, 1, 2);
    scalrenAutoThreshold = new QSpinBox(0, 10000, 500, advancedOptions, "scalrenAutoThreshold");
    scalrenAutoThreshold->setValue(RenderingAttributes::DEFAULT_SCALABLE_AUTO_THRESHOLD);
    scalrenAutoThresholdChanged(RenderingAttributes::DEFAULT_SCALABLE_AUTO_THRESHOLD);
    connect(scalrenAutoThreshold, SIGNAL(valueChanged(int)),
            this, SLOT(scalrenAutoThresholdChanged(int)));
    o2Layout->addWidget(scalrenAutoThreshold, row, 3);
    row++;

    // Create the compress mode widgets.
    scalrenCompressLabel = new QLabel("Compress images (geom too) from engine",
                                                   advancedOptions, "compressModeLabel");
    o2Layout->addMultiCellWidget(scalrenCompressLabel, row, row, 0, 3);
    scalrenCompressMode = new QButtonGroup(0, "compressMode");
    connect(scalrenCompressMode, SIGNAL(clicked(int)),
            this, SLOT(scalrenCompressModeChanged(int)));
    row++;

    QRadioButton *cmp_auto = new QRadioButton("Auto", advancedOptions, "cmp_auto");
    cmp_auto->setEnabled(0); // disabled until implemented
    scalrenCompressMode->insert(cmp_auto);
    o2Layout->addWidget(cmp_auto, row, 1);
    QRadioButton *cmp_always = new QRadioButton("Always", advancedOptions,
        "cmp_always");
    scalrenCompressMode->insert(cmp_always);
    o2Layout->addWidget(cmp_always, row, 2);
    QRadioButton *cmp_never = new QRadioButton("Never", advancedOptions,
        "cmp_never");
    scalrenCompressMode->insert(cmp_never);
    o2Layout->addWidget(cmp_never, row, 3);
    row++;

    // Create the shadow lighting options
    shadowToggle = new QCheckBox("Shadows (scalable rendering only)", advancedOptions,
                                   "shadowToggle");
    connect(shadowToggle, SIGNAL(toggled(bool)),
            this, SLOT(shadowToggled(bool)));
    o2Layout->addMultiCellWidget(shadowToggle, row, row, 0,3);
    row++;

    shadowStrengthSlider = new QvisOpacitySlider(0, 100, 10, 60, advancedOptions,
                                             "shadowStrengthSlider", NULL);
    shadowStrengthSlider->setTickInterval(25);
    connect(shadowStrengthSlider, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(shadowStrengthChanged(int, const void*)));
    shadowStrengthLabel = new QLabel(shadowStrengthSlider, "Strength",
                                       advancedOptions, "shadowStrengthLabel");
    o2Layout->addWidget(shadowStrengthLabel, row,1);
    o2Layout->addMultiCellWidget(shadowStrengthSlider, row, row, 2,3);
    row++;

    // Create the depth cueing options
    depthCueingToggle = new QCheckBox("Depth Cueing (scalable rendering only)", advancedOptions,
                                      "depthCueingToggle");
    connect(depthCueingToggle, SIGNAL(toggled(bool)),
            this, SLOT(depthCueingToggled(bool)));
    o2Layout->addMultiCellWidget(depthCueingToggle, row, row, 0,3);
    row++;

    depthCueingStartLabel = new QLabel("Start point", advancedOptions);
    o2Layout->addMultiCellWidget(depthCueingStartLabel, row, row, 1,1);
    depthCueingStartEdit = new QLineEdit(advancedOptions, "depthCueingStartEdit");
    o2Layout->addMultiCellWidget(depthCueingStartEdit, row, row, 2,3);
    connect(depthCueingStartEdit, SIGNAL(returnPressed()),
            this, SLOT(depthCueingStartChanged()));
    row++;

    depthCueingEndLabel = new QLabel("End point", advancedOptions);
    o2Layout->addMultiCellWidget(depthCueingEndLabel, row, row, 1,1);
    depthCueingEndEdit = new QLineEdit(advancedOptions, "depthCueingEndEdit");
    o2Layout->addMultiCellWidget(depthCueingEndEdit, row, row, 2,3);
    connect(depthCueingEndEdit, SIGNAL(returnPressed()),
            this, SLOT(depthCueingEndChanged()));
    row++;
    
    // Create color texturing options.
    colorTexturingToggle = new QCheckBox("Apply color using textures", advancedOptions,
        "colorTexturingToggle");
    connect(colorTexturingToggle, SIGNAL(toggled(bool)),
            this, SLOT(colorTexturingToggled(bool)));
    o2Layout->addMultiCellWidget(colorTexturingToggle, row, row, 0, 3);
    row++;

    //
    // Create the renderer information group.
    //
    QGroupBox *info = new QGroupBox(central, "info");
#ifdef TABS
    topTab->addTab(info, "Information");
    info->setFrameStyle(QFrame::NoFrame);
#else
    info->setTitle("Information");
    topLayout->addWidget(info);
#endif

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
    fpsLabel = new QLabel("Frames per second:", info, "fpsLabel");
    iLayout->addWidget(fpsLabel, 0, 0);
    fpsMinLabel = new QLabel("0.", info, "fpsMinLabel");
    iLayout->addWidget(fpsMinLabel, 0, 1);
    fpsAvgLabel = new QLabel("0.", info, "fpsAvgLabel");
    iLayout->addWidget(fpsAvgLabel, 0, 2);
    fpsMaxLabel = new QLabel("0.", info, "fpsMaxLabel");
    iLayout->addWidget(fpsMaxLabel, 0, 3);

    QLabel *ntri = new QLabel("Approximate polygon count:", info, "ntri");
    iLayout->addWidget(ntri, 1, 0);
    approxNumPrimitives = new QLabel("0.", info, "approxNumPrimitives");
    iLayout->addWidget(approxNumPrimitives, 1, 1);
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

    vLayout->addStretch(1000);
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
// ****************************************************************************

void
QvisRenderingWindow::UpdateOptions(bool doAll)
{
    QString tmp;
    int itmp, itmp2;
    double *dptr;

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
        case 6: //scalrenActivationMode
            RenderingAttributes::TriStateMode rtmp;
            rtmp = renderAtts->GetScalableActivationMode();
            scalrenActivationMode->blockSignals(true);
            if (rtmp == RenderingAttributes::Always)
               scalrenActivationMode->setButton(1);
            else if (rtmp == RenderingAttributes::Never)
               scalrenActivationMode->setButton(2);
            else
               scalrenActivationMode->setButton(0);
            scalrenActivationMode->blockSignals(false);
            break;
        case 7: //scalrenAutoThreshold
        {
            QString suffix;           
            int step, widgetVal;
            int actualVal = renderAtts->GetScalableAutoThreshold();
            InterpretScalableAutoThreshold(actualVal, &step, &suffix, &widgetVal);
            scalrenAutoThreshold->blockSignals(true);
            scalrenAutoThreshold->setLineStep(step);
            scalrenAutoThreshold->setSuffix(suffix);
            scalrenAutoThreshold->setValue(widgetVal);
            scalrenAutoThreshold->blockSignals(false);
            break;
        }
        case 8: //specularFlag
            specularToggle->blockSignals(true);
            specularToggle->setChecked(renderAtts->GetSpecularFlag());
            specularToggle->blockSignals(false);
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
        case 12: //shadowFlag
            shadowToggle->blockSignals(true);
            shadowToggle->setChecked(renderAtts->GetDoShadowing());
            shadowToggle->blockSignals(false);
            break;
        case 13: //shadowStrength
            shadowStrengthSlider->blockSignals(true);
            shadowStrengthSlider->setValue(int(renderAtts->GetShadowStrength()*100.));
            shadowStrengthSlider->blockSignals(false);
            break;
        case 14: //doDepthCueing
            depthCueingToggle->blockSignals(true);
            depthCueingToggle->setChecked(renderAtts->GetDoDepthCueing());
            depthCueingToggle->blockSignals(false);
            break;
        case 15: //startCuePoint
            depthCueingStartEdit->blockSignals(true);
            dptr = renderAtts->GetStartCuePoint();
            tmp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            depthCueingStartEdit->setText(tmp);
            depthCueingStartEdit->blockSignals(false);
            break;
        case 16: //endCuePoint
            depthCueingEndEdit->blockSignals(true);
            dptr = renderAtts->GetEndCuePoint();
            tmp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            depthCueingEndEdit->setText(tmp);
            depthCueingEndEdit->blockSignals(false);
            break;
        case 17: //scalrenCompressMode
            itmp = (int) renderAtts->GetCompressionActivationMode();
            if (itmp == 2) // Auto for atts's enum type order
                itmp2 = 0; // Order of Auto in window
            else if (itmp == 1) // Always for atts' enum type order
                itmp2 = 1; // Order of Always in window.
            else           // Never for atts' enum type order
                itmp2 = 2; // Order of Never in window.
            scalrenCompressMode->blockSignals(true);
            scalrenCompressMode->setButton(itmp2);
            scalrenCompressMode->blockSignals(false);
            break;
        case 18: //colorTexturingFlag
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
// ****************************************************************************
void
QvisRenderingWindow::UpdateWindowSensitivity()
{
    bool scalableAlways =
        renderAtts->GetScalableActivationMode() == RenderingAttributes::Always;
    bool shadowOn = renderAtts->GetDoShadowing();
    bool depthCueingOn = renderAtts->GetDoDepthCueing();
    bool stereoOn = renderAtts->GetStereoRendering();
    bool specularOn = renderAtts->GetSpecularFlag();

    shadowToggle->setEnabled(scalableAlways);
    shadowStrengthSlider->setEnabled(scalableAlways && shadowOn);
    shadowStrengthLabel->setEnabled(scalableAlways && shadowOn);

    depthCueingToggle->setEnabled(scalableAlways);
    depthCueingStartEdit->setEnabled(scalableAlways && depthCueingOn);
    depthCueingStartLabel->setEnabled(scalableAlways && depthCueingOn);
    depthCueingEndEdit->setEnabled(scalableAlways && depthCueingOn);
    depthCueingEndLabel->setEnabled(scalableAlways && depthCueingOn);

    stereoType->setEnabled(stereoOn);
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
        case 0: //activeSource
        case 1: //activeTimeSlider
        case 2: //timeSliders
        case 3: //timeSliderCurrentStates
        case 4: //animationMode
        case 5: //windowMode
        case 6: //boundingBoxNavigate
        case 7: //spin
        case 8: //fullFrame
        case 9: //perspective
        case 10: //lockView
        case 11: //lockTools
        case 12: //lockTime
        case 13: //viewExtentsType
        case 14: //viewDimension
        case 15: //viewKeyframes
        case 16: //cameraViewMode
            break;
        case 17: // usingScalableRendering
            scalrenUsingLabel->setText(windowInfo->GetUsingScalableRendering() ? "yes" : "no");
            break;
        case 18: //lastRenderMin
            // Determine the fps.
            if(windowInfo->GetLastRenderMin() > 0. &&
               windowInfo->GetLastRenderMin() < 1.e05)
                fps = 1. / windowInfo->GetLastRenderMin();
            else
                fps = 0.;
            tmp.sprintf("%1.3g", fps);
            fpsMaxLabel->setText(tmp);
            break;
        case 19: //lastRenderAvg
            // Determine the fps.
            if(windowInfo->GetLastRenderAvg() > 0.)
                fps = 1. / windowInfo->GetLastRenderAvg();
            else
                fps = 0.;
            if (fps > 0.0 && fps < 1.0)
            {
                fps = 1.0 / fps;
                fpsLabel->setText("Seconds per frame:");
            }
            else
            {
                fpsLabel->setText("Frames per second:");
            }
            tmp.sprintf("%1.3g", fps);
            fpsAvgLabel->setText(tmp);
            break;
        case 20: //lastRenderMax
            // Determine the fps.
            if(windowInfo->GetLastRenderMax() > 0.)
                fps = 1. / windowInfo->GetLastRenderMax();
            else
                fps = 0.;
            tmp.sprintf("%1.3g", fps);
            fpsMinLabel->setText(tmp);
            break;
        case 21: //numPrimitives
            tmp.sprintf("%d", windowInfo->GetNumPrimitives());
            approxNumPrimitives->setText(tmp);
            break;
        case 22: //extents
            for(j = 0; j < 6; ++j)
            {
                double d = windowInfo->GetExtents()[j];
                if(d == DBL_MAX || d == -DBL_MAX)
                    tmp = "not set";
                else
                    tmp.setNum(windowInfo->GetExtents()[j]);
                extents[j]->setText(tmp);
            }
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

    int widgetVal;
    QString suffix;
    InterpretScalableAutoThreshold(actualVal, &step, &suffix, &widgetVal);

    // set the new GUI value
    scalrenAutoThreshold->setLineStep(step);
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
// ****************************************************************************
void
QvisRenderingWindow::GetCurrentValues()
{
    bool    okay;
    double  vals[3];
    QString msg, temp;

    temp = depthCueingStartEdit->displayText().simplifyWhiteSpace();
    okay = !temp.isEmpty();
    if(okay)
    {
        okay = (sscanf(temp.latin1(), "%lg %lg %lg",
                       &vals[0], &vals[1], &vals[2]) == 3);
        if(okay)
        {
            renderAtts->SetStartCuePoint(vals);
        }

        if(!okay)
        {
            const double *d = renderAtts->GetStartCuePoint();
            msg.sprintf("The start point was invalid. "
                        "Resetting to the last good value <%g %g %g>.",
                        d[0], d[1], d[2]);
            Message(msg);
            renderAtts->SetStartCuePoint(d);
        }
    }

    temp = depthCueingEndEdit->displayText().simplifyWhiteSpace();
    okay = !temp.isEmpty();
    if(okay)
    {
        okay = (sscanf(temp.latin1(), "%lg %lg %lg",
                       &vals[0], &vals[1], &vals[2]) == 3);
        if(okay)
        {
            renderAtts->SetEndCuePoint(vals);
        }

        if(!okay)
        {
            const double *d = renderAtts->GetEndCuePoint();
            msg.sprintf("The end point was invalid. "
                        "Resetting to the last good value <%g %g %g>.",
                        d[0], d[1], d[2]);
            Message(msg);
            renderAtts->SetEndCuePoint(d);
        }
    }
}
