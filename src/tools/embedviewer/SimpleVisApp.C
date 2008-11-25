#include <SimpleVisApp.h>
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
#include <SaveWindowAttributes.h>

// Include this last since something about it on X11 systems seems to
// interfere with some of our other enums. X11 headers must have some
// naughty #define directives.
#include <vtkQtRenderWindow.h>

vtkQtRenderWindow *
SimpleVisApp::ReturnVisWin(void *data)
{
    SimpleVisApp *This = (SimpleVisApp *)data;
    return This->viswin;
}

// ****************************************************************************
// Method: SimpleVisApp::SimpleVisApp
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   v : The VisItViewer object that we'll use to control the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 21 10:23:40 PST 2008
//
// Modifications:
//   Brad Whitlock, Fri Nov 21 10:23:46 PST 2008
//   Fixed some slots for Qt 4.
//
// ****************************************************************************

SimpleVisApp::SimpleVisApp(VisItViewer *v) : QMainWindow()
{
    viewer = v;
    setWindowTitle(tr("Simple visualization"));
    plotType = 0;

    // Create the window.
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *hLayout = new QHBoxLayout(central);
    hLayout->setMargin(10);
    hLayout->setSpacing(10);
    QVBoxLayout *leftLayout = new QVBoxLayout(0);
    leftLayout->setSpacing(10);
    hLayout->addLayout(leftLayout);

    scalarLabel = new QLabel(tr("Scalar variables"), central);
    leftLayout->addWidget(scalarLabel);

    variables = new QListWidget(central);
    leftLayout->addWidget(variables);
    connect(variables, SIGNAL(currentTextChanged(const QString &)),
            this, SLOT(changeVariable(const QString &)));

    plotTypeWidget = new QWidget(central);
    leftLayout->addWidget(plotTypeWidget);
    QHBoxLayout *ptLayout = new QHBoxLayout(plotTypeWidget);
    ptLayout->setSpacing(10);
    ptLayout->addWidget(new QLabel(tr("Plot type"), plotTypeWidget));
    plotType = new QButtonGroup(plotTypeWidget);
    QRadioButton *rb = new QRadioButton(tr("Pseudocolor"), plotTypeWidget);
    plotType->addButton(rb, 0);
    ptLayout->addWidget(rb);
    rb = new QRadioButton(tr("Contour"), plotTypeWidget);
    plotType->addButton(rb, 1);
    ptLayout->addWidget(rb);
    connect(plotType, SIGNAL(buttonClicked(int)),
            this, SLOT(changePlotType(int)));

    contourWidget = new QWidget(central);
    leftLayout->addWidget(contourWidget);
    QHBoxLayout *cLayout = new QHBoxLayout(contourWidget);
    cLayout->setSpacing(10);
    nContours = new QSpinBox(contourWidget);
    nContours->setRange(1,40);
    nContours->setValue(10);
    connect(nContours, SIGNAL(valueChanged(int)),
            this, SLOT(setNContours(int)));
    cLayout->addWidget(new QLabel(tr("Number of contours"), contourWidget));
    cLayout->addWidget(nContours);

    // Create the vis window directly.
    viswin = new vtkQtRenderWindow(central);
    viswin->setMinimumSize(QSize(500,500));
    hLayout->addWidget(viswin, 100);

    // Create menus
    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(tr("Open . . ."), this, SLOT(selectFile()));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Save window"), this, SLOT(saveWindow()));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Quit"), qApp, SLOT(quit()));
    
    QMenu *controlsMenu = menuBar()->addMenu(tr("Controls"));
    controlsMenu->addAction(tr("Open GUI"), this, SLOT(openGUI()));

    //
    // Register a window creation function (before Setup) that will
    // return the vtkQtRenderWindow objects that we've already
    // parented into our interface.
    //
    viewer->SetWindowCreationCallback(ReturnVisWin, (void *)this);

    // Set the initial widget sensitivity.
    resetWindow();
}

SimpleVisApp::~SimpleVisApp()
{}

