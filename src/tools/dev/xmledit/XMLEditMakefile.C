// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Kathleen Biagas, Thu Nov  6 11:16:20 PST 2014
//    Added support for DEFINES tag.
//
//    Kathleen Biagas, Wed May 4, 2022
//    Added support for component-specific DEFINES, CXXFLAGS, and LDFLAGS.
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

    topLayout->addWidget(new QLabel(tr("DEFINES"), this), row, 0);
    DEFINES = new QLineEdit(this);
    topLayout->addWidget(DEFINES, row, 1);
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

    topLayout->addWidget(new QLabel(tr("MDServer DEFINES"), this), row, 0);
    MDefines = new QLineEdit(this);
    topLayout->addWidget(MDefines, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("MDServer CXXFLAGS"), this), row, 0);
    MCXXFlags = new QLineEdit(this);
    topLayout->addWidget(MCXXFlags, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("MDServer LDFLAGS"), this), row, 0);
    MLDFlags = new QLineEdit(this);
    topLayout->addWidget(MLDFlags, row, 1);
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

    topLayout->addWidget(new QLabel(tr("Engine DEFINES (ser)"), this), row, 0);
    EDefinesSer = new QLineEdit(this);
    topLayout->addWidget(EDefinesSer, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("Engine CXXFLAGS (ser)"), this), row, 0);
    ECXXFlagsSer = new QLineEdit(this);
    topLayout->addWidget(ECXXFlagsSer, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("Engine LDFLAGS (ser)"), this), row, 0);
    ELDFlagsSer = new QLineEdit(this);
    topLayout->addWidget(ELDFlagsSer, row, 1);
    row++;

    customELibsSer = new QCheckBox(tr("Engine Libs (ser)"), this);
    ELibsSer = new QLineEdit(this);
    topLayout->addWidget(customELibsSer, row, 0);
    topLayout->addWidget(ELibsSer, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("Engine DEFINES (par)"), this), row, 0);
    EDefinesPar = new QLineEdit(this);
    topLayout->addWidget(EDefinesPar, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("Engine CXXFLAGS (par)"), this), row, 0);
    ECXXFlagsPar = new QLineEdit(this);
    topLayout->addWidget(ECXXFlagsPar, row, 1);
    row++;

    topLayout->addWidget(new QLabel(tr("Engine LDFLAGS (par)"), this), row, 0);
    ELDFlagsPar = new QLineEdit(this);
    topLayout->addWidget(ELDFlagsPar, row, 1);
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
    connect(DEFINES, SIGNAL(textChanged(const QString&)),
            this, SLOT(defsTextChanged(const QString&)));
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

    connect(MDefines, SIGNAL(textChanged(const QString&)),
            this, SLOT(mdefsTextChanged(const QString&)));
    connect(MCXXFlags, SIGNAL(textChanged(const QString&)),
            this, SLOT(mcxxflagsTextChanged(const QString&)));
    connect(MLDFlags, SIGNAL(textChanged(const QString&)),
            this, SLOT(mldflagsTextChanged(const QString&)));

    connect(EDefinesSer, SIGNAL(textChanged(const QString&)),
            this, SLOT(edefsSerTextChanged(const QString&)));
    connect(ECXXFlagsSer, SIGNAL(textChanged(const QString&)),
            this, SLOT(ecxxflagsSerTextChanged(const QString&)));
    connect(ELDFlagsSer, SIGNAL(textChanged(const QString&)),
            this, SLOT(eldflagsSerTextChanged(const QString&)));

    connect(EDefinesPar, SIGNAL(textChanged(const QString&)),
            this, SLOT(edefsParTextChanged(const QString&)));
    connect(ECXXFlagsPar, SIGNAL(textChanged(const QString&)),
            this, SLOT(ecxxflagsParTextChanged(const QString&)));
    connect(ELDFlagsPar, SIGNAL(textChanged(const QString&)),
            this, SLOT(eldflagsParTextChanged(const QString&)));

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
//    Kathleen Biagas, Thu Nov  6 11:16:59 PST 2014
//    Added support for DEFINES tag.
//
//    Kathleen Biagas, Wed May 4, 2022
//    Added support for component-specific DEFINES, CXXFLAGS, and LDFLAGS.
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
        DEFINES->setText(JoinValues(p->libs, ' '));
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

        MCXXFlags->setText(JoinValues(p->mcxxflags, ' '));
        MLDFlags->setText(JoinValues(p->mldflags, ' '));
        MDefines->setText(JoinValues(p->mdefs, ' '));

        ECXXFlagsSer->setText(JoinValues(p->ecxxflagsSer, ' '));
        ELDFlagsSer->setText(JoinValues(p->eldflagsSer, ' '));
        EDefinesSer->setText(JoinValues(p->edefsSer, ' '));

        ECXXFlagsPar->setText(JoinValues(p->ecxxflagsPar, ' '));
        ELDFlagsPar->setText(JoinValues(p->eldflagsPar, ' '));
        EDefinesPar->setText(JoinValues(p->edefsPar, ' '));
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
//    Kathleen Biagas, Thu Nov  6 11:16:59 PST 2014
//    Added support for DEFINES tag.
//
//    Kathleen Biagas, Wed May 4, 2022
//    Added support for component-specific DEFINES, CXXFLAGS, and LDFLAGS.
//
// ****************************************************************************
void
XMLEditMakefile::UpdateWindowSensitivity()
{
    bool plugin = (xmldoc->docType == "Plugin");

    CXXFLAGS->setEnabled(plugin);
    LDFLAGS->setEnabled(plugin);
    LIBS->setEnabled(plugin);
    DEFINES->setEnabled(plugin);
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

    MCXXFlags->setEnabled(plugin);
    MLDFlags->setEnabled(plugin);
    MDefines->setEnabled(plugin);

    ECXXFlagsSer->setEnabled(plugin);
    ELDFlagsSer->setEnabled(plugin);
    EDefinesSer->setEnabled(plugin);

    ECXXFlagsPar->setEnabled(plugin);
    ELDFlagsPar->setEnabled(plugin);
    EDefinesPar->setEnabled(plugin);
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
//    Kathleen Biagas, Thu Nov  6 11:16:59 PST 2014
//    Added support for DEFINES tag.
//
//    Kathleen Biagas, Wed May 4, 2022
//    Added support for component-specific DEFINES, CXXFLAGS, and LDFLAGS.
//
// ****************************************************************************
void
XMLEditMakefile::BlockAllSignals(bool block)
{
    CXXFLAGS->blockSignals(block);
    LDFLAGS->blockSignals(block);
    LIBS->blockSignals(block);
    DEFINES->blockSignals(block);
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

    MCXXFlags->blockSignals(block);
    MLDFlags->blockSignals(block);
    MDefines->blockSignals(block);

    ECXXFlagsSer->blockSignals(block);
    ELDFlagsSer->blockSignals(block);
    EDefinesSer->blockSignals(block);

    ECXXFlagsPar->blockSignals(block);
    ELDFlagsPar->blockSignals(block);
    EDefinesPar->blockSignals(block);
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
//  Method:  XMLEditMakefile::defsTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    November 6, 2014
//
// ****************************************************************************
void
XMLEditMakefile::defsTextChanged(const QString &text)
{
    xmldoc->plugin->defs = SplitValues(text);
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

// ****************************************************************************
//  Method:  XMLEditMakefile::mcxxflagsTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
// ****************************************************************************
void
XMLEditMakefile::mcxxflagsTextChanged(const QString &text)
{
    xmldoc->plugin->mcxxflags = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::mldflagsTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
// ****************************************************************************
void
XMLEditMakefile::mldflagsTextChanged(const QString &text)
{
    xmldoc->plugin->mldflags = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::mdefsTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
// ****************************************************************************
void
XMLEditMakefile::mdefsTextChanged(const QString &text)
{
    xmldoc->plugin->mdefs = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::ecxxflagsSerTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
// ****************************************************************************
void
XMLEditMakefile::ecxxflagsSerTextChanged(const QString &text)
{
    xmldoc->plugin->ecxxflagsSer = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::eldflagsSerTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
// ****************************************************************************
void
XMLEditMakefile::eldflagsSerTextChanged(const QString &text)
{
    xmldoc->plugin->eldflagsSer = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::edefsSerTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
// ****************************************************************************
void
XMLEditMakefile::edefsSerTextChanged(const QString &text)
{
    xmldoc->plugin->edefsSer = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::ecxxflagsParTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
// ****************************************************************************
void
XMLEditMakefile::ecxxflagsParTextChanged(const QString &text)
{
    xmldoc->plugin->ecxxflagsPar = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::eldflagsParTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
// ****************************************************************************
void
XMLEditMakefile::eldflagsParTextChanged(const QString &text)
{
    xmldoc->plugin->eldflagsPar = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditMakefile::edefsParTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
// ****************************************************************************
void
XMLEditMakefile::edefsParTextChanged(const QString &text)
{
    xmldoc->plugin->edefsPar = SplitValues(text);
}
