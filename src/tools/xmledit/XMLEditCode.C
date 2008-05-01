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

#include "XMLEditCode.h"

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
//  Constructor:  XMLEditCode::XMLEditCode
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modificiations:
//    Brad Whitlock, Thu Mar 6 16:00:56 PST 2008
//    Added support for target.
//
// ****************************************************************************
XMLEditCode::XMLEditCode(QWidget *p, const QString &n)
    : QFrame(p, n)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);

    QGridLayout *listLayout = new QGridLayout(hLayout, 2,2, 5);

    codelist = new QListBox(this);
    listLayout->addMultiCellWidget(codelist, 0,0, 0,1);

    newButton = new QPushButton(tr("New"), this);
    listLayout->addWidget(newButton, 1,0);

    delButton = new QPushButton(tr("Del"), this);
    listLayout->addWidget(delButton, 1,1);

    hLayout->addSpacing(10);

    QGridLayout *topLayout = new QGridLayout(hLayout, 6,2, 5);
    int row = 0;

    topLayout->addWidget(new QLabel(tr("Target"), this), row, 0);
    target = new QLineEdit(this);
    topLayout->addWidget(target, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("Name"), this), row, 0);
    name = new QLineEdit(this);
    topLayout->addWidget(name, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("Prefix"), this), row, 0);
    row++;

    QFont monospaced("Courier");

    prefix = new QMultiLineEdit(this);
    prefix->setFont(monospaced);
    prefix->setWordWrap(QTextEdit::NoWrap);
    topLayout->addMultiCellWidget(prefix, row,row, 0,1);
    row++;

    topLayout->addWidget(new QLabel(tr("Postfix"), this), row, 0);
    row++;

    postfix = new QMultiLineEdit(this);
    postfix->setFont(monospaced);
    postfix->setWordWrap(QTextEdit::NoWrap);
    topLayout->addMultiCellWidget(postfix, row,row, 0,1);
    row++;

    topLayout->addRowSpacing(row, 20);
    row++;

    connect(codelist, SIGNAL(selectionChanged()),
            this, SLOT(UpdateWindowSingleItem()));
    connect(target, SIGNAL(textChanged(const QString&)),
            this, SLOT(targetTextChanged(const QString&)));
    connect(name, SIGNAL(textChanged(const QString&)),
            this, SLOT(nameTextChanged(const QString&)));
    connect(prefix, SIGNAL(textChanged()),
            this, SLOT(prefixChanged()));
    connect(postfix, SIGNAL(textChanged()),
            this, SLOT(postfixChanged()));
    connect(newButton, SIGNAL(clicked()),
            this, SLOT(codelistNew()));
    connect(delButton, SIGNAL(clicked()),
            this, SLOT(codelistDel()));
}

// ****************************************************************************
// Method: XMLEditCode::CountCodes
//
// Purpose: 
//   Return the number of codes having a given name.
//
// Arguments:
//  name : The name of the code that we're interested in.
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
XMLEditCode::CountCodes(const QString &name) const
{
    Attribute *a = xmldoc->attribute;
    int codeCount = 0;
    for (size_t j=0; j<a->codes.size(); j++)
        codeCount += (name == a->codes[j]->name) ? 1 : 0;
    return codeCount;
}

// ****************************************************************************
//  Method:  XMLEditCode::UpdateWindowContents
//
//  Purpose:
//    Update the window based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:01:50 PST 2008
//    Added target.
//
// ****************************************************************************
void
XMLEditCode::UpdateWindowContents()
{
    BlockAllSignals(true);

    Attribute *a = xmldoc->attribute;
    codelist->clear();
    for (size_t i=0; i<a->codes.size(); i++)
    {
        if(CountCodes(a->codes[i]->name) > 1)
        { 
           QString id; id.sprintf("%s [%s]", a->codes[i]->name.latin1(),
               a->codes[i]->target.latin1());
            codelist->insertItem(id);
        }
        else
            codelist->insertItem(a->codes[i]->name);
    }

    BlockAllSignals(false);
    UpdateWindowSingleItem();
}

// ****************************************************************************
//  Method:  XMLEditCode::UpdateWindowSensitivity
//
//  Purpose:
//    Enable/disable widget sensitivity based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:04:06 PST 2008
//    Added target.
//
// ****************************************************************************
void
XMLEditCode::UpdateWindowSensitivity()
{
    bool active = codelist->currentItem() != -1;

    delButton->setEnabled(codelist->count() > 0);
    target->setEnabled(active);
    name->setEnabled(active);
    prefix->setEnabled(active);
    postfix->setEnabled(active);
}

