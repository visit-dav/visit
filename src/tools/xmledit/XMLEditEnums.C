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
#include "XMLEditStd.h"
#include "XMLEditEnums.h"

#include <XMLDocument.h>
#include <Attribute.h>
#include <Field.h>
#include <QLineEdit>
#include <QLabel>
#include <QLayout>
#include <qlistwidget.h>
#include <QTextEdit>
#include <QPushButton>
#include <Enum.h>
#include <XMLParserUtil.h>


// ****************************************************************************
//  Constructor:  XMLEditEnums::XMLEditEnums
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// Modifications:
//   Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//   Ported to Qt 4.4
//
// ****************************************************************************
XMLEditEnums::XMLEditEnums(QWidget *p)
    : QFrame(p)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    
    QGridLayout *listLayout = new QGridLayout;

    enumlist = new QListWidget(this);
    listLayout->addWidget(enumlist, 0,0, 1,2);

    newButton = new QPushButton(tr("New"), this);
    listLayout->addWidget(newButton, 1,0);

    delButton = new QPushButton(tr("Del"), this);
    listLayout->addWidget(delButton, 1,1);
    
    hLayout->addLayout(listLayout); 
    hLayout->addSpacing(10);

    QGridLayout *topLayout = new QGridLayout();
    topLayout->setColumnMinimumWidth(1, 20);
    int row = 0;

    name = new QLineEdit(this);
    topLayout->addWidget(new QLabel(tr("Name"), this), row, 0);
    topLayout->addWidget(name, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("Values"), this), row, 0);
    row++;

    valuelist = new QTextEdit(this);
    QFont monospaced("Courier");
    valuelist->setFont(monospaced);
    valuelist->setWordWrapMode(QTextOption::NoWrap);
    topLayout->addWidget(valuelist, row,0, 1,2);
    row++;

    topLayout->setRowMinimumHeight(row, 20);
    row++;
    hLayout->addLayout(topLayout); 

    // ------------------------------------------------------------

    connect(enumlist, SIGNAL(currentRowChanged(int)),
            this, SLOT(UpdateWindowSingleItem()));
    connect(name, SIGNAL(textChanged(const QString&)),
            this, SLOT(nameTextChanged(const QString&)));
    connect(valuelist, SIGNAL(textChanged()),
            this, SLOT(valuelistChanged()));
    
    connect(newButton, SIGNAL(pressed()),
            this, SLOT(enumlistNew()));
    connect(delButton, SIGNAL(pressed()),
            this, SLOT(enumlistDel()));
}

// ****************************************************************************
//  Method:  XMLEditEnums::removeEmptyLines
//
//  Purpose:
//    Remove extra empty lines in the widget.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// Modifications:
//   Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//   Ported to Qt 4.4
//
// ****************************************************************************
void
XMLEditEnums::removeEmptyLines()
{
    /* TODO
    bool done = false;
    while (!done)
    {
        done = true;
        for (int i=0; i<valuelist->numLines()-1; i++)
        {
            int line, col;
            valuelist->getCursorPosition(&line, &col);
            if (i == line)
                continue;

            if (valuelist->textLine(i).isEmpty())
            {
                valuelist->removeLine(i);
                done = false;
                break;
            }
        }
    }
    */
}

// ****************************************************************************
//  Method:  XMLEditEnums::addEmptyLine
//
//  Purpose:
//    Add a single empty line in response to a key press.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// Modifications:
//   Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//   Ported to Qt 4.4
//
// ****************************************************************************
void
XMLEditEnums::addEmptyLine()
{
    /* TODO
    int line, col;
    valuelist->getCursorPosition(&line, &col);
    if (line > 0 && valuelist->textLine(line-1).isEmpty())
    {
        valuelist->setCursorPosition(line-1,0);
    }
    */
}

