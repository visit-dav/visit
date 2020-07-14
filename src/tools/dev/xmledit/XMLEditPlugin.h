// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XMLEDITPLUGIN_H
#define XMLEDITPLUGIN_H

#include <qframe.h>

class XMLDocument;
class QLabel;
class QLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;
class QGroupBox;

// ****************************************************************************
//  Class:  XMLEditPlugin
//
//  Purpose:
//    Plugin editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 13 11:13:05 PDT 2003
//    I added support for plugin icons.
//
//    Hank Childs, Fri Aug  1 11:27:57 PDT 2003
//    Added support for curves.
//
//    Jeremy Meredith, Tue Sep 23 17:05:53 PDT 2003
//    Added support for tensor and symmetric tensor variable types.
//    Added support for the "haswriter" database plugin field.
//
//    Jeremy Meredith, Wed Nov  5 13:49:49 PST 2003
//    Added support for enabling or disabling plugins by default.
//
//    Brad Whitlock, Fri Apr 1 16:14:33 PST 2005
//    Added label var type.
//
//    Hank Childs, Tue May 24 09:26:14 PDT 2005
//    Added hasOptions.
//
//    Hank Childs, Tue Jul 19 14:08:19 PDT 2005
//    Added array var type.
//
//    Hank Childs, Thu Jan 10 13:56:32 PST 2008
//    Added the ability to have a plugin only open explicit filenames.
//
//    Brad Whitlock, Fri Mar 7 10:55:40 PDT 2008
//    Added some group boxes.
//
//    Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//    Ported to Qt 4.4
//
//    Jeremy Meredith, Tue Dec 29 11:21:30 EST 2009
//    Replaced "Extensions" and "Filenames" with "FilePatterns".  Removed
//    specifiedFilenames.  Added filePatternsStrict and opensWholeDirectory.
//
//    Hank Childs, Thu Dec 30 22:37:51 PST 2010
//    Add support for expression-creating-operators.
//
//    Hank Childs, Mon Jan 31 16:02:59 PST 2011
//    Add some data members that facilitate greying out operator options when
//    making non-operators.
//
//    Kathleen Biagas, Thu Jan  2 10:05:15 MST 2020
//    Added hasLicense.
//
// ****************************************************************************

class XMLEditPlugin : public QFrame
{
    Q_OBJECT
  public:
    XMLEditPlugin(QWidget *p);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void attpluginGroupChanged(int);
    void nameTextChanged(const QString&);
    void labelTextChanged(const QString&);
    void versionTextChanged(const QString&);
    void iconFileTextChanged(const QString&);
    void hasIconChanged(bool);
    void hasWriterChanged(bool);
    void hasOptionsChanged(bool);
    void hasLicenseChanged(bool);
    void enabledByDefaultChanged(bool);
    void pluginTypeChanged(int);
    void varTypesChanged();
    void createExpressionsChanged(bool);
    void inOpVarTypesChanged();
    void outOpVarTypesChanged();
    void dbTypeChanged(int);
    void filePatternsTextChanged(const QString&);
    void filePatternsStrictChanged(bool);
    void opensWholeDirectoryChanged(bool);
  private:
    XMLDocument     *xmldoc;

    QButtonGroup    *attpluginGroup;
    QGroupBox       *pluginGroup;
    QComboBox       *pluginType;
    QLineEdit       *name;
    QLineEdit       *label;
    QLineEdit       *version;
    QCheckBox       *hasIcon;
    QCheckBox       *hasWriter;
    QCheckBox       *hasOptions;
    QCheckBox       *hasLicense;
    QCheckBox       *enabledByDefault;
    QLineEdit       *iconFile;
    QGroupBox       *plotPluginGroup;
    QCheckBox       *varTypeMesh;
    QCheckBox       *varTypeScalar;
    QCheckBox       *varTypeVector;
    QCheckBox       *varTypeMaterial;
    QCheckBox       *varTypeSubset;
    QCheckBox       *varTypeSpecies;
    QCheckBox       *varTypeCurve;
    QCheckBox       *varTypeTensor;
    QCheckBox       *varTypeSymmetricTensor;
    QCheckBox       *varTypeLabel;
    QCheckBox       *varTypeArray;
    QGroupBox       *operatorPluginGroup;
    QCheckBox       *createExpressions;
    QLabel          *opVarInputType;
    QCheckBox       *inOpVarTypeMesh;
    QCheckBox       *inOpVarTypeScalar;
    QCheckBox       *inOpVarTypeVector;
    QCheckBox       *inOpVarTypeMaterial;
    QCheckBox       *inOpVarTypeSubset;
    QCheckBox       *inOpVarTypeSpecies;
    QCheckBox       *inOpVarTypeCurve;
    QCheckBox       *inOpVarTypeTensor;
    QCheckBox       *inOpVarTypeSymmetricTensor;
    QCheckBox       *inOpVarTypeLabel;
    QCheckBox       *inOpVarTypeArray;
    QLabel          *opVarOutputType;
    QCheckBox       *outOpVarTypeMesh;
    QCheckBox       *outOpVarTypeScalar;
    QCheckBox       *outOpVarTypeVector;
    QCheckBox       *outOpVarTypeMaterial;
    QCheckBox       *outOpVarTypeSubset;
    QCheckBox       *outOpVarTypeSpecies;
    QCheckBox       *outOpVarTypeCurve;
    QCheckBox       *outOpVarTypeTensor;
    QCheckBox       *outOpVarTypeSymmetricTensor;
    QCheckBox       *outOpVarTypeLabel;
    QCheckBox       *outOpVarTypeArray;
    QGroupBox       *dbPluginGroup;
    QComboBox       *dbType;
    QLineEdit       *filePatterns;
    QCheckBox       *filePatternsStrict;
    QCheckBox       *opensWholeDirectory;
};

#endif
