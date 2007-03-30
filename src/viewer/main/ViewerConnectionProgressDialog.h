#ifndef VIEWER_CONNECTION_PROGRESS_DIALOG_H
#define VIEWER_CONNECTION_PROGRESS_DIALOG_H
#include <qwidget.h>

// Forward declarations.
class QLabel;
class QPushButton;
class QRadioButton;
class QTimer;

// ****************************************************************************
// Class: ViewerViewerConnectionProgressDialog
//
// Purpose:
//   This class contains a progress dialog that is displayed when we launch
//   processes and it takes a long time.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 26 13:20:25 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Oct 24 11:46:47 PDT 2002
//   I increased the default timeout to 6 seconds.
//
//   Brad Whitlock, Wed May 7 10:14:04 PDT 2003
//   I added methods to set/get the component name. I also added methods
//   to set/get a flag that determines if the window should be hidden.
//
// ****************************************************************************

class ViewerConnectionProgressDialog : public QWidget
{
    Q_OBJECT
public:
    ViewerConnectionProgressDialog(const QString &component,
        const QString &host, bool par, int t = 6000);
    virtual ~ViewerConnectionProgressDialog();

    bool getCancelled() const { return cancelled; };
    void setTimeout(int t) { timeout = t; };

    const QString &getComponentName() const { return componentName; };
    void setComponentName(const QString &cn);

    bool getIgnoreHide() const   { return ignoreHide; }
    void setIgnoreHide(bool val) { ignoreHide = val; }
public slots:
    virtual void show();
    virtual void hide();
private slots:
    void updateAnimation();
    void timedShow();
    void cancel();
private:
    QTimer       *timer;
    QPushButton  *cancelButton;
    QRadioButton *rb[6];
    QLabel       *msgLabel;
    QString       componentName;
    QString       hostName;
    bool          parallel;
    int           iconFrame;
    bool          cancelled;
    int           timeout;
    bool          cancelledShow;
    bool          ignoreHide;
};

#endif
