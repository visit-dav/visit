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

#include "QvisCurvePlotWindow.h"

#include <CurveAttributes.h>
#include <ViewerProxy.h>

#include <QButtonGroup>
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
//   Added cycleColors. 
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
    QGridLayout *mainLayout = new QGridLayout(0);
    topLayout->addLayout(mainLayout);

    renderMode = new QButtonGroup(central);
    connect(renderMode, SIGNAL(buttonClicked(int)),
            this, SLOT(renderModeChanged(int)));
    QRadioButton *rb0 = new QRadioButton(tr("Draw curve using lines"), central);
    renderMode->addButton(rb0, 0);
    mainLayout->addWidget(rb0, 0, 0, 1, 4);
    QRadioButton *rb1 = new QRadioButton(tr("Draw curve using symbols"), central);
    renderMode->addButton(rb1, 1);
    mainLayout->addWidget(rb1, 5, 0, 1, 4);
    mainLayout->addWidget(new QLabel("     ", central), 1, 0);

    //
    // Create line related controls.
    //
    lineStyle = new QvisLineStyleWidget(0, central);
    lineStyleLabel = new QLabel(tr("Line style"), central);
    lineStyleLabel->setBuddy(lineStyle);
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    mainLayout->addWidget(lineStyleLabel, 1, 1);
    mainLayout->addWidget(lineStyle, 1, 2, 1, 2);

    lineWidth = new QvisLineWidthWidget(0, central);
    lineWidthLabel = new QLabel(tr("Line width"), central);
    lineWidthLabel->setBuddy(lineWidth);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    mainLayout->addWidget(lineWidthLabel,2, 1);
    mainLayout->addWidget(lineWidth, 2, 2, 1, 2);

    showPoints = new QCheckBox(tr("Show points"), central);
    connect(showPoints, SIGNAL(toggled(bool)),
            this, SLOT(showPointsChanged(bool)));
    mainLayout->addWidget(showPoints, 3, 1, 1, 2);

    // Create the point size line edit
    pointSize = new QNarrowLineEdit(central);
    connect(pointSize, SIGNAL(returnPressed()),
            this, SLOT(processPointSizeText())); 
    mainLayout->addWidget(pointSize, 4, 2, 1, 2);
    pointSizeLabel = new QLabel(tr("Point size"), central);
    pointSizeLabel->setBuddy(pointSize);
    mainLayout->addWidget(pointSizeLabel, 4, 1);

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
    mainLayout->addWidget(symbolTypeLabel, 6, 1);
    mainLayout->addWidget(symbolType, 6, 2, 1, 2);

    symbolDensity = new QSpinBox(central);
    symbolDensity->setMinimum(10);
    symbolDensity->setMaximum(1000);
    connect(symbolDensity, SIGNAL(valueChanged(int)),
            this, SLOT(symbolDensityChanged(int)));
    symbolDensityLabel = new QLabel(tr("Density"), central);
    symbolDensityLabel->setBuddy(symbolDensity);
    mainLayout->addWidget(symbolDensityLabel, 7, 1);
    mainLayout->addWidget(symbolDensity, 7, 2, 1, 2);

    //
    // Add color controls
    // 
    cycleColors = new QCheckBox(tr("Cycle colors"), central);
    connect(cycleColors, SIGNAL(toggled(bool)),
            this, SLOT(cycleColorsChanged(bool)));
    mainLayout->addWidget(cycleColors, 8, 0, 1, 2);

    colorLabel = new QLabel(tr("Color"), central);
    color = new QvisColorButton(central);
    color->setButtonColor(QColor(255, 0, 0));
    connect(color, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(colorChanged(const QColor &)));
    mainLayout->addWidget(colorLabel, 8, 2);
    mainLayout->addWidget(color, 8, 3);

    //
    // Global controls
    //
    showLegend = new QCheckBox(tr("Legend"), central);
    connect(showLegend, SIGNAL(toggled(bool)),
            this, SLOT(showLegendChanged(bool)));
    mainLayout->addWidget(showLegend, 9, 0, 1, 2);

    showLabels = new QCheckBox(tr("Labels"), central);
    connect(showLabels, SIGNAL(toggled(bool)),
            this, SLOT(showLabelsChanged(bool)));
    mainLayout->addWidget(showLabels, 9, 2);
    mainLayout->setColumnStretch(3, 10);
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
//   Added cycleColors, made the enabled state of color be dependent upon
//   the value of cycleColors.
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
          case CurveAttributes::ID_color:
            { // new scope
              if (color->isEnabled())
              {
                QColor temp(atts->GetColor().Red(),
                            atts->GetColor().Green(),
                            atts->GetColor().Blue());
                color->blockSignals(true);
                color->setButtonColor(temp);
                color->blockSignals(false);
              }
            }
            break;
          case CurveAttributes::ID_showLabels:
            showLabels->setChecked(atts->GetShowLabels());
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
            showLegend->setChecked(atts->GetShowLegend());
            break;
          case CurveAttributes::ID_cycleColors:
            cycleColors->setChecked(atts->GetCycleColors());
            color->setEnabled(!atts->GetCycleColors()); 
            colorLabel->setEnabled(!atts->GetCycleColors()); 
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
QvisCurvePlotWindow::colorChanged(const QColor &color_)
{
    ColorAttribute temp(color_.red(), color_.green(), color_.blue());
    atts->SetColor(temp);
    Apply();
}

void
QvisCurvePlotWindow::cycleColorsChanged(bool val)
{
    atts->SetCycleColors(val);
    Apply();
}

void
QvisCurvePlotWindow::showLabelsChanged(bool val)
{
    atts->SetShowLabels(val);
    Apply();
}

void
QvisCurvePlotWindow::showLegendChanged(bool val)
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


