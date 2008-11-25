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
#include "XMLEditStd.h"
#include "XMLEditPlugin.h"

#include <XMLDocument.h>
#include <visitstream.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>

#include <Plugin.h>

// ****************************************************************************
//  Constructor:  XMLEditPlugin::XMLEditPlugin
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 13 11:15:25 PDT 2003
//    I added widgets to set the plugin's icon file.
//
//    Hank Childs, Fri Aug  1 11:21:18 PDT 2003
//    Add support for curves.
//
//    Jeremy Meredith, Tue Sep 23 17:06:49 PDT 2003
//    Added a checkbox for "haswriter".  Added support for tensor and
//    symmetric tensor variable types.  Added support for the subset
//    variable type.
//
//    Jeremy Meredith, Wed Nov  5 13:49:32 PST 2003
//    Added checkbox for "enabled by default".
//
//    Hank Childs, Tue May 24 09:26:14 PDT 2005
//    Added hasOptions.
//
//    Hank Childs, Tue Jul 19 14:08:19 PDT 2005
//    Added array variable type.
//
//    Hank Childs, Thu Jan 10 13:56:32 PST 2008
//    Added the ability to have a plugin only open explicit filenames.
//
//    Brad Whitlock, Fri Mar 7 10:35:57 PDT 2008
//    Reorganized some widgets.
//
//   Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//   Ported to Qt 4.4
//
// ****************************************************************************

