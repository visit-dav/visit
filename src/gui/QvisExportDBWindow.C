#include <QvisExportDBWindow.h>

#include <stdio.h> // for sscanf

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qslider.h>

#include <QvisVariableButton.h>
#include <DebugStream.h>
#include <DBPluginInfoAttributes.h>
#include <ExportDBAttributes.h>
#include <FileServerList.h>
#include <ViewerProxy.h>

// ****************************************************************************
// Method: QvisExportDBWindow::QvisExportDBWindow
//
// Purpose: 
//   Cconstructor for the QvisExportDBWindow class.
//
// Arguments:
//   subj      : The ExportDBAttributes object that the window observes.
//   caption   : The string that appears in the window decoration.
//   shortName : The name to use when the window is posted.
//   notepad   : The notepad widget to which the window posts.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// ****************************************************************************

QvisExportDBWindow::QvisExportDBWindow(
    ExportDBAttributes *subj, const char *caption, const char *shortName,
    QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton)
{
    exportDBAtts = subj;
}

// ****************************************************************************
// Method: QvisExportDBWindow::~QvisExportDBWindow
//
// Purpose: 
//   This is the destructor for the QvisExportDBWindow class.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// Modifications:
//   
// ****************************************************************************

QvisExportDBWindow::~QvisExportDBWindow()
{
}

// ****************************************************************************
// Method: QvisExportDBWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets for the Aslice operator window.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// ****************************************************************************

void
QvisExportDBWindow::CreateWindowContents()
{
    // Create a group box for the file information.
    QGroupBox *infoBox = new QGroupBox(central, "infoBox");
    infoBox->setTitle("Output description");
    topLayout->addWidget(infoBox);

    QGridLayout *infoLayout = new QGridLayout(infoBox, 5, 2);
    infoLayout->setMargin(10);
    infoLayout->setSpacing(5);
    infoLayout->addRowSpacing(0, 10);

    filenameLineEdit = new QLineEdit(infoBox, "filenameLineEdit");
    connect(filenameLineEdit, SIGNAL(returnPressed()), this, SLOT(processFilenameText()));
    QLabel *filenameLabel = new QLabel(filenameLineEdit, "File name", infoBox, "filenameLabel");
    infoLayout->addWidget(filenameLabel, 1, 0);
    infoLayout->addWidget(filenameLineEdit, 1, 1);

    directoryNameLineEdit = new QLineEdit(infoBox, "directoryNameLineEdit");
    connect(directoryNameLineEdit, SIGNAL(returnPressed()), this, SLOT(processDirectoryNameText()));
    QLabel *directoryNameLabel = new QLabel(directoryNameLineEdit, "Directory name", infoBox, "directoryNameLabel");
    infoLayout->addWidget(directoryNameLabel, 2, 0);
    infoLayout->addWidget(directoryNameLineEdit, 2, 1);

    fileFormatComboBox = new QComboBox(false, infoBox, "fileFormatComboBox");
  
    DBPluginInfoAttributes *dbPluginInfoAtts = viewer->GetDBPluginInfoAttributes();
    int nTypes = dbPluginInfoAtts->GetTypes().size();
    for (int i = 0 ; i < nTypes ; i++)
        if (dbPluginInfoAtts->GetHasWriter()[i] > 0)
            fileFormatComboBox->insertItem(dbPluginInfoAtts->GetTypes()[i]);
    fileFormatComboBox->setCurrentItem(0);
        
    connect(fileFormatComboBox, SIGNAL(activated(int)),
           this, SLOT(fileFormatChanged(int)));
    QLabel *formatLabel = new QLabel(fileFormatComboBox, "File type",
                                     infoBox, "formatLabel");
    infoLayout->addWidget(formatLabel, 4, 0);
    infoLayout->addWidget(fileFormatComboBox, 4, 1);

    varsButton = new QvisVariableButton(true, false, true, -1,
        infoBox, "varsButton");
    varsButton->setText("Variables");
    varsButton->setChangeTextOnVariableChange(false);
    connect(varsButton, SIGNAL(activated(const QString &)),
            this, SLOT(addVariable(const QString &)));
    infoLayout->addWidget(varsButton, 3, 0);

    varsLineEdit = new QLineEdit(infoBox, "varsLineEdit");
    varsLineEdit->setText("default");
    connect(varsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(variableProcessText()));
    infoLayout->addWidget(varsLineEdit, 3, 1);

    // The export button.
    QHBoxLayout *exportButtonLayout = new QHBoxLayout(topLayout);
    exportButtonLayout->setSpacing(5);
    QPushButton *exportButton = new QPushButton("Export", central, "exportButton");
    connect(exportButton, SIGNAL(clicked()),
            this, SLOT(exportButtonClicked()));
    exportButtonLayout->addWidget(exportButton);
    exportButtonLayout->addStretch(50);
}

