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

#include "QvisStreamlinePlotWindow.h"

#include <StreamlineAttributes.h>
#include <ViewerProxy.h>
#include <DataNode.h>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QTabWidget>
#include <QToolTip>
#include <QvisColorTableButton.h>
#include <QvisColorButton.h>
#include <QvisLineWidthWidget.h>
#include <QvisVariableButton.h>
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
                         const QString &caption,
                         const QString &shortName,
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
//   Dave Pugmire, Mon Aug 4 2:49:38 EDT 2008
//   Added termination, algorithm and integration options.
//
//   Brad Whitlock, Wed Aug  6 10:20:49 PDT 2008
//   Qt 4.
//
//   Dave Pugmire, Wed Aug 6 15:16:23 EST 2008
//   Add accurate distance calculate option.
//
//   Dave Pugmire, Wed Aug 13 12:56:11 EST 2008
//   Changed label text for streamline algorithms.
//
//   Dave Pugmire, Tue Aug 19 17:18:03 EST 2008
//   Removed the accurate distance calculation option.
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Add new coloring methods, length, time and ID.
//
//   Sean Ahern, Wed Sep  3 16:19:27 EDT 2008
//   Mucked with the layout to make it a bit more consistent.
//
//   Dave Pugmire, Tue Oct 7 08:17:22 EDT 2008
//   Changed 'Termination Criteria' to 'Termination Criterion'
//
//   Dave Pugmire, Thu Feb  5 12:20:15 EST 2009
//   Added workGroupSize for the masterSlave algorithm.
//
//   Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//   Added number of steps as a termination criterion.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Add pathline GUI.
//    
//   Hank Childs, Sat May  2 22:14:38 PDT 2009
//   Add support for point lists.
//
//   Dave Pugmire, Wed Jun 10 16:26:25 EDT 2009
//   Add color by variable.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(0);
    topLayout->addLayout(mainLayout);
    mainLayout->setMargin(0);

    // Create the maximum time text field.
    mainLayout->addWidget(new QLabel(tr("Termination criteria"), central),0,0);
    termType = new QComboBox(central);
    termType->addItem(tr("Distance"));
    termType->addItem(tr("Time"));
    termType->addItem(tr("Number of Steps"));
    connect(termType, SIGNAL(activated(int)),
            this, SLOT(termTypeChanged(int)));
    mainLayout->addWidget(termType, 1,0);

    termination = new QLineEdit(central);
    connect(termination, SIGNAL(returnPressed()),
            this, SLOT(terminationProcessText()));
    mainLayout->addWidget(termination, 1,1);

    //Create the direction of integration.
    mainLayout->addWidget(new QLabel(tr("Streamline direction"), central),3,0);
    directionType = new QComboBox(central);
    directionType->addItem(tr("Forward"));
    directionType->addItem(tr("Backward"));
    directionType->addItem(tr("Both"));
    connect(directionType, SIGNAL(activated(int)),
            this, SLOT(directionTypeChanged(int)));
    mainLayout->addWidget(directionType, 3,1);

    /*pathlineFlag = new QCheckBox(tr("Pathlines"), central);
    connect(pathlineFlag, SIGNAL(toggled(bool)),
            this, SLOT(pathlineFlagChanged(bool)));
    mainLayout->addWidget(pathlineFlag, 4,0);
    */

    // Add some space....
    mainLayout->addWidget(new QLabel(tr(""), central), 5,0);

    //
    // Create a tab widget so we can split source type and appearance.
    //
    QTabWidget *tabs = new QTabWidget(central);
    mainLayout->addWidget(tabs, 6, 0, 1, 2);

    //
    // Create a tab for the streamline source widgets.
    //
    QWidget *topPageSource = new QWidget(central);
    tabs->addTab(topPageSource, tr("Source"));
    QGridLayout *hLayout = new QGridLayout(topPageSource);
    hLayout->setMargin(10);
    hLayout->setColumnStretch(1,10);

    // Create the source type combo box.
    hLayout->addWidget(new QLabel(tr("Source type"), topPageSource), 0,0);
    sourceType = new QComboBox(topPageSource);
    sourceType->addItem(tr("Single Point"));
    sourceType->addItem(tr("Line"));
    sourceType->addItem(tr("Plane"));
    sourceType->addItem(tr("Sphere"));
    sourceType->addItem(tr("Box"));
    sourceType->addItem(tr("Point List"));
    connect(sourceType, SIGNAL(activated(int)),
            this, SLOT(sourceTypeChanged(int)));
    hLayout->addWidget(sourceType, 0,1);

    //
    // Create the widget that lets the user set the point density.
    //
    hLayout->addWidget(new QLabel(tr("Point density"), topPageSource), 1,0);
    pointDensity = new QSpinBox(topPageSource);
    pointDensity->setMinimum(1);
    pointDensity->setMaximum(1000);
    connect(pointDensity, SIGNAL(valueChanged(int)), 
            this, SLOT(pointDensityChanged(int)));
    hLayout->addWidget(pointDensity,1,1);

    // Create a group box for the source attributes.
    QGroupBox *pageSource = new QGroupBox(topPageSource);
    sourceAtts = pageSource;
    sourceAtts->setTitle(tr("Point"));
    hLayout->addWidget(pageSource,2,0,1,2);
    QVBoxLayout *sTopLayout = new QVBoxLayout(pageSource);
    QGridLayout *sLayout = new QGridLayout(0);
    sTopLayout->addLayout(sLayout);
    sTopLayout->addStretch(10);
    sLayout->setSpacing(5);
    sLayout->setMargin(0);

    // Create the widgets that specify a point source.
    pointSource = new QLineEdit(pageSource);
    connect(pointSource, SIGNAL(returnPressed()),
            this, SLOT(pointSourceProcessText()));
    pointSourceLabel = new QLabel(tr("Location"), pageSource);
    pointSourceLabel->setBuddy(pointSource);
    sLayout->addWidget(pointSourceLabel, 3, 0);
    sLayout->addWidget(pointSource, 3,1);

    // Create the widgets that specify a line source.
    lineStart = new QLineEdit(pageSource);
    connect(lineStart, SIGNAL(returnPressed()),
            this, SLOT(lineStartProcessText()));
    lineStartLabel = new QLabel(tr("Start"), pageSource);
    lineStartLabel->setBuddy(lineStart);
    sLayout->addWidget(lineStartLabel,4,0);
    sLayout->addWidget(lineStart, 4,1);

    lineEnd = new QLineEdit(pageSource);
    connect(lineEnd, SIGNAL(returnPressed()),
            this, SLOT(lineEndProcessText()));
    lineEndLabel = new QLabel(tr("End"), pageSource);
    lineEndLabel->setBuddy(lineEnd);
    sLayout->addWidget(lineEndLabel,5,0);
    sLayout->addWidget(lineEnd, 5,1);

    // Create the widgets that specify a plane source.
    planeOrigin = new QLineEdit(pageSource);
    connect(planeOrigin, SIGNAL(returnPressed()),
            this, SLOT(planeOriginProcessText()));
    planeOriginLabel = new QLabel(tr("Origin"), pageSource);
    planeOriginLabel->setBuddy(planeOrigin);
    sLayout->addWidget(planeOriginLabel,6,0);
    sLayout->addWidget(planeOrigin, 6,1);

    planeNormal = new QLineEdit(pageSource);
    connect(planeNormal, SIGNAL(returnPressed()),
            this, SLOT(planeNormalProcessText()));
    planeNormalLabel = new QLabel(tr("Normal"), pageSource);
    planeNormalLabel->setBuddy(planeNormal);
    sLayout->addWidget(planeNormalLabel,7,0);
    sLayout->addWidget(planeNormal, 7,1);

    planeUpAxis = new QLineEdit(pageSource);
    connect(planeUpAxis, SIGNAL(returnPressed()),
            this, SLOT(planeUpAxisProcessText()));
    planeUpAxisLabel = new QLabel(tr("Up axis"), pageSource);
    planeUpAxisLabel->setBuddy(planeUpAxis);
    sLayout->addWidget(planeUpAxisLabel,8,0);
    sLayout->addWidget(planeUpAxis, 8,1);

    planeRadius = new QLineEdit(pageSource);
    connect(planeRadius, SIGNAL(returnPressed()),
            this, SLOT(planeRadiusProcessText()));
    planeRadiusLabel = new QLabel(tr("Radius"), pageSource);
    planeRadiusLabel->setBuddy(planeRadius);
    sLayout->addWidget(planeRadiusLabel,9,0);
    sLayout->addWidget(planeRadius, 9,1);

    // Create the widgets that specify a sphere source.
    sphereOrigin = new QLineEdit(pageSource);
    connect(sphereOrigin, SIGNAL(returnPressed()),
            this, SLOT(sphereOriginProcessText()));
    sphereOriginLabel = new QLabel(tr("Origin"), pageSource);
    sphereOriginLabel->setBuddy(sphereOrigin);
    sLayout->addWidget(sphereOriginLabel,10,0);
    sLayout->addWidget(sphereOrigin, 10,1);

    sphereRadius = new QLineEdit(pageSource);
    connect(sphereRadius, SIGNAL(returnPressed()),
            this, SLOT(sphereRadiusProcessText()));
    sphereRadiusLabel = new QLabel(tr("Radius"), pageSource);
    sphereRadiusLabel->setBuddy(sphereRadius);
    sLayout->addWidget(sphereRadiusLabel,11,0);
    sLayout->addWidget(sphereRadius, 11,1);

    // Create the widgets that specify a box source
    useWholeBox = new QCheckBox(tr("Whole data set"), pageSource);
    connect(useWholeBox, SIGNAL(toggled(bool)),
            this, SLOT(useWholeBoxChanged(bool)));
    sLayout->addWidget(useWholeBox, 12, 0);

    boxExtents[0] = new QLineEdit(pageSource);
    connect(boxExtents[0], SIGNAL(returnPressed()),
            this, SLOT(boxExtentsProcessText()));
    boxExtentsLabel[0] = new QLabel(tr("X Extents"), pageSource);
    boxExtentsLabel[0]->setBuddy(boxExtents[0]);
    sLayout->addWidget(boxExtentsLabel[0], 13, 0);
    sLayout->addWidget(boxExtents[0], 13, 1);
    boxExtents[1] = new QLineEdit(pageSource);
    connect(boxExtents[1], SIGNAL(returnPressed()),
            this, SLOT(boxExtentsProcessText()));
    boxExtentsLabel[1] = new QLabel(tr("Y Extents"), pageSource);
    boxExtentsLabel[1]->setBuddy(boxExtents[1]);
    sLayout->addWidget(boxExtentsLabel[1], 14, 0);
    sLayout->addWidget(boxExtents[1], 14, 1);
    boxExtents[2] = new QLineEdit(pageSource);
    connect(boxExtents[2], SIGNAL(returnPressed()),
            this, SLOT(boxExtentsProcessText()));
    boxExtentsLabel[2] = new QLabel(tr("Z Extents"), pageSource);
    boxExtentsLabel[2]->setBuddy(boxExtents[2]);
    sLayout->addWidget(boxExtentsLabel[2], 15, 0);
    sLayout->addWidget(boxExtents[2], 15, 1);

    // Create the widgets that specify a point list.
    pointList = new QLineEdit(pageSource);
    connect(pointList, SIGNAL(returnPressed()),
            this, SLOT(pointListProcessText()));
    pointListLabel = new QLabel(tr("Format as \"X1 Y1 Z1 X2 Y2 Z2 ...\".  For 2D, use 0 for Z."), pageSource);
    pointListLabel->setBuddy(pointList);
    sLayout->addWidget(pointListLabel,16,0);
    sLayout->addWidget(pointList, 17,0);

    //
    // Create appearance-related widgets.
    //
    QWidget *pageAppearance = new QWidget(central);
    tabs->addTab(pageAppearance, tr("Appearance"));
    QGridLayout *aLayout = new QGridLayout(pageAppearance);
    aLayout->setMargin(10);
    aLayout->setSpacing(5);

    // Create widgets that help determine the appearance of the streamlines.
    displayMethod = new QComboBox(pageAppearance);
    displayMethod->addItem(tr("Lines"), 0);
    displayMethod->addItem(tr("Tubes"), 1);
    displayMethod->addItem(tr("Ribbons"), 2);
    connect(displayMethod, SIGNAL(activated(int)),
            this, SLOT(displayMethodChanged(int)));
    aLayout->addWidget(new QLabel(tr("Display as"), pageAppearance), 0,0);
    aLayout->addWidget(displayMethod, 0,1);

    showStart = new QCheckBox(tr("Show start"), pageAppearance);
    connect(showStart, SIGNAL(toggled(bool)),
            this, SLOT(showStartChanged(bool)));
    aLayout->addWidget(showStart, 1,1);

    radius = new QLineEdit(pageAppearance);
    connect(radius, SIGNAL(returnPressed()),
            this, SLOT(radiusProcessText()));
    radiusLabel = new QLabel(tr("Radius"), pageAppearance);
    radiusLabel->setBuddy(radius);
    radiusLabel->setToolTip(tr("Radius used for tubes and ribbons."));
    aLayout->addWidget(radiusLabel,2,0);
    aLayout->addWidget(radius, 2,1);

    lineWidth = new QvisLineWidthWidget(0, pageAppearance);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    lineWidthLabel = new QLabel(tr("Line width"), pageAppearance);
    lineWidthLabel->setBuddy(lineWidth);
    aLayout->addWidget(lineWidthLabel,3,0);
    aLayout->addWidget(lineWidth, 3,1);

    coloringMethod = new QComboBox(pageAppearance);
    coloringMethod->addItem(tr("Solid"),0);
    coloringMethod->addItem(tr("Speed"),1);
    coloringMethod->addItem(tr("Vorticity magnitude"),2);
    coloringMethod->addItem(tr("Arc length"),3);
    coloringMethod->addItem(tr("Time"),4);
    coloringMethod->addItem(tr("Seed point ID"),5);
    coloringMethod->addItem(tr("Variable"),6);
    connect(coloringMethod, SIGNAL(activated(int)),
            this, SLOT(coloringMethodChanged(int)));
    aLayout->addWidget(new QLabel(tr("Color by"), pageAppearance), 4,0);
    aLayout->addWidget(coloringMethod, 4,1);

    varLabel = new QLabel(tr("Color Variable"), pageAppearance);
    var = new QvisVariableButton(false, true, true, QvisVariableButton::Scalars,
                                 pageAppearance);
    aLayout->addWidget(varLabel,5,0);
    aLayout->addWidget(var,5,1);
    connect(var, SIGNAL(activated(const QString &)),
            this, SLOT(coloringVariableChanged(const QString&)));

    colorTableName = new QvisColorTableButton(pageAppearance);
    connect(colorTableName, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(colorTableNameChanged(bool, const QString&)));
    colorTableNameLabel = new QLabel(tr("Color table"), pageAppearance);
    colorTableNameLabel->setBuddy(colorTableName);
    aLayout->addWidget(colorTableNameLabel,6,0);
    aLayout->addWidget(colorTableName, 6,1, Qt::AlignLeft);

    singleColor = new QvisColorButton(pageAppearance);
    connect(singleColor, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(singleColorChanged(const QColor&)));
    singleColorLabel = new QLabel(tr("Single color"), pageAppearance);
    singleColorLabel->setBuddy(singleColor);
    aLayout->addWidget(singleColorLabel,7,0);
    aLayout->addWidget(singleColor, 7,1, Qt::AlignLeft);

    //
    // Create advanced widgets.
    //
    QWidget *pageAdvanced = new QWidget(central);
    tabs->addTab(pageAdvanced, tr("Advanced"));
    QGridLayout *advGLayout = new QGridLayout(pageAdvanced);
    advGLayout->setMargin(5);
    advGLayout->setSpacing(5);

    QGroupBox *algoGrp = new QGroupBox(pageAdvanced);
    algoGrp->setTitle(tr("Parallel streamline options"));
    advGLayout->addWidget(algoGrp, 0, 0, 1, 4);

    // Algorithm group.
    QGridLayout *algoGLayout = new QGridLayout(algoGrp);
    algoGLayout->setSpacing(10);
    algoGLayout->setColumnStretch(1,10);

    slAlgoLabel = new QLabel(tr("Parallelize across"), algoGrp);
    slAlgo = new QComboBox(algoGrp);
    slAlgo->addItem(tr("Streamlines"));
    slAlgo->addItem(tr("Domains"));
    slAlgo->addItem(tr("Streamlines and Domains"));
    connect(slAlgo, SIGNAL(activated(int)),
            this, SLOT(streamlineAlgorithmChanged(int)));
    algoGLayout->addWidget( slAlgoLabel, 1,0);
    algoGLayout->addWidget( slAlgo, 1,1);
    
    maxSLCountLabel = new QLabel(tr("Communication threshold"), algoGrp);
    maxSLCount = new QSpinBox(algoGrp);
    maxSLCount->setMinimum(1);
    maxSLCount->setMaximum(100000);
    connect(maxSLCount, SIGNAL(valueChanged(int)), 
            this, SLOT(maxSLCountChanged(int)));
    algoGLayout->addWidget( maxSLCountLabel, 2,0);
    algoGLayout->addWidget( maxSLCount,2,1);

    maxDomainCacheLabel = new QLabel(tr("Domain cache size"), algoGrp);
    maxDomainCache = new QSpinBox(algoGrp);
    maxDomainCache->setMinimum(1);
    maxDomainCache->setMaximum(100000);
    connect(maxDomainCache, SIGNAL(valueChanged(int)),
            this, SLOT(maxDomainCacheChanged(int)));
    algoGLayout->addWidget( maxDomainCacheLabel, 3,0);
    algoGLayout->addWidget( maxDomainCache, 3,1);

    workGroupSizeLabel = new QLabel(tr("Work group size"), algoGrp);
    workGroupSize = new QSpinBox(algoGrp);
    workGroupSize->setMinimum(2);
    workGroupSize->setMaximum(1000000);
    connect(workGroupSize, SIGNAL(valueChanged(int)),
            this, SLOT(workGroupSizeChanged(int)));
    algoGLayout->addWidget( workGroupSizeLabel, 4,0);
    algoGLayout->addWidget( workGroupSize, 4,1);

    // Integrator group.
    QGroupBox *intGrp = new QGroupBox(pageAdvanced);
    intGrp->setTitle(tr("Integration method") );

    advGLayout->addWidget(intGrp, 1, 0, 1, 4);
    QGridLayout *intGLayout = new QGridLayout(intGrp);
    intGLayout->setSpacing(10);
    intGLayout->setColumnStretch(1,10);

    intGLayout->addWidget( new QLabel(tr("Integrator"), intGrp), 0,0);
    integrationType = new QComboBox(intGrp);
    integrationType->addItem(tr("Dormand-Prince (Runge-Kutta)"));
    integrationType->addItem(tr("Adams-Bashforth (Multi-step)"));
    connect(integrationType, SIGNAL(activated(int)),
            this, SLOT(integrationTypeChanged(int)));
    intGLayout->addWidget(integrationType, 0,1);
    
    // Create the step length text field.
    maxStepLengthLabel = new QLabel(tr("Maximum step length"), intGrp);
    maxStepLength = new QLineEdit(intGrp);
    connect(maxStepLength, SIGNAL(returnPressed()),
            this, SLOT(maxStepLengthProcessText()));
    intGLayout->addWidget(maxStepLengthLabel, 1,0);
    intGLayout->addWidget(maxStepLength, 1,1);

    // Create the absolute tolerance text field.
    absTolLabel = new QLabel(tr("Absolute tolerance"), intGrp);
    absTol = new QLineEdit(intGrp);
    connect(absTol, SIGNAL(returnPressed()),
            this, SLOT(absTolProcessText()));
    intGLayout->addWidget(absTolLabel, 2,0);
    intGLayout->addWidget(absTol, 2,1);

    // Create the relative tolerance text field.
    relTolLabel = new QLabel(tr("Relative tolerance"), intGrp);
    relTol = new QLineEdit(intGrp);
    connect(relTol, SIGNAL(returnPressed()),
            this, SLOT(relTolProcessText()));
    intGLayout->addWidget(relTolLabel, 3,0);
    intGLayout->addWidget(relTol, 3,1);


    legendFlag = new QCheckBox(tr("Legend"), central);
    connect(legendFlag, SIGNAL(toggled(bool)),
            this, SLOT(legendFlagChanged(bool)));
    mainLayout->addWidget(legendFlag, 7,0);

    lightingFlag = new QCheckBox(tr("Lighting"), central);
    connect(lightingFlag, SIGNAL(toggled(bool)),
            this, SLOT(lightingFlagChanged(bool)));
    mainLayout->addWidget(lightingFlag, 7,1);
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
//   Brad Whitlock, Wed Aug  6 10:38:36 PDT 2008
//   Compensate for 2.0.0 version changes.
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
    if (StreamlineAttributes::VersionLessThan(configVersion, "1.4.1") ||
        StreamlineAttributes::VersionLessThan(configVersion, "2.0.0"))
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
//   Brad Whitlock, Wed Aug  6 10:39:12 PDT 2008
//   Qt 4.
//
//   Dave Pugmire, Wed Aug 6 15:16:23 EDT 2008
//   Add accurate distance calculate option.
//
//   Dave Pugmire, Fri Aug 8 16:27:03 EDT 2008
//   Set the termType combo box.
//
//   Dave Pugmire, Tue Aug 19 17:18:03 EST 2008
//   Removed the accurate distance calculation option.
//
//   Dave Pugmire, Thu Feb  5 12:20:15 EST 2009
//   Added workGroupSize for the masterSlave algorithm.
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
        case StreamlineAttributes::ID_sourceType:
            // Update lots of widget visibility and enabled states.
            UpdateSourceAttributes();

            sourceType->blockSignals(true);
            sourceType->setCurrentIndex(streamAtts->GetSourceType());
            sourceType->blockSignals(false);
            break;
        case StreamlineAttributes::ID_maxStepLength:
            temp.setNum(streamAtts->GetMaxStepLength());
            maxStepLength->setText(temp);
            break;
        case StreamlineAttributes::ID_termination:
            temp.setNum(streamAtts->GetTermination());
            termination->setText(temp);
            break;
        case StreamlineAttributes::ID_pointSource:
            pointSource->setText(DoublesToQString(streamAtts->GetPointSource(),3));
            break;
        case StreamlineAttributes::ID_lineStart:
            lineStart->setText(DoublesToQString(streamAtts->GetLineStart(),3));
            break;
        case StreamlineAttributes::ID_lineEnd:
            lineEnd->setText(DoublesToQString(streamAtts->GetLineEnd(),3));
            break;
        case StreamlineAttributes::ID_planeOrigin:
            planeOrigin->setText(DoublesToQString(streamAtts->GetPlaneOrigin(),3));
            break;
        case StreamlineAttributes::ID_planeNormal:
            planeNormal->setText(DoublesToQString(streamAtts->GetPlaneNormal(),3));
            break;
        case StreamlineAttributes::ID_planeUpAxis:
            planeUpAxis->setText(DoublesToQString(streamAtts->GetPlaneUpAxis(),3));
            break;
        case StreamlineAttributes::ID_planeRadius:
            temp.setNum(streamAtts->GetPlaneRadius());
            planeRadius->setText(temp);
            break;
        case StreamlineAttributes::ID_sphereOrigin:
            sphereOrigin->setText(DoublesToQString(streamAtts->GetSphereOrigin(),3));
            break;
        case StreamlineAttributes::ID_sphereRadius:
            temp.setNum(streamAtts->GetSphereRadius());
            sphereRadius->setText(temp);
            break;
        case StreamlineAttributes::ID_pointList:
            pointList->setText(DoublesToQString(streamAtts->GetPointList()));
            break;
        case StreamlineAttributes::ID_boxExtents:
            boxExtents[0]->setText(DoublesToQString(streamAtts->GetBoxExtents(),2));
            boxExtents[1]->setText(DoublesToQString(streamAtts->GetBoxExtents()+2,2));
            boxExtents[2]->setText(DoublesToQString(streamAtts->GetBoxExtents()+4,2));
            break;
        case StreamlineAttributes::ID_useWholeBox:
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
        case StreamlineAttributes::ID_pointDensity:
            pointDensity->blockSignals(true);
            pointDensity->setValue(streamAtts->GetPointDensity());
            pointDensity->blockSignals(false);
            break;
        case StreamlineAttributes::ID_displayMethod:
            { // new scope
            bool showLines = streamAtts->GetDisplayMethod() == 
                StreamlineAttributes::Lines;
            bool showTubes = streamAtts->GetDisplayMethod() == 
                StreamlineAttributes::Tubes;
            bool showRibbons = streamAtts->GetDisplayMethod() == 
                StreamlineAttributes::Ribbons;
            radius->setEnabled(showTubes || showRibbons || showStart);
            radiusLabel->setEnabled(showTubes || showRibbons || showStart);
            lineWidth->setEnabled(showLines);
            lineWidthLabel->setEnabled(showLines);

            displayMethod->blockSignals(true);
            displayMethod->setCurrentIndex(int(streamAtts->GetDisplayMethod()));
            displayMethod->blockSignals(false);
            }
            break;
        case StreamlineAttributes::ID_showStart:
            showStart->blockSignals(true);
            showStart->setChecked(streamAtts->GetShowStart());
            showStart->blockSignals(false);
            break;
        case StreamlineAttributes::ID_radius:
            temp.setNum(streamAtts->GetRadius());
            radius->setText(temp);
            break;
        case StreamlineAttributes::ID_lineWidth:
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(streamAtts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
        case StreamlineAttributes::ID_coloringMethod:
            {// New scope
            bool needCT = streamAtts->GetColoringMethod() != StreamlineAttributes::Solid;
            colorTableName->setEnabled(needCT);
            colorTableNameLabel->setEnabled(needCT);
            singleColor->setEnabled(!needCT);
            singleColorLabel->setEnabled(!needCT);

            coloringMethod->blockSignals(true);
            coloringMethod->setCurrentIndex(int(streamAtts->GetColoringMethod()));
            coloringMethod->blockSignals(false);

            if (streamAtts->GetColoringMethod() == StreamlineAttributes::ColorByVariable)
            {
                varLabel->setEnabled(true);
                var->setEnabled(true);
                varLabel->show();
                var->show();
            }
            else
            {
                varLabel->setEnabled(false);
                var->setEnabled(false);
                varLabel->hide();
                var->hide();
            }

            }
            break;
        case StreamlineAttributes::ID_colorTableName:
            colorTableName->setColorTable(streamAtts->GetColorTableName().c_str());
            break;
        case StreamlineAttributes::ID_singleColor:
            tempcolor = QColor(streamAtts->GetSingleColor().Red(),
                               streamAtts->GetSingleColor().Green(),
                               streamAtts->GetSingleColor().Blue());
            singleColor->setButtonColor(tempcolor);
            break;
        case StreamlineAttributes::ID_legendFlag:
            legendFlag->blockSignals(true);
            legendFlag->setChecked(streamAtts->GetLegendFlag());
            legendFlag->blockSignals(false);
            break;
        case StreamlineAttributes::ID_lightingFlag:
            lightingFlag->blockSignals(true);
            lightingFlag->setChecked(streamAtts->GetLightingFlag());
            lightingFlag->blockSignals(false);
            break;
        case StreamlineAttributes::ID_StreamlineDirection:
            directionType->blockSignals(true);
            directionType->setCurrentIndex(int(streamAtts->GetStreamlineDirection()) );
            directionType->blockSignals(false);
            break;
        case StreamlineAttributes::ID_relTol:
            temp.setNum(streamAtts->GetRelTol());
            relTol->setText(temp);
            break;
        case StreamlineAttributes::ID_absTol:
            temp.setNum(streamAtts->GetAbsTol());
            absTol->setText(temp);
            break;
        case StreamlineAttributes::ID_terminationType:
            UpdateTerminationType();
            termType->blockSignals(true);
            termType->setCurrentIndex( int(streamAtts->GetTerminationType()) );
            termType->blockSignals(false);
            break;
        case StreamlineAttributes::ID_integrationType:
            // Update lots of widget visibility and enabled states.
            UpdateIntegrationAttributes();

            integrationType->blockSignals(true);
            integrationType->setCurrentIndex(streamAtts->GetIntegrationType());
            integrationType->blockSignals(false);
            break;
        case StreamlineAttributes::ID_streamlineAlgorithmType:
            // Update lots of widget visibility and enabled states.
            UpdateAlgorithmAttributes();

            slAlgo->blockSignals(true);
            slAlgo->setCurrentIndex(streamAtts->GetStreamlineAlgorithmType());
            slAlgo->blockSignals(false);
            break;
        case StreamlineAttributes::ID_maxStreamlineProcessCount:
            maxSLCount->blockSignals(true);
            maxSLCount->setValue(streamAtts->GetMaxStreamlineProcessCount());
            maxSLCount->blockSignals(false);
            break;
        case StreamlineAttributes::ID_maxDomainCacheSize:
            maxDomainCache->blockSignals(true);
            maxDomainCache->setValue(streamAtts->GetMaxDomainCacheSize());
            maxDomainCache->blockSignals(false);
            break;
        case StreamlineAttributes::ID_workGroupSize:
            workGroupSize->blockSignals(true);
            workGroupSize->setValue(streamAtts->GetWorkGroupSize());
            workGroupSize->blockSignals(false);
            break;
        case StreamlineAttributes::ID_pathlines:
          /*
            pathlineFlag->blockSignals(true);
            pathlineFlag->setChecked(streamAtts->GetPathlines());
            pathlineFlag->blockSignals(false);
          */
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
//   Hank Childs, Sat May  2 22:05:56 PDT 2009
//   Add support for point lists.
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
    bool usePointList =  streamAtts->GetSourceType() == StreamlineAttributes::SpecifiedPointList;

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
    }
    else
    {
        pointSource->hide();
        pointSourceLabel->hide();
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

    //
    // Update the point list widgets.
    //
    pointList->setEnabled(usePointList);
    pointListLabel->setEnabled(usePointList);
    if(usePointList)
    {
        sourceAtts->setTitle(tr("Point List"));
        pointList->show();
        pointListLabel->show();
    }
    else
    {
        pointList->hide();
        pointListLabel->hide();
    }

    if (usePoint || usePointList)
        pointDensity->setEnabled(false);
    else
        pointDensity->setEnabled(true);
}

