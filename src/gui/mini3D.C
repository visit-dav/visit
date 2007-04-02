/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <mini3D.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <qpainter.h>
#include <qcolor.h>

#include <visitstream.h>

/*---------------------------------------------------------------------------*/

/* We want to work in a right-handed coordinate system */
#define RHCS

/*---------------------------------------------------------------------------*/

static void lubksb(matrix4 *a, int *indx, float *b);
static int  ludcmp(matrix4 *a, int *indx, float *d);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*                            Math Functions                                 */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Modifications:
 *
 */

vector3
vec_create(float x, float y, float z)
{
    vector3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

vector3
vec_scale(vector3 v, float s)
{
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

float
vec_norm(vector3 v)
{
    float n = (v.x*v.x + v.y*v.y + v.z*v.z);
    if (n>0)
        n = sqrt(n);
    return n;
}

vector3
vec_normalize(vector3 v)
{
    float n = (v.x*v.x + v.y*v.y + v.z*v.z);
    if (n>0)
    {
        n = 1./sqrt(n);
        v.x *= n;
        v.y *= n;
        v.z *= n;
    }
    return v;
}

vector3
vec_negate(vector3 v)
{
    v.x *= -1;
    v.y *= -1;
    v.z *= -1;
    return v;
}

float
vec_dot(vector3 a, vector3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

vector3
vec_sub(vector3 a, vector3 b)
{
    vector3 c;
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;
    return c;
}

vector3
vec_cross(vector3 a, vector3 b)
{
    vector3 c;
    c.x = a.y*b.z - a.z*b.y;
    c.y = a.z*b.x - a.x*b.z;
    c.z = a.x*b.y - a.y*b.x;
    return c;
}

vector3
mtx_transform_vertex(matrix4 M, vector3 v)
{
    transform_vertex(&M,&v);
    return v;
}

vector3
mtx_transform_vector(matrix4 M, vector3 v)
{
    transform_vector(&M,&v);
    return v;
}

matrix4
mtx_mult(matrix4 A, matrix4 B)
{
    int r,c;
    matrix4 C;

    for (r=0; r<4; r++)
        for (c=0; c<4; c++)
            C.m[r][c] = A.m[r][0] * B.m[0][c] +
                        A.m[r][1] * B.m[1][c] +
                        A.m[r][2] * B.m[2][c] +
                        A.m[r][3] * B.m[3][c];
    return C;
}

matrix4
mtx_inverse(matrix4 M) {
    matrix4 n, y;
    int            i, j, indx[4];
    float        d, col[4];

    n=M;
    if (ludcmp(&n, indx, &d)) {
        return m3du_create_identity_matrix();
    }

    for (j=0; j<4; j++) {
        for (i=0; i<4; i++) {
            col[i] = 0.0f;
        }
        col[j] = 1.0f;
        lubksb(&n, indx, col);
        for (i=0; i<4; i++) {
            y.m[i][j] = col[i];
        }
    }
    return y;
}

static void 
lubksb(matrix4 *a, int *indx, float *b)
{
    int        i, j, ii=-1, ip;
    float    sum;

    for (i=0; i<4; i++) {
        ip = indx[i];
        sum = b[ip];
        b[ip] = b[i];
        if (ii>=0) {
            for (j=ii; j<=i-1; j++) {
                sum -= a->m[i][j] * b[j];
            }
        } else if (sum != 0.0) {
            ii = i;
        }
        b[i] = sum;
    }
    for (i=3; i>=0; i--) {
        sum = b[i];
        for (j=i+1; j<4; j++) {
            sum -= a->m[i][ j] * b[j];
        }
        b[i] = sum/a->m[i][ i];
    }
}

static int
ludcmp(matrix4 *a, int *indx, float *d)
{
    float    vv[4];               /* implicit scale for each row */
    float    big, dum, sum, tmp;
    int      i, j, k, imax = 0;

    *d = 1.0f;
    for (i=0; i<4; i++) {
        big = 0.0f;
        for (j=0; j<4; j++) {
            if ((tmp = (float) fabs(a->m[i][ j])) > big) {
                big = tmp;
            }
        }

        if (big == 0.0f) {
            return 1;
        }
        
        vv[i] = 1.0f/big;
    }
    for (j=0; j<4; j++) {
        for (i=0; i<j; i++) {
            sum = a->m[i][ j];
            for (k=0; k<i; k++) {
                sum -= a->m[i][ k] * a->m[k][ j];
            }
            a->m[i][ j] = sum;
        }
        big = 0.0f;
        for (i=j; i<4; i++) {
            sum = a->m[i][ j];
            for (k=0; k<j; k++) {
                sum -= a->m[i][ k]*a->m[k][ j];
            }
            a->m[i][ j] = sum;
            if ((dum = vv[i] * (float)fabs(sum)) >= big) {
                big = dum;
                imax = i;
            }
        }
        if (j != imax) {
            for (k=0; k<4; k++) {
                dum = a->m[imax][ k];
                a->m[imax][ k] = a->m[j][ k];
                a->m[j][ k] = dum;
            }
            *d = -(*d);
            vv[imax] = vv[j];
        }
        indx[j] = imax;
        if (a->m[j][ j] == 0.0f) {
            a->m[j][ j] = 1.0e-20f;      /* can be 0.0 also... */
        }
        if (j != 3) {
            dum = 1.0f/a->m[j][ j];
            for (i=j+1; i<4; i++) {
                a->m[i][ j] *= dum;
            }
        }
    }
    return 0;
}

// ****************************************************************************
// Method: m3d_element::m3d_element
//
// Purpose: 
//   Constructor for the m3d_element class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:26:57 PST 2003
//
// Modifications:
//   
// ****************************************************************************

m3d_element::m3d_element()
{
    flags = 0;
}

// ****************************************************************************
// Method: m3d_element::~m3d_element
//
// Purpose: 
//   Destructor for the m3d_element class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:26:57 PST 2003
//
// Modifications:
//   
// ****************************************************************************

m3d_element::~m3d_element()
{
}

bool
m3d_element::wasDrawn() const
{
    return (flags & 0x80) != 0;
}

void
m3d_element::setDrawn(bool val)
{
    if(val)
        flags = flags | 0x80;
    else
        flags = flags & 0x7f;
}

void
m3d_element::setId(unsigned char id)
{
    flags = id & 0x7f;
}

unsigned char
m3d_element::getId() const
{
    return flags & 0x7f;
}

// ****************************************************************************
// Method: m3d_element::get_pixel_color
//
// Purpose: 
//   Turns a color into a QColor.
//
// Arguments:
//   c : The color to convert.
//
// Returns:    A QColor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 08:11:19 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Mar 4 08:10:47 PDT 2003
//   I made it a member of the new m3d_element class.
//
// ****************************************************************************

QColor
m3d_element::get_pixel_color(const color *c)
{
    QColor retval(int(QMAX(QMIN(c->r,1),0) * 255),
                  int(QMAX(QMIN(c->g,1),0) * 255),
                  int(QMAX(QMIN(c->b,1),0) * 255));
    return retval;
}

QPainter *
m3d_element::painter(m3d_renderer &renderer) const
{
    return renderer.painter;
}

// ****************************************************************************
// Method: m3d_element::protectedAddElement
//
// Purpose: 
//   Adds an element to a renderer using a protected renderer addElement method.
//
// Arguments:
//   renderer : The renderer to which we'll add the element.
//   e        : The element to add.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:27:27 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_element::protectedAddElement(m3d_renderer &renderer, m3d_element *e,
    unsigned char id)
{
    e->setId(id);
    renderer.addElement(e);
}

// ****************************************************************************
// Method: m3d_line_c::m3d_line_c
//
// Purpose: 
//   Constructor for the m3d_line class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 08:25:43 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

m3d_line_c::m3d_line_c() : m3d_element()
{
    v[0].x = 0.;
    v[0].y = 0.;
    v[0].z = 0.;
    v[1].x = 0.;
    v[1].y = 0.;
    v[1].z = 0.;
    c.r = 0.;
    c.g = 0.;
    c.b = 0.;
    s = SOLID_LINE;
    segments = 1;
}

m3d_line_c::m3d_line_c(vector3 p0, vector3 p1, color c1, line_style s1,
    int seg) : m3d_element()
{
    v[0].x = p0.x;
    v[0].y = p0.y;
    v[0].z = p0.z;
    v[1].x = p1.x;
    v[1].y = p1.y;
    v[1].z = p1.z;
    c.r = c1.r;
    c.g = c1.g;
    c.b = c1.b;
    s = s1;
    segments = seg;
}

// ****************************************************************************
// Method: m3d_line_c::~m3d_line_c
//
// Purpose: 
//   Destructor for the m3d_line_c class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:28:31 PST 2003
//
// Modifications:
//   
// ****************************************************************************

m3d_line_c::~m3d_line_c()
{
}

// ****************************************************************************
// Method: m3d_line_c::transform
//
// Purpose: 
//   Transforms the line using the specified matrix.
//
// Arguments:
//   M : The transformation matrix.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:28:52 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_line_c::transform(const matrix4 *M)
{
    transform_vertex(M, &v[0]);
    transform_vertex(M, &v[1]);
}

// ****************************************************************************
// Method: m3d_line_c::addToRenderer
//
// Purpose: 
//   Adds the line to the specified renderer.
//
// Arguments:
//   renderer : The renderer to which we'll add the line.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:29:23 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Dec 13 12:04:08 PDT 2004
//   Added a version that accepts a color so the color can be overridden.
//
// ****************************************************************************

void
m3d_line_c::addToRenderer(m3d_renderer &renderer, unsigned char id) const
{
    addToRenderer(renderer, c, id);
}

void
m3d_line_c::addToRenderer(m3d_renderer &renderer, color C,
    unsigned char id) const
{
    if(segments == 1)
    {
        m3d_line_c *copy = new m3d_line_c(v[0], v[1], C, s, 1);
        protectedAddElement(renderer, copy, id);
    }
    else
    {
        vector3 p0, p1;
        for(int i = 0; i < segments; ++i)
        {
            float t0 = float(i) / float(segments);
            float t1 = float(i + 1) / float(segments);

            p0.x = (1. - t0) * v[0].x + t0 * v[1].x;
            p0.y = (1. - t0) * v[0].y + t0 * v[1].y;
            p0.z = (1. - t0) * v[0].z + t0 * v[1].z;
            p1.x = (1. - t1) * v[0].x + t1 * v[1].x;
            p1.y = (1. - t1) * v[0].y + t1 * v[1].y;
            p1.z = (1. - t1) * v[0].z + t1 * v[1].z;

            m3d_line_c *copy = new m3d_line_c(p0, p1, C, s, 1);
            protectedAddElement(renderer, copy, id);
        }
    }
}

// ****************************************************************************
// Method: m3d_line_c::draw
//
// Purpose: 
//   Draws the line using the painter.
//
// Arguments:
//   p : The painter to use to draw the line.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:29:51 PST 2003
//
// Modifications:
//   
//   Hank Childs, Thu Jun  8 14:14:58 PDT 2006
//   Fix compiler warning for casting.
//
// ****************************************************************************

void
m3d_line_c::draw(m3d_renderer &renderer)
{
    QPen pen(get_pixel_color(&c));
    if(s == SOLID_LINE)
       pen.setStyle(Qt::SolidLine);
    else if(s == DASH_LINE)
       pen.setStyle(Qt::DashLine);
    else if(s == DOT_LINE)
       pen.setStyle(Qt::DotLine);
    else if(s == DASH_DOT_LINE)
       pen.setStyle(Qt::DashDotLine);
    QPainter *p = painter(renderer);
    p->setPen(pen);
    p->drawLine((int) (v[0].x), (int) (v[0].y), (int) (v[1].x),(int) (v[1].y));
    setDrawn(true);
}

// ****************************************************************************
// Method: m3d_line_c::calc_avg_z_sort_element
//
// Purpose: 
//   Calculates the average Z for the line.
//
// Returns:    The average Z value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:30:19 PST 2003
//
// Modifications:
//   
// ****************************************************************************

float
m3d_line_c::calc_avg_z_sort_element() const
{
    return (v[0].z + v[1].z) / 2.;
}


// ****************************************************************************
// Class: m3d_tri_c::m3d_tri_c
//
// Purpose:
//   Constructor for the m3d_tri_c class.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 08:37:32 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

m3d_tri_c::m3d_tri_c() : m3d_element()
{
    v[0].x = 0.;
    v[0].y = 0.;
    v[0].z = 0.;
    v[1].x = 0.;
    v[1].y = 0.;
    v[1].z = 0.;
    v[2].x = 0.;
    v[2].y = 0.;
    v[2].z = 0.;
    c.r = 0.;
    c.g = 0.;
    c.b = 0.;
}

m3d_tri_c::m3d_tri_c(vector3 p0, vector3 p1, vector3 p2, color c1) : m3d_element()
{
    v[0].x = p0.x;
    v[0].y = p0.y;
    v[0].z = p0.z;
    v[1].x = p1.x;
    v[1].y = p1.y;
    v[1].z = p1.z;
    v[2].x = p2.x;
    v[2].y = p2.y;
    v[2].z = p2.z;
    c.r = c1.r;
    c.g = c1.g;
    c.b = c1.b;
}

// ****************************************************************************
// Method: m3d_tri_c::~m3d_tri_c
//
// Purpose: 
//   The destructor for the m3d_tri_c class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:30:47 PST 2003
//
// Modifications:
//   
// ****************************************************************************

m3d_tri_c::~m3d_tri_c()
{
}

// ****************************************************************************
// Method: m3d_tri_c::transform
//
// Purpose: 
//   Transforms the triangle using the specified matrix.
//
// Arguments:
//   M : The transformation matrix.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:31:11 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_tri_c::transform(const matrix4 *M)
{
    transform_vertex(M, &v[0]);
    transform_vertex(M, &v[1]);
    transform_vertex(M, &v[2]);
}

// ****************************************************************************
// Method: m3d_tri_c::addToRenderer
//
// Purpose: 
//   Adds the triangle to the renderer.
//
// Arguments:
//   renderer : The renderer to which we're adding the triangle.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:31:39 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Dec 13 12:04:08 PDT 2004
//   Added a version that accepts a color so the color can be overridden.
//   
// ****************************************************************************

void
m3d_tri_c::addToRenderer(m3d_renderer &renderer, unsigned char id) const
{
    addToRenderer(renderer, c, id);
}

void
m3d_tri_c::addToRenderer(m3d_renderer &renderer, color C,
    unsigned char id) const
{
    m3d_tri_c *copy = new m3d_tri_c(v[0], v[1], v[2], C);
    protectedAddElement(renderer, copy, id);
}

// ****************************************************************************
// Method: m3d_tri_c::draw
//
// Purpose: 
//   Called by the renderer to draw the triangle using the specified painter.
//
// Arguments:
//   p : The painter to use to draw the triangle.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:32:19 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Aug 22 08:57:56 PDT 2003
//   Forced it to have a pen color so it draws correctly on MacOS X.
//
//   Hank Childs, Thu Jun  8 14:14:58 PDT 2006
//   Fix compiler warning for casting.
//
// ****************************************************************************

void
m3d_tri_c::draw(m3d_renderer &renderer)
{
    QPointArray tri(3);

    tri.setPoint(0, (int) v[0].x, (int) v[0].y);
    tri.setPoint(1, (int) v[1].x, (int) v[1].y);
    tri.setPoint(2, (int) v[2].x, (int) v[2].y);

    QColor triColor(get_pixel_color(&c));
    QBrush b(triColor);
    b.setStyle(QBrush::SolidPattern);
    QPainter *p = painter(renderer);
    p->setBrush(b);
    p->setPen(QPen(triColor));
    p->drawPolygon(tri, true);
    setDrawn(true);
}

// ****************************************************************************
// Method: m3d_tri_c::calc_avg_z_sort_element
//
// Purpose: 
//   Returns the average Z value for the triangle.
//
// Returns:    The average Z value for the triangle.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:33:10 PST 2003
//
// Modifications:
//   
// ****************************************************************************

float
m3d_tri_c::calc_avg_z_sort_element() const
{
    return (v[0].z + v[1].z + v[2].z) / 3.;
}

// ****************************************************************************
// Method: m3d_tri_c::intersects
//
// Purpose: 
//   Returns whether or not the point intersects the triangle.
//
// Arguments:
//   x : The x location of the point we're checking.
//   y : The y location of the point we're checking.
//
// Returns:    True if the point is inside the triangle.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 5 12:52:00 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
m3d_tri_c::intersects(int x, int y) const
{
#define fAB ((y-v[0].y)*(v[1].x-v[0].x) - (x-v[0].x)*(v[1].y-v[0].y))
#define fCA ((y-v[2].y)*(v[0].x-v[2].x) - (x-v[2].x)*(v[0].y-v[2].y))
#define fBC ((y-v[1].y)*(v[2].x-v[1].x) - (x-v[1].x)*(v[2].y-v[1].y))
  
    float f1 = fAB * fBC;
    float f2 = fBC * fCA;
    return (f1 > 0. && f2 > 0.);
}

// ****************************************************************************
// Method: m3d_tri_n::m3d_tri_n
//
// Purpose: 
//   Constructor for the m3d_tri_n class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 08:45:39 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

m3d_tri_n::m3d_tri_n() : m3d_tri_c()
{
    n.x = 0.;
    n.y = 1.;
    n.z = 0.;
    lit_color.r = 0.;
    lit_color.g = 0.;
    lit_color.b = 0.;
}

m3d_tri_n::m3d_tri_n(vector3 p0, vector3 p1, vector3 p2, vector3 n1, color c1) :
    m3d_tri_c(p0, p1, p2, c1)
{
    n = vec_normalize(n1);
    lit_color.r = 0.;
    lit_color.g = 0.;
    lit_color.b = 0.;
}

// ****************************************************************************
// Method: m3d_tri_n::~m3d_tri_n
//
// Purpose: 
//   Destructor for the m3d_tri_n class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:33:45 PST 2003
//
// Modifications:
//   
// ****************************************************************************

m3d_tri_n::~m3d_tri_n()
{
}

// ****************************************************************************
// Method: m3d_tri_n::transform
//
// Purpose: 
//   Transforms the triangle and its normal.
//
// Arguments:
//   M : The transformation matrix.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:34:04 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Dec 13 12:59:54 PDT 2004
//   Added code to normalize the vector in case the world transform scaled
//   the geometry.
//
// ****************************************************************************

void
m3d_tri_n::transform(const matrix4 *M)
{
    transform_vertex(M, &v[0]);
    transform_vertex(M, &v[1]);
    transform_vertex(M, &v[2]);
    transform_vector(M, &n);
    n = vec_normalize(n);
}

// ****************************************************************************
// Method: m3d_tri_n::addToRenderer
//
// Purpose: 
//   Adds the triangle to the renderer.
//
// Arguments:
//   renderer : The renderer to which we're adding the triangle.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:34:38 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Dec 13 12:04:08 PDT 2004
//   Added a version that accepts a color so the color can be overridden.
//   
// ****************************************************************************

void
m3d_tri_n::addToRenderer(m3d_renderer &renderer, unsigned char id) const
{
    addToRenderer(renderer, c, id);
}

void
m3d_tri_n::addToRenderer(m3d_renderer &renderer, color C,
    unsigned char id) const
{
    m3d_tri_n *copy = new m3d_tri_n(v[0], v[1], v[2], n, C);
    protectedAddElement(renderer, copy, id);
}

// ****************************************************************************
// Method: m3d_tri_n::draw
//
// Purpose: 
//   Renders the triangle.
//
// Arguments:
//   p : The painter to use to draw the triangle.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:35:10 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Aug 22 08:59:04 PDT 2003
//   I made it have a pen color so it draws correctly on MacOS X.
//
//   Hank Childs, Thu Jun  8 14:14:58 PDT 2006
//   Fix compiler warning for casting.
//
// ****************************************************************************

void
m3d_tri_n::draw(m3d_renderer &renderer)
{
    QPointArray tri(3);

    tri.setPoint(0, (int) v[0].x, (int) v[0].y);
    tri.setPoint(1, (int) v[1].x, (int) v[1].y);
    tri.setPoint(2, (int) v[2].x, (int) v[2].y);

    QColor triColor(get_pixel_color(&lit_color));
    QBrush b(triColor);
    b.setStyle(QBrush::SolidPattern);
    QPainter *p = painter(renderer);
    p->setBrush(b);
    p->setPen(QPen(triColor));
    p->drawPolygon(tri, true);
    setDrawn(true);
}

// ****************************************************************************
// Method: m3d_tri_n::light_element_reset
//
// Purpose: 
//   Resets the lighting color to black.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:35:41 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_tri_n::light_element_reset()
{
    lit_color.r = 0.;
    lit_color.g = 0.;
    lit_color.b = 0.;
}

// ****************************************************************************
// Method: m3d_tri_n::notCulled
//
// Purpose: 
//   Returns whether the element should not be culled.
//
// Arguments:
//   cull_vrp : The transformed vrp.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 14:24:05 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
m3d_tri_n::notCulled(const vector3 &cull_vrp) const
{
    float dot = (cull_vrp.x * n.x + 
                 cull_vrp.y * n.y +
                 cull_vrp.z * n.z);
    return (dot > 0.f);
}

// ****************************************************************************
// Method: m3d_tri_n::light_element_amb
//
// Purpose: 
//   Calculates the ambient light contribution.
//
// Arguments:
//   l : The light.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:36:01 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_tri_n::light_element_amb(const m3d_light &l)
{
    lit_color.r += (c.r * l.c.r);
    lit_color.g += (c.g * l.c.g);
    lit_color.b += (c.b * l.c.b);
}

// ****************************************************************************
// Method: m3d_tri_n::light_element_dir
//
// Purpose: 
//   Calculates the directional light contribution.
//
// Arguments:
//   l : The light.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:36:01 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_tri_n::light_element_dir(const m3d_light &l, const vector3 &vrp, bool specular)
{
    float LN = -(n.x * l.v_norm.x +
                 n.y * l.v_norm.y +
                 n.z * l.v_norm.z);

    if (LN > 0.)
    {
        if(specular)
        {
#define ONE_OVER_THREE 0.3333333f;
            // The center of the triangle.
            vector3 location;
            location.x = (v[0].x + v[1].x + v[2].x) * ONE_OVER_THREE;
            location.y = (v[0].y + v[1].y + v[2].y) * ONE_OVER_THREE;
            location.z = (v[0].z + v[1].z + v[2].z) * ONE_OVER_THREE;

            // The vector from the view to C
            vector3 V;
            V.x = vrp.x - location.x;
            V.y = vrp.y - location.y;
            V.z = vrp.z - location.z;
            V = vec_normalize(V);

            // H = (L + V) / 2
            vector3 H;
            H.x = (-l.v.x + V.x) * 0.5f;
            H.y = (-l.v.y + V.y) * 0.5f;
            H.z = (-l.v.z + V.z) * 0.5f;
            H = vec_normalize(H);

            // NH = N * H
           float NH = n.x * H.x +
                      n.y * H.y +
                      n.z * H.z;

           if(NH > 0.f)
           {
               float NH2 = NH * NH;
               float specular_exp = NH2*NH2;

               // diffuse + specular
               lit_color.r += (l.c.r * (c.r * LN + specular_exp));
               lit_color.g += (l.c.g * (c.g * LN + specular_exp));
               lit_color.b += (l.c.b * (c.b * LN + specular_exp));
           }
        }
        else
        {
            // diffuse only
            lit_color.r += (l.c.r * c.r * LN);
            lit_color.g += (l.c.g * c.g * LN);
            lit_color.b += (l.c.b * c.b * LN);
        }

        if(lit_color.r > 1.f) lit_color.r = 1.f;
        if(lit_color.g > 1.f) lit_color.g = 1.f;
        if(lit_color.b > 1.f) lit_color.b = 1.f;
    }
}

// ****************************************************************************
// Method: m3d_tri_n::light_element_eye
//
// Purpose: 
//   Calculates the eye light contribution.
//
// Arguments:
//   l : The light.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:36:01 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_tri_n::light_element_eye(const m3d_light &L, const vector3 &vrp, bool specular)
{
    light_element_dir(L, vrp, specular);
}


// ****************************************************************************
// Method: m3d_complex_element::m3d_complex_element
//
// Purpose: 
//   Constructor for the m3d_complex_element class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 09:38:20 PDT 2003
//
// Modifications:
//
// ****************************************************************************

m3d_complex_element::m3d_complex_element()
{
    default_color.r = 1.;
    default_color.g = 1.;
    default_color.b = 1.;
    default_line_style = SOLID_LINE;
    default_line_segments = 1;

    numElements = 0;
    maxElements = 0;
    elements = 0;
    initialSize = 50;
}

// ****************************************************************************
// Method: m3d_complex_element::~m3d_complex_element
//
// Purpose: 
//   Destructor for the m3d_complex_element class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:37:02 PST 2003
//
// Modifications:
//   
// ****************************************************************************

m3d_complex_element::~m3d_complex_element()
{
    for(int i = 0; i < numElements; ++i)
        delete elements[i];
    delete [] elements;
}

// ****************************************************************************
// Method: m3d_complex_element::addToRenderer
//
// Purpose: 
//   Adds the elements to the renderer.
//
// Arguments:
//   renderer : The renderer to which we'll add the elements.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:37:59 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Dec 13 12:04:08 PDT 2004
//   Added a version that accepts a color so the color can be overridden.
//   
// ****************************************************************************

void
m3d_complex_element::addToRenderer(m3d_renderer &renderer, unsigned char id) const
{
    for(int i = 0; i < numElements; ++i)
        elements[i]->addToRenderer(renderer, id);
}

void
m3d_complex_element::addToRenderer(m3d_renderer &renderer, color C, unsigned char id) const
{
    for(int i = 0; i < numElements; ++i)
        elements[i]->addToRenderer(renderer, C, id);
}

// ****************************************************************************
// Method: m3d_complex_element::draw
//
// Purpose: 
//   Draws the complex element.
//
// Note:       This method should never be called because complex elements are
//             added to the renderer as smaller, simpler elements.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:38:40 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_complex_element::draw(m3d_renderer &)
{
}

// ****************************************************************************
// Method: m3d_complex_element::calc_avg_z_sort_element
//
// Purpose: 
//   Returns the average X value for the element.
//
// Note:       This method should never be called because complex elements are
//             added to the renderer as smaller, simpler elements.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:38:40 PST 2003
//
// Modifications:
//   
// ****************************************************************************

float
m3d_complex_element::calc_avg_z_sort_element() const
{
    return 0.;
}

// ****************************************************************************
// Method: m3d_complex_element::set_initial_size
//
// Purpose: 
//   Sets the initial number of child elements.
//
// Arguments:
//   s : The new number of child elements.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:39:47 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_complex_element::set_initial_size(int s)
{
    initialSize = s;
}

// ****************************************************************************
// Method: m3d_complex_element::set_default_color
//
// Purpose: 
//   Sets the default color used by add routines that don't take color arguments.
//
// Arguments:
//   c : the new default color.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:40:28 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
m3d_complex_element::set_default_color(color c)
{
    default_color = c;
}

// ****************************************************************************
// Method: m3d_complex_element::set_default_line_style
//
// Purpose: 
//   Sets the default line style.
//
// Arguments:
//   s : The new default line style.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:41:00 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_complex_element::set_default_line_style(line_style s)
{
    default_line_style = s;
}

// ****************************************************************************
// Method: m3d_complex_element::set_default_line_segments
//
// Purpose: 
//   Sets the number of line segments that make up the line.
//
// Arguments:
//   seg : The new default number of segments.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 14:04:24 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_complex_element::set_default_line_segments(int seg)
{
    default_line_segments = seg;
}

// ****************************************************************************
// Method: m3d_complex_element::add_line_c
//
// Purpose: 
//   Adds a new line.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:41:27 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_complex_element::add_line_c(
    float p0x, float p0y, float p0z,
    float p1x, float p1y, float p1z)
{
    vector3 p0 = {p0x, p0y, p0z, 1.};
    vector3 p1 = {p1x, p1y, p1z, 1.};
    add_line_c(p0, p1, default_color, default_line_style, default_line_segments);
}

void
m3d_complex_element::add_line_c(
    float p0x, float p0y, float p0z,
    float p1x, float p1y, float p1z,
    float r, float g, float b)
{
    vector3 p0 = {p0x, p0y, p0z, 1.};
    vector3 p1 = {p1x, p1y, p1z, 1.};
    color    c =  {r, g, b, 1.};
    add_line_c(p0, p1, c, default_line_style, default_line_segments);
}

void
m3d_complex_element::add_line_c(vector3 p0, vector3 p1)
{
    add_line_c(p0, p1, default_color, default_line_style, default_line_segments);
}

void
m3d_complex_element::add_line_c(vector3 p0, vector3 p1, color c)
{
    add_line_c(p0, p1, c, default_line_style, default_line_segments);
}

void
m3d_complex_element::add_line_c(vector3 p0, vector3 p1, line_style s)
{
    add_line_c(p0, p1, default_color, s, default_line_segments);
}

void
m3d_complex_element::add_line_c(vector3 p0, vector3 p1, color c, line_style s, int seg)
{
    if(checkStorage())
        elements[numElements++] = new m3d_line_c(p0, p1, c, s, seg);
}

// ****************************************************************************
// Method: m3d_complex_element::add_tri_c
//
// Purpose: 
//   Adds a new non-shaded triangle.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:41:49 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_complex_element::add_tri_c (vector3 p0, vector3 p1, vector3 p2)
{
     add_tri_c(p0, p1, p2, default_color);
}

void
m3d_complex_element::add_tri_c(
    float p0x, float p0y, float p0z,
    float p1x, float p1y, float p1z,
    float p2x, float p2y, float p2z)
{
    vector3 p0 = {p0x, p0y, p0z, 1.};
    vector3 p1 = {p1x, p1y, p1z, 1.};
    vector3 p2 = {p2x, p2y, p2z, 1.};
    add_tri_c(p0, p1, p2, default_color);
}

void
m3d_complex_element::add_tri_c (vector3 p0, vector3 p1, vector3 p2, color c)
{
    if(checkStorage())
        elements[numElements++] = new m3d_tri_c(p0, p1, p2, c);
}

// ****************************************************************************
// Method: m3d_complex_element::add_tri_n
//
// Purpose: 
//   Adds a new lit triangle.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:42:11 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_complex_element::add_tri_n(
    float p0x, float p0y, float p0z,
    float p1x, float p1y, float p1z,
    float p2x, float p2y, float p2z,
    float nx, float ny, float nz)
{
    vector3 p0 = {p0x, p0y, p0z, 1.};
    vector3 p1 = {p1x, p1y, p1z, 1.};
    vector3 p2 = {p2x, p2y, p2z, 1.};
    vector3 n = {nx, ny, nz, 1.};

    add_tri_n(p0, p1, p2, n, default_color);
}

