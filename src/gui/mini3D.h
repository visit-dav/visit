#ifndef _MINI3D_H_
#define _MINI3D_H_
#include <gui_exports.h>
#include <qcolor.h>

class QPainter;

/*----------------------------------------------------*/
/*                     primitives                     */
/*----------------------------------------------------*/

struct GUI_API vector3
{
    float x;
    float y;
    float z;
    float _dummy;  /* This is for some stupid 64-bit compilers */
};

struct GUI_API color
{
    float r;
    float g;
    float b;
    float _dummy;  /* This is for some stupid 64-bit compilers */
};

struct GUI_API matrix4
{
    float m[4][4];
};

/*----------------------------------------------------*/
/*                       lights                       */
/*----------------------------------------------------*/

typedef enum {
    M3D_LIGHT_AMB, /* ambient     */
    M3D_LIGHT_DIR, /* directional */
    M3D_LIGHT_EYE, /* eye-fixed   */
    M3D_LIGHT_OFF  /* off         */
} light_type;

struct GUI_API m3d_light
{
    light_type t;
    vector3    v;
    vector3    v_norm;
    color      c;
};

/*----------------------------------------------------*/
/*                       elements                     */
/*----------------------------------------------------*/

class m3d_renderer;

typedef enum {
    SOLID_LINE,
    DASH_LINE,
    DOT_LINE,
    DASH_DOT_LINE
} line_style;

// ****************************************************************************
// Class: m3d_element
//
// Purpose:
//   Base type for all rendered objects.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 08:38:19 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class GUI_API m3d_element
{
public:
    m3d_element();
    virtual ~m3d_element();

    virtual void transform(const matrix4 *M) { };
    virtual void addToRenderer(m3d_renderer &, unsigned char = 0) const = 0;
    virtual void draw(m3d_renderer &) = 0;
    virtual float calc_avg_z_sort_element() const = 0;
    virtual void light_element_reset() { };
    virtual bool notCulled(const vector3 &) const { return true; }
    virtual void light_element_amb(const m3d_light &) { };
    virtual void light_element_dir(const m3d_light &, const vector3 &, bool) { };
    virtual void light_element_eye(const m3d_light &, const vector3 &, bool) { };
    virtual bool intersects(int, int) const { return false;};

    bool wasDrawn() const;
    void setDrawn(bool);
    void setId(unsigned char id);
    unsigned char getId() const;

    static QColor get_pixel_color(const color *);
protected:
    QPainter *painter(m3d_renderer &) const;
    static void protectedAddElement(m3d_renderer &, m3d_element *,
                                    unsigned char);

    unsigned char flags;
};

// ****************************************************************************
// Class: m3d_line_c
//
// Purpose:
//   Represents a 3d line that can be rendered.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 08:38:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class GUI_API m3d_line_c : public m3d_element
{
public:
    m3d_line_c();
    m3d_line_c(vector3 p0, vector3 p1, color c, line_style s, int segments=1);
    virtual ~m3d_line_c();

    virtual void transform(const matrix4 *M);
    virtual void addToRenderer(m3d_renderer &, unsigned char = 0) const;
    virtual void draw(m3d_renderer &);
    virtual float calc_avg_z_sort_element() const;

protected:
    vector3    v[2];
    color      c;
    line_style s;
    int        segments;
};

// ****************************************************************************
// Class: m3d_tri_c
//
// Purpose:
//   Represents a 3d, colored triangle that can be rendered.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 08:39:06 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class GUI_API m3d_tri_c : public m3d_element
{
public:
    m3d_tri_c();
    m3d_tri_c(vector3 p0, vector3 p1, vector3 p2, color c);
    virtual ~m3d_tri_c();

    virtual void transform(const matrix4 *M);
    virtual void addToRenderer(m3d_renderer &, unsigned char = 0) const;
    virtual void draw(m3d_renderer &);
    virtual float calc_avg_z_sort_element() const;
    virtual bool intersects(int x, int y) const;

protected:
    vector3 v[3];
    color   c;
};

// ****************************************************************************
// Class: m3d_tri_n
//
// Purpose:
//   Represents a lit, 3d, colored triangle that can be rendered.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 08:39:36 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class GUI_API m3d_tri_n : public m3d_tri_c
{
public:
    m3d_tri_n();
    m3d_tri_n(vector3 p0, vector3 p1, vector3 p2, vector3 n1, color c);
    virtual ~m3d_tri_n();

    virtual void transform(const matrix4 *M);
    virtual void addToRenderer(m3d_renderer &, unsigned char = 0) const;
    virtual void draw(m3d_renderer &);
    virtual void light_element_reset();
    virtual bool notCulled(const vector3 &) const;
    virtual void light_element_amb(const m3d_light &);
    virtual void light_element_dir(const m3d_light &, const vector3 &, bool);
    virtual void light_element_eye(const m3d_light &, const vector3 &, bool);

protected:
    vector3 n;
    color   lit_color;
};

