// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisSimulationCommandWindow.h>

#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTextEdit>

#define CMD_BUTTONS_PER_ROW 3

#define MAX_CMD_BUTTONS 500

QvisSimulationCommandWindow::QvisSimulationCommandWindow(
    const QString &caption, const QString &shortName, QvisNotepadArea *notepad) : 
    QvisPostableWindow(caption, shortName, notepad)
{
    commandGroup = 0;
    addLayoutStretch = false;
    CreateEntireWindow();
}

QvisSimulationCommandWindow::~QvisSimulationCommandWindow()
{
}

void
QvisSimulationCommandWindow::CreateEntireWindow()
{
    QvisPostableWindow::CreateEntireWindow();
    dismissButton->setEnabled(false);
}

void
QvisSimulationCommandWindow::CreateWindowContents()
{
    // Create the group box and generic buttons.
    commandGroupBox = new QGroupBox(tr("Commands"), central);
    topLayout->addWidget(commandGroupBox);
    QVBoxLayout *vLayout = new QVBoxLayout(commandGroupBox);

    // Make the button to activate the custom GUI.
    QWidget *h = new QWidget(commandGroupBox);
    vLayout->addWidget(h);
    QHBoxLayout *hLayout = new QHBoxLayout(h);
    hLayout->setContentsMargins(0,0,0,0);
    activateCustomGUI = new QPushButton(tr("Activate Custom UI . . ."), h);
    connect(activateCustomGUI, SIGNAL(clicked()),
            this, SIGNAL(showCustomUIWindow()));
    hLayout->addStretch(10);
    hLayout->addWidget(activateCustomGUI);
    hLayout->addStretch(10);
    activateCustomGUI->setVisible(false);

    // Make the generic command buttons.
    commandButtonParent = new QWidget(commandGroupBox);

    QScrollArea *sa = new QScrollArea(commandGroupBox);
    vLayout->addWidget(sa);
    vLayout->addSpacing(5);
    sa->setWidget(commandButtonParent);
    sa->setWidgetResizable(true);
    sa->setMinimumHeight(150);
    sa->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QVBoxLayout *vb = new QVBoxLayout(commandButtonParent);
    vb->setContentsMargins(0,0,0,0);
    commandGroup = new QButtonGroup(commandButtonParent);
    commandButtonLayout = new QGridLayout(0);
    vb->addLayout(commandButtonLayout);
    vb->addStretch(10);
    commandButtonLayout->setContentsMargins(0,0,0,0);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(commandGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(handleCommandButton(int)));
#else
    connect(commandGroup, SIGNAL(idClicked(int)),
            this, SLOT(handleCommandButton(int)));
#endif
    bool added = false;
    EnsureButtonExists(5, added);

    // Create time controls.
    timeGroup = new QGroupBox(tr("Enable time cycle ranging"), central);
    timeGroup->setCheckable(true);
    timeGroup->setChecked(false);
    connect(timeGroup, SIGNAL(toggled(bool)),
            this, SLOT(handleTimeRanging(bool)));
    topLayout->addWidget(timeGroup);
    
    QGridLayout *timeLayout = new QGridLayout(timeGroup);
    startCycle = new QLineEdit(timeGroup);
    startLabel = new QLabel(timeGroup);
    startLabel->setText(tr("Start"));
    startCycle->setText(tr("0"));
    timeLayout->addWidget(startLabel,0,0);
    timeLayout->addWidget(startCycle,0,1);
    connect(startCycle,SIGNAL(textChanged(const QString &)),
            this,SLOT(handleStart(const QString&)));

    stepCycle = new QLineEdit(timeGroup);
    stepLabel = new QLabel(timeGroup);
    stepLabel->setText(tr("Step"));
    stepCycle->setText(tr("1"));
    timeLayout->addWidget(stepLabel,0,2);
    timeLayout->addWidget(stepCycle,0,3);
    connect(stepCycle,SIGNAL(textChanged(const QString &)),
            this,SLOT(handleStep(const QString&)));
    
    stopCycle = new QLineEdit(timeGroup);
    stopLabel = new QLabel(timeGroup);
    stopLabel->setText(tr("Stop"));
    stopCycle->setText(tr("0"));
    timeLayout->addWidget(stopLabel,0,4);
    timeLayout->addWidget(stopCycle,0,5);
    connect(stopCycle,SIGNAL(textChanged(const QString &)),
            this,SLOT(handleStop(const QString&)));
}