XMLEditPlugin::XMLEditPlugin(QWidget *p)
    : QFrame(p)
{
    QGridLayout *topLayout = new QGridLayout(this);
    setLayout(topLayout);
    int row = 0;

    attpluginGroup = new QButtonGroup(this);
    QRadioButton *pluginButton = new QRadioButton(tr("Plugin"), this);
    QRadioButton *attButton    = new QRadioButton(tr("Attribute only"), this);
    pluginButton->setChecked(true);
    attpluginGroup->addButton(pluginButton,0);
    attpluginGroup->addButton(attButton,1);
    
    topLayout->addWidget(pluginButton, row, 0);
    topLayout->addWidget(attButton,    row, 1);
    row++;

    //
    // General plugin attributes
    //
    int pRow = 0;
    pluginGroup = new QGroupBox(this);
    pluginGroup->setTitle(tr("General Plugin attributes"));
    topLayout->addWidget(pluginGroup, row, 0, 1 , 4);
    ++row;
    QVBoxLayout *innerPluginLayout = new QVBoxLayout(pluginGroup);
    innerPluginLayout->setMargin(10);
   
    QGridLayout *pluginLayout = new QGridLayout;
    pluginLayout->setSpacing(5);

    pluginType = new QComboBox(pluginGroup);
    pluginType->addItem("");
    pluginType->addItem(tr("Plot"));
    pluginType->addItem(tr("Operator"));
    pluginType->addItem(tr("Database"));
    pluginLayout->addWidget(new QLabel(tr("Plugin type"), pluginGroup), pRow, 0);
    pluginLayout->addWidget(pluginType, pRow, 1);

    enabledByDefault = new QCheckBox(tr("Plugin is enabled by default"), pluginGroup);
    enabledByDefault->setChecked(true);
    pluginLayout->addWidget(enabledByDefault, pRow, 4, 1, 2);
    pRow++;

    name = new QLineEdit(pluginGroup);
    pluginLayout->addWidget(new QLabel(tr("Name"), pluginGroup), pRow, 0);
    pluginLayout->addWidget(name, pRow, 1);

    label = new QLineEdit(pluginGroup);
    pluginLayout->addWidget(new QLabel(tr("Label"), pluginGroup), pRow, 2);
    pluginLayout->addWidget(label, pRow, 3);

    version = new QLineEdit(pluginGroup);
    pluginLayout->addWidget(new QLabel(tr("Version"), pluginGroup), pRow, 4);
    pluginLayout->addWidget(version, pRow, 5);
    pRow++;

    hasIcon = new QCheckBox(tr("Has icon"), pluginGroup);
    hasIcon->setChecked(false);
    pluginLayout->addWidget(hasIcon, pRow, 0);
    iconFile = new QLineEdit(pluginGroup);
    iconFile->setEnabled(false);
    pluginLayout->addWidget(iconFile, pRow,1, 1,5);
    pRow++;
    innerPluginLayout->addLayout(pluginLayout);
    
    //
    // Plot plugin attributes
    //
    plotPluginGroup = new QGroupBox(this);
    plotPluginGroup->setTitle(tr("Plot Plugin attributes"));
    topLayout->addWidget(plotPluginGroup, row, 0, 1, 4);
    ++row;
    QVBoxLayout *innerPlotPluginLayout = new QVBoxLayout(plotPluginGroup);
    innerPlotPluginLayout->setMargin(10);
    QGridLayout *plotPluginLayout = new QGridLayout;
    plotPluginLayout->setSpacing(5);
    int plRow = 0;

    plotPluginLayout->addWidget(
        new QLabel(tr("Variable types accepted by the plot"), plotPluginGroup),
        plRow,0, 1,5);
    ++plRow;

    varTypeMesh            = new QCheckBox(tr("Mesh"), plotPluginGroup);
    varTypeScalar          = new QCheckBox(tr("Scalar"), plotPluginGroup);
    varTypeVector          = new QCheckBox(tr("Vector"), plotPluginGroup);
    varTypeMaterial        = new QCheckBox(tr("Material"), plotPluginGroup);
    varTypeSubset          = new QCheckBox(tr("Subset"), plotPluginGroup);
    varTypeSpecies         = new QCheckBox(tr("Species"), plotPluginGroup);
    varTypeCurve           = new QCheckBox(tr("Curve"), plotPluginGroup);
    varTypeTensor          = new QCheckBox(tr("Tensor"), plotPluginGroup);
    varTypeSymmetricTensor = new QCheckBox(tr("Symmetric Tensor"), plotPluginGroup);
    varTypeLabel           = new QCheckBox(tr("Label"), plotPluginGroup);
    varTypeArray           = new QCheckBox(tr("Array"), plotPluginGroup);

    plotPluginLayout->addWidget(varTypeMesh, plRow, 0);
    plotPluginLayout->addWidget(varTypeScalar, plRow, 1);
    plotPluginLayout->addWidget(varTypeVector, plRow, 2);
    plotPluginLayout->addWidget(varTypeMaterial, plRow, 3);
    plotPluginLayout->addWidget(varTypeSubset, plRow, 4);
    plotPluginLayout->addWidget(varTypeSpecies, plRow, 5);
    ++plRow;

    plotPluginLayout->addWidget(varTypeCurve, plRow, 0);
    plotPluginLayout->addWidget(varTypeTensor, plRow, 1);
    plotPluginLayout->addWidget(varTypeSymmetricTensor, plRow, 2);
    plotPluginLayout->addWidget(varTypeLabel, plRow, 3);
    plotPluginLayout->addWidget(varTypeArray, plRow, 4);
    //plotPluginLayout->addWidget(, plRow, 5);
    ++plRow;
    innerPlotPluginLayout->addLayout(plotPluginLayout);
    //
    // Database plugin attributes
    //
    dbPluginGroup = new QGroupBox(this);
    dbPluginGroup->setTitle(tr("Database Plugin attributes"));
    topLayout->addWidget(dbPluginGroup, row, 0, 1, 4);
    ++row;
    QVBoxLayout *innerdbPluginLayout = new QVBoxLayout(dbPluginGroup);
    innerdbPluginLayout->setMargin(10);
    QGridLayout *dbPluginLayout = new QGridLayout;
    dbPluginLayout->setSpacing(5);
    int dbRow = 0;

    dbType = new QComboBox(dbPluginGroup);
    dbType->addItem("");
    dbType->addItem(tr("STSD - Generic single time single domain"));
    dbType->addItem(tr("MTSD - Generic multi  time single domain"));
    dbType->addItem(tr("STMD - Generic single time multi  domain"));
    dbType->addItem(tr("MTMD - Generic multi  time multi  domain"));
    dbType->addItem(tr("Custom - Fully customized database type"));
    dbPluginLayout->addWidget(new QLabel("Database type", dbPluginGroup), dbRow, 0);
    dbPluginLayout->addWidget(dbType, dbRow, 1);
    dbRow++;

    dbPluginLayout->addWidget(new QLabel(tr("Extensions"), dbPluginGroup), dbRow, 0);
    extensions = new QLineEdit(dbPluginGroup);
    dbPluginLayout->addWidget(extensions, dbRow, 1);
    dbRow++;

    hasWriter = new QCheckBox(tr("File format can also write data"), dbPluginGroup);
    hasWriter->setChecked(false);
    dbPluginLayout->addWidget(hasWriter, dbRow,0, 1,2);
    dbRow++;

    hasOptions = new QCheckBox(tr("File format provides options for reading or writing data."), dbPluginGroup);
    hasOptions->setChecked(false);
    dbPluginLayout->addWidget(hasOptions, dbRow,0, 1,2);
    dbRow++;

    specifiedFilenames = new QCheckBox(tr("Format uses explicit filenames"), dbPluginGroup);
    specifiedFilenames->setChecked(false);
    dbPluginLayout->addWidget(specifiedFilenames, dbRow,0,1,2);
    dbRow++;

    dbPluginLayout->addWidget(new QLabel(tr("Filenames"), dbPluginGroup), dbRow, 0);
    filenames = new QLineEdit(dbPluginGroup);
    dbPluginLayout->addWidget(filenames, dbRow,1);
    dbRow++;
    
    innerdbPluginLayout->addLayout(dbPluginLayout);
    
    topLayout->setRowStretch(row, 100);
    row++;

    
    
    xmldoc = NULL;

    connect(attpluginGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(attpluginGroupChanged(int)));
    connect(name, SIGNAL(textChanged(const QString &)),
            this,  SLOT(nameTextChanged(const QString &)));
    connect(label, SIGNAL(textChanged(const QString &)),
            this,  SLOT(labelTextChanged(const QString &)));
    connect(version, SIGNAL(textChanged(const QString &)),
            this,  SLOT(versionTextChanged(const QString &)));
    connect(pluginType, SIGNAL(activated(int)),
            this,  SLOT(pluginTypeChanged(int)));
    connect(dbType, SIGNAL(activated(int)),
            this,  SLOT(dbTypeChanged(int)));
    connect(extensions, SIGNAL(textChanged(const QString &)),
            this,  SLOT(extensionsTextChanged(const QString &)));
    connect(varTypeMesh, SIGNAL(clicked()),
            this, SLOT(varTypesChanged()));
    connect(varTypeScalar, SIGNAL(clicked()),
            this, SLOT(varTypesChanged()));
    connect(varTypeVector, SIGNAL(clicked()),
            this, SLOT(varTypesChanged()));
    connect(varTypeMaterial, SIGNAL(clicked()),
            this, SLOT(varTypesChanged()));
    connect(varTypeSubset, SIGNAL(clicked()),
            this, SLOT(varTypesChanged()));
    connect(varTypeSpecies, SIGNAL(clicked()),
            this, SLOT(varTypesChanged()));
    connect(varTypeCurve, SIGNAL(clicked()),
            this, SLOT(varTypesChanged()));
    connect(varTypeTensor, SIGNAL(clicked()),
            this, SLOT(varTypesChanged()));
    connect(varTypeSymmetricTensor, SIGNAL(clicked()),
            this, SLOT(varTypesChanged()));
    connect(varTypeLabel, SIGNAL(clicked()),
            this, SLOT(varTypesChanged()));
    connect(varTypeArray, SIGNAL(clicked()),
            this, SLOT(varTypesChanged()));
    connect(hasIcon, SIGNAL(toggled(bool)),
            this, SLOT(hasIconChanged(bool)));
    connect(iconFile, SIGNAL(textChanged(const QString &)),
            this,  SLOT(iconFileTextChanged(const QString &)));
    connect(hasWriter, SIGNAL(toggled(bool)),
            this, SLOT(hasWriterChanged(bool)));
    connect(hasOptions, SIGNAL(toggled(bool)),
            this, SLOT(hasOptionsChanged(bool)));
    connect(enabledByDefault, SIGNAL(toggled(bool)),
            this, SLOT(enabledByDefaultChanged(bool)));
    connect(specifiedFilenames, SIGNAL(toggled(bool)),
            this, SLOT(specifiedFilenamesChanged(bool)));
    connect(filenames, SIGNAL(textChanged(const QString &)),
            this,  SLOT(filenamesTextChanged(const QString &)));
}

