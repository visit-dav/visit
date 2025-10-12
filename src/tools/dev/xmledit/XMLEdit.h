// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XMLEDIT_H
#define XMLEDIT_H

#include <QMainWindow>
#include <QString>

class QTabWidget;
class XMLDocument;

class XMLEditAttribute;
class XMLEditCMake;
class XMLEditPlugin;
class XMLEditEnums;
class XMLEditFields;
class XMLEditFunctions;
class XMLEditConstants;
class XMLEditIncludes;
class XMLEditCode;
class XMLEditConditional;
class XMLEditCodeGeneratorWindow;

// ****************************************************************************
//  Class:  XMLEdit
//
//  Purpose:
//    Main window for the XML Editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Mar 7 15:02:37 PST 2008
//    Added code generation window.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
//    Kathleen Biagas, Tue Sep 30 14:41:58 PDT 2025
//    Rename XMLEditMakefile to XMLEditCMake.
//    Add XMLEditConditional.
//
// ****************************************************************************
//
class XMLEdit : public QMainWindow
{
    Q_OBJECT
  public:
    XMLEdit(const QString &file, QWidget *p);
    void OpenFile(const QString &file);
    void SaveFile(const QString &file);
  public slots:
    void newdoc();
    void open();
    void save();
    void saveAs();
    void updateTab(int tab);
    void generateCode();
  private:
    QString  filename;
    QTabWidget  *tabs;
    XMLDocument *xmldoc;

    XMLEditPlugin    *plugintab;
    XMLEditCMake     *cmaketab;
    XMLEditAttribute *attributetab;
    XMLEditEnums     *enumstab;
    XMLEditFields    *fieldstab;
    XMLEditFunctions *functionstab;
    XMLEditConstants *constantstab;
    XMLEditIncludes  *includestab;
    XMLEditCode      *codetab;
    XMLEditConditional *conditionaltab;

    XMLEditCodeGeneratorWindow *codeGenerationWindow;
};

#endif
