// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SESSION_FILE_DATABASE_LOADER_H
#define QVIS_SESSION_FILE_DATABASE_LOADER_H
#include <QObject>
#include <GUIBase.h>
#include <vectortypes.h>

class CancelledObserver;

// ****************************************************************************
// Class: QvisSessionFileDatabaseLoader
//
// Purpose:
//   Helps load session file databases before we tell the viewer to actually
//   load a session file. This class listens for mdserver cancelations, allowing
//   the mdserver cancel button to cancel session loading when done from the
//   GUI.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 27 14:08:04 PST 2005
//
// Modifications:
//   Brad Whitlock, Tue Nov 14 15:20:22 PST 2006
//   Added another complete signal.
//
//   Cyrus Harrison, Tue Jul  1 09:14:16 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

class QvisSessionFileDatabaseLoader : public QObject, public GUIBase
{
    Q_OBJECT
public:
    QvisSessionFileDatabaseLoader(QObject *parent);
    virtual ~QvisSessionFileDatabaseLoader();

    void SetDatabases(const stringVector &s);
    void Start(const QString &sfn);
signals:
    void complete(const QString &);
    void complete(const QString &, const stringVector &);
    void loadFile(const QString &);
protected slots:
    void ProcessFile();

protected:
    int                 index;
    stringVector        databases;
    CancelledObserver  *cancelledObserver;
    QString             sessionFile;
};

#endif
