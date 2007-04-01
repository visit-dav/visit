#include "XMLEditPlugin.h"

#include <XMLDocument.h>
#include <visitstream.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
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
// ****************************************************************************

XMLEditPlugin::XMLEditPlugin(QWidget *p, const QString &n)
    : QFrame(p, n)
{
    QGridLayout *topLayout = new QGridLayout(this, 12,2, 5);
    int row = 0;

    attpluginGroup = new QButtonGroup();
    QRadioButton *pluginButton = new QRadioButton("Plugin", this);
    QRadioButton *attButton    = new QRadioButton("Attribute only", this);
    attpluginGroup->insert(pluginButton);
    attpluginGroup->insert(attButton);
    topLayout->addWidget(pluginButton, row, 0);
    topLayout->addWidget(attButton,    row, 1);
    row++;

    name = new QLineEdit(this);
    topLayout->addWidget(new QLabel("Name", this), row, 0);
    topLayout->addWidget(name, row, 1);
    row++;

    label = new QLineEdit(this);
    topLayout->addWidget(new QLabel("Label", this), row, 0);
    topLayout->addWidget(label, row, 1);
    row++;

    version = new QLineEdit(this);
    topLayout->addWidget(new QLabel("Version", this), row, 0);
    topLayout->addWidget(version, row, 1);
    row++;

    pluginType = new QComboBox(this);
    pluginType->insertItem("");
    pluginType->insertItem("Plot");
    pluginType->insertItem("Operator");
    pluginType->insertItem("Database");
    topLayout->addWidget(new QLabel("Plugin type", this), row, 0);
    topLayout->addWidget(pluginType, row, 1);
    row++;

    hasIcon = new QCheckBox("Has icon", this);
    hasIcon->setChecked(false);
    topLayout->addWidget(hasIcon, row, 0);
    iconFile = new QLineEdit(this);
    iconFile->setEnabled(false);
    topLayout->addWidget(iconFile, row, 1);
    row++;

    hasWriter = new QCheckBox("File format can also write data", this);
    hasWriter->setChecked(false);
    topLayout->addMultiCellWidget(hasWriter, row,row, 0,1);
    row++;

    enabledByDefault = new QCheckBox("Plugin is enabled by default", this);
    hasWriter->setChecked(true);
    topLayout->addMultiCellWidget(enabledByDefault, row,row, 0,1);
    row++;

    topLayout->addWidget(new QLabel("Variable types", this), row, 0);

    varTypeMesh            = new QCheckBox("Mesh", this);
    varTypeScalar          = new QCheckBox("Scalar", this);
    varTypeVector          = new QCheckBox("Vector", this);
    varTypeMaterial        = new QCheckBox("Material", this);
    varTypeSubset          = new QCheckBox("Subset", this);
    varTypeSpecies         = new QCheckBox("Species", this);
    varTypeCurve           = new QCheckBox("Curve", this);
    varTypeTensor          = new QCheckBox("Tensor", this);
    varTypeSymmetricTensor = new QCheckBox("Symmetric Tensor", this);

    QHBoxLayout *varTypeLayout1 = new QHBoxLayout();
    varTypeLayout1->addWidget(varTypeMesh);
    varTypeLayout1->addWidget(varTypeScalar);
    varTypeLayout1->addWidget(varTypeVector);
    varTypeLayout1->addWidget(varTypeMaterial);
    varTypeLayout1->addWidget(varTypeSubset);
    topLayout->addLayout(varTypeLayout1, row, 1);
    row++;
    QHBoxLayout *varTypeLayout2 = new QHBoxLayout();
    varTypeLayout2->addWidget(varTypeSpecies);
    varTypeLayout2->addWidget(varTypeCurve);
    varTypeLayout2->addWidget(varTypeTensor);
    varTypeLayout2->addWidget(varTypeSymmetricTensor);
    topLayout->addLayout(varTypeLayout2, row, 1);
    row++;

    dbType = new QComboBox(this);
    dbType->insertItem("");
    dbType->insertItem("STSD - Generic single time single domain");
    dbType->insertItem("MTSD - Generic multi  time single domain");
    dbType->insertItem("STMD - Generic single time multi  domain");
    dbType->insertItem("MTMD - Generic multi  time multi  domain");
    dbType->insertItem("Custom - Fully customized database type");
    topLayout->addWidget(new QLabel("Database type", this), row, 0);
    topLayout->addWidget(dbType, row, 1);
    row++;

    topLayout->addWidget(new QLabel("Extensions", this), row, 0);
    extensions = new QLineEdit(this);
    topLayout->addMultiCellWidget(extensions, row,row, 1,2);
    row++;

    topLayout->setRowStretch(row, 100);
    row++;

    xmldoc = NULL;

    connect(attpluginGroup, SIGNAL(clicked(int)),
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
    connect(hasIcon, SIGNAL(toggled(bool)),
            this, SLOT(hasIconChanged(bool)));
    connect(iconFile, SIGNAL(textChanged(const QString &)),
            this,  SLOT(iconFileTextChanged(const QString &)));
    connect(hasWriter, SIGNAL(toggled(bool)),
            this, SLOT(hasWriterChanged(bool)));
    connect(enabledByDefault, SIGNAL(toggled(bool)),
            this, SLOT(enabledByDefaultChanged(bool)));
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
// ****************************************************************************

void
XMLEditPlugin::UpdateWindowContents()
{
    BlockAllSignals(true);

    if (xmldoc->docType == "Plugin")
    {
        attpluginGroup->setButton(0);
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
        enabledByDefault->setChecked(xmldoc->plugin->enabledByDefault);

        dbType->setCurrentItem(0);
        extensions->setText("");
        if (xmldoc->plugin->type == "plot")
        {
            iconFile->setText(xmldoc->plugin->iconFile);
            hasIcon->setChecked(xmldoc->plugin->iconFile.length() > 0);
            pluginType->setCurrentItem(1);
            vector<QString> types = SplitValues(xmldoc->plugin->vartype);
            for (int i=0; i<types.size(); i++)
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
            }
        }
        else if (xmldoc->plugin->type == "operator")
        {
            iconFile->setText(xmldoc->plugin->iconFile);
            hasIcon->setChecked(xmldoc->plugin->iconFile.length() > 0);
            pluginType->setCurrentItem(2);
        }
        else if (xmldoc->plugin->type == "database")
        {
            iconFile->setText("");
            hasIcon->setChecked(false);
            hasWriter->setChecked(xmldoc->plugin->haswriter);

            pluginType->setCurrentItem(3);
            extensions->setText(JoinValues(xmldoc->plugin->extensions, ' '));
            if      (xmldoc->plugin->dbtype == "STSD")
                dbType->setCurrentItem(1);
            else if (xmldoc->plugin->dbtype == "MTSD")
                dbType->setCurrentItem(2);
            else if (xmldoc->plugin->dbtype == "STMD")
                dbType->setCurrentItem(3);
            else if (xmldoc->plugin->dbtype == "MTMD")
                dbType->setCurrentItem(4);
            else if (xmldoc->plugin->dbtype == "Custom")
                dbType->setCurrentItem(5);
        }
        else
        {
            iconFile->setText("");
            hasIcon->setChecked(false);
            hasWriter->setChecked(false);
            enabledByDefault->setChecked(true);
            pluginType->setCurrentItem(0);
        }
    }
    else
    {
        attpluginGroup->setButton(1);
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
        hasIcon->setChecked(false);
        iconFile->setText("");
        hasWriter->setChecked(false);
        pluginType->setCurrentItem(0);
        dbType->setCurrentItem(0);
        extensions->setText("");
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
// ****************************************************************************

void
XMLEditPlugin::UpdateWindowSensitivity()
{
    bool plugin   = (xmldoc->docType == "Plugin");
    bool plot     = plugin && (xmldoc->plugin->type == "plot");
    bool op       = plugin && (xmldoc->plugin->type == "operator");
    bool db       = plugin && (xmldoc->plugin->type == "database");

    name->setEnabled(plugin);
    label->setEnabled(plugin);
    version->setEnabled(plugin);
    pluginType->setEnabled(plugin);
    varTypeMesh->setEnabled(plot);
    varTypeScalar->setEnabled(plot);
    varTypeVector->setEnabled(plot);
    varTypeMaterial->setEnabled(plot);
    varTypeSubset->setEnabled(plot);
    varTypeSpecies->setEnabled(plot);
    varTypeCurve->setEnabled(plot);
    varTypeTensor->setEnabled(plot);
    varTypeSymmetricTensor->setEnabled(plot);
    dbType->setEnabled(db);
    extensions->setEnabled(db);
    hasIcon->setEnabled(op || plot);
    hasWriter->setEnabled(db);
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
    dbType->blockSignals(block);
    extensions->blockSignals(block);
    hasIcon->blockSignals(block);
    iconFile->blockSignals(block);
    hasWriter->blockSignals(block);
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
// ****************************************************************************
void
XMLEditPlugin::attpluginGroupChanged(int id)
{
    if (id == 0)
    {
        xmldoc->docType = "Plugin";
        if (!xmldoc->plugin)
        {
            xmldoc->plugin = new Plugin("","","","","","","",false,false,false);
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
    if (!p->vartype.isEmpty())
        p->vartype = p->vartype.left(p->vartype.length()-1);
}
