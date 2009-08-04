/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#include "XMLEditFunctions.h"

#include <XMLDocument.h>
#include <Function.h>
#include <Attribute.h>

#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>
#include <QButtonGroup>
#include <QCheckBox>

// ****************************************************************************
//  Constructor:  XMLEditFunctions::XMLEditFunctions
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 15:42:34 PST 2008
//    Added target.
//
//    Brad Whitlock, Mon Apr 28 15:59:56 PDT 2008
//    Added access, tr()'s.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
XMLEditFunctions::XMLEditFunctions(QWidget *p)
    : QFrame(p)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);

    QGridLayout *listLayout = new QGridLayout();

    functionlist = new QListWidget(this);
    listLayout->addWidget(functionlist, 0,0, 1,2);

    newButton = new QPushButton(tr("New"), this);
    listLayout->addWidget(newButton, 1,0);

    delButton = new QPushButton(tr("Del"), this);
    listLayout->addWidget(delButton, 1,1);
    hLayout->addLayout(listLayout);
    hLayout->addSpacing(10);

    QGridLayout *topLayout = new QGridLayout();
    int row = 0;

    topLayout->addWidget(new QLabel(tr("Target"), this), row, 0);
    target = new QLineEdit(this);
    topLayout->addWidget(target, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("Name"), this), row, 0);
    name = new QLineEdit(this);
    topLayout->addWidget(name, row, 1);
    row++;

    typeGroup = new QButtonGroup(this);
    
    newFunctionButton    = new QRadioButton(tr("New function"), this);
    replaceBuiltinButton = new QRadioButton(tr("Replaces builtin"), this);
    newFunctionButton->setChecked(true);
    
    typeGroup->addButton(newFunctionButton,0);
    typeGroup->addButton(replaceBuiltinButton,1);
    
    topLayout->addWidget(newFunctionButton,    row, 0);
    topLayout->addWidget(replaceBuiltinButton, row, 1);
    row++;

    member = new QCheckBox(tr("Class member"), this);
    topLayout->addWidget(member, row,0,1,2);
    row++;

    accessLabel = new QLabel(tr("Access"), this);
    access = new QComboBox(this);
    access->addItem(tr("private"));
    access->addItem(tr("protected"));
    access->addItem(tr("public"));
    topLayout->addWidget(accessLabel, row, 0);
    topLayout->addWidget(access,      row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("Declaration"), this), row, 0);
    declaration = new QLineEdit(this);
    topLayout->addWidget(declaration, row, 1);
    row++;

    topLayout->addWidget(new QLabel("Definition", this), row, 0);
    row++;

    definition = new QTextEdit(this);
    QFont monospaced("Courier");
    definition->setFont(monospaced);
    definition->setWordWrapMode(QTextOption::NoWrap);
    topLayout->addWidget(definition, row,0, 1,2);
    row++;

    topLayout->setRowMinimumHeight(row, 20);
    row++;
    
    hLayout->addLayout(topLayout);
    
    connect(functionlist, SIGNAL(currentRowChanged(int)),
            this, SLOT(UpdateWindowSingleItem()));
    connect(name, SIGNAL(textChanged(const QString&)),
            this, SLOT(nameTextChanged(const QString&)));
    connect(typeGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(typeGroupChanged(int)));
    connect(member, SIGNAL(clicked()),
            this, SLOT(memberChanged()));
    connect(target, SIGNAL(textChanged(const QString&)),
            this, SLOT(targetTextChanged(const QString&)));
    connect(declaration, SIGNAL(textChanged(const QString&)),
            this, SLOT(declarationTextChanged(const QString&)));
    connect(definition, SIGNAL(textChanged()),
            this, SLOT(definitionChanged()));
    connect(newButton, SIGNAL(clicked()),
            this, SLOT(functionlistNew()));
    connect(delButton, SIGNAL(clicked()),
            this, SLOT(functionlistDel()));
    connect(access, SIGNAL(activated(int)),
            this, SLOT(accessChanged(int)));
}

