// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "XMLEditStd.h"
#include "XMLEditCMake.h"

#include <XMLDocument.h>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>

#include <Plugin.h>

// ****************************************************************************
//  Constructor:  XMLEditCMake::XMLEditCMake
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
//    Kathleen Biagas, Fri Mar 21, 2025
//    Change QLineEdit connections from 'textChanged' to 'editingFinished.'
//
// ****************************************************************************

XMLEditCMake::XMLEditCMake(QWidget *p)
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

    connect(CXXFLAGS, SIGNAL(editingFinished()),
            this, SLOT(cxxflagsTextChanged()));
    connect(LDFLAGS, SIGNAL(editingFinished()),
            this, SLOT(ldflagsTextChanged()));
    connect(LIBS, SIGNAL(editingFinished()),
            this, SLOT(libsTextChanged()));
    connect(DEFINES, SIGNAL(editingFinished()),
            this, SLOT(defsTextChanged()));
    connect(GFiles, SIGNAL(editingFinished()),
            this, SLOT(gfilesTextChanged()));
    connect(GLibs, SIGNAL(editingFinished()),
            this, SLOT(glibsTextChanged()));
    connect(SFiles, SIGNAL(editingFinished()),
            this, SLOT(sfilesTextChanged()));
    connect(VFiles, SIGNAL(editingFinished()),
            this, SLOT(vfilesTextChanged()));
    connect(VLibs, SIGNAL(editingFinished()),
            this, SLOT(vlibsTextChanged()));
    connect(MFiles, SIGNAL(editingFinished()),
            this, SLOT(mfilesTextChanged()));
    connect(MLibs, SIGNAL(editingFinished()),
            this, SLOT(mlibsTextChanged()));
    connect(EFiles, SIGNAL(editingFinished()),
            this, SLOT(efilesTextChanged()));
    connect(ELibsSer, SIGNAL(editingFinished()),
            this, SLOT(elibsSerTextChanged()));
    connect(ELibsPar, SIGNAL(editingFinished()),
            this, SLOT(elibsParTextChanged()));
    connect(WFiles, SIGNAL(editingFinished()),
            this, SLOT(wfilesTextChanged()));
    connect(VWFiles, SIGNAL(editingFinished()),
            this, SLOT(vwfilesTextChanged()));
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

    connect(MDefines, SIGNAL(editingFinished()),
            this, SLOT(mdefsTextChanged()));
    connect(MCXXFlags, SIGNAL(editingFinished()),
            this, SLOT(mcxxflagsTextChanged()));
    connect(MLDFlags, SIGNAL(editingFinished()),
            this, SLOT(mldflagsTextChanged()));

    connect(EDefinesSer, SIGNAL(editingFinished()),
            this, SLOT(edefsSerTextChanged()));
    connect(ECXXFlagsSer, SIGNAL(editingFinished()),
            this, SLOT(ecxxflagsSerTextChanged()));
    connect(ELDFlagsSer, SIGNAL(editingFinished()),
            this, SLOT(eldflagsSerTextChanged()));

    connect(EDefinesPar, SIGNAL(editingFinished()),
            this, SLOT(edefsParTextChanged()));
    connect(ECXXFlagsPar, SIGNAL(editingFinished()),
            this, SLOT(ecxxflagsParTextChanged()));
    connect(ELDFlagsPar, SIGNAL(editingFinished()),
            this, SLOT(eldflagsParTextChanged()));

}

