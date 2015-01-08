/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <QvisDBOptionsDialog.h>
#include <QvisDBOptionsHelpWindow.h>
#include <QLayout>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>

#include <cstring>

#include <DebugStream.h>
#include <DBOptionsAttributes.h>

// ****************************************************************************
// Method: QvisDBOptionsDialog::QvisDBOptionsDialog
//
// Purpose: 
//   Constructor
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: 
// Creation:   Tue Apr  8 11:12:48 PDT 2008
//
// Modifications:
//    Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//    Support for internationalization.
//   
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
//    Mark C. Miller, Mon Mar 16 23:10:47 PDT 2009
//    Added logic to skip obsolete options. Fixed issue setting current
//    index for Enums (combo boxes).
//
//    Jeremy Meredith, Mon Jan  4 14:27:45 EST 2010
//    Fixed bug with floats.
//
//    Mark C. Miller, Thu Dec 18 13:06:50 PST 2014
//    Added tr() around the name strings passed to the Qt items. Also,
//    added the help button.
// ****************************************************************************

QvisDBOptionsDialog::QvisDBOptionsDialog(DBOptionsAttributes *dbatts,
                                         QWidget *parent)
: QDialog(parent), atts(dbatts)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    int size = atts->GetNumberOfOptions();

    QGridLayout *grid = new QGridLayout();

    QLineEdit *ledit;
    
    topLayout->addLayout(grid);
    for (int i=0; i<size; i++)
    {
        QString txt;
        std::string name = atts->GetName(i);
        if (atts->IsObsolete(name))
            continue;
        DBOptionsAttributes::OptionType t = atts->GetType(i);
        switch (t)
        {
          case DBOptionsAttributes::Bool:
            { // new scope
            QCheckBox *chk_box = new QCheckBox(tr(name.c_str()), this);
            chk_box->setChecked(atts->GetBool(name));
            grid->addWidget(chk_box, i,0,1,2);
            checkboxes.append(chk_box);
            }
            break;
          case DBOptionsAttributes::Int:
            txt.setNum(atts->GetInt(name));
            ledit = new QLineEdit(txt, this);
            grid->addWidget(new QLabel(tr(name.c_str()), this), i, 0);
            grid->addWidget(ledit, i, 1);
            lineedits.append(ledit);
            break;
          case DBOptionsAttributes::Float:
            txt.setNum(atts->GetFloat(name));
            ledit = new QLineEdit(txt, this);
            grid->addWidget(new QLabel(tr(name.c_str()), this), i, 0);
            grid->addWidget(ledit, i, 1);
            lineedits.append(ledit);
            break;
          case DBOptionsAttributes::Double:
            txt.setNum(atts->GetDouble(name));
            ledit = new QLineEdit(txt, this);
            grid->addWidget(new QLabel(tr(name.c_str()), this), i, 0);
            grid->addWidget(ledit, i, 1);
            lineedits.append(ledit);
            break;
          case DBOptionsAttributes::String:
            txt = atts->GetString(name).c_str();
            ledit = new QLineEdit(txt, this);
            grid->addWidget(new QLabel(tr(name.c_str()), this), i, 0);
            grid->addWidget(ledit, i, 1);
            lineedits.append(ledit);
            break;
          case DBOptionsAttributes::Enum:
            { // new scope
            QComboBox *cbo_box = new QComboBox(this);
            for (size_t j=0; j<atts->GetEnumStrings(name).size(); j++)
            {
                QString curr_name(tr(atts->GetEnumStrings(name)[j].c_str()));
                cbo_box->addItem(curr_name);
            }
            cbo_box->setCurrentIndex(atts->GetEnum(name));
            
            grid->addWidget(new QLabel(tr(name.c_str()), this), i, 0);
            grid->addWidget(cbo_box, i, 1);
            comboboxes.append(cbo_box);
            }
            break;
        }
    }
   
    QHBoxLayout *btnLayout = new QHBoxLayout();
    topLayout->addLayout(btnLayout);
    //btnLayout->addStretch(10);
    okButton = new QPushButton(tr("OK"), this);
    connect(okButton, SIGNAL(clicked()),
            this, SLOT(okayClicked()));
    btnLayout->addWidget(okButton);

    helpButton = new QPushButton(tr("Help"), this);
    connect(helpButton, SIGNAL(clicked()),
            this, SLOT(helpClicked()));
    btnLayout->addWidget(helpButton);
    if (atts->GetHelp() == "")
        helpButton->setEnabled(false);

    cancelButton = new QPushButton(tr("Cancel"), this);
    connect(cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    btnLayout->addWidget(cancelButton);
}

