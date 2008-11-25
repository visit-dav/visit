#ifndef QVIS_CMAP2_DISPLAY_H
#define QVIS_CMAP2_DISPLAY_H

#include <vector>

#include <visit-config.h>
#ifdef HAVE_LIBSLIVR
#   include <slivr/ColorMap2.h>
#   include <slivr/CM2Widget.h>
#endif
#include <qgl.h>
#include <QMap>

class WidgetRenderer;

typedef int WidgetID;

// ****************************************************************************
// Class: QvisCMap2Display
//
// Purpose:
//   This class provides a container for SLIVR CMap2 widgets so the user can
//   move them around interactively.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 7 14:50:58 PST 2007
//
// Modifications:
//   Tom Fogal, Thu Sep 18 16:54:34 MDT 2008
//   Use forward declarations where possible, and an ifdef where not, to remove
//   a hard SLIVR dependency.
//
//   Brad Whitlock, Tue Sep 30 09:47:39 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class QvisCMap2Display : public QGLWidget
{
    Q_OBJECT
public:
    typedef QMap<WidgetID, SLIVR::CM2Widget *> IdToWidgetMap;

    QvisCMap2Display(QWidget *parent);
    virtual ~QvisCMap2Display();

    virtual void paintGL();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;


    WidgetID addTriangleWidget(const QString &wName,
                          float base,  // X-coordinate of bottom point
                          float top_x, // Offset in X from top left point
                          float top_y, // Y-coordinate of top left point
                          float w,     // Width of top of triangle.
                          float bottom // Distance along triangle median
                          );

    WidgetID addRectangleWidget(const QString &wName,
                           float left_x,
                           float left_y,
                           float w,
                           float h,
                           float offset
                           );

    WidgetID addEllipsoidWidget(const QString &wName,
                                float x,
                                float y,
                                float a,
                                float b,
                                float rot
                                );

    WidgetID addParaboloidWidget(const QString &wName,
                                 float top_x, float top_y,
                                 float bottom_x, float bottom_y,
                                 float left_x, float left_y,
                                 float right_x, float right_y
                                 );

    int numWidgets() const;
    WidgetID getID(int index) const;
    void removeWidget(WidgetID id);

    void   setDefaultColor(const QColor &c);
    QColor getDefaultColor() const;

    void   setDefaultAlpha(float f);
    float  getDefaultAlpha() const;

    void   setColor(WidgetID id, const QColor &c);
    QColor getColor(WidgetID id) const;

    void   setAlpha(WidgetID id, float a);
    float  getAlpha(WidgetID id) const;

    void    setName(WidgetID id, const QString &name);
    QString getName(WidgetID id) const;

    void    setString(WidgetID id, const QString &s);
    QString getString(WidgetID id) const;

    static WidgetID WIDGET_NOT_FOUND;
signals:
    void widgetListChanged();
    void selectWidget(WidgetID id);
    void widgetChanged(WidgetID id);
protected:
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);

private:
    void init();
    bool ChangeCursor(const QString &tkcursor);
    WidgetID LookupWidgetId(SLIVR::CM2Widget *) const;

    GLuint                          histogram_texture;
    bool                            init_done;
    bool                            read_histogram;
    std::vector<SLIVR::ColorMap2*>  cmap2;
    SLIVR::CM2Widget               *activeW;
#ifdef HAVE_LIBSLIVR
    SLIVR::Color                    defaultColor;
#endif
    float                           defaultAlpha;
    IdToWidgetMap                   idToWidget;
    WidgetID                        nextID;
    WidgetRenderer                 *ren;
};
#endif
