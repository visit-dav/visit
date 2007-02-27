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

#include "XMLEditIncludes.h"

#include <XMLDocument.h>
#include <Include.h>
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
//  Constructor:  XMLEditIncludes::XMLEditIncludes
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
XMLEditIncludes::XMLEditIncludes(QWidget *p, const QString &n)
    : QFrame(p, n)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);

    QGridLayout *listLayout = new QGridLayout(hLayout, 2,2, 5);

    includelist = new QListBox(this);
    listLayout->addMultiCellWidget(includelist, 0,0, 0,1);

    newButton = new QPushButton("New", this);
    listLayout->addWidget(newButton, 1,0);

    delButton = new QPushButton("Del", this);
    listLayout->addWidget(delButton, 1,1);

    hLayout->addSpacing(10);

    QGridLayout *topLayout = new QGridLayout(hLayout, 4,2, 5);
    topLayout->addColSpacing(1, 20);
    int row = 0;

    fileGroup = new QButtonGroup();
    CButton = new QRadioButton("Source (.C) file", this);
    HButton = new QRadioButton("Header (.h) file", this);
    fileGroup->insert(CButton);
    fileGroup->insert(HButton);
    topLayout->addWidget(CButton, row, 0);
    topLayout->addWidget(HButton, row, 1);
    row++;

    quotedGroup = new QButtonGroup();
    quotesButton = new QRadioButton("Use quotes \"\"", this);
    bracketsButton = new QRadioButton("Use angle brackets <>", this);
    quotedGroup->insert(quotesButton);
    quotedGroup->insert(bracketsButton);
    topLayout->addWidget(quotesButton, row, 0);
    topLayout->addWidget(bracketsButton, row, 1);
    row++;

    topLayout->addWidget(new QLabel("Include file", this), row, 0);
    file = new QLineEdit(this);
    topLayout->addWidget(file, row, 1);
    row++;

    topLayout->setRowStretch(row, 100);
    row++;

    connect(includelist, SIGNAL(selectionChanged()),
            this, SLOT(UpdateWindowSingleItem()));

    connect(includelist, SIGNAL(selectionChanged()),
            this, SLOT(UpdateWindowSingleItem()));
    connect(fileGroup, SIGNAL(clicked(int)),
            this, SLOT(fileGroupChanged(int)));
    connect(quotedGroup, SIGNAL(clicked(int)),
            this, SLOT(quotedGroupChanged(int)));
    connect(file, SIGNAL(textChanged(const QString&)),
            this, SLOT(includeTextChanged(const QString&)));
    connect(newButton, SIGNAL(clicked()),
            this, SLOT(includelistNew()));
    connect(delButton, SIGNAL(clicked()),
            this, SLOT(includelistDel()));
}

// ****************************************************************************
//  Method:  XMLEditIncludes::UpdateWindowContents
//
//  Purpose:
//    Update the window based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditIncludes::UpdateWindowContents()
{
    BlockAllSignals(true);
    Attribute *a = xmldoc->attribute;
    includelist->clear();
    for (int i=0; i<a->includes.size(); i++)
    {
        includelist->insertItem(a->includes[i]->include);
    }

    BlockAllSignals(false);
    UpdateWindowSingleItem();
}

// ****************************************************************************
//  Method:  XMLEditIncludes::UpdateWindowSensitivity
//
//  Purpose:
//    Enable/disable widget sensitivity based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditIncludes::UpdateWindowSensitivity()
{
    bool active = includelist->currentItem() != -1;

    delButton->setEnabled(includelist->count() > 0);
    file->setEnabled(active);
    CButton->setEnabled(active);
    HButton->setEnabled(active);
    quotesButton->setEnabled(active);
    bracketsButton->setEnabled(active);
}

// ****************************************************************************
//  Method:  XMLEditIncludes::UpdateWindowSingleItem
//
//  Purpose:
//    Update the window based on the state a single item in the list.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Tue May 20 12:11:43 PDT 2003
//    I fixed a bug with quoted vs angle brackets being backwards.
//
// ****************************************************************************

void
XMLEditIncludes::UpdateWindowSingleItem()
{
    BlockAllSignals(true);

    Attribute *a = xmldoc->attribute;
    int index = includelist->currentItem();

    if (index == -1)
    {
        file->setText("");
        fileGroup->setButton(-1);
        quotedGroup->setButton(-1);
    }
    else
    {
        Include *n = a->includes[index];
        file->setText(n->include);
        fileGroup->setButton((n->target == "source") ? 0 : 1);
        quotedGroup->setButton(n->quoted ? 0 : 1);
    }

    UpdateWindowSensitivity();
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditIncludes::BlockAllSignals
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
XMLEditIncludes::BlockAllSignals(bool block)
{
    includelist->blockSignals(block);
    file->blockSignals(block);
}

// ----------------------------------------------------------------------------
//                                 Callbacks
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Method:  XMLEditIncludes::includeTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditIncludes::includeTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = includelist->currentItem();
    if (index == -1)
        return;
    Include *n = a->includes[index];

    QString newinclude = text.stripWhiteSpace();
    n->include = newinclude;

    BlockAllSignals(true);
    includelist->changeItem(newinclude, index);
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditIncludes::fileGroupChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditIncludes::fileGroupChanged(int fg)
{
    Attribute *a = xmldoc->attribute;
    int index = includelist->currentItem();
    if (index == -1)
        return;
    Include *n = a->includes[index];

    n->target = (fg == 0) ? "source" : "header";
}

// ****************************************************************************
//  Method:  XMLEditIncludes::quotedGroupChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Tue May 20 12:09:36 PDT 2003
//    I fixed a bug that had quoted vs angle brackets backwards.
//
// ****************************************************************************

void
XMLEditIncludes::quotedGroupChanged(int qg)
{
    Attribute *a = xmldoc->attribute;
    int index = includelist->currentItem();
    if (index == -1)
        return;
    Include *n = a->includes[index];

    n->quoted = (qg == 0);
}

// ****************************************************************************
//  Method:  XMLEditIncludes::includelistNew
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditIncludes::includelistNew()
{
    Attribute *a = xmldoc->attribute;
    int newid = 1;
    bool okay = false;
    QString newname;
    while (!okay)
    {
        okay = true;
        newname.sprintf("unnamed%d", newid);
        for (int i=0; i<includelist->count() && okay; i++)
        {
            if (includelist->text(i) == newname)
                okay = false;
        }
        if (!okay)
            newid++;
    }
    
    Include *n = new Include("header",false);
    n->include = newname;
    
    a->includes.push_back(n);
    UpdateWindowContents();
    for (int i=0; i<includelist->count(); i++)
    {
        if (includelist->text(i) == newname)
        {
            includelist->setCurrentItem(i);
            UpdateWindowSingleItem();
        }
    }
}

// ****************************************************************************
//  Method:  XMLEditIncludes::includelistDel
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditIncludes::includelistDel()
{
    Attribute *a = xmldoc->attribute;
    int index = includelist->currentItem();

    if (index == -1)
        return;

    Include *n = a->includes[index];
    vector<Include*> newlist;
    for (int i=0; i<a->includes.size(); i++)
    {
        if (a->includes[i] != n)
            newlist.push_back(a->includes[i]);
    }
    a->includes = newlist;

    delete n;

    UpdateWindowContents();

    if (index >= includelist->count())
        index = includelist->count()-1;
    includelist->setCurrentItem(index);
}
