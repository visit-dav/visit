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

#include "QvisStreamlinePlotWindow.h"

#include <StreamlineAttributes.h>
#include <ViewerProxy.h>
#include <DataNode.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qtooltip.h>
#include <QvisColorTableButton.h>
#include <QvisColorButton.h>
#include <QvisLineWidthWidget.h>
#include <stdio.h>


// ****************************************************************************
// Method: QvisStreamlinePlotWindow::QvisStreamlinePlotWindow
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisStreamlinePlotWindow::QvisStreamlinePlotWindow(const int type,
                         StreamlineAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad)
{
    plotType = type;
    streamAtts = subj;
}


// ****************************************************************************
// Method: QvisStreamlinePlotWindow::~QvisStreamlinePlotWindow
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisStreamlinePlotWindow::~QvisStreamlinePlotWindow()
{
}


// ****************************************************************************
// Method: QvisStreamlinePlotWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Dec 22 13:05:04 PST 2004
//   I added support for coloring by vorticity.
//
//   Hank Childs, Sat Mar  3 09:11:44 PST 2007
//   Added useWholeBox.
//
//   Dave Pugmire, Thu Nov 15 12:09:08 EST 2007
//   Add streamline direction option.
//
//   Brad Whitlock, Wed Apr 23 11:46:59 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 5, 2, 10, "mainLayout");

    // Create the step length text field.
    mainLayout->addWidget(new QLabel(tr("Step length"), central, "stepLengthLabel"),0,0);
    stepLength = new QLineEdit(central, "stepLength");
    connect(stepLength, SIGNAL(returnPressed()),
            this, SLOT(stepLengthProcessText()));
    mainLayout->addWidget(stepLength, 0,1);

    // Create the maximum time text field.
    mainLayout->addWidget(new QLabel(tr("Maximum steps"), central, "maxTimeLabel"),1,0);
    maxTime = new QLineEdit(central, "maxTime");
    connect(maxTime, SIGNAL(returnPressed()),
            this, SLOT(maxTimeProcessText()));
    mainLayout->addWidget(maxTime, 1,1);

    //
    // Create a tab widget so we can split source type and appearance.
    //
    QTabWidget *tabs = new QTabWidget(central, "tabs");
    mainLayout->addMultiCellWidget(tabs, 2,2,0,1);

    //
    // Create a tab for the streamline source widgets.
    //
    QGroupBox *topPageSource = new QGroupBox(central, "topPageSource");
    topPageSource->setFrameStyle(QFrame::NoFrame);
    tabs->addTab(topPageSource, tr("Streamline source"));
    QVBoxLayout *topSourceLayout = new QVBoxLayout(topPageSource);
    topSourceLayout->setMargin(10);
    topSourceLayout->setSpacing(5);

    // Create the source type combo box.
    QHBoxLayout *hLayout = new QHBoxLayout(topSourceLayout);
    hLayout->addWidget(new QLabel(tr("Source type"), topPageSource, "sourceTypeLabel"));
    sourceType = new QComboBox(topPageSource, "sourceType");
    sourceType->insertItem(tr("Point"));
    sourceType->insertItem(tr("Line"));
    sourceType->insertItem(tr("Plane"));
    sourceType->insertItem(tr("Sphere"));
    sourceType->insertItem(tr("Box"));
    connect(sourceType, SIGNAL(activated(int)),
            this, SLOT(sourceTypeChanged(int)));
    hLayout->addWidget(sourceType, 10);
    topSourceLayout->addSpacing(5);

    // Create a group box for the source attributes.
    QGroupBox *pageSource = new QGroupBox(topPageSource, "pageSource");
    sourceAtts = pageSource;
    sourceAtts->setTitle(tr("Point"));
    topSourceLayout->addWidget(pageSource);
    topSourceLayout->addStretch(5);
    QVBoxLayout *svLayout = new QVBoxLayout(pageSource, 10, 2);
    svLayout->addSpacing(10);
    QGridLayout *sLayout = new QGridLayout(svLayout, 16, 2);