// ****************************************************************************
//  Method:  XMLEditPlugin::UpdateWindowContents
//
//  Purpose:
//    Update the window based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 13 11:20:57 PDT 2003
//    I added support for plugin icons.
//
//    Hank Childs, Fri Aug  1 11:21:18 PDT 2003
//    Add support for curves.
//
//    Jeremy Meredith, Wed Nov  5 13:49:49 PST 2003
//    Added support for plugins en/disabled by default.
//
//    Brad Whitlock, Fri Apr 1 16:07:24 PST 2005
//    Added support for label vars.
//
//    Hank Childs, Tue May 24 09:26:14 PDT 2005
//    Add hasOptions.
//
//    Hank Childs, Tue Jul 19 14:08:19 PDT 2005
//    Added support for arrays.
//
//    Hank Childs, Thu Jan 10 13:56:32 PST 2008
//    Added the ability to have a plugin only open explicit filenames.
//
//   Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//   Ported to Qt 4.4
//
// ****************************************************************************

void
XMLEditPlugin::UpdateWindowContents()
{
    BlockAllSignals(true);

    if (xmldoc->docType == "Plugin")
    {
        attpluginGroup->button(0)->setChecked(true);
        name->setText(xmldoc->plugin->name);
        label->setText(xmldoc->plugin->label);
        version->setText(xmldoc->plugin->version);
        varTypeMesh->setChecked(false);
        varTypeScalar->setChecked(false);
        varTypeVector->setChecked(false);
        varTypeMaterial->setChecked(false);
        varTypeSubset->setChecked(false);
        varTypeSpecies->setChecked(false);
        varTypeCurve->setChecked(false);
        varTypeTensor->setChecked(false);
        varTypeSymmetricTensor->setChecked(false);
        varTypeLabel->setChecked(false);
        varTypeArray->setChecked(false);
        enabledByDefault->setChecked(xmldoc->plugin->enabledByDefault);

        dbType->setCurrentIndex(0);
        extensions->setText("");
        filenames->setText("");
        if (xmldoc->plugin->type == "plot")
        {
            iconFile->setText(xmldoc->plugin->iconFile);
            hasIcon->setChecked(xmldoc->plugin->iconFile.length() > 0);
            pluginType->setCurrentIndex(1);
            vector<QString> types = SplitValues(xmldoc->plugin->vartype);
            for (size_t i=0; i<types.size(); i++)
            {
                if      (types[i] == "mesh")
                    varTypeMesh->setChecked(true);
                else if (types[i] == "scalar")
                    varTypeScalar->setChecked(true);
                else if (types[i] == "vector")
                    varTypeVector->setChecked(true);
                else if (types[i] == "material")
                    varTypeMaterial->setChecked(true);
                else if (types[i] == "subset")
                    varTypeSubset->setChecked(true);
                else if (types[i] == "species")
                    varTypeSpecies->setChecked(true);
                else if (types[i] == "curve")
                    varTypeCurve->setChecked(true);
                else if (types[i] == "tensor")
                    varTypeTensor->setChecked(true);
                else if (types[i] == "symmetrictensor")
                    varTypeSymmetricTensor->setChecked(true);
                else if (types[i] == "label")
                    varTypeLabel->setChecked(true);
                else if (types[i] == "array")
                    varTypeArray->setChecked(true);
            }
        }
        else if (xmldoc->plugin->type == "operator")
        {
            iconFile->setText(xmldoc->plugin->iconFile);
            hasIcon->setChecked(xmldoc->plugin->iconFile.length() > 0);
            pluginType->setCurrentIndex(2);
        }
        else if (xmldoc->plugin->type == "database")
        {
            iconFile->setText("");
            hasIcon->setChecked(false);
            hasWriter->setChecked(xmldoc->plugin->haswriter);
            hasOptions->setChecked(xmldoc->plugin->hasoptions);
            specifiedFilenames->setChecked(xmldoc->plugin->specifiedFilenames);

            pluginType->setCurrentIndex(3);
            extensions->setText(JoinValues(xmldoc->plugin->extensions, ' '));
            filenames->setText(JoinValues(xmldoc->plugin->filenames, ' '));
            if      (xmldoc->plugin->dbtype == "STSD")
                dbType->setCurrentIndex(1);
            else if (xmldoc->plugin->dbtype == "MTSD")
                dbType->setCurrentIndex(2);
            else if (xmldoc->plugin->dbtype == "STMD")
                dbType->setCurrentIndex(3);
            else if (xmldoc->plugin->dbtype == "MTMD")
                dbType->setCurrentIndex(4);
            else if (xmldoc->plugin->dbtype == "Custom")
                dbType->setCurrentIndex(5);
        }
        else
        {
            iconFile->setText("");
            hasIcon->setChecked(false);
            hasWriter->setChecked(false);
            hasOptions->setChecked(false);
            specifiedFilenames->setChecked(false);
            enabledByDefault->setChecked(true);
            pluginType->setCurrentIndex(0);
        }
    }
    else
    {
        attpluginGroup->button(1)->setChecked(true);
        name->setText(xmldoc->attribute->name);
        label->setText("");
        version->setText("");
        varTypeMesh->setChecked(false);
        varTypeScalar->setChecked(false);
        varTypeVector->setChecked(false);
        varTypeMaterial->setChecked(false);
        varTypeSubset->setChecked(false);
        varTypeSpecies->setChecked(false);
        varTypeCurve->setChecked(false);
        varTypeTensor->setChecked(false);
        varTypeSymmetricTensor->setChecked(false);
        varTypeLabel->setChecked(false);
        varTypeArray->setChecked(false);
        hasIcon->setChecked(false);
        iconFile->setText("");
        hasWriter->setChecked(false);
        hasOptions->setChecked(false);
        specifiedFilenames->setChecked(false);
        pluginType->setCurrentIndex(0);
        dbType->setCurrentIndex(0);
        extensions->setText("");
        filenames->setText("");
        enabledByDefault->setChecked(true);
    }

    UpdateWindowSensitivity();

    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  XMLEditPlugin::UpdateWindowSensitivity
//
//  Purpose:
//    Enable/disable widget sensitivity based on the current state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 13 11:29:22 PDT 2003
//    I added support for plugin icons.
//
//    Hank Childs, Fri Aug  1 11:21:18 PDT 2003
//    Add support for curves.
//
//    Jeremy Meredith, Wed Nov  5 13:49:49 PST 2003
//    Added support for plugins en/disabled by default.
//
//    Brad Whitlock, Fri Apr 1 16:08:18 PST 2005
//    Added label vars.
//
//    Hank Childs, Tue May 24 09:26:14 PDT 2005
//    Add hasOptions.
//
//    Hank Childs, Tue Jul 19 14:08:19 PDT 2005
//    Added array vars.
//
//    Hank Childs, Thu Jan 10 13:56:32 PST 2008
//    Added the ability to have a plugin only open explicit filenames.
//
//    Brad Whitlock, Fri Mar 7 10:58:47 PDT 2008
//    Added group boxes.
//
// ****************************************************************************

void
XMLEditPlugin::UpdateWindowSensitivity()
{
    bool plugin   = (xmldoc->docType == "Plugin");
    bool plot     = plugin && (xmldoc->plugin->type == "plot");
    bool op       = plugin && (xmldoc->plugin->type == "operator");
    bool db       = plugin && (xmldoc->plugin->type == "database");

    pluginGroup->setEnabled(plugin);
    name->setEnabled(plugin);
    label->setEnabled(plugin);
    version->setEnabled(plugin);
    pluginType->setEnabled(plugin);
    plotPluginGroup->setEnabled(plot);
    varTypeMesh->setEnabled(plot);
    varTypeScalar->setEnabled(plot);
    varTypeVector->setEnabled(plot);
    varTypeMaterial->setEnabled(plot);
    varTypeSubset->setEnabled(plot);
    varTypeSpecies->setEnabled(plot);
    varTypeCurve->setEnabled(plot);
    varTypeTensor->setEnabled(plot);
    varTypeSymmetricTensor->setEnabled(plot);
    varTypeLabel->setEnabled(plot);
    varTypeArray->setEnabled(plot);
    dbPluginGroup->setEnabled(db);
    dbType->setEnabled(db);
    extensions->setEnabled(db);
    filenames->setEnabled(db && xmldoc->plugin->specifiedFilenames);
    hasIcon->setEnabled(op || plot);
    hasWriter->setEnabled(db);
    hasOptions->setEnabled(db);
    specifiedFilenames->setEnabled(db);
    enabledByDefault->setEnabled(plugin);
    bool val = (op || plot) && (xmldoc->plugin->iconFile.length() > 0);
    iconFile->setEnabled(val);
}

// ****************************************************************************
//  Method:  XMLEditPlugin::BlockAllSignals
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
//    Brad Whitlock, Thu Mar 13 11:31:39 PDT 2003
//    Added plugin icon support.
//
//    Hank Childs, Fri Aug  1 11:21:18 PDT 2003
//    Add support for curves.
//
//    Jeremy Meredith, Wed Nov  5 13:49:49 PST 2003
//    Added support for plugins en/disabled by default.
//
//    Brad Whitlock, Fri Apr 1 16:08:37 PST 2005
//    Added label.
//
//    Hank Childs, Tue Jul 19 14:08:19 PDT 2005
//    Added array vars.
//
//    Hank Childs, Thu Jan 10 13:56:32 PST 2008
//    Added the ability to have a plugin only open explicit filenames.
//
// ****************************************************************************

void
XMLEditPlugin::BlockAllSignals(bool block)
{
    name->blockSignals(block);
    label->blockSignals(block);
    version->blockSignals(block);
    pluginType->blockSignals(block);
    varTypeMesh->blockSignals(block);
    varTypeScalar->blockSignals(block);
    varTypeVector->blockSignals(block);
    varTypeMaterial->blockSignals(block);
    varTypeSubset->blockSignals(block);
    varTypeSpecies->blockSignals(block);
    varTypeCurve->blockSignals(block);
    varTypeTensor->blockSignals(block);
    varTypeSymmetricTensor->blockSignals(block);
    varTypeLabel->blockSignals(block);
    varTypeArray->blockSignals(block);
    dbType->blockSignals(block);
    extensions->blockSignals(block);
    filenames->blockSignals(block);
    hasIcon->blockSignals(block);
    iconFile->blockSignals(block);
    hasWriter->blockSignals(block);
    hasOptions->blockSignals(block);
    specifiedFilenames->blockSignals(block);
    enabledByDefault->blockSignals(block);
}

// ----------------------------------------------------------------------------
//                                 Callbacks
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Method:  XMLEditPlugin::attpluginGroupChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 13 13:02:53 PST 2003
//    I added another argument to the plugin constructor.
//
//    Hank Childs, Sat Sep 13 08:27:57 PDT 2003
//    Added another argument to plugin constructor.
//
//    Jeremy Meredith, Wed Aug 25 11:57:08 PDT 2004
//    Added the "no-engine" and "engine-only" options to the plugin
//    constructor.
//
//    Hank Childs, Tue May 24 10:19:40 PDT 2005
//    Added another argument to plugin constructor.
//
// ****************************************************************************
void
XMLEditPlugin::attpluginGroupChanged(int id)
{
    if (id == 0)
    {
        xmldoc->docType = "Plugin";
        if (!xmldoc->plugin)
        {
            xmldoc->plugin = new Plugin("","","","","","","",false,false,false,
                                        false);
            xmldoc->plugin->atts = xmldoc->attribute;
        }
    }
    else
    {
        xmldoc->docType = "Attribute";
    }
    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditPlugin::nameTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditPlugin::nameTextChanged(const QString &text)
{
    if (xmldoc->docType != "Plugin")
        return;

    xmldoc->plugin->name = text;
}

// ****************************************************************************
//  Method:  XMLEditPlugin::labelTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditPlugin::labelTextChanged(const QString &text)
{
    if (xmldoc->docType != "Plugin")
        return;

    xmldoc->plugin->label = text;
}

// ****************************************************************************
//  Method:  XMLEditPlugin::versionTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditPlugin::versionTextChanged(const QString &text)
{
    if (xmldoc->docType != "Plugin")
        return;

    xmldoc->plugin->version = text;
}

// ****************************************************************************
//  Method:  XMLEditPlugin::versionTextChanged
//
//  Programmer:  Brad Whitlock
//  Creation:    Thu Mar 13 11:33:10 PDT 2003
//
//  Modifications:
//
// ****************************************************************************

void
XMLEditPlugin::iconFileTextChanged(const QString &text)
{
    if (xmldoc->docType != "Plugin")
        return;

    xmldoc->plugin->iconFile = text;
}

// ****************************************************************************
// Method: XMLEditPlugin::hasIconChanged
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 13 11:35:51 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
XMLEditPlugin::hasIconChanged(bool val)
{
    if (xmldoc->docType != "Plugin")
        return;

    if(val)
        xmldoc->plugin->iconFile = xmldoc->plugin->name + ".xpm";
    else
        xmldoc->plugin->iconFile = "";

    UpdateWindowContents();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: XMLEditPlugin::hasWriterChanged
//
// Programmer: Jeremy Meredith
// Creation:   September 23, 2003
//
// Modifications:
//   
// ****************************************************************************

void
XMLEditPlugin::hasWriterChanged(bool val)
{
    if (xmldoc->docType != "Plugin")
        return;

    xmldoc->plugin->haswriter = val;

    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: XMLEditPlugin::specifiedFilenamesChanged
//
// Programmer: Hank Childs
// Creation:   January 10, 2008
//
// Modifications:
//   
// ****************************************************************************

void
XMLEditPlugin::specifiedFilenamesChanged(bool val)
{
    if (xmldoc->docType != "Plugin")
        return;

    xmldoc->plugin->specifiedFilenames = val;

    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: XMLEditPlugin::hasOptionsChanged
//
// Programmer: Hank Childs
// Creation:   May 24, 2005
//
// Modifications:
//   
// ****************************************************************************

void
XMLEditPlugin::hasOptionsChanged(bool val)
{
    if (xmldoc->docType != "Plugin")
        return;

    xmldoc->plugin->hasoptions = val;

    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: XMLEditPlugin::enabledByDefaultChanged
//
// Programmer: Jeremy Meredith
// Creation:   November  5, 2003
//
// Modifications:
//   
// ****************************************************************************

void
XMLEditPlugin::enabledByDefaultChanged(bool val)
{
    if (xmldoc->docType != "Plugin")
        return;

    xmldoc->plugin->enabledByDefault = val;
}

// ****************************************************************************
//  Method:  XMLEditPlugin::pluginTypeChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditPlugin::pluginTypeChanged(int index)
{
    if (xmldoc->docType != "Plugin")
        return;

    switch (index)
    {
      case 1: xmldoc->plugin->type = "plot";      break;
      case 2: xmldoc->plugin->type = "operator";  break;
      case 3: xmldoc->plugin->type = "database";  break;
      default: xmldoc->plugin->type = "";         break;
    }

    UpdateWindowContents();
}

// ****************************************************************************
//  Method:  XMLEditPlugin::dbTypeChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditPlugin::dbTypeChanged(int index)
{
    if (xmldoc->docType != "Plugin")
        return;

    switch (index)
    {
      case 1: xmldoc->plugin->dbtype = "STSD";    break;
      case 2: xmldoc->plugin->dbtype = "MTSD";    break;
      case 3: xmldoc->plugin->dbtype = "STMD";    break;
      case 4: xmldoc->plugin->dbtype = "MTMD";    break;
      case 5: xmldoc->plugin->dbtype = "Custom";  break;
      default: xmldoc->plugin->dbtype = "";       break;
    }
}

// ****************************************************************************
//  Method:  XMLEditPlugin::extensionsTextChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEditPlugin::extensionsTextChanged(const QString &text)
{
    if (xmldoc->docType != "Plugin")
        return;

    xmldoc->plugin->extensions = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditPlugin::filenamesTextChanged
//
//  Programmer:  Hank Childs
//  Creation:    January 10, 2008
//
// ****************************************************************************

void
XMLEditPlugin::filenamesTextChanged(const QString &text)
{
    if (xmldoc->docType != "Plugin")
        return;

    xmldoc->plugin->filenames = SplitValues(text);
}

// ****************************************************************************
//  Method:  XMLEditPlugin::varTypesChanged
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Aug  1 11:21:18 PDT 2003
//    Add support for curves.
//
//    Brad Whitlock, Fri Apr 1 16:09:04 PST 2005
//    Added label.
//
//    Hank Childs, Tue Jul 19 14:08:19 PDT 2005
//    Added array vars.
//
// ****************************************************************************
void
XMLEditPlugin::varTypesChanged()
{
    if (xmldoc->docType != "Plugin")
        return;

    Plugin *p = xmldoc->plugin;
    p->vartype = "";
    if (varTypeMesh->isChecked())
        p->vartype += "mesh,";
    if (varTypeScalar->isChecked())
        p->vartype += "scalar,";
    if (varTypeVector->isChecked())
        p->vartype += "vector,";
    if (varTypeMaterial->isChecked())
        p->vartype += "material,";
    if (varTypeSubset->isChecked())
        p->vartype += "subset,";
    if (varTypeSpecies->isChecked())
        p->vartype += "species,";
    if (varTypeCurve->isChecked())
        p->vartype += "curve,";
    if (varTypeTensor->isChecked())
        p->vartype += "tensor,";
    if (varTypeSymmetricTensor->isChecked())
        p->vartype += "symmetrictensor,";
    if (varTypeLabel->isChecked())
        p->vartype += "label,";
    if (varTypeArray->isChecked())
        p->vartype += "array,";
    if (!p->vartype.isEmpty())
        p->vartype = p->vartype.left(p->vartype.length()-1);
}