void
m3d_complex_element::add_tri_n (vector3 p0, vector3 p1, vector3 p2, vector3 n)
{
    add_tri_n(p0, p1, p2, n, default_color);
}

void
m3d_complex_element::add_tri_n (vector3 p0, vector3 p1, vector3 p2, vector3 n, color c)
{
    if(checkStorage())
        elements[numElements++] = new m3d_tri_n(p0, p1, p2, n, c);
}

void
m3d_complex_element::add_element(m3d_element *e)
{
    if(checkStorage())
        elements[numElements++] = e;
}

// ****************************************************************************
// Method: m3d_complex_element::checkStorage
//
// Purpose: 
//   Resizes the storage arrays for the complex element.
//
// Returns:    True if we can add another element.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:42:32 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
m3d_complex_element::checkStorage()
{
    if(maxElements == 0)
    {
        // Initial allocation
        maxElements = initialSize;
        numElements = 0;
        elements = new m3d_element *[maxElements];
    }
    else if(numElements == maxElements)
    {
        // Resize allocation.
        m3d_element **newElements = new m3d_element *[maxElements*2];
        if(newElements)
        {
            for(int i = 0; i < maxElements; ++i)
                newElements[i] = elements[i];
        }
        delete [] elements;
        elements = newElements;
        maxElements *= 2;
    }

    return (elements != 0);
}


