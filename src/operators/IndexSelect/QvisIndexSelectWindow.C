#include "QvisIndexSelectWindow.h"

#include <IndexSelectAttributes.h>
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

bool   ParseDimensions(const char *, int *);

// ****************************************************************************
// Method: QvisIndexSelectWindow::QvisIndexSelectWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Thu Jun 6 17:02:08 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisIndexSelectWindow::QvisIndexSelectWindow(const int type,
                         IndexSelectAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisIndexSelectWindow::~QvisIndexSelectWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Thu Jun 6 17:02:08 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisIndexSelectWindow::~QvisIndexSelectWindow()
{
}


// ****************************************************************************
// Method: QvisIndexSelectWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Thu Jun 6 17:02:08 PST 2002
//
// Modifications:
//   
//   Hank Childs, Sat Jun 29 16:17:41 PDT 2002
//   Re-enable coding for the Groups widget.  Modify layout so everything
//   spaces correctly.
//
//   Hank Childs, Fri Jul 12 15:47:43 PDT 2002
//   Fix typo (AI->All).
//
// ****************************************************************************

void
QvisIndexSelectWindow::CreateWindowContents()
{
    QGridLayout *wholeLayout = new QGridLayout(topLayout, 2, 1, 10,
                                              "wholeLayout");
    QGridLayout *mainLayout = new QGridLayout(4,2,  10, "mainLayout");
    wholeLayout->addLayout(mainLayout, 0, 0);


    mainLayout->addWidget(new QLabel("Dimension", central, "dimLabel"),0,0);
    dim = new QButtonGroup(central, "dim");
    dim->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *dimLayout = new QHBoxLayout(dim);
    dimLayout->setSpacing(2);
    QRadioButton *dimDimensionOneD = new QRadioButton("1D", dim);
    dimLayout->addWidget(dimDimensionOneD);
    QRadioButton *dimDimensionTwoD = new QRadioButton("2D", dim);
    dimLayout->addWidget(dimDimensionTwoD);
    QRadioButton *dimDimensionThreeD = new QRadioButton("3D", dim);
    dimLayout->addWidget(dimDimensionThreeD);
    connect(dim, SIGNAL(clicked(int)),
            this, SLOT(dimChanged(int)));
    mainLayout->addWidget(dim, 0,1);

    mainLayout->addWidget(new QLabel("Min, max, incr", central, "oneDLabel"),1,0);
    oneD = new QLineEdit(central, "oneD");
    connect(oneD, SIGNAL(returnPressed()),
            this, SLOT(oneDProcessText()));
    mainLayout->addWidget(oneD, 1,1);

    mainLayout->addWidget(new QLabel("Min, max, incr", central, "twoDLabel"),2,0);
    twoD = new QLineEdit(central, "twoD");
    connect(twoD, SIGNAL(returnPressed()),
            this, SLOT(twoDProcessText()));
    mainLayout->addWidget(twoD, 2,1);

    mainLayout->addWidget(new QLabel("Min, max, incr", central, "threeDLabel"),3,0);
    threeD = new QLineEdit(central, "xMin");
    connect(threeD, SIGNAL(returnPressed()),
            this, SLOT(threeDProcessText()));
    mainLayout->addWidget(threeD, 3,1);

    whichData = new QButtonGroup(central, "whichData");
    whichData->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *whichDataLayout = new QHBoxLayout(whichData);
    whichDataLayout->setSpacing(5);
    QRadioButton *whichDataDataTypeAllDomains = new QRadioButton("All", whichData);
    whichDataLayout->addWidget(whichDataDataTypeAllDomains);
    QRadioButton *whichDataDataTypeOneDomain = new QRadioButton("Block", whichData);
    whichDataLayout->addWidget(whichDataDataTypeOneDomain);

    domainIndex = new QLineEdit(whichData, "domainIndex");
    domainIndex->setMaximumWidth(30);
    connect(domainIndex, SIGNAL(returnPressed()),
            this, SLOT(domainIndexProcessText()));
    whichDataLayout->addWidget(domainIndex);

    QRadioButton *whichDataDataTypeOneGroup = new QRadioButton("Group", whichData);
    whichDataLayout->addWidget(whichDataDataTypeOneGroup);
    connect(whichData, SIGNAL(clicked(int)),
            this, SLOT(whichDataChanged(int)));

    groupIndex = new QLineEdit(whichData, "groupIndex");
    groupIndex->setMaximumWidth(30);
    connect(groupIndex, SIGNAL(returnPressed()),
            this, SLOT(groupIndexProcessText()));
    whichDataLayout->addWidget(groupIndex);

    wholeLayout->addWidget(whichData, 1,0);
}


