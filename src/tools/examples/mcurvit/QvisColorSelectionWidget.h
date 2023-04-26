// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_COLOR_SELECTION_WIDGET_H
#define QVIS_COLOR_SELECTION_WIDGET_H
#include <gui_exports.h>
#include <QWidget>

// Forward declarations.
class QPushButton;
class QTimer;
class QVBoxLayout;
class QvisColorGridWidget;

// ****************************************************************************
// Class: QvisColorSelectionWidget
//
// Purpose:
//   This widget contains an array of colors from which the user may choose.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 14:30:50 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Oct 26 14:07:58 PST 2001
//   Added a timer to hide the widget after a little while.
//
//   Brad Whitlock, Tue Jun  3 15:30:09 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisColorSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    QvisColorSelectionWidget(QWidget *parent = 0, Qt::WindowFlags f = Qt::Widget);
    virtual ~QvisColorSelectionWidget();
    virtual QSize sizeHint() const;

    void setSelectedColor(const QColor &color);
signals:
    void selectedColor(const QColor &color);
public slots:
    virtual void show();
    virtual void hide();
protected:
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
private slots:
    void getCustomColor();
    void handleSelectedColor(const QColor &color);
private:
    int                 numCustomColors;
    QvisColorGridWidget *standardColorGrid;
    QvisColorGridWidget *customColorGrid;
    QPushButton         *moreColorsButton;
    QTimer              *timer;

    static const unsigned char colorComponents[];
    static const int MAX_CUSTOM_COLORS;
};

#endif
