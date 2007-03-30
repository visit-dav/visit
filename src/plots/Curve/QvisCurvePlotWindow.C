#include "QvisCurvePlotWindow.h"

#include <CurveAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <stdio.h>
#include <string>

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
//   
// ****************************************************************************

QvisCurvePlotWindow::QvisCurvePlotWindow(const int type,
                         CurveAttributes *subj,
                         const char *caption,
                         const char *shortName,
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
//   
// ****************************************************************************

void
QvisCurvePlotWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 5,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("Line Style", central, "lineStyleLabel"),0,0);
    lineStyle = new QvisLineStyleWidget(0, central, "lineStyle");
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    mainLayout->addWidget(lineStyle, 0,1);

    mainLayout->addWidget(new QLabel("Line Width", central, "lineWidthLabel"),1,0);
    lineWidth = new QvisLineWidthWidget(0, central, "lineWidth");
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    mainLayout->addWidget(lineWidth, 1,1);

    // Create the single color button.
    mainLayout->addWidget(new QLabel("Color", central, "colorLabel"),2,0);
    color = new QvisColorButton(central, "colorButton");
    color->setButtonColor(QColor(255, 0, 0));
    connect(color, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(colorChanged(const QColor &)));
    mainLayout->addWidget(color, 2, 1);


    showLabels = new QCheckBox("Labels", central, "showLabels");
    connect(showLabels, SIGNAL(toggled(bool)),
            this, SLOT(showLabelsChanged(bool)));
    mainLayout->addWidget(showLabels, 3,0);

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
//   
// ****************************************************************************

void
QvisCurvePlotWindow::UpdateWindow(bool doAll)
{
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
          case 0: //lineStyle
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(atts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;
          case 1: //lineWidth
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(atts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
          case 2: //color
            { // new scope
            QColor temp(atts->GetColor().Red(),
                        atts->GetColor().Green(),
                        atts->GetColor().Blue());
            color->blockSignals(true);
            color->setButtonColor(temp);
            color->blockSignals(false);
            }
            break;
          case 3: //showLabels
            showLabels->setChecked(atts->GetShowLabels());
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
//   
// ****************************************************************************

void
QvisCurvePlotWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
    QString msg, temp;

    // Do lineStyle
    if(which_widget == 0 || doAll)
    {
        // Nothing for lineStyle
    }

    // Do lineWidth
    if(which_widget == 1 || doAll)
    {
        // Nothing for lineWidth
    }

    // Do color
    if(which_widget == 2 || doAll)
    {
        // Nothing for color
    }

    // Do showLabels
    if(which_widget == 3 || doAll)
    {
        // Nothing for showLabels
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

        viewer->SetPlotOptions(plotType);
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
    viewer->SetDefaultPlotOptions(plotType);
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
    viewer->ResetPlotOptions(plotType);
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
QvisCurvePlotWindow::showLabelsChanged(bool val)
{
    atts->SetShowLabels(val);
    Apply();
}