// ****************************************************************************
// Method: m3d_renderer::m3d_renderer
//
// Purpose: 
//   Constructor for the m3d_renderer class.
//
// Arguments:
//   x : the width of the renderer.
//   y : The height of the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 3 13:45:27 PST 2003
//
// Modifications:
//   
// ****************************************************************************

m3d_renderer::m3d_renderer(int w, int h)
{
    world  = m3du_create_identity_matrix();
    view   = m3du_create_identity_matrix();
    proj   = m3du_create_identity_matrix();
    device = m3du_create_identity_matrix();

    bg.r = 0.;
    bg.g = 0.;
    bg.b = 0.;

    vrp = vec_create(0,0,0);
    cull_vrp = vec_create(0,0,0);

    for(int i = 0; i < 8; ++i)
    {
        lights[i].t = M3D_LIGHT_OFF;
        lights[i].v.x = 0.;
        lights[i].v.y = 0.;
        lights[i].v.z = 0.;
        lights[i].v_norm.x = 0.;
        lights[i].v_norm.y = 0.;
        lights[i].v_norm.z = 0.;
        lights[i].c.r = 1.;
        lights[i].c.g = 1.;
        lights[i].c.b = 1.;
    }

    specularEnabled = false;
    backfaceCulling = false;
    width = w;
    height = h;

    device.m[0][0] = (float)QMIN(width,height)/2.;
    device.m[1][1] =-(float)QMIN(width,height)/2.;
    device.m[0][3] = (float)width/2.;
    device.m[1][3] = (float)height/2.;

    numElements = 0;
    maxElements = 0;
    sort_elements = 0;

    painter = 0;
}

