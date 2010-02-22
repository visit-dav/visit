/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

#include <QvisPythonFilterEditor.h>
#include <QtCore>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QFileDialog>

#include <QvisPythonSyntaxHighlighter.h>
#include <visit-config.h>
#include <sstream>
#include <Environment.h>

#include <iostream>
using namespace std;

// ****************************************************************************
// Method: QvisPythonFilterEditor::QvisPythonFilterEditor
//
// Purpose:
//   This is the constructor for the QvisPythonFilterEditor class.
//
// Arguments:
//   parent: Parent Widget.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//
// ****************************************************************************

QvisPythonFilterEditor::QvisPythonFilterEditor(QWidget *parent)
: QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    editor = new QTextEdit(this);
    highlighter = new QvisPythonSyntaxHighlighter(editor->document());

    QHBoxLayout *button_layout = new QHBoxLayout();
    cmdLoad = new QPushButton(tr("Load Script"),this);

    cmdLoadMenu = new QMenu(this);

    QMenu *loadTempsMenu    = cmdLoadMenu->addMenu(tr("&Template"));

    loadFile       = cmdLoadMenu->addAction(tr("&File"));
    loadTempSimple = loadTempsMenu->addAction(tr("&Simple Filter"));
    loadTempAdv    = loadTempsMenu->addAction(tr("&Advanced Filter"));

    cmdLoad->setMenu(cmdLoadMenu);

    cmdSave = new QPushButton(tr("Save Script"),this);


    button_layout->addWidget(cmdLoad);
    button_layout->addWidget(cmdSave);
    button_layout->addStretch(10);

    layout->addWidget(editor);
    layout->addLayout(button_layout);

    connect(editor, SIGNAL(textChanged()),
            this, SIGNAL(sourceTextChanged()));

    connect(cmdSave, SIGNAL(clicked()),
            this, SLOT(cmdSaveClick()));

    connect(cmdLoadMenu, SIGNAL(triggered(QAction *)),
            this, SLOT(loadMenuEvent(QAction *)));

}

// ****************************************************************************
// Method: QvisPythonFilterEditor::~QvisPythonFilterEditor
//
// Purpose:
//   The destructor for the QvisPythonFilterEditor class.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//
// ****************************************************************************

QvisPythonFilterEditor::~QvisPythonFilterEditor()
{

}

// ****************************************************************************
// Method: QvisPythonFilterEditor::getSource
//
// Purpose:
//   Gets the script from the source editor.
//   Optionally escapes the output.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//
// ****************************************************************************

QString
QvisPythonFilterEditor::getSource(bool escape)
{
    // TODO test escape
    QString res = editor->toPlainText();
    res = res.trimmed() + QString("\n");

    if(escape)
    {
        res.replace(QString("\""),QString("\\\""));
        res.replace(QString("\n"),QString("\\n"));
    }

    return res;
}

// ****************************************************************************
// Method: QvisPythonFilterEditor::setSource
//
// Purpose:
//   Sets the script in the source editor.
//   Optionally handles escaped input.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisPythonFilterEditor::setSource(const QString &py_script, bool escaped)
{
    if(escaped)
    {
        QString res = py_script;
        res.replace(QString("\\\""),QString("\""));
        res.replace(QString("\\n"),QString("\n"));
        editor->setText(res);
    }
    else
        editor->setText(py_script);
}


// ****************************************************************************
// Method: QvisPythonFilterEditor::saveScript
//
// Purpose:
//   Saves a script to a file.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisPythonFilterEditor::saveScript(const QString &py_script)
{
    QFile file(py_script);
    if ( file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        QTextStream stream( &file );
        stream << getSource();
        file.close();
    }
    /*
    else
    ERROR
    */
}

// ****************************************************************************
// Method: QvisPythonFilterEditor::loadScript
//
// Purpose:
//   Loads a script from a file.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisPythonFilterEditor::loadScript(const QString &py_script)
{
    QFile file(py_script);
    if ( file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        QTextStream stream( &file );
        setSource(stream.readAll());
        file.close();
    }
    /*
        else
        ERROR
    */
}


// ****************************************************************************
// Method: QvisPythonFilterEditor::cmdSaveClick
//
// Purpose:
//   Slot handler for save click.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisPythonFilterEditor::cmdSaveClick()
{
    QString default_file = QDir::current().path()
                           + "/" + QString("visit_filter.py");

    // Get the name of the file that the user saved.
    QString filter(tr("Python Script File") +  QString(" (*.py)"));

    QString res = QFileDialog::getSaveFileName(this,
                                               tr("Save Python Filter Script"),
                                               default_file,
                                               filter);
    if(!res.isNull())
        saveScript(res);
}

// ****************************************************************************
// Method: QvisPythonFilterEditor::loadMenuEvent
//
// Purpose:
//   Slot handler for load menu actions.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisPythonFilterEditor::loadMenuEvent(QAction *action)
{
    if(action == loadFile)
    {
        QString filter(tr("Python Script File") +  QString(" (*.py)"));
        QString res = QFileDialog::getOpenFileName(this,
                                                   tr("Load Python Filter"),
                                                   QDir::current().path(),
                                                   filter);
        if(!res.isNull())
            loadScript(res);
    }
    else if(action == loadTempSimple)
        emit templateSelected(QString("Simple"));
    else if(action == loadTempAdv)
        emit templateSelected(QString("Advanced"));
}

// ****************************************************************************
// Method: QvisPythonFilterEditor::templateDirectory
//
// Purpose:
//   Provides the path to the filter template directory. 
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//
// ****************************************************************************

QString
QvisPythonFilterEditor::templateDirectory()
{
    QString res(Environment::get("VISITARCHHOME").c_str());
    res += QString(VISIT_SLASH_CHAR)
           + QString("lib")
           + QString(VISIT_SLASH_CHAR)
           + QString("site-packages")
           + QString(VISIT_SLASH_CHAR)
           + QString("pyavt-templates")
           + QString(VISIT_SLASH_CHAR);

    return res;
}


