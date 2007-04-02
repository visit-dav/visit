/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#include "XMLEditFunctions.h"

#include <XMLDocument.h>
#include <Function.h>
#include <Attribute.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>

// ****************************************************************************
//  Constructor:  XMLEditFunctions::XMLEditFunctions
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
XMLEditFunctions::XMLEditFunctions(QWidget *p, const QString &n)
    : QFrame(p, n)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);

    QGridLayout *listLayout = new QGridLayout(hLayout, 2,2, 5);

    functionlist = new QListBox(this);
    listLayout->addMultiCellWidget(functionlist, 0,0, 0,1);

    newButton = new QPushButton("New", this);
    listLayout->addWidget(newButton, 1,0);

    delButton = new QPushButton("Del", this);
    listLayout->addWidget(delButton, 1,1);

    hLayout->addSpacing(10);

    QGridLayout *topLayout = new QGridLayout(hLayout, 7,2, 5);
    int row = 0;

    topLayout->addWidget(new QLabel("Name", this), row, 0);
    name = new QLineEdit(this);
    topLayout->addWidget(name, row, 1);
    row++;

    typeGroup = new QButtonGroup();
    newFunctionButton    = new QRadioButton("New function", this);
    replaceBuiltinButton = new QRadioButton("Replaces builtin", this);
    typeGroup->insert(newFunctionButton);
    typeGroup->insert(replaceBuiltinButton);
    topLayout->addWidget(newFunctionButton,    row, 0);
    topLayout->addWidget(replaceBuiltinButton, row, 1);
    row++;

    member = new QCheckBox("Class member", this);
    topLayout->addMultiCellWidget(member, row,row, 0,1);
    row++;

    topLayout->addWidget(new QLabel("Declaration", this), row, 0);
    declaration = new QLineEdit(this);
    topLayout->addWidget(declaration, row, 1);
    row++;

    topLayout->addWidget(new QLabel("Definition", this), row, 0);
    row++;

    definition = new QMultiLineEdit(this);
    QFont monospaced("Courier");
    definition->setFont(monospaced);
    topLayout->addMultiCellWidget(definition, row,row, 0,1);
    row++;

    topLayout->addRowSpacing(row, 20);
    row++;

    connect(functionlist, SIGNAL(selectionChanged()),
            this, SLOT(UpdateWindowSingleItem()));

    connect(functionlist, SIGNAL(selectionChanged()),
            this, SLOT(UpdateWindowSingleItem()));
    connect(name, SIGNAL(textChanged(const QString&)),
            this, SLOT(nameTextChanged(const QString&)));
    connect(typeGroup, SIGNAL(clicked(int)),
            this, SLOT(typeGroupChanged(int)));
    connect(member, SIGNAL(clicked()),
            this, SLOT(memberChanged()));
    connect(declaration, SIGNAL(textChanged(const QString&)),
            this, SLOT(declarationTextChanged(const QString&)));
    connect(definition, SIGNAL(textChanged()),
            this, SLOT(definitionChanged()));
    connect(newButton, SIGNAL(clicked()),
            this, SLOT(functionlistNew()));
    connect(delButton, SIGNAL(clicked()),
            this, SLOT(functionlistDel()));
}

// ****************************************************************************
//  Method:  XMLEditFunctions::UpdateWindowContents
//
//  Purpose:
//    Update the window based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFunctions::UpdateWindowContents()
{
    BlockAllSignals(true);
    Attribute *a = xmldoc->attribute;
    functionlist->clear();
    for (int i=0; i<a->functions.size(); i++)
    {
        functionlist->insertItem(a->functions[i]->name);
    }

    BlockAllSignals(false);
    UpdateWindowSingleItem();
}

// ****************************************************************************
//  Method:  XMLEditFunctions::UpdateWindowSensitivity
//
//  Purpose:
//    Enable/disable widget sensitivity based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFunctions::UpdateWindowSensitivity()
{
    bool active = functionlist->currentItem() != -1;

    delButton->setEnabled(functionlist->count() > 0);
    name->setEnabled(active);
    declaration->setEnabled(active);
    definition->setEnabled(active);
    newFunctionButton->setEnabled(active);
    replaceBuiltinButton->setEnabled(active);
    member->setEnabled(active);
}

