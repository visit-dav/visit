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
#include "XMLEditAttribute.h"

#include <XMLDocument.h>
#include <Attribute.h>
#include <QLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>

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
//   Brad Whitlock, Fri Mar 7 11:20:58 PDT 2008
//   Changed layout.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
//    Mark C. Miller, Wed Aug 26 11:03:19 PDT 2009
//    Added support for custom base class for derived state objects.
// ****************************************************************************

XMLEditAttribute::XMLEditAttribute(QWidget *p)
    : QFrame(p)
{
    QGridLayout *topLayout = new QGridLayout(this);
    int row = 0;

    name = new QLineEdit(this);
    topLayout->addWidget(new QLabel("Name", this), row, 0);
    topLayout->addWidget(name, row,1);
    row++;

    purpose = new QLineEdit(this);
    topLayout->addWidget(new QLabel("Purpose", this), row, 0);
    topLayout->addWidget(purpose, row,1);
    row++;

    codefile = new QLineEdit(this);
    topLayout->addWidget(new QLabel("Code file", this), row, 0);
    topLayout->addWidget(codefile, row,1);
    row++;

    exportAPI = new QLineEdit(this);
    topLayout->addWidget(new QLabel("Export API", this), row, 0);
    topLayout->addWidget(exportAPI, row,1);
    row++;

    exportInclude = new QLineEdit(this);
    topLayout->addWidget(new QLabel("Export include", this), row, 0);
    topLayout->addWidget(exportInclude, row,1);
    row++;

    persistent = new QCheckBox("Persistent", this);
    topLayout->addWidget(persistent, row,0);
    row++;

    keyframe = new QCheckBox("Generate keyframing methods", this);
    topLayout->addWidget(keyframe, row,0, 1,2);
    row++;

    customBaseClass = new QCheckBox(tr("Base Class"), this);
    baseClass = new QLineEdit(this);
    topLayout->addWidget(customBaseClass, row, 0);
    topLayout->addWidget(baseClass, row, 1);
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

    connect(customBaseClass, SIGNAL(clicked()),
            this, SLOT(customBaseClassChanged()));
    connect(baseClass, SIGNAL(textChanged(const QString &)),
            this,  SLOT(baseClassTextChanged(const QString &)));
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
//    Brad Whitlock, Thu Mar 6 14:51:29 PST 2008
//    Adapted to newer CodeFile implementation.
//
//    Mark C. Miller, Wed Aug 26 11:03:19 PDT 2009
//    Added support for custom base class for derived state objects.
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
        codefile->setText(a->codeFile->FileBase());
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

    customBaseClass->setChecked(a->custombase);
    if (customBaseClass->isChecked())
        baseClass->setText(a->baseClass.isNull()?"AttributeSubject":a->baseClass);
    else
        baseClass->setText("AttributeSubject");

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
//  Modifications:
//
//    Mark C. Miller, Wed Aug 26 11:03:19 PDT 2009
//    Added support for custom base class for derived state objects.
// ****************************************************************************
void
XMLEditAttribute::UpdateWindowSensitivity()
{
    bool plugin =  (xmldoc->docType == "Plugin");
    exportAPI->setEnabled(!plugin);
    exportInclude->setEnabled(!plugin);
    customBaseClass->setEnabled(!plugin);
    baseClass->setEnabled(!plugin && customBaseClass->isChecked());
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
    customBaseClass->blockSignals(block);
    baseClass->blockSignals(block);
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

// ****************************************************************************
//  Method:  XMLEditAttribute::baseClassChanged
//
//  Programmer:  Mark C. Miller
//  Creation:    Mon Aug 10 17:06:18 PDT 2009
//
// ****************************************************************************
void
XMLEditAttribute::customBaseClassChanged()
{
    xmldoc->attribute->custombase = customBaseClass->isChecked();    
    if (customBaseClass->isChecked())
    {
        baseClass->blockSignals(true);
        baseClass->setEnabled(true);
        baseClass->blockSignals(false);
#if 0
        if (xmldoc->attribute->baseClass.isNull())
            baseClass->setText("AttributeSubject");
        else if (xmldoc->attribute->baseClass == "AttributeSubject")
        {
            customBaseClass->blockSignals(true);
            customBaseClass->setChecked(false);
            customBaseClass->blockSignals(false);
        }
        else
        {
            baseClass->blockSignals(true);
            baseClass->setText(xmldoc->attribute->baseClass);
            baseClass->blockSignals(false);
        }
#endif
    }
    else
    {
        baseClass->blockSignals(true);
        baseClass->setEnabled(false);
        //baseClass->setText("AttributeSubject");
        baseClass->blockSignals(false);
    }
}

// ****************************************************************************
//  Method:  XMLEditAttribute::baseClassTextChanged
//
//  Programmer:  Mark C. Miller
//  Creation:    Mon Aug 10 17:19:13 PDT 2009
//
// ****************************************************************************
void
XMLEditAttribute::baseClassTextChanged(const QString &text)
{
    if (xmldoc->docType == "Plugin")
        return;

    xmldoc->attribute->baseClass = text;
}

