// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <XMLEditCodeSelectionDialog.h>

#include <QCheckBox>
#include <QGroupBox>
#include <QBoxLayout>
#include <QLayout>
#include <QPushButton>

// ****************************************************************************
// Method: XMLEditCodeSelectionDialog::XMLEditCodeSelectionDialog
//
// Purpose:
//   Constructor
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 7 16:24:29 PST 2008
//
// Modifications:
//   Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//   Ported to Qt 4.4
//
// ****************************************************************************

XMLEditCodeSelectionDialog::XMLEditCodeSelectionDialog(QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setSpacing(10);

    //
    // Attributes
    //
    QGroupBox *attGroup = new QGroupBox(this);
    attGroup->setTitle(tr("Attributes"));
    QVBoxLayout *attLayout = new QVBoxLayout(attGroup);
    attLayout->setContentsMargins(10,10,10,10);
    attLayout->setSpacing(5);
    buttons[ID_XML2ATTS] = new QCheckBox(tr("C++"), attGroup);
    attLayout->addWidget(buttons[ID_XML2ATTS]);
    buttons[ID_XML2JAVA] = new QCheckBox(tr("Java"), attGroup);
    attLayout->addWidget(buttons[ID_XML2JAVA]);
    buttons[ID_XML2PYTHON] = new QCheckBox(tr("Python"), attGroup);
    attLayout->addWidget(buttons[ID_XML2PYTHON]);
    hLayout->addWidget(attGroup);

    //
    // Plugin
    //
    QGroupBox *pluginGroup = new QGroupBox(this);
    pluginGroup->setTitle(tr("Plugin"));
    QVBoxLayout *pluginLayout = new QVBoxLayout(pluginGroup);
    pluginLayout->setContentsMargins(10,10,10,10);
    pluginLayout->setSpacing(5);
    buttons[ID_XML2WINDOW] = new QCheckBox(tr("Window"), pluginGroup);
    pluginLayout->addWidget(buttons[ID_XML2WINDOW]);
    buttons[ID_XML2CMAKE] = new QCheckBox(tr("CMake"), pluginGroup);
    pluginLayout->addWidget(buttons[ID_XML2CMAKE]);
    buttons[ID_XML2INFO] = new QCheckBox(tr("Plugin information"), pluginGroup);
    pluginLayout->addWidget(buttons[ID_XML2INFO]);
    buttons[ID_XML2AVT] = new QCheckBox(tr("AVT code skeleton"), pluginGroup);
    pluginLayout->addWidget(buttons[ID_XML2AVT]);
    hLayout->addWidget(pluginGroup);

    topLayout->addLayout(hLayout);

    // Add some buttons
    QHBoxLayout *bLayout = new QHBoxLayout();
    bLayout->setSpacing(5);
    bLayout->addStretch(10);
    QPushButton *okay = new QPushButton(tr("Ok"), this);
    QPushButton *cancel = new QPushButton(tr("Cancel"), this);
    bLayout->addWidget(cancel);
    bLayout->addWidget(okay);
//
    topLayout->addLayout(bLayout);

    connect(okay, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
}

// ****************************************************************************
// Method: XMLEditCodeSelectionDialog::~XMLEditCodeSelectionDialog
//
// Purpose:
//   Destructor.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 7 16:24:40 PST 2008
//
// Modifications:
//
// ****************************************************************************

XMLEditCodeSelectionDialog::~XMLEditCodeSelectionDialog()
{
}

// ****************************************************************************
// Method: XMLEditCodeSelectionDialog::selectTools
//
// Purpose:
//   Static function that lets the user select the tools to invoke.
//
// Arguments:
//   caption     : The dialog caption.
//   useTools    : The tools that will be used. Input/Output arg.
//   toolEnabled : Which tools will be enabled.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 7 16:24:48 PST 2008
//
// Modifications:
//
// ****************************************************************************

bool
XMLEditCodeSelectionDialog::selectTools(const QString &caption,
    bool *_useTools, const bool *_toolEnabled)
{
    XMLEditCodeSelectionDialog *inst = new XMLEditCodeSelectionDialog(0);
    inst->setWindowTitle(caption);

    for(int i = 0; i < ID_XML_MAX; ++i)
    {
        inst->buttons[i]->setChecked(_useTools[i]);
        inst->buttons[i]->setEnabled(_toolEnabled[i]);
    }

    bool retval;
    if((retval = (inst->exec() == QDialog::Accepted)) == true)
    {
        // Copy the used tools back so we can see what was selected.
        for(int i = 0; i < ID_XML_MAX; ++i)
            _useTools[i] = inst->buttons[i]->isChecked();
    }

    delete inst;

    return retval;
}