// ****************************************************************************
//  Method:  XMLEditEnums::UpdateWindowContents
//
//  Purpose:
//    Update the window based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// Modifications:
//   Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//   Ported to Qt 4.4
//
// ****************************************************************************
void
XMLEditEnums::UpdateWindowContents()
{
    BlockAllSignals(true);

    enumlist->clear();
    for (size_t i=0; i<EnumType::enums.size(); i++)
    {
        enumlist->addItem(EnumType::enums[i]->type);
    }
    UpdateWindowSingleItem();

    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditEnums::UpdateWindowSensitivity
//
//  Purpose:
//    Enable/disable widget sensitivity based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// Modifications:
//   Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//   Ported to Qt 4.4
//
// ****************************************************************************
void
XMLEditEnums::UpdateWindowSensitivity()
{
    delButton->setEnabled(enumlist->count() > 0);

    if (enumlist->currentRow() != -1)
    {
        name->setEnabled(true);
        valuelist->setEnabled(true);
    }
    else
    {
        name->setEnabled(false);
        valuelist->setEnabled(false);
    }
}

// ****************************************************************************
//  Method:  XMLEditEnums::UpdateWindowSingleItem
//
//  Purpose:
//    Update the window based on the state a single item in the list.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// Modifications:
//   Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//   Ported to Qt 4.4
//
// ****************************************************************************
void
XMLEditEnums::UpdateWindowSingleItem()
{
    BlockAllSignals(true);
    int index = enumlist->currentRow();

    if (index == -1)
    {
        name->setText("");
        valuelist->setText("");
    }
    else
    {
        EnumType *e = EnumType::FindEnum(enumlist->currentItem()->text());
        name->setText(e->type);
        valuelist->setText(JoinValues(e->values, '\n'));
    }
    UpdateWindowSensitivity();
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditEnums::BlockAllSignals
//
//  Purpose:
//    Blocks/unblocks signals to the widgets.  This lets them get
//    updated by changes in state without affecting the state.
//
//  Arguments:
//    block      whether to block (true) or unblock (false) signals
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditEnums::BlockAllSignals(bool block)
{
    enumlist->blockSignals(block);
    name->blockSignals(block);
    valuelist->blockSignals(block);
}

// ----------------------------------------------------------------------------
//                                 Callbacks
// ----------------------------------------------------------------------------

// ****************************************************************************
//  Method:  XMLEditEnums::nameTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// Modifications:
//   Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//   Ported to Qt 4.4
//
// ****************************************************************************
void
XMLEditEnums::nameTextChanged(const QString &text)
{
    int index = enumlist->currentRow();

    if (index == -1)
        return;

    EnumType *e = EnumType::FindEnum(enumlist->currentItem()->text());
    QString newname = text.trimmed();
    e->type = newname;
    BlockAllSignals(true);
    enumlist->item(index)->setText(newname);
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditEnums::valuelistChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// Modifications:
//   Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//   Ported to Qt 4.4
//
// ****************************************************************************
void
XMLEditEnums::valuelistChanged()
{
    removeEmptyLines();

    int index = enumlist->currentRow();

    if (index == -1)
        return;

    EnumType *e = EnumType::FindEnum(enumlist->currentItem()->text());
    e->values = SplitValues(valuelist->toPlainText());
}

// ****************************************************************************
//  Method:  XMLEditEnums::enumlistNew
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// Modifications:
//   Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//   Ported to Qt 4.4
//
// ****************************************************************************
void
XMLEditEnums::enumlistNew()
{
    int newid = 1;
    bool okay = false;
    QString newtype;
    while (!okay)
    {
        okay = true;
        newtype = tr("unnamed%1").arg(newid);
        for (size_t i=0; i<enumlist->count() && okay; i++)
        {
            if (enumlist->item(i)->text() == newtype)
                okay = false;
        }
        if (!okay)
            newid++;
    }
    
    EnumType *e = new EnumType(newtype);
    EnumType::enums.push_back(e);
    UpdateWindowContents();
    for (size_t i=0; i<enumlist->count(); i++)
    {
        if (enumlist->item(i)->text() == newtype)
        {
            enumlist->setCurrentRow(i);
            UpdateWindowSingleItem();
        }
    }
}

// ****************************************************************************
//  Method:  XMLEditEnums::enumlistDel
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// Modifications:
//   Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//   Ported to Qt 4.4
//
// ****************************************************************************
void
XMLEditEnums::enumlistDel()
{
    int index = enumlist->currentRow();

    if (index == -1)
        return;

    EnumType *e = EnumType::FindEnum(enumlist->currentItem()->text());
    vector<EnumType*> newlist;
    for (size_t i=0; i<EnumType::enums.size(); i++)
    {
        if (EnumType::enums[i] != e)
            newlist.push_back(EnumType::enums[i]);
    }
    EnumType::enums = newlist;

    // Make sure anyone with a reference to the old one
    // points to the new one instead
    Attribute *a = xmldoc->attribute;
    for (size_t i=0; i<a->fields.size(); i++)
    {
        Field *f = a->fields[i];
        if (f->type == "enum" && f->GetSubtype() == e->type)
        {
            Field *n = new Int(f->name, f->label);
            n->CopyValues(f);
            a->fields[i] = n;
            delete f;
        }
    }

    delete e;

    UpdateWindowContents();

    if (index >= enumlist->count())
        index = enumlist->count()-1;
    enumlist->setCurrentRow(index);
}
