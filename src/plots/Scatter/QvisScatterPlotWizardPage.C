/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <QvisScatterPlotWizardPage.h>

#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QRadioButton>

#include <QvisScatterWidget.h>
#include <QvisVariableButton.h>
#include <ScatterAttributes.h>


// ****************************************************************************
// Method: QvisScatterPlotWizardPage::QvisScatterPlotWizardPage
//
// Purpose:
//   Constructor for main wizard page for Scatter Plot setup.
//
//
// Arguments:
//   s       : Subject to use when communicating with the viewer
//   parent  : Wizard's parent widget
//   varName : Name of the plot's scalar variable
//
// Programmer: Cyrus Harrison
// Creation:  Wed Aug 18 15:26:15 PDT 2010
//
// Note: Refactored from QvisScatterPlotWizardPage.
//
// Modifications:
//
// ****************************************************************************

QvisScatterPlotWizardPage::QvisScatterPlotWizardPage(AttributeSubject *s,
                                                     QWidget *parent,
                                                 const std::string &xvar_name)
: QWizardPage(parent)
{
    ready = false;

    xVarName = "";
    yVarName = "";
    zVarName = "";
    colorVarName = "";

    QString unselected_text = tr("<Select>");

    QHBoxLayout *main_layout = new QHBoxLayout(this);
    setSubTitle(tr("Select variables for a new Scatter Plot."));
    preview = new QvisScatterWidget(this);
    preview->setThreeD(false);
    preview->setColoredPoints(false);

    main_layout->addWidget(preview);
    main_layout->addSpacing(10);

    QVBoxLayout *right_layout = new QVBoxLayout();
    main_layout->addLayout(right_layout);

    QGridLayout *grid_layout= new QGridLayout();
    right_layout->addLayout(grid_layout);
    xVarButton = new QvisVariableButton(true, false, true,
                     QvisVariableButton::Scalars | QvisVariableButton::Curves, this);

    xVarButton->setText(QString(xvar_name.c_str()));
    xVarName = xvar_name;
    QLabel *xvar_label = new QLabel(tr("X Variable"),this);

    xVarButton->setMinimumWidth(fontMetrics().boundingRect("really_really_long_var_name").width());
    connect(xVarButton, SIGNAL(activated(const QString &)),
            this, SLOT(choseXVariable(const QString &)));
    grid_layout->addWidget(xvar_label,0,1);
    grid_layout->addWidget(xVarButton,0,2);


    yVarButton = new QvisVariableButton(true, false, true,
        QvisVariableButton::Scalars | QvisVariableButton::Curves, this);

    QLabel *yvar_label = new QLabel(tr("Y Variable"),this);
    yVarButton->setMinimumWidth(fontMetrics().boundingRect("really_really_long_var_name").width());
    connect(yVarButton, SIGNAL(activated(const QString &)),
            this, SLOT(choseYVariable(const QString &)));
    grid_layout->addWidget(yvar_label,1,1);
    grid_layout->addWidget(yVarButton,1,2);

    yVarButton->setText(unselected_text);

    zVarButton = new QvisVariableButton(true, false, true,
                      QvisVariableButton::Scalars | QvisVariableButton::Curves,this);

    zVarButton->setEnabled(false);
    zVarCheck= new QCheckBox("",this);
    zVarCheck->setCheckState(Qt::Unchecked);
    QLabel *zvar_label = new QLabel(tr("Z Variable"),this);
    zVarButton->setMinimumWidth(fontMetrics().boundingRect("really_really_long_var_name").width());
    connect(zVarButton, SIGNAL(activated(const QString &)),
            this, SLOT(choseZVariable(const QString &)));
    connect(zVarCheck, SIGNAL(stateChanged (int)),
            this, SLOT(decideZ(int)));

    zVarButton->setText(unselected_text);
    grid_layout->addWidget(zVarCheck,2,0);
    grid_layout->addWidget(zvar_label,2,1);
    grid_layout->addWidget(zVarButton,2,2);

    colorVarButton = new QvisVariableButton(true, false, true,
        QvisVariableButton::Scalars | QvisVariableButton::Curves, this);
    colorVarButton->setEnabled(false);
    colorVarCheck= new QCheckBox(tr(""),this);
    colorVarCheck->setCheckState(Qt::Unchecked);

    QLabel *cvar_label = new QLabel(tr("Color Variable"),this);
    colorVarButton->setText(unselected_text);
    colorVarButton->setMinimumWidth(fontMetrics().boundingRect("really_really_long_var_name").width());
    connect(colorVarButton, SIGNAL(activated(const QString &)),
            this, SLOT(choseColorVariable(const QString &)));
    connect(colorVarCheck, SIGNAL(stateChanged (int)),
            this, SLOT(decideColor(int)));
    grid_layout->addWidget(colorVarCheck,3,0);
    grid_layout->addWidget(cvar_label,3,1);
    grid_layout->addWidget(colorVarButton,3,2);
    grid_layout->setRowStretch(3,10);
    main_layout->addStretch(10);
    right_layout->addStretch(10);
}



