#include "QvisStreamlinePlotWindow.h"

#include <StreamlineAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qtabwidget.h>
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
//   
// ****************************************************************************

void
QvisStreamlinePlotWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 5, 2, 10, "mainLayout");

    // Create the step length text field.
    mainLayout->addWidget(new QLabel("Step length", central, "stepLengthLabel"),0,0);
    stepLength = new QLineEdit(central, "stepLength");
    connect(stepLength, SIGNAL(returnPressed()),
            this, SLOT(stepLengthProcessText()));
    mainLayout->addWidget(stepLength, 0,1);

    // Create the maximum time text field.
    mainLayout->addWidget(new QLabel("Maximum time", central, "maxTimeLabel"),1,0);
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
    tabs->addTab(topPageSource, "Streamline source");
    QVBoxLayout *topSourceLayout = new QVBoxLayout(topPageSource);
    topSourceLayout->setMargin(10);
    topSourceLayout->setSpacing(5);

    // Create the source type combo box.
    QHBoxLayout *hLayout = new QHBoxLayout(topSourceLayout);
    hLayout->addWidget(new QLabel("Source type", topPageSource, "sourceTypeLabel"));
    sourceType = new QComboBox(topPageSource, "sourceType");
    sourceType->insertItem("Point");
    sourceType->insertItem("Line");
    sourceType->insertItem("Plane");
    sourceType->insertItem("Sphere");
    sourceType->insertItem("Box");
    connect(sourceType, SIGNAL(activated(int)),
            this, SLOT(sourceTypeChanged(int)));
    hLayout->addWidget(sourceType, 10);
    topSourceLayout->addSpacing(5);

    // Create a group box for the source attributes.
    QGroupBox *pageSource = new QGroupBox(topPageSource, "pageSource");
    sourceAtts = pageSource;
    sourceAtts->setTitle("Point");
    topSourceLayout->addWidget(pageSource);
    topSourceLayout->addStretch(5);
    QVBoxLayout *svLayout = new QVBoxLayout(pageSource, 10, 2);
    svLayout->addSpacing(10);
    QGridLayout *sLayout = new QGridLayout(svLayout, 14, 2);
