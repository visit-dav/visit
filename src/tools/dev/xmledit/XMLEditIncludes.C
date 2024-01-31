// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "XMLEditStd.h"
#include "XMLEditIncludes.h"

#include <XMLDocument.h>
#include <Include.h>
#include <Attribute.h>
#include <QLabel>
#include <QLayout>
#include <qlistwidget.h>
#include <QTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>
#include <QButtonGroup>
#include <QCheckBox>

#include <vector>

// ****************************************************************************
//  Constructor:  XMLEditIncludes::XMLEditIncludes
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:21:32 PST 2008
//    Added target.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
//    Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//    Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************
XMLEditIncludes::XMLEditIncludes(QWidget *p)
    : QFrame(p)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);

    QGridLayout *listLayout = new QGridLayout();

    includelist = new QListWidget(this);
    listLayout->addWidget(includelist, 0,0, 1,2);

    newButton = new QPushButton(tr("New"), this);
    listLayout->addWidget(newButton, 1,0);

    delButton = new QPushButton(tr("Del"), this);
    listLayout->addWidget(delButton, 1,1);

    hLayout->addLayout(listLayout);
    hLayout->addSpacing(10);

    QGridLayout *topLayout = new QGridLayout();

    topLayout->setColumnMinimumWidth(1, 20);
    int row = 0;

    fileGroup = new QButtonGroup(this);

    CButton = new QRadioButton(tr("Source (.C) file"), this);
    HButton = new QRadioButton(tr("Header (.h) file"), this);
    CButton->setChecked(true);
    fileGroup->addButton(CButton,0);
    fileGroup->addButton(HButton,1);

    topLayout->addWidget(CButton, row, 0);
    topLayout->addWidget(HButton, row, 1);
    row++;

    quotedGroup    = new QButtonGroup(this);
    quotesButton   = new QRadioButton(tr("Use quotes \"\""), this);
    bracketsButton = new QRadioButton(tr("Use angle brackets <>"), this);
    quotesButton->setChecked(true);
    quotedGroup->addButton(quotesButton,0);
    quotedGroup->addButton(bracketsButton,1);

    topLayout->addWidget(quotesButton, row, 0);
    topLayout->addWidget(bracketsButton, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("Target"), this), row, 0);
    target = new QLineEdit(this);
    topLayout->addWidget(target, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("Include file"), this), row, 0);
    file = new QLineEdit(this);
    topLayout->addWidget(file, row, 1);
    row++;

    topLayout->setRowStretch(row, 100);
    row++;

    hLayout->addLayout(topLayout);

    connect(includelist, SIGNAL(currentRowChanged(int)),
            this, SLOT(UpdateWindowSingleItem()));

    connect(includelist, SIGNAL(currentRowChanged(int)),
            this, SLOT(UpdateWindowSingleItem()));
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(fileGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(fileGroupChanged(int)));
    connect(quotedGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(quotedGroupChanged(int)));
#else
    connect(fileGroup, SIGNAL(idClicked(int)),
            this, SLOT(fileGroupChanged(int)));
    connect(quotedGroup, SIGNAL(idClicked(int)),
            this, SLOT(quotedGroupChanged(int)));
#endif
    connect(file, SIGNAL(textChanged(const QString&)),
            this, SLOT(includeTextChanged(const QString&)));
    connect(target, SIGNAL(textChanged(const QString&)),
            this, SLOT(targetTextChanged(const QString&)));
    connect(newButton, SIGNAL(clicked()),
            this, SLOT(includelistNew()));
    connect(delButton, SIGNAL(clicked()),
            this, SLOT(includelistDel()));
}

