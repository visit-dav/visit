// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_DB_OPTIONS_HELP_WINDOW_H
#define QVIS_DB_OPTIONS_HELP_WINDOW_H
#include <QDialog>

class QPushButton;
class DBOptionsAttributes;

// ****************************************************************************
// Class: QvisDBOptionsHelpWindow
//
// Purpose: Help window for DB options.
//
// Notes:      
//
// Creation: Mark C. Miller, Mon Dec 15 10:01:30 PST 2014
// ****************************************************************************

class QvisDBOptionsHelpWindow : public QDialog
{
    Q_OBJECT
public:
    QvisDBOptionsHelpWindow(DBOptionsAttributes *opts, QWidget *parent = 0);
    virtual ~QvisDBOptionsHelpWindow();
private:
    DBOptionsAttributes *atts;
    
    QPushButton         *dismissButton;
};

#endif
