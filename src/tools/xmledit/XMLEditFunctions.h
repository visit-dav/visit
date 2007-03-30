#ifndef XMLEDITFUNCTIONS_H
#define XMLEDITFUNCTIONS_H

#include <qframe.h>

struct XMLDocument;
class QLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;
class QListBox;
class QMultiLineEdit;
class QRadioButton;
class QPushButton;

// ****************************************************************************
//  Class:  XMLEditFunctions
//
//  Purpose:
//    Function editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
class XMLEditFunctions : public QFrame
{
    Q_OBJECT
  public:
    XMLEditFunctions(QWidget *p, const QString &n);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void UpdateWindowSingleItem();
    void nameTextChanged(const QString&);
    void typeGroupChanged(int);
    void memberChanged();
    void declarationTextChanged(const QString&);
    void definitionChanged();
    void functionlistNew();
    void functionlistDel();
  private:
    XMLDocument    *xmldoc;

    QPushButton    *newButton;
    QPushButton    *delButton;

    QListBox       *functionlist;
    QLineEdit      *name;
    QButtonGroup   *typeGroup;
    QRadioButton   *newFunctionButton;
    QRadioButton   *replaceBuiltinButton;
    QCheckBox      *member;
    QLineEdit      *declaration;
    QMultiLineEdit *definition;
};

#endif
