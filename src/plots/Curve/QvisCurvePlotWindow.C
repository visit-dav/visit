/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include "QvisCurvePlotWindow.h"

#include <CurveAttributes.h>
#include <ViewerProxy.h>
#include <ImproperUseException.h>

#include <QButtonGroup>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QPixmap>
#include <QPixmapCache>
#include <QRadioButton>
#include <QSpinBox>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QNarrowLineEdit.h>
#include <stdio.h>
#include <string>

// Icons
#include <ci_triup.xpm>
#include <ci_tridown.xpm>
#include <ci_square.xpm>
#include <ci_circle.xpm>
#include <ci_plus.xpm>
#include <ci_x.xpm>

using std::string;

// ****************************************************************************
// Method: QvisCurvePlotWindow::QvisCurvePlotWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Jul 23 13:34:33 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Nov 20 13:35:30 PST 2006
//   Added renderMode.
//
// ****************************************************************************

QvisCurvePlotWindow::QvisCurvePlotWindow(const int type,
                         CurveAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad)
{
    plotType = type;
    atts = subj;
}


// ****************************************************************************
// Method: QvisCurvePlotWindow::~QvisCurvePlotWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Jul 23 13:34:33 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisCurvePlotWindow::~QvisCurvePlotWindow()
{
}


// ****************************************************************************
// Method: QvisCurvePlotWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Jul 23 13:34:33 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Tue Dec 23 13:27:22 PST 2003
//   Added PointSize and ShowPoints.
//   
//   Kathleen Bonnell, Thu Oct 27 16:10:29 PDT 2005 
//   Added showLegend.
//   
//   Kathleen Bonnell, Mon Oct 31 17:05:35 PST 2005
//   Added curveColor. 
// 
//   Brad Whitlock, Mon Nov 20 13:34:15 PST 2006
//   Added symbol rendering and changed the layout of the window.
//
//   Brad Whitlock, Tue Apr 22 16:43:24 PDT 2008
//   Added tr()'s.
//
//   Brad Whitlock, Fri Jul 18 10:40:08 PDT 2008
//   Qt 4.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Hank Childs, Thu Jul 15 18:20:26 PDT 2010
//   Add cues for the current location.
//
//   Kathleen Bonnell, Wed Aug 11 09:34:41 PDT 2010
//   Modified layout of geometry group.  Symbols/Points can now be drawn
//   same time as lines. Added pointStride for Static symbols.
//
// ****************************************************************************

void
QvisCurvePlotWindow::CreateWindowContents()
{
    //
    // Create the geometry
    //
    QGroupBox * geometryGroup = new QGroupBox(central);
    geometryGroup->setTitle(tr("Line/Point Geometry"));
    topLayout->addWidget(geometryGroup);

    QGridLayout *geometryLayout = new QGridLayout(geometryGroup);
    geometryLayout->setMargin(5);
    geometryLayout->setSpacing(10);
 

    int ROW = 0;
    //
    // Create line related controls.
    //
    showLines = new QCheckBox(tr("Show lines"), central);
    connect(showLines, SIGNAL(toggled(bool)),
            this, SLOT(showLinesChanged(bool)));

    geometryLayout->addWidget(showLines, ROW, 0, 1, 2);

    ++ROW;

    geometryLayout->addWidget(new QLabel("     ", central), ROW, 0, 1, 1);

    lineStyle = new QvisLineStyleWidget(0, central);
    lineStyleLabel = new QLabel(tr("Line style"), central);
    lineStyleLabel->setBuddy(lineStyle);

    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));

    geometryLayout->addWidget(lineStyleLabel, ROW, 1, 1, 1);
    geometryLayout->addWidget(lineStyle, ROW, 2, 1, 1);

    lineWidth = new QvisLineWidthWidget(0, central);
    lineWidthLabel = new QLabel(tr("Line width"), central);
    lineWidthLabel->setBuddy(lineWidth);

    geometryLayout->addWidget(lineWidthLabel, ROW, 3, 1, 1);
    geometryLayout->addWidget(lineWidth, ROW, 4, 1, 1);

    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));

    ++ROW;

    // 
    // Create point related controls.
    //

    showPoints = new QCheckBox(tr("Show points"), central);
    connect(showPoints, SIGNAL(toggled(bool)),
            this, SLOT(showPointsChanged(bool)));

    geometryLayout->addWidget(showPoints, ROW, 0, 1, 2);

    ++ROW;

    //
    // Create symbol-related controls
    //
