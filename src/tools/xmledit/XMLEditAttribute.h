#ifndef XMLEDITATTRIBUTE_H
#define XMLEDITATTRIBUTE_H

#include <qframe.h>

struct XMLDocument;
class QLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;

// ****************************************************************************
//  Class:  XMLEditAttribute
//
//  Purpose:
//    Attribute editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
class XMLEditAttribute : public QFrame
{
    Q_OBJECT
  public:
    XMLEditAttribute(QWidget *p, const QString &n);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void nameTextChanged(const QString&);
    void purposeTextChanged(const QString&);
    void codefileTextChanged(const QString&);
    void exportAPITextChanged(const QString&);
    void exportIncludeTextChanged(const QString&);
    void persistentChanged();
  private:
    XMLDocument     *xmldoc;

    QLineEdit     *name;
    QLineEdit     *purpose;
    QLineEdit     *codefile;
    QLineEdit     *exportAPI;
    QLineEdit     *exportInclude;
    QCheckBox     *persistent;
};

#endif
