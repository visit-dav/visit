/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
// ****************************************************************************

class GUI_API QvisExpressionsWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:

    QvisExpressionsWindow(ExpressionList * exprAtts_,
                          const QString &caption = QString::null,
                          const QString &shortName = QString::null,
                          QvisNotepadArea * notepad = 0);
    virtual ~ QvisExpressionsWindow();
    virtual void CreateWindowContents();
  public  slots:
    virtual void apply();
    void    newExpression();
  protected:
    void    UpdateWindow(bool doAll);
    void    Apply(bool forceUpdate = false);
    void    BlockAllSignals(bool);
  private slots:
    void    addExpression();
    void    delExpression();
    void    nameTextChanged(const QString&);

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
    void    CreatePythonFilterEditor();
    void    UpdatePythonExpression();
    bool    ParsePythonExpression(const QString &, QString &, QString &);
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
    bool                    pyExprActive;
};

#endif
