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

#include "QvisHistogramPlotWindow.h"

#include <HistogramAttributes.h>
#include <ViewerProxy.h>

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QvisColorTableButton.h>
#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisVariableButton.h>

#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisHistogramPlotWindow::QvisHistogramPlotWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Thu Mar 8 08:20:00 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisHistogramPlotWindow::QvisHistogramPlotWindow(const int type,
                         HistogramAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad)
{
    plotType = type;
    atts = subj;
}


// ****************************************************************************
// Method: QvisHistogramPlotWindow::~QvisHistogramPlotWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Thu Mar 8 08:20:00 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisHistogramPlotWindow::~QvisHistogramPlotWindow()
{
}


// ****************************************************************************
// Method: QvisHistogramPlotWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Cyrus Harrison - generated with xml2window
// Creation:   Thu Mar 8 08:20:00 PDT 2007
//
// Modifications:
// 
//    Dave Pugmire, Thu Nov 01 12:39:07 EDT 2007
//    Support for log, sqrt scaling.
//    
//    Hank Childs, Tue Dec 11 20:01:14 PST 2007
//    Add support for scaling by an arbitrary variable.
//
//    Brad Whitlock, Tue Apr 22 16:48:22 PDT 2008
//    Added tr()'s
//
//    Cyrus Harrison, Fri Jul 18 14:38:14 PDT 2008
//    Initial Qt4 Port.
//
//    Dave Pugmire, Wed Oct 29 16:00:48 EDT 2008
//    Swap the min/max in the gui.
//
// ****************************************************************************