//    sLayout->setMargin(10);
    sLayout->setSpacing(5);

    // Create the widgets that specify a point source.
    pointSource = new QLineEdit(pageSource, "pointSource");
    connect(pointSource, SIGNAL(returnPressed()),
            this, SLOT(pointSourceProcessText()));
    pointSourceLabel = new QLabel(pointSource, tr("Location"), pageSource, "pointSourceLabel");
    sLayout->addWidget(pointSourceLabel, 3, 0);
    sLayout->addWidget(pointSource, 3,1);

    // Create the widgets that specify a line source.
    lineStart = new QLineEdit(pageSource, "lineStart");
    connect(lineStart, SIGNAL(returnPressed()),
            this, SLOT(lineStartProcessText()));
    lineStartLabel = new QLabel(lineStart, tr("Start"), pageSource, "lineStartLabel");
    sLayout->addWidget(lineStartLabel,4,0);
    sLayout->addWidget(lineStart, 4,1);

    lineEnd = new QLineEdit(pageSource, "lineEnd");
    connect(lineEnd, SIGNAL(returnPressed()),
            this, SLOT(lineEndProcessText()));
    lineEndLabel = new QLabel(lineEnd, tr("End"), pageSource, "lineEndLabel");
    sLayout->addWidget(lineEndLabel,5,0);
    sLayout->addWidget(lineEnd, 5,1);

    // Create the widgets that specify a plane source.
    planeOrigin = new QLineEdit(pageSource, "planeOrigin");
    connect(planeOrigin, SIGNAL(returnPressed()),
            this, SLOT(planeOriginProcessText()));
    planeOriginLabel = new QLabel(planeOrigin, tr("Origin"), pageSource, "planeOriginLabel");
    sLayout->addWidget(planeOriginLabel,6,0);
    sLayout->addWidget(planeOrigin, 6,1);

    planeNormal = new QLineEdit(pageSource, "planeNormal");
    connect(planeNormal, SIGNAL(returnPressed()),
            this, SLOT(planeNormalProcessText()));
    planeNormalLabel = new QLabel(planeNormal, tr("Normal"), pageSource, "planeNormalLabel");
    sLayout->addWidget(planeNormalLabel,7,0);
    sLayout->addWidget(planeNormal, 7,1);

    planeUpAxis = new QLineEdit(pageSource, "planeUpAxis");
    connect(planeUpAxis, SIGNAL(returnPressed()),
            this, SLOT(planeUpAxisProcessText()));
    planeUpAxisLabel = new QLabel(planeUpAxis, tr("Up axis"), pageSource, "planeUpAxisLabel");
    sLayout->addWidget(planeUpAxisLabel,8,0);
    sLayout->addWidget(planeUpAxis, 8,1);

    planeRadius = new QLineEdit(pageSource, "planeRadius");
    connect(planeRadius, SIGNAL(returnPressed()),
            this, SLOT(planeRadiusProcessText()));
    planeRadiusLabel = new QLabel(planeRadius, tr("Radius"), pageSource, "planeRadiusLabel");
    sLayout->addWidget(planeRadiusLabel,9,0);
    sLayout->addWidget(planeRadius, 9,1);

    // Create the widgets that specify a sphere source.
    sphereOrigin = new QLineEdit(pageSource, "sphereOrigin");
    connect(sphereOrigin, SIGNAL(returnPressed()),
            this, SLOT(sphereOriginProcessText()));
    sphereOriginLabel = new QLabel(sphereOrigin, tr("Origin"), pageSource, "sphereOriginLabel");
    sLayout->addWidget(sphereOriginLabel,10,0);
    sLayout->addWidget(sphereOrigin, 10,1);

    sphereRadius = new QLineEdit(pageSource, "sphereRadius");
    connect(sphereRadius, SIGNAL(returnPressed()),
            this, SLOT(sphereRadiusProcessText()));
    sphereRadiusLabel = new QLabel(sphereRadius, tr("Radius"), pageSource, "sphereRadiusLabel");
    sLayout->addWidget(sphereRadiusLabel,11,0);
    sLayout->addWidget(sphereRadius, 11,1);

    // Create the widgets that specify a box source
    useWholeBox = new QCheckBox(tr("Whole data set"), 
                                pageSource, "useWholeBox");
    connect(useWholeBox, SIGNAL(toggled(bool)),
            this, SLOT(useWholeBoxChanged(bool)));
    sLayout->addWidget(useWholeBox, 12, 0);

    boxExtents[0] = new QLineEdit(pageSource, "boxExtents[0]");
    connect(boxExtents[0], SIGNAL(returnPressed()),
            this, SLOT(boxExtentsProcessText()));
    boxExtentsLabel[0] = new QLabel(boxExtents[0], tr("X Extents"), pageSource, "boxExtentsLabel[0]");
    sLayout->addWidget(boxExtentsLabel[0], 13, 0);
    sLayout->addWidget(boxExtents[0], 13, 1);
    boxExtents[1] = new QLineEdit(pageSource, "boxExtents[1]");
    connect(boxExtents[1], SIGNAL(returnPressed()),
            this, SLOT(boxExtentsProcessText()));
    boxExtentsLabel[1] = new QLabel(boxExtents[1], tr("Y Extents"), pageSource, "boxExtentsLabel[1]");
    sLayout->addWidget(boxExtentsLabel[1], 14, 0);
    sLayout->addWidget(boxExtents[1], 14, 1);
    boxExtents[2] = new QLineEdit(pageSource, "boxExtents[2]");
    connect(boxExtents[2], SIGNAL(returnPressed()),
            this, SLOT(boxExtentsProcessText()));
    boxExtentsLabel[2] = new QLabel(boxExtents[2], tr("Z Extents"), pageSource, "boxExtentsLabel[2]");
    sLayout->addWidget(boxExtentsLabel[2], 15, 0);
    sLayout->addWidget(boxExtents[2], 15, 1);

    //
    // Create appearance-related widgets.
    //
    QGroupBox *pageAppearance = new QGroupBox(central, "pageAppearance");
    pageAppearance->setFrameStyle(QFrame::NoFrame);
    tabs->addTab(pageAppearance, tr("Appearance"));
    QGridLayout *aLayout = new QGridLayout(pageAppearance, 7, 2);
    aLayout->setMargin(10);
    aLayout->setSpacing(5);

    // Create widgets that help determine the appearance of the streamlines.
    displayMethod = new QComboBox(pageAppearance, "displayMethod");
    displayMethod->insertItem(tr("Lines"), 0);
    displayMethod->insertItem(tr("Tubes"), 1);
    displayMethod->insertItem(tr("Ribbons"), 2);
    connect(displayMethod, SIGNAL(activated(int)),
            this, SLOT(displayMethodChanged(int)));
    aLayout->addWidget(new QLabel(displayMethod, tr("Display as"),
        pageAppearance, "displayMethodLabel"), 0,0);
    aLayout->addWidget(displayMethod, 0,1);

    showStart = new QCheckBox(tr("Show start"), pageAppearance, "showStart");
    connect(showStart, SIGNAL(toggled(bool)),
            this, SLOT(showStartChanged(bool)));
    aLayout->addWidget(showStart, 1,1);

    radius = new QLineEdit(pageAppearance, "radius");
    connect(radius, SIGNAL(returnPressed()),
            this, SLOT(radiusProcessText()));
    radiusLabel = new QLabel(radius, tr("Radius"), pageAppearance, "radiusLabel");
    QToolTip::add(radiusLabel, tr("Radius used for tubes and ribbons."));
    aLayout->addWidget(radiusLabel,2,0);
    aLayout->addWidget(radius, 2,1);

    lineWidth = new QvisLineWidthWidget(0, pageAppearance, "lineWidth");
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    lineWidthLabel = new QLabel(lineWidth, tr("Line width"), pageAppearance, "lineWidthLabel");
    aLayout->addWidget(lineWidthLabel,3,0);
    aLayout->addWidget(lineWidth, 3,1);

    coloringMethod = new QComboBox(pageAppearance, "coloringMethod");
    coloringMethod->insertItem(tr("Solid"),0);
    coloringMethod->insertItem(tr("Speed"),1);
    coloringMethod->insertItem(tr("Vorticity magnitude"),2);
    connect(coloringMethod, SIGNAL(activated(int)),
            this, SLOT(coloringMethodChanged(int)));
    aLayout->addWidget(new QLabel(coloringMethod, tr("Color by"), 
        pageAppearance, "colorbylabel"), 4,0);
    aLayout->addWidget(coloringMethod, 4,1);

    colorTableName = new QvisColorTableButton(pageAppearance, "colorTableName");
    connect(colorTableName, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(colorTableNameChanged(bool, const QString&)));
    colorTableNameLabel = new QLabel(colorTableName, tr("Color table"), pageAppearance, "colorTableNameLabel");
    aLayout->addWidget(colorTableNameLabel,5,0);
    aLayout->addWidget(colorTableName, 5,1, Qt::AlignLeft);

    singleColor = new QvisColorButton(pageAppearance, "singleColor");
    connect(singleColor, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(singleColorChanged(const QColor&)));
    singleColorLabel = new QLabel(singleColor, tr("Single color"), pageAppearance, "singleColorLabel");
    aLayout->addWidget(singleColorLabel,6,0);
    aLayout->addWidget(singleColor, 6,1, Qt::AlignLeft);

    //
    // Create the widget that lets the user set the point density.
    //
    mainLayout->addWidget(new QLabel(tr("Point density"), central, "pointDensityLabel"),3,0);
    pointDensity = new QSpinBox(1, 30, 1, central, "pointDensity");
    connect(pointDensity, SIGNAL(valueChanged(int)), 
            this, SLOT(pointDensityChanged(int)));
    mainLayout->addWidget(pointDensity, 3,1);

    //Create the direction of integration.
    mainLayout->addWidget(new QLabel(tr("Streamline direction"), central, "streamlineDirectionLabel"),4,0);
    directionType = new QComboBox(central, "directionType");
    directionType->insertItem(tr("Forward"));
    directionType->insertItem(tr("Backward"));
    directionType->insertItem(tr("Both"));
    connect(directionType, SIGNAL(activated(int)),
            this, SLOT(directionTypeChanged(int)));
    mainLayout->addWidget(directionType, 4,1);

    legendFlag = new QCheckBox(tr("Legend"), central, "legendFlag");
    connect(legendFlag, SIGNAL(toggled(bool)),
            this, SLOT(legendFlagChanged(bool)));
    mainLayout->addWidget(legendFlag, 5,0);

    lightingFlag = new QCheckBox(tr("Lighting"), central, "lightingFlag");
    connect(lightingFlag, SIGNAL(toggled(bool)),
            this, SLOT(lightingFlagChanged(bool)));
    mainLayout->addWidget(lightingFlag, 5,1);

}

