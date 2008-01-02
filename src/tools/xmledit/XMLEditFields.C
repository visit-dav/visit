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

#include "XMLEditFields.h"

#include <XMLDocument.h>
#include <Attribute.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qmultilineedit.h>

#include <QNarrowLineEdit.h>

// ****************************************************************************
//  Constructor:  XMLEditFields::XMLEditFields
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Dec 10 10:29:49 PDT 2004
//    I added code to create widgets that have to do with attributes for the
//    variablename type.
//
//    Brad Whitlock, Fri Apr 1 16:05:13 PST 2005
//    Added Labels variable type.
//
//    Hank Childs, Tue Jul 19 14:08:19 PDT 2005
//    Added arrays.
//
//    Brad Whitlock, Wed Feb 28 18:42:51 PST 2007
//    Added public/protected/private.
//
// ****************************************************************************

XMLEditFields::XMLEditFields(QWidget *p, const QString &n)
    : QFrame(p, n)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);

    QGridLayout *listLayout = new QGridLayout(hLayout, 3,2, 5);

    fieldlist = new QListBox(this);
    fieldlist->insertItem("testint");
    fieldlist->insertItem("testfloat");
    listLayout->addMultiCellWidget(fieldlist, 0,0, 0,1);

    upButton = new QPushButton("Up", this);
    listLayout->addWidget(upButton, 1,0);

    downButton = new QPushButton("Down", this);
    listLayout->addWidget(downButton, 1,1);

    newButton = new QPushButton("New", this);
    listLayout->addWidget(newButton, 2,0);

    delButton = new QPushButton("Del", this);
    listLayout->addWidget(delButton, 2,1);

    hLayout->addSpacing(10);

    QGridLayout *topLayout = new QGridLayout(hLayout, 12,5, 5);
    int row = 0;

    topLayout->setColStretch(0, 0);
    topLayout->setColStretch(1, 0);
    topLayout->setColStretch(2, 75);
    topLayout->setColStretch(3, 0);
    topLayout->setColStretch(4, 25);

    name = new QLineEdit(this);
    topLayout->addMultiCellWidget(new QLabel("Name", this), row,row, 0,0);
    topLayout->addMultiCellWidget(name, row,row, 1,4);
    row++;

    label = new QLineEdit(this);
    topLayout->addMultiCellWidget(new QLabel("Label", this), row,row, 0,0);
    topLayout->addMultiCellWidget(label, row,row, 1,4);
    row++;

    type = new QComboBox(this);
    type->insertItem("");
    type->setMinimumWidth(150);
    topLayout->addMultiCellWidget(new QLabel("Type", this), row,row, 0,0);
    topLayout->addMultiCellWidget(type, row,row, 1,3);
    QHBox *lengthHBox = new QHBox(this, "lengthHBox");
    new QLabel("Length", lengthHBox);
    length = new QNarrowLineEdit(lengthHBox);
    topLayout->addWidget(lengthHBox, row, 4);
    row++;

    subtype = new QLineEdit(this);
    topLayout->addMultiCellWidget(new QLabel("Subtype", this), row,row, 0,0);
    topLayout->addMultiCellWidget(subtype, row,row, 1,4);
    row++;

    enabler = new QComboBox(this);
    enabler->insertItem("(none)");
    topLayout->addMultiCellWidget(new QLabel("Enabler", this), row,row, 0,0);
    topLayout->addMultiCellWidget(enabler, row,row, 1,4);
    row++;

    enableval = new QLineEdit(this);
    topLayout->addMultiCellWidget(new QLabel("Values", this), row,row, 1,1);
    topLayout->addMultiCellWidget(enableval, row,row, 2,4); 
    row++;

    internal = new QCheckBox("Internal use only", this);
    topLayout->addMultiCellWidget(internal, row,row, 0,2);

    // Add a group box that contains controls to set the variable
    // types that will be accepted by a variablename object.
    variableNameGroup = new QGroupBox(this, "variableNameGroup");
    variableNameGroup->setTitle("Accepted variable types");
    QVBoxLayout *innerVarNameLayout = new QVBoxLayout(variableNameGroup);
    innerVarNameLayout->setMargin(10);
    innerVarNameLayout->addSpacing(15);
    QGridLayout *vnLayout = new QGridLayout(innerVarNameLayout, 4, 3);
    vnLayout->setSpacing(5);
    varNameButtons = new QButtonGroup(0, "varNameButtons");
    connect(varNameButtons, SIGNAL(clicked(int)),
            this, SLOT(variableTypeClicked(int)));
    QCheckBox *cb = new QCheckBox("Meshes", variableNameGroup);
    varNameButtons->insert(cb,0);
    vnLayout->addWidget(cb, 0, 0);
    cb = new QCheckBox("Scalars", variableNameGroup);
    varNameButtons->insert(cb,1);
    vnLayout->addWidget(cb, 1, 0);
    cb = new QCheckBox("Materials", variableNameGroup);
    varNameButtons->insert(cb,2);
    vnLayout->addWidget(cb, 2, 0);
    cb = new QCheckBox("Labels", variableNameGroup);
    varNameButtons->insert(cb,9);
    vnLayout->addWidget(cb, 3, 0);

    cb = new QCheckBox("Vectors", variableNameGroup);
    varNameButtons->insert(cb,3);
    vnLayout->addWidget(cb, 0, 1);
    cb = new QCheckBox("Subsets", variableNameGroup);
    varNameButtons->insert(cb,4);
    vnLayout->addWidget(cb, 1, 1);
    cb = new QCheckBox("Species", variableNameGroup);
    varNameButtons->insert(cb,5);
    vnLayout->addWidget(cb, 2, 1);
    cb = new QCheckBox("Arrays", variableNameGroup);
    varNameButtons->insert(cb,10);
    vnLayout->addWidget(cb, 3, 1);

    cb = new QCheckBox("Curves", variableNameGroup);
    varNameButtons->insert(cb,6);
    vnLayout->addWidget(cb, 0, 2);
    cb = new QCheckBox("Tensors", variableNameGroup);
    varNameButtons->insert(cb,7);
    vnLayout->addWidget(cb, 1, 2);
    cb = new QCheckBox("Symmetric Tensors", variableNameGroup);
    varNameButtons->insert(cb,8);
    vnLayout->addWidget(cb, 2, 2);

    topLayout->addMultiCellWidget(variableNameGroup, row,row+2,3,4);
    row++;

    ignoreeq = new QCheckBox("Ignore field for attribute\nequality calculations", this);
    topLayout->addMultiCellWidget(ignoreeq, row,row, 0,2);
    row++;

    access = new QButtonGroup("Access", this, "access");
    access->setColumns(1);
    new QRadioButton("private", access, "private");
    new QRadioButton("protected", access, "protected");
    new QRadioButton("public", access, "public");
    topLayout->addMultiCellWidget(access, row,row, 0,2);
    row++;

    init = new QCheckBox("Special initialization code", this);
    topLayout->addMultiCellWidget(init, row,row, 0,2);
    row++;

    topLayout->addMultiCellWidget(new QLabel("Initialization Values / Code", this), row,row, 0,4);
    row++;

    values = new QMultiLineEdit(this);
    topLayout->addMultiCellWidget(values, row,row, 1,4);

    connect(fieldlist, SIGNAL(selectionChanged()),
            this, SLOT(UpdateWindowSingleItem()));
    connect(name, SIGNAL(textChanged(const QString&)),
            this, SLOT(nameTextChanged(const QString&)));
    connect(label, SIGNAL(textChanged(const QString&)),
            this, SLOT(labelTextChanged(const QString&)));
    connect(type, SIGNAL(activated(int)),
            this, SLOT(typeChanged(int)));
    connect(subtype, SIGNAL(textChanged(const QString&)),
            this, SLOT(subtypeTextChanged(const QString&)));
    connect(enabler, SIGNAL(activated(int)),
            this, SLOT(enablerChanged(int)));
    connect(enableval, SIGNAL(textChanged(const QString&)),
            this, SLOT(enablevalTextChanged(const QString&)));
    connect(length, SIGNAL(textChanged(const QString&)),
            this, SLOT(lengthTextChanged(const QString&)));
    connect(internal, SIGNAL(clicked()),
            this, SLOT(internalChanged()));
    connect(ignoreeq, SIGNAL(clicked()),
            this, SLOT(ignoreeqChanged()));
    connect(init, SIGNAL(clicked()),
            this, SLOT(initChanged()));
    connect(values, SIGNAL(textChanged()),
            this, SLOT(valuesChanged()));
    connect(newButton, SIGNAL(pressed()),
            this, SLOT(fieldlistNew()));
    connect(delButton, SIGNAL(pressed()),
            this, SLOT(fieldlistDel()));
    connect(upButton, SIGNAL(pressed()),
            this, SLOT(fieldlistUp()));
    connect(downButton, SIGNAL(pressed()),
            this, SLOT(fieldlistDown()));
    connect(access, SIGNAL(clicked(int)),
            this, SLOT(accessChanged(int)));
}