#define CREATE_PIXMAP(pixobj, name, xpm) \
    QPixmap pixobj; \
    if(!QPixmapCache::find(name, pixobj)) \
    { \
        char *augmentedData[35], augmentedForeground[15]; \
        for(int i = 0; i < 35; ++i) \
            augmentedData[i] = (char *)xpm[i]; \
        QColor fg(palette().color(QPalette::Text));\
        sprintf(augmentedForeground, ". c #%02x%02x%02x", \
                fg.red(), fg.green(), fg.blue()); \
        augmentedData[2] = augmentedForeground; \
        QPixmap augmented((const char **)augmentedData); \
        QPixmapCache::insert(name, augmented); \
        pixobj = augmented; \
    }
    CREATE_PIXMAP(pix1, "visit_curvewindow_ci_triup", ci_triup_xpm)
    CREATE_PIXMAP(pix2, "visit_curvewindow_ci_tridown", ci_tridown_xpm)
    CREATE_PIXMAP(pix3, "visit_curvewindow_ci_square", ci_square_xpm)
    CREATE_PIXMAP(pix4, "visit_curvewindow_ci_circle", ci_circle_xpm)
    CREATE_PIXMAP(pix5, "visit_curvewindow_ci_plus", ci_plus_xpm)
    CREATE_PIXMAP(pix6, "visit_curvewindow_ci_x", ci_x_xpm)

    symbolType = new QComboBox(central);
    symbolType->setMinimumHeight(20);
    symbolType->addItem(tr("Point"));
    symbolType->addItem(QIcon(pix1), tr("triangle up"));
    symbolType->addItem(QIcon(pix2), tr("triangle down"));
    symbolType->addItem(QIcon(pix3), tr("square"));
    symbolType->addItem(QIcon(pix4), tr("circle"));
    symbolType->addItem(QIcon(pix5), tr("plus"));
    symbolType->addItem(QIcon(pix6), tr("X"));

    connect(symbolType, SIGNAL(activated(int)),
            this, SLOT(symbolTypeChanged(int)));

    symbolTypeLabel = new QLabel(tr("Symbol"), central);
    symbolTypeLabel->setBuddy(symbolType);

    geometryLayout->addWidget(new QLabel("    ", central), ROW, 0, 1 ,1);
    geometryLayout->addWidget(symbolTypeLabel, ROW, 1, 1, 1);
    geometryLayout->addWidget(symbolType, ROW, 2, 1, 1);


    // Create the point size line edit
    pointSize = new QNarrowLineEdit(central);
    pointSizeLabel = new QLabel(tr("Point size"), central);
    pointSizeLabel->setBuddy(pointSize);

    connect(pointSize, SIGNAL(returnPressed()),
            this, SLOT(processPointSizeText())); 

    geometryLayout->addWidget(pointSizeLabel, ROW, 3, 1, 1);
    geometryLayout->addWidget(pointSize, ROW, 4, 1, 1);

    ++ROW;

    fillModeGroup = new QButtonGroup(geometryGroup);

    connect(fillModeGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(fillModeChanged(int)));

    staticButton = new QRadioButton(tr("Static"), geometryGroup);
    staticButton->setChecked(true);
    fillModeGroup->addButton(staticButton, 0);
    geometryLayout->addWidget(staticButton, ROW, 1, 1, 2);

    // Create the point stride 
    pointStride = new QSpinBox(central);
    pointStride->setMinimum(1);
    pointStride->setMaximum(5000);
    pointStrideLabel = new QLabel(tr("Point stride"), central);
    pointStrideLabel->setBuddy(pointStride);

    connect(pointStride, SIGNAL(valueChanged(int)),
            this, SLOT(pointStrideChanged(int))); 

    geometryLayout->addWidget(pointStrideLabel, ROW, 3, 1, 1);
    geometryLayout->addWidget(pointStride, ROW, 4, 1, 1);

    ++ROW;

    dynamicButton = new QRadioButton(tr("Dynamic"), geometryGroup);
    fillModeGroup->addButton(dynamicButton, 1);

    geometryLayout->addWidget(dynamicButton, ROW, 1, 1, 2);

    // Create the point density spin box
    symbolDensity = new QSpinBox(central);
    symbolDensity->setMinimum(10);
    symbolDensity->setMaximum(1000);
    symbolDensityLabel = new QLabel(tr("Point density"), central);
    symbolDensityLabel->setBuddy(symbolDensity);

    connect(symbolDensity, SIGNAL(valueChanged(int)),
            this, SLOT(symbolDensityChanged(int)));

    geometryLayout->addWidget(symbolDensityLabel, ROW, 3, 1, 1);
    geometryLayout->addWidget(symbolDensity, ROW, 4, 1, 1);

    //
    // Create the color
    //
    QGroupBox * colorGroup = new QGroupBox(central);
    colorGroup->setTitle(tr("Color"));
    topLayout->addWidget(colorGroup);

    QGridLayout *colorLayout = new QGridLayout(colorGroup);
    colorLayout->setMargin(5);
    colorLayout->setSpacing(10);
 
    // Create the radio buttons for curve color source
    colorLayout->addWidget(new QLabel(tr("Curve color"), central), 0, 0);

    curveColorButtons = new QButtonGroup(central);

    QRadioButton *rb = new QRadioButton(tr("Cycle"), central);
    rb->setChecked(true);
    curveColorButtons->addButton(rb, 0);
    colorLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("Custom"), central);
    curveColorButtons->addButton(rb, 1);
    colorLayout->addWidget(rb, 0, 2, Qt::AlignRight | Qt::AlignVCenter);

    // Each time a radio button is clicked, call the scale clicked slot.
    connect(curveColorButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(curveColorClicked(int)));

    // Create the curve color button.
    curveColor = new QvisColorButton(central);
    connect(curveColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(curveColorChanged(const QColor &)));
    colorLayout->addWidget(curveColor, 0, 3);

    //
    // Create the time cue stuff
    //
    QGroupBox * timeCueGroup = new QGroupBox(central);
    timeCueGroup->setTitle(tr("Create Cue For Current Location"));
    topLayout->addWidget(timeCueGroup);

    QGridLayout *timeCueLayout = new QGridLayout(timeCueGroup);
    timeCueLayout->setMargin(5);
    timeCueLayout->setSpacing(10);
 
    doBallTimeCue = new QCheckBox(tr("Add Ball"), central);
    connect(doBallTimeCue, SIGNAL(toggled(bool)),
            this, SLOT(doBallTimeCueChanged(bool)));
    timeCueLayout->addWidget(doBallTimeCue, 0,0);

    ballTimeCueColor = new QvisColorButton(central);
    connect(ballTimeCueColor, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(ballTimeCueColorChanged(const QColor&)));
    timeCueLayout->addWidget(ballTimeCueColor, 0,1);

    timeCueBallSizeLabel = new QLabel(tr("Ball size"), central);
    timeCueLayout->addWidget(timeCueBallSizeLabel,0,2);
    timeCueBallSize = new QLineEdit(central);
    connect(timeCueBallSize, SIGNAL(returnPressed()),
            this, SLOT(timeCueBallSizeProcessText()));
    timeCueLayout->addWidget(timeCueBallSize, 0,3);

    doLineTimeCue = new QCheckBox(tr("Add Line"), central);
    connect(doLineTimeCue, SIGNAL(toggled(bool)),
            this, SLOT(doLineTimeCueChanged(bool)));
    timeCueLayout->addWidget(doLineTimeCue, 1,0);

    lineTimeCueColor = new QvisColorButton(central);
    connect(lineTimeCueColor, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(lineTimeCueColorChanged(const QColor&)));
    timeCueLayout->addWidget(lineTimeCueColor, 1,1);

    lineTimeCueWidth = new QvisLineWidthWidget(0, central);
    lineTimeCueWidthLabel = new QLabel(tr("Line width"), central);
    lineTimeCueWidthLabel->setBuddy(lineTimeCueWidth);
    timeCueLayout->addWidget(lineTimeCueWidthLabel,1,2);
    connect(lineTimeCueWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineTimeCueWidthChanged(int)));

    timeCueLayout->addWidget(lineTimeCueWidth, 1,3);

    doCropTimeCue = new QCheckBox(tr("Crop"), central);
    connect(doCropTimeCue, SIGNAL(toggled(bool)),
            this, SLOT(doCropTimeCueChanged(bool)));
    timeCueLayout->addWidget(doCropTimeCue, 2,0);

    timeForTimeCueLabel = new QLabel(tr("Position of cue"), central);
    timeCueLayout->addWidget(timeForTimeCueLabel,3,0);
    timeForTimeCue = new QLineEdit(central);
    connect(timeForTimeCue, SIGNAL(returnPressed()),
            this, SLOT(timeForTimeCueProcessText()));
    timeCueLayout->addWidget(timeForTimeCue, 3,1);

    //
    // Create the misc stuff
    //
    QGroupBox * miscGroup = new QGroupBox(central);
    miscGroup->setTitle(tr("Misc"));
    topLayout->addWidget(miscGroup);

    QGridLayout *miscLayout = new QGridLayout(miscGroup);
    miscLayout->setMargin(5);
    miscLayout->setSpacing(10);
 
    // Create the legend toggle
    legendToggle = new QCheckBox(tr("Legend"), central);
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    miscLayout->addWidget(legendToggle, 0, 0);

    // Create the labels toggle
    labelsToggle = new QCheckBox(tr("Labels"), central);
    connect(labelsToggle, SIGNAL(toggled(bool)),
            this, SLOT(labelsToggled(bool)));
    miscLayout->addWidget(labelsToggle, 0, 1);
}


