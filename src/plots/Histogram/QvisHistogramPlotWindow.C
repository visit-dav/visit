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

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
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
                         const char *caption,
                         const char *shortName,
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
// ****************************************************************************

void
QvisHistogramPlotWindow::CreateWindowContents()
{
    // Figure out the max width that we want to allow for some simple
    // line edit widgets.
    int maxWidth = fontMetrics().width("1.0000000000");
    
    QGridLayout *mainLayout = new QGridLayout(topLayout, 4,2,  10, "mainLayout");


    basedOnLabel = new QLabel(tr("Histogram based on"), central, "basedOnLabel");
    mainLayout->addWidget(basedOnLabel,0,0);
    basedOn = new QButtonGroup(central, "basedOn");
    basedOn->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *basedOnLayout = new QHBoxLayout(basedOn);
    basedOnLayout->setSpacing(10);
    QRadioButton *basedOnBasedOnManyVarsForSingleZone = new QRadioButton(tr("Array of variables (one zone)"), basedOn);
    basedOnLayout->addWidget(basedOnBasedOnManyVarsForSingleZone);
    QRadioButton *basedOnBasedOnManyZonesForSingleVar = new QRadioButton(tr("Many zones"), basedOn);
    basedOnLayout->addWidget(basedOnBasedOnManyZonesForSingleVar);
    connect(basedOn, SIGNAL(clicked(int)),
            this, SLOT(basedOnChanged(int)));
    mainLayout->addWidget(basedOn, 0,1);
    basedOnLabel->setEnabled(false);
    basedOn->setEnabled(false);

    // Histogram Style Group Box
    histGroupBox =new QGroupBox(central, "histGroupBox"); 
    histGroupBox->setTitle(tr("Histogram Options"));
    mainLayout->addMultiCellWidget(histGroupBox, 1, 1, 0, 1);
    QVBoxLayout *hgTopLayout = new QVBoxLayout(histGroupBox);
    hgTopLayout->setMargin(10);
    hgTopLayout->addSpacing(15);
    QGridLayout *hgLayout = new QGridLayout(hgTopLayout, 9, 2);
    hgLayout->setSpacing(10);
    hgLayout->setColStretch(1,10);

    histogramTypeLabel = new QLabel(tr("Bin contribution"), histGroupBox, "histogramTypeLabel");
    hgLayout->addWidget(histogramTypeLabel,0,0);

    histogramType = new QButtonGroup(histGroupBox, "histogramType");
    histogramType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *histogramTypeLayout = new QHBoxLayout(histogramType);
    histogramTypeLayout->setSpacing(10);
    QRadioButton *histogramTypeBinContributionFrequency = new QRadioButton(tr("Frequency"), histogramType);
    histogramTypeLayout->addWidget(histogramTypeBinContributionFrequency);
    QRadioButton *histogramTypeBinContributionWeighted = new QRadioButton(tr("Weighted"), histogramType);
    histogramTypeLayout->addWidget(histogramTypeBinContributionWeighted);
    connect(histogramType, SIGNAL(clicked(int)),
            this, SLOT(histogramTypeChanged(int)));
    hgLayout->addWidget(histogramType, 0,1);

    weightTypeLabel = new QLabel(tr("Weighted by"), histGroupBox, "weightTypeLabel");
    hgLayout->addWidget(weightTypeLabel,1,0);

    weightType = new QButtonGroup(histGroupBox, "weightType");
    weightType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *weightTypeLayout = new QHBoxLayout(weightType);
    weightTypeLayout->setSpacing(10);
    QRadioButton *weightTypeVolumeArea = new QRadioButton(tr("Area (2D) / Volume (3D)"), 
                                                          weightType);
    weightTypeLayout->addWidget(weightTypeVolumeArea);
    QRadioButton *weightTypeVariable = new QRadioButton(tr("Variable"), weightType);
    weightTypeLayout->addWidget(weightTypeVariable);
    connect(weightType, SIGNAL(clicked(int)),
            this, SLOT(weightTypeChanged(int)));
    hgLayout->addWidget(weightType, 1,1);

    weightVariableLabel = new QLabel(tr("Variable to Weight By"), histGroupBox, "weightVariableLabel");
    hgLayout->addWidget(weightVariableLabel,2,0);
    int weightVariableMask = QvisVariableButton::Scalars;
    weightVariable = new QvisVariableButton(true, true, true, weightVariableMask, histGroupBox, "weightVariable");
    weightVariable->setDefaultVariable("default");
    connect(weightVariable, SIGNAL(activated(const QString&)),
            this, SLOT(weightVariableChanged(const QString&)));
    hgLayout->addWidget(weightVariable, 2,1);

    // Add data scale
    QLabel *dataScaleLabel = new QLabel(tr("Data Scale"), histGroupBox, "dataScaleLabel");
    hgLayout->addWidget(dataScaleLabel,3,0);

    dataScale = new QButtonGroup(histGroupBox, "dataScale");
    connect(dataScale, SIGNAL(clicked(int)),this, SLOT(dataScaleChanged(int)));    
    QHBoxLayout *dataScaleLayout = new QHBoxLayout(dataScale);
    QRadioButton *linearScale = new QRadioButton(tr("Linear"), dataScale);
    dataScaleLayout->addWidget(linearScale);
    QRadioButton *logScale = new QRadioButton(tr("Log10"), dataScale);
    dataScaleLayout->addWidget(logScale);
    QRadioButton *sqrtScale = new QRadioButton(tr("Square root"), dataScale);
    dataScaleLayout->addWidget(sqrtScale);
    hgLayout->addWidget(dataScale, 3,1);

    specifyRange = new QCheckBox(tr("Specify Range?"), histGroupBox, "specifyRange");
    connect(specifyRange, SIGNAL(toggled(bool)),
            this, SLOT(specifyRangeChanged(bool)));
    hgLayout->addWidget(specifyRange, 4,0);


    minLabel = new QLabel(tr("Minimum"), histGroupBox, "minLabel");
    hgLayout->addWidget(minLabel,5,0);
    min = new QLineEdit(histGroupBox, "min");
    connect(min, SIGNAL(returnPressed()),
            this, SLOT(minProcessText()));
    hgLayout->addWidget(min,5,1);

    maxLabel = new QLabel(tr("Maximum"), histGroupBox, "maxLabel");
    hgLayout->addWidget(maxLabel,6,0);
    max = new QLineEdit(histGroupBox, "max");
    connect(max, SIGNAL(returnPressed()),
            this, SLOT(maxProcessText()));
    hgLayout->addWidget(max, 6,1);

    numBinsLabel = new QLabel(tr("Number of Bins"), histGroupBox, "numBinsLabel");
    hgLayout->addWidget(numBinsLabel,7,0);

    numBins = new QLineEdit(histGroupBox, "numBins");
    connect(numBins, SIGNAL(returnPressed()),
            this, SLOT(numBinsProcessText()));
    hgLayout->addWidget(numBins, 7,1);

    QLabel *binScaleLabel = new QLabel(tr("Bin Scale"), histGroupBox, "binScaleLabel");
    hgLayout->addWidget(binScaleLabel,8,0);
    binsScale = new QButtonGroup(histGroupBox, "binsScale");
    connect(binsScale, SIGNAL(clicked(int)),this, SLOT(binsScaleChanged(int)));    
    QHBoxLayout *binsScaleLayout = new QHBoxLayout(binsScale);
    QRadioButton *binLinearScale = new QRadioButton(tr("Linear"), binsScale);
    binsScaleLayout->addWidget(binLinearScale);
    QRadioButton *binLogScale = new QRadioButton(tr("Log10"), binsScale);
    binsScaleLayout->addWidget(binLogScale);
    QRadioButton *binSqrtScale = new QRadioButton(tr("Square root"), binsScale);
    binsScaleLayout->addWidget(binSqrtScale);
    hgLayout->addWidget(binsScale, 8,1);    

    // Bar Plot Group Box
    barGroupBox =new QGroupBox(central, "barGroupBox"); 
    barGroupBox->setTitle(tr("Single Zone Plot Options"));
    mainLayout->addMultiCellWidget(barGroupBox, 2, 2, 0, 1);
    QVBoxLayout *bgTopLayout = new QVBoxLayout(barGroupBox);
    bgTopLayout->setMargin(10);
    bgTopLayout->addSpacing(15);
    QGridLayout *bgLayout = new QGridLayout(bgTopLayout, 3, 2);
    bgLayout->setSpacing(10);
    bgLayout->setColStretch(1,10);

    domainLabel = new QLabel(tr("domain"), barGroupBox, "domainLabel");
    bgLayout->addWidget(domainLabel,0,0);

    domain = new QLineEdit(barGroupBox, "domain");
    connect(domain, SIGNAL(returnPressed()),
            this, SLOT(domainProcessText()));
    bgLayout->addWidget(domain, 0,1);

    zoneLabel = new QLabel(tr("zone"), barGroupBox, "zoneLabel");
    bgLayout->addWidget(zoneLabel,1,0);

    zone = new QLineEdit(barGroupBox, "zone");
    connect(zone, SIGNAL(returnPressed()),
            this, SLOT(zoneProcessText()));
    bgLayout->addWidget(zone, 1,1);

    useBinWidths = new QCheckBox(tr("Use bin widths?"), barGroupBox, "useBinWidths");
    connect(useBinWidths, SIGNAL(toggled(bool)),
            this, SLOT(useBinWidthsChanged(bool)));
    bgLayout->addWidget(useBinWidths, 2,0);


    // Plot Syle Group Box

    // Bar Plot Group Box
    styleGroupBox =new QGroupBox(central, "styleGroupBox"); 
    styleGroupBox->setTitle(tr("Plot Style"));
    mainLayout->addMultiCellWidget(styleGroupBox, 3, 3, 0, 1);
    QVBoxLayout *sgTopLayout = new QVBoxLayout(styleGroupBox);
    sgTopLayout->setMargin(10);
    sgTopLayout->addSpacing(15);
    QGridLayout *sgLayout = new QGridLayout(sgTopLayout, 3, 2);
    sgLayout->setSpacing(10);
    sgLayout->setColStretch(1,10);
    
    // Add output type

    outputType = new QButtonGroup(styleGroupBox, "outputType");
    outputType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *outputTypeLayout = new QHBoxLayout(outputType);
    outputTypeLayout->setSpacing(10);
    QRadioButton *outputTypeOutputTypeCurve = new QRadioButton(tr("Curve"), outputType);
    outputTypeLayout->addWidget(outputTypeOutputTypeCurve);
    QRadioButton *outputTypeOutputTypeBlock = new QRadioButton(tr("Block"), outputType);
    outputTypeLayout->addWidget(outputTypeOutputTypeBlock,1);
    connect(outputType, SIGNAL(clicked(int)),
            this, SLOT(outputTypeChanged(int)));
    sgLayout->addWidget(outputType, 0,1);

    outputTypeLabel = new QLabel(tr("Type of Output"), styleGroupBox, "outputTypeLabel");
    sgLayout->addWidget(outputTypeLabel,0,0,AlignRight | AlignVCenter);


    // Add Line Style
    lineStyleLabel = new QLabel(tr("Line Style"), styleGroupBox, "lineStyleLabel");
    sgLayout->addWidget(lineStyleLabel,1,0,AlignRight | AlignVCenter);

    lineStyle = new QvisLineStyleWidget(0, styleGroupBox, "lineStyle");
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    sgLayout->addWidget(lineStyle, 1,1);

    // Add Line Width
    lineWidthLabel = new QLabel(tr("Line Width"), styleGroupBox, "lineWidthLabel");
    sgLayout->addWidget(lineWidthLabel,2,0,AlignRight | AlignVCenter);

    lineWidth = new QvisLineWidthWidget(0, styleGroupBox, "lineWidth");
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    sgLayout->addWidget(lineWidth, 2,1);

    // Add Color Selector
    colorLabel = new QLabel(tr("Color"), styleGroupBox, "colorLabel");
    sgLayout->addWidget(colorLabel,3,0,AlignRight | AlignVCenter);

    color = new QvisColorButton(styleGroupBox, "color");
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
// ****************************************************************************

void
QvisHistogramPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;
    double r;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }

        const double         *dptr;
        const float          *fptr;
        const int            *iptr;
        const char           *cptr;
        const unsigned char  *uptr;
        const string         *sptr;
        QColor                tempcolor;
        switch(i)
        {
          case 0: //basedOn
            if (atts->GetBasedOn() == HistogramAttributes::ManyZonesForSingleVar)
            {
                histGroupBox->setEnabled(true);
                barGroupBox->setEnabled(false);
                histogramType->setEnabled(true);
                histogramTypeLabel->setEnabled(true);

                if(atts->GetHistogramType() == HistogramAttributes::Weighted)
                {
                    weightType->setEnabled(true);
                    weightTypeLabel->setEnabled(true);
                }
                else
                {
                    weightType->setEnabled(false);
                    weightTypeLabel->setEnabled(false);
                }
            }
            else
            {
                histGroupBox->setEnabled(false);
                barGroupBox->setEnabled(true);
                histogramType->setEnabled(false);
                histogramTypeLabel->setEnabled(false);
                weightType->setEnabled(false);
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
            basedOn->blockSignals(true);
            basedOn->setButton(atts->GetBasedOn());
            basedOn->blockSignals(false);
            break;
          case 1: //histogramType
            if (atts->GetHistogramType() == HistogramAttributes::Weighted ||
                atts->GetHistogramType() == HistogramAttributes::Variable)
            {
                weightType->setEnabled(true);
                weightTypeLabel->setEnabled(true);
            }
            else
            {
                weightType->setEnabled(false);
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
            histogramType->blockSignals(true);
            if (atts->GetHistogramType() == HistogramAttributes::Frequency)
                histogramType->setButton(0);
            else if (atts->GetHistogramType() == HistogramAttributes::Weighted ||
                     atts->GetHistogramType() == HistogramAttributes::Variable)
                histogramType->setButton(1);
            histogramType->blockSignals(false);
            if (atts->GetHistogramType() == HistogramAttributes::Weighted ||
                atts->GetHistogramType() == HistogramAttributes::Variable)
            {
                weightType->blockSignals(true);
                if (atts->GetHistogramType() == HistogramAttributes::Weighted)
                    weightType->setButton(0);
                else
                    weightType->setButton(1);
                weightType->blockSignals(false);
            }
            break;
          case 2: //weightVariable
            weightVariable->blockSignals(true);
            weightVariable->setText(atts->GetWeightVariable().c_str());
            weightVariable->blockSignals(false);
            break;
          case 3: //specifyRange
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
          case 4: //min
            min->blockSignals(true);
            temp.setNum(atts->GetMin());
            min->setText(temp);
            min->blockSignals(false);
            break;
          case 5: //max
            max->blockSignals(true);
            temp.setNum(atts->GetMax());
            max->setText(temp);
            max->blockSignals(false);
            break;
          case 6: //numBins
            numBins->blockSignals(true);
            temp.sprintf("%d", atts->GetNumBins());
            numBins->setText(temp);
            numBins->blockSignals(false);
            break;
          case 7: //domain
            domain->blockSignals(true);
            temp.sprintf("%d", atts->GetDomain());
            domain->setText(temp);
            domain->blockSignals(false);
            break;
          case 8: //zone
            zone->blockSignals(true);
            temp.sprintf("%d", atts->GetZone());
            zone->setText(temp);
            zone->blockSignals(false);
            break;
          case 9: //useBinWidths
            useBinWidths->blockSignals(true);
            useBinWidths->setChecked(atts->GetUseBinWidths());
            useBinWidths->blockSignals(false);
            break;
          case 10: //outputType
            outputType->blockSignals(true);
            outputType->setButton(atts->GetOutputType());
            outputType->blockSignals(false);
            break;
          case 11: //lineStyle
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(atts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;
          case 12: //lineWidth
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(atts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
          case 13: //color
            tempcolor = QColor(atts->GetColor().Red(),
                               atts->GetColor().Green(),
                               atts->GetColor().Blue());
            color->blockSignals(true);
            color->setButtonColor(tempcolor);
            color->blockSignals(false);
            break;
	case 14: //data scale
	    dataScale->blockSignals(true);
            dataScale->setButton(atts->GetDataScale() );
	    dataScale->blockSignals(false);
	    break;
	case 15: //bin scale
	    binsScale->blockSignals(true);
            binsScale->setButton(atts->GetBinScale() );
	    binsScale->blockSignals(false);
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
// ****************************************************************************

void
QvisHistogramPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do basedOn
    if(which_widget == 0 || doAll)
    {
        // Nothing for basedOn
    }

    // Do histogramType
    if(which_widget == 1 || doAll)
    {
        // Nothing for histogramType
    }

    // Do weightVariable
    if(which_widget == 2 || doAll)
    {
        // Nothing for weightVariable
    }

    // Do specifyRange
    if(which_widget == 3 || doAll)
    {
        // Nothing for specifyRange
    }

    // Do min
    if(which_widget == 4 || doAll)
    {
        temp = min->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                atts->SetMin(val);
        }

        if(!okay)
        {
            msg = tr("The value of min was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetMin());
            Message(msg);
            atts->SetMin(atts->GetMin());
        }
    }

    // Do max
    if(which_widget == 5 || doAll)
    {
        temp = max->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                atts->SetMax(val);
        }

        if(!okay)
        {
            msg = tr("The value of max was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetMax());
            Message(msg);
            atts->SetMax(atts->GetMax());
        }
    }

    // Do numBins
    if(which_widget == 6 || doAll)
    {
        temp = numBins->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            if(okay)
                atts->SetNumBins(val);
        }

        if(!okay)
        {
            msg = tr("The value of numBins was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetNumBins());
            Message(msg);
            atts->SetNumBins(atts->GetNumBins());
        }
    }

    // Do domain
    if(which_widget == 7 || doAll)
    {
        temp = domain->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            if(okay)
                atts->SetDomain(val);
        }

        if(!okay)
        {
            msg = tr("The value of domain was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetDomain());
            Message(msg);
            atts->SetDomain(atts->GetDomain());
        }
    }

    // Do zone
    if(which_widget == 8 || doAll)
    {
        temp = zone->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            if(okay)
                atts->SetZone(val);
        }

        if(!okay)
        {
            msg = tr("The value of zone was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetZone());
            Message(msg);
            atts->SetZone(atts->GetZone());
        }
    }

    // Do useBinWidths
    if(which_widget == 9 || doAll)
    {
        // Nothing for useBinWidths
    }

    // Do outputType
    if(which_widget == 10 || doAll)
    {
        // Nothing for outputType
    }

    // Do lineStyle
    if(which_widget == 11 || doAll)
    {
        // Nothing for lineStyle
    }

    // Do lineWidth
    if(which_widget == 12 || doAll)
    {
        // Nothing for lineWidth
    }

    // Do color
    if(which_widget == 13 || doAll)
    {
        // Nothing for color
    }

    // Do dataScale
    if (which_widget == 14 || doAll)
    {
    }
    
    // Do dataScale
    if (which_widget == 15 || doAll)
    {
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
            if (weightType->selectedId() == 0)
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
    atts->SetWeightVariable(varName.latin1());
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
    GetCurrentValues(4);
    Apply();
}


void
QvisHistogramPlotWindow::maxProcessText()
{
    GetCurrentValues(5);
    Apply();
}


void
QvisHistogramPlotWindow::numBinsProcessText()
{
    GetCurrentValues(6);
    Apply();
}


void
QvisHistogramPlotWindow::domainProcessText()
{
    GetCurrentValues(7);
    Apply();
}


void
QvisHistogramPlotWindow::zoneProcessText()
{
    GetCurrentValues(8);
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


