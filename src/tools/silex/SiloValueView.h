#ifndef SILOVALUEVIEW_H
#define SILOVALUEVIEW_H

#include <qmainwindow.h>
class SiloFile;

// ****************************************************************************
//  Class:  SiloValueViewWindow
//
//  Purpose:
//    Window which views a single value.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
class SiloValueViewWindow : public QMainWindow
{
    Q_OBJECT
  public:
    SiloValueViewWindow(SiloFile *s, const QString &n, QWidget *p);
  private:
    SiloFile *silo;
    QString   name;
};


#endif