// ****************************************************************************
// Method: QvisExportDBWindow::UpdateWindow
//
// Purpose: 
//   This method updates the window's widgets to reflect changes made
//   in the SaveWindowAttributes object that the window watches.
//
// Arguments:
//   doAll : A flag indicating whether to update all of the widgets
//           regardless of the SaveWindowAttribute object's selected
//           states.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisExportDBWindow::UpdateWindow(bool doAll)
{
    int  i, j;

    QString temp;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for (i = 0; i < exportDBAtts->NumAttributes(); i++)
    {
        if (!doAll)
        {
            if (!exportDBAtts->IsSelected(i))
            continue;
        }

        switch (i)
        {
          case 0: // db type
            {
                fileFormatComboBox->blockSignals(true);
                int count = fileFormatComboBox->count();
                for (j = count-1 ; j >= 0 ; j--)
                    fileFormatComboBox->removeItem(j);
                DBPluginInfoAttributes *dbPluginInfoAtts =
                                           viewer->GetDBPluginInfoAttributes();
                int nTypes = dbPluginInfoAtts->GetTypes().size();
                int curItem = -1;
                int nItems = 0;
                for (j = 0 ; j < nTypes ; j++)
                    if (dbPluginInfoAtts->GetHasWriter()[j] > 0)
                    {
                        if (dbPluginInfoAtts->GetTypes()[j] == 
                                                    exportDBAtts->GetDb_type())
                            curItem = nItems;
                        nItems++;
                        fileFormatComboBox->insertItem(
                                              dbPluginInfoAtts->GetTypes()[j]);
                    }
                if (curItem >= 0)
                    fileFormatComboBox->setCurrentItem(curItem);
                else
                    fileFormatComboBox->setCurrentItem(0);
                fileFormatComboBox->blockSignals(false);
            }
            break;
          case 1: // db type fullname
            break;
          case 2: // format
            filenameLineEdit->setText(exportDBAtts->GetFilename().c_str());
            break;
          case 3: // directory name
            directoryNameLineEdit->setText(
                                     exportDBAtts->GetDirname().c_str());
            break;
          case 4: // variables
            {
                stringVector vars = exportDBAtts->GetVariables();
                std::string allVars;
                for (int i = 0; i < vars.size(); i++)
                {
                   allVars += vars[i];
                   allVars += " ";
                }
                varsLineEdit->setText(allVars.c_str());
            }
            break;
        }
    } // end for
}

// ****************************************************************************
// Method: QvisExportDBWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values for one or all of the lineEdit widgets.
//
// Arguments:
//   which_widget : The number of the widget to update. If -1 is passed,
//                  the routine gets the current values for all widgets.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// ****************************************************************************

void
QvisExportDBWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do the file name
    if(which_widget == 0 || doAll)
    {
        temp = filenameLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            exportDBAtts->SetFilename(temp.latin1());
        }
        else
        {
            msg.sprintf("The filename was invalid. "
                "Resetting to the last good value \"%s\".",
                exportDBAtts->GetFilename().c_str());
            Message(msg);
            exportDBAtts->SetFilename(exportDBAtts->GetFilename());
        }
    }
    if(which_widget == 1 || doAll)
    {
        temp = directoryNameLineEdit->displayText().simplifyWhiteSpace();
        exportDBAtts->SetDirname(temp.latin1());
    }
    if(which_widget == 2 || doAll)
    {
        QString temp;
        stringVector vars;
        temp = varsLineEdit->displayText().simplifyWhiteSpace();
        QStringList lst(QStringList::split(" ", temp));

        QStringList::Iterator it;

        for (it = lst.begin(); it != lst.end(); ++it)
        {
            vars.push_back((*it).latin1());
        }

        exportDBAtts->SetVariables(vars);
    }
}