// ****************************************************************************
// Method: m3d_renderer::~m3d_renderer
//
// Purpose: 
//   Destructor for the m3d_renderer class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 3 13:45:06 PST 2003
//
// Modifications:
//   
// ****************************************************************************

m3d_renderer::~m3d_renderer()
{
    if(sort_elements != 0)
    {
        for(int i = 0; i < numElements; ++i)
            delete sort_elements[i].e;
        delete [] sort_elements;
    }
}

/*---------------------------------------------------------------------------*
 * Function  : m3d_renderer::resize()
 *
 * Purpose   : Resize the M3D framebuffer.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs : w,h (int): the new width and height of the framebuffer
 *
 * Modifications:
 *  Brad Whitlock, Tue Mar 4 13:11:38 PST 2003
 *  I rewrote it.
 *
 *---------------------------------------------------------------------------*/

void
m3d_renderer::resize(int w, int h)
{
    width = w;
    height = h;

    device.m[0][0] = (float)QMIN(width,height)/2.;
    device.m[1][1] =-(float)QMIN(width,height)/2.;
    device.m[0][3] = (float)width/2.;
    device.m[1][3] = (float)height/2.;
}

/*---------------------------------------------------------------------------*
 * Function  : m3d_renderer::set_world_matrix()
 *
 * Purpose   : Set the matrix used for model->world transforms.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs : m (matrix4): the new matrix
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------*/