// ****************************************************************************
// Method: QvisCurvePlotWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue Jul 23 13:34:33 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Tue Dec 23 13:27:22 PST 2003
//   Added pointSize and showPoints. 
//
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Kathleen Bonnell, Thu Oct 27 16:10:29 PDT 2005 
//   Added showLegend.
//   
//   Kathleen Bonnell, Mon Oct 31 17:05:35 PST 2005
//   Added curveColor, made the enabled state of color be dependent upon
//   the value of curveColor.
//
//   Brad Whitlock, Mon Nov 20 14:45:46 PST 2006
//   Added code for new members related to symbol-based rendering.
//
//   Brad Whitlock, Fri Jul 18 10:45:33 PDT 2008
//   Qt 4.
//
//   Hank Childs, Thu Jul 15 18:20:26 PDT 2010
//   Add cue for the current location.
//
//   Kathleen Bonnell, Wed Aug 11 09:34:41 PDT 2010
//   Added pointStride, PointFillMode.  Updated widget 'enabled' dependencies.
//
// ****************************************************************************

void
QvisCurvePlotWindow::UpdateWindow(bool doAll)
{
    QString tempText;
    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }

        switch(i)
        {
          case CurveAttributes::ID_showLines:
            showLines->blockSignals(true);
            showLines->setChecked(atts->GetShowLines());
            lineStyle->setEnabled(atts->GetShowLines());
            lineStyleLabel->setEnabled(atts->GetShowLines());
            lineWidth->setEnabled(atts->GetShowLines());
            lineWidthLabel->setEnabled(atts->GetShowLines());
            showLines->blockSignals(false);
            break;
          case CurveAttributes::ID_lineStyle:
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(atts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;
          case CurveAttributes::ID_lineWidth:
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(atts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
          case CurveAttributes::ID_showPoints:
            {
            showPoints->blockSignals(true);
            showPoints->setChecked(atts->GetShowPoints());
            pointSize->setEnabled(atts->GetShowPoints());
            pointSizeLabel->setEnabled(atts->GetShowPoints());
            symbolType->setEnabled(atts->GetShowPoints());
            symbolTypeLabel->setEnabled(atts->GetShowPoints());
            bool StaticMode = (atts->GetPointFillMode() == CurveAttributes::Static);
            staticButton->setEnabled(atts->GetShowPoints());
            pointStride->setEnabled(atts->GetShowPoints() && StaticMode);
            pointStrideLabel->setEnabled(atts->GetShowPoints() && StaticMode);
            dynamicButton->setEnabled(atts->GetShowPoints());
            symbolDensity->setEnabled(atts->GetShowPoints() && !StaticMode);
            symbolDensityLabel->setEnabled(atts->GetShowPoints() && !StaticMode);
            showPoints->blockSignals(false);
            }
            break;
          case CurveAttributes::ID_symbol:
            symbolType->blockSignals(true);
            symbolType->setCurrentIndex((int)atts->GetSymbol());
            symbolType->blockSignals(false);
            break;
          case CurveAttributes::ID_pointSize:
            tempText.setNum(atts->GetPointSize());
            pointSize->setText(tempText);
            break;
          case CurveAttributes::ID_pointFillMode:
            {
            fillModeGroup->blockSignals(true);
            bool StaticMode = (atts->GetPointFillMode() == CurveAttributes::Static);
            if (StaticMode)
                fillModeGroup->button(0)->setChecked(true);
            else 
                fillModeGroup->button(1)->setChecked(true);
            pointStride->setEnabled(StaticMode && atts->GetShowPoints());
            pointStrideLabel->setEnabled(StaticMode && atts->GetShowPoints());
            symbolDensity->setEnabled(!StaticMode && atts->GetShowPoints());
            symbolDensityLabel->setEnabled(!StaticMode && atts->GetShowPoints());
            fillModeGroup->blockSignals(false);
            }
            break;
          case CurveAttributes::ID_pointStride:
            pointStride->blockSignals(true);
            pointStride->setValue(atts->GetPointStride());
            pointStride->blockSignals(false);
            break;
          case CurveAttributes::ID_symbolDensity:
            symbolDensity->blockSignals(true);
            symbolDensity->setValue(atts->GetSymbolDensity());
            symbolDensity->blockSignals(false);
            break;
          case CurveAttributes::ID_curveColorSource:
            curveColorButtons->blockSignals(true);
            curveColorButtons->button(atts->GetCurveColorSource())->setChecked(true);
            curveColorButtons->blockSignals(false);

            curveColor->setEnabled(atts->GetCurveColorSource());
            break;
          case CurveAttributes::ID_curveColor:
            { // new scope
              if (curveColor->isEnabled())
              {
                QColor temp(atts->GetCurveColor().Red(),
                            atts->GetCurveColor().Green(),
                            atts->GetCurveColor().Blue());
                curveColor->blockSignals(true);
                curveColor->setButtonColor(temp);
                curveColor->blockSignals(false);
              }
            }
            break;
          case CurveAttributes::ID_showLegend:
            legendToggle->setChecked(atts->GetShowLegend());
            break;
          case CurveAttributes::ID_showLabels:
            labelsToggle->setChecked(atts->GetShowLabels());
            break;
          case CurveAttributes::ID_designator: // internal
            break;
         case CurveAttributes::ID_doBallTimeCue:
            if (atts->GetDoBallTimeCue() == true)
            {
                ballTimeCueColor->setEnabled(true);
            }
            else
            {
                ballTimeCueColor->setEnabled(false);
            }
            if (atts->GetDoBallTimeCue() == true)
            {
                timeCueBallSize->setEnabled(true);
                if(timeCueBallSizeLabel)
                    timeCueBallSizeLabel->setEnabled(true);
            }
            else
            {
                timeCueBallSize->setEnabled(false);
                if(timeCueBallSizeLabel)
                    timeCueBallSizeLabel->setEnabled(false);
            }
            doBallTimeCue->blockSignals(true);
            doBallTimeCue->setChecked(atts->GetDoBallTimeCue());
            doBallTimeCue->blockSignals(false);
            break;
          case CurveAttributes::ID_ballTimeCueColor:
            { // new scope
                QColor tempcolor = QColor(atts->GetBallTimeCueColor().Red(),
                                   atts->GetBallTimeCueColor().Green(),
                                   atts->GetBallTimeCueColor().Blue());
                ballTimeCueColor->blockSignals(true);
                ballTimeCueColor->setButtonColor(tempcolor);
                ballTimeCueColor->blockSignals(false);
            }
            break;
          case CurveAttributes::ID_timeCueBallSize:
            timeCueBallSize->setText(DoubleToQString(atts->GetTimeCueBallSize()));
            break;
          case CurveAttributes::ID_doLineTimeCue:
            if (atts->GetDoLineTimeCue() == true)
            {
                lineTimeCueColor->setEnabled(true);
            }
            else
            {
                lineTimeCueColor->setEnabled(false);
            }
            if (atts->GetDoLineTimeCue() == true)
            {
                lineTimeCueWidth->setEnabled(true);
                if(lineTimeCueWidthLabel)
                    lineTimeCueWidthLabel->setEnabled(true);
            }
            else
            {
                lineTimeCueWidth->setEnabled(false);
                if(lineTimeCueWidthLabel)
                    lineTimeCueWidthLabel->setEnabled(false);
            }
            doLineTimeCue->blockSignals(true);
            doLineTimeCue->setChecked(atts->GetDoLineTimeCue());
            doLineTimeCue->blockSignals(false);
            break;
          case CurveAttributes::ID_lineTimeCueColor:
            { // new scope
                QColor tempcolor = QColor(atts->GetLineTimeCueColor().Red(),
                                   atts->GetLineTimeCueColor().Green(),
                                   atts->GetLineTimeCueColor().Blue());
                lineTimeCueColor->blockSignals(true);
                lineTimeCueColor->setButtonColor(tempcolor);
                lineTimeCueColor->blockSignals(false);
            }
            break;
          case CurveAttributes::ID_lineTimeCueWidth:
            lineTimeCueWidth->blockSignals(true);
            lineTimeCueWidth->SetLineWidth(atts->GetLineTimeCueWidth());
            lineTimeCueWidth->blockSignals(false);
            break;
          case CurveAttributes::ID_doCropTimeCue:
            doCropTimeCue->blockSignals(true);
            doCropTimeCue->setChecked(atts->GetDoCropTimeCue());
            doCropTimeCue->blockSignals(false);
            break;
          case CurveAttributes::ID_timeForTimeCue:
            timeForTimeCue->setText(DoubleToQString(atts->GetTimeForTimeCue()));
            break;
        }
    }

    bool shouldEnableWidget = atts->GetDoBallTimeCue() ||
                               atts->GetDoLineTimeCue() ||
                               atts->GetDoCropTimeCue();
    
    timeForTimeCueLabel->setEnabled(shouldEnableWidget);
    timeForTimeCue->setEnabled(shouldEnableWidget);
}


