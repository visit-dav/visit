#include <visit-config.h>
#include <slivr/CM2Widget.h>
#include <slivr/VolumeRenderer.h>
#include <slivr/VideoCardInfo.h>
#include <slivr/ShaderProgramARB.h>
#include <slivr/ColorMap2.h>
#include <slivr/Plane.h>

#include <QvisCMap2Display.h>
#include <QCursor>
#include <QImage>
#include <QList>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>

// ****************************************************************************
// Class: WidgetRenderer
//
// Purpose:
//   This is a subclass of TextureRenderer that renders its cmap2 widgets into
//   a texture that it then uses to texture a rectangle.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 7 14:52:53 PST 2007
//
// Modifications:
//   Brad Whitlock, Mon Jan 12 16:38:53 PST 2009
//   I changed the draw method so it does not do anything if there are no
//   widgets to draw. This seems to fix a problem with drawing the empty widget.
//
// ****************************************************************************

class WidgetRenderer : public SLIVR::TextureRenderer
{
public:
    WidgetRenderer(std::vector<SLIVR::ColorMap2*> &cmap2, int tex_mem) : 
        SLIVR::TextureRenderer(0, 0, cmap2, tex_mem)
    {
    }

    virtual ~WidgetRenderer()
    {
    }

    void make_colormap()
    {
        // build the color map.
        build_colormap2();
    }

    void draw()
    {
        // Find the number of widgets
        int nWidgets = 0;
        for(int c = 0; c < cmap2_.size(); ++c)
        {
            std::vector<SLIVR::CM2Widget*> &widgets = cmap2_[c]->widgets();
            nWidgets += widgets.size();
        }
        if(nWidgets < 1)
            return;

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        // Texture a quad using the widget texture
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, cmap2_widget_tex_id_);
        glBegin(GL_QUADS);
        {
          glTexCoord2f( 0.0,  0.0);
          glVertex2f( 0.0,  0.0);
          glTexCoord2f(1.0,  0.0);
          glVertex2f( 1.0,  0.0);
          glTexCoord2f(1.0,  1.0);
          glVertex2f( 1.0,  1.0);
          glTexCoord2f( 0.0,  1.0);
          glVertex2f( 0.0,  1.0);
        }
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);

        // Draw the widgets
        for(int c = 0; c < cmap2_.size(); ++c)
        {
            std::vector<SLIVR::CM2Widget*> &widgets = cmap2_[c]->widgets();
            for(int w = 0; w < widgets.size(); ++w)
                 widgets[w]->draw();
        }
    }
};

WidgetID QvisCMap2Display::WIDGET_NOT_FOUND = -1;

// ****************************************************************************
// Method: QvisCMap2Display::QvisCMap2Display
//
// Purpose: 
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 13:55:08 PDT 2008
//
// Modifications:
//   Brad Whitlock, Tue Sep 30 10:13:29 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisCMap2Display::QvisCMap2Display(QWidget *parent) : 
  QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::AlphaChannel), parent), 
  init_done(false), defaultColor(1., 1., 1.), defaultAlpha(0.8), 
  idToWidget(), nextID(0), histogram_texture(0)
{
    activeW = 0;
    hist_data = 0;
    hist_size = 0;

    // Add the widgets to the colormap.
    SLIVR::ColorMap2 *c = new SLIVR::ColorMap2;
    cmap2.push_back(c);

    // Create a renderer to draw the widgets.
    ren = new WidgetRenderer(cmap2, 256*1000*1000);
}

// ****************************************************************************
// Method: QvisCMap2Display::~QvisCMap2Display
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 30 10:21:06 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QvisCMap2Display::~QvisCMap2Display()
{
    // Delete the cmap2 color maps...

    delete ren;
}

// ****************************************************************************
// Method: QvisCMap2Display::sizeHint
//
// Purpose: 
//   Returns the widget's size hint.
//
// Returns:    The widget's size hint.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 30 10:21:20 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisCMap2Display::sizeHint() const
{
    return QSize(200,200);
}

