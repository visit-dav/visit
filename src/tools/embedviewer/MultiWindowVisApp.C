#include <MultiWindowVisApp.h>
#include <VisItViewer.h>
#include <ViewerMethods.h>

#include <QButtonGroup>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QMenuBar>
#include <QMenu>
#include <QRadioButton>
#include <QSpinBox>
#include <QWidget>

// State objects that we use.
#include <avtDatabaseMetaData.h>
#include <PlotList.h>

// Include this last since something about it on X11 systems seems to
// interfere with some of our other enums. X11 headers must have some
// naughty #define directives.
#include <vtkQtRenderWindow.h>

QWidget *
create_application_main_window(VisItViewer *v)
{
    return new MultiWindowVisApp(v);
}

void
show_application_main_window(QWidget *w)
{
    // Do it like this since virtual show isn't working...
    MultiWindowVisApp *app = (MultiWindowVisApp *)w;
    app->show();
    app->raise();
}

// Window creation callback. Returns the vis window pointers to VisIt.
vtkQtRenderWindow *
MultiWindowVisApp::ReturnVisWin(void *data)
{
    MultiWindowVisApp *This = (MultiWindowVisApp *)data;
    // Return vis window 1, 2, ...
    int idx = ++This->currentWindow;
qDebug("Return window %d", idx);
    return (idx <= 2) ? This->viswindows[idx] : NULL;
}

MultiWindowVisApp::MultiWindowVisApp(VisItViewer *v) : QMainWindow()
{
    viewer = v;
    windowsAdded = false;
    currentWindow = 0;
    for(int i = 0; i < NWINDOWS + 1; ++i)
        viswindows[i] = 0;

    setWindowTitle(tr("Multiwindow visualization"));

    // Create the window.
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *hLayout = new QHBoxLayout(central);
    hLayout->setMargin(10);
    hLayout->setSpacing(10);
    QVBoxLayout *leftLayout = new QVBoxLayout(0);
    leftLayout->setSpacing(10);
    hLayout->addLayout(leftLayout);

    QLabel *scalarLabel = new QLabel(tr("Scalar variables"), central);
    leftLayout->addWidget(scalarLabel);

    variables = new QListWidget(central);
    leftLayout->addWidget(variables);
    connect(variables, SIGNAL(currentTextChanged(const QString &)),
            this, SLOT(onSelectVariable(const QString &)));

    // Create the visualization windows. Do an easy layout in this example.
    QSplitter *splitter = new QSplitter(central);
    splitter->setOrientation(Qt::Horizontal);
    hLayout->addWidget(splitter, 100);

    viswindows[PSEUDOCOLOR_WINDOW] = new vtkQtRenderWindow(splitter);
    viswindows[PSEUDOCOLOR_WINDOW]->setMinimumSize(QSize(300,400));

    viswindows[VOLUME_WINDOW] = new vtkQtRenderWindow(splitter);
    viswindows[VOLUME_WINDOW]->setMinimumSize(QSize(300,400));
 
    //
    // Register a window creation function (before Setup) that will
    // return the vtkQtRenderWindow objects that we've already
    // parented into our interface.
    //
    viewer->SetWindowCreationCallback(ReturnVisWin, (void *)this);

    // Create menus
    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(tr("Open . . ."), this, SLOT(selectFile()));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Quit"), qApp, SLOT(quit()));

    QMenu *controlsMenu = menuBar()->addMenu(tr("Controls"));
    controlsMenu->addAction(tr("Open GUI"), this, SLOT(openGUI()));
}

MultiWindowVisApp::~MultiWindowVisApp()
{
}

void
MultiWindowVisApp::show()
{
qDebug("show");
    // Tell the viewer to hide toolbars in all windows.
    viewer->DelayedMethods()->HideToolbars(true);

    if(!windowsAdded)
    {
        windowsAdded = true;
        // Call AddWindow() N-1 times to make sure that VisIt calls the window
        // creation callback the appropriate number of times.
        viewer->DelayedMethods()->ToggleLockViewMode();
        for(int i = 0; i < NWINDOWS-1; ++i)
        {
            viewer->DelayedMethods()->AddWindow();
            viewer->DelayedMethods()->ToggleLockViewMode();
        }
    }
    viewer->DelayedMethods()->ShowAllWindows();
 
    QMainWindow::show();
}

// ****************************************************************************
// Method: MultiWindowVisApp::selectFile
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 21 10:23:40 PST 2008
//
// Modifications:
//
// ****************************************************************************

void
MultiWindowVisApp::selectFile()
{
    // Get a filename from the file dialog.
    QString filename = QFileDialog::getOpenFileName(this,
               tr("Open File"),
               QDir::current().path(),
               tr("Data files (*.silo *.vtk *.cgns *.nc *.h5 *.pdb *.visit)"));

    filename = QDir::toNativeSeparators(filename);

    openFile(filename);
}

void
MultiWindowVisApp::openFile(const QString &filename)
{
    if(!filename.isEmpty())
    {
        activeFile = filename;
        resetWindow();

        // Get the file's metadata and populate the variable list.
        const avtDatabaseMetaData *md = viewer->GetMetaData(filename.toStdString());
        if(md != 0)
        {
            variables->blockSignals(true);
            for(int i = 0; i < md->GetNumScalars(); ++i)
                variables->addItem(md->GetScalar(i)->name.c_str());
            variables->blockSignals(false);
            variables->setEnabled(md->GetNumScalars() > 0);
        }
    }
}

void
MultiWindowVisApp::resetWindow()
{
    variables->blockSignals(true);
    variables->clear();
    variables->blockSignals(false);

    viewer->Methods()->SetActiveWindow(VOLUME_WINDOW);
    viewer->Methods()->DeleteActivePlots();
    viewer->Methods()->SetActiveWindow(PSEUDOCOLOR_WINDOW);
    viewer->Methods()->DeleteActivePlots();
}

void
MultiWindowVisApp::onSelectVariable(const QString &var)
{
    if(viewer->State()->GetPlotList()->GetNumPlots() == 1)
    {
        viewer->DelayedMethods()->SetActiveWindow(VOLUME_WINDOW);
        viewer->DelayedMethods()->ChangeActivePlotsVar(var.toStdString());
        viewer->DelayedMethods()->SetActiveWindow(PSEUDOCOLOR_WINDOW);
        viewer->DelayedMethods()->ChangeActivePlotsVar(var.toStdString());
    }
    else
    {
        viewer->DelayedMethods()->SetActiveWindow(VOLUME_WINDOW);
        viewer->DelayedMethods()->OpenDatabase(activeFile.toStdString());
        viewer->DelayedMethods()->DeleteActivePlots();
        viewer->DelayedMethods()->AddPlot(viewer->GetPlotIndex("Volume"),
                                   var.toStdString());
        viewer->DelayedMethods()->DrawPlots();

        viewer->DelayedMethods()->SetActiveWindow(PSEUDOCOLOR_WINDOW);
        viewer->DelayedMethods()->OpenDatabase(activeFile.toStdString());
        viewer->DelayedMethods()->DeleteActivePlots();
        viewer->DelayedMethods()->AddPlot(viewer->GetPlotIndex("Pseudocolor"),
                                   var.toStdString());
        viewer->DelayedMethods()->DrawPlots();
    }
}

void
MultiWindowVisApp::openGUI()
{
    stringVector args;
    viewer->DelayedMethods()->OpenClient("GUI", viewer->GetVisItCommand(), args);
}

void
MultiWindowVisApp::showTheWindow()
{
    show();
    raise();
}
