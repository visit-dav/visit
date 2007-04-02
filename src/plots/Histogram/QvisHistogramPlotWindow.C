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

#include "QvisHistogramPlotWindow.h"

#include <HistogramAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
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
// Creation:   Fri Jan 12 15:22:02 PST 2007
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
// Creation:   Fri Jan 12 15:22:02 PST 2007
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
// Programmer: xml2window
// Creation:   Fri Jan 12 15:22:02 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramPlotWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 13,2,  10, "mainLayout");


    basedOnLabel = new QLabel("Histogram based on", central, "basedOnLabel");
    mainLayout->addWidget(basedOnLabel,0,0);
    basedOn = new QButtonGroup(central, "basedOn");
    basedOn->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *basedOnLayout = new QHBoxLayout(basedOn);
    basedOnLayout->setSpacing(10);
    QRadioButton *basedOnBasedOnManyVarsForSingleZone = new QRadioButton("Array of variables (one zone)", basedOn);
    basedOnLayout->addWidget(basedOnBasedOnManyVarsForSingleZone);
    QRadioButton *basedOnBasedOnManyZonesForSingleVar = new QRadioButton("Many zones", basedOn);
    basedOnLayout->addWidget(basedOnBasedOnManyZonesForSingleVar);
    connect(basedOn, SIGNAL(clicked(int)),
            this, SLOT(basedOnChanged(int)));
    mainLayout->addWidget(basedOn, 0,1);
    basedOnLabel->setEnabled(false);
    basedOn->setEnabled(false);

    specifyRange = new QCheckBox("Specify Range?", central, "specifyRange");
    connect(specifyRange, SIGNAL(toggled(bool)),
            this, SLOT(specifyRangeChanged(bool)));
    mainLayout->addWidget(specifyRange, 1,0);

    minLabel = new QLabel("Minimum", central, "minLabel");
    mainLayout->addWidget(minLabel,2,0);
    min = new QLineEdit(central, "min");
    connect(min, SIGNAL(returnPressed()),
            this, SLOT(minProcessText()));
    mainLayout->addWidget(min, 2,1);

    maxLabel = new QLabel("Maximum", central, "maxLabel");
    mainLayout->addWidget(maxLabel,3,0);
    max = new QLineEdit(central, "max");
    connect(max, SIGNAL(returnPressed()),
            this, SLOT(maxProcessText()));
    mainLayout->addWidget(max, 3,1);

    numBinsLabel = new QLabel("Number of Bins", central, "numBinsLabel");
    mainLayout->addWidget(numBinsLabel,4,0);
    numBins = new QLineEdit(central, "numBins");
    connect(numBins, SIGNAL(returnPressed()),
            this, SLOT(numBinsProcessText()));
    mainLayout->addWidget(numBins, 4,1);

    twoDAmountLabel = new QLabel("Calculate 2D based on", central, "twoDAmountLabel");
    mainLayout->addWidget(twoDAmountLabel,5,0);
    twoDAmount = new QButtonGroup(central, "twoDAmount");
    twoDAmount->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *twoDAmountLayout = new QHBoxLayout(twoDAmount);
    twoDAmountLayout->setSpacing(10);
    QRadioButton *twoDAmountTwoDAmountArea = new QRadioButton("Area", twoDAmount);
    twoDAmountLayout->addWidget(twoDAmountTwoDAmountArea);
    QRadioButton *twoDAmountTwoDAmountRevolvedVolume = new QRadioButton("RevolvedVolume", twoDAmount);
    twoDAmountLayout->addWidget(twoDAmountTwoDAmountRevolvedVolume);
    connect(twoDAmount, SIGNAL(clicked(int)),
            this, SLOT(twoDAmountChanged(int)));
    mainLayout->addWidget(twoDAmount, 5,1);

    domainLabel = new QLabel("domain", central, "domainLabel");
    mainLayout->addWidget(domainLabel,6,0);
    domain = new QLineEdit(central, "domain");
    connect(domain, SIGNAL(returnPressed()),
            this, SLOT(domainProcessText()));
    mainLayout->addWidget(domain, 6,1);

    zoneLabel = new QLabel("zone", central, "zoneLabel");
    mainLayout->addWidget(zoneLabel,7,0);
    zone = new QLineEdit(central, "zone");
    connect(zone, SIGNAL(returnPressed()),
            this, SLOT(zoneProcessText()));
    mainLayout->addWidget(zone, 7,1);

    useBinWidths = new QCheckBox("Use bin widths?", central, "useBinWidths");
    connect(useBinWidths, SIGNAL(toggled(bool)),
            this, SLOT(useBinWidthsChanged(bool)));
    mainLayout->addWidget(useBinWidths, 8,0);

    outputTypeLabel = new QLabel("Type of Output", central, "outputTypeLabel");
    mainLayout->addWidget(outputTypeLabel,9,0);
    outputType = new QButtonGroup(central, "outputType");
    outputType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *outputTypeLayout = new QHBoxLayout(outputType);
    outputTypeLayout->setSpacing(10);
    QRadioButton *outputTypeOutputTypeCurve = new QRadioButton("Curve", outputType);
    outputTypeLayout->addWidget(outputTypeOutputTypeCurve);
    QRadioButton *outputTypeOutputTypeBlock = new QRadioButton("Block", outputType);
    outputTypeLayout->addWidget(outputTypeOutputTypeBlock);
    connect(outputType, SIGNAL(clicked(int)),
            this, SLOT(outputTypeChanged(int)));
    mainLayout->addWidget(outputType, 9,1);

    lineStyleLabel = new QLabel("Line Style", central, "lineStyleLabel");
    mainLayout->addWidget(lineStyleLabel,10,0);
    lineStyle = new QvisLineStyleWidget(0, central, "lineStyle");
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    mainLayout->addWidget(lineStyle, 10,1);

    lineWidthLabel = new QLabel("Line Width", central, "lineWidthLabel");
    mainLayout->addWidget(lineWidthLabel,11,0);
    lineWidth = new QvisLineWidthWidget(0, central, "lineWidth");
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    mainLayout->addWidget(lineWidth, 11,1);

    colorLabel = new QLabel("Color", central, "colorLabel");
    mainLayout->addWidget(colorLabel,12,0);
    color = new QvisColorButton(central, "color");
    connect(color, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(colorChanged(const QColor&)));
    mainLayout->addWidget(color, 12,1);

}