// ****************************************************************************
//  Method:  XMLEditFields::UpdateWindowContents
//
//  Purpose:
//    Update the window based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFields::UpdateWindowContents()
{
    BlockAllSignals(true);

    Attribute *a = xmldoc->attribute;
    fieldlist->clear();
    for (int i=0; i<a->fields.size(); i++)
    {
        fieldlist->insertItem(a->fields[i]->name);
    }
    BlockAllSignals(false);

    UpdateWindowSingleItem();
}

// ****************************************************************************
//  Method:  XMLEditFields::UpdateWindowSensitivity
//
//  Purpose:
//    Enable/disable widget sensitivity based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Dec 10 10:35:26 PDT 2004
//    I added variableNameGroup.
//
//    Brad Whitlock, Wed Feb 28 18:47:36 PST 2007
//    Added access.
//
// ****************************************************************************

void
XMLEditFields::UpdateWindowSensitivity()
{
    bool active = fieldlist->currentItem() != -1;

    delButton->setEnabled(fieldlist->count() > 0);
    upButton->setEnabled(fieldlist->currentItem() > 0);
    downButton->setEnabled(fieldlist->currentItem() < fieldlist->count()-1);
    name->setEnabled(active);
    label->setEnabled(active);
    type->setEnabled(active);
    subtype->setEnabled(active && (type->currentText() == "att" ||
                                   type->currentText() == "attVector"));
    length->setEnabled(active && (type->currentText().right(5) == "Array"));
    enabler->setEnabled(active);
    enableval->setEnabled(active && enabler->currentText() != "(none)");
    internal->setEnabled(active);
    ignoreeq->setEnabled(active);
    variableNameGroup->setEnabled(active && type->currentText() == "variablename");
    init->setEnabled(active);
    values->setEnabled(active);
    access->setEnabled(active);
}