void
m3d_renderer::set_world_matrix(matrix4 m)
{
    world = m;
}

/*---------------------------------------------------------------------------*
 * Function  : m3d_renderer::set_view_matrix()
 *
 * Purpose   : Set the matrix used for world->view transforms.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs : m (matrix4): the new matrix
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------*/

void
m3d_renderer::set_view_matrix(matrix4 m)
{
    view = m;
}

/*---------------------------------------------------------------------------*
 * Function  : m3d_renderer::set_proj_matrix()
 *
 * Purpose   : Set the matrix used for view->projection transforms.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs : m (matrix4): the new matrix
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------*/

void
m3d_renderer::set_proj_matrix(matrix4 m)
{
    proj = m;
}

void
m3d_renderer::set_view_reference_point(vector3 v)
{
    vrp = v;

    cull_vrp = v;
    transform_vector(&view, &cull_vrp);
    cull_vrp = vec_normalize(cull_vrp);
}

void
m3d_renderer::set_backface_culling(bool v)
{
    backfaceCulling = v;
}

bool
m3d_renderer::get_backface_culling() const
{
    return backfaceCulling;
}

// ****************************************************************************
// Method: m3d_renderer::transform_world_point
//
// Purpose: 
//   Transforms a world point to screen coordinates.
//
// Arguments:
//   p : The point.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 15:11:45 PST 2003
//
// Modifications:
//   
// ****************************************************************************

