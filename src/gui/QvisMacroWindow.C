// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisMacroWindow.h>
#include <QButtonGroup>
#include <QLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QvisNotepadArea.h>

#include <Utility.h>
#include <DataNode.h>
#include <DebugStream.h>

#include <icons/macroexec.xpm>

// ****************************************************************************
// Method: QvisMacroWindow::QvisMacroWindow
//
// Purpose: 
//   QvisMacroWindow constructor.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 14 16:54:38 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 11:57:36 PDT 2008
//   QString for captionString, shortName.
//
// ****************************************************************************

QvisMacroWindow::QvisMacroWindow(const QString &captionString,
    const QString &shortName, QvisNotepadArea *n) : 
    QvisPostableWindow(captionString, shortName, n)
{
    macroButtons = 0;
    CreateEntireWindow();
    isCreated = true;
}

// ****************************************************************************
// Method: QvisMacroWindow::~QvisMacroWindow
//
// Purpose: 
//   QvisMacroWindow destructor.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 14 16:54:38 PST 2007
//
// Modifications:
//   
// ****************************************************************************

QvisMacroWindow::~QvisMacroWindow()
{
}

// ****************************************************************************
// Method: QvisMacroWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the window contents.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 14 16:54:38 PST 2007
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisMacroWindow::CreateWindowContents()
{ 
    macroGroup = new QGroupBox(central);
    macroGroup->setTitle(tr("Macros"));
//    macroGroup->setMinimumWidth(200);
//    macroGroup->setMinimumHeight(100);
    topLayout->addWidget(macroGroup);
    QVBoxLayout *innerMacroLayout = new QVBoxLayout(macroGroup);
    innerMacroLayout->setContentsMargins(10,10,10,10);
    innerMacroLayout->addSpacing(15);

    // Create the button layout.
    buttonLayout = new QGridLayout();
    innerMacroLayout->addLayout(buttonLayout);
    buttonLayout->setSpacing(5);

    // Create the button group.
    macroButtons = new QButtonGroup(macroGroup);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(macroButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(invokeMacro(int)));
#else
    connect(macroButtons, SIGNAL(idClicked(int)),
            this, SLOT(invokeMacro(int)));
#endif
}


//
// Qt slots
//

// ****************************************************************************
// Method: QvisMacroWindow::executeClicked
//
// Purpose: 
//   This is a Qt slot function that gets called when we click on one of the
//   macro buttons.
//
// Arguments:
//   index : The index of the execute button that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 14 16:59:32 PST 2007
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
//   Brad Whitlock, Wed Aug 18 11:30:40 PDT 2010
//   Escape quotes in the macro name so they get sent to Python correctly.
//
// ****************************************************************************

void
QvisMacroWindow::invokeMacro(int index)
{
    QString invoke("ExecuteMacro(\"%1\")\n");
    invoke = invoke.arg(macroButtons->button(index)->text().replace("\"", "\\\""));
    emit runCommand(invoke);
}

// ****************************************************************************
// Method: QvisMacroWindow::addMacro
//
// Purpose: 
//   This is a Qt slot function that gets called when we need to add a new
//   macro button.
//
// Arguments:
//   s : The macro name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 14 17:00:27 PST 2007
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
//   Gunther H. Weber, Wed Feb 17 14:34:03 PST 2010
//   Work around issue that layout does not work when macro buttons are
//   added while the macro window is posted.
//
// ****************************************************************************

void
QvisMacroWindow::addMacro(const QString &s)
{
    QPushButton *newMacro = new QPushButton(QIcon(QPixmap(macroexec_xpm)),
         s, macroGroup);

    // Add the button to the layout.
    int nbuttons = macroButtons->buttons().size();
    int row = nbuttons / 3;
    int col = nbuttons % 3;
    buttonLayout->addWidget(newMacro, row, col);

    debug1 << "Added macro button \"" << s.toStdString() 
           << "\" to grid at (" << row << ", " << col << ")\n";

    // Add the button to the button group.
    macroButtons->addButton(newMacro,nbuttons);
    newMacro->show();

    // Update the layout so the button gets added to the window.
    if (isPosted)
    {
        // <HACK> This should not be necessary since Qt should respect the
        // minimum size hint. However, otherwise the buttons will overlap if
        // the window is posted.
        central->setMinimumWidth(central->minimumSizeHint().width());
        central->setMinimumHeight(central->minimumSizeHint().height());
        // </HACK>

        // If the window is posted, the main widget is not visible. Thus,
        // we need to update the central area.
        central->updateGeometry();
        central->update();
    }
    else
    {
        // Window is visible, update the layout of the window
        updateGeometry();
        update();
    }
}

// ****************************************************************************
// Method: QvisMacroWindow::clearMacros
//
// Purpose: 
//   This is a Qt slot function that clears all of the macros.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 09:29:09 PDT 2007
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisMacroWindow::clearMacros()
{
    int n = macroButtons->buttons().size();
    for(int i = 0; i < n; ++i)
    {
        QPushButton *btn = (QPushButton*)macroButtons->button(i);
        if(btn != 0)
        {
            macroButtons->removeButton(btn);
            delete btn;
        }
    }
}
