#ifndef XMLEDITMAKEFILE_H
#define XMLEDITMAKEFILE_H

#include <qframe.h>

struct XMLDocument;
class QLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;

// ****************************************************************************
//  Class:  XMLEditMakefile
//
//  Purpose:
//    Makefile editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
// ****************************************************************************
class XMLEditMakefile : public QFrame
{
    Q_OBJECT
  public:
    XMLEditMakefile(QWidget *p, const QString &n);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void cxxflagsTextChanged(const QString&);
    void ldflagsTextChanged(const QString&);
    void libsTextChanged(const QString&);
    void gfilesTextChanged(const QString&);
    void sfilesTextChanged(const QString&);
    void vfilesTextChanged(const QString&);
    void mfilesTextChanged(const QString&);
    void efilesTextChanged(const QString&);
    void wfilesTextChanged(const QString&);
    void customgfilesChanged();
    void customsfilesChanged();
    void customvfilesChanged();
    void custommfilesChanged();
    void customefilesChanged();
    void customwfilesChanged();
    void mdSpecificCodeChanged();
  private:
    XMLDocument     *xmldoc;

    QLineEdit       *CXXFLAGS;
    QLineEdit       *LDFLAGS;
    QLineEdit       *LIBS;
    QCheckBox       *customGFiles;
    QLineEdit       *GFiles;
    QCheckBox       *customSFiles;
    QLineEdit       *SFiles;
    QCheckBox       *customVFiles;
    QLineEdit       *VFiles;
    QCheckBox       *customMFiles;
    QLineEdit       *MFiles;
    QCheckBox       *customEFiles;
    QLineEdit       *EFiles;
    QCheckBox       *customWFiles;
    QLineEdit       *WFiles;
    QCheckBox       *mdSpecificCode;
};

#endif