vector3
m3d_renderer::transform_world_point(const vector3 &p) const
{
    vector3 p2 = p;
    transform_vertex(&view, &p2);
    transform_vertex(&proj, &p2);
    transform_vertex(&device, &p2);

    return p2;
}

/*---------------------------------------------------------------------------*
 * Function  : m3d_renderer::set_background()
 *
 * Purpose   : Set the new background color.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs : r,g,b (float): the new bg color in red,green,blue form
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------*/

void
m3d_renderer::set_background(float r, float g, float b)
{
    bg.r = r;
    bg.g = g;
    bg.b = b;
}

// ****************************************************************************
// Method: m3d_renderer::set_specular_enabled
//
// Purpose: 
//   Sets the renderer's specular flag.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 5 09:42:47 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_renderer::set_specular_enabled(bool v)
{
    specularEnabled = v;
}

/*---------------------------------------------------------------------------*
 * Function  : m3d_renderer::set_light()
 *
 * Purpose   : Set the attributes of a light source.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs : n  (int): the light source number (1..8)
 *          t  (int): the light type (one of enum light_type)
 *          x,y,z (float): the direction of the light (when applicable)
 *          r,g,b (float): the color of the light
 *
 * Modifications:
 *   Brad Whitlock, Tue Mar 4 13:10:28 PST 2003
 *   Changed the second argument to light_type.
 *
 *---------------------------------------------------------------------------*/

void
m3d_renderer::set_light(int n, light_type t,
              float x, float y, float z,
              float r, float g, float b)
{
    if (n<1 || n>8)
    {
        fprintf(stderr,"m3d_set_light() called on invalid light\n");
        return;
    }
    
    lights[n-1].t   = (light_type)t;

    lights[n-1].v.x = x;
    lights[n-1].v.y = y;
    lights[n-1].v.z = z;
    lights[n-1].v_norm = vec_normalize(lights[n-1].v);
    lights[n-1].c.r = r;
    lights[n-1].c.g = g;
    lights[n-1].c.b = b;
}

/*---------------------------------------------------------------------------*
 * Function  : m3d_renderer::light_element_amb()
 *
 * Purpose   : Light an element by any ambient lights in the scene.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs :  t   (int)   : the element type (one of element_type)
 *           e (element*): the element
 *
 * Modifications:
 *   Brad Whitlock, Mon Mar 3 12:20:12 PDT 2003
 *   I rewrote it.
 *
 *---------------------------------------------------------------------------*/

void
m3d_renderer::light_element_amb(m3d_element *e)
{
    for (int i = 0; i < 8; i++)
    {
        if(lights[i].t == M3D_LIGHT_AMB)
            e->light_element_amb(lights[i]);
    }
}

/*---------------------------------------------------------------------------*
 * Function  : m3d_renderer::light_element_dir()
 *
 * Purpose   : Light an element by any directional lights in the scene.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs :  t   (int)   : the element type (one of element_type)
 *           e (element*): the element
 *
 * Modifications:
 *   Brad Whitlock, Mon Mar 3 12:21:47 PDT 2003
 *   I rewrote it.
 *
 *---------------------------------------------------------------------------*/

void
m3d_renderer::light_element_dir(m3d_element *e)
{
    for (int i = 0; i < 8; i++)
    {
        if(lights[i].t == M3D_LIGHT_DIR)
            e->light_element_dir(lights[i], vrp, specularEnabled);
    }
}

/*---------------------------------------------------------------------------*
 * Function  : m3d_renderer::light_element_eye()
 *
 * Purpose   : Light an element by any eye lights in the scene.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs :  t   (int)   : the element type (one of element_type)
 *           e (element*): the element
 *
 * Modifications:
 *   Brad Whitlock, Mon Mar 3 12:24:05 PDT 2003
 *   I rewrote it.
 *
 *---------------------------------------------------------------------------*/

void
m3d_renderer::light_element_eye(m3d_element *e)
{
    for (int i = 0; i < 8; i++)
    {
        if(lights[i].t == M3D_LIGHT_EYE)
            e->light_element_eye(lights[i], vrp, specularEnabled);
    }
}

// ****************************************************************************
// Method: m3d_renderer::begin_scene
//
// Purpose: 
//   Prepare the renderer for adding new elements.
//
// Arguments:
//   p : The painter object that we'll use to draw the scene.
//
// Programmer: Jeremy Meredith
// Creation:   October 27, 1999
//
// Modifications:
//   Brad Whitlock, Tue Mar 4 09:04:01 PDT 2003
//   I rewrote it.
//
// ****************************************************************************

void
m3d_renderer::begin_scene(QPainter *p)
{
    painter = p;

    // Delete old elements stored in the sort elements.
    for(int i = 0; i < numElements; ++i)
        delete sort_elements[i].e;
    numElements = 0;
}

// ****************************************************************************
// Method: m3d_renderer::end_scene
//
// Purpose: 
//   Sorts the elements and draws them.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 08:23:56 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
m3d_renderer::end_scene()
{
    int i;

    sort_element *e = sort_elements;
    for (i = 0; i < numElements; ++i, ++e)
        e->Z = e->e->calc_avg_z_sort_element();

    // Sort the elements in Z.
    qsort(sort_elements, numElements, sizeof(sort_element),
          sort_element_compare);

    // Draw the elements.
    e = sort_elements;
    for (i = 0; i < numElements; ++i, ++e)
        e->e->draw(*this);
}

// ****************************************************************************
// Method: m3d_renderer::probe
//
// Purpose: 
//   Looks through the elements that were drawn and tries to figure out which
//   one we clicked on. If one was clicked, return its id; otherwise return -1.
//
// Arguments:
//   x : The x location of the click.
//   y : The y location of the click.
//
// Returns:    The id of the element that we clicked or -1.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 5 12:11:34 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

int
m3d_renderer::probe(int x, int y) const
{
    int retval = -1;

    // Start looking through the elements (that were drawn) in reverse order
    // to see if we can find the one that that we clicked on.
    for(int i = numElements - 1; i >= 0; --i)
    {
        if(sort_elements[i].e->wasDrawn() &&
           sort_elements[i].e->intersects(x, y))
        {
            retval = int(sort_elements[i].e->getId());
            break;
        }
    }

    return retval;
}