// ****************************************************************************
//  Method:  XMLEditFields::UpdateWindowSingleItem
//
//  Purpose:
//    Update the window based on the state a single item in the list.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//     Brad Whitlock, Fri Dec 10 10:35:44 PDT 2004
//     I added code to update a new button group.
//
//     Brad Whitlock, Wed Feb 28 18:49:14 PST 2007
//     Added access.
//
// ****************************************************************************

void
XMLEditFields::UpdateWindowSingleItem()
{
    BlockAllSignals(true);

    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();

    if (index == -1)
    {
        name->setText("");
        label->setText("");
        type->clear();
        subtype->setText("");
        length->setText("");
        enabler->clear();
        enableval->setText("");
        internal->setChecked(false);
        ignoreeq->setChecked(false);
        access->setButton(0);
        init->setChecked(false);
        values->setText("");
        for(int i = 0; i < varNameButtons->count(); ++i)
        {
            QButton *b = varNameButtons->find(i);
            if(b != 0 && b->isA("QCheckBox"))
            {
                QCheckBox *cb = (QCheckBox *)b;
                cb->setChecked(false);
            }
        }
    }
    else
    {
        Field *f = a->fields[index];
        name->setText(f->name);
        label->setText(f->label);
        UpdateTypeList();
        if (f->type == "enum")
        {
            QString enumtype = QString("enum:")+f->GetSubtype();
            for (int i=0; i<type->count(); i++)
            {
                if (type->text(i) == enumtype)
                    type->setCurrentItem(i);
            }
        }
        else
        {
            for (int i=0; i<type->count(); i++)
            {
                if (type->text(i) == f->type)
                    type->setCurrentItem(i);
            }
        }
        if (f->type == "att" || f->type == "attVector")
        {
            subtype->setText(f->GetSubtype());
        }
        else
        {
            subtype->setText("");
        }
        if (f->type.right(5) == "Array")
        {
            QString str;
            str.sprintf("%d",f->length);
            length->setText(str);
        }
        else
        {
            length->setText("");
        }
        UpdateEnablerList();
        if (f->enabler)
        {
            for (int i=0; i<enabler->count(); i++)
            {
                if (enabler->text(i) == f->enabler->name)
                    enabler->setCurrentItem(i);
            }
            enableval->setText(JoinValues(f->enableval, ' '));
        }
        else
        {
            enabler->setCurrentItem(0);
            enableval->setText("");
        }
        internal->setChecked(f->internal);
        ignoreeq->setChecked(f->ignoreEquality);
        if(f->accessType == Field::AccessPrivate)
            access->setButton(0);
        else if(f->accessType == Field::AccessProtected)
            access->setButton(1);
        else if(f->accessType == Field::AccessPublic)
            access->setButton(2);
        if(f->type == "variablename")
        {
            int mask = 1;
            for(int i = 0; i < varNameButtons->count(); ++i)
            {
                QButton *b = varNameButtons->find(i);
                if(b != 0 && b->isA("QCheckBox"))
                {
                    QCheckBox *cb = (QCheckBox *)b;
                    cb->setChecked((f->varTypes & mask) != 0);
                }
                mask = mask << 1;
            }
        }

        if (f->initcode.isNull())
        {
            init->setChecked(false);
            values->setText(JoinValues(f->GetValueAsText(), '\n'));
        }
        else
        {
            init->setChecked(true);
            values->setText(f->initcode);
        }
    }

    UpdateWindowSensitivity();

    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditFields::UpdateTypeList
//
//  Purpose:
//    Update the combo box listing the available types.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Mon Dec 9 13:33:54 PST 2002
//    I added ucharVector.
//
//    Brad Whitlock, Fri Dec 10 10:01:30 PDT 2004
//    I added variablename.
//
//    Brad Whitlock, Thu Mar 1 17:15:02 PST 2007
//    Added built-in AVT enums.
//
//    Kathleen Bonnell, Thu Mar 22 17:12:44 PDT 2007
//    Added scalemode.
//
// ****************************************************************************
void
XMLEditFields::UpdateTypeList()
{
    type->clear();
    type->insertItem("int");
    type->insertItem("intArray");
    type->insertItem("intVector");
    type->insertItem("bool");
    type->insertItem("float");
    type->insertItem("floatArray");
    type->insertItem("double");
    type->insertItem("doubleArray");
    type->insertItem("doubleVector");
    type->insertItem("uchar");
    type->insertItem("ucharArray");
    type->insertItem("ucharVector");
    type->insertItem("string");
    type->insertItem("stringVector");
    type->insertItem("colortable");
    type->insertItem("color");
    type->insertItem("opacity");
    type->insertItem("linestyle");
    type->insertItem("linewidth");
    type->insertItem("scalemode");
    type->insertItem("variablename");
    type->insertItem("att");
    type->insertItem("attVector");

    // Add built-in AVT enums
    type->insertItem("LoadBalanceScheme");
    type->insertItem("avtCentering");
    type->insertItem("avtExtentType");
    type->insertItem("avtGhostType");
    type->insertItem("avtMeshCoordType");
    type->insertItem("avtMeshType");
    type->insertItem("avtSubsetType");
    type->insertItem("avtVarType");

    // Add enums
    for (int i=0; i<EnumType::enums.size(); i++)
    {
        type->insertItem(QString("enum:") + EnumType::enums[i]->type);
    }
}

// ****************************************************************************
//  Method:  XMLEditFields::UpdateEnablerList
//
//  Purpose:
//    Update the combo box lsting the available fields as enablers.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFields::UpdateEnablerList()
{
    Attribute *a = xmldoc->attribute;

    enabler->clear();
    enabler->insertItem("(none)");
    for (int i=0; i<a->fields.size(); i++)
    {
        if (name->text() != a->fields[i]->name)
            enabler->insertItem(a->fields[i]->name);
    }
}

// ****************************************************************************
//  Method:  XMLEditFields::BlockAllSignals
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
//    Brad Whitlock, Fri Dec 10 10:45:37 PDT 2004
//    Added variablename type support.
//
//    Brad Whitlock, Wed Feb 28 18:49:38 PST 2007
//    Added access.
//
// ****************************************************************************
void
XMLEditFields::BlockAllSignals(bool block)
{
    fieldlist->blockSignals(block);
    name->blockSignals(block);
    label->blockSignals(block);
    type->blockSignals(block);
    subtype->blockSignals(block);
    length->blockSignals(block);
    enabler->blockSignals(block);
    enableval->blockSignals(block);
    internal->blockSignals(block);
    ignoreeq->blockSignals(block);
    varNameButtons->blockSignals(block);
    init->blockSignals(block);
    values->blockSignals(block);
    access->blockSignals(block);
}

// ----------------------------------------------------------------------------
//                                 Callbacks
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Method:  XMLEditFields::nameTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFields::nameTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    QString newname = text.stripWhiteSpace();
    f->name = newname;

    BlockAllSignals(true);
    fieldlist->changeItem(newname, index);
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditFields::labelTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFields::labelTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    f->label = text;
}

