// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "XMLEditStd.h"
#include "XMLEditConditional.h"

#include <XMLDocument.h>
#include <Attribute.h>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>

#include <vector>

// ****************************************************************************
//  Constructor:  XMLEditConditional::XMLEditConditional
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modificiations:
//    Kathleen Biagas, Thu Oct 9, 2025
//    Add cxxflags.
//
// ****************************************************************************

XMLEditConditional::XMLEditConditional(QWidget *p)
    : QFrame(p)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);

    QGridLayout *listLayout = new QGridLayout();

    conditionList = new QListWidget(this);
    listLayout->addWidget(conditionList, 0,0, 1,2);

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
    target->setText("xml2cmake");
    // for now xml2cmake is the only tool that uses conditionals
    target->setReadOnly(true);
    target->setEnabled(false);
    topLayout->addWidget(target, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("Condition"), this), row, 0);
    condition = new QLineEdit(this);
    topLayout->addWidget(condition, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("Definitions"), this), row, 0);
    row++;

    QFont monospaced("Courier");

    definitions = new QTextEdit(this);
    definitions->setFont(monospaced);
    definitions->setWordWrapMode(QTextOption::NoWrap);
    topLayout->addWidget(definitions, row,0, 1,2);
    row++;

    topLayout->addWidget(new QLabel(tr("CXXFlags"), this), row, 0);
    row++;

    cxxflags = new QTextEdit(this);
    cxxflags->setFont(monospaced);
    cxxflags->setWordWrapMode(QTextOption::NoWrap);
    topLayout->addWidget(cxxflags, row,0, 1,2);
    row++;

    topLayout->addWidget(new QLabel(tr("MLinkLibraries"), this), row, 0);
    row++;

    mlinklibs = new QTextEdit(this);
    mlinklibs->setFont(monospaced);
    mlinklibs->setWordWrapMode(QTextOption::NoWrap);
    topLayout->addWidget(mlinklibs, row,0, 1,2);
    row++;

    topLayout->addWidget(new QLabel(tr("ELinkLibraries"), this), row, 0);
    row++;
    elinklibs = new QTextEdit(this);
    elinklibs->setFont(monospaced);
    elinklibs->setWordWrapMode(QTextOption::NoWrap);
    topLayout->addWidget(elinklibs, row,0, 1,2);
    row++;

    topLayout->setRowMinimumHeight(row, 20);
    row++;
    hLayout->addLayout(topLayout);

    connect(conditionList, SIGNAL(currentRowChanged(int)),
            this, SLOT(UpdateWindowSingleItem()));
    connect(condition, SIGNAL(editingFinished()),
            this, SLOT(conditionTextChanged()));
    connect(definitions, SIGNAL(textChanged()),
            this, SLOT(definitionsChanged()));
    connect(cxxflags, SIGNAL(textChanged()),
            this, SLOT(cxxflagsChanged()));
    connect(mlinklibs, SIGNAL(textChanged()),
            this, SLOT(mlinklibsChanged()));
    connect(elinklibs, SIGNAL(textChanged()),
            this, SLOT(elinklibsChanged()));
    connect(newButton, SIGNAL(clicked()),
            this, SLOT(conditionlistNew()));
    connect(delButton, SIGNAL(clicked()),
            this, SLOT(conditionlistDel()));
}

// ****************************************************************************
//  Method:  XMLEditConditional::UpdateWindowContents
//
//  Purpose:
//    Update the window based on the current state.
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modifications:
//
// ****************************************************************************

void
XMLEditConditional::UpdateWindowContents()
{
    BlockAllSignals(true);

    Attribute *a = xmldoc->attribute;
    conditionList->clear();
    for (size_t i=0; i<a->conditionals.size(); i++)
    {
        conditionList->addItem(a->conditionals[i]->condition);
    }

    BlockAllSignals(false);
    UpdateWindowSingleItem();
}

// ****************************************************************************
//  Method:  XMLEditConditional::UpdateWindowSensitivity
//
//  Purpose:
//    Enable/disable widget sensitivity based on the current state.
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modifications:
//    Kathleen Biagas, Thu Oct 9, 2025
//    Add cxxflags.
//
// ****************************************************************************

void
XMLEditConditional::UpdateWindowSensitivity()
{
    bool active = conditionList->currentRow() != -1;

    delButton->setEnabled(conditionList->count() > 0);
    target->setEnabled(active);
    condition->setEnabled(active);
    definitions->setEnabled(active);
    cxxflags->setEnabled(active);
    mlinklibs->setEnabled(active);
    elinklibs->setEnabled(active);
}

// ****************************************************************************
//  Method:  XMLEditConditional::UpdateWindowSingleItem
//
//  Purpose:
//    Update the window based on the state a single item in the list.
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modifications:
//    Kathleen Biagas, Thu Oct 9, 2025
//    Add cxxflags.
//
// ****************************************************************************