/*---------------------------------------------------------------------------*
 * Function  : m3d_renderer::addElement()
 *
 * Purpose   : Add a single element to be displayed
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs :  t   (int)  : the element type, one of enum element_type
 *           e (element): the element to add
 *
 * Modifications:
 *   Brad Whitlock, Tue Mar 4 10:47:32 PDT 2003
 *   Rewrote it.
 *
 *---------------------------------------------------------------------------*/

void
m3d_renderer::addElement(m3d_element *e)
{
    if(checkStorage())
    {
        // Pipeline 
        light_element_amb(e);

        e->transform(&world);
        light_element_dir(e);

        e->transform(&view);

        bool addIt = true;
        if(backfaceCulling)
            addIt = e->notCulled(cull_vrp);

        if(addIt)
        {
            light_element_eye(e);
            e->transform(&proj);
            e->transform(&device);

            // Store it for sorting and rasterization.
            sort_elements[numElements].e = e;
            sort_elements[numElements].Z = 0.;
            ++numElements;
        }
        else
        {
            delete e;
        }
    }
}

/*---------------------------------------------------------------------------*
 * Function  : m3d_renderer::sort_element_compare()
 *
 * Purpose   : Comparison function for sorting two elements.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs :  a,b (const void*): the elements to compare
 *
 * Returns:  int:  -1 => 'a' comes before 'b'
 *                  0 => 'a' is the same as 'b'
 *                  1 => 'a' comes after 'b'
 *
 *
 * Notes:
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------*/
int
m3d_renderer::sort_element_compare(const void *a, const void *b)
{
#ifdef RHCS
    if      (((sort_element*)a)->Z  > ((sort_element*)b)->Z)
#else
    if      (((sort_element*)a)->Z  < ((sort_element*)b)->Z)
#endif
        return -1;
    else if (((sort_element*)a)->Z == ((sort_element*)b)->Z)
        return  0;
    else
        return  1;
}

