#ifndef QVIS_LINE_WIDTH_WIDGET_H
#define QVIS_LINE_WIDTH_WIDGET_H
#include <gui_exports.h>
#include <qwidget.h>

class QComboBox;

// ****************************************************************************
// Class: QvisLineWidthWidget
//
// Purpose:
//   This widget contains a small option menu for selecting line widths.
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
//   Kathleen Bonnell, Tue Dec  3 16:14:25 PST 2002 
//   Added more styles for larger line widths. 
//   
// ****************************************************************************

class GUI_API QvisLineWidthWidget : public QWidget
{
    Q_OBJECT
public:
    QvisLineWidthWidget(int width_ = 0, QWidget *parent = 0,
                        const char *name = 0);
    virtual ~QvisLineWidthWidget();
    void SetLineWidth(int style);
    int  GetLineWidth() const;
signals:
    void lineWidthChanged(int);
public slots:
    virtual void setEnabled(bool val);
private:
    void AugmentPixmap(const char *xpm[]);

    QComboBox         *lineWidthComboBox;
    static const char *style1[];
    static const char *style2[];
    static const char *style3[];
    static const char *style4[];
    static const char *style5[];
    static const char *style6[];
    static const char *style7[];
    static const char *style8[];
    static const char *style9[];
    static const char *style10[];
    static char       *augmentedData[];
    static char        augmentedForeground[];
};

#endif
