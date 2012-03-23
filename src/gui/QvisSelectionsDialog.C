/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

#include <QvisSelectionsDialog.h>

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QRadioButton>

#include <QvisVariableButton.h>

// ****************************************************************************
// Method: QvisSelectionsDialog::QvisSelectionsDialog
//
// Purpose: 
//   Constructor
//
// Arguments:
//   m      : The selection mode.
//   parent : The widget's parent.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 15:55:11 PDT 2011
//
// Modifications:
//   Brad Whitlock, Sat Nov  5 02:42:38 PDT 2011
//   Let the user pick different id types for the selection.
//
//   Dave Pugmire, Thu Mar 15 11:19:33 EDT 2012
//   Add location named selections.
//
// ****************************************************************************

QvisSelectionsDialog::QvisSelectionsDialog(
    QvisSelectionsDialog::SourceSelectionMode m, QWidget *parent) :
    QDialog(parent),
    choosePlot(true), selectionName(), plot(), db()
{
    // When plots are available, prefer plots.
    choosePlot = (m == SOURCE_USE_DB_OR_PLOT);

    setWindowTitle(tr("Selection Name and Source"));

    QWidget *central = this;
    QGridLayout *gLayout = new QGridLayout(central);

    QLabel *selName = new QLabel(tr("Selection name"), central);
    gLayout->addWidget(selName, 0, 0);

    int row = 0;
    selectionNameLineEdit = new QLineEdit(central);
    gLayout->addWidget(selectionNameLineEdit, row, 1, 1, 2);
    ++row;

    QString dbTxt(tr("Database"));
    if(m == SOURCE_USE_DB_OR_PLOT)
    {
        QButtonGroup *bg = new QButtonGroup(central);
        connect(bg, SIGNAL(buttonClicked(int)),
                this, SLOT(selectionChoiceChanged(int)));
        QRadioButton *b0 = new QRadioButton(tr("Plot"), central);
        bg->addButton(b0, 0);
        QRadioButton *b1 = new QRadioButton(dbTxt, central);
        bg->addButton(b1, 1);
        bg->blockSignals(true);
        if(choosePlot)
            b0->setChecked(true);
        else
            b1->setChecked(true);
        bg->blockSignals(false);

        plotLabel = new QLabel(central);
        gLayout->addWidget(b0, row, 0);
        gLayout->addWidget(plotLabel, row, 1, 1, 2);
        ++row;

        dbLabel = new QLabel(central);
        gLayout->addWidget(b1, row, 0);
        gLayout->addWidget(dbLabel, row, 1, 1, 2);
        ++row;
    }
    else
    {
        dbLabel = new QLabel(central);
        gLayout->addWidget(new QLabel(dbTxt, central), 1, 0);
        gLayout->addWidget(dbLabel, row, 1, 1, 2);
        ++row;

        plotLabel = 0;
    }

    // Add controls that let the user pick the method for relating cells.
    QGroupBox *idGroup = new QGroupBox(tr("Relate cells using"), central);
    QGridLayout *iLayout = new QGridLayout(idGroup);
    QButtonGroup *bg2 = new QButtonGroup(central);
    QRadioButton *b0 = new QRadioButton(tr("Domain and cell numbers"), central);
    bg2->addButton(b0, 0);
    iLayout->addWidget(b0, 0, 0);
    QRadioButton *b1 = new QRadioButton(tr("Global cell numbers"), central);
    bg2->addButton(b1, 1);
    iLayout->addWidget(b1, 0, 1);
    QRadioButton *b2 = new QRadioButton(tr("Locations"), central);
    bg2->addButton(b2, 2);
    iLayout->addWidget(b2, 0, 2);

    QRadioButton *b3 = new QRadioButton(tr("Variable"), central);
    bg2->addButton(b3, 3);
    iLayout->addWidget(b3, 0, 3);
    b0->setChecked(true);
    connect(bg2, SIGNAL(buttonClicked(int)),
            this, SLOT(idVariableTypeChanged(int)));
    idVariableButton = new QvisVariableButton(central);
    idVariableButton->setVarTypes(QvisVariableButton::Scalars);
    idVariableButton->setAddDefault(false);
    idVariableButton->setEnabled(false);
    connect(idVariableButton, SIGNAL(activated(const QString &)),
            this, SLOT(idVariableChanged(const QString &)));
    iLayout->addWidget(idVariableButton, 0, 4);
    gLayout->addWidget(idGroup, row, 0, 1, 4);
    ++row;

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, central);
    gLayout->addWidget(buttonBox, row, 0, 1, 3);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