// ****************************************************************************
//  Method:  XMLEditFunctions::UpdateWindowSingleItem
//
//  Purpose:
//    Update the window based on the state a single item in the list.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFunctions::UpdateWindowSingleItem()
{
    BlockAllSignals(true);

    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentItem();

    if (index == -1)
    {
        name->setText("");
        typeGroup->setButton(-1);
        member->setChecked(false);
        declaration->setText("");
        definition->setText("");
    }
    else
    {
        Function *f = a->functions[index];
        name->setText(f->name);
        typeGroup->setButton(f->user ? 0 : 1);
        declaration->setText(f->decl);
        definition->setText(f->def);
        member->setChecked(f->member);
    }

    UpdateWindowSensitivity();
    BlockAllSignals(false);
}


// ****************************************************************************
//  Method:  XMLEditFunctions::BlockAllSignals
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
XMLEditFunctions::BlockAllSignals(bool block)
{
    functionlist->blockSignals(block);
    name->blockSignals(block);
    member->blockSignals(block);
    declaration->blockSignals(block);
    definition->blockSignals(block);
}

// ----------------------------------------------------------------------------
//                                 Callbacks
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Method:  XMLEditFunctions::nameTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFunctions::nameTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentItem();
    if (index == -1)
        return;
    Function *f = a->functions[index];

    QString newname = text.stripWhiteSpace();
    f->name = newname;

    BlockAllSignals(true);
    functionlist->changeItem(newname, index);
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditFunctions::typeGroupChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFunctions::typeGroupChanged(int tg)
{
    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentItem();
    if (index == -1)
        return;
    Function *f = a->functions[index];

    f->user = (tg == 0) ? true : false;
}

// ****************************************************************************
//  Method:  XMLEditFunctions::memberChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFunctions::memberChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentItem();
    if (index == -1)
        return;
    Function *f = a->functions[index];

    f->member = member->isChecked();
}

// ****************************************************************************
//  Method:  XMLEditFunctions::declarationTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFunctions::declarationTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentItem();
    if (index == -1)
        return;
    Function *f = a->functions[index];

    f->decl = text;
}

// ****************************************************************************
//  Method:  XMLEditFunctions::definitionChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFunctions::definitionChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentItem();
    if (index == -1)
        return;
    Function *f = a->functions[index];

    f->def = definition->text();
}

// ****************************************************************************
//  Method:  XMLEditFunctions::functionlistNew
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFunctions::functionlistNew()
{
    Attribute *a = xmldoc->attribute;
    int newid = 1;
    bool okay = false;
    QString newname;
    while (!okay)
    {
        okay = true;
        newname.sprintf("unnamed%d", newid);
        for (int i=0; i<functionlist->count() && okay; i++)
        {
            if (functionlist->text(i) == newname)
                okay = false;
        }
        if (!okay)
            newid++;
    }
    
    Function *f = new Function(newname,"","",true,true);
    
    a->functions.push_back(f);
    UpdateWindowContents();
    for (int i=0; i<functionlist->count(); i++)
    {
        if (functionlist->text(i) == newname)
        {
            functionlist->setCurrentItem(i);
            UpdateWindowSingleItem();
        }
    }
}

// ****************************************************************************
//  Method:  XMLEditFunctions::functionlistDel
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFunctions::functionlistDel()
{
    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentItem();

    if (index == -1)
        return;

    Function *f = a->functions[index];
    vector<Function*> newlist;
    for (int i=0; i<a->functions.size(); i++)
    {
        if (a->functions[i] != f)
            newlist.push_back(a->functions[i]);
    }
    a->functions = newlist;

    delete f;

    UpdateWindowContents();

    if (index >= functionlist->count())
        index = functionlist->count()-1;
    functionlist->setCurrentItem(index);
}
