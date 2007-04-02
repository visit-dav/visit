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

#include "XMLEditMakefile.h"

#include <XMLDocument.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>

#include <Plugin.h>

// ****************************************************************************
//  Constructor:  XMLEditMakefile::XMLEditMakefile
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Sean Ahern, Fri Nov 15 15:25:23 PST 2002
//    Added "widget files" so we can have custom GUI elements.
//
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Brad Whitlock, Fri Feb 23 17:47:37 PST 2007
//    Added viewer widgets.
//
// ****************************************************************************
XMLEditMakefile::XMLEditMakefile(QWidget *p, const QString &n)
    : QFrame(p, n)
{
    QGridLayout *topLayout = new QGridLayout(this, 11,2, 5);
    int row = 0;

    topLayout->addWidget(new QLabel("CXXFLAGS", this), row, 0);
    CXXFLAGS = new QLineEdit(this);
    topLayout->addMultiCellWidget(CXXFLAGS, row,row, 1,2);
    row++;

    topLayout->addWidget(new QLabel("LDFLAGS", this), row, 0);
    LDFLAGS = new QLineEdit(this);
    topLayout->addMultiCellWidget(LDFLAGS, row,row, 1,2);
    row++;

    topLayout->addWidget(new QLabel("LIBS", this), row, 0);
    LIBS = new QLineEdit(this);
    topLayout->addMultiCellWidget(LIBS, row,row, 1,2);
    row++;

    customGFiles = new QCheckBox("GUI Files", this);
    GFiles = new QLineEdit(this);
    topLayout->addWidget(customGFiles, row, 0);
    topLayout->addMultiCellWidget(GFiles, row,row, 1,2);
    row++;

    customWFiles = new QCheckBox("GUI Widget Files", this);
    WFiles = new QLineEdit(this);
    topLayout->addWidget(customWFiles, row, 0);
    topLayout->addMultiCellWidget(WFiles, row,row, 1,2);
    row++;

    customSFiles = new QCheckBox("Scripting Files", this);
    SFiles = new QLineEdit(this);
    topLayout->addWidget(customSFiles, row, 0);
    topLayout->addMultiCellWidget(SFiles, row,row, 1,2);
    row++;

    customVFiles = new QCheckBox("Viewer Files", this);
    VFiles = new QLineEdit(this);
    topLayout->addWidget(customVFiles, row, 0);
    topLayout->addMultiCellWidget(VFiles, row,row, 1,2);
    row++;

    customVWFiles = new QCheckBox("Viewer Widget Files", this);
    VWFiles = new QLineEdit(this);
    topLayout->addWidget(customVWFiles, row, 0);
    topLayout->addMultiCellWidget(VWFiles, row,row, 1,2);
    row++;

    customMFiles = new QCheckBox("MDServer Files", this);
    MFiles = new QLineEdit(this);
    topLayout->addWidget(customMFiles, row, 0);
    topLayout->addMultiCellWidget(MFiles, row,row, 1,2);
    row++;

    customEFiles = new QCheckBox("Engine Files", this);
    EFiles = new QLineEdit(this);
    topLayout->addWidget(customEFiles, row, 0);
    topLayout->addMultiCellWidget(EFiles, row,row, 1,2);
    row++;

    mdSpecificCode = new QCheckBox("Plugin has code specific to the MDServer", this);
    topLayout->addMultiCellWidget(mdSpecificCode, row,row, 0,2);
    row++;

    topLayout->setRowStretch(row, 100);
    row++;

    connect(CXXFLAGS, SIGNAL(textChanged(const QString&)),
            this, SLOT(cxxflagsTextChanged(const QString&)));
    connect(LDFLAGS, SIGNAL(textChanged(const QString&)),
            this, SLOT(ldflagsTextChanged(const QString&)));
    connect(LIBS, SIGNAL(textChanged(const QString&)),
            this, SLOT(libsTextChanged(const QString&)));
   connect(GFiles, SIGNAL(textChanged(const QString&)),
            this, SLOT(gfilesTextChanged(const QString&)));
    connect(SFiles, SIGNAL(textChanged(const QString&)),
            this, SLOT(sfilesTextChanged(const QString&)));
    connect(VFiles, SIGNAL(textChanged(const QString&)),
            this, SLOT(vfilesTextChanged(const QString&)));
    connect(MFiles, SIGNAL(textChanged(const QString&)),
            this, SLOT(mfilesTextChanged(const QString&)));
    connect(EFiles, SIGNAL(textChanged(const QString&)),
            this, SLOT(efilesTextChanged(const QString&)));
    connect(WFiles, SIGNAL(textChanged(const QString&)),
            this, SLOT(wfilesTextChanged(const QString&)));
    connect(VWFiles, SIGNAL(textChanged(const QString&)),
            this, SLOT(vwfilesTextChanged(const QString&)));
    connect(customGFiles, SIGNAL(clicked()),
            this, SLOT(customgfilesChanged()));
    connect(customSFiles, SIGNAL(clicked()),
            this, SLOT(customsfilesChanged()));
    connect(customVFiles, SIGNAL(clicked()),
            this, SLOT(customvfilesChanged()));
    connect(customMFiles, SIGNAL(clicked()),
            this, SLOT(custommfilesChanged()));
    connect(customEFiles, SIGNAL(clicked()),
            this, SLOT(customefilesChanged()));
    connect(customWFiles, SIGNAL(clicked()),
            this, SLOT(customwfilesChanged()));
    connect(customVWFiles, SIGNAL(clicked()),
            this, SLOT(customvwfilesChanged()));
    connect(mdSpecificCode, SIGNAL(clicked()),
            this, SLOT(mdSpecificCodeChanged()));
}

