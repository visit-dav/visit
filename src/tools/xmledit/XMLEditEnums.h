#ifndef XMLEDITENUMS_H
#define XMLEDITENUMS_H

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
//  Class:  XMLEditEnums
//
//  Purpose:
//    Enum editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
class XMLEditEnums : public QFrame
{
    Q_OBJECT
  public:
    XMLEditEnums(QWidget *p, const QString &n);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void removeEmptyLines();
    void addEmptyLine();
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void UpdateWindowSingleItem();
    void nameTextChanged(const QString&);
    void valuelistChanged();
    void enumlistNew();
    void enumlistDel();
  private:
    XMLDocument     *xmldoc;

    QPushButton     *newButton;
    QPushButton     *delButton;

    QListBox        *enumlist;
    QLineEdit       *name;
    QMultiLineEdit  *valuelist;
};

#endif