// ****************************************************************************
// Method: QvisStreamlinePlotWindow::ProcessOldVersions
//
// Purpose: 
//   Allows modification of the data node before it is used.
//
// Arguments:
//   parentNode    : The node that's a parent node of the node that is used
//                   by this window.
//   configVersion : The version of the config file.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 3 11:24:24 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisStreamlinePlotWindow::ProcessOldVersions(DataNode *parentNode,
    const char *configVersion)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("Streamline plot attributes");
    if(searchNode == 0)
        return;

    // Remove height/width information if the config file is older than
    // 1.4.1 since this window changed size quite a bit in version 1.4.1.
    if (StreamlineAttributes::VersionLessThan(configVersion, "1.4.1"))
    {
        searchNode->RemoveNode("width");
        searchNode->RemoveNode("height");
    }
}

// ****************************************************************************
// Method: QvisStreamlinePlotWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Brad Whitlock, Wed Dec 22 13:10:59 PST 2004
//   I added support for coloring by vorticity and for showing ribbons.
//
//   Hank Childs, Sat Mar  3 09:11:44 PST 2007
//   Added support for useWholeBox.
//
//   Dave Pugmire, Thu Nov 15 12:09:08 EST 2007
//   Add streamline direction option.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::UpdateWindow(bool doAll)
{
    QString       temp;
    const double *dptr;
    QColor        tempcolor;

    for(int i = 0; i < streamAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!streamAtts->IsSelected(i))
            {
                continue;
            }
        }

        switch(i)
        {
        case 0: // sourceType
            // Update lots of widget visibility and enabled states.
            UpdateSourceAttributes();

            sourceType->blockSignals(true);
            sourceType->setCurrentItem(streamAtts->GetSourceType());
            sourceType->blockSignals(false);
            break;
        case 1: // stepLength
            temp.setNum(streamAtts->GetStepLength());
            stepLength->setText(temp);
            break;
        case 2: // maxTime
            temp.setNum(streamAtts->GetMaxTime());
            maxTime->setText(temp);
            break;
        case 3: // pointSource
            dptr = streamAtts->GetPointSource();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            pointSource->setText(temp);
            break;
        case 4: // lineStart
            dptr = streamAtts->GetLineStart();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            lineStart->setText(temp);
            break;
        case 5: // lineEnd
            dptr = streamAtts->GetLineEnd();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            lineEnd->setText(temp);
            break;
        case 6: // planeOrigin
            dptr = streamAtts->GetPlaneOrigin();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            planeOrigin->setText(temp);
            break;
        case 7: // planeNormal
            dptr = streamAtts->GetPlaneNormal();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            planeNormal->setText(temp);
            break;
        case 8: // planeUpAxis
            dptr = streamAtts->GetPlaneUpAxis();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            planeUpAxis->setText(temp);
            break;
        case 9: // planeRadius
            temp.setNum(streamAtts->GetPlaneRadius());
            planeRadius->setText(temp);
            break;
        case 10: // sphereOrigin
            dptr = streamAtts->GetSphereOrigin();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            sphereOrigin->setText(temp);
            break;
        case 11: // sphereRadius
            temp.setNum(streamAtts->GetSphereRadius());
            sphereRadius->setText(temp);
            break;
        case 12: // boxExtents
            temp.sprintf("%g %g", streamAtts->GetBoxExtents()[0],
                streamAtts->GetBoxExtents()[1]);
            boxExtents[0]->setText(temp);
            temp.sprintf("%g %g", streamAtts->GetBoxExtents()[2],
                streamAtts->GetBoxExtents()[3]);
            boxExtents[1]->setText(temp);
            temp.sprintf("%g %g", streamAtts->GetBoxExtents()[4],
                streamAtts->GetBoxExtents()[5]);
            boxExtents[2]->setText(temp);
            break;
        case 13: // useWholeBox
            useWholeBox->blockSignals(true);
            useWholeBox->setChecked(streamAtts->GetUseWholeBox());
            if (streamAtts->GetUseWholeBox())
            {
                boxExtents[0]->setEnabled(false);
                boxExtents[1]->setEnabled(false);
                boxExtents[2]->setEnabled(false);
                boxExtentsLabel[0]->setEnabled(false);
                boxExtentsLabel[1]->setEnabled(false);
                boxExtentsLabel[2]->setEnabled(false);
            }
            else
            {
                boxExtents[0]->setEnabled(true);
                boxExtents[1]->setEnabled(true);
                boxExtents[2]->setEnabled(true);
                boxExtentsLabel[0]->setEnabled(true);
                boxExtentsLabel[1]->setEnabled(true);
                boxExtentsLabel[2]->setEnabled(true);
            }
            useWholeBox->blockSignals(false);
            break;
        case 14: // pointDensity
            pointDensity->blockSignals(true);
            pointDensity->setValue(streamAtts->GetPointDensity());
            pointDensity->blockSignals(false);
            break;
        case 15: // displayMethod
            { // new scope
            bool showLines = streamAtts->GetDisplayMethod() == 
                StreamlineAttributes::Lines;
            bool showTubes = streamAtts->GetDisplayMethod() == 
                StreamlineAttributes::Tubes;
            bool showRibbons = streamAtts->GetDisplayMethod() == 
                StreamlineAttributes::Ribbons;
            showStart->setEnabled(showTubes);
            radius->setEnabled(showTubes || showRibbons);
            radiusLabel->setEnabled(showTubes || showRibbons);
            lineWidth->setEnabled(showLines);
            lineWidthLabel->setEnabled(showLines);

            displayMethod->blockSignals(true);
            displayMethod->setCurrentItem(int(streamAtts->GetDisplayMethod()));
            displayMethod->blockSignals(false);
            }
            break;
        case 16: // showStart
            showStart->blockSignals(true);
            showStart->setChecked(streamAtts->GetShowStart());
            showStart->blockSignals(false);
            break;
        case 17: // radius
            temp.setNum(streamAtts->GetRadius());
            radius->setText(temp);
            break;
        case 18: // lineWidth
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(streamAtts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
        case 19: // coloringMethod
            {// New scope
            bool needCT = streamAtts->GetColoringMethod() != StreamlineAttributes::Solid;
            colorTableName->setEnabled(needCT);
            colorTableNameLabel->setEnabled(needCT);
            singleColor->setEnabled(!needCT);
            singleColorLabel->setEnabled(!needCT);

            coloringMethod->blockSignals(true);
            coloringMethod->setCurrentItem(int(streamAtts->GetColoringMethod()));
            coloringMethod->blockSignals(false);
            }
            break;
        case 20: // colorTableName
            colorTableName->setColorTable(streamAtts->GetColorTableName().c_str());
            break;
        case 21: // singleColor
            tempcolor = QColor(streamAtts->GetSingleColor().Red(),
                               streamAtts->GetSingleColor().Green(),
                               streamAtts->GetSingleColor().Blue());
            singleColor->setButtonColor(tempcolor);
            break;
        case 22: // legendFlag
            legendFlag->blockSignals(true);
            legendFlag->setChecked(streamAtts->GetLegendFlag());
            legendFlag->blockSignals(false);
            break;
        case 23: // lightingFlag
            lightingFlag->blockSignals(true);
            lightingFlag->setChecked(streamAtts->GetLightingFlag());
            lightingFlag->blockSignals(false);
            break;
        case 24: // streamline direction.

            directionType->blockSignals(true);
	    directionType->setCurrentItem( int(streamAtts->GetStreamlineDirection()) );
	    directionType->blockSignals(false);

	    break;
        }
    }
}

