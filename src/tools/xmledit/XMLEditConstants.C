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

#include "XMLEditConstants.h"

#include <XMLDocument.h>
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
//  Constructor:  XMLEditConstants::XMLEditConstants
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:11:47 PST 2008
//    Added targets.
//
// ****************************************************************************
XMLEditConstants::XMLEditConstants(QWidget *p, const QString &n)
    : QFrame(p, n)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);

    QGridLayout *listLayout = new QGridLayout(hLayout, 2,2, 5);

    constantlist = new QListBox(this);
    listLayout->addMultiCellWidget(constantlist, 0,0, 0,1);

    newButton = new QPushButton("New", this);
    listLayout->addWidget(newButton, 1,0);

    delButton = new QPushButton("Del", this);
    listLayout->addWidget(delButton, 1,1);

    hLayout->addSpacing(10);

    QGridLayout *topLayout = new QGridLayout(hLayout, 6,2, 5);
    int row = 0;

    topLayout->addWidget(new QLabel("Target", this), row, 0);
    target = new QLineEdit(this);
    topLayout->addWidget(target, row, 1);
    row++;

    topLayout->addWidget(new QLabel("Name", this), row, 0);
    name = new QLineEdit(this);
    topLayout->addWidget(name, row, 1);
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
    definition->setWordWrap(QTextEdit::NoWrap);
    topLayout->addMultiCellWidget(definition, row,row, 0,1);
    row++;

    topLayout->addRowSpacing(row, 20);
    row++;

    connect(constantlist, SIGNAL(selectionChanged()),
            this, SLOT(UpdateWindowSingleItem()));
    connect(target, SIGNAL(textChanged(const QString&)),
            this, SLOT(targetTextChanged(const QString&)));
    connect(name, SIGNAL(textChanged(const QString&)),
            this, SLOT(nameTextChanged(const QString&)));
    connect(member, SIGNAL(clicked()),
            this, SLOT(memberChanged()));
    connect(declaration, SIGNAL(textChanged(const QString&)),
            this, SLOT(declarationTextChanged(const QString&)));
    connect(definition, SIGNAL(textChanged()),
            this, SLOT(definitionChanged()));
    connect(newButton, SIGNAL(clicked()),
            this, SLOT(constantlistNew()));
    connect(delButton, SIGNAL(clicked()),
            this, SLOT(constantlistDel()));
}

// ****************************************************************************
// Method: XMLEditConstants::CountConstants
//
// Purpose: 
//   Return the number of constants having a given name.
//
// Arguments:
//  name : The name of the constants that we're interested in.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 6 15:53:04 PST 2008
//
// Modifications:
//   
// ****************************************************************************

int
XMLEditConstants::CountConstants(const QString &name) const
{
    Attribute *a = xmldoc->attribute;
    int constantCount = 0;
    for (int j=0; j<a->constants.size(); j++)
        constantCount += (name == a->constants[j]->name) ? 1 : 0;
    return constantCount;
}

// ****************************************************************************
//  Method:  XMLEditConstants::UpdateWindowContents
//
//  Purpose:
//    Update the window based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:12:45 PST 2008
//    Added targets.
//
// ****************************************************************************
void
XMLEditConstants::UpdateWindowContents()
{
    BlockAllSignals(true);

    Attribute *a = xmldoc->attribute;
    constantlist->clear();
    for (int i=0; i<a->constants.size(); i++)
    {
        if(CountConstants(a->constants[i]->name) > 1)
        { 
           QString id; id.sprintf("%s [%s]", a->constants[i]->name.latin1(),
               a->constants[i]->target.latin1());
            constantlist->insertItem(id);
        }
        else
            constantlist->insertItem(a->constants[i]->name);
    }

    BlockAllSignals(false);
    UpdateWindowSingleItem();
}

// ****************************************************************************
//  Method:  XMLEditConstants::UpdateWindowSensitivity
//
//  Purpose:
//    Enable/disable widget sensitivity based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:12:45 PST 2008
//    Added targets.
//
// ****************************************************************************
void
XMLEditConstants::UpdateWindowSensitivity()
{
    bool active = constantlist->currentItem() != -1;

    delButton->setEnabled(constantlist->count() > 0);
    target->setEnabled(active);
    name->setEnabled(active);
    declaration->setEnabled(active);
    definition->setEnabled(active);
    member->setEnabled(active);
}

