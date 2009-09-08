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
#include "XMLEditMakefile.h"

#include <XMLDocument.h>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QRadioButton>

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
//    Cyrus Harrison, Wed Mar  7 09:03:38 PST 2007
//    Allow for engine-specific code in a plugin's source files.
//
//    Brad Whitlock, Fri Mar 7 10:02:36 PDT 2008
//    Changed layout.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
//    Cyrus Harrison, Fri Sep 19 13:58:39 PDT 2008
//    Added support for custom libs for gui,engine,mdserver, and viewer libs
//
// ****************************************************************************

XMLEditMakefile::XMLEditMakefile(QWidget *p)
    : QFrame(p)
{
    QGridLayout *topLayout = new QGridLayout(this);
    
    int row = 0;

    topLayout->addWidget(new QLabel(tr("CXXFLAGS"), this), row, 0);
    CXXFLAGS = new QLineEdit(this);
    topLayout->addWidget(CXXFLAGS, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("LDFLAGS"), this), row, 0);
    LDFLAGS = new QLineEdit(this);
    topLayout->addWidget(LDFLAGS, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("LIBS"), this), row, 0);
    LIBS = new QLineEdit(this);
    topLayout->addWidget(LIBS, row, 1);
    row++;

    customGFiles = new QCheckBox(tr("GUI Files"), this);
    GFiles = new QLineEdit(this);
    topLayout->addWidget(customGFiles, row, 0);
    topLayout->addWidget(GFiles, row, 1);
    row++;

    customGLibs = new QCheckBox(tr("GUI Libs"), this);
    GLibs = new QLineEdit(this);
    topLayout->addWidget(customGLibs, row, 0);
    topLayout->addWidget(GLibs, row, 1);
    row++;
    
    customWFiles = new QCheckBox(tr("GUI Widget Files"), this);
    WFiles = new QLineEdit(this);
    topLayout->addWidget(customWFiles, row, 0);
    topLayout->addWidget(WFiles, row,1);
    row++;

    customSFiles = new QCheckBox(tr("Scripting Files"), this);
    SFiles = new QLineEdit(this);
    topLayout->addWidget(customSFiles, row, 0);
    topLayout->addWidget(SFiles, row,1);
    row++;

    customVFiles = new QCheckBox(tr("Viewer Files"), this);
    VFiles = new QLineEdit(this);
    topLayout->addWidget(customVFiles, row, 0);
    topLayout->addWidget(VFiles, row,1);
    row++;
    
    customVLibs = new QCheckBox(tr("Viewer Libs"), this);
    VLibs = new QLineEdit(this);
    topLayout->addWidget(customVLibs, row, 0);
    topLayout->addWidget(VLibs, row, 1);
    row++;

    customVWFiles = new QCheckBox(tr("Viewer Widget Files"), this);
    VWFiles = new QLineEdit(this);
    topLayout->addWidget(customVWFiles, row, 0);
    topLayout->addWidget(VWFiles, row,1);
    row++;

    customMFiles = new QCheckBox(tr("MDServer Files"), this);
    MFiles = new QLineEdit(this);
    topLayout->addWidget(customMFiles, row, 0);
    topLayout->addWidget(MFiles, row,1);
    row++;

    customMLibs = new QCheckBox(tr("MDServer Libs"), this);
    MLibs = new QLineEdit(this);
    topLayout->addWidget(customMLibs, row, 0);
    topLayout->addWidget(MLibs, row, 1);
    row++;
    
    customEFiles = new QCheckBox(tr("Engine Files"), this);
    EFiles = new QLineEdit(this);
    topLayout->addWidget(customEFiles, row, 0);
    topLayout->addWidget(EFiles, row,1);
    row++;
    
    customELibsSer = new QCheckBox(tr("Engine Libs (ser)"), this);
    ELibsSer = new QLineEdit(this);
    topLayout->addWidget(customELibsSer, row, 0);
    topLayout->addWidget(ELibsSer, row, 1);
    row++;

    customELibsPar = new QCheckBox(tr("Engine Libs (par)"), this);
    ELibsPar = new QLineEdit(this);
    topLayout->addWidget(customELibsPar, row, 0);
    topLayout->addWidget(ELibsPar, row, 1);
    row++;

    engSpecificCode = new QCheckBox(tr("Plugin has code specific to the Engine"),
                                    this);
    topLayout->addWidget(engSpecificCode, row,0, 1,2);
    row++;

    mdSpecificCode = new QCheckBox(tr("Plugin has code specific to the MDServer "
                                   "(Database Plugins only)"), this);
    topLayout->addWidget(mdSpecificCode, row,0, 1,2);
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
    connect(GLibs, SIGNAL(textChanged(const QString&)),
            this, SLOT(glibsTextChanged(const QString&)));
    connect(SFiles, SIGNAL(textChanged(const QString&)),
            this, SLOT(sfilesTextChanged(const QString&)));
    connect(VFiles, SIGNAL(textChanged(const QString&)),
            this, SLOT(vfilesTextChanged(const QString&)));
    connect(VLibs, SIGNAL(textChanged(const QString&)),
            this, SLOT(vlibsTextChanged(const QString&)));
    connect(MFiles, SIGNAL(textChanged(const QString&)),
            this, SLOT(mfilesTextChanged(const QString&)));
    connect(MLibs, SIGNAL(textChanged(const QString&)),
            this, SLOT(mlibsTextChanged(const QString&)));
    connect(EFiles, SIGNAL(textChanged(const QString&)),
            this, SLOT(efilesTextChanged(const QString&)));
    connect(ELibsSer, SIGNAL(textChanged(const QString&)),
            this, SLOT(elibsSerTextChanged(const QString&)));
    connect(ELibsPar, SIGNAL(textChanged(const QString&)),
            this, SLOT(elibsParTextChanged(const QString&)));
    connect(WFiles, SIGNAL(textChanged(const QString&)),
            this, SLOT(wfilesTextChanged(const QString&)));
    connect(VWFiles, SIGNAL(textChanged(const QString&)),
            this, SLOT(vwfilesTextChanged(const QString&)));
    connect(customGFiles, SIGNAL(clicked()),
            this, SLOT(customgfilesChanged()));
    connect(customGLibs, SIGNAL(clicked()),
            this, SLOT(customglibsChanged()));
    connect(customSFiles, SIGNAL(clicked()),
            this, SLOT(customsfilesChanged()));
    connect(customVFiles, SIGNAL(clicked()),
            this, SLOT(customvfilesChanged()));
    connect(customVLibs, SIGNAL(clicked()),
            this, SLOT(customvlibsChanged()));
    connect(customMFiles, SIGNAL(clicked()),
            this, SLOT(custommfilesChanged()));
    connect(customMLibs, SIGNAL(clicked()),
            this, SLOT(custommlibsChanged()));
    connect(customEFiles, SIGNAL(clicked()),
            this, SLOT(customefilesChanged()));
    connect(customELibsSer, SIGNAL(clicked()),
            this, SLOT(customelibsSerChanged()));                            
    connect(customELibsPar, SIGNAL(clicked()),
            this, SLOT(customelibsParChanged()));                            
    connect(customWFiles, SIGNAL(clicked()),
            this, SLOT(customwfilesChanged()));
    connect(customVWFiles, SIGNAL(clicked()),
            this, SLOT(customvwfilesChanged()));
    connect(mdSpecificCode, SIGNAL(clicked()),
            this, SLOT(mdSpecificCodeChanged()));
    connect(engSpecificCode, SIGNAL(clicked()),
            this, SLOT(engSpecificCodeChanged()));
    
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
//    Cyrus Harrison, Wed Mar  7 09:07:37 PST 2007
//    Allow for engine-specific code in a plugin's source files.
//
//    Cyrus Harrison, Fri Sep 19 13:58:39 PDT 2008
//    Added support for custom libs for gui,engine,mdserver, and viewer libs.
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
        // gui
        if (p->customgfiles)
            GFiles->setText(JoinValues(p->gfiles, ' '));
        else
            GFiles->setText(JoinValues(p->defaultgfiles, ' '));
        customGFiles->setChecked(p->customgfiles);
        // scripting
        if (p->customglibs)
            GLibs->setText(JoinValues(p->glibs, ' '));
        customGLibs->setChecked(p->customglibs);
        if (p->customsfiles)
            SFiles->setText(JoinValues(p->sfiles, ' '));
        else
            SFiles->setText(JoinValues(p->defaultsfiles, ' '));
        customSFiles->setChecked(p->customsfiles);
        // viewer
        if (p->customvfiles)
            VFiles->setText(JoinValues(p->vfiles, ' '));
        else
            VFiles->setText(JoinValues(p->defaultvfiles, ' '));
        customVFiles->setChecked(p->customvfiles);
        if (p->customvlibs)
            VLibs->setText(JoinValues(p->vlibs, ' '));
        customVLibs->setChecked(p->customvlibs);
        // mdserver
        if (p->custommfiles)
            MFiles->setText(JoinValues(p->mfiles, ' '));
        else
            MFiles->setText(JoinValues(p->defaultmfiles, ' '));
        customMFiles->setChecked(p->custommfiles);
        if (p->custommlibs)
            MLibs->setText(JoinValues(p->mlibs, ' '));
        customMLibs->setChecked(p->custommlibs);
        // engine
        if (p->customefiles)
            EFiles->setText(JoinValues(p->efiles, ' '));
        else
            EFiles->setText(JoinValues(p->defaultefiles, ' '));
        customEFiles->setChecked(p->customefiles);
        if (p->customelibsSer)
            ELibsSer->setText(JoinValues(p->elibsSer, ' '));
        customELibsSer->setChecked(p->customelibsSer);
        if (p->customelibsPar)
            ELibsPar->setText(JoinValues(p->elibsPar, ' '));
        customELibsPar->setChecked(p->customelibsPar);
        // widgets
        if (p->customwfiles)
            WFiles->setText(JoinValues(p->wfiles, ' '));
        else
            WFiles->setText(JoinValues(p->defaultwfiles, ' '));
        customWFiles->setChecked(p->customwfiles);
        // viewer widgets
        if (p->customvwfiles)
            VWFiles->setText(JoinValues(p->vwfiles, ' '));
        else
            VWFiles->setText("");
        customVWFiles->setChecked(p->customvwfiles);
        // engine spec code
        engSpecificCode->setChecked(p->hasEngineSpecificCode);
        // only allow mdserver-specific code if this is a database plugin
        if(xmldoc->plugin->type !="database")
            p->has_MDS_specific_code=false;

        mdSpecificCode->setChecked(p->has_MDS_specific_code);
    }
    else
    {
        mdSpecificCode->setChecked(false);
        engSpecificCode->setChecked(false);
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
//    Cyrus Harrison, Wed Mar  7 09:07:37 PST 2007
//    Allow for engine-specific code in a plugin's source files.
//
//    Cyrus Harrison, Fri Sep 19 13:58:39 PDT 2008
//    Added support for custom libs for gui,engine,mdserver, and viewer libs.
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
    GLibs->setEnabled(plugin && xmldoc->plugin->customglibs);
    customGLibs->setEnabled(plugin);
    SFiles->setEnabled(plugin && xmldoc->plugin->customsfiles);
    customSFiles->setEnabled(plugin);
    VFiles->setEnabled(plugin && xmldoc->plugin->customvfiles);
    customVFiles->setEnabled(plugin);
    VLibs->setEnabled(plugin && xmldoc->plugin->customvlibs);
    customVLibs->setEnabled(plugin);
    MFiles->setEnabled(plugin && xmldoc->plugin->custommfiles);
    customMFiles->setEnabled(plugin);
    MLibs->setEnabled(plugin && xmldoc->plugin->custommlibs);
    customMLibs->setEnabled(plugin);
    EFiles->setEnabled(plugin && xmldoc->plugin->customefiles);
    customEFiles->setEnabled(plugin);
    ELibsSer->setEnabled(plugin && xmldoc->plugin->customelibsSer);
    customELibsSer->setEnabled(plugin);
    ELibsPar->setEnabled(plugin && xmldoc->plugin->customelibsPar);
    customELibsPar->setEnabled(plugin);
    WFiles->setEnabled(plugin && xmldoc->plugin->customwfiles);
    customWFiles->setEnabled(plugin);
    VWFiles->setEnabled(plugin && xmldoc->plugin->customvwfiles);
    customVWFiles->setEnabled(plugin);
    engSpecificCode->setEnabled(plugin);
    // only enable for a database plugin
    mdSpecificCode->setEnabled(plugin && xmldoc->plugin->type =="database");
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
//    Cyrus Harrison, Wed Mar  7 09:07:37 PST 2007
//    Allow for engine-specific code in a plugin's source files.
//
//    Cyrus Harrison, Fri Sep 19 13:58:39 PDT 2008
//    Added support for custom libs for gui,engine,mdserver, and viewer libs.
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
    GLibs->blockSignals(block);
    customGLibs->blockSignals(block);
    SFiles->blockSignals(block);
    customSFiles->blockSignals(block);
    VFiles->blockSignals(block);
    VLibs->blockSignals(block);
    customVLibs->blockSignals(block);
    customVFiles->blockSignals(block);
    MFiles->blockSignals(block);
    customMFiles->blockSignals(block);
    MLibs->blockSignals(block);
    customMLibs->blockSignals(block);
    EFiles->blockSignals(block);
    customEFiles->blockSignals(block);
    ELibsSer->blockSignals(block);
    customELibsSer->blockSignals(block);
    ELibsPar->blockSignals(block);
    customELibsPar->blockSignals(block);
    WFiles->blockSignals(block);
    customWFiles->blockSignals(block);
    VWFiles->blockSignals(block);
    customVWFiles->blockSignals(block);
    mdSpecificCode->blockSignals(block);
    engSpecificCode->blockSignals(block);
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
//  Method:  XMLEditMakefile::glibsTextChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
// ****************************************************************************
void
XMLEditMakefile::glibsTextChanged(const QString &text)
{
    xmldoc->plugin->glibs = SplitValues(text);
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
//  Method:  XMLEditMakefile::vlibsTextChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
// ****************************************************************************
void
XMLEditMakefile::vlibsTextChanged(const QString &text)
{
    xmldoc->plugin->vlibs = SplitValues(text);
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
//  Method:  XMLEditMakefile::mlibsTextChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
// ****************************************************************************
void
XMLEditMakefile::mlibsTextChanged(const QString &text)
{
    xmldoc->plugin->mlibs = SplitValues(text);
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
//  Method:  XMLEditMakefile::elibsSerTextChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Sep  8 14:57:13 EDT 2009
//    Split into ser/par versions.
//
// ****************************************************************************
void
XMLEditMakefile::elibsSerTextChanged(const QString &text)
{
    xmldoc->plugin->elibsSer = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::elibsParTextChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Sep  8 14:57:13 EDT 2009
//    Split into ser/par versions.
//
// ****************************************************************************
void
XMLEditMakefile::elibsParTextChanged(const QString &text)
{
    xmldoc->plugin->elibsPar = SplitValues(text);
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
//  Method:  XMLEditMakefile::customglibsChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
// ****************************************************************************
void
XMLEditMakefile::customglibsChanged()
{
    xmldoc->plugin->customglibs = customGLibs->isChecked();
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
//  Method:  XMLEditMakefile::customvlibsChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
// ****************************************************************************
void
XMLEditMakefile::customvlibsChanged()
{
    xmldoc->plugin->customvlibs = customVLibs->isChecked();
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
//  Method:  XMLEditMakefile::custommlibsChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
// ****************************************************************************
void
XMLEditMakefile::custommlibsChanged()
{
    xmldoc->plugin->custommlibs = customMLibs->isChecked();
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
//  Method:  XMLEditMakefile::customelibsSerChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Sep  8 14:57:13 EDT 2009
//    Split into ser/par versions.
//
// ****************************************************************************
void
XMLEditMakefile::customelibsSerChanged()
{
    xmldoc->plugin->customelibsSer = customELibsSer->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditMakefile::customelibsParChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Sep  8 14:57:13 EDT 2009
//    Split into ser/par versions.
//
// ****************************************************************************
void
XMLEditMakefile::customelibsParChanged()
{
    xmldoc->plugin->customelibsPar = customELibsPar->isChecked();
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


// ****************************************************************************
//  Method:  XMLEditMakefile::engSpecificCodeChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    March  7, 2007
//
// ****************************************************************************
void
XMLEditMakefile::engSpecificCodeChanged()
{
    xmldoc->plugin->hasEngineSpecificCode = engSpecificCode->isChecked();
    UpdateWindowContents();
}

