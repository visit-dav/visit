/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
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

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QWidget>

#include <QvisVariableButton.h>
#include <DebugStream.h>
#include <DBPluginInfoAttributes.h>
#include <ExportDBAttributes.h>
#include <FileServerList.h>
#include <Plot.h>
#include <PlotList.h>
#include <ViewerProxy.h>

#include <QvisDBOptionsDialog.h>

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
//    Brad Whitlock, Wed Apr  9 11:58:41 PDT 2008
//    QString for caption, shortName.
//
//    Kathleen Biagas, Wed Apr 23 14:49:26 MST 2014
//    Added plotList, localPlot.
//
// ****************************************************************************

QvisExportDBWindow::QvisExportDBWindow(
    const QString &caption, const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowSimpleObserver(caption, shortName, notepad,
                                QvisPostableWindowSimpleObserver::ApplyButton)
{
    exportDBAtts = NULL;
    dbPluginInfoAtts = NULL;
    delimiter = 0;
    plotList = 0;
    localPlot = true;
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
//    Kathleen Biagas, Wed Apr 23 14:49:26 MST 2014
//    Added plotList.
//
// ****************************************************************************

QvisExportDBWindow::~QvisExportDBWindow()
{
    if (plotList)
        plotList->Detach(this);
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
//    Kathleen Biagas, Wed Apr 23 14:49:26 MST 2014
//    Added plotList.
//
// ****************************************************************************

void
QvisExportDBWindow::ConnectSubjects(ExportDBAttributes *edb,
                                    DBPluginInfoAttributes *dbp,
                                    PlotList *pl)
{
    exportDBAtts = edb;
    exportDBAtts->Attach(this);

    dbPluginInfoAtts = dbp;
    dbPluginInfoAtts->Attach(this);

    plotList = pl;
    plotList->Attach(this);
}


// ****************************************************************************
// Method: QvisExportDBWindow::Update
//
// Purpose: 
//   This function is called when a subject is updated.
//
// Arguments:
//   TheChangedSubject : The subject being changed.
//
// Programmer: Kathleen Biagas
// Creation:   April 23, 2014
//
// Modifications:
//
// ****************************************************************************

void
QvisExportDBWindow::Update(Subject *TheChangedSubject)
{
    if (plotList != 0 &&
        TheChangedSubject == plotList &&
        plotList->GetNumPlots() > 0)
    {
        int sel = plotList->FirstSelectedIndex();
        if (sel >=0)
        {
            QualifiedFilename dbName(plotList->GetPlots(sel).GetDatabaseName());
            localPlot = (dbName.host == "localhost");
             
            bool enableDirSelect = fileFormatComboBox->currentText() !=
                                 QString("SimV1");
            directorySelectButton->setEnabled(localPlot && enableDirSelect);
        }
    }
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
//    Kathleen Biagas, Wed Apr 23 14:50:24 MST 2014
//    Added plotList.
//
// ****************************************************************************

void
QvisExportDBWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if (TheRemovedSubject == exportDBAtts)
        exportDBAtts = 0;
    else if (TheRemovedSubject == dbPluginInfoAtts)
        dbPluginInfoAtts = 0;
    else if (TheRemovedSubject == plotList)
        plotList = 0;
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
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
//   Brad Whitlock, Mon Feb 10 17:31:32 PST 2014
//   Let the user pick the delimiter.
//   Work partially supported by DOE Grant SC0007548.
//
//   Kathleen Biagas, Wed Apr 23 15:31:45 PDT 2014
//   Enable directorySelectButton based on host for active plot (if any).
// 
//   Brad Whitlock, Thu Jul 24 13:55:34 EDT 2014
//   Added check box for doing all time steps.
//
// ****************************************************************************

void
QvisExportDBWindow::CreateWindowContents()
{
    // Create a group box for the file information.
    QGroupBox *infoBox = new QGroupBox(central);
    infoBox->setTitle(tr("Output"));
    topLayout->addWidget(infoBox);

    QGridLayout *infoLayout = new QGridLayout(infoBox);
    infoLayout->setMargin(5);

    allTimes = new QCheckBox(tr("Export all time states"), infoBox);
    connect(allTimes, SIGNAL(toggled(bool)),
            this, SLOT(allTimesToggled(bool)));
    infoLayout->addWidget(allTimes, 0, 0, 1, 2);

    filenameLineEdit = new QLineEdit(infoBox);
    connect(filenameLineEdit, SIGNAL(returnPressed()), this, SLOT(processFilenameText()));
    QLabel *filenameLabel = new QLabel(tr("File name"), infoBox);
    infoLayout->addWidget(filenameLabel, 1, 0);
    infoLayout->addWidget(filenameLineEdit, 1, 1);

    directoryNameLabel = new QLabel(tr("Directory name"), infoBox);
    
    QHBoxLayout *directoryLayout = new QHBoxLayout();
    directoryNameLineEdit = new QLineEdit(infoBox);
    directorySelectButton = new QPushButton("...",infoBox);
    connect(directoryNameLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processDirectoryNameText()));
    directoryLayout->addWidget(directoryNameLineEdit);
    directoryLayout->addWidget(directorySelectButton);
    
#if !(defined(Q_WS_MACX) || defined(Q_OS_MAC))
    directorySelectButton->setMaximumWidth(
         fontMetrics().boundingRect("...").width() + 6);
#endif
    directorySelectButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
         QSizePolicy::Minimum));
    connect(directorySelectButton, SIGNAL(clicked()),
            this, SLOT(selectOutputDirectory()));
    directoryLayout->setSpacing(0);
    
    infoLayout->addWidget(directoryNameLabel, 2, 0);
    infoLayout->addLayout(directoryLayout, 2, 1);

    fileFormatComboBox = new QComboBox(infoBox);
    fileFormatComboBox->clear();

    connect(fileFormatComboBox, SIGNAL(activated(int)),
           this, SLOT(fileFormatChanged(int)));
    QLabel *formatLabel = new QLabel(tr("Export to"),infoBox);
    infoLayout->addWidget(formatLabel, 3, 0);
    infoLayout->addWidget(fileFormatComboBox, 3, 1);

    QGroupBox *varGroup = new QGroupBox(tr("Variables"), infoBox);
    infoLayout->addWidget(varGroup, 4, 0, 1, 2);
    QGridLayout *varLayout = new QGridLayout(varGroup);
    varLayout->setMargin(5);
    QButtonGroup *delimGroup = new QButtonGroup(0);
    QRadioButton *rb0 = new QRadioButton(tr("Space"), varGroup);
    QRadioButton *rb1 = new QRadioButton(tr("Comma"), varGroup);
    delimGroup->addButton(rb0, 0);
    delimGroup->addButton(rb1, 1);
    varLayout->addWidget(new QLabel(tr("Delimiter"), varGroup), 0, 0);
    varLayout->addWidget(rb0, 0, 1);
    varLayout->addWidget(rb1, 0, 2);
    rb0->setChecked(true);
    connect(delimGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(delimiterChanged(int)));

    varsButton = new QvisVariableButton(true, false, true, -1,varGroup);
    varsButton->setText(tr("Add Variable"));
    varsButton->setChangeTextOnVariableChange(false);
    connect(varsButton, SIGNAL(activated(const QString &)),
            this, SLOT(addVariable(const QString &)));
    varLayout->addWidget(varsButton, 1, 0);

    varsLineEdit = new QLineEdit(varGroup);
    varsLineEdit->setText("default");
    connect(varsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(variableProcessText()));
    varLayout->addWidget(varsLineEdit, 1, 1, 1, 2);

    // The export button.
    QHBoxLayout *exportButtonLayout = new QHBoxLayout();
    topLayout->addLayout(exportButtonLayout);
    //exportButtonLayout->setSpacing(5);

    QPushButton *exportButton = new QPushButton(tr("Export"), central);
    connect(exportButton, SIGNAL(clicked()),
            this, SLOT(exportButtonClicked()));
    exportButtonLayout->addWidget(exportButton);
    exportButtonLayout->addStretch(50);

    // The information about the file format is only updated when it is
    // changed.  So if it is not changed, then it never updates.  So tell it
    // now that it has changed, in case anyone tries to export a database using
    // the first option (i.e. without changing it).
    fileFormatChanged(0);

    if (plotList)
    {
        int sel = plotList->FirstSelectedIndex();
        if (sel >=0)
        {
            QualifiedFilename dbName(plotList->GetPlots(sel).GetDatabaseName());
            localPlot = (dbName.host == "localhost");
            directorySelectButton->setEnabled(localPlot);
        }
    }
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
//   Jeremy Meredith, Fri Oct 12 10:35:42 EDT 2007
//   Added trigger of the full file format update when we have to choose
//   one for the user (because none was selected).
//
//   Brad Whitlock, Fri Dec 14 17:06:39 PST 2007
//   Made it use ids.
//
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
//   Kathleen Biagas, Wed Apr 23 14:50:24 MST 2014
//   Make directorySelectButton also dependent upon the plot being local.
//
//   Brad Whitlock, Thu Jul 24 13:55:34 EDT 2014
//   Added check box for doing all time steps.
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
          case ExportDBAttributes::ID_allTimes:
            allTimes->blockSignals(true);
            allTimes->setChecked(exportDBAtts->GetAllTimes());
            allTimes->blockSignals(false);
            break;
          case ExportDBAttributes::ID_db_type:
            {
                fileFormatComboBox->blockSignals(true);
                fileFormatComboBox->clear();
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
                        fileFormatComboBox->addItem(
                            dbPluginInfoAtts->GetTypes()[j].c_str());
                    }
                if (curItem >= 0)
                    fileFormatComboBox->setCurrentIndex(curItem);
                else if (nItems > 0)
                {
                    fileFormatComboBox->setCurrentIndex(0);
                    fileFormatChanged(0);
                }
                fileFormatComboBox->blockSignals(false);

                // This isn't so clean since we should not be doing things
                // based on a plugin name but it's okay for now.
                // Disable directories if the database type is a simulation.
                bool enableDir = !fileFormatComboBox->currentText().startsWith("SimV");
                directoryNameLineEdit->setEnabled(enableDir);
                directoryNameLabel->setEnabled(enableDir);
                directorySelectButton->setEnabled(enableDir && localPlot);
            }
            break;
          case ExportDBAttributes::ID_db_type_fullname:
            break;
          case ExportDBAttributes::ID_filename:
            filenameLineEdit->setText(exportDBAtts->GetFilename().c_str());
            break;
          case ExportDBAttributes::ID_dirname:
            directoryNameLineEdit->setText(
                                     exportDBAtts->GetDirname().c_str());
            break;
          case ExportDBAttributes::ID_variables:
            UpdateVariablesList();
            break;
        }
    } // end for
}

