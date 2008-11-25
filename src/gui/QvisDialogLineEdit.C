/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#include <QvisDialogLineEdit.h>
#include <QFileDialog>
#include <QFontDialog>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QEvent>

#ifndef DESIGNER_PLUGIN
#include <QvisFileOpenDialog.h>
#endif

// ****************************************************************************
// Method: QvisDialogLineEdit::QvisDialogLineEdit
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The widget's parent.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 16:24:08 PST 2006
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 11:26:42 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisDialogLineEdit::QvisDialogLineEdit(QWidget *parent)
    : QWidget(parent), dialogFilter("*"), dialogCaption("Open")
{
    dialogMode = ChooseFile;
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setSpacing(0);
    hLayout->setMargin(0);

    lineEdit = new QLineEdit(this);
    lineEdit->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
        QSizePolicy::Minimum));
    connect(lineEdit, SIGNAL(returnPressed()),
            this, SIGNAL(returnPressed()));
    connect(lineEdit, SIGNAL(textChanged(const QString &)),
            this, SIGNAL(textChanged(const QString &)));
    hLayout->addWidget(lineEdit);
    pushButton = new QPushButton("...", this);
#ifndef Q_WS_MACX
    pushButton->setMaximumWidth(
        fontMetrics().boundingRect("...").width() + 6);
#endif
    pushButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
        QSizePolicy::Minimum));
    connect(pushButton, SIGNAL(clicked()),
            this, SLOT(pushButtonClicked()));
    hLayout->addWidget(pushButton);

    // Make the line edit take most of the space.
    hLayout->setStretchFactor(lineEdit, 100);
    setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
        QSizePolicy::Minimum));
}

// ****************************************************************************
// Method: QvisDialogLineEdit::~QvisDialogLineEdit
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 16:24:46 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisDialogLineEdit::~QvisDialogLineEdit()
{
}

//
// Set/Get methods.
//

void
QvisDialogLineEdit::setText(const QString &s)
{
    lineEdit->setToolTip(s);
    lineEdit->setText(s);
}

QString
QvisDialogLineEdit::text()
{
    return lineEdit->text();
}

QString
QvisDialogLineEdit::displayText()
{
    return lineEdit->displayText();
}

void
QvisDialogLineEdit::setDialogFilter(const QString &s)
{
    dialogFilter = s;
}

void
QvisDialogLineEdit::setDialogCaption(const QString &s)
{
    dialogCaption = s;
}

void
QvisDialogLineEdit::setDialogMode(QvisDialogLineEdit::DialogMode m)
{
    dialogMode = m;
}

// ****************************************************************************
// Method: QvisDialogLineEdit::changeEvent
//
// Purpose: 
//   Update the width of the font button when the font changes.
//
// Arguments:
//   oldFont : The old font.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 15 17:03:41 PST 2007
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 11:36:17 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisDialogLineEdit::changeEvent(QEvent *e)
{
#ifndef Q_WS_MACX
    if(e->type() == QEvent::FontChange)
    {
        pushButton->setMaximumWidth(
            QFontMetrics(font()).boundingRect("...").width() + 6);
        update();
        e->accept();
    }
#endif
    QWidget::changeEvent(e);
}

//
// Qt slots
//

// ****************************************************************************
// Method: QvisDialogLineEdit::pushButtonClicked
//
// Purpose: 
//   This method is called when the "..." button is clicked and it lets the
//   user select a directory or a filename and that string gets put into
//   the text line edit.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 16:25:09 PST 2006
//
// Modifications:
//   Brad Whitlock, Fri Mar 16 15:09:38 PST 2007
//   Added support for fonts.
//
//   Brad Whitlock, Thu Jun 19 11:27:32 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisDialogLineEdit::pushButtonClicked()
{
    QString name(text());
    QFont f;

    if(dialogMode == ChooseFile)
    {
#ifdef DESIGNER_PLUGIN
        // Do this when we build the widget as a designer plugin
        name = QFileDialog::getOpenFileName (this, dialogCaption, name, dialogFilter);
#else
        // Choose a new filename using VisIt's file open dialog.
        name = QvisFileOpenDialog::getOpenFileName(name, dialogCaption);
#endif
    }
    else if(dialogMode == ChooseLocalFile)
    { 
        // Choose a new filename.
        name = QFileDialog::getOpenFileName (this, dialogCaption, name, dialogFilter);
    }
    else if(dialogMode == ChooseDirectory)
    {
        // Choose a directory.
        name = QFileDialog::getExistingDirectory(this, dialogCaption, name);
    }
    else if(dialogMode == ChooseFont)
    {
        // Choose a font.
        bool okay = false;
        f = QFontDialog::getFont(&okay, font(), this, dialogCaption);
        if(okay)
            name = f.toString();
        else
            name = QString();
    }

    //
    // If a file or directory was chosen, use it.
    //
    if(!name.isEmpty())
    {
        lineEdit->setToolTip(name);

        lineEdit->blockSignals(true);
        lineEdit->setText(name);
        lineEdit->home(false);
        lineEdit->blockSignals(false);

        emit returnPressed();
        emit textChanged(name);
    }
}

