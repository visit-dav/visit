#ifndef QVIS_VCR_CONTROLS_H
#define QVIS_VCR_CONTROLS_H
#include <gui_exports.h>
#include <qwidget.h>

class QPushButton;

// *******************************************************************
// Class: QvisVCRControl
//
// Purpose:
//   This is a widget that encapsulates the individual VCR buttons
//   into a new VCR controls widget.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 24 13:41:24 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Sep 1 10:15:50 PDT 2000
//   Changed the signal names and made it inherit directly from
//   the QWidget class.
//
//   Brad Whitlock, Fri Dec 1 16:31:36 PST 2000
//   Added a method to change the pixmap strings. It is used before the
//   pixmaps are created.
//
// *******************************************************************

class GUI_API QvisVCRControl : public QWidget
{
    Q_OBJECT
public:
    QvisVCRControl(QWidget * parent=0, const char * name=0);
    ~QvisVCRControl();
    void SetActiveButton(int btn);
signals:
    void prevFrame();
    void reversePlay();
    void stop();
    void play();
    void nextFrame();
private slots:
    void b0_clicked();
    void b1_clicked();
    void b2_clicked();
    void b3_clicked();
    void b4_clicked();
private:
    void AugmentPixmap(const char *xpm[]);

    int          activeButton;
    QPushButton *buttons[5];

    static char *augmentedData[];
    static char  augmentedForeground[];
};

#endif