void
QvisHistogramPlotWindow::CreateWindowContents()
{
    // Figure out the max width that we want to allow for some simple
    // line edit widgets.
    int maxWidth = fontMetrics().width("1.0000000000");
    
    QGridLayout *mainLayout = new QGridLayout();
    topLayout->addLayout(mainLayout);

    basedOnLabel = new QLabel(tr("Histogram based on"), central);
    mainLayout->addWidget(basedOnLabel,0,0);

    QWidget *basedOnWidget = new QWidget(central);
    QHBoxLayout *basedOnLayout = new QHBoxLayout(basedOnWidget);
    
    QRadioButton *basedOnBasedOnManyVarsForSingleZone = new QRadioButton(tr("Array of variables (one zone)"), 
                                                                         basedOnWidget);
    QRadioButton *basedOnBasedOnManyZonesForSingleVar = new QRadioButton(tr("Many zones"),
                                                                         basedOnWidget);
    basedOnLayout->addWidget(basedOnBasedOnManyVarsForSingleZone);
    basedOnLayout->addWidget(basedOnBasedOnManyZonesForSingleVar);
    
    basedOnGroup = new QButtonGroup(basedOnWidget);
    basedOnGroup->addButton(basedOnBasedOnManyVarsForSingleZone,0);
    basedOnGroup->addButton(basedOnBasedOnManyZonesForSingleVar,1);
    
    
    connect(basedOnGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(basedOnChanged(int)));
    mainLayout->addWidget(basedOnWidget, 0,1);
    basedOnLabel->setEnabled(false);
    basedOnWidget->setEnabled(false);
    
    // Histogram Style Group Box
    histGroupBox =new QGroupBox(central); 
    histGroupBox->setTitle(tr("Histogram Options"));
    mainLayout->addWidget(histGroupBox, 1, 0, 1, 2);
    QVBoxLayout *hgTopLayout = new QVBoxLayout(histGroupBox);
    QGridLayout *hgLayout = new QGridLayout();
    hgTopLayout->addLayout(hgLayout);
    hgLayout->setColumnStretch(1,10);

    histogramTypeLabel = new QLabel(tr("Bin contribution"), histGroupBox);
    hgLayout->addWidget(histogramTypeLabel,0,0);

    
    histogramTypeWidget = new QWidget(central);
    QHBoxLayout *histogramTypeLayout = new QHBoxLayout(histogramTypeWidget);
    histogramTypeLayout->setMargin(0);
    
    QRadioButton *histogramTypeBinContributionFrequency = new QRadioButton(tr("Frequency"),
                                                                           histogramTypeWidget);
    QRadioButton *histogramTypeBinContributionWeighted = new QRadioButton(tr("Weighted"),
                                                                          histogramTypeWidget);
    histogramTypeGroup = new QButtonGroup(histogramTypeWidget);
    histogramTypeGroup->addButton(histogramTypeBinContributionFrequency,0);
    histogramTypeGroup->addButton(histogramTypeBinContributionWeighted,1);
    
    histogramTypeLayout->addWidget(histogramTypeBinContributionFrequency);
    histogramTypeLayout->addWidget(histogramTypeBinContributionWeighted);
    
    connect(histogramTypeGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(histogramTypeChanged(int)));
    hgLayout->addWidget(histogramTypeWidget, 0,1);


    weightTypeLabel = new QLabel(tr("Weighted by"), histGroupBox);
    hgLayout->addWidget(weightTypeLabel,1,0);
    
    weightTypeWidget = new QWidget(histGroupBox);
    QHBoxLayout *weightTypeLayout = new QHBoxLayout(weightTypeWidget);
    weightTypeLayout->setMargin(0);
    
    QRadioButton *weightTypeVolumeArea = new QRadioButton(tr("Area (2D) / Volume (3D)"), 
                                                          weightTypeWidget);
    QRadioButton *weightTypeVariable = new QRadioButton(tr("Variable"),
                                                        weightTypeWidget);
    
    weightTypeGroup = new QButtonGroup(weightTypeWidget);
    weightTypeGroup->addButton(weightTypeVolumeArea,0);
    weightTypeGroup->addButton(weightTypeVariable,1);
    
    weightTypeLayout->addWidget(weightTypeVolumeArea);
    weightTypeLayout->addWidget(weightTypeVariable);
    
    connect(weightTypeGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(weightTypeChanged(int)));
    hgLayout->addWidget(weightTypeWidget, 1,1);

    weightVariableLabel = new QLabel(tr("Variable to Weight By"), histGroupBox);
    hgLayout->addWidget(weightVariableLabel,2,0);
    int weightVariableMask = QvisVariableButton::Scalars;
    weightVariable = new QvisVariableButton(true, true, true, weightVariableMask, histGroupBox);
    weightVariable->setDefaultVariable("default");
    connect(weightVariable, SIGNAL(activated(const QString&)),
            this, SLOT(weightVariableChanged(const QString&)));
    hgLayout->addWidget(weightVariable, 2,1);

    // Add data scale
    QLabel *dataScaleLabel = new QLabel(tr("Data Scale"), histGroupBox);
    hgLayout->addWidget(dataScaleLabel,3,0);

    
    QWidget     *dataScaleWidget =new QWidget(histGroupBox);
    QHBoxLayout *dataScaleLayout = new QHBoxLayout(dataScaleWidget);
    
    dataScaleLayout->setMargin(0);
    QRadioButton *linearScale = new QRadioButton(tr("Linear"), dataScaleWidget);
    QRadioButton *logScale = new QRadioButton(tr("Log10"), dataScaleWidget);
    QRadioButton *sqrtScale = new QRadioButton(tr("Square root"), dataScaleWidget);
    
    dataScaleGroup = new QButtonGroup(dataScaleWidget);
    dataScaleGroup->addButton(linearScale,0);
    dataScaleGroup->addButton(logScale,1);
    dataScaleGroup->addButton(sqrtScale,2);
    
    dataScaleLayout->addWidget(linearScale);
    dataScaleLayout->addWidget(logScale);
    dataScaleLayout->addWidget(sqrtScale);
    
    connect(dataScaleGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(dataScaleChanged(int)));    
    hgLayout->addWidget(dataScaleWidget, 3,1);

    specifyRange = new QCheckBox(tr("Specify Range?"), histGroupBox);
    connect(specifyRange, SIGNAL(toggled(bool)),
            this, SLOT(specifyRangeChanged(bool)));
    hgLayout->addWidget(specifyRange, 4,0);

    maxLabel = new QLabel(tr("Maximum"), histGroupBox);
    hgLayout->addWidget(maxLabel,5,0);
    max = new QLineEdit(histGroupBox);
    connect(max, SIGNAL(returnPressed()),
            this, SLOT(maxProcessText()));
    hgLayout->addWidget(max, 5,1);

    minLabel = new QLabel(tr("Minimum"), histGroupBox);
    hgLayout->addWidget(minLabel,6,0);
    min = new QLineEdit(histGroupBox);
    connect(min, SIGNAL(returnPressed()),
            this, SLOT(minProcessText()));
    hgLayout->addWidget(min,6,1);

    numBinsLabel = new QLabel(tr("Number of Bins"), histGroupBox);
    hgLayout->addWidget(numBinsLabel,7,0);

    numBins = new QLineEdit(histGroupBox);
    connect(numBins, SIGNAL(returnPressed()),
            this, SLOT(numBinsProcessText()));
    hgLayout->addWidget(numBins, 7,1);

    QLabel *binScaleLabel = new QLabel(tr("Bin Scale"), histGroupBox);
    hgLayout->addWidget(binScaleLabel,8,0);
    
    QWidget     *binsScaleWidget = new QWidget(histGroupBox);
    QHBoxLayout *binsScaleLayout = new QHBoxLayout(binsScaleWidget);
    
    binsScaleLayout->setMargin(0);
    QRadioButton *binLinearScale = new QRadioButton(tr("Linear"), binsScaleWidget);
    QRadioButton *binLogScale = new QRadioButton(tr("Log10"), binsScaleWidget);
    QRadioButton *binSqrtScale = new QRadioButton(tr("Square root"), binsScaleWidget);
    
    binsScaleGroup = new QButtonGroup(binsScaleWidget);
    binsScaleGroup->addButton(binLinearScale,0);
    binsScaleGroup->addButton(binLogScale,1);
    binsScaleGroup->addButton(binSqrtScale,2);
    
    binsScaleLayout->addWidget(binLinearScale);
    binsScaleLayout->addWidget(binLogScale);
    binsScaleLayout->addWidget(binSqrtScale);
    
    hgLayout->addWidget(binsScaleWidget, 8,1);    
    
    connect(binsScaleGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(binsScaleChanged(int)));    

    // Bar Plot Group Box
    barGroupBox =new QGroupBox(central); 
    barGroupBox->setTitle(tr("Single Zone Plot Options"));
    mainLayout->addWidget(barGroupBox, 2, 0, 1, 2);
    QVBoxLayout *bgTopLayout = new QVBoxLayout(barGroupBox);

    QGridLayout *bgLayout = new QGridLayout();
    bgTopLayout->addLayout(bgLayout);
    bgLayout->setColumnStretch(1,10);

    domainLabel = new QLabel(tr("domain"), barGroupBox);
    bgLayout->addWidget(domainLabel,0,0);

    domain = new QLineEdit(barGroupBox);
    connect(domain, SIGNAL(returnPressed()),
            this, SLOT(domainProcessText()));
    bgLayout->addWidget(domain, 0,1);

    zoneLabel = new QLabel(tr("zone"), barGroupBox);
    bgLayout->addWidget(zoneLabel,1,0);

    zone = new QLineEdit(barGroupBox);
    connect(zone, SIGNAL(returnPressed()),
            this, SLOT(zoneProcessText()));
    bgLayout->addWidget(zone, 1,1);

    useBinWidths = new QCheckBox(tr("Use bin widths?"), barGroupBox);
    connect(useBinWidths, SIGNAL(toggled(bool)),
            this, SLOT(useBinWidthsChanged(bool)));
    bgLayout->addWidget(useBinWidths, 2,0);


    // Plot Syle Group Box

    // Bar Plot Group Box
    styleGroupBox =new QGroupBox(central); 
    styleGroupBox->setTitle(tr("Plot Style"));
    mainLayout->addWidget(styleGroupBox, 3, 0, 1, 2);
    QVBoxLayout *sgTopLayout = new QVBoxLayout(styleGroupBox);
    QGridLayout *sgLayout= new QGridLayout();
    sgTopLayout->addLayout(sgLayout);
    sgLayout->setColumnStretch(2,10);
    
    // Add output type

    QWidget     *outputTypeWidget = new QWidget(styleGroupBox);
    QHBoxLayout *outputTypeLayout = new QHBoxLayout(outputTypeWidget);
    outputTypeLayout->setMargin(0);
    
    QRadioButton *outputTypeOutputTypeCurve = new QRadioButton(tr("Curve"), outputTypeWidget);
    QRadioButton *outputTypeOutputTypeBlock = new QRadioButton(tr("Block"), outputTypeWidget);
    
    outputTypeGroup = new QButtonGroup(outputTypeWidget);
    outputTypeGroup->addButton(outputTypeOutputTypeCurve,0);
    outputTypeGroup->addButton(outputTypeOutputTypeBlock,1);
    
    outputTypeLayout->addWidget(outputTypeOutputTypeCurve);
    outputTypeLayout->addWidget(outputTypeOutputTypeBlock,1);
    
    connect(outputTypeGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(outputTypeChanged(int)));

    sgLayout->addWidget(outputTypeWidget, 0,1,1,2);

    outputTypeLabel = new QLabel(tr("Type of Output"), styleGroupBox);
    sgLayout->addWidget(outputTypeLabel,0,0, Qt::AlignRight | Qt::AlignVCenter);

    // Add Line Style
    lineStyleLabel = new QLabel(tr("Line Style"), styleGroupBox);
    sgLayout->addWidget(lineStyleLabel,1,0,Qt::AlignRight | Qt::AlignVCenter);

    lineStyle = new QvisLineStyleWidget(0, styleGroupBox);
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    sgLayout->addWidget(lineStyle, 1,1,1,2);

    // Add Line Width
    lineWidthLabel = new QLabel(tr("Line Width"), styleGroupBox);
    sgLayout->addWidget(lineWidthLabel,2,0,Qt::AlignRight | Qt::AlignVCenter);

    lineWidth = new QvisLineWidthWidget(0, styleGroupBox);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    sgLayout->addWidget(lineWidth, 2,1,1,2);

    // Add Color Selector
    colorLabel = new QLabel(tr("Color"), styleGroupBox);
    sgLayout->addWidget(colorLabel,3,0,Qt::AlignRight | Qt::AlignVCenter);

    color = new QvisColorButton(styleGroupBox);
    connect(color, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(colorChanged(const QColor&)));
    sgLayout->addWidget(color, 3,1);

}


