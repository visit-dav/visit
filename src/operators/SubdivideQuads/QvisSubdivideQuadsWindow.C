#include "QvisSubdivideQuadsWindow.h"

#include <SubdivideQuadsAttributes.h>
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
#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisSubdivideQuadsWindow::QvisSubdivideQuadsWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Nov 2 06:28:41 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisSubdivideQuadsWindow::QvisSubdivideQuadsWindow(const int type,
                         SubdivideQuadsAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisSubdivideQuadsWindow::~QvisSubdivideQuadsWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Nov 2 06:28:41 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisSubdivideQuadsWindow::~QvisSubdivideQuadsWindow()
{
}


// ****************************************************************************
// Method: QvisSubdivideQuadsWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Nov 2 06:28:41 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubdivideQuadsWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 5,2,  10, "mainLayout");


    thresholdLabel = new QLabel("Biggest change allowed in a zone?", central, "thresholdLabel");
    mainLayout->addWidget(thresholdLabel,0,0);
    threshold = new QLineEdit(central, "threshold");
    connect(threshold, SIGNAL(returnPressed()),
            this, SLOT(thresholdProcessText()));
    mainLayout->addWidget(threshold, 0,1);

    maxSubdivsLabel = new QLabel("Maximum number of subdivisions", central, "maxSubdivsLabel");
    mainLayout->addWidget(maxSubdivsLabel,1,0);
    maxSubdivs = new QLineEdit(central, "maxSubdivs");
    connect(maxSubdivs, SIGNAL(returnPressed()),
            this, SLOT(maxSubdivsProcessText()));
    mainLayout->addWidget(maxSubdivs, 1,1);

    fanOutPoints = new QCheckBox("Fan out points to cover up T-intersections?", central, "fanOutPoints");
    connect(fanOutPoints, SIGNAL(toggled(bool)),
            this, SLOT(fanOutPointsChanged(bool)));
    mainLayout->addWidget(fanOutPoints, 2,0);

    doTriangles = new QCheckBox("Subdivide triangles", central, "doTriangles");
    connect(doTriangles, SIGNAL(toggled(bool)),
            this, SLOT(doTrianglesChanged(bool)));
    mainLayout->addWidget(doTriangles, 3,0);

    variableLabel = new QLabel("Variable to base subdivision on", central, "variableLabel");
    mainLayout->addWidget(variableLabel,4,0);
    variable = new QLineEdit(central, "variable");
    connect(variable, SIGNAL(returnPressed()),
            this, SLOT(variableProcessText()));
    mainLayout->addWidget(variable, 4,1);

}


// ****************************************************************************
// Method: QvisSubdivideQuadsWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue Nov 2 06:28:41 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubdivideQuadsWindow::UpdateWindow(bool doAll)
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
          case 0: //threshold
            temp.sprintf("%g", atts->GetThreshold());
            threshold->setText(temp);
            break;
          case 1: //maxSubdivs
            temp.sprintf("%d", atts->GetMaxSubdivs());
            maxSubdivs->setText(temp);
            break;
          case 2: //fanOutPoints
            fanOutPoints->setChecked(atts->GetFanOutPoints());
            break;
          case 3: //doTriangles
            doTriangles->setChecked(atts->GetDoTriangles());
            break;
          case 4: //variable
            temp = atts->GetVariable().c_str();
            variable->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisSubdivideQuadsWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Nov 2 06:28:41 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubdivideQuadsWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do threshold
    if(which_widget == 0 || doAll)
    {
        temp = threshold->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetThreshold(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of threshold was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetThreshold());
            Message(msg);
            atts->SetThreshold(atts->GetThreshold());
        }
    }

    // Do maxSubdivs
    if(which_widget == 1 || doAll)
    {
        temp = maxSubdivs->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetMaxSubdivs(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of maxSubdivs was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetMaxSubdivs());
            Message(msg);
            atts->SetMaxSubdivs(atts->GetMaxSubdivs());
        }
    }

    // Do fanOutPoints
    if(which_widget == 2 || doAll)
    {
        // Nothing for fanOutPoints
    }

    // Do doTriangles
    if(which_widget == 3 || doAll)
    {
        // Nothing for doTriangles
    }

    // Do variable
    if(which_widget == 4 || doAll)
    {
        temp = variable->displayText();
        okay = !temp.isEmpty();
        if(okay)
        {
            atts->SetVariable(temp.latin1());
        }

        if(!okay)
        {
            msg.sprintf("The value of variable was invalid. "
                "Resetting to the last good value of %s.",
                atts->GetVariable().c_str());
            Message(msg);
            atts->SetVariable(atts->GetVariable());
        }
    }

}


//
// Qt Slot functions
//


void
QvisSubdivideQuadsWindow::thresholdProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisSubdivideQuadsWindow::maxSubdivsProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisSubdivideQuadsWindow::fanOutPointsChanged(bool val)
{
    atts->SetFanOutPoints(val);
    Apply();
}


void
QvisSubdivideQuadsWindow::doTrianglesChanged(bool val)
{
    atts->SetDoTriangles(val);
    Apply();
}


void
QvisSubdivideQuadsWindow::variableProcessText()
{
    GetCurrentValues(4);
    Apply();
}


