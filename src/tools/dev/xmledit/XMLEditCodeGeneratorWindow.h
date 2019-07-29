// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XMLEDIT_CODEGENERATOR_WINDOW_H
#define XMLEDIT_CODEGENERATOR_WINDOW_H
#include <QMainWindow>
#include <XMLToolIds.h>
class QProcess;
class QTextEdit;

// ****************************************************************************
// Class: XMLEditCodeGeneratorWindow
//
// Purpose:
//   This class is a window that invokes various code generation tools and 
//   prints their output to the window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 7 16:27:04 PST 2008
//
// Modifications:
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************

class XMLEditCodeGeneratorWindow : public QMainWindow
{
    Q_OBJECT
public:

    XMLEditCodeGeneratorWindow(QWidget *parent);
    virtual ~XMLEditCodeGeneratorWindow();

    void GenerateCode(const QString &, const bool tools[ID_XML_MAX]);
private slots:
    void generateOne();
    void readProcessStdout();
    void readProcessStderr();
    void cleanupProcess();
private:
    QString    xmlFile;
    bool       useTools[ID_XML_MAX];
    int        toolIndex;
    QProcess  *currentProcess;
    QTextEdit *outputText;
};

#endif