// ****************************************************************************
// Method: QvisStreamlinePlotWindow::UpdateIntegrationAttributes
//
// Purpose: 
//   Updates the widgets for the various integration types.
//
// Programmer: Dave Pugmire
// Creation:   Thu Jul 31 14:41:00 EDT 2008
//
// Modifications:
//
//   Dave Pugmire, Fri Aug 8 16:27:03 EDT 2008
//   Change the step label text based on the integration method.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::UpdateIntegrationAttributes()
{
    bool useDormandPrince = streamAtts->GetIntegrationType() == StreamlineAttributes::DormandPrince;
    bool useAdamsBashforth = streamAtts->GetIntegrationType() == StreamlineAttributes::AdamsBashforth;

    //Turn off everything.
    maxStepLength->hide();
    maxStepLengthLabel->hide();
    relTol->hide();
    relTolLabel->hide();
    absTol->hide();
    absTolLabel->hide();

    if ( useDormandPrince )
    {
        maxStepLength->show();
        maxStepLengthLabel->show();
        maxStepLengthLabel->setText(tr("Maximum step length"));
        relTol->show();
        relTolLabel->show();
        absTol->show();
        absTolLabel->show();
    }
    else if ( useAdamsBashforth )
    {
        maxStepLength->show();
        maxStepLengthLabel->show();
        maxStepLengthLabel->setText(tr("Step length"));
        absTol->show();
        absTolLabel->show();
    }
}

