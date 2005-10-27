#ifndef QVIS_SESSION_FILE_DATABASE_LOADER_H
#define QVIS_SESSION_FILE_DATABASE_LOADER_H
#include <qobject.h>
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
//   
// ****************************************************************************

class QvisSessionFileDatabaseLoader : public QObject, public GUIBase
{
    Q_OBJECT
public:
    QvisSessionFileDatabaseLoader(QObject *parent, const char *name = 0);
    virtual ~QvisSessionFileDatabaseLoader();

    void SetDatabases(const stringVector &s);
    void Start(const QString &sfn);
signals:
    void complete(const QString &);
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
