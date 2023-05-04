// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "VisItPasswordWindow.h"

#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

// ****************************************************************************
//  Constructor:  VisItPasswordWindow::VisItPasswordWindow
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 26, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Apr 15 11:22:44 PDT 2002
//    Added an ok button.
//
//    Jeremy Meredith. Tue Dec  9 15:16:52 PST 2003
//    Added a cancel button, as well as supported Rejected functionality.
//
//    Brad Whitlock, Mon Feb 23 15:07:41 PST 2004
//    Added space between the password line edit and the cancel button.
//
//    Hank Childs, Sun Nov 11 22:21:55 PST 2007
//    Add support for changing the username.
//
//    Brad Whitlock, Tue Apr 29 15:09:31 PDT 2008
//    Added tr()'s
//
//    Brad Whitlock, Tue May 27 13:41:28 PDT 2008
//    Qt 4.
//
// ****************************************************************************

VisItPasswordWindow::VisItPasswordWindow(QWidget *parent) : QDialog(parent)
{
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10,10,10,10);

    QHBoxLayout *l2 = new QHBoxLayout;
    layout->addLayout(l2);
    l2->setSpacing(5);
    label = new QLabel(tr("Password for localhost: "), this);
    l2->addWidget(label);

    passedit = new QLineEdit(this);
    passedit->setEchoMode(QLineEdit::Password);
    l2->addWidget(passedit);
    connect(passedit, SIGNAL(returnPressed()), this, SLOT(accept()));
    layout->addSpacing(20);

    QHBoxLayout *l3 = new QHBoxLayout;
    layout->addLayout(l3);
    QPushButton *okay = new QPushButton(tr("OK"), this);
    connect(okay, SIGNAL(clicked()), this, SLOT(accept()));
    l3->addWidget(okay);
    l3->addStretch(10);

    QPushButton *cub = new QPushButton(tr("Change username"), this);
    connect(cub, SIGNAL(clicked()), this, SLOT(changeUserName()));
    l3->addWidget(cub);
    l3->addStretch(10);

    QPushButton *cancel = new QPushButton(tr("Cancel"), this);
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    l3->addWidget(cancel);

    setWindowTitle(tr("Enter password"));

    this->changedUserName = false;
}

// ****************************************************************************
//  Destructor:  VisItPasswordWindow::~VisItPasswordWindow
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 26, 2001
//
// ****************************************************************************

VisItPasswordWindow::~VisItPasswordWindow()
{
}

// ****************************************************************************
// Method: VisItPasswordWindow::getPassword
//
// Purpose: 
//   Pops up the password window and returns the password string.
//
// Arguments:
//   username   : The user's login name.
//   host       : The name of the host for which we're getting the password.
//   passphrase : True if we're doing a passphrase. False if password.
//   ret        : The return code.
//
// Returns:    The password string.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 12 11:13:23 PDT 2012
//
// Modifications:
//
// ****************************************************************************

QString
VisItPasswordWindow::getPassword(const QString &username,
                                 const QString &host,
                                 QString phrase,
                                 VisItPasswordWindow::ReturnCode &ret)
{
    this->changedUserName = false;

    this->setWindowTitle(QString("Enter %1").arg(phrase));

    // Set the password prompt.
    QString labelText;
    this->label->setTextFormat(Qt::RichText);
    if(username.isEmpty() || username == QString("notset"))
        labelText = QString("%1 for %2: ").arg(phrase).arg(host);
    else
    {
        // "nobr" means no line breaks, which is how it worked when we weren't
        // using rich text.
        labelText = QString("<nobr>%1 for <font color=\"red\">%2</font>@%3: </nobr>").
                           arg(phrase).arg(username).arg(host);
    }
    this->label->setText(labelText);

    // Make the password window be the active window.
    this->topLevelWidget()->activateWindow();
    this->topLevelWidget()->raise();

    // Clear the password.
    this->passedit->clear();

    // Give focus to the password window.
    QTimer::singleShot(300, this->passedit, SLOT(setFocus()));

    // Enter the local event loop for the dialog.
    int status = this->exec();

    // Return the password string.
    QString pass;
    if (status == Accepted)
    {
        // Accepted; hit return or Okay.
        pass = this->passedit->text();
        ret = PW_Accepted;
    }
    else 
    {
        ret = this->changedUserName ? PW_ChangedUsername : PW_Rejected;
    }

    return pass;
}

// ****************************************************************************
// Method: VisItPasswordWindow::changeUsername
//
// Purpose:
//    A slot for changing the user name.
//
// Programmer: Hank Childs
// Creation:   November 10, 2007
//
// ****************************************************************************

void
VisItPasswordWindow::changeUserName(void)
{
    this->changedUserName = true;
    reject();
}