// ****************************************************************************
// Method: QvisHistogramPlotWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Jan 12 15:22:02 PST 2007
//
// Modifications:
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
            if (atts->GetBasedOn() == HistogramAttributes::ManyZonesForSingleVar)
            {
                twoDAmount->setEnabled(true);
                twoDAmountLabel->setEnabled(true);
            }
            else
            {
                twoDAmount->setEnabled(false);
                twoDAmountLabel->setEnabled(false);
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
            basedOn->setButton(atts->GetBasedOn());
            break;
          case 1: //specifyRange
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
            specifyRange->setChecked(atts->GetSpecifyRange());
            break;
          case 2: //min
            temp.setNum(atts->GetMin());
            min->setText(temp);
            break;
          case 3: //max
            temp.setNum(atts->GetMax());
            max->setText(temp);
            break;
          case 4: //numBins
            temp.sprintf("%d", atts->GetNumBins());
            numBins->setText(temp);
            break;
          case 5: //twoDAmount
            twoDAmount->setButton(atts->GetTwoDAmount());
            break;
          case 6: //domain
            temp.sprintf("%d", atts->GetDomain());
            domain->setText(temp);
            break;
          case 7: //zone
            temp.sprintf("%d", atts->GetZone());
            zone->setText(temp);
            break;
          case 8: //useBinWidths
            useBinWidths->setChecked(atts->GetUseBinWidths());
            break;
          case 9: //outputType
            outputType->setButton(atts->GetOutputType());
            break;
          case 10: //lineStyle
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(atts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;
          case 11: //lineWidth
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(atts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
          case 12: //color
            tempcolor = QColor(atts->GetColor().Red(),
                               atts->GetColor().Green(),
                               atts->GetColor().Blue());
            color->setButtonColor(tempcolor);
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
// Creation:   Fri Jan 12 15:22:02 PST 2007
//
// Modifications:
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

    // Do specifyRange
    if(which_widget == 1 || doAll)
    {
        // Nothing for specifyRange
    }

    // Do min
    if(which_widget == 2 || doAll)
    {
        temp = min->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMin(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of min was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMin());
            Message(msg);
            atts->SetMin(atts->GetMin());
        }
    }

    // Do max
    if(which_widget == 3 || doAll)
    {
        temp = max->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetMax(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of max was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetMax());
            Message(msg);
            atts->SetMax(atts->GetMax());
        }
    }

    // Do numBins
    if(which_widget == 4 || doAll)
    {
        temp = numBins->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetNumBins(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of numBins was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetNumBins());
            Message(msg);
            atts->SetNumBins(atts->GetNumBins());
        }
    }

    // Do twoDAmount
    if(which_widget == 5 || doAll)
    {
        // Nothing for twoDAmount
    }

    // Do domain
    if(which_widget == 6 || doAll)
    {
        temp = domain->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetDomain(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of domain was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetDomain());
            Message(msg);
            atts->SetDomain(atts->GetDomain());
        }
    }

    // Do zone
    if(which_widget == 7 || doAll)
    {
        temp = zone->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetZone(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of zone was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetZone());
            Message(msg);
            atts->SetZone(atts->GetZone());
        }
    }

    // Do useBinWidths
    if(which_widget == 8 || doAll)
    {
        // Nothing for useBinWidths
    }

    // Do outputType
    if(which_widget == 9 || doAll)
    {
        // Nothing for outputType
    }

    // Do lineStyle
    if(which_widget == 10 || doAll)
    {
        // Nothing for lineStyle
    }

    // Do lineWidth
    if(which_widget == 11 || doAll)
    {
        // Nothing for lineWidth
    }

    // Do color
    if(which_widget == 12 || doAll)
    {
        // Nothing for color
    }

}