// ****************************************************************************
// Method: QvisExportDBWindow::UpdateVariablesList
//
// Purpose:
//   Set the var text from the list of variables that we'll export.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 13:49:09 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
QvisExportDBWindow::UpdateVariablesList()
{
    const stringVector &vars = exportDBAtts->GetVariables();
    std::string allVars;
    for (size_t i = 0; i < vars.size(); i++)
    {
        allVars += vars[i];
        if(i < vars.size()-1)
            allVars += Delimiter().toStdString();
    }
    varsLineEdit->setText(allVars.c_str());
}

// ****************************************************************************
// Method: QvisExportDBWindow::Delimiter
//
// Purpose:
//   Return the delimiter string used to split the variable list.
//
// Returns:    The delimiter string.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 13:48:35 PDT 2014
//
// Modifications:
//
// ****************************************************************************

QString
QvisExportDBWindow::Delimiter() const
{
    return QString(delimiter == 0 ? " " : ",");
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
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//   
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Fixed problem with empty string from split propagating into the export
//   vars.
//   
//   Brad Whitlock, Thu Mar 20 13:47:57 PDT 2014
//   I added support for delimiters other than spaces.
//   Work partially supported by DOE Grant SC0007548.
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
        temp = filenameLineEdit->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            exportDBAtts->SetFilename(temp.toStdString());
        }
        else
        {
            msg = tr("The filename was invalid. "
                     "Resetting to the last good value \"%1\".").
                  arg(exportDBAtts->GetFilename().c_str());
            Message(msg);
            exportDBAtts->SetFilename(exportDBAtts->GetFilename());
        }
    }
    if(which_widget == 1 || doAll)
    {
        temp = directoryNameLineEdit->displayText().simplified();
        exportDBAtts->SetDirname(temp.toStdString());
    }
    if(which_widget == 2 || doAll)
    {
        QString temp;
        stringVector vars;
        temp = varsLineEdit->displayText().simplified();
        QStringList lst = temp.split(Delimiter(),QString::SkipEmptyParts);
        
        QStringListIterator it(lst);
        while(it.hasNext())
            vars.push_back(it.next().simplified().toStdString());

        exportDBAtts->SetVariables(vars);
    }
}

