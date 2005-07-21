#ifndef XMLEDITPLUGIN_H
#define XMLEDITPLUGIN_H

#include <qframe.h>

struct XMLDocument;
class QLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;

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
// ****************************************************************************

class XMLEditPlugin : public QFrame
{
    Q_OBJECT
  public:
    XMLEditPlugin(QWidget *p, const QString &n);
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
    void enabledByDefaultChanged(bool);
    void pluginTypeChanged(int);
    void varTypesChanged();
    void dbTypeChanged(int);
    void extensionsTextChanged(const QString&);
  private:
    XMLDocument     *xmldoc;

    QButtonGroup    *attpluginGroup;
    QComboBox       *pluginType;
    QLineEdit       *name;
    QLineEdit       *label;
    QLineEdit       *version;
    QCheckBox       *hasIcon;
    QCheckBox       *hasWriter;
    QCheckBox       *hasOptions;
    QCheckBox       *enabledByDefault;
    QLineEdit       *iconFile;
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
    QComboBox       *dbType;
    QLineEdit       *extensions;
};

#endif
