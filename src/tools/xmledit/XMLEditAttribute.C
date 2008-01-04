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

#include "XMLEditAttribute.h"

#include <XMLDocument.h>
#include <Attribute.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>

// ****************************************************************************
//  Constructor:  XMLEditAttribute::XMLEditAttribute
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// Modifications:
//   Brad Whitlock, Wed Mar 7 16:05:46 PST 2007
//   Added keyframing check box.
//
// ****************************************************************************

XMLEditAttribute::XMLEditAttribute(QWidget *p, const QString &n)
    : QFrame(p, n)
{
    QGridLayout *topLayout = new QGridLayout(this, 8,3, 5);
    int row = 0;

    name = new QLineEdit(this);
    topLayout->addWidget(new QLabel("Name", this), row, 0);
    topLayout->addMultiCellWidget(name, row,row, 1,2);
    row++;

    purpose = new QLineEdit(this);
    topLayout->addWidget(new QLabel("Purpose", this), row, 0);
    topLayout->addMultiCellWidget(purpose, row,row, 1,2);
    row++;

    codefile = new QLineEdit(this);
    topLayout->addWidget(new QLabel("Code file", this), row, 0);
    topLayout->addMultiCellWidget(codefile, row,row, 1,2);
    row++;

    exportAPI = new QLineEdit(this);
    topLayout->addWidget(new QLabel("Export API", this), row, 0);
    topLayout->addMultiCellWidget(exportAPI, row,row, 1,2);
    row++;

    exportInclude = new QLineEdit(this);
    topLayout->addWidget(new QLabel("Export include", this), row, 0);
    topLayout->addMultiCellWidget(exportInclude, row,row, 1,2);
    row++;

    persistent = new QCheckBox("Persistent", this);
    topLayout->addMultiCellWidget(persistent, row,row, 0,2);
    row++;

    keyframe = new QCheckBox("Generate keyframing methods", this);
    topLayout->addMultiCellWidget(keyframe, row,row, 0,2);
    row++;

    topLayout->setRowStretch(row, 100);

    connect(name, SIGNAL(textChanged(const QString &)),
            this,  SLOT(nameTextChanged(const QString &)));
    connect(purpose, SIGNAL(textChanged(const QString &)),
            this,  SLOT(purposeTextChanged(const QString &)));
    connect(codefile, SIGNAL(textChanged(const QString &)),
            this,  SLOT(codefileTextChanged(const QString &)));
    connect(exportAPI, SIGNAL(textChanged(const QString &)),
            this,  SLOT(exportAPITextChanged(const QString &)));
    connect(exportInclude, SIGNAL(textChanged(const QString &)),
            this,  SLOT(exportIncludeTextChanged(const QString &)));
    connect(persistent, SIGNAL(clicked()),
            this, SLOT(persistentChanged()));
    connect(keyframe, SIGNAL(clicked()),
            this, SLOT(keyframeChanged()));
}

// ****************************************************************************
//  Method:  XMLEditAttribute::UpdateWindowContents
//
//  Purpose:
//    Update the window based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Mar 7 16:06:43 PST 2007
//    Added keyframe.
//
// ****************************************************************************
void
XMLEditAttribute::UpdateWindowContents()
{
    BlockAllSignals(true);

    bool plugin =  (xmldoc->docType == "Plugin");

    Attribute *a = xmldoc->attribute;
    name->setText(a->name);
    purpose->setText(a->purpose);
    if (a->codeFile)
        codefile->setText(a->codeFile->filebase);
    else
        codefile->setText("");
    if (plugin)
    {
        exportAPI->setText("");
        exportInclude->setText("");
    }
    else
    {
        exportAPI->setText(a->exportAPI);
        exportInclude->setText(a->exportInclude);
    }
    persistent->setChecked(a->persistent);
    keyframe->setChecked(a->keyframe);

    UpdateWindowSensitivity();

    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditAttribute::UpdateWindowSensitivity
//
//  Purpose:
//    Enable/disable widget sensitivity based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditAttribute::UpdateWindowSensitivity()
{
    bool plugin =  (xmldoc->docType == "Plugin");
    exportAPI->setEnabled(!plugin);
    exportInclude->setEnabled(!plugin);
}

// ****************************************************************************
//  Method:  XMLEditAttribute::BlockAllSignals
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
//    Brad Whitlock, Wed Mar 7 16:06:43 PST 2007
//    Added keyframe.
//
// ****************************************************************************
void
XMLEditAttribute::BlockAllSignals(bool block)
{
    name->blockSignals(block);
    purpose->blockSignals(block);
    codefile->blockSignals(block);
    exportAPI->blockSignals(block);
    exportInclude->blockSignals(block);
    persistent->blockSignals(block);
    keyframe->blockSignals(block);
}

// ----------------------------------------------------------------------------
//                                 Callbacks
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Method:  XMLEditAttribute::nameTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditAttribute::nameTextChanged(const QString &text)
{
    xmldoc->attribute->name = text;
}

// ****************************************************************************
//  Method:  XMLEditAttribute::purposeTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditAttribute::purposeTextChanged(const QString &text)
{
    xmldoc->attribute->purpose = text;
}

// ****************************************************************************
//  Method:  XMLEditAttribute::codefileTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Dec 13 16:16:43 PST 2005
//    Handle empty code files more gracefully.
//
// ****************************************************************************
void
XMLEditAttribute::codefileTextChanged(const QString &text)
{
    if (text.isEmpty())
    {
        xmldoc->attribute->codeFile = NULL;
        return;
    }
    QString file = text;
    QString path = FilePath(xmldoc->filename);
    if (!path.isEmpty())
        file = path + file;
    xmldoc->attribute->codeFile = new CodeFile(file);
}

// ****************************************************************************
//  Method:  XMLEditAttribute::exportAPITextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditAttribute::exportAPITextChanged(const QString &text)
{
    if (xmldoc->docType == "Plugin")
        return;

    xmldoc->attribute->exportAPI = text;
}

// ****************************************************************************
//  Method:  XMLEditAttribute::exportIncludeTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditAttribute::exportIncludeTextChanged(const QString &text)
{
    if (xmldoc->docType == "Plugin")
        return;

    xmldoc->attribute->exportInclude = text;
}

// ****************************************************************************
//  Method:  XMLEditAttribute::persistentChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditAttribute::persistentChanged()
{
    xmldoc->attribute->persistent = persistent->isChecked();    
}

// ****************************************************************************
//  Method:  XMLEditAttribute::keyframeChanged
//
//  Programmer:  Brad Whitlock
//  Creation:    Wed Mar 7 16:07:46 PST 2007
//
// ****************************************************************************
void
XMLEditAttribute::keyframeChanged()
{
    xmldoc->attribute->keyframe = keyframe->isChecked();    
}