// ****************************************************************************
// Method: QvisHistogramPlotWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: xml2window
// Creation:   Fri Jan 12 15:22:02 PST 2007
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
// Creation:   Fri Jan 12 15:22:02 PST 2007
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
// Creation:   Fri Jan 12 15:22:02 PST 2007
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
// Creation:   Fri Jan 12 15:22:02 PST 2007
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
QvisHistogramPlotWindow::specifyRangeChanged(bool val)
{
    atts->SetSpecifyRange(val);
    Apply();
}


void
QvisHistogramPlotWindow::minProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisHistogramPlotWindow::maxProcessText()
{
    GetCurrentValues(3);
    Apply();
}


void
QvisHistogramPlotWindow::numBinsProcessText()
{
    GetCurrentValues(4);
    Apply();
}


void
QvisHistogramPlotWindow::twoDAmountChanged(int val)
{
    if(val != atts->GetTwoDAmount())
    {
        atts->SetTwoDAmount(HistogramAttributes::TwoDAmount(val));
        Apply();
    }
}


void
QvisHistogramPlotWindow::domainProcessText()
{
    GetCurrentValues(6);
    Apply();
}


void
QvisHistogramPlotWindow::zoneProcessText()
{
    GetCurrentValues(7);
    Apply();
}


void
QvisHistogramPlotWindow::useBinWidthsChanged(bool val)
{
    atts->SetUseBinWidths(val);
    Apply();
}


void
QvisHistogramPlotWindow::outputTypeChanged(int val)
{
    if(val != atts->GetOutputType())
    {
        atts->SetOutputType(HistogramAttributes::OutputType(val));
        Apply();
    }
}


void
QvisHistogramPlotWindow::lineStyleChanged(int style)
{
    atts->SetLineStyle(style);
    Apply();
}


void
QvisHistogramPlotWindow::lineWidthChanged(int style)
{
    atts->SetLineWidth(style);
    Apply();
}


void
QvisHistogramPlotWindow::colorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    atts->SetColor(temp);
    Apply();
}