// ****************************************************************************
// Method: XMLEditIncludes::CountIncludes
//
// Purpose:
//   Return the number of include having a given name.
//
// Arguments:
//  name : The name of the include that we're interested in.
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
XMLEditIncludes::CountIncludes(const QString &name) const
{
    Attribute *a = xmldoc->attribute;
    int count = 0;
    for (size_t j=0; j<a->includes.size(); j++)
        count += (name == a->includes[j]->include) ? 1 : 0;
    return count;
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
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:21:32 PST 2008
//    Added target.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditIncludes::UpdateWindowContents()
{
    BlockAllSignals(true);
    Attribute *a = xmldoc->attribute;
    includelist->clear();
    for (size_t i=0; i<a->includes.size(); i++)
    {
        if(CountIncludes(a->includes[i]->include) > 1)
        {
            QString id = QString("%1 [%2]").arg(a->includes[i]->include).arg(a->includes[i]->target);
            includelist->addItem(id);
        }
        else
            includelist->addItem(a->includes[i]->include);
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
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:21:32 PST 2008
//    Added target.
//
// ****************************************************************************
void
XMLEditIncludes::UpdateWindowSensitivity()
{
    bool active = includelist->currentRow() != -1;

    delButton->setEnabled(includelist->count() > 0);
    target->setEnabled(active);
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
//    Brad Whitlock, Thu Mar 6 16:21:32 PST 2008
//    Added target.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
//    Kathleen Biagas, Thu Jan  2 08:59:31 MST 2020
//    Set fileGroup and quoteGroup checked buttons when index != -1.
//
// ****************************************************************************

void
XMLEditIncludes::UpdateWindowSingleItem()
{
    BlockAllSignals(true);

    Attribute *a = xmldoc->attribute;
    int index = includelist->currentRow();

    if (index == -1)
    {
        target->setText("");
        file->setText("");
        //fileGroup->setButton(-1);
        //quotedGroup->setButton(-1);
    }
    else
    {
        Include *n = a->includes[index];
        target->setText(n->target);
        file->setText(n->include);
        fileGroup->button((n->destination == "source") ? 0 : 1)->setChecked(true);
        quotedGroup->button(n->quoted ? 0 : 1)->setChecked(true);
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
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:21:32 PST 2008
//    Added target.
//
// ****************************************************************************
void
XMLEditIncludes::BlockAllSignals(bool block)
{
    includelist->blockSignals(block);
    target->blockSignals(block);
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
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:21:32 PST 2008
//    Added target.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditIncludes::includeTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = includelist->currentRow();
    if (index == -1)
        return;
    Include *n = a->includes[index];

    QString newinclude = text.trimmed();
    n->include = newinclude;
    if(CountIncludes(newinclude) > 1)
    {
        newinclude += "[";
        newinclude += n->target;
        newinclude += "]";
    }
    BlockAllSignals(true);
    includelist->item(index)->setText(newinclude);
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditIncludes::targetTextChanged
//
//  Programmer:  Brad Whitlock
//  Creation:    Thu Mar 6 15:56:05 PST 2008
//
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditIncludes::targetTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = includelist->currentRow();
    if (index == -1)
        return;
    Include *n = a->includes[index];

    n->target = text;
    includeTextChanged(n->include);
}

// ****************************************************************************
//  Method:  XMLEditIncludes::fileGroupChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditIncludes::fileGroupChanged(int fg)
{
    Attribute *a = xmldoc->attribute;
    int index = includelist->currentRow();
    if (index == -1)
        return;
    Include *n = a->includes[index];

    n->destination = (fg == 0) ? "source" : "header";
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
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************

void
XMLEditIncludes::quotedGroupChanged(int qg)
{
    Attribute *a = xmldoc->attribute;
    int index = includelist->currentRow();
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
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:28:33 PST 2008
//    Added default target.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
//    Kathleen Biagas, Thu Jan 21, 2021
//    Replace QString.asprintf with QString.arg.
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
        newname = QString("unnamed%1").arg(newid);
        for (int i=0; i<includelist->count() && okay; i++)
        {
            if (includelist->item(i)->text() == newname)
                okay = false;
        }
        if (!okay)
            newid++;
    }

    Include *n = new Include("header",false, "xml2atts");
    n->include = newname;

    a->includes.push_back(n);
    UpdateWindowContents();
    for (int i=0; i<includelist->count(); i++)
    {
        if (includelist->item(i)->text() == newname)
        {
            includelist->setCurrentRow(i);
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
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditIncludes::includelistDel()
{
    Attribute *a = xmldoc->attribute;
    int index = includelist->currentRow();

    if (index == -1)
        return;

    Include *n = a->includes[index];
    std::vector<Include*> newlist;
    for (size_t i=0; i<a->includes.size(); i++)
    {
        if (a->includes[i] != n)
            newlist.push_back(a->includes[i]);
    }
    a->includes = newlist;

    delete n;

    UpdateWindowContents();

    if (index >= includelist->count())
        index = includelist->count()-1;
    includelist->setCurrentRow(index);
}