// ****************************************************************************
//  Method:  XMLEditFields::typeChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Jeremy Meredith, Tue Dec 10 10:24:31 PST 2002
//    Fixed a bug where setting a type to an enum would fail.
//
// ****************************************************************************
void
XMLEditFields::typeChanged(int typeindex)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    QString t = type->text(typeindex);
    QString st;
    if (t.left(5) == "enum:")
    {
        st = t.right(t.length()-5);
        t = "enum";
    }
    else
    {
        st = subtype->text();
    }

    Field *n = FieldFactory::createField(f->name,
                                         t,
                                         st,
                                         length->text(),
                                         f->label);
    n->CopyValues(f);

    for (int i=0; i<a->fields.size(); i++)
    {
        if (i == index)
            continue;

        if (a->fields[i]->enabler == f)
            a->fields[i]->enabler = n;
    }

    a->fields[index] = n;
    delete f;

    UpdateWindowContents();
    fieldlist->setCurrentItem(index);
}

// ****************************************************************************
//  Method:  XMLEditFields::subtypeTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFields::subtypeTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    f->SetSubtype(text);
}

// ****************************************************************************
//  Method:  XMLEditFields::enablerChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Mar 27 12:55:33 PST 2003
//    Fixed a bug with the enabler field selection not being translated
//    into the correct Field pointer.
//
// ****************************************************************************
void
XMLEditFields::enablerChanged(int enablerindex)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    // The index of the signal is the index of the selected entry in the list.
    // The current field is not in that list, but a "(none)" entry  does
    // appear at the top.  Thus, the mapping of list index to field index is
    // shifted up one because of the first entry, and then shifted back down
    // one after the index of the current field because it is skipped.
    int enablerfieldindex = (enablerindex <= index) ? 
                                                enablerindex-1 : enablerindex;
    if (enablerfieldindex == -1)
    {
        f->enabler = NULL;
        f->enableval.clear();
        enableval->clear();
    }
    else
    {
        f->enabler = a->fields[enablerfieldindex];
    }

    UpdateWindowSensitivity();
}