// ****************************************************************************
// Method: QvisCMap2Display::sizePolicy
//
// Purpose: 
//   Returns the widget's size policy.
//
// Returns:    The widget's size policy.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 30 10:21:44 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QSizePolicy
QvisCMap2Display::sizePolicy() const
{
    QSizePolicy s(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    return s;
}

// ****************************************************************************
// Method: QvisCMap2Display::setHistogramTexture
//
// Purpose: 
//   Set the histogram texture data.
//
// Arguments:
//   data : The new texture data.
//   dim  : The size of the new texture data.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 12 16:50:53 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisCMap2Display::setHistogramTexture(const unsigned char *data, int size)
{
    if(hist_data != 0)
        delete [] hist_data;

    if(data != 0)
    {
        hist_data = new unsigned char[size*size];
        memcpy(hist_data, data, size*size*sizeof(unsigned char));
        hist_size = size;
    }
    else
    {
        hist_data = 0;
        hist_size = 0;
    }

    update();
}

// ****************************************************************************
// Method: QvisCMap2Display::paintGL
//
// Purpose: 
//   Paints the color table and the widgets onto the GL context.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 30 10:19:31 PDT 2008
//
// Modifications:
//   Josh Stratton, August 2008
//   Added histogram painting code.
//
//   Brad Whitlock, Fri Dec 12 16:42:55 PST 2008
//   Rewrote histogram painting code.
//
// ****************************************************************************

void
QvisCMap2Display::paintGL()
{
    // Make a texture from the color map widgets.
    ren->make_colormap();

    //
    // Draw the background and widgets
    //
    init();

    glDrawBuffer(GL_BACK);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(1.0); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_LIGHTING);

    if(hist_data != 0)
    {
        if(histogram_texture == 0)
            glGenTextures(1, &histogram_texture);

        // read to a texture
        glBindTexture(GL_TEXTURE_2D, histogram_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, hist_size, hist_size,
                     0, GL_LUMINANCE, GL_UNSIGNED_BYTE, hist_data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // draw the texture to the screen
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, histogram_texture);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glBegin(GL_QUADS);
        {
          glTexCoord2f(0,0);
          glVertex2f(0,0);
          glTexCoord2f(1,0);
          glVertex2f(1,0);
          glTexCoord2f(1,1);
          glVertex2f(1,1);
          glTexCoord2f(0,1);
          glVertex2f(0,1);
        }
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }
    else
    {
        glColor3f(0.,0.,0.);
        glBegin(GL_QUADS);
        glVertex2f( 0.0,  0.0);
        glVertex2f( 1.0,  0.0);
        glVertex2f( 1.0,  1.0);
        glVertex2f( 0.0,  1.0);
        glEnd();
    }

    // Draw the widgets.
    ren->draw();

    glFlush();
    swapBuffers();
}