// ****************************************************************************
// Method: QvisStreamlinePlotWindow::UpdateSourceAttributes
//
// Purpose: 
//   Updates the widgets for the various stream source types.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 17:22:05 PST 2002
//
// Modifications:
//   
//   Hank Childs, Sat Mar  3 09:11:44 PST 2007
//   Hide/show useWholeBox.  Also enable/disable pointDensity if source type
//   is point.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::UpdateSourceAttributes()
{
    bool usePoint =  streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedPoint;
    bool useLine  =  streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedLine;
    bool usePlane =  streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedPlane;
    bool useSphere =  streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedSphere;
    bool useBox =  streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedBox;

    //
    // Update the point widgets.
    //
    pointSource->setEnabled(usePoint);
    pointSourceLabel->setEnabled(usePoint);
    if(usePoint)
    {
        sourceAtts->setTitle(tr("Point"));
        pointSource->show();
        pointSourceLabel->show();
        pointDensity->setEnabled(false);
    }
    else
    {
        pointSource->hide();
        pointSourceLabel->hide();
        pointDensity->setEnabled(true);
    }

    //
    // Update the line widgets.
    //
    lineStart->setEnabled(useLine);
    lineEnd->setEnabled(useLine);
    lineStartLabel->setEnabled(useLine);
    lineEndLabel->setEnabled(useLine);
    if(useLine)
    {
        sourceAtts->setTitle(tr("Line"));
        lineStart->show();
        lineEnd->show();
        lineStartLabel->show();
        lineEndLabel->show();
    }
    else
    {
        lineStart->hide();
        lineEnd->hide();
        lineStartLabel->hide();
        lineEndLabel->hide();
    }

    //
    // Update the plane widgets.
    //
    planeOrigin->setEnabled(usePlane);
    planeNormal->setEnabled(usePlane);
    planeUpAxis->setEnabled(usePlane);
    planeRadius->setEnabled(usePlane);
    planeOriginLabel->setEnabled(usePlane);
    planeNormalLabel->setEnabled(usePlane);
    planeUpAxisLabel->setEnabled(usePlane);
    planeRadiusLabel->setEnabled(usePlane);
    if(usePlane)
    {
        sourceAtts->setTitle(tr("Plane"));
        planeOrigin->show();
        planeNormal->show();
        planeUpAxis->show();
        planeRadius->show();
        planeOriginLabel->show();
        planeNormalLabel->show();
        planeUpAxisLabel->show();
        planeRadiusLabel->show();
    }
    else
    {
        planeOrigin->hide();
        planeNormal->hide();
        planeUpAxis->hide();
        planeRadius->hide();
        planeOriginLabel->hide();
        planeNormalLabel->hide();
        planeUpAxisLabel->hide();
        planeRadiusLabel->hide();
    }

    //
    // Update the sphere widgets.
    //
    sphereOrigin->setEnabled(useSphere);
    sphereRadius->setEnabled(useSphere);
    sphereOriginLabel->setEnabled(useSphere);
    sphereRadiusLabel->setEnabled(useSphere);
    if(useSphere)
    {
        sourceAtts->setTitle(tr("Sphere"));
        sphereOrigin->show();
        sphereRadius->show();
        sphereOriginLabel->show();
        sphereRadiusLabel->show();
    }
    else
    {
        sphereOrigin->hide();
        sphereRadius->hide();
        sphereOriginLabel->hide();
        sphereRadiusLabel->hide();
    }

    //
    // Update the box widgets
    //
    if(useBox)
    {
        sourceAtts->setTitle(tr("Box"));
        useWholeBox->show();
    }
    else
        useWholeBox->hide();
    for(int i = 0; i < 3; ++i)
    {
        boxExtents[i]->setEnabled(useBox);
        boxExtentsLabel[i]->setEnabled(useBox);
        if(useBox)
        {
            boxExtents[i]->show();
            boxExtentsLabel[i]->show();
            if (streamAtts->GetUseWholeBox())
            {
                boxExtents[i]->setEnabled(false);
                boxExtentsLabel[i]->setEnabled(false);
            }
        }
        else
        {
            boxExtents[i]->hide();
            boxExtentsLabel[i]->hide();
        }
    }
}