// ****************************************************************************
//  Method:  XMLEditFields::enablevalTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFields::enablevalTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    f->enableval = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditFields::lengthTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFields::lengthTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    f->length = text.toInt();
}

// ****************************************************************************
//  Method:  XMLEditFields::internalChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFields::internalChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    f->internal = internal->isChecked();
}

// ****************************************************************************
//  Method:  XMLEditFields::ignoreeqChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFields::ignoreeqChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    f->ignoreEquality = ignoreeq->isChecked();
}

// ****************************************************************************
//  Method:  XMLEditFields::accessChanged
//
//  Programmer:  Brad Whitlock
//  Creation:    Wed Feb 28 18:50:06 PST 2007
//
// ****************************************************************************
void
XMLEditFields::accessChanged(int btn)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    if(btn == 0)
        f->SetPrivateAccess();
    else if(btn == 1)
        f->SetProtectedAccess();
    else
        f->SetPublicAccess();
}

// ****************************************************************************
// Method: XMLEditFields::variableTypeClicked
//
// Purpose: 
//   This slot function is called when one of the vartype checkboxes is
//   clicked.
//
// Arguments:
//   bIndex : The index of the button that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 10 10:51:49 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
XMLEditFields::variableTypeClicked(int bIndex)
{
    QButton *b = varNameButtons->find(bIndex);
    if(b != 0 && b->isA("QCheckBox"))
    {
        Attribute *a = xmldoc->attribute;
        int index = fieldlist->currentItem();
        if (index == -1)
            return;
        Field *f = a->fields[index];
        if(f->type == "variablename")
        {
            QCheckBox *cb = (QCheckBox *)b;

            int mask = ~(1 << bIndex);
            int bit = (cb->isChecked() ? 1 : 0) << bIndex;
            f->varTypes = (f->varTypes & mask) | bit;
        }
    }
}

