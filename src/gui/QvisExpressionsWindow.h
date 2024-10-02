// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_EXPRESSIONS_WINDOW_H
#define QVIS_EXPRESSIONS_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations
class ExpressionList;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QCheckBox;
class QGroupBox;
class QLineEdit;
class QTextEdit;
class QLabel;
class QListWidget;
class QComboBox;
class QTabWidget;
class QvisVariableButton;
class QvisPythonFilterEditor;


// ****************************************************************************
// Class: QvisExpressionsWindow
//
// Purpose:
//   This class contains the widgets that manipulate expressions
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   October 10, 2004
//
// Modifications:
//    Jeremy Meredith, Mon Oct 25 11:32:14 PDT 2004
//    Reversed the sense of the "hidden" button.  Added a list-box-index to
//    expresion-list-index map so we didn't have to index expressions by name.
//
//    Brad Whitlock, Thu Dec 9 10:15:25 PDT 2004
//    Added a newExpression slot function and a variable button that lets
//    us pick variables from the active source.
//
//    Brad Whitlock, Wed Apr  9 10:55:20 PDT 2008
//    QString for caption, shortName.
//
//    Cyrus Harrison, Wed Jun 11 13:49:19 PDT 2008
//    Initial Qt4 Port.
//
//    Brad Whitlock, Fri May  6 14:42:31 PDT 2011
//    I added an override for loadSubject.
//
//    Eddie Rusu, Tue Jun 23 14:02:35 PDT 2020
//    Added FinalizeExpressionNameChange that detects when a user has finished
//    editing an expression's name to detect for duplicate expression names
//    in the gui.
// ****************************************************************************

class GUI_API QvisExpressionsWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:

    QvisExpressionsWindow(ExpressionList * exprAtts_,
                          const QString &caption = QString(),
                          const QString &shortName = QString(),
                          QvisNotepadArea * notepad = 0);
    virtual ~ QvisExpressionsWindow();
    virtual void CreateWindowContents();
  public  slots:
    virtual void apply();
    virtual void loadSubject();
    void    newExpression();
  protected:
    void    UpdateWindow(bool doAll);
    void    Apply(bool forceUpdate = false);
    void    BlockAllSignals(bool);
  private slots:
    void    addExpression();
    void    delExpression();
    void    FinalizeExpressionNameChange();

    void    typeChanged(int);
    void    displayAllVarsChanged();

    void    notHiddenChanged();

    void    stdDefinitionTextChanged();

    void    stdInsertFunction(QAction *);
    void    stdInsertVariable(const QString &);


    void    pyArgsTextChanged();
    void    pyFilterSourceChanged();
    void    pyTemplateSelected(const QString &);

    void    pyInsertFunction(QAction *);
    void    pyInsertVariable(const QString &);

    void    UpdateWindowSingleItem();
    void    UpdateWindowSensitivity();

  private:

    QString ExpandFunction(const QString &);
    QString QuoteVariable(const QString &);

    void    CreateStandardEditor();
    void    SetStandardEditorReadOnly(bool read_only);
    void    CreatePythonFilterEditor();
    void    UpdatePythonExpression();
    bool    ParsePythonExpression(const QString &, QString &, QString &);
    void    UpdateStandardExpressionEditor(const QString &);
    void    UpdatePythonExpressionEditor(const QString &);

    // Widgets and layouts.
    QListWidget        *exprListBox;

    QLabel             *nameEditLabel;
    QLabel             *typeLabel;

    QLineEdit          *nameEdit;
    QComboBox          *typeList;
    QCheckBox          *notHidden;

    QPushButton        *newButton;
    QPushButton        *delButton;

    QCheckBox          *displayAllVars;

    QTabWidget         *editorTabs;

    // widgets for standard editor
    QWidget            *stdEditorWidget;
    QLabel             *stdDefinitionEditLabel;
    QTextEdit          *stdDefinitionEdit;
    QPushButton        *stdInsertFunctionButton;
    QMenu              *stdInsertFunctionMenu;
    QvisVariableButton *stdInsertVariableButton;


    // widgets for python filter editor
    QWidget                *pyEditorWidget;
    QLabel                 *pyArgsEditLabel;
    QLineEdit              *pyArgsEdit;
    QLabel                 *pyFilterEditLabel;
    QvisPythonFilterEditor *pyFilterEdit;
    QPushButton            *pyInsertFunctionButton;
    QMenu                  *pyInsertFunctionMenu;
    QvisVariableButton     *pyInsertVariableButton;


    // State information
    ExpressionList         *exprList;
    std::map<int,int>       indexMap;
    bool                    stdExprActive;
    bool                    pyExprActive;
};

#endif