//    sLayout->setMargin(10);
    sLayout->setSpacing(5);

    // Create the widgets that specify a point source.
    pointSource = new QLineEdit(pageSource, "pointSource");
    connect(pointSource, SIGNAL(returnPressed()),
            this, SLOT(pointSourceProcessText()));
    pointSourceLabel = new QLabel(pointSource, "Location", pageSource, "pointSourceLabel");
    sLayout->addWidget(pointSourceLabel, 3, 0);
    sLayout->addWidget(pointSource, 3,1);

    // Create the widgets that specify a line source.
    lineStart = new QLineEdit(pageSource, "lineStart");
    connect(lineStart, SIGNAL(returnPressed()),
            this, SLOT(lineStartProcessText()));
    lineStartLabel = new QLabel(lineStart, "Start", pageSource, "lineStartLabel");
    sLayout->addWidget(lineStartLabel,4,0);
    sLayout->addWidget(lineStart, 4,1);

    lineEnd = new QLineEdit(pageSource, "lineEnd");
    connect(lineEnd, SIGNAL(returnPressed()),
            this, SLOT(lineEndProcessText()));
    lineEndLabel = new QLabel(lineEnd, "End", pageSource, "lineEndLabel");
    sLayout->addWidget(lineEndLabel,5,0);
    sLayout->addWidget(lineEnd, 5,1);

    // Create the widgets that specify a plane source.
    planeOrigin = new QLineEdit(pageSource, "planeOrigin");
    connect(planeOrigin, SIGNAL(returnPressed()),
            this, SLOT(planeOriginProcessText()));
    planeOriginLabel = new QLabel(planeOrigin, "Origin", pageSource, "planeOriginLabel");
    sLayout->addWidget(planeOriginLabel,6,0);
    sLayout->addWidget(planeOrigin, 6,1);

    planeNormal = new QLineEdit(pageSource, "planeNormal");
    connect(planeNormal, SIGNAL(returnPressed()),
            this, SLOT(planeNormalProcessText()));
    planeNormalLabel = new QLabel(planeNormal, "Normal", pageSource, "planeNormalLabel");
    sLayout->addWidget(planeNormalLabel,7,0);
    sLayout->addWidget(planeNormal, 7,1);

    planeUpAxis = new QLineEdit(pageSource, "planeUpAxis");
    connect(planeUpAxis, SIGNAL(returnPressed()),
            this, SLOT(planeUpAxisProcessText()));
    planeUpAxisLabel = new QLabel(planeUpAxis, "Up axis", pageSource, "planeUpAxisLabel");
    sLayout->addWidget(planeUpAxisLabel,8,0);
    sLayout->addWidget(planeUpAxis, 8,1);

    planeRadius = new QLineEdit(pageSource, "planeRadius");
    connect(planeRadius, SIGNAL(returnPressed()),
            this, SLOT(planeRadiusProcessText()));
    planeRadiusLabel = new QLabel(planeRadius, "Radius", pageSource, "planeRadiusLabel");
    sLayout->addWidget(planeRadiusLabel,9,0);
    sLayout->addWidget(planeRadius, 9,1);

    // Create the widgets that specify a sphere source.
    sphereOrigin = new QLineEdit(pageSource, "sphereOrigin");
    connect(sphereOrigin, SIGNAL(returnPressed()),
            this, SLOT(sphereOriginProcessText()));
    sphereOriginLabel = new QLabel(sphereOrigin, "Origin", pageSource, "sphereOriginLabel");
    sLayout->addWidget(sphereOriginLabel,10,0);
    sLayout->addWidget(sphereOrigin, 10,1);

    sphereRadius = new QLineEdit(pageSource, "sphereRadius");
    connect(sphereRadius, SIGNAL(returnPressed()),
            this, SLOT(sphereRadiusProcessText()));
    sphereRadiusLabel = new QLabel(sphereRadius, "Radius", pageSource, "sphereRadiusLabel");
    sLayout->addWidget(sphereRadiusLabel,11,0);
    sLayout->addWidget(sphereRadius, 11,1);

    // Create the widgets that specify a box source
    boxExtents[0] = new QLineEdit(pageSource, "boxExtents[0]");
    connect(boxExtents[0], SIGNAL(returnPressed()),
            this, SLOT(boxExtentsProcessText()));
    boxExtentsLabel[0] = new QLabel(boxExtents[0], "X Extents", pageSource, "boxExtentsLabel[0]");
    sLayout->addWidget(boxExtentsLabel[0], 11, 0);
    sLayout->addWidget(boxExtents[0], 11, 1);
    boxExtents[1] = new QLineEdit(pageSource, "boxExtents[1]");
    connect(boxExtents[1], SIGNAL(returnPressed()),
            this, SLOT(boxExtentsProcessText()));
    boxExtentsLabel[1] = new QLabel(boxExtents[1], "Y Extents", pageSource, "boxExtentsLabel[1]");
    sLayout->addWidget(boxExtentsLabel[1], 12, 0);
    sLayout->addWidget(boxExtents[1], 12, 1);
    boxExtents[2] = new QLineEdit(pageSource, "boxExtents[2]");
    connect(boxExtents[2], SIGNAL(returnPressed()),
            this, SLOT(boxExtentsProcessText()));
    boxExtentsLabel[2] = new QLabel(boxExtents[2], "Z Extents", pageSource, "boxExtentsLabel[2]");
    sLayout->addWidget(boxExtentsLabel[2], 13, 0);
    sLayout->addWidget(boxExtents[2], 13, 1);

    //
    // Create appearance-related widgets.
    //
    QGroupBox *pageAppearance = new QGroupBox(central, "pageAppearance");
    pageAppearance->setFrameStyle(QFrame::NoFrame);
    tabs->addTab(pageAppearance, "Appearance");
    QGridLayout *aLayout = new QGridLayout(pageAppearance, 6, 2);
    aLayout->setMargin(10);
    aLayout->setSpacing(5);

    // Create widgets that help determine the appearance of the streamlines.
    showTube = new QCheckBox("Show tubes", pageAppearance, "showTube");
    connect(showTube, SIGNAL(toggled(bool)),
            this, SLOT(showTubeChanged(bool)));
    aLayout->addWidget(showTube, 0,0);

    showStart = new QCheckBox("Show tube start", pageAppearance, "showStart");
    connect(showStart, SIGNAL(toggled(bool)),
            this, SLOT(showStartChanged(bool)));
    aLayout->addWidget(showStart, 0,1);

    tubeRadius = new QLineEdit(pageAppearance, "tubeRadius");
    connect(tubeRadius, SIGNAL(returnPressed()),
            this, SLOT(tubeRadiusProcessText()));
    tubeRadiusLabel = new QLabel(tubeRadius, "Tube radius", pageAppearance, "tubeRadiusLabel");
    aLayout->addWidget(tubeRadiusLabel,1,0);
    aLayout->addWidget(tubeRadius, 1,1);

    lineWidth = new QvisLineWidthWidget(0, pageAppearance, "lineWidth");
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    lineWidthLabel = new QLabel(lineWidth, "Line width", pageAppearance, "lineWidthLabel");
    aLayout->addWidget(lineWidthLabel,2,0);
    aLayout->addWidget(lineWidth, 2,1);

    colorBySpeed = new QCheckBox("Color by speed", pageAppearance, "colorBySpeed");
    connect(colorBySpeed, SIGNAL(toggled(bool)),
            this, SLOT(colorBySpeedChanged(bool)));
    aLayout->addWidget(colorBySpeed, 3,0);

    colorTableName = new QvisColorTableButton(pageAppearance, "colorTableName");
    connect(colorTableName, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(colorTableNameChanged(bool, const QString&)));
    colorTableNameLabel = new QLabel(colorTableName, "Color table", pageAppearance, "colorTableNameLabel");
    aLayout->addWidget(colorTableNameLabel,4,0);
    aLayout->addWidget(colorTableName, 4,1, Qt::AlignLeft);

    singleColor = new QvisColorButton(pageAppearance, "singleColor");
    connect(singleColor, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(singleColorChanged(const QColor&)));
    singleColorLabel = new QLabel(singleColor, "Single color", pageAppearance, "singleColorLabel");
    aLayout->addWidget(singleColorLabel,5,0);
    aLayout->addWidget(singleColor, 5,1, Qt::AlignLeft);

    //
    // Create the widget that lets the user set the point density.
    //
    mainLayout->addWidget(new QLabel("Point density", central, "pointDensityLabel"),3,0);
    pointDensity = new QSpinBox(1, 30, 1, central, "pointDensity");
    connect(pointDensity, SIGNAL(valueChanged(int)), 
            this, SLOT(pointDensityChanged(int)));
    mainLayout->addWidget(pointDensity, 3,1);

    legendFlag = new QCheckBox("Legend", central, "legendFlag");
    connect(legendFlag, SIGNAL(toggled(bool)),
            this, SLOT(legendFlagChanged(bool)));
    mainLayout->addWidget(legendFlag, 4,0);

    lightingFlag = new QCheckBox("Lighting", central, "lightingFlag");
    connect(lightingFlag, SIGNAL(toggled(bool)),
            this, SLOT(lightingFlagChanged(bool)));
    mainLayout->addWidget(lightingFlag, 4,1);

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
            temp.sprintf("%g", streamAtts->GetStepLength());
            stepLength->setText(temp);
            break;
        case 2: // maxTime
            temp.sprintf("%g", streamAtts->GetMaxTime());
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
            temp.sprintf("%g", streamAtts->GetPlaneRadius());
            planeRadius->setText(temp);
            break;
        case 10: // sphereOrigin
            dptr = streamAtts->GetSphereOrigin();
            temp.sprintf("%g %g %g", dptr[0], dptr[1], dptr[2]);
            sphereOrigin->setText(temp);
            break;
        case 11: // sphereRadius
            temp.sprintf("%g", streamAtts->GetSphereRadius());
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
        case 13: // pointDensity
            pointDensity->blockSignals(true);
            pointDensity->setValue(streamAtts->GetPointDensity());
            pointDensity->blockSignals(false);
            break;
        case 14: // showTube
            showStart->setEnabled(streamAtts->GetShowTube());
            tubeRadius->setEnabled(streamAtts->GetShowTube());
            tubeRadiusLabel->setEnabled(streamAtts->GetShowTube());
            lineWidth->setEnabled(!streamAtts->GetShowTube());
            lineWidthLabel->setEnabled(!streamAtts->GetShowTube());

            showTube->blockSignals(true);
            showTube->setChecked(streamAtts->GetShowTube());
            showTube->blockSignals(false);
            break;
        case 15: // showStart
            showStart->blockSignals(true);
            showStart->setChecked(streamAtts->GetShowStart());
            showStart->blockSignals(false);
            break;
        case 16: // tubeRadius
            temp.sprintf("%g", streamAtts->GetTubeRadius());
            tubeRadius->setText(temp);
            break;
        case 17: // lineWidth
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(streamAtts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
        case 18: // colorBySpeed
            colorTableName->setEnabled(streamAtts->GetColorBySpeed());
            colorTableNameLabel->setEnabled(streamAtts->GetColorBySpeed());
            singleColor->setEnabled(!streamAtts->GetColorBySpeed());
            singleColorLabel->setEnabled(!streamAtts->GetColorBySpeed());

            colorBySpeed->blockSignals(true);
            colorBySpeed->setChecked(streamAtts->GetColorBySpeed());
            colorBySpeed->blockSignals(false);
            break;
        case 19: // colorTableName
            colorTableName->setColorTable(streamAtts->GetColorTableName().c_str());
            break;
        case 20: // singleColor
            tempcolor = QColor(streamAtts->GetSingleColor().Red(),
                               streamAtts->GetSingleColor().Green(),
                               streamAtts->GetSingleColor().Blue());
            singleColor->setButtonColor(tempcolor);
            break;
        case 21: // legendFlag
            legendFlag->blockSignals(true);
            legendFlag->setChecked(streamAtts->GetLegendFlag());
            legendFlag->blockSignals(false);
            break;
        case 22: // lightingFlag
            lightingFlag->blockSignals(true);
            lightingFlag->setChecked(streamAtts->GetLightingFlag());
            lightingFlag->blockSignals(false);
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
        sourceAtts->setTitle("Point");
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
        sourceAtts->setTitle("Line");
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
        sourceAtts->setTitle("Plane");
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
        sourceAtts->setTitle("Sphere");
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
        sourceAtts->setTitle("Box");
    for(int i = 0; i < 3; ++i)
    {
        boxExtents[i]->setEnabled(useBox);
        boxExtentsLabel[i]->setEnabled(useBox);
        if(useBox)
        {
            boxExtents[i]->show();
            boxExtentsLabel[i]->show();
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
            streamAtts->SetStepLength(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of stepLength was invalid. "
                "Resetting to the last good value of %g.",
                streamAtts->GetStepLength());
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
            streamAtts->SetMaxTime(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of maxTime was invalid. "
                "Resetting to the last good value of %g.",
                streamAtts->GetMaxTime());
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
            sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]);
            streamAtts->SetPointSource(val);
        }

        if(!okay)
        {
            const double *val = streamAtts->GetPointSource();
            msg.sprintf("The value of pointSource was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
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
            sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]);
            streamAtts->SetLineStart(val);
        }

        if(!okay)
        {
            const double *val = streamAtts->GetLineStart();
            msg.sprintf("The value of lineStart was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
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
            sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]);
            streamAtts->SetLineEnd(val);
        }

        if(!okay)
        {
            const double *val = streamAtts->GetLineEnd();
            msg.sprintf("The value of lineEnd was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
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
            sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]);
            streamAtts->SetPlaneOrigin(val);
        }

        if(!okay)
        {
            const double *val = streamAtts->GetPlaneOrigin();
            msg.sprintf("The value of planeOrigin was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
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
            sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]);
            streamAtts->SetPlaneNormal(val);
        }

        if(!okay)
        {
            const double *val = streamAtts->GetPlaneNormal();
            msg.sprintf("The value of planeNormal was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
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
            sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]);
            streamAtts->SetPlaneUpAxis(val);
        }

        if(!okay)
        {
            const double *val = streamAtts->GetPlaneUpAxis();
            msg.sprintf("The value of planeUpAxis was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
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
            streamAtts->SetPlaneRadius(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of planeRadius was invalid. "
                "Resetting to the last good value of %g.",
                streamAtts->GetPlaneRadius());
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
            sscanf(temp.latin1(), "%lg %lg %lg", &val[0], &val[1], &val[2]);
            streamAtts->SetSphereOrigin(val);
        }

        if(!okay)
        {
            const double *val = streamAtts->GetSphereOrigin();
            msg.sprintf("The value of sphereOrigin was invalid. "
                "Resetting to the last good value of <%g %g %g>", 
                val[0], val[1], val[2]);
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
            streamAtts->SetSphereRadius(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of sphereRadius was invalid. "
                "Resetting to the last good value of %g.",
                streamAtts->GetSphereRadius());
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
            Message("The box extents contained errors so the previous "
                    "values will be used.");
            streamAtts->SelectBoxExtents();
        }
        else
            streamAtts->SetBoxExtents(d);
    }

    // Do tubeRadius
    if(which_widget == 16 || doAll)
    {
        temp = tubeRadius->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            streamAtts->SetTubeRadius(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of tubeRadius was invalid. "
                "Resetting to the last good value of %g.",
                streamAtts->GetTubeRadius());
            Message(msg);
            streamAtts->SetTubeRadius(streamAtts->GetTubeRadius());
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

        viewer->SetPlotOptions(plotType);
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
    viewer->SetDefaultPlotOptions(plotType);
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
    viewer->ResetPlotOptions(plotType);
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
QvisStreamlinePlotWindow::showTubeChanged(bool val)
{
    streamAtts->SetShowTube(val);
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
QvisStreamlinePlotWindow::tubeRadiusProcessText()
{
    GetCurrentValues(16);
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
QvisStreamlinePlotWindow::colorBySpeedChanged(bool val)
{
    streamAtts->SetColorBySpeed(val);
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