// ****************************************************************************
//  Method:  XMLEditFields::initChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFields::initChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    if (init->isChecked())
    {
        f->initcode=values->text();
    }
    else
    {
        vector<QString> splitvalues = SplitValues(values->text());
        int length = splitvalues.size();
        if (!f->isVector && (length > f->length))
            length = f->length;
        f->ClearValues();
        for (int i=0; i<length; i++)
        {
            try
            {
                f->SetValue(splitvalues[i], i);
            }
            catch(...)
            {
            }
        }
        f->initcode = QString();
    }
}

// ****************************************************************************
//  Method:  XMLEditFields::valuesChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFields::valuesChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    if (init->isChecked())
    {
        f->initcode=values->text();
    }
    else
    {
        vector<QString> splitvalues = SplitValues(values->text());
        int length = splitvalues.size();
        if (!f->isVector && (length > f->length))
            length = f->length;
        f->ClearValues();
        for (int i=0; i<length; i++)
        {
            try
            {
                f->SetValue(splitvalues[i], i);
            }
            catch(...)
            {
            }
        }
        f->initcode = QString();
    }
}

// ****************************************************************************
//  Method:  XMLEditFields::fieldlistNew
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFields::fieldlistNew()
{
    Attribute *a = xmldoc->attribute;
    int newid = 1;
    bool okay = false;
    QString newname;
    while (!okay)
    {
        okay = true;
        newname.sprintf("unnamed%d", newid);
        for (int i=0; i<fieldlist->count() && okay; i++)
        {
            if (fieldlist->text(i) == newname)
                okay = false;
        }
        if (!okay)
            newid++;
    }
    
    Field *f = FieldFactory::createField(newname,"int","","",newname);
    
    a->fields.push_back(f);
    UpdateWindowContents();
    for (int i=0; i<fieldlist->count(); i++)
    {
        if (fieldlist->text(i) == newname)
        {
            fieldlist->setCurrentItem(i);
            UpdateWindowSingleItem();
        }
    }
}

// ****************************************************************************
//  Method:  XMLEditFields::fieldlistDel
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFields::fieldlistDel()
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();

    if (index == -1)
        return;

    Field *f = a->fields[index];
    vector<Field*> newlist;
    int i;
    for (i=0; i<a->fields.size(); i++)
    {
        if (a->fields[i] != f)
            newlist.push_back(a->fields[i]);
    }
    a->fields = newlist;

    // Make sure anyone with a reference to the old one
    // points to the new one instead
    for (i=0; i<a->fields.size(); i++)
    {
        if (a->fields[i]->enabler == f)
        {
            a->fields[i]->enabler = NULL;
            a->fields[i]->enableval.clear();
        }
    }

    delete f;

    UpdateWindowContents();

    if (index >= fieldlist->count())
        index = fieldlist->count()-1;
    fieldlist->setCurrentItem(index);
}

// ****************************************************************************
//  Method:  XMLEditFields::fieldlistUp
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFields::fieldlistUp()
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();

    if (index == -1)
        return;

    if (index < 1)
        return;

    Field *f = a->fields[index];
    a->fields[index] = a->fields[index-1];
    a->fields[index-1] = f;

    a->fields[index]->index   = index;
    a->fields[index-1]->index = index-1;

    BlockAllSignals(true);
    fieldlist->changeItem(a->fields[index]->name, index);
    fieldlist->changeItem(a->fields[index-1]->name, index-1);
    fieldlist->setCurrentItem(index-1);
    BlockAllSignals(false);

    UpdateWindowSensitivity();
}

// ****************************************************************************
//  Method:  XMLEditFields::fieldlistDown
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditFields::fieldlistDown()
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentItem();

    if (index == -1)
        return;

    if (index > a->fields.size() - 2)
        return;

    Field *f = a->fields[index];
    a->fields[index] = a->fields[index+1];
    a->fields[index+1] = f;

    a->fields[index]->index   = index;
    a->fields[index+1]->index = index+1;

    BlockAllSignals(true);
    fieldlist->changeItem(a->fields[index]->name, index);
    fieldlist->changeItem(a->fields[index+1]->name, index+1);
    fieldlist->setCurrentItem(index+1);
    BlockAllSignals(false);

    UpdateWindowSensitivity();
}
