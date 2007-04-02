/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#include <QvisDialogLineEdit.h>
#include <qfiledialog.h>
#include <qfontdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include <QvisFileOpenDialog.h>

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
//   
// ****************************************************************************

QvisDialogLineEdit::QvisDialogLineEdit(QWidget *parent, const char *name)
    : QHBox(parent, name), dialogFilter("*"), dialogCaption("Open")
{
    dialogMode = ChooseFile;
    setSpacing(0);

    lineEdit = new QLineEdit(this, "lineEdit");
    lineEdit->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
        QSizePolicy::Minimum));
    connect(lineEdit, SIGNAL(returnPressed()),
            this, SIGNAL(returnPressed()));
    connect(lineEdit, SIGNAL(textChanged(const QString &)),
            this, SIGNAL(textChanged(const QString &)));
    pushButton = new QPushButton("...",
        this, "pushButton");
#ifndef Q_WS_MACX
    pushButton->setMaximumWidth(
        fontMetrics().boundingRect("...").width() + 6);
#endif
    pushButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
        QSizePolicy::Minimum));
    connect(pushButton, SIGNAL(clicked()),
            this, SLOT(pushButtonClicked()));

    // Make the line edit take most of the space.
    setStretchFactor(lineEdit, 100);
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
    QToolTip::remove(lineEdit);
    QToolTip::add(lineEdit, s);
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
// Method: QvisDialogLineEdit::fontChange
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
//   
// ****************************************************************************

void
QvisDialogLineEdit::fontChange(const QFont &oldFont)
{
    QHBox::fontChange(oldFont);

#ifndef Q_WS_MACX
    pushButton->setMaximumWidth(
         QFontMetrics(font()).boundingRect("...").width() + 6);
    update();
#endif
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
// ****************************************************************************

void
QvisDialogLineEdit::pushButtonClicked()
{
    QString name(text());
    QFont f;

    if(dialogMode == ChooseFile)
    {
        // Choose a new filename using VisIt's file open dialog.
        name = QvisFileOpenDialog::getOpenFileName(name, dialogCaption);
    }
    else if(dialogMode == ChooseLocalFile)
    { 
        // Choose a new filename.
        name = QFileDialog::getOpenFileName(name, dialogFilter, this,
            "getFileDialog", dialogCaption);
    }
    else if(dialogMode == ChooseDirectory)
    {
        // Choose a directory.
        name = QFileDialog::getExistingDirectory(name, this,
            "getDirectoryDialog", dialogCaption);
    }
    else if(dialogMode == ChooseFont)
    {
        // Choose a directory.
        bool okay = false;
        f = QFontDialog::getFont(&okay, font(), this, "getFontDialog");
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
        QToolTip::remove(lineEdit);
        QToolTip::add(lineEdit, name);

        lineEdit->blockSignals(true);
        lineEdit->setText(name);
        lineEdit->home(false);
        lineEdit->blockSignals(false);

        emit returnPressed();
        emit textChanged(name);
    }
}

