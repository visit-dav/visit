// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
    hLayout->setContentsMargins(0,0,0,0);

    lineEdit = new QLineEdit(this);
    lineEdit->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
        QSizePolicy::Minimum));
    connect(lineEdit, SIGNAL(returnPressed()),
            this, SIGNAL(returnPressed()));
    connect(lineEdit, SIGNAL(textChanged(const QString &)),
            this, SIGNAL(textChanged(const QString &)));
    hLayout->addWidget(lineEdit);
    pushButton = new QPushButton("...", this);
#if !defined(Q_OS_MAC)
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
// Method: QvisDialogLineEdit::setFallbackPath
//
// Purpose:
//   Sets a fallback path to use if the default path in the File Open Dialog
//   does not exist.
//
// Programmer: Kathleen Bonnell
// Creation:   Fri May 13 14:07:12 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void
QvisDialogLineEdit::setFallbackPath(const QString &p)
{
    fallbackPath = p;
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
#if !defined(Q_OS_MAC)
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
//   Kathleen Bonnell, Fri May 13 14:08:08 PDT 2011
//   Use new getOpenFileNameWithFallbackPath for ChooseFile option.
//
//   Kathleen Biagas, Thu Dec 27 16:26:03 PST 2018
//   Added ChooseSaveFile dialog mode.
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
        name = QvisFileOpenDialog::getOpenFileNameWithFallbackPath(name, dialogCaption, fallbackPath);
#endif
    }
    else if(dialogMode == ChooseLocalFile)
    {
        // Choose an existing filename.
        name = QFileDialog::getOpenFileName (this, dialogCaption, name, dialogFilter);
    }
    else if(dialogMode == ChooseSaveFile)
    {
        // Choose a new filename.
        name = QFileDialog::getSaveFileName (this, dialogCaption, name, dialogFilter);
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

