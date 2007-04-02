/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "XMLEditEnums.h"

#include <XMLDocument.h>
#include <Attribute.h>
#include <Field.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <Enum.h>
#include <XMLParserUtil.h>

// ****************************************************************************
//  Constructor:  XMLEditEnums::XMLEditEnums
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
XMLEditEnums::XMLEditEnums(QWidget *p, const QString &n)
    : QFrame(p, n)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);

    QGridLayout *listLayout = new QGridLayout(hLayout, 2,2, 5);

    enumlist = new QListBox(this);
    listLayout->addMultiCellWidget(enumlist, 0,0, 0,1);

    newButton = new QPushButton("New", this);
    listLayout->addWidget(newButton, 1,0);

    delButton = new QPushButton("Del", this);
    listLayout->addWidget(delButton, 1,1);

    hLayout->addSpacing(10);

    QGridLayout *topLayout = new QGridLayout(hLayout, 4,2, 5);
    topLayout->addColSpacing(1, 20);
    int row = 0;

    name = new QLineEdit(this);
    topLayout->addWidget(new QLabel("Name", this), row, 0);
    topLayout->addWidget(name, row, 1);
    row++;

    topLayout->addWidget(new QLabel("Values", this), row, 0);
    row++;

    valuelist = new QMultiLineEdit(this);
    topLayout->addMultiCellWidget(valuelist, row,row, 0,1);
    row++;

    topLayout->addRowSpacing(row, 20);
    row++;

    // ------------------------------------------------------------

    connect(enumlist, SIGNAL(selectionChanged()),
            this, SLOT(UpdateWindowSingleItem()));
    connect(name, SIGNAL(textChanged(const QString&)),
            this, SLOT(nameTextChanged(const QString&)));
    connect(valuelist, SIGNAL(textChanged()),
            this, SLOT(valuelistChanged()));
    connect(valuelist, SIGNAL(returnPressed()),
            this, SLOT(addEmptyLine()));
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
// ****************************************************************************
void
XMLEditEnums::removeEmptyLines()
{
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
// ****************************************************************************
void
XMLEditEnums::addEmptyLine()
{
    int line, col;
    valuelist->getCursorPosition(&line, &col);
    if (line > 0 && valuelist->textLine(line-1).isEmpty())
    {
        valuelist->setCursorPosition(line-1,0);
    }
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
// ****************************************************************************
void
XMLEditEnums::UpdateWindowContents()
{
    BlockAllSignals(true);

    enumlist->clear();
    for (int i=0; i<EnumType::enums.size(); i++)
    {
        enumlist->insertItem(EnumType::enums[i]->type);
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
// ****************************************************************************
void
XMLEditEnums::UpdateWindowSensitivity()
{
    delButton->setEnabled(enumlist->count() > 0);

    if (enumlist->currentItem() != -1)
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
// ****************************************************************************
void
XMLEditEnums::UpdateWindowSingleItem()
{
    BlockAllSignals(true);
    int index = enumlist->currentItem();

    if (index == -1)
    {
        name->setText("");
        valuelist->setText("");
    }
    else
    {
        EnumType *e = EnumType::FindEnum(enumlist->currentText());
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
// ****************************************************************************
void
XMLEditEnums::nameTextChanged(const QString &text)
{
    int index = enumlist->currentItem();

    if (index == -1)
        return;

    EnumType *e = EnumType::FindEnum(enumlist->currentText());
    QString newname = text.stripWhiteSpace();
    e->type = newname;
    BlockAllSignals(true);
    enumlist->changeItem(newname, index);
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditEnums::valuelistChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditEnums::valuelistChanged()
{
    removeEmptyLines();

    int index = enumlist->currentItem();

    if (index == -1)
        return;

    EnumType *e = EnumType::FindEnum(enumlist->currentText());
    e->values = SplitValues(valuelist->text());
}

// ****************************************************************************
//  Method:  XMLEditEnums::enumlistNew
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
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
        newtype.sprintf("unnamed%d", newid);
        for (int i=0; i<enumlist->count() && okay; i++)
        {
            if (enumlist->text(i) == newtype)
                okay = false;
        }
        if (!okay)
            newid++;
    }
    
    EnumType *e = new EnumType(newtype);
    EnumType::enums.push_back(e);
    UpdateWindowContents();
    for (int i=0; i<enumlist->count(); i++)
    {
        if (enumlist->text(i) == newtype)
        {
            enumlist->setCurrentItem(i);
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
// ****************************************************************************
void
XMLEditEnums::enumlistDel()
{
    int index = enumlist->currentItem();

    if (index == -1)
        return;

    EnumType *e = EnumType::FindEnum(enumlist->currentText());
    vector<EnumType*> newlist;
    int i;
    for (i=0; i<EnumType::enums.size(); i++)
    {
        if (EnumType::enums[i] != e)
            newlist.push_back(EnumType::enums[i]);
    }
    EnumType::enums = newlist;

    // Make sure anyone with a reference to the old one
    // points to the new one instead
    Attribute *a = xmldoc->attribute;
    for (i=0; i<a->fields.size(); i++)
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
    enumlist->setCurrentItem(index);
}