// ****************************************************************************
// Method: QvisStreamlinePlotWindow::UpdateTerminationType
//
// Purpose: 
//   Updates the widgets for the various termination types.
//
// Programmer: Dave Pugmire
// Creation:   Thu Feb 19 12:35:38 EST 2008
//
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::UpdateTerminationType()
{
  /*
    if (streamAtts->GetTerminationType() == StreamlineAttributes::Time ||
        streamAtts->GetTerminationType() == StreamlineAttributes::Distance)
      {
        //termination->SetPrecision(5);
      }
    else
      {
      streamAtts->SetTermination( (int)streamAtts->GetTermination());
      }
  */
}


// ****************************************************************************
// Method: QvisStreamlinePlotWindow::UpdateAlgorithmAttributes
//
// Purpose: 
//   Updates the widgets for the various integration types.
//
// Programmer: Dave Pugmire
// Creation:   Fri Aug 1 16:41:38 EDT 2008
//
//
// Modifications:
//
//   Dave Pugmire, Thu Feb  5 12:20:15 EST 2009
//   Added workGroupSize for the masterSlave algorithm.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::UpdateAlgorithmAttributes()
{
    bool useLoadOnDemand = streamAtts->GetStreamlineAlgorithmType() == StreamlineAttributes::LoadOnDemand;
    bool useStaticDomains = streamAtts->GetStreamlineAlgorithmType() == StreamlineAttributes::ParallelStaticDomains;
    bool useMasterSlave = streamAtts->GetStreamlineAlgorithmType() == StreamlineAttributes::MasterSlave;
    
    //Turn off everything.
    maxDomainCacheLabel->hide();
    maxDomainCache->hide();
    maxSLCountLabel->hide();
    maxSLCount->hide();
    workGroupSizeLabel->hide();
    workGroupSize->hide();

    if ( useLoadOnDemand )
    {
        maxDomainCacheLabel->show();
        maxDomainCache->show();
    }
    else if ( useStaticDomains )
    {
        maxSLCountLabel->show();
        maxSLCount->show();
    }
    else if (useMasterSlave)
    {
        maxDomainCacheLabel->show();
        maxDomainCache->show();
        maxSLCountLabel->show();
        maxSLCount->show();
        workGroupSizeLabel->show();
        workGroupSize->show();
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
//   Dave Pugmire, Mon Aug 4 2:49:38 EDT 2008
//   Added termination, algorithm and integration options.
//
//   Brad Whitlock, Wed Aug  6 15:34:13 PDT 2008
//   Use new methods.
//
//   Dave Pugmire, Thu Feb  5 12:20:15 EST 2009
//   Added workGroupSize for the masterSlave algorithm.
//
// ****************************************************************************

void
QvisStreamlinePlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do stepLength
    if(which_widget == StreamlineAttributes::ID_maxStepLength || doAll)
    {
        double val;
        if(LineEditGetDouble(maxStepLength, val))
            streamAtts->SetMaxStepLength(val);
        else
        {
            ResettingError(tr("step length"),
                DoubleToQString(streamAtts->GetMaxStepLength()));
            streamAtts->SetMaxStepLength(streamAtts->GetMaxStepLength());
        }
    }

    // Do termination
    if(which_widget == StreamlineAttributes::ID_termination || doAll)
    {
        double val;
        if(LineEditGetDouble(termination, val))
            streamAtts->SetTermination(val);
        else
        {
            ResettingError(tr("termination"),
                DoubleToQString(streamAtts->GetTermination()));
            streamAtts->SetTermination(streamAtts->GetTermination());
        }
    }

    // Do relTol
    if(which_widget == StreamlineAttributes::ID_relTol || doAll)
    {
        double val;
        if(LineEditGetDouble(relTol, val))
            streamAtts->SetRelTol(val);
        else
        {
            ResettingError(tr("relative tolerance"),
                DoubleToQString(streamAtts->GetRelTol()));
            streamAtts->SetRelTol(streamAtts->GetRelTol());
        }
    }

    // Do absTol
    if(which_widget == StreamlineAttributes::ID_absTol || doAll)
    {
        double val;
        if(LineEditGetDouble(absTol, val))
            streamAtts->SetAbsTol(val);
        else
        {
            ResettingError(tr("absolute tolerance"),
                DoubleToQString(streamAtts->GetAbsTol()));
            streamAtts->SetAbsTol(streamAtts->GetAbsTol());
        }
    }

    // Do pointSource
    if(which_widget == StreamlineAttributes::ID_pointSource || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(pointSource, val, 3))
            streamAtts->SetPointSource(val);
        else
        {
            ResettingError(tr("point source"),
                DoublesToQString(streamAtts->GetPointSource(), 3));
            streamAtts->SetPointSource(streamAtts->GetPointSource());
        }
    }

    // Do lineStart
    if(which_widget == StreamlineAttributes::ID_lineStart || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(lineStart, val, 3))
            streamAtts->SetLineStart(val);
        else
        {
            ResettingError(tr("line start"),
                DoublesToQString(streamAtts->GetLineStart(), 3));
            streamAtts->SetLineStart(streamAtts->GetLineStart());
        }
    }

    // Do lineEnd
    if(which_widget == StreamlineAttributes::ID_lineEnd || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(lineEnd, val, 3))
            streamAtts->SetLineEnd(val);
        else
        {
            ResettingError(tr("line end"),
                DoublesToQString(streamAtts->GetLineEnd(), 3));
            streamAtts->SetLineEnd(streamAtts->GetLineEnd());
        }
    }

    // Do planeOrigin
    if(which_widget == StreamlineAttributes::ID_planeOrigin || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(planeOrigin, val, 3))
            streamAtts->SetPlaneOrigin(val);
        else
        {
            ResettingError(tr("plane origin"),
                DoublesToQString(streamAtts->GetPlaneOrigin(), 3));
            streamAtts->SetPlaneOrigin(streamAtts->GetPlaneOrigin());
        }
    }

    // Do planeNormal
    if(which_widget == StreamlineAttributes::ID_planeNormal || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(planeNormal, val, 3))
            streamAtts->SetPlaneNormal(val);
        else
        {
            ResettingError(tr("plane normal"),
                DoublesToQString(streamAtts->GetPlaneNormal(), 3));
            streamAtts->SetPlaneNormal(streamAtts->GetPlaneNormal());
        }
    }

    // Do planeUpAxis
    if(which_widget == StreamlineAttributes::ID_planeUpAxis || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(planeUpAxis, val, 3))
            streamAtts->SetPlaneUpAxis(val);
        else
        {
            ResettingError(tr("plane up axis"),
                DoublesToQString(streamAtts->GetPlaneUpAxis(), 3));
            streamAtts->SetPlaneUpAxis(streamAtts->GetPlaneUpAxis());
        }
    }

    // Do planeRadius
    if(which_widget == StreamlineAttributes::ID_planeRadius || doAll)
    {
        double val;
        if(LineEditGetDouble(planeRadius, val))
            streamAtts->SetPlaneRadius(val);
        else
        {
            ResettingError(tr("plane radius"),
                DoubleToQString(streamAtts->GetPlaneRadius()));
            streamAtts->SetPlaneRadius(streamAtts->GetPlaneRadius());
        }
    }

    // Do sphereOrigin
    if(which_widget == StreamlineAttributes::ID_sphereOrigin || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(sphereOrigin, val, 3))
            streamAtts->SetSphereOrigin(val);
        else
        {
            ResettingError(tr("sphere origin"),
                DoublesToQString(streamAtts->GetSphereOrigin(), 3));
            streamAtts->SetSphereOrigin(streamAtts->GetSphereOrigin());
        }
    }

    // Do sphereRadius
    if(which_widget == StreamlineAttributes::ID_sphereRadius || doAll)
    {
        double val;
        if(LineEditGetDouble(sphereRadius, val))
            streamAtts->SetSphereRadius(val);
        else
        {
            ResettingError(tr("sphere radius"),
                DoubleToQString(streamAtts->GetSphereRadius()));
            streamAtts->SetSphereRadius(streamAtts->GetSphereRadius());
        }
    }

    // Do boxExtents
    if(which_widget == StreamlineAttributes::ID_boxExtents || doAll)
    {
        double d[6];
        bool allOkay = true;
        for(int i = 0; i < 3; ++i)
        {
            if(!LineEditGetDoubles(boxExtents[i], &d[i*2], 2))
                allOkay = false;
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

    // Do pointList
    if(which_widget == StreamlineAttributes::ID_pointList || doAll)
    {
        double d[3];
        bool allOkay = true;
        std::vector<double> pointListV;
        if(!LineEditGetDoubles(pointList, pointListV))
            allOkay = false;
        bool multipleOf3Error = false;
        if ((pointListV.size() % 3) != 0)
        {
            allOkay = false;
            multipleOf3Error = true;
        }

        if(!allOkay)
        {
            if (multipleOf3Error)
                Message(tr("The number of values in the point list was not a multiple" 
                           " of three, so the previous values will be used."));
            else
                Message(tr("The point list contained errors so the previous "
                           "values will be used."));
            streamAtts->SelectPointList();
        }
        else
            streamAtts->SetPointList(pointListV);
    }

    // pointDensity
    if (which_widget == StreamlineAttributes::ID_pointDensity|| doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = pointDensity->value();
        if (val >= 1)
            streamAtts->SetPointDensity(val);
    }

    // Do radius
    if(which_widget == StreamlineAttributes::ID_radius || doAll)
    {
        double val;
        if(LineEditGetDouble(radius, val))
            streamAtts->SetRadius(val);
        else
        {
            ResettingError(tr("tube radius"),
                DoubleToQString(streamAtts->GetRadius()));
            streamAtts->SetRadius(streamAtts->GetRadius());
        }
    }

    // maxStreamlineProcessCount
    if (which_widget == StreamlineAttributes::ID_maxStreamlineProcessCount || doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = maxSLCount->value();
        if (val >= 1)
            streamAtts->SetMaxStreamlineProcessCount(val);
    }

    // workGroupSize
    if (which_widget == StreamlineAttributes::ID_workGroupSize || doAll)
    {
        // This can only be an integer, so no error checking is needed.
        int val = workGroupSize->value();
        if (val >= 2)
            streamAtts->SetWorkGroupSize(val);
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
QvisStreamlinePlotWindow::termTypeChanged(int val)
 {
    if(val != streamAtts->GetTerminationType())
    {
        streamAtts->SetTerminationType(StreamlineAttributes::TerminationType(val));
        Apply();
    }
}   

void
QvisStreamlinePlotWindow::integrationTypeChanged(int val)
 {
    if(val != streamAtts->GetIntegrationType())
    {
        streamAtts->SetIntegrationType(StreamlineAttributes::IntegrationType(val));
        Apply();
    }
}   

void
QvisStreamlinePlotWindow::streamlineAlgorithmChanged(int val)
{
    if(val != streamAtts->GetStreamlineAlgorithmType())
    {
        streamAtts->SetStreamlineAlgorithmType(StreamlineAttributes::StreamlineAlgorithmType(val));
        Apply();
    }
}   

void
QvisStreamlinePlotWindow::maxStepLengthProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_maxStepLength);
    Apply();
}

void
QvisStreamlinePlotWindow::terminationProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_termination);
    Apply();
}

