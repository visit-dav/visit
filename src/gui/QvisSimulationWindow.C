// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisSimulationWindow.h>

#include <DebugStream.h>
#include <EngineList.h>
#include <EngineProperties.h>
#include <SimulationUIValues.h>
#include <StatusAttributes.h>
#include <ViewerProxy.h>
#include <avtSimulationInformation.h>
#include <avtSimulationCommandSpecification.h>
#include <QualifiedFilename.h>

#include <QvisSimulationCommandWindow.h>
#include <QvisSimulationMessageWindow.h>
#include <QvisStripChartMgr.h>
#include <QvisStripChartTabWidget.h>
#include <QvisStripChart.h>
#include <QvisNotepadArea.h>
#include <QvisUiLoader.h>
#include <SimCommandSlots.h>

#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDial>
#include <QFile>
#include <QGroupBox>
#include <QLCDNumber>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QMetaMethod>
#include <QMetaObject>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QTextEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTreeWidget>

#include <string>
#include <time.h>

// ****************************************************************************
// Method: QvisSimulationWindow::QvisSimulationWindow
//
// Purpose:
//   This is the constructor for the QvisSimulationWindow class.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Shelly Prevost, Tue Jan 24 17:06:49 PST 2006
//   Added a custom simulation control window.
//
//   Brad Whitlock, Wed Apr  9 11:50:52 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************
QvisSimulationWindow::QvisSimulationWindow(EngineList *engineList,
                                           const QString &caption,
                                           const QString &shortName,
                                           QvisNotepadArea *notepad) :
  QvisPostableWindowObserver(engineList, caption, shortName, notepad,
                             QvisPostableWindowObserver::NoExtraButtons),
  activeEngine(""), statusMap()
{
    engines = engineList;
    caller = engines;
    statusAtts = 0;
    metadata = new avtDatabaseMetaData;
    uiValues = NULL;
    CustomUIWindow = NULL;
    uiLoader = new QvisUiLoader;
    stripChartMgr = 0;
    simMessages = 0;
}

// ****************************************************************************
// Method: QvisSimulationWindow::~QvisSimulationWindow
//
// Purpose:
//   This is the destructor for the QvisSimulationWindow class.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//
// ****************************************************************************
QvisSimulationWindow::~QvisSimulationWindow()
{
    // Delete the status attributes in the status map.
    SimulationStatusMap::Iterator pos;

    for (pos = statusMap.begin(); pos != statusMap.end(); ++pos)
    {
        delete pos.value();
    }

    // Detach from the status atts if they are still around.
    if (statusAtts)
        statusAtts->Detach(this);

    // Detach from the sim ui values
    if (uiValues)
        uiValues->Detach(this);

    delete stripChartMgr;
}

// ****************************************************************************
// Method: QvisSimulationWindow::CreateWindowContents
//
// Purpose:
//   Creates the widgets for the window.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Shelly Prevost, Tue Jan 24 17:06:49 PST 2006
//   Added a custom simulation control window.
//
//   Brad Whitlock, Tue Jan 31 16:04:07 PST 2006
//   I moved the generic button creation code back to this method.
//
//   Shelly Prevost, Tue Sep 12 14:53:40 PDT 2006
//   I added a new set of time range text boxes and activation checkbox.
//   I Added an message box to display simulation code messages.
//
//   Shelly Prevost Tue Nov 28 17:01:58 PST 2006
//   I added the strip chart widget to the main simulatioin window.
//
//   Shelly Prevost Fri Apr 13 14:03:03 PDT 2007
//   added splitter to help with widow space issues.
//
//   Shelly Prevost Fri Oct 12 15:19:40 PDT 2007
//   modified splitter to help with widow space issues.
//   Factored out strip chart window widgets and replaced them
//   with a strip chart window manager.
//
// ****************************************************************************
void
QvisSimulationWindow::CreateWindowContents()
{
    isCreated = true;

    QGridLayout *grid1 = new QGridLayout(0);
    topLayout->addLayout(grid1);
    grid1->setColumnStretch(1, 10);

    simCombo = new QComboBox(central);
    connect(simCombo, SIGNAL(activated(int)), this, SLOT(selectEngine(int)));
    grid1->addWidget(simCombo, 0, 1);
    QLabel *engineLabel = new QLabel(tr("Simulation"), central);
    engineLabel->setBuddy(simCombo);
    grid1->addWidget(engineLabel, 0, 0);

    // Create the widgets needed to show the engine information.
    simInfo = new QTreeWidget(central);
    simInfo->setColumnCount(2);
    simInfo->setHeaderLabels(QStringList(tr("Attribute")) +
                             QStringList(tr("Value")));
    simInfo->setAllColumnsShowFocus(true);
    simInfo->setAlternatingRowColors(true);
//    simInfo->setResizeMode(QTreeView::AllColumns);
    grid1->addWidget(simInfo, 1, 0, 1, 2);

    // Create status displays
    QGridLayout *statusLayout = new QGridLayout(0);
    topLayout->addLayout(statusLayout);
    simulationMode = new QLabel(tr("Not connected"), central);
    statusLayout->addWidget(new QLabel(tr("Simulation status"), central), 0, 0);
    statusLayout->addWidget(simulationMode, 0, 1, 1, 3);

    totalProgressBar = new QProgressBar(central);
    totalProgressBar->setMaximum(100);
    statusLayout->addWidget(new QLabel(tr("VisIt status"), central), 1, 0);
    statusLayout->addWidget(totalProgressBar, 1, 1, 1, 3);

    // Create engine buttons for interruption, etc.
    interruptEngineButton = new QPushButton(tr("Interrupt"), central);
    connect(interruptEngineButton, SIGNAL(clicked()), this, SLOT(interruptEngine()));
    interruptEngineButton->setEnabled(false);
    statusLayout->addWidget(interruptEngineButton, 2, 1);

    closeEngineButton = new QPushButton(tr("Disconnect"), central);
    connect(closeEngineButton, SIGNAL(clicked()), this, SLOT(closeEngine()));
    closeEngineButton->setEnabled(false);
    statusLayout->addWidget(closeEngineButton, 2, 3);

    clearCacheButton = new QPushButton(tr("Clear cache"), central);
    connect(clearCacheButton, SIGNAL(clicked()), this, SLOT(clearCache()));
    clearCacheButton->setEnabled(false);
    statusLayout->addWidget(clearCacheButton, 2, 2);

    // Create the notepad area.
    QvisNotepadArea *notepadAux = new QvisNotepadArea(central);
    topLayout->addWidget(notepadAux, 10);

    // Create the command window and post it to the notepad.
    simCommands = new QvisSimulationCommandWindow(tr("Simulation controls"),
        tr("Controls"), notepadAux);
    simCommands->post();
    connect(simCommands, SIGNAL(executeButtonCommand(const QString &)),
            this, SLOT(executePushButtonCommand(const QString &)));
    connect(simCommands, SIGNAL(showCustomUIWindow()),
            this, SLOT(showCustomUIWindow()));

    connect(simCommands, SIGNAL(executeStart(const QString &)),
            this, SLOT(executeStartCommand(const QString &)));
    connect(simCommands, SIGNAL(executeStop(const QString &)),
            this, SLOT(executeStopCommand(const QString &)));
    connect(simCommands, SIGNAL(executeStep(const QString &)),
            this, SLOT(executeStepCommand(const QString &)));
    connect(simCommands, SIGNAL(timeRangingToggled(const QString &)),
            this, SLOT(executeEnableTimeRange(const QString &)));

    // Create the simulation message window and post it to the notepad.
    simMessages = new QvisSimulationMessageWindow(tr("Simulation messages"),
        tr("Messages"), notepadAux);
    simMessages->post();

    // Create the strip chart manager and post it to the notepad.
    int index = GetEngineListIndex(activeEngine);
    stripChartMgr =
      new QvisStripChartMgr(this, GetViewerProxy(), engines, index, notepadAux);
    stripChartMgr->post();

    // Make sure we show the commands page.
    notepadAux->showPage(simCommands);
}

// ****************************************************************************
// Method: ConnectUIChildren
//
// Purpose:
//   Connect the custon Ui gui signals to the availble slots
//   for an object's children.
//
// Arguments:
//   obj : The Qt object
//   cc  : Simulation command slots
//
// Returns:    Connect slots to the object.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::ConnectUIChildren(QObject *obj)
{
    // Connect up handlers to all signals based on component type.
    const QObjectList &GUI_Objects = obj->children();

    for (int i = 0; i < GUI_Objects.size(); ++i)
    {
        QObject *ui = GUI_Objects[i];

        ConnectUI(ui);
    }
}

