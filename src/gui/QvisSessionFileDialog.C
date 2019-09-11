// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisSessionFileDialog.h>
#include <FileServerList.h>

// ****************************************************************************
// Method: QvisSessionFileDialog::QvisSessionvisFileDialog
//
// Purpose:
//   Constructor for the QvisSessionFileDialog class.
//   This dialog is used for both Save and Restore of Session files.
//
// Arguments:
//   caption : The name of the window.
//
// Programmer: David Camp
// Creation:   Tue Aug  4 11:04:14 PDT 2015
//
// Modifications:
//   Kathleen Biagas, Tue Oct 13 09:23:54 PDT 2015
//   Save filter, file grouping settings from file server so they can
//   be restored when this dialog is finished.
//
// ****************************************************************************

QvisSessionFileDialog::QvisSessionFileDialog(const QString &caption) : 
    QvisFileOpenDialog(caption)
{
    origFilter = fileServer->GetFilter();
    autoFileGrouping  = fileServer->GetAutomaticFileGrouping();
    smartFileGrouping = fileServer->GetSmartFileGrouping();
    
    // this prevents the file list pane from flashing files filtered by
    // the filter stored in fileServer
    fileServer->SetFilter("*.session");

    // prevents sessionfiles from being grouped together
    fileServer->SetAutomaticFileGrouping(false);
    fileServer->SetSmartFileGrouping(false);
}

// ****************************************************************************
// Method: QvisSessionFileDialog::~QvisSessionFileDialog
//
// Purpose: 
//   Destructor.
//
// Programmer: David Camp
// Creation:   Tue Aug  4 11:04:14 PDT 2015
//
// Modifications:
//   Kathleen Biagas, Tue Oct 13 09:23:26 PDT 2015
//   Restore the original filter, grouping settings to the file server.
//
// ****************************************************************************

QvisSessionFileDialog::~QvisSessionFileDialog()
{
    RestoreFileServerSettings();
}


void
QvisSessionFileDialog::RestoreFileServerSettings()
{
    fileServer->SetFilter(origFilter);
    fileServer->SetAutomaticFileGrouping(autoFileGrouping);
    fileServer->SetSmartFileGrouping(smartFileGrouping);
}

// ****************************************************************************
// Method: QvisFileDialog::getFileName
//
// Purpose:
//   Function for getting the name of an existing file using VisIt's
//   File open window or creating a filename if in save mode.
//
// Arguments:
//   type        : Type of dlg Save or Load.
//   initialFile : The host, path, and filename of the initial file.
//   caption     : The name of the window.
//
// Returns:    The name of the file, selected or created, or a null QString if
//             the user did not select a file.
//
// Note:
//
// Programmer: David Camp
// Creation:   Thu Aug  6 07:32:42 PDT 2015
//
// Modifications:
//   Kathleen Biagas, Tue Oct 13 09:49:51 PDT 2015
//   Windows hasn't used '.vses' in a very long time, so don't use it.
//
//   Kathleen Biagas, Tue Mar 15 18:14:44 MST 2016
//   Restore fileserver settings after filename is retrieved, so that
//   SessionSourceChanger (if used) will be using the correct settings.
//
// ****************************************************************************

void
QvisSessionFileDialog::getFileName(DLG_TYPE type, 
                                   const QString &initialFile,
                                   QualifiedFilename &filename)
{
    QString qfilename;
    QString fltr("*.session");

    SetUsageMode(QvisFileOpenDialog::SelectFilename);
    SetHideFileFormat(true);
    SetHideOptions(true);
    SetShowFilename(true);
    qfilename = getOpenFileNameEx(initialFile, fltr);

    if(!qfilename.isEmpty())
    {
        filename.SetFromString(qfilename.toStdString());
    }

    RestoreFileServerSettings();
}