// ****************************************************************************
// Method: QvisIndexSelectWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Thu Jun 6 17:02:08 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisIndexSelectWindow::UpdateWindow(bool doAll)
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

        char tmp[1024];
        switch(i)
        {
          case 0: //dim
            if (atts->GetDim() == IndexSelectAttributes::TwoD ||
                atts->GetDim() == IndexSelectAttributes::ThreeD)
                twoD->setEnabled(true);
            else
                twoD->setEnabled(false);
            if (atts->GetDim() == IndexSelectAttributes::ThreeD)
                threeD->setEnabled(true);
            else
                threeD->setEnabled(false);
            dim->setButton(atts->GetDim());
            break;
          case 1: //xMin
          case 2: //xMax
          case 3: //xIncr
            if (atts->GetXMax() < 0)
            {
                sprintf(tmp, "%d:max:%d", atts->GetXMin(), atts->GetXIncr());
            }
            else
            {
                sprintf(tmp, "%d:%d:%d", atts->GetXMin(), atts->GetXMax(),
                                         atts->GetXIncr());
            }
            temp = tmp;
            oneD->setText(temp);
            break;
          case 4: //yMin
          case 5: //yMax
          case 6: //yIncr
            if (atts->GetYMax() < 0)
            {
                sprintf(tmp, "%d:max:%d", atts->GetYMin(), atts->GetYIncr());
            }
            else
            {
                sprintf(tmp, "%d:%d:%d", atts->GetYMin(), atts->GetYMax(),
                                         atts->GetYIncr());
            }
            temp = tmp;
            twoD->setText(temp);
            break;

          case 7: //zMin
          case 8: //zMax
          case 9: //zIncr
            if (atts->GetZMax() < 0)
            {
                sprintf(tmp, "%d:max:%d", atts->GetZMin(), atts->GetZIncr());
            }
            else
            {
                sprintf(tmp, "%d:%d:%d", atts->GetZMin(), atts->GetZMax(),
                                         atts->GetZIncr());
            }
            temp = tmp;
            threeD->setText(temp);
            break;
          case 10: //whichData
            whichData->setButton(atts->GetWhichData());
            if (atts->GetWhichData() == IndexSelectAttributes::OneDomain)
            {
                domainIndex->setEnabled(true);
                groupIndex->setEnabled(false);
            }
            else if (atts->GetWhichData() == IndexSelectAttributes::OneGroup)
            {
                domainIndex->setEnabled(false);
                groupIndex->setEnabled(true);
            }
            else
            {
                domainIndex->setEnabled(false);
                groupIndex->setEnabled(false);
            }
            break;
          case 11: //domainIndex
            temp.sprintf("%d", atts->GetDomainIndex());
            domainIndex->setText(temp);
            break;
          case 12: //groupIndex
            temp.sprintf("%d", atts->GetGroupIndex());
            groupIndex->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisIndexSelectWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Thu Jun 6 17:02:08 PST 2002
//
// Modifications:
//   
//   Hank Childs, Wed Jun 19 10:00:03 PDT 2002
//   If a tuple does not parse, make sure not to use those values.
//
// ****************************************************************************

void
QvisIndexSelectWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do dim
    if(which_widget == 0 || doAll)
    {
        // Nothing for dim
    }

    // Do oneD
    if(which_widget == 1 || doAll)
    {
        temp = oneD->displayText().simplifyWhiteSpace();
        const char *tmp = temp;
        int vals[3];
        if (!ParseDimensions(tmp, vals))
        {
            msg.sprintf("The format for the first \"min, max, "
                "incr\" tuple is invalid.");
            Message(msg);
            // reset a value so the text for the window will be regenerated.
            atts->SetXMin(atts->GetXMin());
        }
        else
        {
            atts->SetXMin(vals[0]);
            atts->SetXMax(vals[1]);
            atts->SetXIncr(vals[2]);
        }
    }

    // Do twoD
    if(which_widget == 2 || doAll)
    {
        temp = twoD->displayText().simplifyWhiteSpace();
        const char *tmp = temp;
        int vals[3];
        if (!ParseDimensions(tmp, vals))
        {
            msg.sprintf("The format for the second \"min, max, "
                "incr\" tuple is invalid.");
            Message(msg);
            // reset a value so the text for the window will be regenerated.
            atts->SetYMin(atts->GetYMin());
        }
        else
        {
            atts->SetYMin(vals[0]);
            atts->SetYMax(vals[1]);
            atts->SetYIncr(vals[2]);
        }
    }

    // Do threeD
    if(which_widget == 3 || doAll)
    {
        temp = threeD->displayText().simplifyWhiteSpace();
        const char *tmp = temp;
        int vals[3];
        if (!ParseDimensions(tmp, vals))
        {
            msg.sprintf("The format for the third \"min, max, "
                "incr\" tuple is invalid.");
            Message(msg);
            // reset a value so the text for the window will be regenerated.
            atts->SetZMin(atts->GetZMin());
        }
        else
        {
            atts->SetZMin(vals[0]);
            atts->SetZMax(vals[1]);
            atts->SetZIncr(vals[2]);
        }
    }

    // Do whichData
    if(which_widget == 4 || doAll)
    {
        // Nothing for whichData
    }

    // Do domainIndex
    if(which_widget == 5 || doAll)
    {
        temp = domainIndex->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetDomainIndex(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of domainIndex was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetDomainIndex());
            Message(msg);
            atts->SetDomainIndex(atts->GetDomainIndex());
        }
    }

    // Do groupIndex
    if(which_widget == 6 || doAll)
    {
        temp = groupIndex->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            atts->SetGroupIndex(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of groupIndex was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetGroupIndex());
            Message(msg);
            atts->SetGroupIndex(atts->GetGroupIndex());
        }
    }
}