// ****************************************************************************
// Method: QvisHistogramPlotWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Cyrus Harrison - generated using xml2window
// Creation:   Thu Mar 8 08:20:00 PDT 2007
//
// Modifications:
//
//    Dave Pugmire, Thu Nov 01 12:39:07 EDT 2007
//    Support for log, sqrt scaling.
//    
//    Hank Childs, Tue Dec 11 20:01:14 PST 2007
//    Add support for scaling by an arbitrary variable.
//
//    Kathleen Bonnell, Wed Jun 4 07:58:17 PDT 2008
//    Removed unreferenced variables.
//
//    Cyrus Harrison, Fri Jul 18 14:38:14 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************

void
QvisHistogramPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }

        QColor                tempcolor;
        switch(i)
        {
          case HistogramAttributes::ID_basedOn:
            if (atts->GetBasedOn() == HistogramAttributes::ManyZonesForSingleVar)
            {
                histGroupBox->setEnabled(true);
                barGroupBox->setEnabled(false);
                histogramTypeWidget->setEnabled(true);
                histogramTypeLabel->setEnabled(true);

                if(atts->GetHistogramType() == HistogramAttributes::Weighted)
                {
                    weightTypeWidget->setEnabled(true);
                    weightTypeLabel->setEnabled(true);
                }
                else
                {
                    weightTypeWidget->setEnabled(false);
                    weightTypeLabel->setEnabled(false);
                }
            }
            else
            {
                histGroupBox->setEnabled(false);
                barGroupBox->setEnabled(true);
                histogramTypeWidget->setEnabled(false);
                histogramTypeLabel->setEnabled(false);
                weightTypeWidget->setEnabled(false);
                weightTypeLabel->setEnabled(false);
            }
            if (atts->GetBasedOn() == HistogramAttributes::ManyZonesForSingleVar)
            {
                specifyRange->setEnabled(true);
            }
            else
            {
                specifyRange->setEnabled(false);
            }
            if (atts->GetBasedOn() == HistogramAttributes::ManyZonesForSingleVar)
            {
                numBins->setEnabled(true);
                numBinsLabel->setEnabled(true);
            }
            else
            {
                numBins->setEnabled(false);
                numBinsLabel->setEnabled(false);
            }
            if (atts->GetBasedOn() == HistogramAttributes::ManyVarsForSingleZone)
            {
                domain->setEnabled(true);
                domainLabel->setEnabled(true);
            }
            else
            {
                domain->setEnabled(false);
                domainLabel->setEnabled(false);
            }
            if (atts->GetBasedOn() == HistogramAttributes::ManyVarsForSingleZone)
            {
                zone->setEnabled(true);
                zoneLabel->setEnabled(true);
            }
            else
            {
                zone->setEnabled(false);
                zoneLabel->setEnabled(false);
            }
            if (atts->GetBasedOn() == HistogramAttributes::ManyVarsForSingleZone)
            {
                useBinWidths->setEnabled(true);
            }
            else
            {
                useBinWidths->setEnabled(false);
            }
            basedOnGroup->blockSignals(true);
            basedOnGroup->button(atts->GetBasedOn())->setChecked(true);
            basedOnGroup->blockSignals(false);
            break;
          case HistogramAttributes::ID_histogramType:
            if (atts->GetHistogramType() == HistogramAttributes::Weighted ||
                atts->GetHistogramType() == HistogramAttributes::Variable)
            {
                weightTypeWidget->setEnabled(true);
                weightTypeLabel->setEnabled(true);
            }
            else
            {
                weightTypeWidget->setEnabled(false);
                weightTypeLabel->setEnabled(false);
            }
            if (atts->GetHistogramType() == HistogramAttributes::Variable)
            {
                weightVariable->setEnabled(true);
                if(weightVariableLabel)
                    weightVariableLabel->setEnabled(true);
            }
            else
            {
                weightVariable->setEnabled(false);
                if(weightVariableLabel)
                    weightVariableLabel->setEnabled(false);
            }
            histogramTypeGroup->blockSignals(true);
            if (atts->GetHistogramType() == HistogramAttributes::Frequency)
                histogramTypeGroup->button(0)->setChecked(true);
            else if (atts->GetHistogramType() == HistogramAttributes::Weighted ||
                     atts->GetHistogramType() == HistogramAttributes::Variable)
                histogramTypeGroup->button(1)->setChecked(true);
            histogramTypeGroup->blockSignals(false);
            if (atts->GetHistogramType() == HistogramAttributes::Weighted ||
                atts->GetHistogramType() == HistogramAttributes::Variable)
            {
                weightTypeGroup->blockSignals(true);
                if (atts->GetHistogramType() == HistogramAttributes::Weighted)
                    weightTypeGroup->button(0)->setChecked(true);
                else
                    weightTypeGroup->button(1)->setChecked(true);
                weightTypeGroup->blockSignals(false);
            }
            break;
          case HistogramAttributes::ID_weightVariable:
            weightVariable->blockSignals(true);
            weightVariable->setText(atts->GetWeightVariable().c_str());
            weightVariable->blockSignals(false);
            break;
          case HistogramAttributes::ID_specifyRange:
            if (atts->GetSpecifyRange() == true)
            {
                min->setEnabled(true);
                minLabel->setEnabled(true);
            }
            else
            {
                min->setEnabled(false);
                minLabel->setEnabled(false);
            }
            if (atts->GetSpecifyRange() == true)
            {
                max->setEnabled(true);
                maxLabel->setEnabled(true);
            }
            else
            {
                max->setEnabled(false);
                maxLabel->setEnabled(false);
            }
            specifyRange->blockSignals(true);
            specifyRange->setChecked(atts->GetSpecifyRange());
            specifyRange->blockSignals(false);
            break;
          case HistogramAttributes::ID_min:
            min->blockSignals(true);
            temp.setNum(atts->GetMin());
            min->setText(temp);
            min->blockSignals(false);
            break;
          case HistogramAttributes::ID_max:
            max->blockSignals(true);
            temp.setNum(atts->GetMax());
            max->setText(temp);
            max->blockSignals(false);
            break;
          case HistogramAttributes::ID_numBins:
            numBins->blockSignals(true);
            temp.setNum(atts->GetNumBins());
            numBins->setText(temp);
            numBins->blockSignals(false);
            break;
          case HistogramAttributes::ID_domain:
            domain->blockSignals(true);
            temp.setNum(atts->GetDomain());
            domain->setText(temp);
            domain->blockSignals(false);
            break;
          case HistogramAttributes::ID_zone:
            zone->blockSignals(true);
            temp.setNum(atts->GetZone());
            zone->setText(temp);
            zone->blockSignals(false);
            break;
          case HistogramAttributes::ID_useBinWidths:
            useBinWidths->blockSignals(true);
            useBinWidths->setChecked(atts->GetUseBinWidths());
            useBinWidths->blockSignals(false);
            break;
          case HistogramAttributes::ID_outputType:
            outputTypeGroup->blockSignals(true);
            outputTypeGroup->button(atts->GetOutputType())->setChecked(true);
            outputTypeGroup->blockSignals(false);
            break;
          case HistogramAttributes::ID_lineStyle:
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(atts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;
          case HistogramAttributes::ID_lineWidth:
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(atts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
          case HistogramAttributes::ID_color:
            tempcolor = QColor(atts->GetColor().Red(),
                               atts->GetColor().Green(),
                               atts->GetColor().Blue());
            color->blockSignals(true);
            color->setButtonColor(tempcolor);
            color->blockSignals(false);
            break;
        case HistogramAttributes::ID_dataScale:
            dataScaleGroup->blockSignals(true);
            dataScaleGroup->button(atts->GetDataScale())->setChecked(true);
            dataScaleGroup->blockSignals(false);
            break;
        case HistogramAttributes::ID_binScale:
            binsScaleGroup->blockSignals(true);
            binsScaleGroup->button(atts->GetBinScale())->setChecked(true);
            binsScaleGroup->blockSignals(false);
            break;        
        }
    }
}


