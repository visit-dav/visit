#ifndef QVIS_LINE_STYLE_WIDGET_H
#define QVIS_LINE_STYLE_WIDGET_H
#include <gui_exports.h>
#include <qwidget.h>

class QComboBox;

// ****************************************************************************
// Class: QvisLineStyleWidget
//
// Purpose:
//   This widget contains a small option menu for selecting line styles.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 14:39:02 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Dec 1 16:31:36 PST 2000
//   Added a method to change the pixmap strings. It is used before the
//   pixmaps are created.
//
// ****************************************************************************

class GUI_API QvisLineStyleWidget : public QWidget
{
    Q_OBJECT
public:
    QvisLineStyleWidget(int style_ = 0, QWidget *parent = 0,
                        const char *name = 0);
    virtual ~QvisLineStyleWidget();
    void SetLineStyle(int style);
    int  GetLineStyle() const;
signals:
    void lineStyleChanged(int);
public slots:
    virtual void setEnabled(bool val);
private:
    void AugmentPixmap(const char *xpm[]);

    QComboBox         *lineStyleComboBox;
    static const char *style1[];
    static const char *style2[];
    static const char *style3[];
    static const char *style4[];
    static char       *augmentedData[];
    static char        augmentedForeground[];
};

#endif