// ****************************************************************************
//  Method:  XMLEditCMake::UpdateWindowContents
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
//    Kathleen Biagas, Thu Oct 23, 2025 
//    Set text of Defines from correct plugin variable. 
//
// ****************************************************************************
void
XMLEditCMake::UpdateWindowContents()
{
    BlockAllSignals(true);

    if (xmldoc->docType == "Plugin")
    {
        Plugin *p = xmldoc->plugin;
        CXXFLAGS->setText(JoinValues(p->cxxflags, ' '));
        LDFLAGS->setText(JoinValues(p->ldflags, ' '));
        LIBS->setText(JoinValues(p->libs, ' '));
        DEFINES->setText(JoinValues(p->defs, ' '));
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
//  Method:  XMLEditCMake::UpdateWindowSensitivity
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
//    Kathleen Biagas, Thu Oct 23, 2025
//    Disable Gui and Viewer widgets if plugin type isn't Database.
//    Only enable MDServer widgets if pluging type is Database.
//
// ****************************************************************************
void
XMLEditCMake::UpdateWindowSensitivity()
{
    bool plugin = (xmldoc->docType == "Plugin");
    bool isDB = plugin && xmldoc->plugin->type == "database";

    CXXFLAGS->setEnabled(plugin);
    LDFLAGS->setEnabled(plugin);
    LIBS->setEnabled(plugin);
    DEFINES->setEnabled(plugin);
    GFiles->setEnabled(plugin && !isDB && xmldoc->plugin->customgfiles);
    customGFiles->setEnabled(plugin && !isDB);
    GLibs->setEnabled(plugin && !isDB  && xmldoc->plugin->customglibs);
    customGLibs->setEnabled(plugin && !isDB);
    SFiles->setEnabled(plugin && xmldoc->plugin->customsfiles);
    customSFiles->setEnabled(plugin);
    VFiles->setEnabled(plugin && !isDB && xmldoc->plugin->customvfiles);
    customVFiles->setEnabled(plugin && !isDB);
    VLibs->setEnabled(plugin && !isDB && xmldoc->plugin->customvlibs);
    customVLibs->setEnabled(plugin && !isDB);
    MFiles->setEnabled(plugin && isDB && xmldoc->plugin->custommfiles);
    customMFiles->setEnabled(plugin && isDB);
    MLibs->setEnabled(plugin && isDB && xmldoc->plugin->custommlibs);
    customMLibs->setEnabled(plugin && isDB);
    EFiles->setEnabled(plugin && xmldoc->plugin->customefiles);
    customEFiles->setEnabled(plugin);
    ELibsSer->setEnabled(plugin && xmldoc->plugin->customelibsSer);
    customELibsSer->setEnabled(plugin);
    ELibsPar->setEnabled(plugin && xmldoc->plugin->customelibsPar);
    customELibsPar->setEnabled(plugin);

    WFiles->setEnabled(plugin  && !isDB && xmldoc->plugin->customwfiles);
    customWFiles->setEnabled(plugin && !isDB);
    VWFiles->setEnabled(plugin && !isDB && xmldoc->plugin->customvwfiles);
    customVWFiles->setEnabled(plugin && !isDB);

    engSpecificCode->setEnabled(plugin);

    // only enable for a database plugin
    mdSpecificCode->setEnabled(plugin && isDB);

    MCXXFlags->setEnabled(plugin && isDB);
    MLDFlags->setEnabled(plugin && isDB);
    MDefines->setEnabled(plugin && isDB);

    ECXXFlagsSer->setEnabled(plugin);
    ELDFlagsSer->setEnabled(plugin);
    EDefinesSer->setEnabled(plugin);

    ECXXFlagsPar->setEnabled(plugin);
    ELDFlagsPar->setEnabled(plugin);
    EDefinesPar->setEnabled(plugin);
}

// ****************************************************************************
//  Method:  XMLEditCMake::BlockAllSignals
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
XMLEditCMake::BlockAllSignals(bool block)
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
//  Method:  XMLEditCMake::cxxflagsTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::cxxflagsTextChanged()
{
    xmldoc->plugin->cxxflags = SplitValues(CXXFLAGS->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::ldflagsTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::ldflagsTextChanged()
{
    xmldoc->plugin->ldflags = SplitValues(LDFLAGS->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::libsTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::libsTextChanged()
{
    xmldoc->plugin->libs = SplitValues(LIBS->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::defsTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    November 6, 2014
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::defsTextChanged()
{
    xmldoc->plugin->defs = SplitValues(DEFINES->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::gfilesTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::gfilesTextChanged()
{
    xmldoc->plugin->gfiles = SplitValues(GFiles->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::glibsTextChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::glibsTextChanged()
{
    xmldoc->plugin->glibs = SplitValues(GLibs->text());
}


// ****************************************************************************
//  Method:  XMLEditCMake::sfilesTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::sfilesTextChanged()
{
    xmldoc->plugin->sfiles = SplitValues(SFiles->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::vfilesTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::vfilesTextChanged()
{
    xmldoc->plugin->vfiles = SplitValues(VFiles->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::vlibsTextChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::vlibsTextChanged()
{
    xmldoc->plugin->vlibs = SplitValues(VLibs->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::mfilesTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::mfilesTextChanged()
{
    xmldoc->plugin->mfiles = SplitValues(MFiles->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::mlibsTextChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::mlibsTextChanged()
{
    xmldoc->plugin->mlibs = SplitValues(MLibs->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::efilesTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::efilesTextChanged()
{
    xmldoc->plugin->efiles = SplitValues(EFiles->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::elibsSerTextChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Sep  8 14:57:13 EDT 2009
//    Split into ser/par versions.
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::elibsSerTextChanged()
{
    xmldoc->plugin->elibsSer = SplitValues(ELibsSer->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::elibsParTextChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Sep  8 14:57:13 EDT 2009
//    Split into ser/par versions.
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::elibsParTextChanged()
{
    xmldoc->plugin->elibsPar = SplitValues(ELibsPar->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::wfilesTextChanged
//
//  Programmer:  Sean Ahern
//  Creation:    Mon Nov 18 13:55:18 PST 2002
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::wfilesTextChanged()
{
    xmldoc->plugin->wfiles = SplitValues(WFiles->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::vwfilesTextChanged
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Feb 23 17:53:33 PST 2007
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::vwfilesTextChanged()
{
    xmldoc->plugin->vwfiles = SplitValues(VWFiles->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::customgfilesChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditCMake::customgfilesChanged()
{
    xmldoc->plugin->customgfiles = customGFiles->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditCMake::customglibsChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
// ****************************************************************************
void
XMLEditCMake::customglibsChanged()
{
    xmldoc->plugin->customglibs = customGLibs->isChecked();
    UpdateWindowContents();
}


// ****************************************************************************
//  Method:  XMLEditCMake::customsfilesChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditCMake::customsfilesChanged()
{
    xmldoc->plugin->customsfiles = customSFiles->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditCMake::customvfilesChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditCMake::customvfilesChanged()
{
    xmldoc->plugin->customvfiles = customVFiles->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditCMake::customvlibsChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
// ****************************************************************************
void
XMLEditCMake::customvlibsChanged()
{
    xmldoc->plugin->customvlibs = customVLibs->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditCMake::custommfilesChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditCMake::custommfilesChanged()
{
    xmldoc->plugin->custommfiles = customMFiles->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditCMake::custommlibsChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    September 19, 2008
//
// ****************************************************************************
void
XMLEditCMake::custommlibsChanged()
{
    xmldoc->plugin->custommlibs = customMLibs->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditCMake::customefilesChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditCMake::customefilesChanged()
{
    xmldoc->plugin->customefiles = customEFiles->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditCMake::customelibsSerChanged
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
XMLEditCMake::customelibsSerChanged()
{
    xmldoc->plugin->customelibsSer = customELibsSer->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditCMake::customelibsParChanged
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
XMLEditCMake::customelibsParChanged()
{
    xmldoc->plugin->customelibsPar = customELibsPar->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditCMake::customwfilesChanged
//
//  Programmer:  Sean Ahern
//  Creation:    Fri Nov 15 15:29:19 PST 2002
//
// ****************************************************************************
void
XMLEditCMake::customwfilesChanged()
{
    xmldoc->plugin->customwfiles = customWFiles->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditCMake::customvwfilesChanged
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Feb 23 17:54:08 PST 2007
//
// ****************************************************************************
void
XMLEditCMake::customvwfilesChanged()
{
    xmldoc->plugin->customvwfiles = customVWFiles->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditCMake::mdSpecificCodeChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    July  7, 2004
//
// ****************************************************************************
void
XMLEditCMake::mdSpecificCodeChanged()
{
    xmldoc->plugin->has_MDS_specific_code = mdSpecificCode->isChecked();
    UpdateWindowContents();
}


// ****************************************************************************
//  Method:  XMLEditCMake::engSpecificCodeChanged
//
//  Programmer:  Cyrus Harrison
//  Creation:    March  7, 2007
//
// ****************************************************************************
void
XMLEditCMake::engSpecificCodeChanged()
{
    xmldoc->plugin->hasEngineSpecificCode = engSpecificCode->isChecked();
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditCMake::mcxxflagsTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::mcxxflagsTextChanged()
{
    xmldoc->plugin->mcxxflags = SplitValues(MCXXFlags->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::mldflagsTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::mldflagsTextChanged()
{
    xmldoc->plugin->mldflags = SplitValues(MLDFlags->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::mdefsTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::mdefsTextChanged()
{
    xmldoc->plugin->mdefs = SplitValues(MDefines->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::ecxxflagsSerTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::ecxxflagsSerTextChanged()
{
    xmldoc->plugin->ecxxflagsSer = SplitValues(ECXXFlagsSer->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::eldflagsSerTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::eldflagsSerTextChanged()
{
    xmldoc->plugin->eldflagsSer = SplitValues(ELDFlagsSer->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::edefsSerTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::edefsSerTextChanged()
{
    xmldoc->plugin->edefsSer = SplitValues(EDefinesSer->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::ecxxflagsParTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::ecxxflagsParTextChanged()
{
    xmldoc->plugin->ecxxflagsPar = SplitValues(ECXXFlagsPar->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::eldflagsParTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::eldflagsParTextChanged()
{
    xmldoc->plugin->eldflagsPar = SplitValues(ELDFlagsPar->text());
}

// ****************************************************************************
//  Method:  XMLEditCMake::edefsParTextChanged
//
//  Programmer:  Kathleen Biagas
//  Creation:    May 4, 2022
//
//  Modifications:
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg as this slot is now connected to 'editingFinished'.
//
// ****************************************************************************
void
XMLEditCMake::edefsParTextChanged()
{
    xmldoc->plugin->edefsPar = SplitValues(EDefinesPar->text());
}
