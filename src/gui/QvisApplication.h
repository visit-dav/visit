#ifndef QVIS_APPLICATION_H
#define QVIS_APPLICATION_H
#include <qapplication.h>

// ****************************************************************************
// Class: QvisApplication
//
// Purpose:
//   This class inherits from QApplication so it can redefine some event
//   handling methods on MacOS X. Otherwise it is the same as QApplication.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 4 10:17:21 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class QvisApplication : public QApplication
{
    Q_OBJECT
public:
    QvisApplication(int &argc, char **argv);
    QvisApplication(int &argc, char **argv, bool GUIenabled);
    virtual ~QvisApplication();
    
signals:
    void showApplication();
    void hideApplication();

#ifdef Q_WS_MACX
public:
    virtual bool macEventFilter(EventRef);
#endif
};
#endif
