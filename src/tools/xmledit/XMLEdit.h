#ifndef XMLEDIT_H
#define XMLEDIT_H

#include <qmainwindow.h>
#include <qstring.h>

class QTabWidget;
struct XMLDocument;

class XMLEditAttribute;
class XMLEditMakefile;
class XMLEditPlugin;
class XMLEditEnums;
class XMLEditFields;
class XMLEditFunctions;
class XMLEditConstants;
class XMLEditIncludes;
class XMLEditCode;

// ****************************************************************************
//  Class:  XMLEdit
//
//  Purpose:
//    Main window for the XML Editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
class XMLEdit : public QMainWindow
{
    Q_OBJECT
  public:
    XMLEdit(const QString &file, QWidget *p, const QString &n);
    void OpenFile(const QString &file);
    void SaveFile(const QString &file);
  public slots:
    void newdoc();
    void open();
    void save();
    void saveAs();
    void updateTab(QWidget *tab);
  private:
    QString  filename;
    QTabWidget  *tabs;
    XMLDocument *xmldoc;

    XMLEditPlugin    *plugintab;
    XMLEditMakefile  *makefiletab;
    XMLEditAttribute *attributetab;
    XMLEditEnums     *enumstab;
    XMLEditFields    *fieldstab;
    XMLEditFunctions *functionstab;
    XMLEditConstants *constantstab;
    XMLEditIncludes  *includestab;
    XMLEditCode      *codetab;
};

#endif
