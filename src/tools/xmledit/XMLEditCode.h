#ifndef XMLEDITCODE_H
#define XMLEDITCODE_H

#include <qframe.h>

struct XMLDocument;
class QLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;
class QListBox;
class QMultiLineEdit;
class QPushButton;

// ****************************************************************************
//  Class:  XMLEditCode
//
//  Purpose:
//    Code snippet editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
class XMLEditCode : public QFrame
{
    Q_OBJECT
  public:
    XMLEditCode(QWidget *p, const QString &n);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void UpdateWindowSingleItem();
    void nameTextChanged(const QString&);
    void prefixChanged();
    void postfixChanged();
    void codelistNew();
    void codelistDel();
  private:
    XMLDocument     *xmldoc;

    QPushButton    *newButton;
    QPushButton    *delButton;

    QListBox       *codelist;
    QLineEdit      *name;
    QMultiLineEdit *prefix;
    QMultiLineEdit *postfix;
};

#endif