// ****************************************************************************
// Method: ConnectUI
//
// Purpose:
//   Connect the custon Ui gui signals to the availble slots
//   for an object.
//
// Arguments:
//   obj : The Qt object
//   cc  : Simulation command slots
//
// Returns:    Connect slots to the object.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::ConnectUI(QObject *ui)
{
    SimCommandSlots *cc = CommandConnections;

    const QMetaObject *mo = ui->metaObject();

#if 0
    const QMetaObject *ccmo = cc->metaObject();

    for(int m = 0; m < uimo->methodCount(); ++m)
    {
        QMetaMethod uimm = uimo->method(m);

        if(uimm.methodType() == QMetaMethod::Signal)
        {
            for(int n = 0; n < ccmo->methodCount(); ++n)
            {
                QMetaMethod ccmm = ccmo->method(n);
                if(ccmm.methodType() == QMetaMethod::Slot &&
                   strcmp(uimm.signature(), ccmm.signature()) == 0)
                {
                    connect(ui, uimm.signature(),
                            cc, ccmm.signature());
                }
            }
        }
    }

    if(mm.methodType() == QMetaMethod::Method)
      qDebug("    %d: method: %s", m, mm.signature());
    else if(mm.methodType() == QMetaMethod::Signal)
      qDebug("    %d: signal: %s", m, mm.signature());
    else if(mm.methodType() == QMetaMethod::Slot)
      qDebug("    %d: slot:   %s", m, mm.signature());
    else if(mm.methodType() == QMetaMethod::Constructor)
      qDebug("    %d: ctor:   %s", m, mm.signature());
#endif

#if 0
    // Useful for getting slot signature
    for(int m = 0; m < mo->methodCount(); ++m)
    {
        QMetaMethod mm = mo->method(m);

        if(mm.methodType() == QMetaMethod::Signal)
        {
          if( ui->objectName().toStdString() == "ListWidget" )
            std::cerr << ui->objectName().toStdString() << "  "
                      << mm.methodSignature().constData() << "  "
                      << std::endl;
        }
    }
#endif

    if (mo->indexOfSignal("clicked()") != -1)
    {
        QObject::connect(ui, SIGNAL(clicked()),
                         cc, SLOT(ClickedHandler()));
    }

    if (mo->indexOfSignal("toggled(bool)") != -1)
    {
        QObject::connect(ui, SIGNAL(toggled(bool)),
                         cc, SLOT(ToggledHandler(bool)));
    }

    if (mo->indexOfSignal("triggered(QAction*)") != -1)
    {
        QObject::connect(ui, SIGNAL(triggered(QAction *)),
                         cc, SLOT(TriggeredHandler(QAction *)));
    }

    if (mo->indexOfSignal("valueChanged(int)") != -1)
    {
        QObject::connect(ui, SIGNAL(valueChanged(int)),
                         cc, SLOT(ValueChangedHandler(int)));
    }

    if (mo->indexOfSignal("valueChanged(const QDate&)") != -1)
    {
        QObject::connect(ui, SIGNAL(valueChanged(const QDate&)),
                         cc, SLOT(ValueChangedHandler(const QDate &)));
    }

    if (mo->indexOfSignal("valueChanged(const QTime&)") != -1)
    {
        QObject::connect(ui, SIGNAL(valueChanged(const QTime&)),
                         cc, SLOT(ValueChangedHandler(const QTime &)));
    }

    if (mo->indexOfSignal("stateChanged(int)") != -1)
    {
        //qDebug("connect %s stateChanged(int)\n", ui->objectName().toStdString().c_str());
        QObject::connect(ui, SIGNAL(stateChanged(int)),
                         cc, SLOT(StateChangedHandler(int)));
    }

    if (mo->indexOfSignal("activated(int)") != -1)
    {
        //qDebug("connect %s activated(int)\n", ui->objectName().toStdString().c_str());
        QObject::connect(ui, SIGNAL(activated(int)),
                         cc, SLOT(ActivatedHandler(int)));
    }

    if (mo->indexOfSignal("textChanged(QString)") != -1)
    {
        QObject::connect(ui, SIGNAL(textChanged(const QString &)),
                         cc, SLOT(TextChangedHandler(const QString&)));
    }

    // List widget related
    if (mo->indexOfSignal("currentRowChanged(int)") != -1)
    {
        QObject::connect(ui, SIGNAL(currentRowChanged(int)),
                         cc, SLOT(CurrentRowChangedHandler(int)));
    }

    if (mo->indexOfSignal("currentTextChanged(QString)") != -1)
    {
        QObject::connect(ui, SIGNAL(currentTextChanged(QString)),
                         cc, SLOT(CurrentTextChangedHandler(QString)));
    }

    if (mo->indexOfSignal("itemChanged(QListWidgetItem*)") != -1)
    {
        QObject::connect(ui, SIGNAL(itemChanged(QListWidgetItem*)),
                         cc, SLOT(ItemChangedHandler(QListWidgetItem*)));
    }

    // Table widget related
    if (mo->indexOfSignal("cellChanged(int,int)") != -1)
    {
        QObject::connect(ui, SIGNAL(cellChanged(int, int)),
                         cc, SLOT(CellChangedHandler(int, int)));
    }

    // Combobox related
    if (mo->indexOfSignal("currentIndexChanged(int)") != -1)
    {
        QObject::connect(ui, SIGNAL(currentIndexChanged(int)),
                         cc, SLOT(CurrentIndexChangedHandler(int)));
    }

    // We've hooked up signals for this object, now do its children.
    ConnectUIChildren(ui);
}

// ****************************************************************************
// Method: void QvisSimulationWindow::CreateCustomUIWindowdow
//
// Purpose:
//   Updates the ui components in the Custom UI popup. It check for matches
//   between ui updates sent from the simulations to ui components in the
//   custom ui popup. If it finds a match it update the ui component.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//   Brad Whitlock, Tue Jan 31 16:33:45 PST 2006
//   I renamed the method and moved a bunch of its code into helper methods.
//
//   Shelly Prevost Tue Jun 19 16:10:17 PDT 2007
//   Added signal connection for row col table widget.
//
// ****************************************************************************
void
QvisSimulationWindow::CreateCustomUIWindow()
{
    // qDebug("QvisSimulationWindow::CreateCommandUI");

    // Try and get the name of the UI file.
    QString fname(GetUIFile(activeEngine));

    if(fname.isEmpty())
    {
        if(CustomUIWindow != NULL)
        {
            delete CustomUIWindow;
            CustomUIWindow = NULL;
        }

        simCommands->setCustomButtonEnabled(false);

        clearStripCharts();

        return;
    }

    debug5 << "UI_DIR = " << fname.toStdString() << endl;
    // qDebug("QvisSimulationWindow::CreateCommandUI: creating ui");

    // Dynamically create the custom UI be reading in it's xml file.
    int index = GetEngineListIndex(activeEngine);
    CommandConnections = new SimCommandSlots(GetViewerProxy(), engines, index);

    QFile uiFile(fname);
    if(uiFile.open(QIODevice::ReadOnly))
        CustomUIWindow = uiLoader->load(&uiFile, NULL);

    // If creation failed then jump out
    if (CustomUIWindow == NULL)
    {
        simCommands->setCustomButtonEnabled(false);
        QString msg = tr("VisIt could not locate the simulation's "
                         "user interface creation file at: %1. The custom user "
                         "interface for this simulation will be unavailable.").
                      arg(fname);
        Error(msg);
        return;
    }

    CustomUIWindow->setWindowTitle(simCombo->currentText());
    // qDebug("QvisSimulationWindow::CreateCommandUI: hooking up ui");

    // Hook up the widget and its children to the command connections
    // object which will translate its signals into commands that we
    // will send to the simulation.
    ConnectUIChildren(CustomUIWindow);

    // enable custom command UI button
    debug5 << "enabling custom command button" << endl;
    simCommands->setCustomButtonEnabled(true);
    debug5 << "successfully added simulation interface" << endl;
}

// ****************************************************************************
// Method: QvisSimulationWindow::Update
//
// Purpose:
//   This method is called when the subjects that the window observes are
//   modified.
//
// Arguments:
//   TheChangedSubject : A pointer to the subject that changed.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::Update(Subject *TheChangedSubject)
{
    caller = TheChangedSubject;

    if (isCreated)
        UpdateWindow(false);
}

// ****************************************************************************
// Method: QvisSimulationWindow::SubjectRemoved
//
// Purpose:
//   This method is called when the subjects observed by the window are
//   destructed.
//
// Arguments:
//   TheRemovedSubject : A pointer to the subject to remove.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if (TheRemovedSubject == engines)
        engines = 0;
    else if (TheRemovedSubject == statusAtts)
        statusAtts = 0;
}