void
SimpleVisApp::resetWindow()
{
    variables->blockSignals(true);
    variables->clear();
    variables->blockSignals(false);

    plotType->blockSignals(true);
    plotType->button(0)->setChecked(true);
    plotType->blockSignals(false);

    scalarLabel->setEnabled(false);
    variables->setEnabled(false);
    plotTypeWidget->setEnabled(false);
    contourWidget->setEnabled(false);

    viewer->Methods()->DeleteActivePlots();
}

//
// Qt slots
//

void
SimpleVisApp::show()
{
    // Tell the viewer to hide toolbars in all windows.
    viewer->Methods()->HideToolbars(true);
    // Tell the viewer to show all windows. This does not show our windows
    // since our windows are embedded but it does do some extra setup for
    // the windows and thus needs to be called.
    viewer->Methods()->ShowAllWindows();

    QMainWindow::show();
}

void
SimpleVisApp::selectFile()
{
    // Get a filename from the file dialog.
    QString filename = QFileDialog::getOpenFileName(this,
               tr("Open File"),
               QDir::current().path(),
               tr("Data files (*.silo *.vtk *.cgns *.nc *.h5 *.pdb *.visit)"));
        
    if(!filename.isEmpty())
    {
        // Open the file.
        viewer->Methods()->OpenDatabase(filename.toStdString());

        // Get the file's metadata and populate the variable list.
        const avtDatabaseMetaData *md = viewer->GetMetaData(filename.toStdString());
        if(md != 0)
        {
            variables->blockSignals(true);
            for(int i = 0; i < md->GetNumScalars(); ++i)
                variables->addItem(md->GetScalar(i)->name.c_str());
            variables->setCurrentRow(0);
            variables->blockSignals(false);
            if(md->GetNumScalars() > 0)
            {
                variables->setEnabled(true);
                plotTypeWidget->setEnabled(true);
                contourWidget->setEnabled(plotType->checkedId() == 1);
                // Add a plot of the first variable
                changePlotType(plotType->checkedId());
            }
            scalarLabel->setEnabled(true);
            variables->setEnabled(true);
        }
        else
            resetWindow();
    }
    else
        resetWindow();
}

void
SimpleVisApp::changeVariable(const QString &var)
{
    viewer->Methods()->ChangeActivePlotsVar(var.toStdString());
}

void
SimpleVisApp::changePlotType(int val)
{
    if(variables->currentRow() == -1)
        return;

    // Determine the variable.
    std::string var(variables->currentItem()->text().toStdString());

    // Delete the active plots.
    viewer->Methods()->DeleteActivePlots();
    if(val == 0)
    {
        int Pseudocolor = viewer->GetPlotIndex("Pseudocolor");
        viewer->Methods()->AddPlot(Pseudocolor, var);
    }
    else
    {
        int Contour = viewer->GetPlotIndex("Contour");
        viewer->Methods()->AddPlot(Contour, var);
    }
    contourWidget->setEnabled(val == 1);
    viewer->Methods()->DrawPlots();
}

void
SimpleVisApp::setNContours(int nc)
{
    int Contour = viewer->GetPlotIndex("Contour");
    AttributeSubject *contourAtts = viewer->DelayedState()->GetPlotAttributes(Contour);
    if(contourAtts != 0)
    {
        contourAtts->SetValue("contourNLevels", nc);
        contourAtts->Notify();
        viewer->DelayedMethods()->SetPlotOptions(Contour);
    }
}

void
SimpleVisApp::saveWindow()
{
    // Set the output format to JPEG
    SaveWindowAttributes *swa = viewer->State()->GetSaveWindowAttributes();
    swa->SetFormat(SaveWindowAttributes::JPEG);
    swa->SetScreenCapture(true);

    // Save the window
    viewer->Methods()->SaveWindow();
}

void
SimpleVisApp::openGUI()
{
    stringVector args;
    viewer->DelayedMethods()->OpenClient("GUI", viewer->GetVisItCommand(), args);
}
