// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisSimulationMessageWindow.h>

#include <QLayout>
#include <QPushButton>
#include <QTextEdit>

QvisSimulationMessageWindow::QvisSimulationMessageWindow(
    const QString &caption, const QString &shortName, QvisNotepadArea *notepad) : 
    QvisPostableWindow(caption, shortName, notepad)
{
    messages = 0;
    addLayoutStretch = false;
}

QvisSimulationMessageWindow::~QvisSimulationMessageWindow()
{
}

void
QvisSimulationMessageWindow::CreateEntireWindow()
{
    QvisPostableWindow::CreateEntireWindow();
    dismissButton->setEnabled(false);
}

void
QvisSimulationMessageWindow::CreateWindowContents()
{
    messages = new QTextEdit(central);
    messages->setReadOnly(true);
    topLayout->addWidget(messages);

    QPushButton *clearButton = new QPushButton(tr("Clear"), central);
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    clearButton->setEnabled(true);
    topLayout->addWidget(clearButton, 1);
}

//
// Qt slots
//

void
QvisSimulationMessageWindow::addMessage(const QString &txt)
{
    if(messages != 0)
        messages->append(txt);
}

void
QvisSimulationMessageWindow::clear()
{
    if(messages != 0)
        messages->clear();
}

void 
QvisSimulationMessageWindow::unpost()
{
   QvisPostableWindow::unpost();
}

void 
QvisSimulationMessageWindow::post()
{
    QvisPostableWindow::post();
}
