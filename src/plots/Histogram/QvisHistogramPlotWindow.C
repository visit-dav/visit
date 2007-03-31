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
#include <QvisColorButton.h>
#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
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
// Creation:   Thu Jun 26 10:33:56 PDT 2003
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
// Creation:   Thu Jun 26 10:33:56 PDT 2003
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
// Creation:   Thu Jun 26 10:33:56 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramPlotWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 9,2,  10, "mainLayout");


    specifyRange = new QCheckBox("Specify Range?", central, "specifyRange");
    connect(specifyRange, SIGNAL(toggled(bool)),
            this, SLOT(specifyRangeChanged(bool)));
    mainLayout->addWidget(specifyRange, 0,0);

    mainLayout->addWidget(new QLabel("Minimum", central, "minLabel"),1,0);
    min = new QLineEdit(central, "min");
    connect(min, SIGNAL(returnPressed()),
            this, SLOT(minProcessText()));
    mainLayout->addWidget(min, 1,1);

    mainLayout->addWidget(new QLabel("Maximum", central, "maxLabel"),2,0);
    max = new QLineEdit(central, "max");
    connect(max, SIGNAL(returnPressed()),
            this, SLOT(maxProcessText()));
    mainLayout->addWidget(max, 2,1);

    mainLayout->addWidget(new QLabel("Type of Output", central, "outputTypeLabel"),3,0);
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
    mainLayout->addWidget(outputType, 3,1);

    mainLayout->addWidget(new QLabel("Number of Bins", central, "numBinsLabel"),4,0);
    numBins = new QLineEdit(central, "numBins");
    connect(numBins, SIGNAL(returnPressed()),
            this, SLOT(numBinsProcessText()));
    mainLayout->addWidget(numBins, 4,1);

    mainLayout->addWidget(new QLabel("Calculate 2D based on", central, "twoDAmountLabel"),5,0);
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

    mainLayout->addWidget(new QLabel("Line Style", central, "lineStyleLabel"),6,0);
    lineStyle = new QvisLineStyleWidget(0, central, "lineStyle");
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    mainLayout->addWidget(lineStyle, 6,1);

    mainLayout->addWidget(new QLabel("Line Width", central, "lineWidthLabel"),7,0);
    lineWidth = new QvisLineWidthWidget(0, central, "lineWidth");
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    mainLayout->addWidget(lineWidth, 7,1);

    mainLayout->addWidget(new QLabel("Color", central, "colorLabel"),8,0);
    color = new QvisColorButton(central, "colorButton");
    color->setButtonColor(QColor(255, 0, 0));
    connect(color, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(colorChanged(const QColor &)));
    mainLayout->addWidget(color, 8, 1);

}


// ****************************************************************************
// Method: QvisHistogramPlotWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Thu Jun 26 10:33:56 PDT 2003
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
          case 0: //specifyRange
            if (atts->GetSpecifyRange() == true)
                min->setEnabled(true);
            else
                min->setEnabled(false);
            if (atts->GetSpecifyRange() == true)
                max->setEnabled(true);
            else
                max->setEnabled(false);
            specifyRange->setChecked(atts->GetSpecifyRange());
            break;
          case 1: //min
            temp.sprintf("%g", atts->GetMin());
            min->setText(temp);
            break;
          case 2: //max
            temp.sprintf("%g", atts->GetMax());
            max->setText(temp);
            break;
          case 3: //outputType
            outputType->setButton(atts->GetOutputType());
            break;
          case 4: //numBins
            temp.sprintf("%d", atts->GetNumBins());
            numBins->setText(temp);
            break;
          case 5: //twoDAmount
            twoDAmount->setButton(atts->GetTwoDAmount());
            break;
          case 6: //lineStyle
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(atts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;
          case 7: //lineWidth
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(atts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
          case 8: //color
            { // new scope
            QColor temp(atts->GetColor().Red(),
                        atts->GetColor().Green(),
                        atts->GetColor().Blue());
            color->blockSignals(true);
            color->setButtonColor(temp);
            color->blockSignals(false);
            }
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
// Creation:   Thu Jun 26 10:33:56 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do specifyRange
    if(which_widget == 0 || doAll)
    {
        // Nothing for specifyRange
    }

    // Do min
    if(which_widget == 1 || doAll)
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
    if(which_widget == 2 || doAll)
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

    // Do outputType
    if(which_widget == 3 || doAll)
    {
        // Nothing for outputType
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

    // Do lineStyle
    if(which_widget == 6 || doAll)
    {
        // Nothing for lineStyle
    }

    // Do lineWidth
    if(which_widget == 7 || doAll)
    {
        // Nothing for lineWidth
    }

    // Do color
    if(which_widget == 8 || doAll)
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
// Creation:   Thu Jun 26 10:33:56 PDT 2003
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

        viewer->SetPlotOptions(plotType);
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
// Creation:   Thu Jun 26 10:33:56 PDT 2003
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
// Creation:   Thu Jun 26 10:33:56 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramPlotWindow::makeDefault()
{
    GetCurrentValues(-1);
    atts->Notify();
    viewer->SetDefaultPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisHistogramPlotWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: xml2window
// Creation:   Thu Jun 26 10:33:56 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramPlotWindow::reset()
{
    viewer->ResetPlotOptions(plotType);
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
    GetCurrentValues(1);
    Apply();
}


void
QvisHistogramPlotWindow::maxProcessText()
{
    GetCurrentValues(2);
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
QvisHistogramPlotWindow::colorChanged(const QColor &color_)
{
    ColorAttribute temp(color_.red(), color_.green(), color_.blue());
    atts->SetColor(temp);
    Apply();
}