// ****************************************************************************
// Method: QvisStreamlinePlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Dec 22 14:47:44 PST 2004
//   Changed tubeRadius to radius.
//
//   Hank Childs, Sat Mar  3 09:11:44 PST 2007
//   Added support for useWholeBox and changed numbers.  Also fix a bug where
//   the point density is not getting updated correctly if you don't click
//   return.
//
//   Brad Whitlock, Wed Apr 23 11:49:55 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do stepLength
    if(which_widget == 1 || doAll)
    {
        temp = stepLength->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                streamAtts->SetStepLength(val);
        }

        if(!okay)
        {
            msg = tr("The value of stepLength was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(streamAtts->GetStepLength());
            Message(msg);
            streamAtts->SetStepLength(streamAtts->GetStepLength());
        }
    }

    // Do maxTime
    if(which_widget == 2 || doAll)
    {
        temp = maxTime->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                streamAtts->SetMaxTime(val);
        }

        if(!okay)
        {
            msg = tr("The value of maxTime was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(streamAtts->GetMaxTime());
            Message(msg);
            streamAtts->SetMaxTime(streamAtts->GetMaxTime());
        }
    }

    // Do pointSource
    if(which_widget == 3 || doAll)
    {
        temp = pointSource->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            if((okay = (sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]) == 3)) == true)
                streamAtts->SetPointSource(val);
        }

        if(!okay)
        {
            const double *val = streamAtts->GetPointSource();
            QString values; values.sprintf("<%g %g %g>", val[0], val[1], val[2]);
            msg = tr("The value of pointSource was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(values);
            Message(msg);
            streamAtts->SetPointSource(streamAtts->GetPointSource());
        }
    }

    // Do lineStart
    if(which_widget == 4 || doAll)
    {
        temp = lineStart->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            if((okay = (sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]) == 3)) == true)
                streamAtts->SetLineStart(val);
        }

        if(!okay)
        {
            const double *val = streamAtts->GetLineStart();
            QString values; values.sprintf("<%g %g %g>", val[0], val[1], val[2]);
            msg = tr("The value of lineStart was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(values);
            Message(msg);
            streamAtts->SetLineStart(streamAtts->GetLineStart());
        }
    }

    // Do lineEnd
    if(which_widget == 5 || doAll)
    {
        temp = lineEnd->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            if((okay = (sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]) == 3)) == true)
                streamAtts->SetLineEnd(val);
        }

        if(!okay)
        {
            const double *val = streamAtts->GetLineEnd();
            QString values; values.sprintf("<%g %g %g>", val[0], val[1], val[2]);
            msg = tr("The value of lineEnd was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(values);
            Message(msg);
            streamAtts->SetLineEnd(streamAtts->GetLineEnd());
        }
    }

    // Do planeOrigin
    if(which_widget == 6 || doAll)
    {
        temp = planeOrigin->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            if((okay = (sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]) == 3)) == true)
                streamAtts->SetPlaneOrigin(val);
        }

        if(!okay)
        {
            const double *val = streamAtts->GetPlaneOrigin();
            QString values; values.sprintf("<%g %g %g>", val[0], val[1], val[2]);
            msg = tr("The value of planeOrigin was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(values);
            Message(msg);
            streamAtts->SetPlaneOrigin(streamAtts->GetPlaneOrigin());
        }
    }

    // Do planeNormal
    if(which_widget == 7 || doAll)
    {
        temp = planeNormal->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            if((okay = (sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]) == 3)) == true)
                streamAtts->SetPlaneNormal(val);
        }

        if(!okay)
        {
            const double *val = streamAtts->GetPlaneNormal();
            QString values; values.sprintf("<%g %g %g>", val[0], val[1], val[2]);
            msg = tr("The value of planeNormal was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(values);
            Message(msg);
            streamAtts->SetPlaneNormal(streamAtts->GetPlaneNormal());
        }
    }

    // Do planeUpAxis
    if(which_widget == 8 || doAll)
    {
        temp = planeUpAxis->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            if((okay = (sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]) == 3)) == true)
                streamAtts->SetPlaneUpAxis(val);
        }

        if(!okay)
        {
            const double *val = streamAtts->GetPlaneUpAxis();
            QString values; values.sprintf("<%g %g %g>", val[0], val[1], val[2]);
            msg = tr("The value of planeUpAxis was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(values);
            Message(msg);
            streamAtts->SetPlaneUpAxis(streamAtts->GetPlaneUpAxis());
        }
    }

    // Do planeRadius
    if(which_widget == 9 || doAll)
    {
        temp = planeRadius->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                streamAtts->SetPlaneRadius(val);
        }

        if(!okay)
        {
            msg = tr("The value of planeRadius was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(streamAtts->GetPlaneRadius());
            Message(msg);
            streamAtts->SetPlaneRadius(streamAtts->GetPlaneRadius());
        }
    }

    // Do sphereOrigin
    if(which_widget == 10 || doAll)
    {
        temp = sphereOrigin->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val[3];
            if((okay = (sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]) == 3)) == true)
                streamAtts->SetSphereOrigin(val);
        }

        if(!okay)
        {
            const double *val = streamAtts->GetSphereOrigin();
            QString values; values.sprintf("<%g %g %g>", val[0], val[1], val[2]);
            msg = tr("The value of sphereOrigin was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(values);
            Message(msg);
            streamAtts->SetSphereOrigin(streamAtts->GetSphereOrigin());
        }
    }

    // Do sphereRadius
    if(which_widget == 11 || doAll)
    {
        temp = sphereRadius->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                streamAtts->SetSphereRadius(val);
        }

        if(!okay)
        {
            msg = tr("The value of sphereRadius was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(streamAtts->GetSphereRadius());
            Message(msg);
            streamAtts->SetSphereRadius(streamAtts->GetSphereRadius());
        }
    }

    // Do boxExtents
    if(which_widget == 12 || doAll)
    {
        double d[6];
        bool allOkay = true;
        for(int i = 0; i < 3; ++i)
        {
            temp = boxExtents[i]->displayText().simplifyWhiteSpace();
            okay = !temp.isEmpty();
            if(okay)
            {
                okay = (sscanf(temp.latin1(), "%lg %lg", &d[i*2], &d[i*2+1]) == 2);
            }

            allOkay &= okay;
        }

        if(!allOkay)
        {
            Message(tr("The box extents contained errors so the previous "
                       "values will be used."));
            streamAtts->SelectBoxExtents();
        }
        else
            streamAtts->SetBoxExtents(d);
    }

    // pointDensity
    if (which_widget == 14 || doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = pointDensity->value();
        if (val >= 1)
            streamAtts->SetPointDensity(val);
    }

    // Do radius
    if(which_widget == 17 || doAll)
    {
        temp = radius->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            streamAtts->SetRadius(val);
        }

        if(!okay)
        {
            msg = tr("The value of radius was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(streamAtts->GetRadius());
            Message(msg);
            streamAtts->SetRadius(streamAtts->GetRadius());
        }
    }
}


