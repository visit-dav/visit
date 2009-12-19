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
#include "XMLEditFields.h"

#include <XMLDocument.h>
#include <Attribute.h>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QTextEdit>

// ****************************************************************************
//  Class:  QNarrowLineEdit
//
//  Purpose:
//    A QLineEdit that has a narrower default size.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 25, 2001
//
// ****************************************************************************
class QNarrowLineEdit : public QLineEdit
{
  public:
    QNarrowLineEdit(QWidget *p)
        : QLineEdit(p)
    {
    }
    QNarrowLineEdit(const QString &s, QWidget *p)
        : QLineEdit(s, p)
    {
    }
    QSize sizeHint() const
    {
        QSize size = QLineEdit::sizeHint();
        QFontMetrics fm(font());
        int w = fm.width('x') * 4; // 4 characters
        size.setWidth(w);
        return size;
    }
};

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
//    Brad Whitlock, Fri Mar 7 10:26:23 PDT 2008
//    Made values/code monospace.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************

XMLEditFields::XMLEditFields(QWidget *p)
    : QFrame(p)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    
    QGridLayout *listLayout = new QGridLayout();

    fieldlist = new QListWidget(this);
    fieldlist->addItem("testint");
    fieldlist->addItem("testfloat");
    listLayout->addWidget(fieldlist, 0,0, 1,2);

    upButton = new QPushButton(tr("Up"), this);
    listLayout->addWidget(upButton, 1,0);

    downButton = new QPushButton(tr("Down"), this);
    listLayout->addWidget(downButton, 1,1);

    newButton = new QPushButton(tr("New"), this);
    listLayout->addWidget(newButton, 2,0);

    delButton = new QPushButton(tr("Del"), this);
    listLayout->addWidget(delButton, 2,1);
    
    hLayout->addLayout(listLayout);
    hLayout->addSpacing(10);

    QGridLayout *topLayout = new QGridLayout();
    int row = 0;

    topLayout->setColumnMinimumWidth(0, 0);
    topLayout->setColumnMinimumWidth(1, 0);
    topLayout->setColumnMinimumWidth(2, 75);
    topLayout->setColumnMinimumWidth(3, 0);
    topLayout->setColumnMinimumWidth(4, 25);

    name = new QLineEdit(this);
    topLayout->addWidget(new QLabel(tr("Name"), this), row,0);
    topLayout->addWidget(name, row,1,1,5);
    row++;

    label = new QLineEdit(this);
    topLayout->addWidget(new QLabel(tr("Label"), this), row,0);
    topLayout->addWidget(label, row,1, 1,5);
    row++;

    type = new QComboBox(this);
    type->addItem("");
    type->setMinimumWidth(150);
    topLayout->addWidget(new QLabel(tr("Type"), this), row,0);
    topLayout->addWidget(type, row,1, 1,3);
    
    QHBoxLayout *lengthHBox = new QHBoxLayout();
    lengthHBox->addWidget(new QLabel(tr("Length"), this)); 
    length = new QNarrowLineEdit(this);
    lengthHBox->addWidget(length);
    topLayout->addLayout(lengthHBox, row, 4);
    row++;
    
    subtype = new QLineEdit(this);
    topLayout->addWidget(new QLabel(tr("Subtype"), this), row,0);
    topLayout->addWidget(subtype, row,1, 1,5);
    row++;

    enabler = new QComboBox(this);
    enabler->addItem(tr("(none)"));
    topLayout->addWidget(new QLabel(tr("Enabler"), this), row,0);
    topLayout->addWidget(enabler, row,1, 1,5);
    row++;

    enableval = new QLineEdit(this);
    topLayout->addWidget(new QLabel(tr("Values"), this), row,1, 1,1);
    topLayout->addWidget(enableval, row,2, 1,3); 
    row++;

    internal = new QCheckBox(tr("Internal use only"), this);
    topLayout->addWidget(internal, row,0, 1,3);

    // Add a group box that contains controls to set the variable
    // types that will be accepted by a variablename object.
    variableNameGroup = new QGroupBox(this);
    variableNameGroup->setTitle(tr("Accepted variable types"));
    
    QHBoxLayout *innerVarNameLayout = new QHBoxLayout(variableNameGroup);
    innerVarNameLayout->setMargin(10);
    QGridLayout *vnLayout = new QGridLayout();
    vnLayout->setSpacing(5);
    varNameButtons = new QButtonGroup(this);
    varNameButtons->setExclusive(false);
    connect(varNameButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(variableTypeClicked(int)));
    QCheckBox *cb = new QCheckBox(tr("Meshes"), variableNameGroup);
    varNameButtons->addButton(cb,0);
    vnLayout->addWidget(cb, 0, 0);
    cb = new QCheckBox(tr("Scalars"), variableNameGroup);
    varNameButtons->addButton(cb,1);
    vnLayout->addWidget(cb, 1, 0);
    cb = new QCheckBox(tr("Materials"), variableNameGroup);
    varNameButtons->addButton(cb,2);
    vnLayout->addWidget(cb, 2, 0);
    cb = new QCheckBox(tr("Labels"), variableNameGroup);
    varNameButtons->addButton(cb,9);
    vnLayout->addWidget(cb, 3, 0);

    cb = new QCheckBox(tr("Vectors"), variableNameGroup);
    varNameButtons->addButton(cb,3);
    vnLayout->addWidget(cb, 0, 1);
    cb = new QCheckBox(tr("Subsets"), variableNameGroup);
    varNameButtons->addButton(cb,4);
    vnLayout->addWidget(cb, 1, 1);
    cb = new QCheckBox(tr("Species"), variableNameGroup);
    varNameButtons->addButton(cb,5);
    vnLayout->addWidget(cb, 2, 1);
    cb = new QCheckBox(tr("Arrays"), variableNameGroup);
    varNameButtons->addButton(cb,10);
    vnLayout->addWidget(cb, 3, 1);

    cb = new QCheckBox(tr("Curves"), variableNameGroup);
    varNameButtons->addButton(cb,6);
    vnLayout->addWidget(cb, 0, 2);
    cb = new QCheckBox(tr("Tensors"), variableNameGroup);
    varNameButtons->addButton(cb,7);
    vnLayout->addWidget(cb, 1, 2);
    cb = new QCheckBox(tr("Symmetric Tensors"), variableNameGroup);
    varNameButtons->addButton(cb,8);
    vnLayout->addWidget(cb, 2, 2);
    innerVarNameLayout->addLayout(vnLayout);
    
    topLayout->addWidget(variableNameGroup, row,3,3,2);
    row++;

    ignoreeq = new QCheckBox(tr("Ignore field for attribute\nequality calculations"), this);
    topLayout->addWidget(ignoreeq, row,0, 1,3);
    row++;

    accessGroup = new QGroupBox(tr("Access"),this);
    
    access = new QButtonGroup(accessGroup);
    QRadioButton *access_private   = new QRadioButton(tr("private"), accessGroup);
    QRadioButton *access_protected = new QRadioButton(tr("protected"), accessGroup);
    QRadioButton *access_public    = new QRadioButton(tr("public"), accessGroup);
    
    access->addButton(access_private,0);
    access->addButton(access_protected,1);
    access->addButton(access_public,2);
    
    // create a vbox layout for the access group box
    QVBoxLayout *access_layout= new QVBoxLayout(accessGroup);
    accessGroup->setLayout(access_layout);
    access_layout->addWidget(access_private);
    access_layout->addWidget(access_protected);
    access_layout->addWidget(access_public);
        
    topLayout->addWidget(accessGroup, row,0, 1,3);
    row++;

    init = new QCheckBox(tr("Special initialization code"), this);
    topLayout->addWidget(init, row,0, 1,3);
    row++;

    topLayout->addWidget(new QLabel(tr("Initialization Values / Code"), this), row,0, 1,4);
    row++;

    values = new QTextEdit(this);
    QFont monospaced("Courier");
    values->setFont(monospaced);
    values->setWordWrapMode(QTextOption::NoWrap);
    topLayout->addWidget(values, row,1, 1,4);
    hLayout->addLayout(topLayout);
    
    connect(fieldlist, SIGNAL(currentRowChanged(int)),
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
    connect(access, SIGNAL(buttonClicked(int)),
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
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFields::UpdateWindowContents()
{
    BlockAllSignals(true);

    Attribute *a = xmldoc->attribute;
    fieldlist->clear();
    for (size_t i=0; i<a->fields.size(); i++)
    {
        fieldlist->addItem(a->fields[i]->name);
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
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************

void
XMLEditFields::UpdateWindowSensitivity()
{
    bool active = fieldlist->currentRow() != -1;

    delButton->setEnabled(fieldlist->count() > 0);
    upButton->setEnabled(fieldlist->currentRow() > 0);
    downButton->setEnabled(fieldlist->currentRow() < fieldlist->count()-1);
    name->setEnabled(active);
    label->setEnabled(active);
    type->setEnabled(active);
    subtype->setEnabled(active && (type->currentText() == "att" ||
                                   type->currentText() == "attVector"));
    length->setEnabled(active && (type->currentText().right(5) == "Array"));
    enabler->setEnabled(active);
    enableval->setEnabled(active && enabler->currentText() != tr("(none)"));
    internal->setEnabled(active);
    ignoreeq->setEnabled(active);
    variableNameGroup->setEnabled(active && type->currentText() == "variablename");
    init->setEnabled(active);
    values->setEnabled(active);
    accessGroup->setEnabled(active);
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
//     Brad Whitlock, Thu Mar 6 15:00:08 PST 2008
//     Deal with multi-target init codes.
//
//     Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//     First pass at porting to Qt 4.4.0
//
//     Jeremy Meredith, Thu Mar 19 12:01:02 EDT 2009
//     Finish Qt4 port -- ids for variable type buttons weren't used.
//
// ****************************************************************************

void
XMLEditFields::UpdateWindowSingleItem()
{
    BlockAllSignals(true);

    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();

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
        access->button(0)->setChecked(true);
        init->setChecked(false);
        values->setText("");
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
                if (type->itemText(i) == enumtype)
                    type->setCurrentIndex(i);
            }
        }
        else
        {
            for (int i=0; i<type->count(); i++)
            {
                if (type->itemText(i) == f->type)
                    type->setCurrentIndex(i);
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
                if (enabler->itemText(i) == f->enabler->name)
                    enabler->setCurrentIndex(i);
            }
            enableval->setText(JoinValues(f->enableval, ' '));
        }
        else
        {
            enabler->setCurrentIndex(0);
            enableval->setText("");
        }
        internal->setChecked(f->internal);
        ignoreeq->setChecked(f->ignoreEquality);
        if(f->accessType == Field::AccessPrivate)
            access->button(0)->setChecked(true);
        else if(f->accessType == Field::AccessProtected)
            access->button(1)->setChecked(true);
        else if(f->accessType == Field::AccessPublic)
            access->button(2)->setChecked(true);
        if(f->type == "variablename")
        {
            foreach (QAbstractButton *b, varNameButtons->buttons())
            {
                int mask = (1 << varNameButtons->id(b));
                b->setChecked((f->varTypes & mask) != 0);
            }
        }

        if (f->initcode.size() < 1)
        {
            init->setChecked(false);
            values->setText(JoinValues(f->GetValueAsText(), '\n'));
        }
        else
        {
            init->setChecked(true);
            values->setText(f->InitCodeAsString());
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
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFields::UpdateTypeList()
{
    type->clear();
    type->addItem("int");
    type->addItem("intArray");
    type->addItem("intVector");
    type->addItem("bool");
    type->addItem("float");
    type->addItem("floatArray");
    type->addItem("double");
    type->addItem("doubleArray");
    type->addItem("doubleVector");
    type->addItem("uchar");
    type->addItem("ucharArray");
    type->addItem("ucharVector");
    type->addItem("string");
    type->addItem("stringVector");
    type->addItem("colortable");
    type->addItem("color");
    type->addItem("opacity");
    type->addItem("linestyle");
    type->addItem("linewidth");
    type->addItem("scalemode");
    type->addItem("variablename");
    type->addItem("att");
    type->addItem("attVector");

    // Add built-in AVT enums
    type->addItem("LoadBalanceScheme");
    type->addItem("avtCentering");
    type->addItem("avtExtentType");
    type->addItem("avtGhostType");
    type->addItem("avtMeshCoordType");
    type->addItem("avtMeshType");
    type->addItem("avtSubsetType");
    type->addItem("avtVarType");

    // Add enums
    for (size_t i=0; i<EnumType::enums.size(); i++)
    {
        type->addItem(QString("enum:") + EnumType::enums[i]->type);
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
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFields::UpdateEnablerList()
{
    Attribute *a = xmldoc->attribute;

    enabler->clear();
    enabler->addItem(tr("(none)"));
    for (size_t i=0; i<a->fields.size(); i++)
    {
        if (name->text() != a->fields[i]->name)
            enabler->addItem(a->fields[i]->name);
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
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFields::nameTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    QString newname = text.trimmed();
    f->name = newname;

    BlockAllSignals(true);
    fieldlist->item(index)->setText(newname);
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditFields::labelTextChanged
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
XMLEditFields::labelTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();
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
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFields::typeChanged(int typeindex)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    QString t = type->itemText(typeindex);
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

    for (size_t i=0; i<a->fields.size(); i++)
    {
        if (i == index)
            continue;

        if (a->fields[i]->enabler == f)
            a->fields[i]->enabler = n;
    }

    a->fields[index] = n;
    delete f;

    UpdateWindowContents();
    fieldlist->setCurrentRow(index);
}

// ****************************************************************************
//  Method:  XMLEditFields::subtypeTextChanged
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
XMLEditFields::subtypeTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();
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
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFields::enablerChanged(int enablerindex)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();
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
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFields::enablevalTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();
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
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFields::lengthTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();
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
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFields::internalChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();
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
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFields::ignoreeqChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();
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
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFields::accessChanged(int btn)
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();
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
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
//    Jeremy Meredith, Thu Mar 19 12:01:02 EDT 2009
//    Finish Qt4 port.
//
// ****************************************************************************

void
XMLEditFields::variableTypeClicked(int bIndex)
{
    QAbstractButton *b = varNameButtons->button(bIndex);

    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();
    if (index == -1)
        return;

    Field *f = a->fields[index];
    if(f->type == "variablename")
    {
        int mask = ~(1 << bIndex);
        int bit = (b->isChecked() ? 1 : 0) << bIndex;
        f->varTypes = (f->varTypes & mask) | bit;
    }
}

// ****************************************************************************
//  Method:  XMLEditFields::initChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 15:36:09 PST 2008
//    Updated.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFields::initChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    if (init->isChecked())
    {
        f->SetInitCodeFromString(values->toPlainText());
    }
    else
    {
        vector<QString> splitvalues = SplitValues(values->toPlainText());
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
        f->SetInitCodeFromString(QString());
    }
}

// ****************************************************************************
//  Method:  XMLEditFields::valuesChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 15:38:41 PST 2008
//    Updated init coding.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFields::valuesChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();
    if (index == -1)
        return;
    Field *f = a->fields[index];

    if (init->isChecked())
    {
        f->SetInitCodeFromString(values->toPlainText());
    }
    else
    {
        vector<QString> splitvalues = SplitValues(values->toPlainText());
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
        f->SetInitCodeFromString(QString());
    }
}

// ****************************************************************************
//  Method:  XMLEditFields::fieldlistNew
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
        for (size_t i=0; i<fieldlist->count() && okay; i++)
        {
            if (fieldlist->item(i)->text() == newname)
                okay = false;
        }
        if (!okay)
            newid++;
    }
    
    Field *f = FieldFactory::createField(newname,"int","","",newname);
    
    a->fields.push_back(f);
    UpdateWindowContents();
    for (size_t i=0; i<fieldlist->count(); i++)
    {
        if (fieldlist->item(i)->text() == newname)
        {
            fieldlist->setCurrentRow(i);
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
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLEditFields::fieldlistDel()
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();

    if (index == -1)
        return;

    Field *f = a->fields[index];
    vector<Field*> newlist;
    for (size_t i=0; i<a->fields.size(); i++)
    {
        if (a->fields[i] != f)
            newlist.push_back(a->fields[i]);
    }
    a->fields = newlist;

    // Make sure anyone with a reference to the old one
    // points to the new one instead
    for (size_t i=0; i<a->fields.size(); i++)
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
    fieldlist->setCurrentRow(index);
}

// ****************************************************************************
//  Method:  XMLEditFields::fieldlistUp
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
//
//    Cyrus Harrison, Mon Nov 24 09:09:51 PST 2008
//    Fixed bug w/ wrong text label in fieldList.
//
//
// ****************************************************************************
void
XMLEditFields::fieldlistUp()
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();

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
    fieldlist->item(index)->setText(a->fields[index]->name);
    fieldlist->item(index-1)->setText(a->fields[index-1]->name);
    fieldlist->setCurrentRow(index-1);
    BlockAllSignals(false);

    UpdateWindowSensitivity();
}

// ****************************************************************************
//  Method:  XMLEditFields::fieldlistDown
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
XMLEditFields::fieldlistDown()
{
    Attribute *a = xmldoc->attribute;
    int index = fieldlist->currentRow();

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
    fieldlist->item(index)->setText(a->fields[index]->name);
    fieldlist->item(index+1)->setText(a->fields[index+1]->name);
    fieldlist->setCurrentRow(index+1);
    BlockAllSignals(false);

    UpdateWindowSensitivity();
}