// ****************************************************************************
// Method: QvisCurvePlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Jul 23 13:34:33 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Tue Dec 23 13:27:22 PST 2003
//   Added pointSize.  Removed do-nothing code.
//   
//   Hank Childs, Sat Mar  3 10:33:59 PST 2007
//   Do explicit checking for symbolDensity, since spin boxes don't call
//   the "valueChanged" signal unless you press "Enter".
//
//   Brad Whitlock, Tue Apr 22 16:44:03 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri Jul 18 10:48:24 PDT 2008
//   Qt 4.
//
//   Hank Childs, Thu Jul 15 18:20:26 PDT 2010
//   Add cue for the current location.
//
// ****************************************************************************

void
QvisCurvePlotWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do pointSize
    if(which_widget == CurveAttributes::ID_pointSize || doAll)
    {
        double val;
        if(LineEditGetDouble(pointSize, val))
            atts->SetPointSize(val);
        else
        {
            ResettingError("point size", DoubleToQString(atts->GetPointSize()));
            atts->SetPointSize(atts->GetPointSize());
        }
    }

    // Do pointStride
    if(which_widget == CurveAttributes::ID_pointStride || doAll)
        if (pointStride->value() != atts->GetPointStride())
            atts->SetPointStride(pointStride->value());

    // Do symbolDensity
    if (which_widget == CurveAttributes::ID_symbolDensity || doAll)
        if (symbolDensity->value() != atts->GetSymbolDensity())
            atts->SetSymbolDensity(symbolDensity->value());


    // Do timeCueBallSize
    if(which_widget == CurveAttributes::ID_timeCueBallSize || doAll)
    {
        double val;
        if(LineEditGetDouble(timeCueBallSize, val))
            atts->SetTimeCueBallSize(val);
        else
        {
            ResettingError(tr("Time Cue Ball Size"),
                DoubleToQString(atts->GetTimeCueBallSize()));
            atts->SetTimeCueBallSize(atts->GetTimeCueBallSize());
        }
    }

    // Do timeForTimeCue
    if(which_widget == CurveAttributes::ID_timeForTimeCue || doAll)
    {
        double val;
        if(LineEditGetDouble(timeForTimeCue, val))
            atts->SetTimeForTimeCue(val);
        else
        {
            ResettingError(tr("timeForTimeCue"),
                DoubleToQString(atts->GetTimeForTimeCue()));
            atts->SetTimeForTimeCue(atts->GetTimeForTimeCue());
        }
    }
}


