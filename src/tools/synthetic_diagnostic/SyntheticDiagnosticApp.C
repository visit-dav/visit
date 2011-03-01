#include <SyntheticDiagnosticApp.h>
#include <VisItViewer.h>
#include <ViewerMethods.h>
#include <GlobalAttributes.h>

#include <QButtonGroup>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QTabWidget>
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
    return new SyntheticDiagnosticApp(v);
}

void
show_application_main_window(QWidget *w)
{
    // Do it like this since virtual show isn't working...
    SyntheticDiagnosticApp *app = (SyntheticDiagnosticApp *)w;
    app->show();
    app->raise();
}

// Window creation callback. Returns the vis window pointers to VisIt.
vtkQtRenderWindow *
SyntheticDiagnosticApp::ReturnVisWin(void *data)
{
    SyntheticDiagnosticApp *This = (SyntheticDiagnosticApp *)data;
    // Return vis window 1, 2, ...
    int idx = ++This->currentWindow;
    return (idx <= 3) ? This->viswindows[idx] : NULL;
}

SyntheticDiagnosticApp::SyntheticDiagnosticApp(VisItViewer *v) : QMainWindow()
{
    viewer = v;
    windowsAdded = false;
    currentWindow = 0;
    for(int i = 0; i < NWINDOWS + 1; ++i)
        viswindows[i] = 0;

    setWindowTitle(tr("Synthetic Diagnostic for Fusion"));

    // Create the window.
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout *Layout = new QHBoxLayout(central);
    Layout->setMargin(10);
    Layout->setSpacing(10);

    //Create the left side controls.
    QVBoxLayout *controlsLayout = new QVBoxLayout(0);
    controlsLayout->setSpacing(10);
    Layout->addLayout(controlsLayout);

    controlsLayout->addWidget(new QLabel(tr("Diagnostics"), central));

    diagnostics = new QListWidget(central);
    controlsLayout->addWidget(diagnostics);
    connect(diagnostics, SIGNAL(currentTextChanged(const QString &)),
            this, SLOT(onSelectDiagnostic(const QString &)));

    QHBoxLayout *buttonsLayout = new QHBoxLayout(0);
    controlsLayout->addLayout(buttonsLayout);
    buttonsLayout->addWidget(new QPushButton(tr("New Diagnostic"), central));
    buttonsLayout->addWidget(new QPushButton(tr("Delete"), central));

    QTabWidget *controlTabs = new QTabWidget(central);
    controlsLayout->addWidget(controlTabs);
    QWidget *diagnosticTab = new QWidget(central);
    controlTabs->addTab(diagnosticTab, tr("Diagnostic"));
    QWidget *simulationTab = new QWidget(central);
    controlTabs->addTab(simulationTab, tr("Simulation"));
    QWidget *miscTab = new QWidget(central);
    controlTabs->addTab(miscTab, tr("Miscellaneous"));

    //Diagnostic tab.
    QVBoxLayout *diagnosticLayout = new QVBoxLayout(diagnosticTab);
    diagnosticLayout->addWidget(new QLabel(tr("Diagnostic source"), diagnosticTab));
    QComboBox *diagnosticSource = new QComboBox(diagnosticTab);
    diagnosticSource->addItem(tr("Red Barchetta"));
    diagnosticSource->addItem(tr("The Pass"));
    diagnosticSource->addItem(tr("La Villa Strangiato"));
    diagnosticLayout->addWidget(diagnosticSource);
    
    diagnosticLayout->addWidget(new QLabel(tr("Post processing"), diagnosticTab));

    diagnosticLayout->addWidget(new QLabel(tr("View"), diagnosticTab));
    QHBoxLayout *diagViewLayout = new QHBoxLayout(0);
    diagnosticLayout->addLayout(diagViewLayout);
    diagViewLayout->addWidget(new QCheckBox(tr("1D"), diagnosticTab));
    diagViewLayout->addWidget(new QCheckBox(tr("2D"), diagnosticTab));
    diagViewLayout->addWidget(new QCheckBox(tr("3D"), diagnosticTab));
    
    diagnosticLayout->addWidget(new QLabel(tr("Options"), diagnosticTab));

    //Simulation tab.
    QVBoxLayout *simulationLayout = new QVBoxLayout(simulationTab);
    simulationLayout->addWidget(new QLabel(tr("Simulation source"), simulationTab));
    QComboBox *simulationSource = new QComboBox(simulationTab);
    simulationSource->addItem(tr("YYZ"));
    simulationSource->addItem(tr("Limelight"));
    simulationSource->addItem(tr("Witch Hunt"));
    simulationSource->addItem(tr("New World Man"));
    simulationLayout->addWidget(simulationSource);
    simulationLayout->addWidget(new QPushButton(tr("Line Probe Properties"), simulationTab));

    simulationLayout->addWidget(new QLabel(tr("View"), simulationTab));
    QHBoxLayout *simViewLayout = new QHBoxLayout(0);
    simulationLayout->addLayout(simViewLayout);
    simViewLayout->addWidget(new QCheckBox(tr("1D"), simulationTab));
    simViewLayout->addWidget(new QCheckBox(tr("2D"), simulationTab));
    simViewLayout->addWidget(new QCheckBox(tr("3D"), simulationTab));
    
    simulationLayout->addWidget(new QLabel(tr("Options"), simulationTab));

    //Misc tab.
    QVBoxLayout *miscLayout = new QVBoxLayout(miscTab);
    miscLayout->addWidget(new QLabel(tr("misc"), miscTab));
    miscLayout->addWidget(new QLabel(tr("misc"), miscTab));
    miscLayout->addWidget(new QLabel(tr("misc"), miscTab));
    
    //Create the viz windows.
    QSplitter *splitter1 = new QSplitter(central);
    splitter1->setOrientation(Qt::Horizontal);
    Layout->addWidget(splitter1, 100);

    viswindows[WINDOW_2D] = new vtkQtRenderWindow(splitter1);
    viswindows[WINDOW_2D]->setMinimumSize(QSize(100,150));

    QSplitter *splitter2 = new QSplitter(central);
    splitter2->setOrientation(Qt::Vertical);
    Layout->addWidget(splitter2, 100);

    viswindows[WINDOW_3D] = new vtkQtRenderWindow(splitter2);
    viswindows[WINDOW_3D]->setMinimumSize(QSize(100,150));
    
    viswindows[WINDOW_1D] = new vtkQtRenderWindow(splitter2);
    viswindows[WINDOW_1D]->setMinimumSize(QSize(100,150));
 
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

SyntheticDiagnosticApp::~SyntheticDiagnosticApp()
{
}

void
SyntheticDiagnosticApp::show()
{
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

void
SyntheticDiagnosticApp::selectFile()
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
SyntheticDiagnosticApp::openFile(const QString &filename)
{
    if(!filename.isEmpty())
    {
        activeFile = filename;
        resetWindow();

        // Get the file's metadata and populate the variable list.
        const avtDatabaseMetaData *md = viewer->GetMetaData(filename.toStdString());
        if(md != 0)
        {
            diagnostics->blockSignals(true);
            for(int i = 0; i < md->GetNumScalars(); ++i)
                diagnostics->addItem(md->GetScalar(i)->name.c_str());
            diagnostics->blockSignals(false);
            diagnostics->setEnabled(md->GetNumScalars() > 0);
        }
    }
}

void
SyntheticDiagnosticApp::resetWindow()
{
    diagnostics->blockSignals(true);
    diagnostics->clear();
    diagnostics->blockSignals(false);

    viewer->Methods()->SetActiveWindow(WINDOW_1D);
    viewer->Methods()->DeleteActivePlots();
    viewer->Methods()->SetActiveWindow(WINDOW_2D);
    viewer->Methods()->DeleteActivePlots();
    viewer->Methods()->SetActiveWindow(WINDOW_3D);
    viewer->Methods()->DeleteActivePlots();
}

void
SyntheticDiagnosticApp::onSelectDiagnostic(const QString &var)
{
    /*
    viewer->State()->GetGlobalAttributes()->SetApplyOperator(false);

    if(viewer->State()->GetPlotList()->GetNumPlots() == 1)
    {
        viewer->DelayedMethods()->SetActiveWindow(WINDOW_1D);
        viewer->DelayedMethods()->ChangeActivePlotsVar(var.toStdString());

        viewer->DelayedMethods()->SetActiveWindow(WINDOW_2D);
        viewer->DelayedMethods()->ChangeActivePlotsVar(var.toStdString());

        viewer->DelayedMethods()->SetActiveWindow(WINDOW_3D);
        viewer->DelayedMethods()->ChangeActivePlotsVar(var.toStdString());
    }
    else
    {
        viewer->DelayedMethods()->SetActiveWindow(WINDOW_3D);
        viewer->DelayedMethods()->OpenDatabase(activeFile.toStdString());
        viewer->DelayedMethods()->DeleteActivePlots();
        viewer->DelayedMethods()->AddPlot(viewer->GetPlotIndex("Contour"),
                                          var.toStdString());
        viewer->DelayedMethods()->DrawPlots();
        

        viewer->DelayedMethods()->SetActiveWindow(WINDOW_2D);
        viewer->DelayedMethods()->OpenDatabase(activeFile.toStdString());
        viewer->DelayedMethods()->DeleteActivePlots();
        viewer->DelayedMethods()->AddPlot(viewer->GetPlotIndex("Pseudocolor"),
                                          var.toStdString());
        viewer->DelayedMethods()->AddOperator(viewer->GetOperatorIndex("Slice"));
        viewer->DelayedMethods()->DrawPlots();

        viewer->DelayedMethods()->SetActiveWindow(WINDOW_1D);
        viewer->DelayedMethods()->OpenDatabase(activeFile.toStdString());
        viewer->DelayedMethods()->DeleteActivePlots();
        viewer->DelayedMethods()->AddPlot(viewer->GetPlotIndex("Pseudocolor"),
                                          var.toStdString());
        viewer->DelayedMethods()->DrawPlots();
    }
    */
}

void
SyntheticDiagnosticApp::openGUI()
{
    stringVector args;
    viewer->DelayedMethods()->OpenClient("GUI", viewer->GetVisItCommand(), args);
}

void
SyntheticDiagnosticApp::showTheWindow()
{
    show();
    raise();
}