// ****************************************************************************
// Method: QvisSelectionsDialog::~QvisSelectionsDialog
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 15:55:51 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

QvisSelectionsDialog::~QvisSelectionsDialog()
{
}

// Set methods.
void
QvisSelectionsDialog::setSelectionName(const QString &val)
{
    selectionName = val;
}

void
QvisSelectionsDialog::setPlotName(const QString &val)
{
    plot = val;
}

void
QvisSelectionsDialog::setDBName(const QString &val)
{
    db = val;
}

// ****************************************************************************
// Method: QvisSelectionsDialog::exec
//
// Purpose: 
//   Dialog exec method that returns the selection name, source, and whether
//   the selection source was a plot.
//
// Arguments:
//   selName : the selection name.
//   selSource : the selection source.
//   plotSource : Whether the selection source is a plot.
//
// Returns:    Accepted or Aborted.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 15:56:13 PDT 2011
//
// Modifications:
//   Brad Whitlock, Sat Nov  5 02:17:31 PDT 2011
//   Return the idtype and id variable.
//
// ****************************************************************************

int
QvisSelectionsDialog::exec(QString &selName, QString &selSource, bool &plotSource,
    IDVariableType &idtype, QString &idvar)
{
    selectionNameLineEdit->setText(selectionName);
    dbLabel->setText(db);
    dbLabel->setToolTip(db);
    if(plotLabel != NULL)
    {
        plotLabel->setText(plot);
        plotLabel->setToolTip(plot);
    }

    int retval = QDialog::exec();

    selName = selectionNameLineEdit->text();
    selSource = choosePlot ? plot : db;
    plotSource = choosePlot;
    idtype = idType;
    idvar = (idType == UseVariableForID) ? idVariable : QString();

    return retval;
}

//
// Qt slots
//

// ****************************************************************************
// Method: QvisSelectionsDialog::selectionChoiceChanged
//
// Purpose: 
//   This Qt slot lets us pick the selection type.
//
// Arguments:
//   index : 0 for plots, 1 for databases.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 15:57:21 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsDialog::selectionChoiceChanged(int index)
{
    choosePlot = index == 0;

    plotLabel->setEnabled(index == 0);
    plotLabel->setEnabled(index == 1); 
}

// ****************************************************************************
// Method: QvisSelectionsDialog::idVariableChanged
//
// Purpose: 
//   This slot is called when we select a new id variable.
//
// Arguments:
//   var : The new id variable.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov  7 14:22:41 PST 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSelectionsDialog::idVariableChanged(const QString &var)
{
    idVariable = var;
}

// ****************************************************************************
// Method: QvisSelectionsDialog::idVariableTypeChanged
//
// Purpose: 
//   This slot is called when we select a new id variable type.
//
// Arguments:
//   val : The new id variable type.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov  7 14:23:24 PST 2011
//
// Modifications:
//
//   Dave Pugmire, Thu Mar 15 11:19:33 EDT 2012
//   Add location named selections.
//   
// ****************************************************************************

void
QvisSelectionsDialog::idVariableTypeChanged(int val)
{
    if(val == 0)
       idType = UseZoneIDForID;
    else if(val == 1)
       idType = UseGlobalZoneIDForID;
    else if(val == 2)
        idType = UseLocationsForID;
    else if(val == 3)
       idType = UseVariableForID;

    idVariableButton->setEnabled(val == 2);
}