// ****************************************************************************
// Method: QvisHistogramPlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Thu Mar 8 08:20:00 PDT 2007
//
// Modifications:
//
//    Dave Pugmire, Thu Nov 01 12:39:07 EDT 2007
//    Support for log, sqrt scaling.
//
//    Hank Childs, Tue Dec 11 20:01:14 PST 2007
//    Add support for scaling by an arbitrary variable.
//
//    Brad Whitlock, Tue Apr 22 16:52:56 PDT 2008
//    Support for internationalization.
//
//    Cyrus Harrison, Fri Jul 18 14:38:14 PDT 2008
//    Initial Qt4 Port.
//
//    Brad Whitlock, Mon Aug 11 11:56:58 PDT 2008
//    Changed to new style.
//
// ****************************************************************************

void
QvisHistogramPlotWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do min
    if(which_widget == HistogramAttributes::ID_min || doAll)
    {
        double val;
        if(LineEditGetDouble(min, val))
            atts->SetMin(val);
        else
        {
            ResettingError(tr("minimum"),
                DoubleToQString(atts->GetMin()));
            atts->SetMin(atts->GetMin());
        }
    }

    // Do max
    if(which_widget == HistogramAttributes::ID_max || doAll)
    {
        double val;
        if(LineEditGetDouble(max, val))
            atts->SetMax(val);
        else
        {
            ResettingError(tr("maximum"),
                DoubleToQString(atts->GetMax()));
            atts->SetMax(atts->GetMax());
        }
    }

    // Do numBins
    if(which_widget == HistogramAttributes::ID_numBins || doAll)
    {
        int val;
        if(LineEditGetInt(numBins, val))
            atts->SetNumBins(val);
        else
        {
            ResettingError(tr("number of bins"),
                  IntToQString(atts->GetNumBins()));
            atts->SetNumBins(atts->GetNumBins());
        }
    }

    // Do domain
    if(which_widget == HistogramAttributes::ID_domain || doAll)
    {
        int val;
        if(LineEditGetInt(domain, val))
            atts->SetDomain(val);
        else
        {
            ResettingError(tr("domain"),
                IntToQString(atts->GetDomain()));
            atts->SetDomain(atts->GetDomain());
        }
    }

    // Do zone
    if(which_widget == HistogramAttributes::ID_zone || doAll)
    {
        int val;
        if(LineEditGetInt(zone, val))
            atts->SetZone(val);
        else
        {
            ResettingError(tr("zone"),
                IntToQString(atts->GetZone()));
            atts->SetZone(atts->GetZone());
        }
    }
}


