#include "QvisDeferExpressionWindow.h"

#include <DeferExpressionAttributes.h>
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
// Method: QvisDeferExpressionWindow::QvisDeferExpressionWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Sep 20 13:25:13 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QvisDeferExpressionWindow::QvisDeferExpressionWindow(const int type,
                         DeferExpressionAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisDeferExpressionWindow::~QvisDeferExpressionWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Sep 20 13:25:13 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QvisDeferExpressionWindow::~QvisDeferExpressionWindow()
{
}


// ****************************************************************************
// Method: QvisDeferExpressionWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Sep 20 13:25:13 PST 2005
//
// Modifications:
//   
//   Hank Childs, Wed Sep 21 11:34:19 PDT 2005
//   Use QvisVariableButton to create variable list.
//
// ****************************************************************************

void
QvisDeferExpressionWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 1,2,  10, "mainLayout");

    varsButton = new QvisVariableButton(true, false, true, -1,
        central, "varsButton");
    varsButton->setText("Variables");
    varsButton->setChangeTextOnVariableChange(false);
    connect(varsButton, SIGNAL(activated(const QString &)),
            this, SLOT(addVariable(const QString &)));
    mainLayout->addWidget(varsButton, 0, 0);

    varsLineEdit = new QLineEdit(central, "varsLineEdit");
    varsLineEdit->setText("");
    connect(varsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(variableProcessText()));
    mainLayout->addWidget(varsLineEdit, 0, 1);


}


// ****************************************************************************
// Method: QvisDeferExpressionWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue Sep 20 13:25:13 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisDeferExpressionWindow::UpdateWindow(bool doAll)
{
    QString temp;
    double r;
    char str[1024];

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
          case 0: //exprs
            {
               str[0] = '\0';
               for (int j = 0 ; j < atts->GetExprs().size() ; j++)
                   sprintf(str + strlen(str), "%s%s", (j == 0 ? "" : " "),
                           atts->GetExprs()[j].c_str());
               temp = str;
               varsLineEdit->setText(temp);
            }
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisDeferExpressionWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Sep 20 13:25:13 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisDeferExpressionWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do exprs
    if(which_widget == 0 || doAll)
    {
        stringVector userVars;
        temp = varsLineEdit->displayText().simplifyWhiteSpace();
        QStringList lst(QStringList::split(" ", temp));

        QStringList::Iterator it;

        for (it = lst.begin(); it != lst.end(); ++it)
        {
            userVars.push_back((*it).latin1());
        }

        atts->SetExprs(userVars);
    }
}


//
// Qt Slot functions
//
void
QvisDeferExpressionWindow::variableProcessText()
{
    GetCurrentValues(0);
    Apply();
}

// ****************************************************************************
// Method: QvisPickWindow::addVariable
//
// Purpose:
//   This is a Qt slot function that is called when the user selects a new
//   pick variable.
//
// Arguments:
//   var : The pick variable to add.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 10 09:57:14 PDT 2004
//
// Modifications:
//
// ****************************************************************************

void
QvisDeferExpressionWindow::addVariable(const QString &var)
{
    // Add the new variable to the pick variable line edit.
    QString varString(varsLineEdit->displayText());
    if(varString.length() > 0)
        varString += " ";
    varString += var;
    varsLineEdit->setText(varString);

    // Process the list of pick vars.
    variableProcessText();
}


