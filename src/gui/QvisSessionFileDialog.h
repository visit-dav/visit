// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SESSION_FILE_OPEN_DIALOG_H
#define QVIS_SESSION_FILE_OPEN_DIALOG_H

#include <QvisFileOpenDialog.h>
#include <string>

// ****************************************************************************
// Class: QvisSessionFileDialog
//
// Purpose:
//   This is a dialog version of the file open window for use to load or Save
//   Session files. It has support to save or load from remote hosts.
//
// Notes:      
//
// Programmer: David Camp
// Creation:   Tue Aug  4 11:04:14 PDT 2015
//
// Modifications:
//   Kathleen Biagas, Tue Oct 13 09:21:01 PDT 2015
//   Add storage for original filter, grouping settings retrieved from 
//   file server.
//
//   Kathleen Biagas, Tue Mar 15 18:27:17 MST 2016
//   Added RestoreFileServerSettings method.
//
// ****************************************************************************

class QvisSessionFileDialog : public QvisFileOpenDialog
{
public:
    enum DLG_TYPE { OPEN_DLG, SAVE_DLG };

    QvisSessionFileDialog(const QString &caption);
    virtual ~QvisSessionFileDialog();

    // Blocking function to use the dialog to get a filename.
    void getFileName(DLG_TYPE type,
                     const QString &initialFile, 
                     QualifiedFilename &filename);

private:
    void RestoreFileServerSettings(void);
    std::string origFilter;
    bool autoFileGrouping;
    bool smartFileGrouping;
};

#endif

