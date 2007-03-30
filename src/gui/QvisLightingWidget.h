#ifndef QVIS_LIGHTING_WIDGET_H
#define QVIS_LIGHTING_WIDGET_H
#include <gui_exports.h>
#include <qwidget.h>
#include <mini3D.h>

// ****************************************************************************
// Class: QvisLightingWidget
//
// Purpose:
//   This class uses the mini3D renderer to draw a 3D representation of the
//   lighting being used for plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:30:31 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 3 13:39:52 PST 2003
//   I updated the code to use the new m3d_renderer class instead of using
//   global m3d functions.
//
// ****************************************************************************

class GUI_API QvisLightingWidget : public QWidget
{
    Q_OBJECT
public:
    QvisLightingWidget(QWidget *parent, const char *name = 0);
    virtual ~QvisLightingWidget();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

    void setPreviewMode(bool val);
    void clearLights();
    void addLight(const double position[3], const QColor &c, int type);
    void setLightColor(const QColor &c);
    void setLightPosition(const double position[3]);
    void setLightType(int type);
signals:
    void lightMoved(double x, double y, double z);
protected:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

    void createSharedElements();
    void deleteBackingPixmap();
    void redrawScene(QPainter *painter);
    void setupAndDraw(QPainter *painter);

    void initializeCube();
    void initializeSphere();
    void initializeLitecone();
    void initializeLitearrow();
    void initializeLitebulb();

    QPixmap    *pixmap;
    bool        previewMode;
    bool        rendererCreated;
    bool        mouseDown;
    bool        needsRedrawPixmap;
    float       lastX;
    float       lastY;

    // Some internal state for lights.
    int         numberOfLights;

    m3d_light   lights[8];
    matrix4     view;
    matrix4     view2;
    matrix4     proj;

    m3d_renderer renderer;

    static bool sharedElementsCreated;
    static m3d_complex_element litearrow;
    static m3d_complex_element litecone;
    static m3d_complex_element litebulb;
    static m3d_complex_element sphere;
    static m3d_complex_element cube;
};


#endif