void
XMLEditConditional::UpdateWindowSingleItem()
{
    BlockAllSignals(true);

    Attribute *a = xmldoc->attribute;
    int index = conditionList->currentRow();
    if (index == -1)
    {
        target->setText("");
        condition->setText("");
        definitions->setText("");
        cxxflags->setText("");
        mlinklibs->setText("");
        elinklibs->setText("");
    }
    else
    {
        Conditional *c = a->conditionals[index];
        target->setText(c->target);
        condition->setText(c->condition);
        definitions->setText(c->definitions);
        cxxflags->setText(c->cxxflags);
        mlinklibs->setText(c->mlinklibs);
        elinklibs->setText(c->elinklibs);
    }

    UpdateWindowSensitivity();
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditConditional::BlockAllSignals
//
//  Purpose:
//    Blocks/unblocks signals to the widgets.  This lets them get
//    updated by changes in state without affecting the state.
//
//  Arguments:
//    block      whether to block (true) or unblock (false) signals
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modifications:
//    Kathleen Biagas, Thu Oct 9, 2025
//    Add cxxflags.
//
// ****************************************************************************
void
XMLEditConditional::BlockAllSignals(bool block)
{
    conditionList->blockSignals(block);
    target->blockSignals(block);
    condition->blockSignals(block);
    definitions->blockSignals(block);
    cxxflags->blockSignals(block);
    mlinklibs->blockSignals(block);
    elinklibs->blockSignals(block);
}


// ****************************************************************************
//  Method:  XMLEditConditional::conditionTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modifications:
//
// ****************************************************************************

void
XMLEditConditional::conditionTextChanged()
{
    conditionTextChanged(condition->text());
}

void
XMLEditConditional::conditionTextChanged(const QString &text)
{
    Attribute *a = xmldoc->attribute;
    int index = conditionList->currentRow();
    if (index == -1)
        return;

    QString newcondition = text.trimmed();

    bool alreadyExists = false;
    for (size_t j=0 && !alreadyExists; j<a->conditionals.size(); j++)
        alreadyExists = (newcondition == a->conditionals[j]->condition);

    if(alreadyExists)
    {
        QMessageBox::warning(0, "VisIt", QString("Warning, Condition %1 already exists, choose another condition.").arg(newcondition), QMessageBox::Ok);
        return;
    }
    Conditional *c = a->conditionals[index];
    c->condition = newcondition;
    BlockAllSignals(true);
    conditionList->item(index)->setText(newcondition);
    BlockAllSignals(false);
}


// ****************************************************************************
//  Method:  XMLEditConditional::definitionsChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modifications:
//
// ****************************************************************************

void
XMLEditConditional::definitionsChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = conditionList->currentRow();
    if (index == -1)
        return;
    Conditional *c = a->conditionals[index];

    c->definitions = definitions->toPlainText();
}

// ****************************************************************************
//  Method:  XMLEditConditional::cxxflagsChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    Oct 9, 2025
//
//  Modifications:
//
// ****************************************************************************

void
XMLEditConditional::cxxflagsChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = conditionList->currentRow();
    if (index == -1)
        return;
    Conditional *c = a->conditionals[index];

    c->cxxflags = cxxflags->toPlainText();
}

// ****************************************************************************
//  Method:  XMLEditConditional::mlinklibsChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modifications:
//
// ****************************************************************************

void
XMLEditConditional::mlinklibsChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = conditionList->currentRow();
    if (index == -1)
        return;
    Conditional *c = a->conditionals[index];

    c->mlinklibs = mlinklibs->toPlainText();
}

// ****************************************************************************
//  Method:  XMLEditConditional::elinklibsChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modifications:
//
// ****************************************************************************

void
XMLEditConditional::elinklibsChanged()
{
    Attribute *a = xmldoc->attribute;
    int index = conditionList->currentRow();
    if (index == -1)
        return;
    Conditional *c = a->conditionals[index];

    c->elinklibs = elinklibs->toPlainText();
}

// ****************************************************************************
//  Method:  XMLEditConditional::conditionlistNew
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modifications:
//
// ****************************************************************************

void
XMLEditConditional::conditionlistNew()
{
    Attribute *a = xmldoc->attribute;
    int newid = 1;
    bool okay = false;
    QString newcondition;
    while (!okay)
    {
        okay = true;
        newcondition = tr("unknown%1").arg(newid);
        for (int i=0; i<conditionList->count() && okay; i++)
        {
            if (conditionList->item(i)->text() == newcondition)
                okay = false;
        }
        if (!okay)
            newid++;
    }

    Conditional *c = new Conditional("xml2cmake", newcondition);

    a->conditionals.push_back(c);
    UpdateWindowContents();
    for (int i=0; i<conditionList->count(); i++)
    {
        if (conditionList->item(i)->text() == newcondition)
        {
            conditionList->setCurrentRow(i);
            UpdateWindowSingleItem();
        }
    }
}

// ****************************************************************************
//  Method:  XMLEditConditional::conditionlistDel
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modifications:
//
// ****************************************************************************

void
XMLEditConditional::conditionlistDel()
{
    Attribute *a = xmldoc->attribute;
    int index = conditionList->currentRow();

    if (index == -1)
        return;

    Conditional *c = a->conditionals[index];
    std::vector<Conditional*> newlist;
    for (size_t i=0; i<a->conditionals.size(); i++)
    {
        if (a->conditionals[i] != c)
            newlist.push_back(a->conditionals[i]);
    }
    a->conditionals = newlist;

    delete c;

    UpdateWindowContents();

    if (index >= conditionList->count())
        index = conditionList->count()-1;
    conditionList->setCurrentRow(index);
}
