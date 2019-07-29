// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_PYTHON_FILTER_EDITOR_H
#define QVIS_PYTHON_FILTER_EDITOR_H

#include <gui_exports.h>
#include <QWidget>

// Forward declarations
class QTextEdit;
class QSyntaxHighlighter;
class QPushButton;
class QMenu;

// ****************************************************************************
// Class: QvisPythonFilterEditor
//
// Purpose:
//   TODO
//
// Notes:
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:31:58 PST 2010
//
// Modifications:
//
// ****************************************************************************

class GUI_API QvisPythonFilterEditor : public QWidget
{
    Q_OBJECT
public:
             QvisPythonFilterEditor(QWidget *parent= 0);
    virtual ~QvisPythonFilterEditor();

    QString getSource(bool escape=false);

    static QString templateDirectory();

signals:
    void     sourceTextChanged();
    void     templateSelected(const QString &tname);

public slots:
    void     setSource(const QString &py_script, bool escaped=false);
    void     saveScript(const QString &py_file);
    void     loadScript(const QString &py_file);


private slots:
    void     cmdSaveClick();
    void     loadMenuEvent(QAction *);

private:
    QTextEdit          *editor;
    QSyntaxHighlighter *highlighter;
    QPushButton        *cmdLoad;
    QMenu              *cmdLoadMenu;
    QPushButton        *cmdSave;

    QAction            *loadFile;
    QAction            *loadTempSimple;
    QAction            *loadTempAdv;
};

#endif

