#ifndef XMLEDITCONSTANTS_H
#define XMLEDITCONSTANTS_H

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
//  Class:  XMLEditConstants
//
//  Purpose:
//    Constant editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
class XMLEditConstants : public QFrame
{
    Q_OBJECT
  public:
    XMLEditConstants(QWidget *p, const QString &n);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void UpdateWindowSingleItem();
    void nameTextChanged(const QString&);
    void memberChanged();
    void declarationTextChanged(const QString&);
    void definitionChanged();
    void constantlistNew();
    void constantlistDel();
  private:
    XMLDocument     *xmldoc;

    QPushButton    *newButton;
    QPushButton    *delButton;

    QListBox       *constantlist;
    QLineEdit      *name;
    QCheckBox      *member;
    QLineEdit      *declaration;
    QMultiLineEdit *definition;
};

#endif
