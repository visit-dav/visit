#ifndef VIEWERPASSWORDWINDOW_H
#define VIEWERPASSWORDWINDOW_H
#include <qdialog.h>

// Forward declarations
class QLineEdit;
class QLabel;
class ViewerConnectionProgressDialog;

// ****************************************************************************
//  Class:  ViewerPasswordWindow
//
//  Purpose:
//    Main window for the program.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 25, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Aug 29 10:40:19 PDT 2002
//    I removed the Windows API stuff since this file is not part of the
//    viewer on Windows. I also added a new static function.
//
//    Brad Whitlock, Thu Aug 29 17:50:25 PST 2002
//    I added a userName argument to the getPassword and authenticate methods.
//
// ****************************************************************************

class ViewerPasswordWindow : public QDialog
{
    Q_OBJECT
  public:
    ViewerPasswordWindow(QWidget *parent=NULL, const char *name=NULL);
    ~ViewerPasswordWindow();

    static const char *getPassword(const char *, const char *, bool = false);
    static void authenticate(const char *, const char *, int);
    static void SetConnectionProgressDialog(ViewerConnectionProgressDialog *d)
    {
        dialog = d;
    }
  private:
    QLineEdit *passedit;
    QLabel    *label;
    static ViewerConnectionProgressDialog *dialog;
    static ViewerPasswordWindow *instance;
};

#endif
