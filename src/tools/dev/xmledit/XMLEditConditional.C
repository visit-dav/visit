// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "XMLEditStd.h"
#include "XMLEditConditional.h"

#include <XMLDocument.h>
#include <Attribute.h>
#include <CodeFile.h>
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
//    Kathleen Biagas, Tue Oct 21, 2025
//    Add support for vlinklibs.
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

    topLayout->addWidget(new QLabel(tr("VLinkLibraries"), this), row, 0);
    row++;

    vlinklibs = new QTextEdit(this);
    vlinklibs->setFont(monospaced);
    vlinklibs->setWordWrapMode(QTextOption::NoWrap);
    topLayout->addWidget(vlinklibs, row,0, 1,2);
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
    connect(vlinklibs, SIGNAL(textChanged()),
            this, SLOT(vlinklibsChanged()));
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
//    Kathleen Biagas, Tue Oct 21, 2025
//    Conditions now stored in CodeFile.
//
// ****************************************************************************

void
XMLEditConditional::UpdateWindowContents()
{
    BlockAllSignals(true);

    conditionList->clear();
    CodeFile *codeFile = xmldoc->attribute->codeFile;
    if(codeFile)
    {
        for (size_t i = 0; i < codeFile->conditions.size(); i++)
        {
            conditionList->addItem(codeFile->conditions[i].condition);
        }
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
//    Kathleen Biagas, Tue Oct 21, 2025
//    Add vlinklibs.
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
    vlinklibs->setEnabled(active);
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
//    Kathleen Biagas, Tue Oct 21, 2025
//    Add vlinklibs.
//    Conditions now stored in CodeFile.
//
// ****************************************************************************

void
XMLEditConditional::UpdateWindowSingleItem()
{
    BlockAllSignals(true);

    int index = conditionList->currentRow();
    if (index == -1)
    {
        target->setText("");
        condition->setText("");
        definitions->setText("");
        cxxflags->setText("");
        mlinklibs->setText("");
        vlinklibs->setText("");
        elinklibs->setText("");
    }
    else
    {
        CodeFile *codeFile = xmldoc->attribute->codeFile;
        if(codeFile && !codeFile->conditions.empty())
        {
            Conditional c = codeFile->conditions[index];
            target->setText(c.target);
            condition->setText(c.condition);
            for (auto const& [key, val] : c.keyVals)
            {
                if(key == "Definitions:")
                    definitions->setText(val);
                else if(key == "CXXFlags:")
                    cxxflags->setText(val);
                else if(key == "MLinkLibraries:")
                    mlinklibs->setText(val);
                else if(key == "VLinkLibraries:")
                    vlinklibs->setText(val);
                else if(key == "ELinkLibraries:")
                    elinklibs->setText(val);
            }
        }
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
//    Kathleen Biagas, Tue Oct 21, 2025
//    Add vlinklibs.
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
    vlinklibs->blockSignals(block);
    elinklibs->blockSignals(block);
}


// ****************************************************************************
//  Method:  XMLEditConditional::conditionTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modifications:
//    Kathleen Biagas, Tue Oct 21, 2025
//    Conditions now stored in CodeFile.
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
    int index = conditionList->currentRow();
    if (index == -1)
        return;

    CodeFile *codeFile = xmldoc->attribute->codeFile;
    QString newcondition = text.trimmed();

    bool alreadyExists = false;
    for (size_t j=0 && !alreadyExists; j < codeFile->conditions.size(); j++)
        alreadyExists = (newcondition == codeFile->conditions[j].condition);

    if(alreadyExists)
    {
        QMessageBox::warning(0, "VisIt", QString("Warning, Condition %1 already exists, choose another condition.").arg(newcondition), QMessageBox::Ok);
        return;
    }

    Conditional c = codeFile->conditions[index];
    c.condition = newcondition;
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
//    Kathleen Biagas, Tue Oct 21, 2025
//    Conditions now stored in CodeFile.
//
// ****************************************************************************

void
XMLEditConditional::definitionsChanged()
{
    int index = conditionList->currentRow();
    if (index == -1)
        return;
    Conditional c = xmldoc->attribute->codeFile->conditions[index];
    c.keyVals["Definitions:"] = definitions->toPlainText();
}

// ****************************************************************************
//  Method:  XMLEditConditional::cxxflagsChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    Oct 9, 2025
//
//  Modifications:
//    Kathleen Biagas, Tue Oct 21, 2025
//    Conditions now stored in CodeFile.
//
// ****************************************************************************

void
XMLEditConditional::cxxflagsChanged()
{
    int index = conditionList->currentRow();
    if (index == -1)
        return;
    Conditional c = xmldoc->attribute->codeFile->conditions[index];
    c.keyVals["CXXFlags:"] = cxxflags->toPlainText();
}

// ****************************************************************************
//  Method:  XMLEditConditional::mlinklibsChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modifications:
//    Kathleen Biagas, Tue Oct 21, 2025
//    Conditions now stored in CodeFile.
//
// ****************************************************************************

void
XMLEditConditional::mlinklibsChanged()
{
    int index = conditionList->currentRow();
    if (index == -1)
        return;
    Conditional c = xmldoc->attribute->codeFile->conditions[index];
    c.keyVals["MLinkLibraries:"] = mlinklibs->toPlainText();
}

// ****************************************************************************
//  Method:  XMLEditConditional::vlinklibsChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    Oct 21, 2025 
//
//  Modifications:
//
// ****************************************************************************

void
XMLEditConditional::vlinklibsChanged()
{
    CodeFile *codeFile = xmldoc->attribute->codeFile;
    int index = conditionList->currentRow();
    if (index == -1)
        return;
    Conditional c = codeFile->conditions[index];
    c.keyVals["VLinkLibraries:"] = vlinklibs->toPlainText();
}

// ****************************************************************************
//  Method:  XMLEditConditional::elinklibsChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modifications:
//    Kathleen Biagas, Tue Oct 21, 2025
//    Conditions now stored in CodeFile.
//
// ****************************************************************************

void
XMLEditConditional::elinklibsChanged()
{
    CodeFile *codeFile = xmldoc->attribute->codeFile;
    int index = conditionList->currentRow();
    if (index == -1)
        return;
    Conditional c = codeFile->conditions[index];
    c.keyVals["ELinkLibraries:"] = elinklibs->toPlainText();
}

// ****************************************************************************
//  Method:  XMLEditConditional::conditionlistNew
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modifications:
//    Kathleen Biagas, Tue Oct 21, 2025
//    Conditions now stored in CodeFile.
//    Added test for existence of codeFile.
//
// ****************************************************************************

void
XMLEditConditional::conditionlistNew()
{
    if(!xmldoc->attribute->codeFile)
    {
        QMessageBox::warning(0, "VisIt", QString("Warning, Conditionals require a code file, but none has been specified."), QMessageBox::Ok);
        return;
    }
 
    CodeFile *codeFile = xmldoc->attribute->codeFile;
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

    Conditional c("xml2cmake", newcondition);
    codeFile->conditions.push_back(c);
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
//    Kathleen Biagas, Tue Oct 21, 2025
//    Conditions now stored in CodeFile.
//
// ****************************************************************************

void
XMLEditConditional::conditionlistDel()
{
    int index = conditionList->currentRow();

    if (index == -1)
        return;

    CodeFile *codeFile = xmldoc->attribute->codeFile;
    if(codeFile)
    {
        Conditional c = codeFile->conditions[index];
        std::vector<Conditional> newlist;
        for (size_t i = 0; i < codeFile->conditions.size(); i++)
        {
            if (codeFile->conditions[i] != c)
                newlist.push_back(codeFile->conditions[i]);
        }
        codeFile->conditions = newlist;
    }
    UpdateWindowContents();

    if (index >= conditionList->count())
        index = conditionList->count()-1;
    conditionList->setCurrentRow(index);
}