bool ParseDimensions(const char *tmp, int *vals)
{
    if (strstr(tmp, "max") != NULL)
    {
        if (strstr(tmp, ":") != NULL)
        {
            if (sscanf(tmp, "%d : max : %d", &(vals[0]), &(vals[2])) != 2)
            {
                return false;
            }
        }
        else
        {
            if (sscanf(tmp, "%d max %d", &(vals[0]), &(vals[2])) != 2)
            {
                return false;
            }
        }
        vals[1] = -1;
    }
    else
    {
        if (strstr(tmp, ":") != NULL)
        {
            if (sscanf(tmp, "%d : %d : %d", &(vals[0]), &(vals[1]), &(vals[2]))
                  != 3)
            {
                return false;
            }
        }
        else
        {
            if (sscanf(tmp, "%d %d %d", &(vals[0]), &(vals[1]), &(vals[2]))!=3)
            {
                return false;
            }
        }
    }

    if (vals[1] >= 0 && vals[1] < vals[0])
    {
        return false;
    }
    if (vals[2] <= 0)
    {
        return false;
    }

    return true;
}    

//
// Qt Slot functions
//


void
QvisIndexSelectWindow::dimChanged(int val)
{
    if(val != atts->GetDim())
    {
        atts->SetDim(IndexSelectAttributes::Dimension(val));
        Apply();
    }
}


void
QvisIndexSelectWindow::oneDProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisIndexSelectWindow::twoDProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisIndexSelectWindow::threeDProcessText()
{
    GetCurrentValues(3);
    Apply();
}


void
QvisIndexSelectWindow::whichDataChanged(int val)
{
    if(val != atts->GetWhichData())
    {
        atts->SetWhichData(IndexSelectAttributes::DataType(val));
        Apply();
    }
}


void
QvisIndexSelectWindow::domainIndexProcessText()
{
    GetCurrentValues(5);
    Apply();
}


void
QvisIndexSelectWindow::groupIndexProcessText()
{
    GetCurrentValues(6);
    Apply();
}