// ****************************************************************************
// Method: QvisCurvePlotWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: xml2window
// Creation:   Tue Jul 23 13:34:33 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisCurvePlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        atts->Notify();

        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        atts->Notify();
}


//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisCurvePlotWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: xml2window
// Creation:   Tue Jul 23 13:34:33 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisCurvePlotWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisCurvePlotWindow::makeDefault
//
// Purpose: 
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: xml2window
// Creation:   Tue Jul 23 13:34:33 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisCurvePlotWindow::makeDefault()
{
    GetCurrentValues(-1);
    atts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisCurvePlotWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: xml2window
// Creation:   Tue Jul 23 13:34:33 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisCurvePlotWindow::reset()
{
    GetViewerMethods()->ResetPlotOptions(plotType);
}


void
QvisCurvePlotWindow::lineStyleChanged(int style)
{
    atts->SetLineStyle(style);
    Apply();
}


void
QvisCurvePlotWindow::lineWidthChanged(int style)
{
    atts->SetLineWidth(style);
    Apply();
}


void
QvisCurvePlotWindow::curveColorChanged(const QColor &color_)
{
    ColorAttribute temp(color_.red(), color_.green(), color_.blue());
    atts->SetCurveColor(temp);
    Apply();
}

void
QvisCurvePlotWindow::curveColorClicked(int val)
{
    // Only do it if it changed.
    if(val != atts->GetCurveColorSource())
    {
        atts->SetCurveColorSource(CurveAttributes::CurveColor(val));
        curveColor->setEnabled(val);
        Apply();
    }
}

void
QvisCurvePlotWindow::labelsToggled(bool val)
{
    atts->SetShowLabels(val);
    Apply();
}

void
QvisCurvePlotWindow::legendToggled(bool val)
{
    atts->SetShowLegend(val);
    Apply();
}

void
QvisCurvePlotWindow::showLinesChanged(bool val)
{
    atts->SetShowLines(val);
    Apply();
}

void
QvisCurvePlotWindow::showPointsChanged(bool val)
{
    atts->SetShowPoints(val);
    Apply();
}

void
QvisCurvePlotWindow::processPointSizeText()
{
    GetCurrentValues(CurveAttributes::ID_pointSize); 
    Apply();
}

void
QvisCurvePlotWindow::pointStrideChanged(int val)
{
    atts->SetPointStride(val);
    SetUpdate(false);
    Apply();
}

void
QvisCurvePlotWindow::symbolTypeChanged(int val)
{
    atts->SetSymbol((CurveAttributes::SymbolTypes)val);
    SetUpdate(false);
    Apply();
}

void
QvisCurvePlotWindow::symbolDensityChanged(int val)
{
    atts->SetSymbolDensity(val);
    SetUpdate(false);
    Apply();
}


void
QvisCurvePlotWindow::fillModeChanged(int val)
{
    switch(val)
    {
      case 0:
        atts->SetPointFillMode(CurveAttributes::Static);
        break;
      case 1:
        atts->SetPointFillMode(CurveAttributes::Dynamic);
        break;
      default:
        EXCEPTION1(ImproperUseException,
                   "The Curve plot received a signal for a fill mode"
                   "that it didn't understand");
        break;
    }
    Apply();
}


void
QvisCurvePlotWindow::doBallTimeCueChanged(bool val)
{
    atts->SetDoBallTimeCue(val);
    Apply();
}


void
QvisCurvePlotWindow::ballTimeCueColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    atts->SetBallTimeCueColor(temp);
    SetUpdate(false);
    Apply();
}


void
QvisCurvePlotWindow::timeCueBallSizeProcessText()
{
    GetCurrentValues(CurveAttributes::ID_timeCueBallSize);
    Apply();
}


void
QvisCurvePlotWindow::doLineTimeCueChanged(bool val)
{
    atts->SetDoLineTimeCue(val);
    Apply();
}


void
QvisCurvePlotWindow::lineTimeCueColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    atts->SetLineTimeCueColor(temp);
    SetUpdate(false);
    Apply();
}


void
QvisCurvePlotWindow::lineTimeCueWidthChanged(int val)
{
    atts->SetLineTimeCueWidth(val);
    Apply();
}


void
QvisCurvePlotWindow::doCropTimeCueChanged(bool val)
{
    atts->SetDoCropTimeCue(val);
    Apply();
}


void
QvisCurvePlotWindow::timeForTimeCueProcessText()
{
    GetCurrentValues(CurveAttributes::ID_timeForTimeCue);
    Apply();
}


