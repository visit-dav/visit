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

#include "QvisCurvePlotWindow.h"

#include <CurveAttributes.h>
#include <ViewerProxy.h>

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

    renderMode = 0;
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
// ****************************************************************************

void
QvisCurvePlotWindow::CreateWindowContents()
{
    //
    // Create the geometry
    //
    QGroupBox * geometryGroup = new QGroupBox(central);
    geometryGroup->setTitle(tr("Geometry"));
    topLayout->addWidget(geometryGroup);

    QGridLayout *geometryLayout = new QGridLayout(geometryGroup);
    geometryLayout->setMargin(5);
    geometryLayout->setSpacing(10);
 

    renderMode = new QButtonGroup(central);
    connect(renderMode, SIGNAL(buttonClicked(int)),
            this, SLOT(renderModeChanged(int)));
    QRadioButton *rb0 = new QRadioButton(tr("Draw curve using lines"), central);
    renderMode->addButton(rb0, 0);
    geometryLayout->addWidget(rb0, 0, 0, 1, 4);
    QRadioButton *rb1 = new QRadioButton(tr("Draw curve using symbols"), central);
    renderMode->addButton(rb1, 1);
    geometryLayout->addWidget(rb1, 3, 0, 1, 4);

    geometryLayout->addWidget(new QLabel("     ", central), 1, 0);

    //
    // Create line related controls.
    //
    lineStyle = new QvisLineStyleWidget(0, central);
    lineStyleLabel = new QLabel(tr("Line style"), central);
    lineStyleLabel->setBuddy(lineStyle);
    geometryLayout->addWidget(lineStyleLabel, 1, 1);
    geometryLayout->addWidget(lineStyle, 1, 2);

    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));

    lineWidth = new QvisLineWidthWidget(0, central);
    lineWidthLabel = new QLabel(tr("Line width"), central);
    lineWidthLabel->setBuddy(lineWidth);
    geometryLayout->addWidget(lineWidthLabel, 1, 3);
    geometryLayout->addWidget(lineWidth, 1, 4);

    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));

    showPoints = new QCheckBox(tr("Show points"), central);
    geometryLayout->addWidget(showPoints, 2, 1);

    connect(showPoints, SIGNAL(toggled(bool)),
            this, SLOT(showPointsChanged(bool)));

    // Create the point size line edit
    pointSize = new QNarrowLineEdit(central);
    pointSizeLabel = new QLabel(tr("Point size"), central);
    pointSizeLabel->setBuddy(pointSize);
    geometryLayout->addWidget(pointSizeLabel, 2, 2,
                              Qt::AlignRight | Qt::AlignVCenter);
    geometryLayout->addWidget(pointSize, 2, 3);

    connect(pointSize, SIGNAL(returnPressed()),
            this, SLOT(processPointSizeText())); 

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
    symbolType->setMinimumHeight(35);
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
    geometryLayout->addWidget(symbolTypeLabel, 4, 1);
    geometryLayout->addWidget(symbolType, 4, 2);

    symbolDensity = new QSpinBox(central);
    symbolDensity->setMinimum(10);
    symbolDensity->setMaximum(1000);
    connect(symbolDensity, SIGNAL(valueChanged(int)),
            this, SLOT(symbolDensityChanged(int)));
    symbolDensityLabel = new QLabel(tr("Density"), central);
    symbolDensityLabel->setBuddy(symbolDensity);
    geometryLayout->addWidget(symbolDensityLabel, 4, 3);
    geometryLayout->addWidget(symbolDensity, 4, 4);

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

    QRadioButton * rb = new QRadioButton(tr("Cycle"), central);
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
          case CurveAttributes::ID_showLabels:
            labelsToggle->setChecked(atts->GetShowLabels());
            break;
          case CurveAttributes::ID_designator: // internal
            break;
          case CurveAttributes::ID_showPoints:
            showPoints->blockSignals(true);
            showPoints->setChecked(atts->GetShowPoints());
            pointSize->setEnabled(atts->GetShowPoints());
            pointSizeLabel->setEnabled(atts->GetShowPoints());
            showPoints->blockSignals(false);
            break;
          case CurveAttributes::ID_pointSize:
            tempText.setNum(atts->GetPointSize());
            pointSize->setText(tempText);
            break;
          case CurveAttributes::ID_showLegend:
            legendToggle->setChecked(atts->GetShowLegend());
            break;

          case CurveAttributes::ID_curveColorSource:
            curveColorButtons->blockSignals(true);
            curveColorButtons->button(atts->GetCurveColorSource())->setChecked(true);
            curveColorButtons->blockSignals(false);

            curveColor->setEnabled(atts->GetCurveColorSource());
            break;
          case CurveAttributes::ID_renderMode:
            {
            renderMode->blockSignals(true);
            renderMode->button((int)atts->GetRenderMode())->setChecked(true);
            renderMode->blockSignals(false);

            bool asLines = atts->GetRenderMode() == CurveAttributes::RenderAsLines;
            showPoints->setEnabled(asLines);
            lineStyle->setEnabled(asLines);
            lineStyleLabel->setEnabled(asLines);
            lineWidth->setEnabled(asLines);
            lineWidthLabel->setEnabled(asLines);
            pointSize->setEnabled(asLines && atts->GetShowPoints());
            pointSizeLabel->setEnabled(asLines && atts->GetShowPoints());

            symbolType->setEnabled(!asLines);
            symbolTypeLabel->setEnabled(!asLines);
            symbolDensity->setEnabled(!asLines);
            symbolDensityLabel->setEnabled(!asLines);
            }
            break;
          case CurveAttributes::ID_symbol:
            symbolType->blockSignals(true);
            symbolType->setCurrentIndex((int)atts->GetSymbol());
            symbolType->blockSignals(false);
            break;
          case CurveAttributes::ID_symbolDensity:
            symbolDensity->blockSignals(true);
            symbolDensity->setValue(atts->GetSymbolDensity());
            symbolDensity->blockSignals(false);
            break;
        }
    }
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

    if (which_widget == CurveAttributes::ID_symbolDensity || doAll)
        if (symbolDensity->value() != atts->GetSymbolDensity())
            atts->SetSymbolDensity(symbolDensity->value());
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
QvisCurvePlotWindow::renderModeChanged(int val)
{
    atts->SetRenderMode((CurveAttributes::RenderMode)val);
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