// ****************************************************************************
// Method: QvisStreamlinePlotWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisStreamlinePlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        streamAtts->Notify();

        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        streamAtts->Notify();
}


//
// Qt Slot functions
//

// ****************************************************************************
// Method: QvisStreamlinePlotWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisStreamlinePlotWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisStreamlinePlotWindow::makeDefault
//
// Purpose: 
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisStreamlinePlotWindow::makeDefault()
{
    GetCurrentValues(-1);
    streamAtts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisStreamlinePlotWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:19:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisStreamlinePlotWindow::reset()
{
    GetViewerMethods()->ResetPlotOptions(plotType);
}


void
QvisStreamlinePlotWindow::sourceTypeChanged(int val)
{
    if(val != streamAtts->GetSourceType())
    {
        streamAtts->SetSourceType(StreamlineAttributes::SourceType(val));
        Apply();
    }
}

void
QvisStreamlinePlotWindow::directionTypeChanged(int val)
 {
    if(val != streamAtts->GetStreamlineDirection())
    {
        streamAtts->SetStreamlineDirection(StreamlineAttributes::IntegrationDirection(val));
        Apply();
    }
}   

void
QvisStreamlinePlotWindow::stepLengthProcessText()
{
    GetCurrentValues(1);
    Apply();
}

void
QvisStreamlinePlotWindow::maxTimeProcessText()
{
    GetCurrentValues(2);
    Apply();
}

void
QvisStreamlinePlotWindow::pointSourceProcessText()
{
    GetCurrentValues(3);
    Apply();
}

void
QvisStreamlinePlotWindow::lineStartProcessText()
{
    GetCurrentValues(4);
    Apply();
}

void
QvisStreamlinePlotWindow::lineEndProcessText()
{
    GetCurrentValues(5);
    Apply();
}

void
QvisStreamlinePlotWindow::planeOriginProcessText()
{
    GetCurrentValues(6);
    Apply();
}

void
QvisStreamlinePlotWindow::planeNormalProcessText()
{
    GetCurrentValues(7);
    Apply();
}

void
QvisStreamlinePlotWindow::planeUpAxisProcessText()
{
    GetCurrentValues(8);
    Apply();
}

void
QvisStreamlinePlotWindow::planeRadiusProcessText()
{
    GetCurrentValues(9);
    Apply();
}

void
QvisStreamlinePlotWindow::sphereOriginProcessText()
{
    GetCurrentValues(10);
    Apply();
}

void
QvisStreamlinePlotWindow::sphereRadiusProcessText()
{
    GetCurrentValues(11);
    Apply();
}

void
QvisStreamlinePlotWindow::pointDensityChanged(int val)
{
    streamAtts->SetPointDensity(val);
    Apply();
}

void
QvisStreamlinePlotWindow::displayMethodChanged(int val)
{
    streamAtts->SetDisplayMethod((StreamlineAttributes::DisplayMethod)val);
    Apply();
}

void
QvisStreamlinePlotWindow::showStartChanged(bool val)
{
    streamAtts->SetShowStart(val);
    SetUpdate(false);
    Apply();
}

void
QvisStreamlinePlotWindow::radiusProcessText()
{
    GetCurrentValues(16);
    Apply();
}

void
QvisStreamlinePlotWindow::useWholeBoxChanged(bool val)
{
    streamAtts->SetUseWholeBox(val);
    Apply();
}


void
QvisStreamlinePlotWindow::boxExtentsProcessText()
{
    GetCurrentValues(12);
    Apply();
}

void
QvisStreamlinePlotWindow::lineWidthChanged(int style)
{
    streamAtts->SetLineWidth(style);
    SetUpdate(false);
    Apply();
}

void
QvisStreamlinePlotWindow::coloringMethodChanged(int val)
{
    streamAtts->SetColoringMethod((StreamlineAttributes::ColoringMethod)val);
    Apply();
}

void
QvisStreamlinePlotWindow::colorTableNameChanged(bool useDefault, const QString &ctName)
{
    streamAtts->SetColorTableName(ctName.latin1());
    SetUpdate(false);
    Apply();
}

void
QvisStreamlinePlotWindow::singleColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    streamAtts->SetSingleColor(temp);
    SetUpdate(false);
    Apply();
}

void
QvisStreamlinePlotWindow::legendFlagChanged(bool val)
{
    streamAtts->SetLegendFlag(val);
    SetUpdate(false);
    Apply();
}

void
QvisStreamlinePlotWindow::lightingFlagChanged(bool val)
{
    streamAtts->SetLightingFlag(val);
    SetUpdate(false);
    Apply();
}


