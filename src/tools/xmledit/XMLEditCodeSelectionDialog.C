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
    attLayout->setMargin(10);
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
    pluginLayout->setMargin(10);
    pluginLayout->setSpacing(5);
    buttons[ID_XML2WINDOW] = new QCheckBox(tr("Window"), pluginGroup);
    pluginLayout->addWidget(buttons[ID_XML2WINDOW]);
#if defined(_WIN32)
    buttons[ID_XML2MAKEFILE] = new QCheckBox(tr("Project file"), pluginGroup);
#else
    buttons[ID_XML2MAKEFILE] = new QCheckBox(tr("Makefile"), pluginGroup);
#endif
    pluginLayout->addWidget(buttons[ID_XML2MAKEFILE]);
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
