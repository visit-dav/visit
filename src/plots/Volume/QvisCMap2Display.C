#include <slivr/CM2Widget.h>
#include <slivr/VolumeRenderer.h>
#include <slivr/VideoCardInfo.h>
#include <slivr/ShaderProgramARB.h>
#include <slivr/ColorMap2.h>
#include <slivr/Plane.h>

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <QvisCMap2Display.h>
#include <qcursor.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qimage.h>

#include <iostream>

#include <avtVolumeRenderer.h> // just for USE_HISTOGRAM

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
//   
// ****************************************************************************

QvisCMap2Display::QvisCMap2Display(QWidget *parent, const char *name) : 
  QGLWidget(QGLFormat(DoubleBuffer | AlphaChannel),parent,name), 
  init_done(false), defaultColor(1., 1., 1.), defaultAlpha(0.8), 
  idToWidget(), nextID(0), read_histogram(true), histogram_texture(0)
{
    activeW = 0;

    // Add the widgets to the colormap.
    SLIVR::ColorMap2 *c = new SLIVR::ColorMap2;
    cmap2.push_back(c);

    // Create a renderer to draw the widgets.
    ren = new WidgetRenderer(cmap2, 256*1000*1000);
}

QvisCMap2Display::~QvisCMap2Display()
{
    // Delete the cmap2 color maps...

    delete ren;
}

QSize
QvisCMap2Display::sizeHint() const
{
    return QSize(200,200);
}

QSizePolicy
QvisCMap2Display::sizePolicy() const
{
    QSizePolicy s(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding, 5, 5);
    return s;
}

void
QvisCMap2Display::paintGL()
{
    // Make a texture from the color map widgets.
    ren->make_colormap();

    if (histogram_texture == 0) {
      glGenTextures(1, &histogram_texture);

      // initialize texture to no data
      /* (not working for some reason)
      QPixmap pix(512,512);
      QColor bg(255,0,128);
      pix.fill(bg);
      QPainter painter(&pix);
      painter.drawText(10,10,tr("No histogram data"));
      QImage image = pix.convertToImage();
      QImage gl_image = QGLWidget::convertToGLFormat(image);
      glBindTexture(GL_TEXTURE_2D, histogram_texture);
      glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 512,
                   0, GL_RGBA, GL_UNSIGNED_BYTE, gl_image.bits());
      */
      read_histogram = true;
    }

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

    read_histogram = true;
    if (read_histogram) {
#if USE_HISTOGRAM
      FILE* file = fopen("/tmp/histogram.dump", "r");
      if (file == 0) {
        //        debug1 << "Error opening histogram file" << endl;


      }
      else {
        int hist_dim;

        // read in histogram size
        fread(&hist_dim, 1, sizeof(int), file);

        // read in histogram
        GLfloat hist[1024*1024];
        for (int i = 0; i < 1024*1024; i++)
          hist[i] = 0;
        fread(hist, 1, sizeof(GLfloat)*hist_dim*hist_dim, file);

        float count = 0;
        for (int i = 0; i < hist_dim*hist_dim; i++)
          if (hist[i] > 0)
            count += 1;
        count = count / (hist_dim * hist_dim);

        // read to a texture
        glBindTexture(GL_TEXTURE_2D, histogram_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, hist_dim, hist_dim,
                     0, GL_LUMINANCE, GL_FLOAT, hist);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        fclose(file);
      }
#endif
    }



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

    // Draw the widgets.
    ren->draw();

    glFlush();
    swapBuffers();
}

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

void
QvisCMap2Display::mousePressEvent(QMouseEvent *e)
{
    int doColor = (e->button() == MidButton) ? 1 : 0;

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
        if(w == it.data())
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
    newW->set_name(wName.latin1());
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

    WId thisID = nextID++;
    idToWidget[thisID] = newW;

    ren->set_colormap2(cmap2);
    updateGL();

    emit widgetListChanged();

    return thisID;
}

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

    newW->set_name(wName.latin1());
    newW->set_color(defaultColor);
    newW->set_faux(true);
    newW->set_alpha(defaultAlpha);
    newW->set_onState(1);
    newW->set_shadeType(SLIVR::CM2_SHADE_FALLOFF);

    // Append the new widget
    std::vector<SLIVR::CM2Widget*> &widgets = cmap2[0]->widgets();
    widgets.push_back(newW);

    WId thisID = nextID++;
    idToWidget[thisID] = newW;

    ren->set_colormap2(cmap2);
    updateGL();

    emit widgetListChanged();

    return thisID;
}

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

    newW->set_name(wName.latin1());
    newW->set_color(defaultColor);
    newW->set_faux(true);
    newW->set_alpha(defaultAlpha);
    newW->set_onState(1);
    newW->set_shadeType(SLIVR::CM2_SHADE_FALLOFF);

    // Append the new widget
    std::vector<SLIVR::CM2Widget*> &widgets = cmap2[0]->widgets();
    widgets.push_back(newW);

    WId thisID = nextID++;
    idToWidget[thisID] = newW;

    ren->set_colormap2(cmap2);
    updateGL();

    //    qDebug("%s", newW->tcl_pickle().c_str());

    emit widgetListChanged();

    return thisID;
}

WidgetID
QvisCMap2Display::addParaboloidWidget(const QString &wName,
                                      float top_x, float top_y,
                                      float bottom_x, float bottom_y,
                                      float left_x, float left_y,
                                      float right_x, float right_y)
{
    SLIVR::ParaboloidCM2Widget *newW = 0;
    newW = new SLIVR::ParaboloidCM2Widget(top_x, top_y, bottom_x, bottom_y, left_x, left_y, right_x, right_y);

    newW->set_name(wName.latin1());
    newW->set_color(defaultColor);
    newW->set_faux(true);
    newW->set_alpha(defaultAlpha);
    newW->set_onState(1);
    newW->set_shadeType(SLIVR::CM2_SHADE_FALLOFF);

    // Append the new widget
    std::vector<SLIVR::CM2Widget*> &widgets = cmap2[0]->widgets();
    widgets.push_back(newW);

    WId thisID = nextID++;
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
        it.data()->set_name(n.latin1());

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
        retval = QString(it.data()->get_name().c_str());
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
        it.data()->set_color(SLIVR::Color(r,g,b));

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
        int r = (int)(it.data()->get_color().r() * 255.f);
        int g = (int)(it.data()->get_color().g() * 255.f);
        int b = (int)(it.data()->get_color().b() * 255.f);
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
        it.data()->set_alpha(a);

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
        retval = it.data()->get_alpha();
    return retval;
}

void
QvisCMap2Display::setString(WidgetID id, const QString &s)
{
    IdToWidgetMap::iterator it =  idToWidget.find(id);
    if(it != idToWidget.end())
    {
        it.data()->tcl_unpickle(std::string(s.latin1()));

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
        retval = it.data()->tcl_pickle().c_str();
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
                if(*w == it.data())
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

WidgetID
QvisCMap2Display::getID(int index) const
{
    WidgetID retval = WIDGET_NOT_FOUND;
    QValueList<WidgetID> keys(idToWidget.keys());
    if(index >= 0 && index < keys.count())
        retval = keys[index];
    return retval;
}