// ****************************************************************************
// Method: XMLEditFunctions::CountFunctions
//
// Purpose: 
//   Return the number of functions having a given name.
//
// Arguments:
//  name : The name of the function that we're interested in.
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
XMLEditFunctions::CountFunctions(const QString &name) const
{
    Attribute *a = xmldoc->attribute;
    int funcCount = 0;
    for (size_t j=0; j<a->functions.size(); j++)
        funcCount += (name == a->functions[j]->name) ? 1 : 0;
    return funcCount;
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
//  Modifications:
//    Brad Whitlock, Thu Mar 6 15:49:31 PST 2008
//    Added support for multiple targets.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFunctions::UpdateWindowContents()
{
    BlockAllSignals(true);
    Attribute *a = xmldoc->attribute;

    functionlist->clear();
    for (size_t i=0; i<a->functions.size(); i++)
    {
        if(CountFunctions(a->functions[i]->name) > 1)
        {
            QString id = QString("%1 [%2]").arg(a->functions[i]->name).arg(a->functions[i]->target);
            functionlist->addItem(id);
        }
        else
        {
            functionlist->addItem(a->functions[i]->name);
        }
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
//  Brad Whitlock, Thu Mar 6 15:50:09 PST 2008
//  Added target.
//
//  Brad Whitlock, Mon Apr 28 16:07:57 PDT 2008
//  Added access.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFunctions::UpdateWindowSensitivity()
{
    bool active = functionlist->currentRow() != -1;

    delButton->setEnabled(functionlist->count() > 0);
    name->setEnabled(active);
    target->setEnabled(active);
    declaration->setEnabled(active);
    definition->setEnabled(active);
    newFunctionButton->setEnabled(active);
    replaceBuiltinButton->setEnabled(active);
    member->setEnabled(active);

    bool accessActive = active;
    int index = functionlist->currentRow();
    if (index != -1)
    {
        Attribute *a = xmldoc->attribute;
        Function *f = a->functions[index];
        accessActive &= f->user;
    }
    access->setEnabled(accessActive);
    accessLabel->setEnabled(accessActive);
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
//  Modifications:
//    Brad Whitlock, Thu Mar 6 15:50:51 PST 2008
//    Added target.
//
//    Brad Whitlock, Mon Apr 28 16:08:06 PDT 2008
//    Added access.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFunctions::UpdateWindowSingleItem()
{
    BlockAllSignals(true);

    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentRow();

    if (index == -1)
    {
        name->setText("");
        typeGroup->button(0)->setChecked(false);
        member->setChecked(false);
        target->setText("");
        declaration->setText("");
        definition->setText("");
        access->setCurrentIndex(0);
    }
    else
    {
        Function *f = a->functions[index];
        name->setText(f->name);
        typeGroup->button(f->user ? 0 : 1)->setChecked(true);
        target->setText(f->target);
        declaration->setText(f->decl);
        definition->setText(f->def);
        member->setChecked(f->member);
        access->setCurrentIndex((int)f->accessType);
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
//  Modifications:
//    Brad Whitlock, Thu Mar 6 15:50:51 PST 2008
//    Added target.
//
//    Brad Whitlock, Mon Apr 28 16:09:11 PDT 2008
//    Added access.
//
// ****************************************************************************
void
XMLEditFunctions::BlockAllSignals(bool block)
{
    functionlist->blockSignals(block);
    name->blockSignals(block);
    member->blockSignals(block);
    target->blockSignals(block);
    declaration->blockSignals(block);
    definition->blockSignals(block);
    access->blockSignals(block);
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
//  Modifications:
//    Brad Whitlock, Thu Mar 6 15:55:17 PST 2008
//    Added support for multiple targets.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFunctions::nameTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentRow();
    if (index == -1)
        return;
    Function *f = a->functions[index];

    QString newname = text.trimmed();
    f->name = newname;
    if(CountFunctions(newname) > 1)
    {
        newname += "[";
        newname += f->target;
        newname += "]";
    }
    BlockAllSignals(true);
    functionlist->item(index)->setText(newname);
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditFunctions::typeGroupChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFunctions::typeGroupChanged(int tg)
{
    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentRow();
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
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFunctions::memberChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentRow();
    if (index == -1)
        return;
    Function *f = a->functions[index];

    f->member = member->isChecked();
}

// ****************************************************************************
//  Method:  XMLEditFunctions::targetTextChanged
//
//  Programmer:  Brad Whitlock
//  Creation:    Thu Mar 6 15:56:05 PST 2008
//
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFunctions::targetTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentRow();
    if (index == -1)
        return;
    Function *f = a->functions[index];

    f->target = text;
    nameTextChanged(f->name);
}

// ****************************************************************************
//  Method:  XMLEditFunctions::declarationTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFunctions::declarationTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentRow();
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
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFunctions::definitionChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentRow();
    if (index == -1)
        return;
    Function *f = a->functions[index];

    f->def = definition->toPlainText();
}

// ****************************************************************************
//  Method:  XMLEditFunctions::functionlistNew
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 15:57:07 PST 2008
//    Added default target of xml2atts.
//
//    Brad Whitlock, Mon Apr 28 16:11:51 PDT 2008
//    Added tr().
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
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
        newname = tr("unnamed%1").arg(newid);
        for (size_t i=0; i<functionlist->count() && okay; i++)
        {
            if (functionlist->item(i)->text() == newname)
                okay = false;
        }
        if (!okay)
            newid++;
    }
    
    Function *f = new Function(newname,"","",true,true,"xml2atts");
    
    a->functions.push_back(f);
    UpdateWindowContents();
    for (size_t i=0; i<functionlist->count(); i++)
    {
        if (functionlist->item(i)->text() == newname)
        {
            functionlist->setCurrentRow(i);
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
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFunctions::functionlistDel()
{
    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentRow();

    if (index == -1)
        return;

    Function *f = a->functions[index];
    vector<Function*> newlist;
    for (size_t i=0; i<a->functions.size(); i++)
    {
        if (a->functions[i] != f)
            newlist.push_back(a->functions[i]);
    }
    a->functions = newlist;

    delete f;

    UpdateWindowContents();

    if (index >= functionlist->count())
        index = functionlist->count()-1;
    functionlist->setCurrentRow(index);
}

// ****************************************************************************
//  Method:  XMLEditFunctions::accessChanged
//
//  Programmer:  ?
//  Creation:    ?
//
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFunctions::accessChanged(int val)
{
    Attribute *a = xmldoc->attribute;
    int index = functionlist->currentRow();
    if (index == -1)
        return;
    Function *f = a->functions[index];

    if(val == 0)
        f->accessType = Function::AccessPublic;
    else if(val == 1)
        f->accessType = Function::AccessProtected;
    else
        f->accessType = Function::AccessPrivate;
}