// ****************************************************************************
// Method: QvisExportDBWindow::Apply
//
// Purpose: 
//   This method applies the export attributes and optionally tells
//   the viewer to apply them.
//
// Arguments:
//   ignore : This flag, when true, tells the code to ignore the
//            AutoUpdate function and tell the viewer to apply the
//            export attributes.
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
        // Get the current export attributes and tell the other
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
//    Jeremy Meredith, Fri Oct 12 10:38:02 EDT 2007
//    Enhancements to attribute groups allowed separate vectors for the
//    read and write options, so I switched to this simpler organization.
//
// ****************************************************************************

void
QvisExportDBWindow::fileFormatChanged(int)
{
    if (dbPluginInfoAtts == 0 || exportDBAtts == 0)
        return;

    QString tmp = fileFormatComboBox->currentText();
    if (tmp.isNull())
        return;

    exportDBAtts->SetDb_type(tmp.toStdString());
    
    int ntypes = dbPluginInfoAtts->GetTypes().size();
    for (int i = 0 ; i < ntypes ; i++)
    {
        if (dbPluginInfoAtts->GetTypes()[i] == tmp.toStdString())
        {
            exportDBAtts->SetDb_type_fullname(
                                     dbPluginInfoAtts->GetTypesFullNames()[i]);
            DBOptionsAttributes &atts = 
                               dbPluginInfoAtts->GetDbWriteOptions(i);
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
//   Brad Whitlock, Thu Mar 20 14:04:35 PDT 2014
//   Show license text.
//   Work partially supported by DOE Grant SC0007548.
//
// ****************************************************************************

void
QvisExportDBWindow::exportDB()
{
    Apply();

    // See if we can get a license string for the plugin.
    QString license;
    int ntypes = dbPluginInfoAtts->GetTypes().size();
    for (int i = 0 ; i < ntypes ; i++)
    {
        if (dbPluginInfoAtts->GetTypes()[i] == exportDBAtts->GetDb_type())
        {
            license = QString(dbPluginInfoAtts->GetLicense()[i].c_str());
            break;
        }
    }
    // If we have a non-empty license string that we have not shown before,
    // show the license.
    if(!license.isEmpty())
    {
        QString dbType(exportDBAtts->GetDb_type().c_str());
        if(licenseShown.indexOf(dbType) == -1)
        {
            QMessageBox::information(this, tr("%1 License").arg(dbType), license);
            licenseShown.append(dbType);
        }
    }

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
//   Jeremy Meredith, Fri Oct 12 10:38:24 EDT 2007
//   Added a modal dialog to get the options for exporting.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisExportDBWindow::exportButtonClicked()
{
    apply();

    int result = QDialog::Accepted;
    if (exportDBAtts->GetOpts().GetNumberOfOptions() > 0)
    {
        QvisDBOptionsDialog *optsdlg =
            new QvisDBOptionsDialog(&(exportDBAtts->GetOpts()), NULL);
        QString caption = tr("Export options for %1 writer").
                          arg(exportDBAtts->GetDb_type().c_str());
        optsdlg->setWindowTitle(caption);
        result = optsdlg->exec();
        delete optsdlg;
    }
    if (result == QDialog::Accepted)
    {
        exportDB();
    }
    else
    {
        // rejected
    }
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
        varString += Delimiter();
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
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//   
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisExportDBWindow::selectOutputDirectory()
{
    //
    // Try and get a directory using a file dialog.
    //
    QString initialDir(exportDBAtts->GetDirname().c_str());
    QString dirName = QFileDialog::getExistingDirectory(this,
                           tr("Select output directory"),initialDir);

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

// ****************************************************************************
// Method: QvisExportDBWindow::delimiterChanged
//
// Purpose:
//   This is a Qt slot function that we call to update the variable list when
//   the selected delimiter changes.
//
// Arguments:
//   val  : The new delimiter index.
//
// Returns:    
//
// Note:       Work partially supported by DOE Grant SC0007548.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr  7 15:43:35 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
QvisExportDBWindow::delimiterChanged(int val)
{
    delimiter = val;
    UpdateVariablesList();
}

// ****************************************************************************
// Method: QvisExportDBWindow::allTimesToggled
//
// Purpose:
//   This is a Qt slot function that we call when the allTimes checkbox is toggled.
//
// Arguments:
//   val  : The new toggle value
//
// Returns:    
//
// Note:       Work partially supported by DOE Grant SC0007548.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 24 14:12:23 EDT 2014
//
// Modifications:
//
// ****************************************************************************

void
QvisExportDBWindow::allTimesToggled(bool val)
{
    exportDBAtts->SetAllTimes(val);
    SetUpdate(false);
    Apply();
}