// ****************************************************************************
//  Method:  XMLEditCode::UpdateWindowSingleItem
//
//  Purpose:
//    Update the window based on the state a single item in the list.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:05:54 PST 2008
//    Added target.
//
// ****************************************************************************
void
XMLEditCode::UpdateWindowSingleItem()
{
    BlockAllSignals(true);

    Attribute *a = xmldoc->attribute;
    int index = codelist->currentItem();

    if (index == -1)
    {
        target->setText("");
        name->setText("");
        prefix->setText("");
        postfix->setText("");
    }
    else
    {
        Code *c = a->codes[index];
        target->setText(c->target);
        name->setText(c->name);
        prefix->setText(c->prefix);
        postfix->setText(c->postfix);
    }

    UpdateWindowSensitivity();
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditCode::BlockAllSignals
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
//    Brad Whitlock, Thu Mar 6 16:05:54 PST 2008
//    Added target.
//
// ****************************************************************************
void
XMLEditCode::BlockAllSignals(bool block)
{
    codelist->blockSignals(block);
    target->blockSignals(block);
    name->blockSignals(block);
    prefix->blockSignals(block);
    postfix->blockSignals(block);
}

// ----------------------------------------------------------------------------
//                                 Callbacks
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Method:  XMLEditCode::nameTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditCode::nameTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = codelist->currentItem();
    if (index == -1)
        return;
    Code *c = a->codes[index];

    QString newname = text.stripWhiteSpace();
    c->name = newname;
    if(CountCodes(newname) > 1)
    {
        newname += "[";
        newname += c->target;
        newname += "]";
    }
    BlockAllSignals(true);
    codelist->changeItem(text, index);
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditCode::targetTextChanged
//
//  Programmer:  Brad Whitlock
//  Creation:    Thu Mar 6 15:56:05 PST 2008
//
// ****************************************************************************
void
XMLEditCode::targetTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = codelist->currentItem();
    if (index == -1)
        return;
    Code *c = a->codes[index];

    c->target = text;
    nameTextChanged(c->name);
}

// ****************************************************************************
//  Method:  XMLEditCode::prefixChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditCode::prefixChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = codelist->currentItem();
    if (index == -1)
        return;
    Code *c = a->codes[index];

    c->prefix = prefix->text();
}

// ****************************************************************************
//  Method:  XMLEditCode::postfixChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditCode::postfixChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = codelist->currentItem();
    if (index == -1)
        return;
    Code *c = a->codes[index];

    c->postfix = postfix->text();
}

// ****************************************************************************
//  Method:  XMLEditCode::codelistNew
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:07:22 PST 2008
//    Added default target of xml2atts.
//
// ****************************************************************************
void
XMLEditCode::codelistNew()
{
    Attribute *a = xmldoc->attribute;
    int newid = 1;
    bool okay = false;
    QString newname;
    while (!okay)
    {
        okay = true;
        newname = tr("unnamed%1").arg(newid);
        for (size_t i=0; i<codelist->count() && okay; i++)
        {
            if (codelist->text(i) == newname)
                okay = false;
        }
        if (!okay)
            newid++;
    }
    
    Code *c = new Code(newname,"","", "xml2atts");
    
    a->codes.push_back(c);
    UpdateWindowContents();
    for (size_t i=0; i<codelist->count(); i++)
    {
        if (codelist->text(i) == newname)
        {
            codelist->setCurrentItem(i);
            UpdateWindowSingleItem();
        }
    }
}

// ****************************************************************************
//  Method:  XMLEditCode::codelistDel
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditCode::codelistDel()
{
    Attribute *a = xmldoc->attribute;
    int index = codelist->currentItem();

    if (index == -1)
        return;

    Code *c = a->codes[index];
    vector<Code*> newlist;
    for (size_t i=0; i<a->codes.size(); i++)
    {
        if (a->codes[i] != c)
            newlist.push_back(a->codes[i]);
    }
    a->codes = newlist;

    delete c;

    UpdateWindowContents();

    if (index >= codelist->count())
        index = codelist->count()-1;
    codelist->setCurrentItem(index);
}