// ****************************************************************************
// Method: QvisScatterPlotWizardPage::~QvisScatterPlotWizardPage
//
// Purpose: Destructor for the QvisScatterPlotWizardPage class.
//
// Programmer: Cyrus Harrison
// Creation:  Wed Aug 18 15:26:15 PDT 2010
//
//
// Modifications:
//
//
// ****************************************************************************
QvisScatterPlotWizardPage::~QvisScatterPlotWizardPage()
{

}

// ****************************************************************************
// Method: QvisScatterPlotWizardPage::GetSelectedVars()
//
// Purpose: Returns the names of the selected variables.
//
// Programmer: Cyrus Harrison
// Creation:  Wed Aug 18 15:26:15 PDT 2010
//
//
// Modifications:
//
// ****************************************************************************
void
QvisScatterPlotWizardPage::GetSelectedVars(stringVector &res) const
{
    res.clear();
    res.push_back(xVarName);
    res.push_back(yVarName);
    res.push_back(zVarName);
    res.push_back(colorVarName);
}

// ****************************************************************************
// Method: QvisScatterPlotWizardPage::ZEnabled()
//
// Purpose: Returns true if the z var check box is checked.
//
// Programmer: Cyrus Harrison
// Creation:  Wed Aug 18 15:26:15 PDT 2010
//
//
// Modifications:
//
// ****************************************************************************
bool
QvisScatterPlotWizardPage::ZEnabled() const
{
    return zVarCheck->checkState() == Qt::Checked;
}

// ****************************************************************************
// Method: QvisScatterPlotWizardPage::ColorEnabled()
//
// Purpose: Returns true if the color var check box is checked.
//
// Programmer: Cyrus Harrison
// Creation:  Wed Aug 18 15:26:15 PDT 2010
//
//
// Modifications:
//
// ****************************************************************************
bool
QvisScatterPlotWizardPage::ColorEnabled() const
{
    return colorVarCheck->checkState() == Qt::Checked;
}


// ****************************************************************************
// Method: QvisScatterPlotWizardPage::isComplete
//
// Purpose:
//   Checks that enables the 'Finish' button.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Aug 18 15:27:51 PDT 2010
//
//
// Modifications:
//
// ****************************************************************************

bool QvisScatterPlotWizardPage::isComplete() const
{
    return ready;
}


// ****************************************************************************
// Method: QvisScatterPlotWizardPage::checkComplete
//
// Purpose:
//   Sets value of 'ready' based on if all required vars are selected.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Aug 18 15:27:51 PDT 2010
//
//
// Modifications:
//
// ****************************************************************************

void QvisScatterPlotWizardPage::checkComplete()
{
    bool nready = (xVarName != "") && (yVarName != "");

    if(ZEnabled())
        nready = nready && (zVarName != "");

    if(ColorEnabled())
        nready = nready && (colorVarName != "");

    if(nready!=ready)
    {
        ready = nready;
        emit completeChanged();
    }
}


void
QvisScatterPlotWizardPage::choseXVariable(const QString &var)
{
    xVarName = var.toStdString();
    checkComplete();
}

void
QvisScatterPlotWizardPage::choseYVariable(const QString &var)
{
    yVarName = var.toStdString();
    checkComplete();
}

void
QvisScatterPlotWizardPage::choseZVariable(const QString &var)
{
    zVarName = var.toStdString();
    checkComplete();
}

void
QvisScatterPlotWizardPage::choseColorVariable(const QString &var)
{
    colorVarName = var.toStdString();
    checkComplete();
}

void
QvisScatterPlotWizardPage::decideZ(int state)
{
    if(state == Qt::Checked)
    {
        // enable z var selection
        zVarButton->setEnabled(true);
        preview->setThreeD(true);
    }
    else
    {
        // disable z var selection
        zVarButton->setEnabled(false);
        preview->setThreeD(false);
    }
    checkComplete();
}


void
QvisScatterPlotWizardPage::decideColor(int state)
{
    if(state == Qt::Checked)
    {
        // enable color var sel
        colorVarButton->setEnabled(true);
        preview->setColoredPoints(true);
    }
    else
    {
        // disable enable color var sel
        colorVarButton->setEnabled(false);
        preview->setColoredPoints(false);
    }
    checkComplete();
}

