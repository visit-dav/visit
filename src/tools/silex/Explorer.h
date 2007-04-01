#ifndef EXPLORER_H
#define EXPLORER_H

#include <qmainwindow.h>

class SiloView;

// ****************************************************************************
//  Class:  Explorer
//
//  Purpose:
//    Main window for the Silo Explorer.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
class Explorer : public QMainWindow
{
    Q_OBJECT
  public:
    Explorer(const QString &file, QWidget *p, const QString &n);
    virtual ~Explorer();
  public slots:
    void open();
  private:
    SiloView *view;
};

#endif
