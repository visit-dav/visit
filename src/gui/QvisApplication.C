#include <QvisApplication.h>

#ifdef Q_WS_MACX
// Include some MacOS X stuff
#include <Carbon/Carbon.h>
#include <visit-config.h>
#endif

// ****************************************************************************
// Method: QvisApplication::QvisApplication
//
// Purpose: 
//   Constructor for the QvisApplication class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 4 10:18:47 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisApplication::QvisApplication( int &argc, char **argv) :
    QApplication(argc, argv)
{
}

QvisApplication::QvisApplication( int &argc, char **argv, bool GUIenabled ) :
    QApplication(argc, argv, GUIenabled)
{
}

// ****************************************************************************
// Method: QvisApplication::~QvisApplication
//
// Purpose: 
//   Destructor for the QvisApplication class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 4 10:18:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisApplication::~QvisApplication()
{
}

#ifdef Q_WS_MACX
// ****************************************************************************
// Method: QvisApplication::macEventFilter
//
// Purpose: 
//   Handles some events for VisIt when running on MacOS X.
//
// Arguments:
//   event : The event that we might trap.
//
// Returns:    false so the main event handler will also trap the event and
//             do the right thing with it. We use this method to trap for
//             window hide/show events for the main window. When we get such
//             an event, we emit hideApplication and showApplication signals
//             so we can tell the viewer to hide and show too.
//
// Note:       This code is only built on MacOS X.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 4 10:18:57 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
QvisApplication::macEventFilter(EventRef event)
{
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    
    switch(eclass)
    {
    case kEventClassWindow:
      { // new scope
        //qDebug("macEventFilter: kEventClassWindow");
        WindowRef wid;
        GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL,
                          sizeof(WindowRef), NULL, &wid);
        QWidget *widget = QWidget::find((WId)wid);
        if(widget)
        {
            if(ekind == kEventWindowShown)
            {
                QString mainWindowName(QString("VisIt ") + QString(VERSION));
                if(mainWindowName == QString(widget->name()))
                    emit showApplication();
                //qDebug("ekind = kEventWindowShown");
            }
            else if(ekind == kEventWindowHidden)
            {
                QString mainWindowName(QString("VisIt ") + QString(VERSION));
                if(mainWindowName == QString(widget->name()))
                    emit hideApplication();            
                //qDebug("ekind = kEventWindowHidden");
            }
        }        
        break;
      }
#if 0
    // Trap for other Carbon events.
    case kEventClassApplication:
        qDebug("kEventClassApplication");
        break;
    case kEventClassCommand:
        qDebug("kEventClassCommand");
        break;
    case kEventClassControl:
        qDebug("kEventClassControl");
        break;
    case kEventClassKeyboard:
        qDebug("kEventClassKeyboard");
        break;
    case kEventClassMenu:
        qDebug("kEventClassMenu");
        break;
    case kEventClassMouse:
        qDebug("kEventClassMouse");
        break;
#endif
    }

    return false;
}
#endif
