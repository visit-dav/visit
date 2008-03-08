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

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qpushbutton.h>

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
//   
// ****************************************************************************

XMLEditCodeSelectionDialog::XMLEditCodeSelectionDialog(QWidget *parent, const char *name) :
    QDialog(parent, name)
{
    QVBoxLayout *pageLayout = new QVBoxLayout(this);
    pageLayout->setMargin(10);
    pageLayout->setSpacing(10);

    QWidget *top = this;
    QHBoxLayout *hLayout = new QHBoxLayout(pageLayout);
    hLayout->setSpacing(10);

    //
    // Attributes
    //
    QGroupBox *attGroup = new QGroupBox(top, "attGroup");
    attGroup->setTitle("Attributes");
    hLayout->addWidget(attGroup);
    QVBoxLayout *attLayout = new QVBoxLayout(attGroup);
    attLayout->setMargin(10);
    attLayout->addSpacing(15);
    attLayout->setSpacing(5);
    buttons[ID_XML2ATTS] = new QCheckBox("C++", attGroup);
    attLayout->addWidget(buttons[ID_XML2ATTS]);
    buttons[ID_XML2JAVA] = new QCheckBox("Java", attGroup);
    attLayout->addWidget(buttons[ID_XML2JAVA]);
    buttons[ID_XML2PYTHON] = new QCheckBox("Python", attGroup);
    attLayout->addWidget(buttons[ID_XML2PYTHON]);

    //
    // Plugin
    //
    QGroupBox *pluginGroup = new QGroupBox(top, "pluginGroup");
    pluginGroup->setTitle("Plugin");
    hLayout->addWidget(pluginGroup);
    QVBoxLayout *pluginLayout = new QVBoxLayout(pluginGroup);
    pluginLayout->setMargin(10);
    pluginLayout->addSpacing(15);
    pluginLayout->setSpacing(5);
    buttons[ID_XML2WINDOW] = new QCheckBox("Window", pluginGroup);
    pluginLayout->addWidget(buttons[ID_XML2WINDOW]);
#if defined(_WIN32)
    buttons[ID_XML2MAKEFILE] = new QCheckBox("Project file", pluginGroup);
#else
    buttons[ID_XML2MAKEFILE] = new QCheckBox("Makefile", pluginGroup);
#endif
    pluginLayout->addWidget(buttons[ID_XML2MAKEFILE]);
    buttons[ID_XML2INFO] = new QCheckBox("Plugin information", pluginGroup);
    pluginLayout->addWidget(buttons[ID_XML2INFO]);
    buttons[ID_XML2AVT] = new QCheckBox("AVT code skeleton", pluginGroup);
    pluginLayout->addWidget(buttons[ID_XML2AVT]);

    // Add some buttons
    QHBoxLayout *bLayout = new QHBoxLayout(pageLayout);
    bLayout->setSpacing(5);
    bLayout->addStretch(10);
    QPushButton *okay = new QPushButton("Ok", top, "okay");
    QPushButton *cancel = new QPushButton("Cancel", top, "cancel");
    bLayout->addWidget(cancel);
    bLayout->addWidget(okay);
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
    XMLEditCodeSelectionDialog *inst = new XMLEditCodeSelectionDialog(0, "codesel");
    inst->setCaption(caption);

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