// ****************************************************************************
// Method: QvisExportDBWindow::Apply
//
// Purpose: 
//   This method applies the save image attributes and optionally tells
//   the viewer to apply them.
//
// Arguments:
//   ignore : This flag, when true, tells the code to ignore the
//            AutoUpdate function and tell the viewer to apply the
//            aslice attributes.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// ****************************************************************************

void
QvisExportDBWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current aslice attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        exportDBAtts->Notify();
    }
    else
        exportDBAtts->Notify();
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisExportDBWindow::apply
//
// Purpose: 
//   This is a Qt slot function to apply the save image attributes.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisExportDBWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisExportDBWindow::processFilenameText
//
// Purpose: 
//   This is a Qt slot function that sets the file name.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisExportDBWindow::processFilenameText()
{
    GetCurrentValues(0);
    Apply();
}

// ****************************************************************************
// Method: QvisExportDBWindow::processDirectoryNameText
//
// Purpose: 
//   This is a Qt slot function that sets the directory name.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisExportDBWindow::processDirectoryNameText()
{
    GetCurrentValues(1);
    Apply();
}

// ****************************************************************************
// Method: QvisExportDBWindow::fileFormatChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new file format
//   is selected.
//
// Arguments:
//   index : The index of the file format that was selected.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// ****************************************************************************

void
QvisExportDBWindow::fileFormatChanged(int index)
{
    QString tmp = fileFormatComboBox->currentText();
    exportDBAtts->SetDb_type(tmp.latin1());
    
    DBPluginInfoAttributes *dbPluginInfoAtts =
                                           viewer->GetDBPluginInfoAttributes();
    int ntypes = dbPluginInfoAtts->GetTypes().size();
    for (int i = 0 ; i < ntypes ; i++)
    {
        if (dbPluginInfoAtts->GetTypes()[i] == tmp.latin1())
        {
            exportDBAtts->SetDb_type_fullname(
                                     dbPluginInfoAtts->GetTypesFullNames()[i]);
            // The atts have the array as [R, W, R, W, ...], so for plugin I,
            // its write atts are 2*I+1.
            DBOptionsAttributes &atts = 
                               dbPluginInfoAtts->GetDBOptionsAttributes(2*i+1);
            exportDBAtts->SetOpts(atts);
        }
    }

    Apply();
}

// ****************************************************************************
// Method: QvisExportDBWindow::exportWindow
//
// Purpose: 
//   This is Qt slot function that is called when the Export button is clicked.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisExportDBWindow::exportDB()
{
    Apply();
    if(isVisible() && !posted())
        hide();
    viewer->ExportDatabase();
}

// ****************************************************************************
// Method: QvisExportDBWindow::exportButtonClicked
//
// Purpose: 
//   This is Qt slot function that is called when the export button is clicked. 
//
// Programmer: Hank Childs 
// Creation:   May 25, 2005
//
// Modifications:
//
// ****************************************************************************

void
QvisExportDBWindow::exportButtonClicked()
{
    apply();
    exportDB();
}

// ****************************************************************************
// Method: QvisExportDBWindow::addVariable
//
// Purpose:
//   This is a Qt slot function that is called when the user selects a new
//   variable.
//
// Arguments:
//   var : The variable to add.
//
// Programmer: Hank Childs
// Creation:   May 27, 2005
//
// Modifications:
//
// ****************************************************************************

void
QvisExportDBWindow::addVariable(const QString &var)
{
    // Add the new variable to the variable line edit.
    QString varString(varsLineEdit->displayText());
    if(varString.length() > 0)
        varString += " ";
    varString += var;
    varsLineEdit->setText(varString);

    // Process the list of vars.
    variableProcessText();
}


void
QvisExportDBWindow::variableProcessText()
{
    GetCurrentValues(2);
    Apply();
}