// ****************************************************************************
//  Method:  XMLEditConstants::UpdateWindowSingleItem
//
//  Purpose:
//    Update the window based on the state a single item in the list.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:12:45 PST 2008
//    Added targets.
//
// ****************************************************************************
void
XMLEditConstants::UpdateWindowSingleItem()
{
    BlockAllSignals(true);

    Attribute *a = xmldoc->attribute;
    int index = constantlist->currentItem();

    if (index == -1)
    {
        target->setText("");
        name->setText("");
        member->setChecked(false);
        declaration->setText("");
        definition->setText("");
    }
    else
    {
        Constant *c = a->constants[index];
        target->setText(c->target);
        name->setText(c->name);
        declaration->setText(c->decl);
        definition->setText(c->def);
        member->setChecked(c->member);
    }

    UpdateWindowSensitivity();
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditConstants::BlockAllSignals
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
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:12:45 PST 2008
//    Added targets.
//
// ****************************************************************************
void
XMLEditConstants::BlockAllSignals(bool block)
{
    constantlist->blockSignals(block);
    target->blockSignals(block);
    name->blockSignals(block);
    member->blockSignals(block);
    declaration->blockSignals(block);
    definition->blockSignals(block);
}

// ----------------------------------------------------------------------------
//                                 Callbacks
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Method:  XMLEditConstants::nameTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:12:45 PST 2008
//    Added targets.
//
// ****************************************************************************
void
XMLEditConstants::nameTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = constantlist->currentItem();
    if (index == -1)
        return;
    Constant *c = a->constants[index];

    QString newname = text.stripWhiteSpace();
    c->name = newname;
    if(CountConstants(newname) > 1)
    {
        newname += "[";
        newname += c->target;
        newname += "]";
    }
    BlockAllSignals(true);
    constantlist->changeItem(text, index);
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditConstants::targetTextChanged
//
//  Programmer:  Brad Whitlock
//  Creation:    Thu Mar 6 15:56:05 PST 2008
//
// ****************************************************************************
void
XMLEditConstants::targetTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = constantlist->currentItem();
    if (index == -1)
        return;
    Constant *c = a->constants[index];

    c->target = text;
    nameTextChanged(c->name);
}

// ****************************************************************************
//  Method:  XMLEditConstants::memberChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditConstants::memberChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = constantlist->currentItem();
    if (index == -1)
        return;
    Constant *c = a->constants[index];

    c->member = member->isChecked();
}

// ****************************************************************************
//  Method:  XMLEditConstants::declarationTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditConstants::declarationTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = constantlist->currentItem();
    if (index == -1)
        return;
    Constant *c = a->constants[index];

    c->decl = text;
}

// ****************************************************************************
//  Method:  XMLEditConstants::definitionChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditConstants::definitionChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = constantlist->currentItem();
    if (index == -1)
        return;
    Constant *c = a->constants[index];

    c->def = definition->text();
}

// ****************************************************************************
//  Method:  XMLEditConstants::constantlistNew
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:17:13 PST 2008
//    Added xml2atts target.
//
// ****************************************************************************
void
XMLEditConstants::constantlistNew()
{
    Attribute *a = xmldoc->attribute;
    int newid = 1;
    bool okay = false;
    QString newname;
    while (!okay)
    {
        okay = true;
        newname.sprintf("unnamed%d", newid);
        for (int i=0; i<constantlist->count() && okay; i++)
        {
            if (constantlist->text(i) == newname)
                okay = false;
        }
        if (!okay)
            newid++;
    }
    
    Constant *c = new Constant(newname,"","",false, "xml2atts");
    
    a->constants.push_back(c);
    UpdateWindowContents();
    for (int i=0; i<constantlist->count(); i++)
    {
        if (constantlist->text(i) == newname)
        {
            constantlist->setCurrentItem(i);
            UpdateWindowSingleItem();
        }
    }
}

// ****************************************************************************
//  Method:  XMLEditConstants::constantlistDel
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditConstants::constantlistDel()
{
    Attribute *a = xmldoc->attribute;
    int index = constantlist->currentItem();

    if (index == -1)
        return;

    Constant *c = a->constants[index];
    vector<Constant*> newlist;
    for (int i=0; i<a->constants.size(); i++)
    {
        if (a->constants[i] != c)
            newlist.push_back(a->constants[i]);
    }
    a->constants = newlist;

    delete c;

    UpdateWindowContents();

    if (index >= constantlist->count())
        index = constantlist->count()-1;
    constantlist->setCurrentItem(index);
}