int
QvisSimulationCommandWindow::numCommandButtons() const
{
    return (commandGroup == 0) ? 0 : commandGroup->buttons().count();
}

bool
QvisSimulationCommandWindow::setButtonCommand(int index, const QString &cmd)
{
    bool added = false;
    if(EnsureButtonExists(index, added))
        commandGroup->buttons().at(index)->setText(cmd);
    return added;
}

bool
QvisSimulationCommandWindow::setButtonEnabled(int index, bool enabled, bool clearText)
{
    bool added = false;
    if(EnsureButtonExists(index, added))
    {
        QAbstractButton *b = commandGroup->buttons().at(index);
        b->setEnabled(enabled);
        if(!enabled && clearText)
            b->setText("");
    }
    return added;
}

bool
QvisSimulationCommandWindow::EnsureButtonExists(int index, bool &added)
{
    added = false;
    if(index < 0)
        return false;
    if(index > MAX_CMD_BUTTONS)
        return false;

    if(index >= numCommandButtons())
    {
        // We need to make more buttons.
        int newIndex = numCommandButtons();
        while(newIndex <= index)
        {
            int r = newIndex / CMD_BUTTONS_PER_ROW;
            int c = newIndex % CMD_BUTTONS_PER_ROW;

            QPushButton *b = new QPushButton("", commandButtonParent);
            commandButtonLayout->addWidget(b, r + 1,c);
            commandGroup->addButton(b, newIndex);

            ++newIndex;
            added = true;
        }
    }

    return true;
}

void
QvisSimulationCommandWindow::setCustomButtonEnabled(bool value)
{
    activateCustomGUI->setVisible(value);
}

void
QvisSimulationCommandWindow::setTimeValues(bool timeRanging, 
    const QString &start, const QString &stop, const QString &step)
{
    timeGroup->setChecked(timeRanging);
    startCycle->setText(start);
    stopCycle->setText(stop);
    stepCycle->setText(step);
}

void
QvisSimulationCommandWindow::setTimeRanging(bool timeRanging)
{
    timeGroup->setChecked(timeRanging);
}

void
QvisSimulationCommandWindow::setTimeStart(const QString &start)
{
    startCycle->setText(start);
}

void
QvisSimulationCommandWindow::setTimeStep(const QString &step)
{
    stepCycle->setText(step);
}

void
QvisSimulationCommandWindow::setTimeStop(const QString &stop)
{
    stopCycle->setText(stop);
}

//
// Qt slots
//

void
QvisSimulationCommandWindow::handleCommandButton(int btn)
{
    emit executeButtonCommand(commandGroup->button(btn)->text());
}

void
QvisSimulationCommandWindow::handleTimeRanging(bool b)
{
    QString value(tr("%1").arg(b));
    if(!value.isEmpty())
        emit timeRangingToggled(value);
}

void
QvisSimulationCommandWindow::handleStart(const QString &text)
{
    QString value(startCycle->text().trimmed());
    if(!value.isEmpty())
        emit executeStart(value);
}

void
QvisSimulationCommandWindow::handleStop(const QString &text)
{
    QString value(stopCycle->text().trimmed());
    if(!value.isEmpty())
        emit executeStop(value);
}

void
QvisSimulationCommandWindow::handleStep(const QString &text)
{
    QString value(stepCycle->text().trimmed());
    if(!value.isEmpty())
        emit executeStep(value);
}

void 
QvisSimulationCommandWindow::unpost()
{
    QvisPostableWindow::unpost();
}

void 
QvisSimulationCommandWindow::post()
{
    QvisPostableWindow::post();
}