// ****************************************************************************
// Method: QvisDBOptionsDialog::~QvisDBOptionsDialog
//
// Purpose: 
//   Destructor
//
// Creation:   Tue Apr  8 11:12:48 PDT 2008
//
// Modifications:
//
// ****************************************************************************

QvisDBOptionsDialog::~QvisDBOptionsDialog()
{}

// ****************************************************************************
// Method: QvisDBOptionsDialog::okayClicked
//
// Purpose: 
//   Slot to handle updating options.
//
// Creation:   Tue Apr  8 11:12:48 PDT 2008
//
// Modifications:
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
//    Brad Whitlock, Wed Dec  3 08:52:25 PST 2008
//    Index into the widget arrays using different indices. Added debugging code.
//
//    Mark C. Miller, Mon Mar 16 23:10:47 PDT 2009
//    Added logic to skip obsolete options.
// ****************************************************************************

void
QvisDBOptionsDialog::okayClicked()
{
    const char *mName = "QvisDBOptionsDialog::okayClicked: ";
    int size = atts->GetNumberOfOptions();
    int lineedit_index = 0;
    int checkbox_index = 0;
    int combobox_index = 0;
    for (int i=0; i<size; i++)
    {
        QString txt;
        std::string name = atts->GetName(i);
        if (atts->IsObsolete(name))
            continue;
        DBOptionsAttributes::OptionType t = atts->GetType(i);
        switch (t)
        {
          case DBOptionsAttributes::Bool:
          {
            bool val = checkboxes[checkbox_index++]->isChecked();
            debug5 << mName << "Setting \"" << name.c_str() << "\" to " 
                   << (val?"true":"false") << endl;
            atts->SetBool(name, val);
          }
            break;
          case DBOptionsAttributes::Int:
          {
            int val = lineedits[lineedit_index++]->displayText().toInt();
            debug5 << mName << "Setting \"" << name.c_str() << "\" to " << val << endl;
            atts->SetInt(name, val);
          }
            break;
          case DBOptionsAttributes::Float:
          {
            float val = lineedits[lineedit_index++]->displayText().toFloat();
            debug5 << mName << "Setting \"" << name.c_str() << "\" to " << val << endl;
            atts->SetFloat(name, val);
          }
            break;
          case DBOptionsAttributes::Double:
          {
            double val = lineedits[lineedit_index++]->displayText().toDouble();
            debug5 << mName << "Setting \"" << name.c_str() << "\" to " << val << endl;
            atts->SetDouble(name, val);
          }
            break;
          case DBOptionsAttributes::String:
          {
            std::string val = lineedits[lineedit_index++]->displayText().toStdString();
            debug5 << mName << "Setting \"" << name.c_str() << "\" to " << val.c_str() << endl;
            atts->SetString(name, val);
          }
            break;
          case DBOptionsAttributes::Enum:
          {
            int val = comboboxes[combobox_index++]->currentIndex();
            debug5 << mName << "Setting \"" << name.c_str() << "\" to " << val << endl;
            atts->SetEnum(name, val);
          }
            break;
        }
    }
    

    accept();
}

// ****************************************************************************
// Method: QvisDBOptionsDialog::helpClicked
//
// Purpose: Slot to handle help button
//
// Creation: Mark C. Miller, Fri Dec 19 14:58:24 PST 2014
//
// ****************************************************************************
void
QvisDBOptionsDialog::helpClicked()
{
    char format[128];
    QvisDBOptionsHelpWindow *optshelp = new QvisDBOptionsHelpWindow(atts, NULL);
    QString title = this->windowTitle();
    if (sscanf(title.toStdString().c_str(), "Default file opening options for %s reader", format) != 1)
        if (sscanf(title.toStdString().c_str(), "Export options for %s writer", format) != 1)
            strncpy(format, "unknown", sizeof(format));
    QString caption = tr("Options help for %1 plugin").arg(format);
    optshelp->setWindowTitle(caption);
    optshelp->exec();
    delete optshelp;
}