// ****************************************************************************
// Method: m3d_renderer::checkStorage
//
// Purpose: 
//   Makes sure that the renderer has enough memory to store the elements that
//   we want to draw.
//
// Returns:    true if we have enough memory to store one more element.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 10:53:09 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
m3d_renderer::checkStorage()
{
    if(maxElements == 0)
    {
        // Initial allocation
        maxElements = 500;
        numElements = 0;
        sort_elements = new sort_element[maxElements];
    }
    else if(numElements == maxElements)
    {
        // Resize allocation.
        sort_element *newElements = new sort_element[maxElements*2];
        if(newElements)
        {
            for(int i = 0; i < maxElements; ++i)
            {
                newElements[i].e = sort_elements[i].e;
                newElements[i].Z = 0.;
            }
        }
        delete [] sort_elements;
        sort_elements = newElements;
        maxElements *= 2;
    }

    return (sort_elements != 0);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*                            Utility Functions                              */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Function  : transform_vertex()
 *
 * Purpose   : Transform a 3D vertex by a 4x4 matrix.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs :  M (matrix4*): the matrix
 *           v (vector3*): the vector
 *
 * Returns:  
 *
 *
 * Notes:
 *
 * Modifications:
 *   Brad Whitlock, Tue Mar 4 11:49:04 PDT 2003
 *   I made the matrix be const.
 *
 *---------------------------------------------------------------------------*/

void
transform_vertex(const matrix4 *M, vector3 *v)
{
    float x,y,z,w,iw;
    const float *m0 = (const float *)M->m[0];
    const float *m1 = (const float *)M->m[1];
    const float *m2 = (const float *)M->m[2];
    const float *m3 = (const float *)M->m[3];

    x = m0[0] * v->x +
        m0[1] * v->y +
        m0[2] * v->z +
        m0[3];
    y = m1[0] * v->x +
        m1[1] * v->y +
        m1[2] * v->z +
        m1[3];
    z = m2[0] * v->x +
        m2[1] * v->y +
        m2[2] * v->z +
        m2[3];
    w = m3[0] * v->x +
        m3[1] * v->y +
        m3[2] * v->z +
        m3[3];

    iw   = 1. / w;
    v->x = x * iw;
    v->y = y * iw;
    v->z = z * iw;
}

/*---------------------------------------------------------------------------*
 * Function  : transform_vector()
 *
 * Purpose   : Transform a 3D vector by a 4x4 matrix.  Since this is a vector,
 *             do NOT perform translation or homogenization.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs :  M (matrix4*): the matrix
 *           v (vector3*): the vector
 *
 * Returns:  
 *
 *
 * Notes:
 *
 * Modifications:
 *   Brad Whitlock, Tue Mar 4 11:49:19 PDT 2003
 *   I made the matrix be const.
 *
 *---------------------------------------------------------------------------*/

void
transform_vector(const matrix4 *M, vector3 *v)
{
    float x,y,z;

    x = M->m[0][0] * v->x +
        M->m[0][1] * v->y +
        M->m[0][2] * v->z;
    y = M->m[1][0] * v->x +
        M->m[1][1] * v->y +
        M->m[1][2] * v->z;
    z = M->m[2][0] * v->x +
        M->m[2][1] * v->y +
        M->m[2][2] * v->z;

    v->x = x;
    v->y = y;
    v->z = z;
    /* *v = vec_normalize(*v);  << This is a good idea, but not yet needed */
}

/*---------------------------------------------------------------------------*
 * Function  : m3du_create_identity_matrix()
 *
 * Purpose   : Return the identity matrix.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs : 
 *
 * Returns:  matrix4
 *
 *
 * Notes:
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------*/
matrix4
m3du_create_identity_matrix(void)
{
    matrix4 M = {{{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}}};
    return M;
}

/*---------------------------------------------------------------------------*
 * Function  : m3du_create_zero_matrix()
 *
 * Purpose   : Return the zero matrix.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs : 
 *
 * Returns:  matrix4
 *
 *
 * Notes:
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------*/
matrix4
m3du_create_zero_matrix(void)
{
    matrix4 M = {{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}};
    return M;
}

/*---------------------------------------------------------------------------*
 * Function  : m3du_create_view_matrix()
 *
 * Purpose   : Create a matrix for the world->view transform.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs :  from     (vector3): the viewer's origin
 *           at       (vector3): the viewer's target
 *           world_up (vector3): the world up vector
 *
 * Returns:  matrix4
 *
 *
 * Notes:
 *    Care was taken to ensure that this performed correctly in a RH system.
 *    Nevertheless, since this was not documented sufficiently in any
 *    texts, so caveat emptor.
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------*/
matrix4
m3du_create_view_matrix(const vector3 from, 
                        const vector3 at, 
                        const vector3 world_up)
{
    matrix4 M;
    vector3 up, right, view_dir;

    view_dir = vec_normalize(vec_sub(at,from));
#ifdef RHCS
    view_dir = vec_negate(view_dir);
#endif
    right    = vec_cross(world_up,view_dir);
    up       = vec_cross(view_dir,right);

    right = vec_normalize(right);
    up    = vec_normalize(up);

    M = m3du_create_identity_matrix();
    
    M.m[0][0] = right.x;
    M.m[0][1] = right.y;
    M.m[0][2] = right.z;
    M.m[1][0] = up.x;
    M.m[1][1] = up.y;
    M.m[1][2] = up.z;
    M.m[2][0] = view_dir.x;
    M.m[2][1] = view_dir.y;
    M.m[2][2] = view_dir.z;

    M.m[0][3] = -vec_dot(right   ,from);
    M.m[1][3] = -vec_dot(up      ,from);
    M.m[2][3] = -vec_dot(view_dir,from);

    return M;
}

/*---------------------------------------------------------------------------*
 * Function  : m3du_create_proj_matrix()
 *
 * Purpose   : Create a matrix for the view->projection transform.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs : near_plane (float): the distance to the near clipping plane
 *          far_plane  (float): the distance to the far clipping plane
 *          fov        (float): the full-width field of view
 *
 * Returns:  matrix4
 *
 *
 * Notes:
 *    Care was taken to ensure that this performed correctly in a RH system.
 *    Nevertheless, since this was not documented sufficiently in any
 *    texts, so caveat emptor.
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------*/
matrix4
m3du_create_proj_matrix(const float near_plane,
                        const float far_plane,
                        const float fov)
{
    float  c, s, Q;
    matrix4 M;

    c = (float) cos(fov*0.5);
    s = (float) sin(fov*0.5);
    Q = s/(1.0f - near_plane/far_plane);

    M = m3du_create_zero_matrix();

    M.m[0][0] = c;
    M.m[1][1] = c;
    M.m[2][2] = Q;
    M.m[2][3] = -Q*near_plane;
    M.m[3][2] = s;
#ifdef RHCS
    M.m[3][2] *= -1;
#endif

    return M;
}

/*---------------------------------------------------------------------------*
 * Function  : m3du_create_RBT_matrix()
 *
 * Purpose   : Create a rigid-body-transform matrix.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs :  from     (vector3): the object's new origin
 *           at       (vector3): the object's new Z-axis target direction
 *           world_up (vector3): the object's new world-coordinate up vector
 *
 * Returns:  matrix4
 *
 *
 * Notes:
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------*/
matrix4
m3du_create_RBT_matrix(const vector3 from, 
                       const vector3 at, 
                       const vector3 world_up)
{
    matrix4 M;
    vector3 up, right, view_dir;

    view_dir = vec_normalize(vec_sub(at,from));
    right    = vec_normalize(vec_cross(world_up,view_dir));
    up       = vec_normalize(vec_cross(view_dir,right));

    M = m3du_create_identity_matrix();
    
    M.m[0][0] = right.x;
    M.m[0][1] = right.y;
    M.m[0][2] = right.z;
    M.m[1][0] = up.x;
    M.m[1][1] = up.y;
    M.m[1][2] = up.z;
    M.m[2][0] = view_dir.x;
    M.m[2][1] = view_dir.y;
    M.m[2][2] = view_dir.z;

    /*
     * The matrix so far will put us in the local coordinate system...
     * We want the inverse of that.
     */
    M=mtx_inverse(M);

    /* Don't forget the translation */
    M.m[0][3] = from.x;
    M.m[1][3] = from.y;
    M.m[2][3] = from.z;

    return M;
}


/*---------------------------------------------------------------------------*
 * Function  : m3du_create_trackball_matrix()
 *
 * Purpose   : Create a matrix based on a virtual trackball.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Inputs : p1x,p1y (float): the mouse start coordinates [-1.0,1.0]
 *          p2x,p2y (float): the mouse end   coordinates [-1.0,1.0]
 *
 * Returns:  matrix4
 *
 *
 * Notes:
 *    This is based on the trackball code from Spi.
 *
 * Modifications:
 *
 *---------------------------------------------------------------------------*/
matrix4
m3du_create_trackball_matrix(float p1x, float p1y, float p2x, float p2y)
{
#define RADIUS       0.8        /* z value at x = y = 0.0  */
#define COMPRESSION  3.5        /* multipliers for x and y */
#define AR3 (RADIUS*RADIUS*RADIUS)

   float   q[4];   /* quaternion */
   vector3 p1, p2; /* pointer loactions on trackball */
   vector3 axis;   /* axis of rotation               */
   double  phi;    /* rotation angle (radians)       */
   double  t;
   matrix4 M;

   /* Check for zero mouse movement */
   if (p1x==p2x && p1y==p2y)
      return m3du_create_identity_matrix();


   /* Compute z-coordinates for projection of P1 and P2 onto    */
   /* the trackball.                        */
   p1 = vec_create(p1x, p1y, AR3/((p1x*p1x+p1y*p1y)*COMPRESSION+AR3));
   p2 = vec_create(p2x, p2y, AR3/((p2x*p2x+p2y*p2y)*COMPRESSION+AR3));

   /* Compute the axis of rotation and temporarily store it    */
   /* in the quaternion.                    */
   axis = vec_normalize(vec_cross(p2,p1));

   /* Figure how much to rotate around that axis.        */
   t = vec_norm(vec_sub(p2, p1));
   t = QMIN(QMAX(t, -1.0), 1.0);
   phi = 2.0*asin(t/(2.0*RADIUS));

   axis = vec_scale(axis, sin(phi/2.0));
   q[0] = axis.x;
   q[1] = axis.y;
   q[2] = axis.z;
   q[3] = cos(phi/2.0);

   /* normalize quaternion to unit magnitude */
   t =  1.0 / sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
   q[0] *= t;
   q[1] *= t;
   q[2] *= t;
   q[3] *= t;

#ifndef RHCS
   q[2]*=-1;   /*  This isn't needed in a RH coordinate system */
#endif

   /* create the rotation matrix from the quaternion */
   M = m3du_create_identity_matrix();

   M.m[0][0] = 1.0 - 2.0 * (q[1]*q[1] + q[2]*q[2]);
   M.m[0][1] = 2.0 * (q[0]*q[1] + q[2]*q[3]);
   M.m[0][2] = (2.0 * (q[2]*q[0] - q[1]*q[3]) );

   M.m[1][0] = 2.0 * (q[0]*q[1] - q[2]*q[3]);
   M.m[1][1] = 1.0 - 2.0 * (q[2]*q[2] + q[0]*q[0]);
   M.m[1][2] = (2.0 * (q[1]*q[2] + q[0]*q[3]) );

   M.m[2][0] = (2.0 * (q[2]*q[0] + q[1]*q[3]) );
   M.m[2][1] = (2.0 * (q[1]*q[2] - q[0]*q[3]) );
   M.m[2][2] = (1.0 - 2.0 * (q[1]*q[1] + q[0]*q[0]) );

   return M;
}

// ****************************************************************************
// Function: m3du_create_scaling_matrix
//
// Purpose:
//   Creates a scaling matrix.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:43:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

matrix4
m3du_create_scaling_matrix(const float sx,
                           const float sy,
                           const float sz)
{
    // m3d seems to use matrices that are the transpose of what is common.
    matrix4 m = m3du_create_identity_matrix();
    m.m[0][0] = sx;
    m.m[1][1] = sy;
    m.m[2][2] = sz;
    return m;
}

// ****************************************************************************
// Function: m3du_create_translation_matrix
//
// Purpose:
//   Creates a translation matrix.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 4 13:43:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

matrix4
m3du_create_translation_matrix(const float tx,
                               const float ty,
                               const float tz)
{
    // m3d seems to use matrices that are the transpose of what is common.
    matrix4 m = m3du_create_identity_matrix();
    m.m[0][3] = tx;
    m.m[1][3] = ty;
    m.m[2][3] = tz;
    return m;
}
