#ifndef XMLEDITINCLUDES_H
#define XMLEDITINCLUDES_H

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
//  Class:  XMLEditIncludes
//
//  Purpose:
//    Include editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
class XMLEditIncludes : public QFrame
{
    Q_OBJECT
  public:
    XMLEditIncludes(QWidget *p, const QString &n);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void UpdateWindowSingleItem();
    void includeTextChanged(const QString&);
    void fileGroupChanged(int);
    void quotedGroupChanged(int);
    void includelistNew();
    void includelistDel();
  private:
    XMLDocument    *xmldoc;

    QPushButton    *newButton;
    QPushButton    *delButton;

    QListBox       *includelist;
    QRadioButton   *CButton;
    QRadioButton   *HButton;
    QRadioButton   *quotesButton;
    QRadioButton   *bracketsButton;
    QButtonGroup   *fileGroup;
    QButtonGroup   *quotedGroup;
    QLineEdit      *file;
};

#endif
