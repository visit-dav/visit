/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
// Modifications:
//    Jeremy Meredith, Tue Jul 17 11:29:24 EDT 2007
//    Converted to a simple observer so we can watch not only the 
//    export atts but also the DB plugin info atts.
//
// ****************************************************************************

QvisExportDBWindow::QvisExportDBWindow(
    const char *caption, const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowSimpleObserver(caption, shortName, notepad,
                                QvisPostableWindowSimpleObserver::ApplyButton)
{
    exportDBAtts = NULL;
    dbPluginInfoAtts = NULL;
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
// Method: QvisExportDBWindow::ConnectSubjects
//
// Purpose: 
//   This function connects subjects so that the window observes them.
//
// Programmer: Jeremy Meredith
// Creation:   July 17, 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisExportDBWindow::ConnectSubjects(ExportDBAttributes *edb,
                                    DBPluginInfoAttributes *dbp)
{
    exportDBAtts = edb;
    exportDBAtts->Attach(this);

    dbPluginInfoAtts = dbp;
    dbPluginInfoAtts->Attach(this);
}


// ****************************************************************************
// Method: QvisExportDBWindow::SubjectRemoved
//
// Purpose: 
//   This function is called when a subject is removed.
//
// Arguments:
//   TheRemovedSubject : The subject being removed.
//
// Programmer: Jeremy Meredith
// Creation:   July 17, 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisExportDBWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if (TheRemovedSubject == exportDBAtts)
        exportDBAtts = 0;
    else if (TheRemovedSubject == dbPluginInfoAtts)
        dbPluginInfoAtts = 0;
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
// Modifications:
//   Brad Whitlock, Mon Jun 27 13:52:24 PST 2005
//   I added a directory selection button.
//
//   Brad Whitlock, Thu Jul 14 13:50:15 PST 2005
//   Prevented "..." button max width from being set on MacOS X.
//
//   Hank Childs, Mon Apr 24 09:57:22 PDT 2006
//   Tell the file format that is has "changed", so that it will always be
//   up-to-date ('7177).
//
//   Brad Whitlock, Thu Nov 2 17:57:52 PST 2006
//   I changed "File type" to "Export to" since we may not be exporting
//   to a file after all.
//
//   Jeremy Meredith, Tue Jul 17 11:28:43 EDT 2007
//   Just leave the file format combo box empty.  It will update when
//   necessary on demand now that we observe DB plugin info.
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

    QHBox *directoryParent = new QHBox(infoBox, "directoryParent");
    directoryNameLineEdit = new QLineEdit(directoryParent, "directoryNameLineEdit");
    connect(directoryNameLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processDirectoryNameText()));
    directoryNameLabel = new QLabel(directoryNameLineEdit,
        "Directory name", infoBox, "directoryNameLabel");
    directorySelectButton = new QPushButton("...",
         directoryParent, "directorySelectButton");
#ifndef Q_WS_MACX
    directorySelectButton->setMaximumWidth(
         fontMetrics().boundingRect("...").width() + 6);
#endif
    directorySelectButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
         QSizePolicy::Minimum));
    connect(directorySelectButton, SIGNAL(clicked()),
            this, SLOT(selectOutputDirectory()));
    directoryParent->setSpacing(0);
    directoryParent->setStretchFactor(directoryNameLineEdit, 100);
    infoLayout->addWidget(directoryNameLabel, 2, 0);
    infoLayout->addWidget(directoryParent, 2, 1);

    fileFormatComboBox = new QComboBox(false, infoBox, "fileFormatComboBox");
    fileFormatComboBox->clear();
  
    connect(fileFormatComboBox, SIGNAL(activated(int)),
           this, SLOT(fileFormatChanged(int)));
    QLabel *formatLabel = new QLabel(fileFormatComboBox, "Export to",
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

    // The information about the file format is only updated when it is
    // changed.  So if it is not changed, then it never updates.  So tell it
    // now that it has changed, in case anyone tries to export a database using
    // the first option (i.e. without changing it).
    fileFormatChanged(0);
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
//   Brad Whitlock, Thu Nov 2 18:03:12 PST 2006
//   Added code to disable the directory when we're saving as SimV1.
//
//   Jeremy Meredith, Tue Jul 17 11:28:12 EDT 2007
//   Changed to observe dbPluginInfoAtts and update file format combo
//   box as necessary.
//
// ****************************************************************************

void
QvisExportDBWindow::UpdateWindow(bool doAll)
{
    if (dbPluginInfoAtts == 0 || exportDBAtts == 0)
        return;

    if (SelectedSubject() == dbPluginInfoAtts)
    {
        doAll = true;
    }

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
                            dbPluginInfoAtts->GetTypes()[j].c_str());
                    }
                if (curItem >= 0)
                    fileFormatComboBox->setCurrentItem(curItem);
                else
                    fileFormatComboBox->setCurrentItem(0);
                fileFormatComboBox->blockSignals(false);

                // This isn't so clean since we should not be doing things
                // based on a plugin name but it's okay for now.
                // Disable directories if the database type is "SimV1".
                bool enableDir = fileFormatComboBox->currentText() != 
                                 QString("SimV1");
                directoryNameLineEdit->setEnabled(enableDir);
                directorySelectButton->setEnabled(enableDir);
                directoryNameLabel->setEnabled(enableDir);
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
// Modifications:
//    Jeremy Meredith, Tue Jul 17 11:30:54 EDT 2007
//    Allow for observed subjects not yet being connected and for an empty
//    file format combo box.  Assume we're directly observing the DB
//    plugin info atts.
//
// ****************************************************************************

void
QvisExportDBWindow::fileFormatChanged(int index)
{
    if (dbPluginInfoAtts == 0 || exportDBAtts == 0)
        return;

    QString tmp = fileFormatComboBox->currentText();
    if (tmp.isNull())
        return;

    exportDBAtts->SetDb_type(tmp.latin1());
    
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
                               dbPluginInfoAtts->GetDbOptions(2*i+1);
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
    GetViewerMethods()->ExportDatabase();
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

// ****************************************************************************
// Method: QvisExportDBWindow::selectOutputDirectory
//
// Purpose: 
//   This is a Qt slot function that is called when we want to interactively
//   select a new output directory.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 27 13:52:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisExportDBWindow::selectOutputDirectory()
{
    //
    // Try and get a directory using a file dialog.
    //
    QString initialDir(exportDBAtts->GetDirname().c_str());
    QString dirName = QFileDialog::getExistingDirectory(initialDir, this,
        "getDirectoryDialog", "Select output directory");

    //
    // If a directory was chosen, use it as the output directory.
    //
    if(!dirName.isEmpty())
    {
        directoryNameLineEdit->setText(dirName);
        GetCurrentValues(1);
        Apply();
    }
}