// ****************************************************************************
//  Method:  XMLEditMakefile::UpdateWindowContents
//
//  Purpose:
//    Update the window based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Sean Ahern, Fri Nov 15 15:25:23 PST 2002
//    Added "widget files" so we can have custom GUI elements.
//
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Brad Whitlock, Fri Feb 23 17:49:59 PST 2007
//    Added viewer widgets.
//
// ****************************************************************************
void
XMLEditMakefile::UpdateWindowContents()
{
    BlockAllSignals(true);

    if (xmldoc->docType == "Plugin")
    {
        Plugin *p = xmldoc->plugin;
        CXXFLAGS->setText(JoinValues(p->cxxflags, ' '));
        LDFLAGS->setText(JoinValues(p->ldflags, ' '));
        LIBS->setText(JoinValues(p->libs, ' '));
        if (p->customgfiles)
            GFiles->setText(JoinValues(p->gfiles, ' '));
        else
            GFiles->setText(JoinValues(p->defaultgfiles, ' '));
        customGFiles->setChecked(p->customgfiles);
        if (p->customsfiles)
            SFiles->setText(JoinValues(p->sfiles, ' '));
        else
            SFiles->setText(JoinValues(p->defaultsfiles, ' '));
        customSFiles->setChecked(p->customsfiles);
        if (p->customvfiles)
            VFiles->setText(JoinValues(p->vfiles, ' '));
        else
            VFiles->setText(JoinValues(p->defaultvfiles, ' '));
        customVFiles->setChecked(p->customvfiles);
        if (p->custommfiles)
            MFiles->setText(JoinValues(p->mfiles, ' '));
        else
            MFiles->setText(JoinValues(p->defaultmfiles, ' '));
        customMFiles->setChecked(p->custommfiles);
        if (p->customefiles)
            EFiles->setText(JoinValues(p->efiles, ' '));
        else
            EFiles->setText(JoinValues(p->defaultefiles, ' '));
        customEFiles->setChecked(p->customefiles);
        if (p->customwfiles)
            WFiles->setText(JoinValues(p->wfiles, ' '));
        else
            WFiles->setText(JoinValues(p->defaultwfiles, ' '));
        customWFiles->setChecked(p->customwfiles);
        if (p->customvwfiles)
            VWFiles->setText(JoinValues(p->vwfiles, ' '));
        else
            VWFiles->setText("");
        customVWFiles->setChecked(p->customvwfiles);
        mdSpecificCode->setChecked(p->has_MDS_specific_code);
    }

    UpdateWindowSensitivity();

    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::UpdateWindowSensitivity
//
//  Purpose:
//    Enable/disable widget sensitivity based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Sean Ahern, Fri Nov 15 15:25:23 PST 2002
//    Added "widget files" so we can have custom GUI elements.
//
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Brad Whitlock, Fri Feb 23 17:52:42 PST 2007
//    Added viewer widgets.
//
// ****************************************************************************
void
XMLEditMakefile::UpdateWindowSensitivity()
{
    bool plugin = (xmldoc->docType == "Plugin");

    CXXFLAGS->setEnabled(plugin);
    LDFLAGS->setEnabled(plugin);
    LIBS->setEnabled(plugin);
    GFiles->setEnabled(plugin && xmldoc->plugin->customgfiles);
    customGFiles->setEnabled(plugin);
    SFiles->setEnabled(plugin && xmldoc->plugin->customsfiles);
    customSFiles->setEnabled(plugin);
    VFiles->setEnabled(plugin && xmldoc->plugin->customvfiles);
    customVFiles->setEnabled(plugin);
    MFiles->setEnabled(plugin && xmldoc->plugin->custommfiles);
    customMFiles->setEnabled(plugin);
    EFiles->setEnabled(plugin && xmldoc->plugin->customefiles);
    customEFiles->setEnabled(plugin);
    WFiles->setEnabled(plugin && xmldoc->plugin->customwfiles);
    customWFiles->setEnabled(plugin);
    VWFiles->setEnabled(plugin && xmldoc->plugin->customvwfiles);
    customVWFiles->setEnabled(plugin);
    mdSpecificCode->setEnabled(plugin);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::BlockAllSignals
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
//    Sean Ahern, Fri Nov 15 15:25:23 PST 2002
//    Added "widget files" so we can have custom GUI elements.
//
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Brad Whitlock, Fri Feb 23 17:53:03 PST 2007
//    Added viewer widgets.
//
// ****************************************************************************
void
XMLEditMakefile::BlockAllSignals(bool block)
{
    CXXFLAGS->blockSignals(block);
    LDFLAGS->blockSignals(block);
    LIBS->blockSignals(block);
    GFiles->blockSignals(block);
    customGFiles->blockSignals(block);
    SFiles->blockSignals(block);
    customSFiles->blockSignals(block);
    VFiles->blockSignals(block);
    customVFiles->blockSignals(block);
    MFiles->blockSignals(block);
    customMFiles->blockSignals(block);
    EFiles->blockSignals(block);
    customEFiles->blockSignals(block);
    WFiles->blockSignals(block);
    customWFiles->blockSignals(block);
    VWFiles->blockSignals(block);
    customVWFiles->blockSignals(block);
    mdSpecificCode->blockSignals(block);
}

// ----------------------------------------------------------------------------
//                                 Callbacks
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Method:  XMLEditMakefile::cxxflagsTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditMakefile::cxxflagsTextChanged(const QString &text)
{
    xmldoc->plugin->cxxflags = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::ldflagsTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditMakefile::ldflagsTextChanged(const QString &text)
{
    xmldoc->plugin->ldflags = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::libsTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditMakefile::libsTextChanged(const QString &text)
{
    xmldoc->plugin->libs = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::gfilesTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditMakefile::gfilesTextChanged(const QString &text)
{
    xmldoc->plugin->gfiles = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::sfilesTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditMakefile::sfilesTextChanged(const QString &text)
{
    xmldoc->plugin->sfiles = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::vfilesTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditMakefile::vfilesTextChanged(const QString &text)
{
    xmldoc->plugin->vfiles = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::mfilesTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditMakefile::mfilesTextChanged(const QString &text)
{
    xmldoc->plugin->mfiles = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::efilesTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditMakefile::efilesTextChanged(const QString &text)
{
    xmldoc->plugin->efiles = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::wfilesTextChanged
//
//  Programmer:  Sean Ahern
//  Creation:    Mon Nov 18 13:55:18 PST 2002
//
// ****************************************************************************
void
XMLEditMakefile::wfilesTextChanged(const QString &text)
{
    xmldoc->plugin->wfiles = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::vwfilesTextChanged
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Feb 23 17:53:33 PST 2007
//
// ****************************************************************************
void
XMLEditMakefile::vwfilesTextChanged(const QString &text)
{
    xmldoc->plugin->vwfiles = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::customgfilesChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditMakefile::customgfilesChanged()
{
    xmldoc->plugin->customgfiles = customGFiles->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditMakefile::customsfilesChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditMakefile::customsfilesChanged()
{
    xmldoc->plugin->customsfiles = customSFiles->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditMakefile::customvfilesChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditMakefile::customvfilesChanged()
{
    xmldoc->plugin->customvfiles = customVFiles->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditMakefile::custommfilesChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditMakefile::custommfilesChanged()
{
    xmldoc->plugin->custommfiles = customMFiles->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditMakefile::customefilesChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditMakefile::customefilesChanged()
{
    xmldoc->plugin->customefiles = customEFiles->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditMakefile::customwfilesChanged
//
//  Programmer:  Sean Ahern
//  Creation:    Fri Nov 15 15:29:19 PST 2002
//
// ****************************************************************************
void
XMLEditMakefile::customwfilesChanged()
{
    xmldoc->plugin->customwfiles = customWFiles->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditMakefile::customvwfilesChanged
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Feb 23 17:54:08 PST 2007
//
// ****************************************************************************
void
XMLEditMakefile::customvwfilesChanged()
{
    xmldoc->plugin->customvwfiles = customVWFiles->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditMakefile::mdSpecificCodeChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    July  7, 2004
//
// ****************************************************************************
void
XMLEditMakefile::mdSpecificCodeChanged()
{
    xmldoc->plugin->has_MDS_specific_code = mdSpecificCode->isChecked();
    UpdateWindowContents();
}