// ****************************************************************************
// Method: QvisSimulationWindow::UpdateUIComponent
//
// Purpose:
//   Updates the ui components in the Custom UI popup. It check for matches
//   between ui updates sent from the simulations to ui components in the
//   custom ui popup. If it finds a match it update the ui component.
//
// Arguments:
//   window : The parent window that contains the ui
//   name   : The name of the widget to be updated.
//   value  : If applicable the value for the widge
//   e      : Boolean to enable/disable the gui
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//   Brad Whitlock, Tue Jan 31 16:53:30 PST 2006
//   I made the method return early if the dynamic commands window has not
//   yet been created. It's now created more on-demand.
//
//   Shelly Prevost, Tue Sep 12 14:53:40 PDT 2006
//   I generalized this function to work with both the custom GUI
//   and this window. I also modified the QTextWidget to not append the
//   text if it is equal to "". This fixes the problem of not being able
//   to shut off messages.
//
// ****************************************************************************
void
QvisSimulationWindow::UpdateUIComponent(QWidget *window, const QString &name,
                                        const QString &value, bool e)
{
    QObject *ui = window->findChild<QWidget *>(name);

    if (ui)
    {
        debug5 << "Looking up component = "
               << name.toStdString().c_str() << endl;

        // Block signals so updating the user interface does not cause a
        // command to go back to the simulation.
        ui->blockSignals(true);

        if (ui->inherits("QWidget"))
        {
            ((QWidget *)ui)->setEnabled(e);

            if( value == QString("HIDE_WIDGET") )
              ((QWidget *)ui)->hide();
            else //if( value == QString("SHOW_WIDGET") )
              ((QWidget *)ui)->show();
        }

        if (ui->inherits("QLabel"))
        {
            debug5 << "found QLabel " << name.toStdString()
                   << " label = " << value.toStdString() << endl;
            ((QLabel*)ui)->setText(value);
        }

        if (ui->inherits( "QLineEdit"))
        {
            debug5 << "found QLineEdit " << name.toStdString()
                   << " text = " << value.toStdString() << endl;
            ((QLineEdit*)ui)->setText(value );
        }

        if (ui->inherits("QRadioButton"))
        {
            debug5 << "found QRadioButton " << name.toStdString()
                   << " checked = " <<  (value == "1" ? "true" : "false")
                   << endl;
            ((QRadioButton*)ui)->setChecked( value == "1" );
        }

        if (ui->inherits("QPushButton"))
        {
            debug5 << "found QPushButton " << name.toStdString()
                   << " menu items = " << value.toStdString() << endl;

            // One can optionally add a menu to push button. This step
            // comes after the push button has been constructed and is
            // dynamic.

            // Check to see if a menu has already been added if so re-use it.
            QMenu *menu = ((QPushButton*)ui)->menu();

            if( menu )
            {
              menu->clear();
            }
            else
            {
              menu = new QMenu( name );
              menu->setObjectName( name );

              ConnectUI(menu);
            }

            // Add all of the actions to the menu.
            std::string actions = value.toStdString();

            while( !actions.empty() )
            {
              std::string action = getNextString( actions, "|" );

              menu->addAction( QString( action.c_str() ) );
            }

            ((QPushButton*)ui)->setMenu(menu);
        }

        if (ui->inherits("QProgressBar"))
        {
            debug5 << "found ProgressBar " << name.toStdString()
                   << " value = " << value.toStdString() << endl;
            ((QProgressBar*)ui)->setValue(value.toInt());
        }

        if (ui->inherits("QSpinBox"))
        {
            debug5 << "found QSpinBox " << name.toStdString()
                   << " value = " << value.toStdString() << endl;
            ((QSpinBox*)ui)->setValue(value.toInt());
        }

        if (ui->inherits("QDial"))
        {
            debug5 << "found QDial " << name.toStdString()
                   << " value = " << value.toStdString() << endl;
            ((QDial*)ui)->setValue(value.toInt());
        }

        if (ui->inherits("QSlider"))
        {
            debug5 << "found QSlider " << name.toStdString()
                   << " value = " << value.toStdString() << endl;
            ((QSlider*)ui)->setValue(value.toInt());
        }

        if (ui->inherits("QTextEdit"))
        {
            debug5 << "found QTextEdit " << name.toStdString()
                   << " text = " << value.toStdString() << endl;
            ((QTextEdit*)ui)->setText(value);
        }

        if ( ui->inherits ( "QLCDNumber"))
        {
            debug5 << "found QLCDNumber " << name.toStdString()
                   << " value = " << value.toStdString() << endl;
            ((QLCDNumber*)ui)->display(value);
        }

        if ( ui->inherits ( "QTimeEdit"))
        {
            debug5 << "found QTimeEdit " << name.toStdString()
                   << " time = " << value.toStdString() << endl;
            QTime time1 = QTime::fromString(value);
            ((QTimeEdit*)ui)->setTime(time1);
        }

        if (ui->inherits("QDateEdit"))
        {
            debug5 << "found QDateEdit " << name.toStdString()
                   << " date = " << value.toStdString() << endl;
            QDate date = QDate::fromString( value );
            ((QDateEdit*)ui)->setDate(date);
        }

        if (ui->inherits("QCheckBox"))
        {
            debug5 << "found QCheckBox " << name.toStdString()
                   << " checked = " << (value == "1" ? "true" : "false")
                   << endl;
            ((QCheckBox*)ui)->setChecked( value == "1" );
        }

        if (ui->inherits("QGroupBox"))
        {
            debug5 << "found QGroupBox " << name.toStdString()
                   << " checked = " <<  (value == "1" ? "true" : "false")
                   << endl;
            ((QGroupBox*)ui)->setChecked( value == "1" );
        }

        if (ui->inherits("QComboBox"))
        {
            // For ComboBox one can change the menu items
            // dynamically. So check to see if the string value is an
            // int or characters.
            bool okay;
            int index = value.toInt( &okay );

            // The string value is an int so set the current index.
            if( okay )
            {
              debug5 << "found QComboBox " << name.toStdString()
                     << " current index = " << value.toInt() << endl;

              ((QComboBox*)ui)->setCurrentIndex(index);
            }
            // The string value is character so clear the drop menu
            // and add the new entries.
            else
            {
              debug5 << "found QComboBox " << name.toStdString()
                     << " menu items = " << value.toStdString() << endl;

              ((QComboBox*)ui)->clear();
              ((QComboBox*)ui)->addItems( value.split('|') );
            }
        }

        if (ui->inherits("QListWidget"))
        {
            QListWidget* lWidget = ((QListWidget*)ui);

            lWidget->setEnabled(true);

            unsigned int row, editable;
            std::string text;

            parseCompositeCMD( value.toStdString(), row, text, editable );

            debug5 << "found QListWidget " << name.toStdString()
                   << " row = " << row << " with text = " << text << std::endl;

            if( text == std::string("CLEAR_LIST") )
            {
              while( lWidget->count() )
              {
                QListWidgetItem *item =
                  lWidget->takeItem( lWidget->count() - 1 );

                if( item )
                  delete item;
              }
            }
            else if( text == std::string("CURRENT_ROW") &&
                     row < lWidget->count() )
            {
              lWidget->setCurrentRow( row );
            }
            else if( text == std::string("REMOVE_ROW") &&
                     row < lWidget->count() )
            {
              QListWidgetItem *item = lWidget->takeItem( row );

              if( item )
                delete item;
            }
            else
            {
              QListWidgetItem *item = lWidget->item(row);

              // See if the item has already been created.
              if( item )
              {
                // Update the text
                item->setText(tr("%1").arg(text.c_str()));
              }
              else
              {
                // Insert blank items if the count is less than the
                // row. Otherwise the item will be inserted at the end
                // of the list.
                while( lWidget->count() < row )
                  lWidget->addItem(QString(""));

                // Create a new item and insert it into the list
                item = new QListWidgetItem(tr("%1").arg(text.c_str()));

                lWidget->insertItem(row, item);
              }

              // Is the item editable?
              if( editable )
                item->setFlags( Qt::ItemIsEditable |
                                Qt::ItemIsEnabled |
                                Qt::ItemIsSelectable );
              // Is the item enabled?
              else if( e )
                item->setFlags( Qt::ItemIsEnabled |
                                Qt::ItemIsSelectable );
              else
                item->setFlags(Qt::NoItemFlags);
            }
        }

        if (ui->inherits("QTableWidget"))
        {
            QTableWidget* tWidget = ((QTableWidget*)ui);

            tWidget->setEnabled(true);

            unsigned int row, column;
            std::string text;

            parseCompositeCMD( value.toStdString(), row, column, text );

            debug5 << "found QTableWidget " << name.toStdString()
                   << " row = " << row << " column = " << column
                   << " with text = " << text << std::endl;

            // NOTE : It is not clear if when removing a row or column
            // if the items are deleted. Possible memory leak?
            if( text == std::string("CLEAR_TABLE") )
            {
              while( tWidget->rowCount() )
                tWidget->removeRow( tWidget->rowCount() - 1);
            }
            else if( text == std::string("REMOVE_ROW") &&
                     row < tWidget->rowCount() )
            {
              tWidget->removeRow( row );
            }
            else if( text == std::string("REMOVE_COLUMN") &&
                     column <= 0 && column < tWidget->columnCount() )
            {
              tWidget->removeColumn( column );
            }
            else
            {
              QTableWidgetItem *item = tWidget->item(row, column);

              // See if the item has already been created.
              if( item )
              {
                // Update the text
                item->setText(tr("%1").arg(text.c_str()));
              }
              else
              {
                // Create a new item and make sure there is room for it.
                item = new QTableWidgetItem(tr("%1").arg(text.c_str()));

                if( tWidget->rowCount() <= row )
                  tWidget->setRowCount(row+1);

                if( tWidget->columnCount() <= column )
                  tWidget->setColumnCount(column+1);

                tWidget->setItem(row, column, item);
              }

              // Is the item editable/enabled?
              if( e )
                item->setFlags( Qt::ItemIsEditable |
                                Qt::ItemIsEnabled |
                                Qt::ItemIsSelectable );
              else
                item->setFlags(Qt::NoItemFlags);
            }
        }

        // Unblock signals.
        ui->blockSignals(false);
    }
    else
        debug5 << "Could not find UI component named "
               << name.toStdString() << endl;
}

