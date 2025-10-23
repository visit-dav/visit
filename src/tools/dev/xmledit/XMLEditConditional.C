// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "XMLEditStd.h"
#include "XMLEditConditional.h"

#include <XMLDocument.h>
#include <Attribute.h>
#include <CodeFile.h>
#include <Plugin.h>
#include <QFormLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>


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
//    Kathleen Biagas, Thu Oct 23, 2025
//    Switch QTextEdit to QLineEdit. Add labels ivars so that they can be
//    enabled/disabled along with their widgets.
//    Use QFormLayout instead of QGridLayout for topLayout to get reduced
//    blank space between widgets.
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

    QFormLayout *topLayout = new QFormLayout();

    targetLabel = new QLabel(tr("Target"), this);
    target = new QLineEdit(this);
    target->setText("xml2cmake");
    // for now xml2cmake is the only tool that uses conditionals
    target->setReadOnly(true);
    target->setEnabled(false);
    topLayout->addRow(targetLabel, target);

    conditionLabel = new QLabel(tr("Condition"), this);
    condition = new QLineEdit(this);
    topLayout->addRow(conditionLabel, condition);

    defLabel = new QLabel(tr("Definitions"), this);
    definitions = new QLineEdit(this);
    topLayout->addRow(defLabel, definitions);

    flagsLabel = new QLabel(tr("CXXFlags"), this);
    cxxflags = new QLineEdit(this);
    topLayout->addRow(flagsLabel, cxxflags);

    mlinkLabel = new QLabel(tr("MLinkLibraries"), this);
    mlinklibs = new QLineEdit(this);
    topLayout->addRow(mlinkLabel, mlinklibs);

    vlinkLabel = new QLabel(tr("VLinkLibraries"), this);
    vlinklibs = new QLineEdit(this);
    topLayout->addRow(vlinkLabel, vlinklibs);

    elinkLabel = new QLabel(tr("ELinkLibraries"), this);
    elinklibs = new QLineEdit(this);
    topLayout->addRow(elinkLabel, elinklibs);

    hLayout->addLayout(topLayout);

    connect(conditionList, SIGNAL(currentRowChanged(int)),
            this, SLOT(UpdateWindowSingleItem()));
    connect(condition, SIGNAL(editingFinished()),
            this, SLOT(conditionTextChanged()));

    connect(definitions, SIGNAL(editingFinished()),
            this, SLOT(definitionsChanged()));

    connect(cxxflags, SIGNAL(editingFinished()),
            this, SLOT(cxxflagsChanged()));
    connect(mlinklibs, SIGNAL(editingFinished()),
            this, SLOT(mlinklibsChanged()));
    connect(vlinklibs, SIGNAL(editingFinished()),
            this, SLOT(vlinklibsChanged()));
    connect(elinklibs, SIGNAL(editingFinished()),
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
            conditionList->addItem(codeFile->conditions[i]->condition);
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
//    Kathleen Biagas, Thu Oct 23, 2025
//    Since conditionals only apply to plugins take into account whether we
//    are dealing with a Plugin and which type.
//
// ****************************************************************************

void
XMLEditConditional::UpdateWindowSensitivity()
{
    // Conditionals are only for plugins, make sure
    // necessary fields are available.

    bool active = conditionList->currentRow() != -1;
    bool validPlugin = xmldoc->docType == "Plugin" && xmldoc->plugin;

    delButton->setEnabled(conditionList->count() > 0);
    newButton->setEnabled(validPlugin);

    bool isDB = validPlugin && xmldoc->plugin->type == "database";

    target->setEnabled(active && validPlugin);
    targetLabel->setEnabled(active && validPlugin);
    condition->setEnabled(active && validPlugin);
    conditionLabel->setEnabled(active && validPlugin);
    definitions->setEnabled(active && validPlugin);
    defLabel->setEnabled(active && validPlugin);
    cxxflags->setEnabled(active && validPlugin);
    flagsLabel->setEnabled(active && validPlugin);
    mlinklibs->setEnabled(active  && validPlugin && isDB);
    mlinkLabel->setEnabled(active  && validPlugin && isDB);
    vlinklibs->setEnabled(active  && validPlugin && !isDB);
    vlinkLabel->setEnabled(active  && validPlugin && !isDB);
    elinklibs->setEnabled(active && validPlugin);
    elinkLabel->setEnabled(active && validPlugin);
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
            Conditional *c = codeFile->conditions[index];
            target->setText(c->target);
            condition->setText(c->condition);
            for (auto const& [key, val] : c->keyVals)
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
        alreadyExists = (newcondition == codeFile->conditions[j]->condition);

    if(alreadyExists)
    {
        QMessageBox::warning(0, "VisIt", QString("Warning, Condition %1 already exists, choose another condition.").arg(newcondition), QMessageBox::Ok);
        return;
    }

    Conditional *c = codeFile->conditions[index];
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

    Conditional *c = xmldoc->attribute->codeFile->conditions[index];
    c->keyVals["Definitions:"] = definitions->text();
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
    Conditional *c = xmldoc->attribute->codeFile->conditions[index];
    c->keyVals["CXXFlags:"] = cxxflags->text();
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
    Conditional *c = xmldoc->attribute->codeFile->conditions[index];
    c->keyVals["MLinkLibraries:"] = mlinklibs->text();
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
    Conditional *c = codeFile->conditions[index];
    c->keyVals["VLinkLibraries:"] = vlinklibs->text();
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
}

// ****************************************************************************
//  Method:  XMLEditConditional::conditionlistNew
//
//  Programmer:  Kathleen Biagas
//  Creation:    Sep 29, 2025
//
//  Modifications:
//    Kathleen Biagas, Tue Oct 21, 2025
//    Added test for existence of codeFile, create one if it doesn't exist
//    since the conditions are now stored in the codefile.
//
// ****************************************************************************

void
XMLEditConditional::conditionlistNew()
{
    if(!xmldoc->attribute->codeFile)
    {
        QString codefilename(xmldoc->filename);
        // remove '.xml'
        codefilename.chop(4);
        codefilename.append(".code");
        xmldoc->attribute->codeFile = new CodeFile(codefilename);
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

    Conditional *c = new Conditional("xml2cmake", newcondition);
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
        Conditional *c = codeFile->conditions[index];
        std::vector<Conditional *> newlist;
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

