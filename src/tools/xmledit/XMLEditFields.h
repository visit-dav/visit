#ifndef XMLEDITFIELDS_H
#define XMLEDITFIELDS_H

#include <qframe.h>

struct XMLDocument;
class QLineEdit;
class QNarrowLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;
class QListBox;
class QTextEdit;
class QMultiLineEdit;
class QPushButton;

// ****************************************************************************
//  Class:  XMLEditFields
//
//  Purpose:
//    Field editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
class XMLEditFields : public QFrame
{
    Q_OBJECT
  public:
    XMLEditFields(QWidget *p, const QString &n);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void UpdateWindowSingleItem();
    void UpdateTypeList();
    void UpdateEnablerList();
    void nameTextChanged(const QString&);
    void labelTextChanged(const QString&);
    void typeChanged(int);
    void subtypeTextChanged(const QString&);
    void enablerChanged(int);
    void enablevalTextChanged(const QString&);
    void lengthTextChanged(const QString&);
    void internalChanged();
    void ignoreeqChanged();
    void initChanged();
    void valuesChanged();
    void fieldlistNew();
    void fieldlistDel();
    void fieldlistUp();
    void fieldlistDown();
  private:
    XMLDocument     *xmldoc;

    QPushButton     *newButton;
    QPushButton     *delButton;

    QPushButton     *upButton;
    QPushButton     *downButton;

    QListBox        *fieldlist;
    QLineEdit       *name;
    QLineEdit       *label;
    QComboBox       *type;
    QLineEdit       *subtype;
    QComboBox       *enabler;
    QLineEdit       *enableval;
    QNarrowLineEdit *length;
    QCheckBox       *internal;
    QCheckBox       *ignoreeq;
    QCheckBox       *init;
    QMultiLineEdit  *values;
};

#endif