void
QvisStreamlinePlotWindow::relTolProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_relTol);
    Apply();
}

void
QvisStreamlinePlotWindow::lineStartProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_lineStart);
    Apply();
}

void
QvisStreamlinePlotWindow::lineEndProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_lineEnd);
    Apply();
}

void
QvisStreamlinePlotWindow::pointListProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_pointList);
    Apply();
}

void
QvisStreamlinePlotWindow::planeOriginProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_planeOrigin);
    Apply();
}

void
QvisStreamlinePlotWindow::planeNormalProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_planeNormal);
    Apply();
}

void
QvisStreamlinePlotWindow::planeUpAxisProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_planeUpAxis);
    Apply();
}

void
QvisStreamlinePlotWindow::planeRadiusProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_planeRadius);
    Apply();
}

void
QvisStreamlinePlotWindow::sphereOriginProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_sphereOrigin);
    Apply();
}

void
QvisStreamlinePlotWindow::sphereRadiusProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_sphereRadius);
    Apply();
}

void
QvisStreamlinePlotWindow::pointDensityChanged(int val)
{
    streamAtts->SetPointDensity(val);
    Apply();
}

void
QvisStreamlinePlotWindow::maxSLCountChanged(int val)
{
    streamAtts->SetMaxStreamlineProcessCount(val);
    Apply();
}

void
QvisStreamlinePlotWindow::maxDomainCacheChanged(int val)
{
    streamAtts->SetMaxDomainCacheSize(val);
    Apply();
}

void
QvisStreamlinePlotWindow::workGroupSizeChanged(int val)
{
    streamAtts->SetWorkGroupSize(val);
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
    GetCurrentValues(StreamlineAttributes::ID_radius);
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
    GetCurrentValues(StreamlineAttributes::ID_boxExtents);
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
QvisStreamlinePlotWindow::coloringVariableChanged(const QString &var)
{
    streamAtts->SetColoringVariable(var.toStdString());
    Apply();
}

void
QvisStreamlinePlotWindow::colorTableNameChanged(bool useDefault, const QString &ctName)
{
    streamAtts->SetColorTableName(ctName.toStdString());
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

void
QvisStreamlinePlotWindow::pathlineFlagChanged(bool val)
{
    streamAtts->SetPathlines(val);
    SetUpdate(false);
    Apply();
}

void
QvisStreamlinePlotWindow::absTolProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_absTol);
    Apply();
}

void
QvisStreamlinePlotWindow::pointSourceProcessText()
{
    GetCurrentValues(StreamlineAttributes::ID_pointSource);
    Apply();
}