// ****************************************************************************
// Method: QvisSimulationWindow::ConnectStatusAttributes
//
// Purpose:
//   Connects the status attributes subject that the window will observe.
//
// Arguments:
//   s : A pointer to the status attributes that the window will observe.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::ConnectStatusAttributes(StatusAttributes *s)
{
    if (s)
    {
        statusAtts = s;
        statusAtts->Attach(this);
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::ConnectSimulationUIValues
//
// Purpose:
//   Connects the UI value subject that the window will observe.
//
// Arguments:
//   s : A pointer to the UI value that the window will observe.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::ConnectSimulationUIValues(SimulationUIValues *s)
{
    if (s)
    {
        uiValues = s;
        uiValues->Attach(this);
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::UpdateWindow
//
// Purpose:
//   This method is called to update the window's widgets when the subjects
//   change.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Brad Whitlock, Tue Jan 31 17:35:31 PST 2006
//   I added code to delete the dynamic commands window if we've created it
//   before and there are no hosts. If it is allowed to persist then it
//   causes a crash when you go to click on it because of its SimCommandSlots
//   object.
//
// ****************************************************************************
void
QvisSimulationWindow::UpdateWindow(bool doAll)
{
    if (caller == engines || doAll)
    {
        const stringVector &host = engines->GetEngineName();
        const stringVector &sim  = engines->GetSimulationName();

        // If we have zero hosts but we have a dynamic command window then
        // we must have disconnected from the simulation. Blow away the
        // dynamic command window.
        if(host.size() < 1 )
        {
          if( CustomUIWindow != NULL)
          {
            delete CustomUIWindow;
            CustomUIWindow = NULL;
            simCommands->setCustomButtonEnabled(false);
          }

          clearMessages();
          clearStripCharts();
        }

        // Add the engines to the widget.
        simCombo->blockSignals(true);
        simCombo->clear();
        int index = -1;

        for (size_t i = 0; i < host.size(); ++i)
        {
            if (!sim[i].empty())
            {
                int lastSlashPos = QString(sim[i].c_str()).lastIndexOf('/');
                QString newsim = QString(sim[i].substr(lastSlashPos+1).c_str());
                int lastDotPos =  newsim.lastIndexOf('.');
                int firstDotPos =  newsim.indexOf('.');

                QString name = newsim.mid(firstDotPos+1,
                                          lastDotPos-firstDotPos-1);

                simCombo->
                  addItem(tr("%1 on %2").arg(name).arg(host[i].c_str()));

                QString key = MakeKey(host[i], sim[i]);
                if (key == activeEngine)
                    index = i;
            }
        }

        // The active engine is in the list of engines returned by the
        // viewer so update the combo box index to its index.
        if (index >= 0)
        {
            simCombo->setCurrentIndex(index);
        }
        // The active engine was not in the list of engines returned
        // by the viewer.
        else
        {
            // Remove the entry from the list.
            RemoveStatusEntry(activeEngine);
            RemoveMetaDataEntry(activeEngine);
            activeEngine = QString("");

            // If other engines are running switch to the first one.
            if (simCombo->count() > 0)
            {
                index = 0;
                simCombo->setCurrentIndex(index);

                QString key = MakeKey(host[index], sim[index]);

                // ARS - FIXME How do we know that the statusAtts and
                // metadata belong to the first engine?

                // Add current status
                AddStatusEntry(key, *statusAtts);
                // Add current metadata
                AddMetaDataEntry(key, *metadata);

                // Select the engine.
                selectEngine(index);
            }
        }

        simCombo->blockSignals(false);

        // Update the engine information.
        UpdateInformation();

        // Set the enabled state of the various widgets.
        interruptEngineButton->setEnabled(simCombo->count() > 0);
        closeEngineButton->setEnabled(simCombo->count() > 0);
        clearCacheButton->setEnabled(simCombo->count() > 0);
        simCombo->setEnabled(simCombo->count() > 0);
    }

    if (caller == statusAtts || doAll)
    {
        QString key(statusAtts->GetSender().c_str());

        if (key != QString("viewer"))
        {
            debug5 << "Status being updated" << endl;
            UpdateStatusEntry(key, *statusAtts);

            // If the sender of the status message is the engine that we're
            // currently looking at, update the status widgets.
            if (key == activeEngine)
            {
                UpdateStatusArea();
            }
        }
    }

    if(caller == uiValues || doAll)
    {
      std::string host = uiValues->GetHost();

      if (host == "localhost")
        host = GetViewerProxy()->GetLocalHostName();

      std::string path = uiValues->GetSim();
      QString key(MakeKey(host, path));

      // If the sender of the uiValues is the engine that we're
      // currently looking at, update the status widgets.
      if (key == activeEngine)
      {
          debug5 << "uiValues being updated "
                 << uiValues->GetName() << "  " << uiValues->GetSvalue()
                 << std::endl;

          if(uiValues->GetName() == "SIMULATION_ENABLE_BUTTON")
          {
            // Use activeEngine to get the metadata
            SimulationMetaDataMap::Iterator pos;

            if ((pos = metadataMap.find(activeEngine)) != metadataMap.end())
            {
              const avtDatabaseMetaData *md = pos.value();

              // Loop through all of the buttons to find the one being
              // enabled/disabled.
              int nButtons = simCommands->numCommandButtons();

              for (int c=0; c<nButtons; ++c)
              {
                avtSimulationCommandSpecification cmd =
                  md->GetSimInfo().GetGenericCommands(c);

                avtSimulationCommandSpecification::CommandArgumentType t =
                  cmd.GetArgumentType();

                if (t == avtSimulationCommandSpecification::CmdArgNone)
                {
                  QString bName = QString(cmd.GetName().c_str());

                  if( bName == QString(uiValues->GetSvalue().c_str() ) )
                  {
                    bool enabled = uiValues->GetEnabled();

                    simCommands->setButtonEnabled(c, enabled, false);
                  }
                }
              }
            }
          }

          else if(uiValues->GetName() == "SIMULATION_TIME_LIMITS_ENABLED")
          {
            simCommands->setTimeRanging( uiValues->GetSvalue() == "1" );
          }
          else if(uiValues->GetName() == "SIMULATION_TIME_START_CYCLE")
          {
            simCommands->setTimeStart( QString(uiValues->GetSvalue().c_str()) );
          }
          else if(uiValues->GetName() == "SIMULATION_TIME_STEP_CYCLE")
          {
            simCommands->setTimeStep( QString(uiValues->GetSvalue().c_str()) );
          }
          else if(uiValues->GetName() == "SIMULATION_TIME_STOP_CYCLE")
          {
            simCommands->setTimeStop( QString(uiValues->GetSvalue().c_str()) );
          }
          else if(uiValues->GetName() == "SIMULATION_MESSAGE_BOX")
          {
            QString msg = QString(uiValues->GetSvalue().c_str());

            // Post the message to the user.
            if (QMessageBox::warning(this, "VisIt", msg, QMessageBox::Ok ) ==
                QMessageBox::Ok)
              return;
          }

          else if(uiValues->GetName() == "SIMULATION_MODE")
          {
            QString mode = QString(uiValues->GetSvalue().c_str());
            simulationMode->setText(mode);
          }

          else if(uiValues->GetName() == "SIMULATION_MESSAGE")
          {
            QString message = QString(uiValues->GetSvalue().c_str());
            QString error = QString("<span style=\"color:#000000;\">%1</span>").arg(message);
            simMessages->addMessage(error);
          }

          else if(uiValues->GetName() == "SIMULATION_MESSAGE_WARNING")
          {
            QString message = QString(uiValues->GetSvalue().c_str());
            QString warning = QString("<span style=\" color:#aaaa00;\">%1</span>").arg(message);
            simMessages->addMessage(warning);
          }

          else if(uiValues->GetName() == "SIMULATION_MESSAGE_ERROR")
          {
            QString message = QString(uiValues->GetSvalue().c_str());
            QString error = QString("<span style=\" color:#aa0000;\">%1</span>").arg(message);
            simMessages->addMessage(error);
          }

          else if(uiValues->GetName() == "SIMULATION_MESSAGE_CLEAR")
          {
            clearMessages();
          }

          else if(uiValues->GetName() == "STRIP_CHART_CLEAR_ALL")
          {
            stripChartMgr->clearAll();
          }

          else if(uiValues->GetName() == "STRIP_CHART_CLEAR_MENU")
          {
            stripChartMgr->clearMenu();
          }

          else if(uiValues->GetName() == "STRIP_CHART_ADD_MENU_ITEM")
          {
            stripChartMgr->addMenuItem( uiValues->GetSvalue().c_str() );
          }

          else if(uiValues->GetName() == "STRIP_CHART_CLEAR")
          {
            unsigned int chart = atoi( uiValues->GetSvalue().c_str() );
            stripChartMgr->clear( chart );
          }

          else if(uiValues->GetName() == "STRIP_CHART_SET_NAME_OLD")
          {
            unsigned int chart, index;
            std::string text;

            parseCompositeCMD( uiValues->GetSvalue(), chart, index, text );

            if( index == 0 )
              stripChartMgr->setTabLabel(chart, text.c_str() );
            else
              stripChartMgr->setCurveTitle(chart, index-1, text.c_str() );
          }

          else if(uiValues->GetName() == "STRIP_CHART_SET_NAME")
          {
            unsigned int chart;
            std::string text;

            parseCompositeCMD( uiValues->GetSvalue(), chart, text );

            stripChartMgr->setTabLabel(chart, text.c_str() );
          }

          else if(uiValues->GetName() == "STRIP_CHART_SET_CURVE_NAME")
          {
            unsigned int chart, curve;
            std::string text;

            parseCompositeCMD( uiValues->GetSvalue(), chart, curve, text );

            stripChartMgr->setCurveTitle(chart, curve, text.c_str() );
          }

          else if(uiValues->GetName() == "STRIP_CHART_ADD_POINT")
          {
            unsigned int chart, curve;
            double x, y;

            parseCompositeCMD( uiValues->GetSvalue(), chart, curve, x, y );

            stripChartMgr->addDataPoint(chart, curve, x, y);
          }
          else if(uiValues->GetName() == "STRIP_CHART_ADD_POINTS")
          {
            unsigned int chart, curve, npts;
            double *x, *y;

            parseCompositeCMD( uiValues->GetSvalue(),
                               chart, curve, npts, x, y );

            stripChartMgr->addDataPoints(chart, curve, npts, x, y);
          }
          else if(CustomUIWindow != 0)
          {
            UpdateUIComponent(CustomUIWindow,
                              uiValues->GetName().c_str(),
                              uiValues->GetSvalue().c_str(),
                              uiValues->GetEnabled());
          }
      }
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::UpdateStatusArea
//
// Purpose:
//   Updates the window so it reflects the status information for the
//   currently selected engine.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::UpdateStatusArea()
{
    SimulationStatusMap::Iterator pos;

    if ((pos = statusMap.find(activeEngine)) == statusMap.end())
        return;

    StatusAttributes *s = pos.value();
    if (s->GetClearStatus())
    {
        s->SetStatusMessage("");
        totalProgressBar->reset();
    }
    else
    {
        int total;
        if (s->GetMaxStage() > 0)
        {
            float pct0  = float(s->GetPercent()) / 100.;
            float pct1  = float(s->GetCurrentStage()-1) / float(s->GetMaxStage());
            float pct2  = float(s->GetCurrentStage())   / float(s->GetMaxStage());
            total = int(100. * ((pct0 * pct2) + ((1.-pct0) * pct1)));
        }
        else
            total = 0;

        // Set the progress bar percent done.
        totalProgressBar->setValue(total);
   }
}

// ****************************************************************************
// Method: QvisSimulationWindow::UpdateInformation
//
// Purpose:
//   Updates the engine information.
//
// Arguments:
//   index : The index of the engine to update. The index is into EngineList.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Jeremy Meredith, Thu Apr 28 18:05:36 PDT 2005
//   Added concept of disabled control command.
//   Added concept of a simulation mode (e.g. running/stopped).
//
//   Shelly Prevost, Tue Jan 24 17:06:49 PST 2006
//   Added a custom simulation control window.
//
//   Brad Whitlock, Tue Jan 31 16:32:36 PST 2006
//   Added code to set the enabled state of the custom command button.
//
//   Shelly Prevost, Tue Sep 12 15:05:31 PDT 2006
//   Remove hard coded number of buttons and use a definded const instead.
//   Added Update SimulationUi call now that the main window needs to be
//   updated also.
//
//   Shelly Prevost, Tue Nov 28 17:12:04 PST 2006
//   Removed hard code button number
//
//   Brad Whitlock, Fri Mar 9 17:10:40 PST 2007
//   Made it use new metadata api.
//
//   Brad Whitlock, Fri Nov 19 10:48:34 PDT 2010
//   I made it use activeEngine to get the metadata.
//
//   Jean M. Favre, Mon Nov 28 13:04:28 PST 2011
//   Use current time in some cases.
//
//   Kathleen Biagas, Thu Jan 21, 2021
//   Replace QString.asprintf with QString.setNum.
//
// ****************************************************************************
void
QvisSimulationWindow::UpdateInformation()
{
    debug5 << "Update Information was called" << endl;

    // Clear the sim information.
    simInfo->clear();

    // Use activeEngine to get the metadata
    SimulationMetaDataMap::Iterator pos;

    if ((pos = metadataMap.find(activeEngine)) == metadataMap.end())
    {
        // We did not find metadata for activeEngine so just clear the window
        simInfo->setEnabled(false);

        for (int c=0; c<simCommands->numCommandButtons(); ++c)
        {
            simCommands->setButtonEnabled(c, false, true);
        }

        // FIX ME - clear msg, strip charts, and customUI
    }
    else
    {
        // Which EngineList index matches activeEngine?
        int index = GetEngineListIndex(activeEngine);
        int np = 1;
        std::string sim;
        if(index >= 0 && (size_t)index < engines->GetEngineName().size())
        {
            sim = engines->GetSimulationName()[index];
            np = engines->GetProperties(index).GetNumProcessors();
        }

        // Use the metadata we looked up to populate the window.
        const avtDatabaseMetaData *md = pos.value();
        QString tmp1,tmp2;
        QTreeWidgetItem *item = NULL;

        // Host
        item = new QTreeWidgetItem(simInfo,
                                   QStringList(tr("Host")) +
                                   QStringList(md->GetSimInfo().GetHost().c_str()));

        item->type(); // No-op to avoid a warning.

        // Simulation name
        int lastSlashPos = QString(sim.c_str()).lastIndexOf('/');
        QString newsim = QString(sim.substr(lastSlashPos+1).c_str());
        int lastDotPos =  newsim.lastIndexOf('.');
        int firstDotPos =  newsim.indexOf('.');
        QString name = newsim.mid(firstDotPos+1,
                                  lastDotPos-firstDotPos-1);
        item = new QTreeWidgetItem(simInfo,
                                   QStringList(tr("Name")) +
                                   QStringList(name));

        // Date
        QString timesecstr = newsim.left(firstDotPos);
        time_t timesec = timesecstr.toInt();
        if(timesec == 0)
            timesec = time(NULL);
        QString timestr = ctime(&timesec);
        item = new QTreeWidgetItem(simInfo,
            QStringList(tr("Date")) +
            QStringList(timestr.left(timestr.length()-1)));

        // Num processors
        tmp1.setNum(np);
        item = new QTreeWidgetItem(simInfo,
                                   QStringList(tr("Num Processors")) +
                                   QStringList(tmp1));

        // Other values from the .sim2 file
        const std::vector<std::string> &names  =
          md->GetSimInfo().GetOtherNames();

        const std::vector<std::string> &values =
          md->GetSimInfo().GetOtherValues();

        for (size_t i=0; i<names.size(); ++i)
        {
            item = new QTreeWidgetItem(simInfo,
                                       QStringList(names[i].c_str()) +
                                       QStringList(values[i].c_str()));
        }

        // Status
        debug5 << "Updating Status information" << endl;
        switch (md->GetSimInfo().GetMode())
        {
          case avtSimulationInformation::Unknown:
            simulationMode->setText(tr("Unknown"));
            break;
          case avtSimulationInformation::Running:
            simulationMode->setText(tr("Running"));
            break;
          case avtSimulationInformation::Stopped:
            simulationMode->setText(tr("Stopped"));
            break;
        }

        // Update the message window
        QString message = QString(md->GetSimInfo().GetMessage().c_str());
        simMessages->addMessage(message);

        // Update command buttons
        bool updateSize = false;
        int nButtonsToUpdate = std::max(simCommands->numCommandButtons(),
                                        md->GetSimInfo().GetNumGenericCommands());
        for (int c=0; c < nButtonsToUpdate; ++c)
        {
            if (c >= md->GetSimInfo().GetNumGenericCommands())
            {
                updateSize |= simCommands->setButtonEnabled(c, false, true);
            }
            else
            {
                avtSimulationCommandSpecification cmd =
                  md->GetSimInfo().GetGenericCommands(c);

                avtSimulationCommandSpecification::CommandArgumentType t =
                  cmd.GetArgumentType();

                if (t == avtSimulationCommandSpecification::CmdArgNone)
                {
                    QString bName = QString(cmd.GetName().c_str());
                    bool e = cmd.GetEnabled();

                    updateSize |= simCommands->setButtonCommand(c, bName);
                    simCommands->setButtonEnabled(c, e, false);
                }
                else
                {
                    updateSize |= simCommands->setButtonEnabled(c, false, true);
                }
            }
        }
        if(updateSize)
        {
            simCommands->adjustSize();
        }

        // If we've not created a dynamic commands window already and
        // we can get a decent-looking UI filename, enabled the custom
        // command button so we can create a window when that button
        // is clicked.
        if(CustomUIWindow == NULL)
        {
            QString fname(GetUIFile(activeEngine));

            simCommands->setCustomButtonEnabled(!fname.isEmpty());
        }

        simInfo->setEnabled(true);
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::AddStatusEntry
//
// Purpose:
//   Adds an engine to the internal status map.
//
// Arguments:
//   key : The name of the engine to add to the map.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::AddStatusEntry(const QString &key,
                                     const StatusAttributes &s)
{
    // If the entry is not in the map add the entry to the map.
    if (!statusMap.contains(key))
        statusMap.insert(key, new StatusAttributes(s));
}

// ****************************************************************************
// Method: QvisSimulationWindow::RemoveStatusEntry
//
// Purpose:
//   Removes an engine from the internal status map.
//
// Arguments:
//   key : The name of the engine to remove from the map.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::RemoveStatusEntry(const QString &key)
{
    SimulationStatusMap::Iterator pos;

    // If the entry is in the map delete the key and remove it from
    // the map.
    if ((pos = statusMap.find(key)) != statusMap.end())
    {
        // Delete the status attributes that are in the map.
        delete pos.value();
        // Remove the key from the map.
        statusMap.remove(key);
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::UpdateStatusEntry
//
// Purpose:
//   Makes the specified entry in the status map update to the current status
//   in the status attributes.
//
// Arguments:
//   key : The name of the engine whose status entry we want to update.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::UpdateStatusEntry(const QString &key,
                                        const StatusAttributes &s)
{
    // If the sender is in the status map, copy the status into the map entry.
    // If the sender is not in the map, add it.
    SimulationStatusMap::Iterator pos;

    if ((pos = statusMap.find(key)) != statusMap.end())
    {
        // Copy the status attributes.
        *(pos.value()) = *statusAtts;
    }
    else
        AddStatusEntry(key, *statusAtts);
}

// ****************************************************************************
// Method: QvisSimulationWindow::SetNewMetaData
//
// Purpose:
//    Update the meta data for the given file.
//
// Arguments:
//   qf        the host+file
//   md        the new meta data
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//    Jeremy Meredith, Thu Apr 28 18:03:57 PDT 2005
//    It needed more info to construct the key correctly.
//
// ****************************************************************************
void
QvisSimulationWindow::SetNewMetaData(const QualifiedFilename &qf,
                                     const avtDatabaseMetaData *md)
{
    if (md && md->GetIsSimulation())
    {
        // Save the metadata in case we need it later.
        *metadata = *md;

        // Create a key with which to associate this metadata.
        std::string host = qf.host;
        if (host == "localhost")
            host = GetViewerProxy()->GetLocalHostName();
        std::string path = qf.PathAndFile();
        QString key(MakeKey(host, path));

        // Store the metadata.
        UpdateMetaDataEntry(key, *md);

        // If the sender of the status message is the engine that we're
        // currently looking at, update the status widgets.
        if (key == activeEngine)
        {
            UpdateStatusArea();
            UpdateInformation();
        }
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::AddMetaDataEntry
//
// Purpose:
//   Adds an engine to the internal meta data map.
//
// Arguments:
//   key : The name of the engine to add to the map.
//   md  : The metadata to add to the map.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::AddMetaDataEntry(const QString &key,
                                       const avtDatabaseMetaData &md)
{
    // If the entry is not in the map add the entry to the map.
    if (!metadataMap.contains(key))
      metadataMap.insert(key, new avtDatabaseMetaData(md));
}

// ****************************************************************************
// Method: QvisSimulationWindow::RemoveMetaDataEntry
//
// Purpose:
//   Removes an engine from the internal meta data map.
//
// Arguments:
//   key : The name of the engine to remove from the map.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::RemoveMetaDataEntry(const QString &key)
{
    SimulationMetaDataMap::Iterator pos;

    // If the entry is in the map delete the key and remove it from
    // the map.
    if ((pos = metadataMap.find(key)) != metadataMap.end())
    {
        // Delete the meta data that are in the map.
        delete pos.value();

        // Remove the key from the map.
        metadataMap.remove(key);
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::UpdateMetaDataEntry
//
// Purpose:
//   Makes the specified entry in the meta data map update to the current
//   meta data.
//
// Arguments:
//   key : The name of the engine whose meta data entry we want to update.
//   md  : The metadata to associate with the key.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::UpdateMetaDataEntry(const QString &key,
                                          const avtDatabaseMetaData &md)
{
    // If the sender is in the meta data map, copy the MD into the map entry.
    SimulationMetaDataMap::Iterator pos;

    if ((pos = metadataMap.find(key)) != metadataMap.end())
    {
        // Copy the status attributes.
        *(pos.value()) = md;
    }
    // If the sender is not in the meta data map, add it.
    else
        AddMetaDataEntry(key, md);
}

// ****************************************************************************
// Method: QvisSimulationWindow::clearMessages
//
// Purpose:
//   Clear all of the strips charts.
//
// Programmer: Allen Sanderson
// Creation:   6 May 2016
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::clearMessages()
{
    // Clear the message and strip charts.
    simMessages->clear();
}

// ****************************************************************************
// Method: QvisSimulationWindow::clearStripCharts
//
// Purpose:
//   Clear all of the strips charts.
//
// Programmer: Allen Sanderson
// Creation:   6 May 2016
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::clearStripCharts()
{
    for( unsigned int i=0; i<MAX_STRIP_CHARTS; ++i )
    {
      stripChartMgr->clear( i );
      stripChartMgr->setTabLabel(i, "" );

      for( unsigned int j=0; j<MAX_STRIP_CHART_VARS; ++j )
        stripChartMgr->setCurveTitle(i, j, "" );
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::MakeKey
//
// Purpose:
//   Creates a key using the host name and the simulation.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Kathleen Biagas, Thu Jan 21, 2021
//   Replace QString.asprintf with QString.arg.
//
// ****************************************************************************
QString
QvisSimulationWindow::MakeKey(const std::string &host,
                              const std::string &sim) const
{
    if(sim.empty())
        return QString(host.c_str());
    else
        return QString("%1:%2").arg(host.c_str()).arg(sim.c_str());
}

// ****************************************************************************
// Method: QvisSimulationWindow::GetEngineListIndex
//
// Purpose:
//   Gets the engine list index for the given host:sim key.
//
// Arguments:
//   key : The host:sim whose EngineList index we want to get.
//
// Programmer: Brad Whitlock
// Creation:   November 19 2010
//
// Modifications:
//
// ****************************************************************************
int
QvisSimulationWindow::GetEngineListIndex(const QString &key) const
{
    for(size_t i = 0; i < engines->GetEngineName().size(); ++i)
    {
        QString testkey(MakeKey(engines->GetEngineName()[i],
                                engines->GetSimulationName()[i]));
        if(testkey == key)
            return i;
    }

    return -1;
}

// ****************************************************************************
// Method: QvisSimulationWindow::GetUIFileDirectory
//
// Purpose:
//   Returns the name of the directory where VisIt looks for UI files.
//
// Returns:    The directory where VisIt looks for UI files.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//   Brad Whitlock, Tue Jan 31 16:40:02 PST 2006
//   Refactored code from elsewhere into this method.
//
//   Shelly Prevost, Mon Mar 13 14:49:07 PST 2006
//   Changed the default directory to check for ui files from UI to ui.
//
// ****************************************************************************
QString
QvisSimulationWindow::GetUIFileDirectory() const
{
    // First look in user defined variable
    QString dirName(getenv("VISITSIMDIR"));
    // if still not defined then look in the users home directory
    if (dirName.isEmpty())
    {
        dirName = getenv("HOME");
        if (!dirName.isEmpty()) dirName += "/.visit/ui/";
    }
    // if not defined then look in global location
    if ( dirName.isEmpty())
    {
        dirName = getenv("VISITDIR");
        if (!dirName.isEmpty()) dirName += "/ui/";
    }

    return dirName;
}

// ****************************************************************************
// Method: QvisSimulationWindow::GetUIfile
//
// Purpose:
//   Returns the name of the directory where VisIt looks for UI files.
//
// Arguments:
//   key : The key for the simulation whose UI file we want to get.
//
// Returns:    The directory name of the UI file that is appropriate for the
//             currently selected simulation, or an empty string if there
//             is no simulation or if it does not have a user interface.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//   Brad Whitlock, Tue Jan 31 16:40:02 PST 2006
//   Refactored code from elsewhere into this method.
//
//   Brad Whitlock, Fri Nov 19 11:23:23 PDT 2010
//   Pass the key in instead of getting it from a widget.
//
// ****************************************************************************
QString
QvisSimulationWindow::GetUIFile(const QString &key) const
{
  QString retval("");

    SimulationMetaDataMap::ConstIterator pos = metadataMap.find(key);

    if(pos != metadataMap.end())
    {
        // get ui filename from the value array
        avtDatabaseMetaData *md = pos.value();
        const stringVector &names  = md->GetSimInfo().GetOtherNames();
        const stringVector &values = md->GetSimInfo().GetOtherValues();

        for (size_t i=0; i<names.size(); ++i)
        {
            if (names[i] == "uiFile" && !values[i].empty())
            {
                retval = GetUIFileDirectory() + QString(values[i].c_str());
                break;
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: QvisSimulationWindow::parseCompositeCMD
//
// Purpose:
//   This method is called to parse a composite cmd to get the
//   index and name.
//
// Arguments:
//   cmd      : The properly form command string.
//
// Programmer: Allen Sanderson
// Creation:   6 May 2016
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::parseCompositeCMD( const std::string cmd,
                                         std::string &ctrl,
                                         std::string &text )
{
  std::string strcmd(cmd);

  ctrl = getNextString( strcmd, " | " );
  text = getNextString( strcmd, " | " );
}

// ****************************************************************************
// Method: QvisSimulationWindow::parseCompositeCMD
//
// Purpose:
//   This method is called to parse a composite cmd to get the
//   index and name.
//
// Arguments:
//   cmd      : The properly form command string.
//
// Programmer: Allen Sanderson
// Creation:   6 May 2016
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::parseCompositeCMD( const std::string cmd,
                                         unsigned int &index,
                                         std::string &text )
{
  std::string strcmd(cmd);

  std::string str = getNextString( strcmd, " | " );
  index = atoi( str.c_str() );

  text = getNextString( strcmd, " | " );
}

// ****************************************************************************
// Method: QvisSimulationWindow::parseCompositeCMD
//
// Purpose:
//   This method is called to parse a composite cmd to get the
//   row, column, and name.
//
// Arguments:
//   cmd      : The properly form command string.
//
// Programmer: Allen Sanderson
// Creation:   6 May 2016
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::parseCompositeCMD( const std::string cmd,
                                         unsigned int &row,
                                         unsigned int &column,
                                         std::string &text )
{
  std::string strcmd(cmd);

  std::string str = getNextString( strcmd, " | " );
  row = atoi( str.c_str() );

  str = getNextString( strcmd, " | " );
  column = atoi( str.c_str() );

  text = getNextString( strcmd, " | " );
}

// ****************************************************************************
// Method: QvisSimulationWindow::parseCompositeCMD
//
// Purpose:
//   This method is called to parse a composite cmd to get the
//   row, column, and name.
//
// Arguments:
//   cmd      : The properly form command string.
//
// Programmer: Allen Sanderson
// Creation:   6 May 2016
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::parseCompositeCMD( const std::string cmd,
                                         unsigned int &row,
                                         std::string &text,
                                         unsigned int &editable )
{
  std::string strcmd(cmd);

  std::string str = getNextString( strcmd, " | " );
  row = atoi( str.c_str() );

  text = getNextString( strcmd, " | " );

  str = getNextString( strcmd, " | " );
  editable = atoi( str.c_str() );
}

// ****************************************************************************
// Method: QvisSimulationWindow::parseCompositeCMD
//
// Purpose:
//   This method is called to parse a composite cmd to get the
//   row, column, x, and y values.
//
// Arguments:
//   cmd      : The properly form command string.
//
// Programmer: Allen Sanderson
// Creation:   6 May 2016
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::parseCompositeCMD( const std::string cmd,
                                         unsigned int &row,
                                         unsigned int &column,
                                         double &x, double &y )
{
  std::string strcmd(cmd);

  std::string str = getNextString( strcmd, " | " );
  row = atof( str.c_str() );

  str = getNextString( strcmd, " | " );
  column = atof( str.c_str() );

  str = getNextString( strcmd, " | " );
  x = atof( str.c_str() );

  str = getNextString( strcmd, " | " );
  y = atof( str.c_str() );
}

// ****************************************************************************
// Method: QvisSimulationWindow::parseCompositeCMD
//
// Purpose:
//   This method is called to parse a composite cmd to get the
//   chart, cruve, and npts, and an array of x, and y values.
//
// Arguments:
//   cmd      : The properly form command string.
//
// Programmer: Allen Sanderson
// Creation:   29 May 2020
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::parseCompositeCMD( const std::string cmd,
                                         unsigned int &chart,
                                         unsigned int &curve,
                                         unsigned int &npts,
                                         double *&x, double *&y )
{
  std::string strcmd(cmd);

  std::string str = getNextString( strcmd, " | " );
  chart = atof( str.c_str() );

  str = getNextString( strcmd, " | " );
  curve = atof( str.c_str() );

  str = getNextString( strcmd, " | " );
  npts = atof( str.c_str() );

  x = new double[npts];
  y = new double[npts];

  for( unsigned int i=0; i<npts; ++i )
  {
    str = getNextString( strcmd, " | " );
    x[i] = atof( str.c_str() );

    str = getNextString( strcmd, " | " );
    y[i] = atof( str.c_str() );
  }
}

// ****************************************************************************
// Method: QvisSimulationWindow::getNextString
//
// Purpose:
//   This method is called to get the next string value and return
//   the remaining part.
//
// Arguments:
//   cmd      : the properly form command string.
//   delimiter: the sentinal that separtates the strings.
//
// Programmer: Allen Sanderson
// Creation:   6 May 2016
//
// Modifications:
//
// ****************************************************************************
std::string QvisSimulationWindow::getNextString( std::string &cmd,
                                                 const std::string delimiter )
{
  size_t delim = cmd.find_first_of( delimiter );

  std::string str = cmd;

  if( delim != std::string::npos )
  {
    str.erase(delim, std::string::npos);
    cmd.erase(0, delim+delimiter.length());
  }
  else
  {
    cmd.clear();
  }

  return str;
}


//
// Qt slots
//

// ****************************************************************************
// Method: QvisSimulationWindow::selectEngine
//
// Purpose:
//   This is a Qt slot function that is called when selecting a new engine
//   to display.
//
// Arguments:
//   index : The index to use for the next active engine.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Brad Whitlock, Fri Nov 19 11:56:34 PDT 2010
//   Call MakeKey.
//
// ****************************************************************************
void
QvisSimulationWindow::selectEngine(int index)
{
    // Clear the message and strip charts.
    clearMessages();
    clearStripCharts();

    // Get the new active engine.
    activeEngine = MakeKey(engines->GetEngineName()[index],
                           engines->GetSimulationName()[index]);

    if (CustomUIWindow != NULL)
    {
        bool show = CustomUIWindow->isVisible();
        CustomUIWindow->hide();

        delete CustomUIWindow;
        CustomUIWindow = NULL;
        simCommands->setCustomButtonEnabled(false);

        if( show )
          showCustomUIWindow();
    }

    // Update the rest of the widgets using the information for the
    // active engine.
    UpdateStatusArea();
    // Update the engine information.
    UpdateInformation();
}

// ****************************************************************************
// Method: QvisSimulationWindow::interruptEngine
//
// Purpose:
//   This is a Qt slot function that is called to interrupt the engine that's
//   displayed in the window.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Brad Whitlock, Fri Nov 19 11:56:34 PDT 2010
//   Use the sim to enginelist map to get the right engine list index.
//
// ****************************************************************************
void
QvisSimulationWindow::interruptEngine()
{
    int index = GetEngineListIndex(activeEngine);
    if(index < 0)
        return;

    std::string host = engines->GetEngineName()[index];
    std::string sim  = engines->GetSimulationName()[index];

    GetViewerProxy()->InterruptComputeEngine(host, sim);
}

// ****************************************************************************
// Method: QvisSimulationWindow::closeEngine
//
// Purpose:
//   This is a Qt slot function that is called when the "Close engine" button
//   is clicked. Its job is to tell the viewer to close the engine being
//   displayed by the window.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Brad Whitlock, Fri Nov 19 11:56:34 PDT 2010
//   Use the sim to enginelist map to get the right engine list index.
//
// ****************************************************************************
void
QvisSimulationWindow::closeEngine()
{
    int index = GetEngineListIndex(activeEngine);
    if (index < 0)
        return;

    std::string host = engines->GetEngineName()[index];
    std::string sim  = engines->GetSimulationName()[index];

    // Create a prompt for the user.
    QString msg;
    if (sim == "")
    {
        msg = tr("Really close the compute engine on host \"%1\"?\n\n").
              arg(host.c_str());
    }
    else
    {
        msg = tr("Really disconnect from the simulation \"%1\" on "
                 "host \"%2\"?\n\n").
              arg(sim.c_str()).
              arg(host.c_str());
    }

    // Ask the user if they really wants to close the engine.
    if (QMessageBox::warning(this, "VisIt", msg,
                             QMessageBox::Ok | QMessageBox::Cancel) ==
        QMessageBox::Ok)
    {
        stripChartMgr->clearAll();

        // Close the engine.
        GetViewerMethods()->CloseComputeEngine(host, sim);
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::clearCache
//
// Purpose:
//   This is a Qt slot function that is called to tell the current engine
//   to clear its cache.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Brad Whitlock, Fri Nov 19 11:56:34 PDT 2010
//   Use the sim to enginelist map to get the right engine list index.
//
// ****************************************************************************
void
QvisSimulationWindow::clearCache()
{
    int index = GetEngineListIndex(activeEngine);
    if (index < 0)
        return;

    std::string host = engines->GetEngineName()[index];
    std::string sim  = engines->GetSimulationName()[index];

    if (GetViewerProxy()->GetLocalHostName() == host)
        GetViewerMethods()->ClearCache("localhost", sim);
    else
        GetViewerMethods()->ClearCache(host, sim);
}

// ****************************************************************************
// Method: QvisSimulationWindow::showCustomUIWindow
//
// Purpose:
//   If specficed, creates and shows the Custom UI window.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//    Brad Whitlock, Tue Jan 31 15:54:43 PST 2006
//    I made it create the custom ui window on the fly instead of creating
//    it at the same time as the regular Window is created.
//
// ****************************************************************************
void
QvisSimulationWindow::showCustomUIWindow()
{
    if( CustomUIWindow == NULL )
        CreateCustomUIWindow();

    if( CustomUIWindow != NULL )
    {
        CustomUIWindow->show();
        CustomUIWindow->raise();

        int index = GetEngineListIndex(activeEngine);
        if (index < 0)
          return;

        std::string host = engines->GetEngineName()[index];
        std::string sim  = engines->GetSimulationName()[index];

        QString cmd("ActivateCustomUI");
        QString args(QString("clicked();%1;QPushButton;Simulations;NONE").arg(cmd));
        GetViewerMethods()->SendSimulationCommand(host, sim, cmd.toStdString(), args.toStdString());
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::executePushButtonCommand
//
// Purpose:
//   This method is called when the command buttons are pushed.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::executePushButtonCommand(const QString &btncmd)
{
    int index = GetEngineListIndex(activeEngine);
    if (index < 0)
        return;

    std::string host = engines->GetEngineName()[index];
    std::string sim  = engines->GetSimulationName()[index];

    // Special handlers for abort and terminate buttons. Ask for a
    // confirmation before sending the signal.
    if( btncmd == QString("Abort") ||
        btncmd == QString("abort") ||
        btncmd == QString("Terminate") ||
        btncmd == QString("terminate") )
    {
      // Create a prompt for the user.
      QString msg;
      if (sim == "")
      {
        msg = tr("Really %1 the simulation on host \"%2\"?\n\n")
          .arg(btncmd)
          .arg(host.c_str());
      }
      else
      {
        msg = tr("Really %1 the simulation \"%2\" on host \"%3\"?\n\n")
          .arg(btncmd)
          .arg(sim.c_str())
          .arg(host.c_str());
      }

      // Ask the user if they really want to execute this command.
      if (QMessageBox::warning(this, "VisIt", msg,
                               QMessageBox::Ok | QMessageBox::Cancel) ==
          QMessageBox::Cancel)
        return;
      else
        stripChartMgr->clearAll();
    }

    QString cmd(btncmd);
    QString args(QString("clicked();%1;QPushButton;Simulations;NONE").arg(cmd));
    GetViewerMethods()->SendSimulationCommand(host, sim, btncmd.toStdString(), args.toStdString());
}

// ****************************************************************************
// Method: QvisSimulationWindow::executeEnableTimeRange()
//
// Purpose:
//   This method is called when the user clicks on the enable button
//   for the time range ui
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//
// ****************************************************************************
void
QvisSimulationWindow::executeEnableTimeRange(const QString &value)
{
    int index = GetEngineListIndex(activeEngine);
    if (index < 0)
        return;

    std::string host = engines->GetEngineName()[index];
    std::string sim  = engines->GetSimulationName()[index];

    QString cmd("TimeLimitsEnabled");
    QString args(QString("clicked();%1;QCheckBox;Simulations;%2").arg(cmd).arg(value));
    GetViewerMethods()->SendSimulationCommand(host, sim, cmd.toStdString(), args.toStdString());
}

// ****************************************************************************
// Method: QvisSimulationWindow::executeStartCommand()
//
// Purpose:
//   This method is called when the types into the start text box for time
//   range control.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//   Shelly Prevost Fri Dec  1 10:36:07 PST 2006
//   Corrected the widget name being passed to the simulation in the
//   command string.
//
// ****************************************************************************
void
QvisSimulationWindow::executeStartCommand(const QString &value)
{
    int index = GetEngineListIndex(activeEngine);
    if (index < 0)
        return;

    std::string host = engines->GetEngineName()[index];
    std::string sim  = engines->GetSimulationName()[index];

    QString cmd("StartCycle");
    QString args(QString("returnedPressed();%1;QLineEdit;Simulations;%2").arg(cmd).arg(value));
    GetViewerMethods()->SendSimulationCommand(host, sim, cmd.toStdString(), args.toStdString());
}

// ****************************************************************************
// Method: QvisSimulationWindow::executeStepCommand()
//
// Purpose:
//   This method is called when the types into the step text box for time
//   range control.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//    Shelly Prevost Fri Dec  1 10:36:07 PST 2006
//    Corrected the widget name being passed to the simulation in the
//    command string.
//
// ****************************************************************************
void
QvisSimulationWindow::executeStepCommand(const QString &value)
{
    int index = GetEngineListIndex(activeEngine);
    if (index < 0)
        return;

    std::string host = engines->GetEngineName()[index];
    std::string sim  = engines->GetSimulationName()[index];

    QString cmd("StepCycle");
    QString args(QString("returnedPressed();%1;QLineEdit;Simulations;%2").arg(cmd).arg(value));
    GetViewerMethods()->SendSimulationCommand(host, sim, cmd.toStdString(), args.toStdString());
}

// ****************************************************************************
// Method: QvisSimulationWindow::executeStopCommand()
//
// Purpose:
//   This method is called when the types into the stop text box for time
//   range control.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//    Shelly Prevost Fri Dec  1 10:36:07 PST 2006
//    Corrected the widget name being passed to the simulation in the
//    command string.
//
// ****************************************************************************
void
QvisSimulationWindow::executeStopCommand(const QString &value)
{
    int index = GetEngineListIndex(activeEngine);
    if (index < 0)
        return;

    std::string host = engines->GetEngineName()[index];
    std::string sim  = engines->GetSimulationName()[index];

    QString cmd("StopCycle");
    QString args(QString("returnedPressed();%1;QLineEdit;Simulations;%2").arg(cmd).arg(value));
    GetViewerMethods()->SendSimulationCommand(host, sim, cmd.toStdString(), args.toStdString());
}

// ****************************************************************************
// Method: QvisSimulationWindow::setStripChartVar()
//
// Purpose:
//   This method is called when the types into the stop text box for time
//   range control.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//    Shelly Prevost Fri Dec  1 10:36:07 PST 2006
//    Corrected the widget name being passed to the simulation in the
//    command string.
//
// ****************************************************************************
void
QvisSimulationWindow::setStripChartVar(const QString &value)
{
    int index = GetEngineListIndex(activeEngine);
    if (index < 0)
        return;

    std::string host = engines->GetEngineName()[index];
    std::string sim  = engines->GetSimulationName()[index];

    QString cmd("StripChartVar");
    QString args(QString("triggered();%1;QMenu;Simulations;%2").arg(cmd).arg(value));
    GetViewerMethods()->SendSimulationCommand(host, sim, cmd.toStdString(), args.toStdString());
}
