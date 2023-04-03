// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisLineoutQueryWidget.h>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

#include <MapNode.h>


// ****************************************************************************
// Method: QvisLineoutQueryWidget::QvisLineoutQueryWidget
//
// Purpose: 
//   This is the constructor for the QvisLineoutQueryWidget class.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The name of the instance.
//   f      : The window flags.
//
// Note:       This widget acts as a layout for its children since it 
//   positions the children manually and dispatches mouse events to them.
//
// Programmer: Kathleen Biagas 
// Creation:   June 9, 2011 
//
// Modifications:
//    Kathleen Biagas, Fri Nov 15 17:14:13 PST 2013
//    Add sampling options.
//
// ****************************************************************************

QvisLineoutQueryWidget::QvisLineoutQueryWidget(QWidget *parent,
    Qt::WindowFlags f) : QWidget(parent, f)
{
    // 
    // Main layout for this widget
    // 
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0,0,0,0);
    topLayout->setSpacing(10);
  
    // 
    // startPoint
    // 
    QGridLayout *cLayout = new QGridLayout(); 

    cLayout->addWidget(new QLabel(tr("Start point")), 0, 0);
    startPoint = new QLineEdit();
    startPoint->setText("0 0 0");
    cLayout->addWidget(startPoint, 0, 1);

    // 
    // endPoint
    // 
    cLayout->addWidget(new QLabel(tr("End point")), 1, 0);
    endPoint = new QLineEdit();
    endPoint->setText("1 0 0");
    cLayout->addWidget(endPoint, 1, 1);

    // SamplingOn
    samplingOn = new QCheckBox(tr("Use sampling"));
    samplingOn->setChecked(0);
    cLayout->addWidget(samplingOn, 2, 1);

    numSamplesLabel = new QLabel(tr("Sample points"));
    numSamplesLabel->setAlignment(Qt::AlignCenter);
    cLayout->addWidget(numSamplesLabel, 3, 0);

    numSamples = new QLineEdit();
    numSamples->setText("50");
    cLayout->addWidget(numSamples, 3, 1);

    topLayout->addLayout(cLayout);
}

// ****************************************************************************
// Method: QvisLineoutQueryWidget::~QvisLineoutQueryWidget
//
// Purpose: 
//   This is the destructor for the QvisLineoutQueryWidget class.
//
// Programmer: Kathleen Biagas
// Creation:   Tue Dec 5 16:53:38 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisLineoutQueryWidget::~QvisLineoutQueryWidget()
{
    // nothing here
}


// ****************************************************************************
// Method: QvisLineoutQueryWidget::GetPoint
//
// Purpose:
//   Gets a point from one of the point text fields.
//
// Arguments:
//   pt    : The array in which the point will be stored.
//
// Returns:    True if it worked.
//
// Programmer: Kathleen Biagas 
// Creation:   June 9, 2011
//
// Modifications:
//
// ****************************************************************************

bool 
QvisLineoutQueryWidget::GetPoint(int whichWidget, double *pt)
{
    QString temp;
    if (whichWidget == 0) // StartPoint
        temp = startPoint->displayText().simplified();
    else // EndPoint 
        temp = endPoint->displayText().simplified();
    bool okay = !temp.isEmpty();
    if(okay)
    {
        pt[2] = 0.;
        int numScanned = sscanf(temp.toStdString().c_str(), "%lg %lg %lg",
                        &pt[0], &pt[1], &pt[2]);
        okay = (numScanned == 2 || numScanned == 3);
    }
    return okay;
}


// ****************************************************************************
// Method: QvisLineoutQueryWidget::GetQueryParameters
//
// Purpose:
//
// Arguments:
//   params    : The map node in which to store the info.
//
// Returns:    True if it worked.
//
// Programmer: Kathleen Biagas 
// Creation:   June 9, 2011
//
// Modifications:
//    Kathleen Biagas, Fri Nov 15 17:14:13 PST 2013
//    Add sampling options.
//
// ****************************************************************************

bool 
QvisLineoutQueryWidget::GetQueryParameters(MapNode &params)
{
    doubleVector p1(3);
    doubleVector p2(3);
    int ns;

    bool noerrors = true;

    noerrors &= GetPoint(0, &p1[0]); 
    noerrors &= GetPoint(1, &p2[0]); 

    // numSamples
    QString temp;

    temp = numSamples->displayText().simplified();
    bool okay = !temp.isEmpty();

    if(okay)
    {
        int numScanned = sscanf(temp.toStdString().c_str(), "%d", &ns);
        okay = (numScanned == 1 );
    }
    noerrors &= okay;

    if (noerrors)
    {
        params["start_point"] = p1;
        params["end_point"] = p2;
        params["use_sampling"] = (int)samplingOn->isChecked();
        params["num_samples"]  = ns;
    }

    return noerrors;
}