// ****************************************************************************
// Method: QvisHistogramPlotWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: xml2window
// Creation:   Thu Mar 8 08:20:00 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramPlotWindow::Apply(bool ignore)
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
// Method: QvisHistogramPlotWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: xml2window
// Creation:   Thu Mar 8 08:20:00 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramPlotWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisHistogramPlotWindow::makeDefault
//
// Purpose: 
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: xml2window
// Creation:   Thu Mar 8 08:20:00 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramPlotWindow::makeDefault()
{
    GetCurrentValues(-1);
    atts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisHistogramPlotWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: xml2window
// Creation:   Thu Mar 8 08:20:00 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramPlotWindow::reset()
{
    GetViewerMethods()->ResetPlotOptions(plotType);
}


void
QvisHistogramPlotWindow::basedOnChanged(int val)
{
    if(val != atts->GetBasedOn())
    {
        atts->SetBasedOn(HistogramAttributes::BasedOn(val));
        Apply();
    }
}


void
QvisHistogramPlotWindow::histogramTypeChanged(int val)
{
    if (val != atts->GetHistogramType())
    {
        if (val == 0)
            atts->SetHistogramType(HistogramAttributes::Frequency);
        else
        {
            if (weightTypeGroup->checkedId() == 0)
                atts->SetHistogramType(HistogramAttributes::Weighted);
            else
                atts->SetHistogramType(HistogramAttributes::Variable);
        }
        Apply();
    }
}


void
QvisHistogramPlotWindow::weightTypeChanged(int val)
{
    HistogramAttributes::BinContribution bc;
    if (val == 0)
        bc = HistogramAttributes::Weighted;
    else
        bc = HistogramAttributes::Variable;
        
    if (bc != atts->GetHistogramType())
    {
        atts->SetHistogramType(bc);
        //SetUpdate(false);
        Apply();
    }
}


void
QvisHistogramPlotWindow::weightVariableChanged(const QString &varName)
{
    atts->SetWeightVariable(varName.toStdString());
    SetUpdate(false);
    Apply();
}


void
QvisHistogramPlotWindow::dataScaleChanged(int val)
{
    if ( val != atts->GetDataScale() )
    {
    atts->SetDataScale(HistogramAttributes::DataScale(val));
    Apply();
    }
}

void
QvisHistogramPlotWindow::binsScaleChanged(int val)
{
    if ( val != atts->GetBinScale() )
    {
    atts->SetBinScale(HistogramAttributes::DataScale(val));
    Apply();
    }
}

void
QvisHistogramPlotWindow::specifyRangeChanged(bool val)
{
    atts->SetSpecifyRange(val);
    Apply();
}

void
QvisHistogramPlotWindow::minProcessText()
{
    GetCurrentValues(HistogramAttributes::ID_min);
    Apply();
}


void
QvisHistogramPlotWindow::maxProcessText()
{
    GetCurrentValues(HistogramAttributes::ID_max);
    Apply();
}


void
QvisHistogramPlotWindow::numBinsProcessText()
{
    GetCurrentValues(HistogramAttributes::ID_numBins);
    Apply();
}


void
QvisHistogramPlotWindow::domainProcessText()
{
    GetCurrentValues(HistogramAttributes::ID_domain);
    Apply();
}


void
QvisHistogramPlotWindow::zoneProcessText()
{
    GetCurrentValues(HistogramAttributes::ID_zone);
    Apply();
}


void
QvisHistogramPlotWindow::useBinWidthsChanged(bool val)
{
    atts->SetUseBinWidths(val);
    SetUpdate(false);
    Apply();
}


void
QvisHistogramPlotWindow::outputTypeChanged(int val)
{
    if(val != atts->GetOutputType())
    {
        atts->SetOutputType(HistogramAttributes::OutputType(val));
        SetUpdate(false);
        Apply();
    }
}


void
QvisHistogramPlotWindow::lineStyleChanged(int style)
{
    atts->SetLineStyle(style);
    SetUpdate(false);
    Apply();
}


void
QvisHistogramPlotWindow::lineWidthChanged(int style)
{
    atts->SetLineWidth(style);
    SetUpdate(false);
    Apply();
}


void
QvisHistogramPlotWindow::colorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    atts->SetColor(temp);
    SetUpdate(false);
    Apply();
}


