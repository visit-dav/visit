/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <DBOptionsAttributes.h>

QvisDBOptionsDialog::QvisDBOptionsDialog(DBOptionsAttributes *dbatts,
                                         QWidget *parent, const char *name)
    : QDialog(parent,name), atts(dbatts)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setMargin(10);
    topLayout->setSpacing(5);

    int size = atts->GetNumberOfOptions();
    checkboxes = new QCheckBox*[size];
    lineedits = new QLineEdit*[size];
    comboboxes = new QComboBox*[size];
    QGridLayout *grid = new QGridLayout(topLayout, size, 2);
    for (int i=0; i<size; i++)
    {
        QString txt;
        std::string name = atts->GetName(i);
        DBOptionsAttributes::OptionType t = atts->GetType(i);
        switch (t)
        {
          case DBOptionsAttributes::Bool:
            checkboxes[i] = new QCheckBox(name.c_str(), this);
            checkboxes[i]->setChecked(atts->GetBool(name));
            grid->addMultiCellWidget(checkboxes[i], i,i, 0,1);
            break;
          case DBOptionsAttributes::Int:
            txt.setNum(atts->GetInt(name));
            lineedits[i] = new QLineEdit(txt, this);
            grid->addWidget(new QLabel(name.c_str(), this), i, 0);
            grid->addWidget(lineedits[i], i, 1);
            break;
          case DBOptionsAttributes::Float:
            txt.setNum(atts->GetFloat(name));
            lineedits[i] = new QLineEdit(txt, this);
            grid->addWidget(new QLabel(name.c_str(), this), i, 0);
            grid->addWidget(lineedits[i], i, 1);
            break;
          case DBOptionsAttributes::Double:
            txt.setNum(atts->GetDouble(name));
            lineedits[i] = new QLineEdit(txt, this);
            grid->addWidget(new QLabel(name.c_str(), this), i, 0);
            grid->addWidget(lineedits[i], i, 1);
            break;
          case DBOptionsAttributes::String:
            txt = atts->GetString(name).c_str();
            lineedits[i] = new QLineEdit(txt, this);
            grid->addWidget(new QLabel(name.c_str(), this), i, 0);
            grid->addWidget(lineedits[i], i, 1);
            break;
          case DBOptionsAttributes::Enum:
            comboboxes[i] = new QComboBox(false, this);
            for (int j=0; j<atts->GetEnumStrings(name).size(); j++)
                comboboxes[i]->insertItem(atts->GetEnumStrings(name)[j].c_str());
            comboboxes[i]->setCurrentItem(atts->GetEnum(name));
            grid->addWidget(new QLabel(name.c_str(), this), i, 0);
            grid->addWidget(comboboxes[i], i, 1);
            break;
        }
    }
   
    QHBoxLayout *btnLayout = new QHBoxLayout(topLayout);
    btnLayout->addStretch(10);
    okButton = new QPushButton("OK", this, "okButton");
    connect(okButton, SIGNAL(clicked()),
            this, SLOT(okayClicked()));
    btnLayout->addWidget(okButton);

    cancelButton = new QPushButton("Cancel", this, "cancelButton");
    connect(cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    btnLayout->addWidget(cancelButton);
}

QvisDBOptionsDialog::~QvisDBOptionsDialog()
{
}

void
QvisDBOptionsDialog::okayClicked()
{
    int size = atts->GetNumberOfOptions();
    for (int i=0; i<size; i++)
    {
        QString txt;
        std::string name = atts->GetName(i);
        DBOptionsAttributes::OptionType t = atts->GetType(i);
        switch (t)
        {
          case DBOptionsAttributes::Bool:
            atts->SetBool(name, checkboxes[i]->isChecked());
            break;
          case DBOptionsAttributes::Int:
            atts->SetInt(name, lineedits[i]->displayText().toInt());
            break;
          case DBOptionsAttributes::Float:
            atts->SetFloat(name, lineedits[i]->displayText().toFloat());
            break;
          case DBOptionsAttributes::Double:
            atts->SetDouble(name, lineedits[i]->displayText().toDouble());
            break;
          case DBOptionsAttributes::String:
            atts->SetString(name, lineedits[i]->displayText().latin1());
            break;
          case DBOptionsAttributes::Enum:
            atts->SetEnum(name, comboboxes[i]->currentItem());
            break;
        }
    }
    

    accept();
}