// ****************************************************************************
// Method: QvisCMap2Display::init
//
// Purpose: 
//   Initializes the GL context.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 30 10:15:59 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisCMap2Display::init()
{
    makeCurrent();

    // Must call init_shaders_supported, it also inits glew.
    if (!init_done)
        SLIVR::ShaderProgramARB::init_shaders_supported();

    float w = width();
    float h = height();

    glViewport(0, 0, (GLint)w, (GLint)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glOrtho(0., 1., 0., 1., -1., 1.);

    init_done = true;
}

// ****************************************************************************
// Method: QvisCMap2Display::ChangeCursor
//
// Purpose: 
//   Changes the Qt cursor based on the SLIVR cursor name.
//
// Arguments:
//   tkcursor : The name of the Cursor in SLIVR.
//
// Returns:    True if the cursor was set; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 30 10:15:03 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
QvisCMap2Display::ChangeCursor(const QString &tkcursor)
{
    bool retval = true;

    if(tkcursor == "fleur")
        setCursor(QCursor(Qt::SizeAllCursor));
    else if(tkcursor == "left_ptr")
        setCursor(QCursor(Qt::ArrowCursor));
    else if(tkcursor == "right_ptr")
        setCursor(QCursor(Qt::ArrowCursor));
    else if(tkcursor == "sb_h_double_arrow")
        setCursor(QCursor(Qt::SplitHCursor));
    else if(tkcursor == "sb_v_double_arrow")
        setCursor(QCursor(Qt::SplitVCursor));
    else if(tkcursor == "bottom_left_corner")
        setCursor(QCursor(Qt::SizeBDiagCursor));
    else if(tkcursor == "bottom_right_corner")
        setCursor(QCursor(Qt::SizeFDiagCursor));
    else if(tkcursor == "top_right_corner")
        setCursor(QCursor(Qt::SizeBDiagCursor));
    else if(tkcursor == "top_left_corner")
        setCursor(QCursor(Qt::SizeFDiagCursor));
    else
        retval = false;

    return retval;
}

// ****************************************************************************
// Method: QvisCMap2Display::mousePressEvent
//
// Purpose: 
//   Called in response to a mouse press. We use the mouse press to select
//   a new widget.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 30 10:16:23 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisCMap2Display::mousePressEvent(QMouseEvent *e)
{
    int doColor = (e->button() == Qt::MidButton) ? 1 : 0;

    for(int c = 0; c < cmap2.size(); ++c)
    {
        std::vector<SLIVR::CM2Widget*> &widgets = cmap2[c]->widgets();
        for(int w = 0; w < widgets.size(); ++w)
        {
            int obj = widgets[w]->pick1(e->x(), height() - e->y(), width(), height());
            if(obj <= 0)
                obj = widgets[w]->pick2(e->x(), height() - e->y(), width(), height(), doColor);
            if(obj > 0)
            {
                activeW = widgets[w];
                activeW->select(obj);

                ren->set_colormap2(cmap2);
                updateGL();

                // Change the cursor.
                ChangeCursor(QString(activeW->tk_cursorname(obj).c_str()));

                // Signal selection?
                emit selectWidget(LookupWidgetId(activeW));

                return;
            }          
        }
    }
}

// ****************************************************************************
// Method: QvisCMap2Display::mouseMoveEvent
//
// Purpose: 
//   This method is called when the mouse moves and we're pressing. We use 
//   this method to move the active SLIVR widget.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 30 10:17:19 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisCMap2Display::mouseMoveEvent(QMouseEvent *e)
{
    if(activeW != 0)
    {
        activeW->move(e->x(), height() - e->y(), width(), height());

        ren->set_colormap2(cmap2);
        updateGL();
    }
}

// ****************************************************************************
// Method: QvisCMap2Display::mouseReleaseEvent
//
// Purpose: 
//   This method is called when we release the mouse. We use this method to 
//   unselect the active widget.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 30 10:17:19 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisCMap2Display::mouseReleaseEvent(QMouseEvent *e)
{
    if(activeW != 0)
    {
        activeW->release(e->x(), height() - e->y(), width(), height());
        activeW->unselect_all();

        ren->set_colormap2(cmap2);
        updateGL();

        // Restore the cursor.
        setCursor(QCursor(Qt::ArrowCursor));

        // signal that activeW released?
        emit widgetChanged(LookupWidgetId(activeW));
//        qDebug("release: %s", activeW->tcl_pickle().c_str());
    }

    activeW = 0;
}

WidgetID
QvisCMap2Display::LookupWidgetId(SLIVR::CM2Widget *w) const
{
    for(IdToWidgetMap::const_iterator it = idToWidget.begin();
        it != idToWidget.end(); ++it)
    {
        if(w == it.value())
            return it.key();
    }
    return WIDGET_NOT_FOUND;
}

void
QvisCMap2Display::setDefaultColor(const QColor &c)
{
    float r = float(c.red()) / 255.f;
    float g = float(c.green()) / 255.f;
    float b = float(c.blue()) / 255.f;
    defaultColor = SLIVR::Color(r,g,b);
}

QColor
QvisCMap2Display::getDefaultColor() const
{
    int r = (int)(defaultColor.r() * 255.f);
    int g = (int)(defaultColor.g() * 255.f);
    int b = (int)(defaultColor.b() * 255.f);
    return QColor(r,g,b);
}

void
QvisCMap2Display::setDefaultAlpha(float f)
{
    defaultAlpha = f;
}

float
QvisCMap2Display::getDefaultAlpha() const
{
    return defaultAlpha;
}

// ***************************************************************************
//  Modifications:
//    Kathleen Bonnell, Thu Sep 25 09:15:32 PDT 2008
//    Changed WId to WidgetID so code will compile on Windows.
//
// ***************************************************************************

WidgetID
QvisCMap2Display::addTriangleWidget(const QString &wName,
                          float base,  // X-coordinate of bottom point
                          float top_x, // Offset in X from top left point
                          float top_y, // Y-coordinate of top left point
                          float w,     // Width of top of triangle.
                          float bottom // Distance along triangle median
                          )
{
    SLIVR::TriangleCM2Widget *newW = 0;
    newW = new SLIVR::TriangleCM2Widget(base, top_x, top_y,
                         w, bottom);
    newW->set_name(wName.toStdString());
    newW->set_value_range(SLIVR::range_t(0., 1.));
    newW->set_color(defaultColor);
    newW->set_faux(true);
    newW->set_alpha(0.8);
    newW->set_onState(1);
    newW->set_shadeType(SLIVR::CM2_SHADE_FALLOFF);
    //    qDebug("%s", newW->tcl_pickle().c_str());

    // Append the new widget
    std::vector<SLIVR::CM2Widget*> &widgets = cmap2[0]->widgets();
    widgets.push_back(newW);

    WidgetID thisID = nextID++;
    idToWidget[thisID] = newW;

    ren->set_colormap2(cmap2);
    updateGL();

    emit widgetListChanged();

    return thisID;
}


// ***************************************************************************
//  Modifications:
//    Kathleen Bonnell, Thu Sep 25 09:15:32 PDT 2008
//    Changed WId to WidgetID so code will compile on Windows.
//
// ***************************************************************************

WidgetID
QvisCMap2Display::addRectangleWidget(const QString &wName,
                           float left_x,
                           float left_y,
                           float w,
                           float h,
                           float offset
                           )
{
    SLIVR::RectangleCM2Widget *newW = 0;
    SLIVR::CM2RectangleType type = SLIVR::CM2_RECTANGLE_1D;
    newW = new SLIVR::RectangleCM2Widget(type, left_x, left_y,
                          w, h, offset);

    newW->set_name(wName.toStdString());
    newW->set_color(defaultColor);
    newW->set_faux(true);
    newW->set_alpha(defaultAlpha);
    newW->set_onState(1);
    newW->set_shadeType(SLIVR::CM2_SHADE_FALLOFF);

    // Append the new widget
    std::vector<SLIVR::CM2Widget*> &widgets = cmap2[0]->widgets();
    widgets.push_back(newW);

    WidgetID thisID = nextID++;
    idToWidget[thisID] = newW;

    ren->set_colormap2(cmap2);
    updateGL();

    emit widgetListChanged();

    return thisID;
}


// ***************************************************************************
//  Modifications:
//    Kathleen Bonnell, Thu Sep 25 09:15:32 PDT 2008
//    Changed WId to WidgetID so code will compile on Windows.
//
// ***************************************************************************

WidgetID
QvisCMap2Display::addEllipsoidWidget(const QString &wName,
                                     float x,
                                     float y,
                                     float a,
                                     float b,
                                     float rot
                                     )
{
    SLIVR::EllipsoidCM2Widget *newW = 0;
    newW = new SLIVR::EllipsoidCM2Widget(x, y, a, b, rot);

    newW->set_name(wName.toStdString());
    newW->set_color(defaultColor);
    newW->set_faux(true);
    newW->set_alpha(defaultAlpha);
    newW->set_onState(1);
    newW->set_shadeType(SLIVR::CM2_SHADE_FALLOFF);

    // Append the new widget
    std::vector<SLIVR::CM2Widget*> &widgets = cmap2[0]->widgets();
    widgets.push_back(newW);

    WidgetID thisID = nextID++;
    idToWidget[thisID] = newW;

    ren->set_colormap2(cmap2);
    updateGL();

    //    qDebug("%s", newW->tcl_pickle().c_str());

    emit widgetListChanged();

    return thisID;
}


// ***************************************************************************
//  Modifications:
//    Kathleen Bonnell, Thu Sep 25 09:15:32 PDT 2008
//    Changed WId to WidgetID so code will compile on Windows.
//
// ***************************************************************************

WidgetID
QvisCMap2Display::addParaboloidWidget(const QString &wName,
                                      float top_x, float top_y,
                                      float bottom_x, float bottom_y,
                                      float left_x, float left_y,
                                      float right_x, float right_y)
{
    SLIVR::ParaboloidCM2Widget *newW = 0;
    newW = new SLIVR::ParaboloidCM2Widget(top_x, top_y, bottom_x, bottom_y, left_x, left_y, right_x, right_y);

    newW->set_name(wName.toStdString());
    newW->set_color(defaultColor);
    newW->set_faux(true);
    newW->set_alpha(defaultAlpha);
    newW->set_onState(1);
    newW->set_shadeType(SLIVR::CM2_SHADE_FALLOFF);

    // Append the new widget
    std::vector<SLIVR::CM2Widget*> &widgets = cmap2[0]->widgets();
    widgets.push_back(newW);

    WidgetID thisID = nextID++;
    idToWidget[thisID] = newW;

    ren->set_colormap2(cmap2);
    updateGL();

    //    qDebug("%s", newW->tcl_pickle().c_str());

    emit widgetListChanged();

    return thisID;
}

//
// Set/Get methods.
//

void
QvisCMap2Display::setName(WidgetID id, const QString &n)
{
    IdToWidgetMap::iterator it =  idToWidget.find(id);
    if(it != idToWidget.end())
    {
        it.value()->set_name(n.toStdString());

        emit widgetChanged(id);
        emit widgetListChanged();
    }
}

QString
QvisCMap2Display::getName(WidgetID id) const
{
    QString retval;
    IdToWidgetMap::const_iterator it =  idToWidget.find(id);
    if(it != idToWidget.end())
        retval = QString(it.value()->get_name().c_str());
    return retval;
}

void
QvisCMap2Display::setColor(WidgetID id, const QColor &c)
{
    IdToWidgetMap::iterator it =  idToWidget.find(id);
    if(it != idToWidget.end())
    {
        float r = float(c.red()) / 255.f;
        float g = float(c.green()) / 255.f;
        float b = float(c.blue()) / 255.f;
        it.value()->set_color(SLIVR::Color(r,g,b));

        ren->set_colormap2(cmap2);
        updateGL();

        emit widgetChanged(id);
    }
}

QColor
QvisCMap2Display::getColor(WidgetID id) const
{
    QColor retval;
    IdToWidgetMap::const_iterator it =  idToWidget.find(id);
    if(it != idToWidget.end())
    {
        int r = (int)(it.value()->get_color().r() * 255.f);
        int g = (int)(it.value()->get_color().g() * 255.f);
        int b = (int)(it.value()->get_color().b() * 255.f);
        retval = QColor(r,g,b);
    }
    return retval;
}

void
QvisCMap2Display::setAlpha(WidgetID id, float a)
{
    IdToWidgetMap::iterator it =  idToWidget.find(id);
    if(it != idToWidget.end())
    {
        it.value()->set_alpha(a);

        ren->set_colormap2(cmap2);
        updateGL();

        emit widgetChanged(id);
    }
}

float
QvisCMap2Display::getAlpha(WidgetID id) const
{
    float retval;
    IdToWidgetMap::const_iterator it =  idToWidget.find(id);
    if(it != idToWidget.end())
        retval = it.value()->get_alpha();
    return retval;
}

void
QvisCMap2Display::setString(WidgetID id, const QString &s)
{
    IdToWidgetMap::iterator it =  idToWidget.find(id);
    if(it != idToWidget.end())
    {
        it.value()->tcl_unpickle(std::string(s.toStdString()));

        ren->set_colormap2(cmap2);
        updateGL();

        emit widgetChanged(id);
    }
}

QString
QvisCMap2Display::getString(WidgetID id) const
{
    QString retval;
    IdToWidgetMap::const_iterator it =  idToWidget.find(id);
    if(it != idToWidget.end())
        retval = it.value()->tcl_pickle().c_str();
    return retval;
}

//
// Other methods
//
int
QvisCMap2Display::numWidgets() const
{
    int retval = 0;
    for(int c = 0; c < cmap2.size(); ++c)
    {
        std::vector<SLIVR::CM2Widget*> &widgets = cmap2[c]->widgets();
        retval += widgets.size();
    }
    return retval;
}

void
QvisCMap2Display::removeWidget(WidgetID id)
{
    IdToWidgetMap::iterator it = idToWidget.find(id);
    if(it != idToWidget.end())
    {
        // Remove from the cmap.
        for(int c = 0; c < cmap2.size(); ++c)
        {
            std::vector<SLIVR::CM2Widget*> &widgets = cmap2[c]->widgets();
            std::vector<SLIVR::CM2Widget*>::iterator w = widgets.begin();
            for(; w != widgets.end(); ++w)
            { 
                if(*w == it.value())
                {
                    widgets.erase(w);
                    break;
                }
            }
        }
        // Remove from the idToWidget map.
        idToWidget.erase(it);

        // Redraw the widget
        ren->set_colormap2(cmap2);
        updateGL();

        // Tell the world that the widget list changed.
        emit widgetListChanged();
    } 
}

void
QvisCMap2Display::clear()
{
    for(int c = 0; c < cmap2.size(); ++c)
    {
        std::vector<SLIVR::CM2Widget*> &widgets = cmap2[c]->widgets();
        widgets.clear();
    }
    idToWidget.clear();

    // Redraw the widget
    ren->set_colormap2(cmap2);
    updateGL();

    // Tell the world that the widget list changed.
    emit widgetListChanged();   
}

WidgetID
QvisCMap2Display::getID(int index) const
{
    WidgetID retval = WIDGET_NOT_FOUND;
    QList<WidgetID> keys(idToWidget.keys());
    if(index >= 0 && index < keys.count())
        retval = keys[index];
    return retval;
}
