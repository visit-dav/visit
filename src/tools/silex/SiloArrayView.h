#ifndef SILOARRAYVIEW_H
#define SILOARRAYVIEW_H

#include <qmainwindow.h>
class SiloFile;
class QListBox;

// ****************************************************************************
//  Class:  SiloArrayViewWindow
//
//  Purpose:
//    Window which views an array.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
class SiloArrayViewWindow : public QMainWindow
{
    Q_OBJECT
  public:
    SiloArrayViewWindow(SiloFile *s, const QString &n, QWidget *p);

    virtual QSize sizeHint() const;
  private:
    QListBox *lb;
    SiloFile *silo;
    QString   name;
};

#endif
