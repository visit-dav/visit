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
    topLayout->addMultiCellWidget(definition, row,row, 0,1);
    row++;

    topLayout->addRowSpacing(row, 20);
    row++;

    connect(constantlist, SIGNAL(selectionChanged()),
            this, SLOT(UpdateWindowSingleItem()));
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
//  Method:  XMLEditConstants::UpdateWindowContents
//
//  Purpose:
//    Update the window based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
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
// ****************************************************************************
void
XMLEditConstants::UpdateWindowSensitivity()
{
    bool active = constantlist->currentItem() != -1;

    delButton->setEnabled(constantlist->count() > 0);
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
// ****************************************************************************
void
XMLEditConstants::UpdateWindowSingleItem()
{
    BlockAllSignals(true);

    Attribute *a = xmldoc->attribute;
    int index = constantlist->currentItem();

    if (index == -1)
    {
        name->setText("");
        member->setChecked(false);
        declaration->setText("");
        definition->setText("");
    }
    else
    {
        Constant *c = a->constants[index];
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
// ****************************************************************************
void
XMLEditConstants::BlockAllSignals(bool block)
{
    constantlist->blockSignals(block);
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

    BlockAllSignals(true);
    constantlist->changeItem(text, index);
    BlockAllSignals(false);
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
    
    Constant *c = new Constant(newname,"","",false);
    
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