// ****************************************************************************
// Class: m3d_complex_element
//
// Purpose:
//   Represents a collection of more primitive elements.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 08:40:06 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class GUI_API m3d_complex_element : public m3d_element
{
public:
    m3d_complex_element();
    virtual ~m3d_complex_element();

    virtual void addToRenderer(m3d_renderer &, unsigned char = 0) const;
    virtual void draw(m3d_renderer &);
    virtual float calc_avg_z_sort_element() const;

    void set_initial_size(int s);
    void set_default_color(color c);
    void set_default_line_style(line_style s);
    void set_default_line_segments(int seg);

    void add_line_c(float, float, float,
                    float, float, float);
    void add_line_c(float, float, float,
                    float, float, float,
                    float, float, float);
    void add_line_c(vector3, vector3);
    void add_line_c(vector3, vector3, color);
    void add_line_c(vector3, vector3, line_style);
    void add_line_c(vector3, vector3, color, line_style, int=1);

    void add_tri_c (vector3, vector3, vector3);
    void add_tri_c (float, float, float,
                    float, float, float,
                    float, float, float);
    void add_tri_c (vector3, vector3, vector3, color);

    void add_tri_n (float, float, float,
                    float, float, float,
                    float, float, float,
                    float, float, float);
    void add_tri_n (vector3, vector3, vector3, vector3);
    void add_tri_n (vector3, vector3, vector3, vector3, color);

    void add_element(m3d_element *);

protected:
    bool          checkStorage();

    color         default_color;
    line_style    default_line_style;
    int           default_line_segments;

    int           initialSize;
    int           numElements;
    int           maxElements;
    m3d_element **elements;
};

// ****************************************************************************
// Class: m3d_renderer
//
// Purpose:
//   This class renders m3d_element objects using a QPainter.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:13:56 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class GUI_API m3d_renderer
{
    typedef struct
    {
        float        Z;
        m3d_element *e;
    } sort_element;

public:
    friend class m3d_element;

    m3d_renderer(int w, int h);
    ~m3d_renderer();

    void resize(int w, int h);

    void set_world_matrix(matrix4 m);
    void set_proj_matrix (matrix4 m);
    void set_view_matrix (matrix4 m);
    void set_view_reference_point(vector3 v);

    void set_background(float,float,float);
    void set_light(int n, light_type t, float,float,float, float,float,float);
    void set_specular_enabled(bool);
    void set_backface_culling(bool);
    bool get_backface_culling() const;
    vector3 transform_world_point(const vector3 &) const;

    void begin_scene(QPainter *p);
    void end_scene();

    // Look for the object at x, y.
    int probe(int x, int y) const;

protected:
    // protected prototypes
    void addElement(m3d_element *);
    void light_element_amb(m3d_element *e);
    void light_element_dir(m3d_element *e);
    void light_element_eye(m3d_element *e);
    bool checkStorage();

    static int sort_element_compare(const void *a, const void *b);

protected:
    vector3       vrp, cull_vrp;
    matrix4       world;
    matrix4       view;
    matrix4       proj;
    matrix4       device;

    color         bg;
    m3d_light     lights[8];
    bool          specularEnabled;
    bool          backfaceCulling;

    int           width, height;

    int           numElements;
    int           maxElements;
    sort_element *sort_elements;

    QPainter     *painter;
};


/*----------------------------* 
 *    math functions
 *----------------------------*/

GUI_API void transform_vertex(const matrix4 *M, vector3 *v);
GUI_API void transform_vector(const matrix4 *M, vector3 *v);

GUI_API matrix4 mtx_mult(matrix4 A, matrix4 B);
GUI_API matrix4 mtx_inverse(matrix4 M);
GUI_API vector3 mtx_transform_vertex(matrix4 M, vector3 v);
GUI_API vector3 mtx_transform_vector(matrix4 M, vector3 v);

GUI_API vector3 vec_create(float x, float y, float z);
GUI_API vector3 vec_scale(vector3 v, float s);
GUI_API float   vec_norm(vector3 v);
GUI_API vector3 vec_normalize(vector3 v);
GUI_API vector3 vec_negate(vector3 v);
GUI_API float   vec_dot(vector3 a,vector3 b);
GUI_API vector3 vec_cross(vector3 a,vector3 b);
GUI_API vector3 vec_sub(vector3 a,vector3 b);

/*----------------------------* 
 *    utility functions
 *----------------------------*/

GUI_API matrix4 m3du_create_identity_matrix(void);
GUI_API matrix4 m3du_create_zero_matrix(void);
GUI_API matrix4 m3du_create_view_matrix(const vector3 from, 
                                const vector3 at, 
                                const vector3 world_up);
GUI_API matrix4 m3du_create_proj_matrix(const float near_plane,
                                const float far_plane,
                                const float fov);
GUI_API matrix4 m3du_create_RBT_matrix(const vector3 from, 
                               const vector3 at, 
                               const vector3 world_up);
GUI_API matrix4 m3du_create_translation_matrix(const float tx,
                                       const float ty,
                                       const float tz);
GUI_API matrix4 m3du_create_scaling_matrix(const float sx,
                                   const float sy,
                                   const float sz);
GUI_API matrix4 m3du_create_trackball_matrix(float p1x,float p1y, float p2x,float p2y);

#endif
