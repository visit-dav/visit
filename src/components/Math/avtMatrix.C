/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#include "avtMatrix.h"
#include <avtVector.h>
#include <math.h>

#define MAX(a,b) ((a)>(b) ? (a) : (a))
#define MIN(a,b) ((a)>(b) ? (b) : (a))

avtMatrix::avtMatrix()
{
    MakeIdentity();
}

avtMatrix::avtMatrix(const avtMatrix &R)
{
    for (int r=0; r<4; r++)
        for (int c=0; c<4; c++)
            m[r][c] = R.m[r][c];
}

avtMatrix::avtMatrix(const double *R)
{
    int index = 0;
    for (int r=0; r<4; r++)
        for (int c=0; c<4; c++)
            m[r][c] = R[index++];
}

avtMatrix::~avtMatrix()
{
    // nothing
}

void
avtMatrix::operator=(const avtMatrix &R)
{
    for (int r=0; r<4; r++)
        for (int c=0; c<4; c++)
            m[r][c] = R.m[r][c];
}


avtMatrix
avtMatrix::operator*(const avtMatrix &R) const
{
    avtMatrix C;
    for (int r=0; r<4; r++)
        for (int c=0; c<4; c++)
            C.m[r][c] = m[r][0] * R.m[0][c] +
                        m[r][1] * R.m[1][c] +
                        m[r][2] * R.m[2][c] +
                        m[r][3] * R.m[3][c];
    return C;
}

avtVector
avtMatrix::operator*(const avtVector &r) const
{
    double x,y,z,w;

    x = m[0][0] * r.x +
        m[0][1] * r.y +
        m[0][2] * r.z +
        m[0][3];
    y = m[1][0] * r.x +
        m[1][1] * r.y +
        m[1][2] * r.z +
        m[1][3];
    z = m[2][0] * r.x +
        m[2][1] * r.y +
        m[2][2] * r.z +
        m[2][3];
    w = m[3][0] * r.x +
        m[3][1] * r.y +
        m[3][2] * r.z +
        m[3][3];

    double iw = 1. / w;
    x *= iw;
    y *= iw;
    z *= iw;

    return avtVector(x,y,z);
}

avtVector
avtMatrix::operator^(const avtVector &r) const
{
    double x,y,z;

    x = m[0][0] * r.x +
        m[0][1] * r.y +
        m[0][2] * r.z;
    y = m[1][0] * r.x +
        m[1][1] * r.y +
        m[1][2] * r.z;
    z = m[2][0] * r.x +
        m[2][1] * r.y +
        m[2][2] * r.z;

    // note -- could insert "normalize" in here
    return avtVector(x,y,z);
}

// ----------------------------------------------------------------------------
void
avtMatrix::MakeIdentity()
{
    for (int r=0; r<4; r++)
        for (int c=0; c<4; c++)
            m[r][c] = (r==c) ? 1 : 0;
}
avtMatrix
avtMatrix::CreateIdentity()
{
    avtMatrix M;
    M.MakeIdentity();
    return M;
}

void
avtMatrix::MakeZero()
{
    for (int r=0; r<4; r++)
        for (int c=0; c<4; c++)
            m[r][c] = 0;
}
avtMatrix
avtMatrix::CreateZero()
{
    avtMatrix M;
    M.MakeZero();
    return M;
}

void
avtMatrix::MakeTranslate(double x,double y,double z)
{
    MakeIdentity();
    m[0][3] = x;
    m[1][3] = y;
    m[2][3] = z;
}
avtMatrix
avtMatrix::CreateTranslate(double x,double y,double z)
{
    avtMatrix M;
    M.MakeTranslate(x,y,z);
    return M;
}

void
avtMatrix::MakeTranslate(const avtVector &t)
{
    MakeIdentity();
    m[0][3] = t.x;
    m[1][3] = t.y;
    m[2][3] = t.z;
}
avtMatrix
avtMatrix::CreateTranslate(const avtVector &t)
{
    avtMatrix M;
    M.MakeTranslate(t);
    return M;
}

void
avtMatrix::MakeScale(double s)
{
    MakeIdentity();
    m[0][0] = s;
    m[1][1] = s;
    m[2][2] = s;
}
avtMatrix
avtMatrix::CreateScale(double s)
{
    avtMatrix M;
    M.MakeScale(s);
    return M;
}

void
avtMatrix::MakeScale(double x,double y,double z)
{
    MakeIdentity();
    m[0][0] = x;
    m[1][1] = y;
    m[2][2] = z;
}
avtMatrix
avtMatrix::CreateScale(double x,double y,double z)
{
    avtMatrix M;
    M.MakeScale(x, y, z);
    return M;
}

void
avtMatrix::MakeView(const avtVector &from, 
                      const avtVector &at, 
                      const avtVector &world_up)
{
    avtVector up, right, view_dir;

    view_dir = (at - from).normalized();
    right    = (world_up % view_dir).normalized();
    up       = (view_dir % right).normalized();

    MakeIdentity();
        
    m[0][0] = right.x;
    m[0][1] = right.y;
    m[0][2] = right.z;
    m[1][0] = up.x;
    m[1][1] = up.y;
    m[1][2] = up.z;
    m[2][0] = view_dir.x;
    m[2][1] = view_dir.y;
    m[2][2] = view_dir.z;

    m[0][3] = -(right*from);
    m[1][3] = -(up*from);
    m[2][3] = -(view_dir*from);
}
avtMatrix
avtMatrix::CreateView(const avtVector &from, 
                      const avtVector &at, 
                      const avtVector &world_up)
{
    avtMatrix M;
    M.MakeView(from, at, world_up);
    return M;
}

void
avtMatrix::MakeRotation(const avtVector &from, 
                        const avtVector &at, 
                        const avtVector &world_up)
{
    avtVector new_z = (from - at).normalized();
    avtVector new_x = (world_up % new_z).normalized();
    avtVector new_y = (new_z % new_x).normalized();

    MakeIdentity();
        
    m[0][0] = new_x.x;
    m[0][1] = new_y.x;
    m[0][2] = new_z.x;
    m[1][0] = new_x.y;
    m[1][1] = new_y.y;
    m[1][2] = new_z.y;
    m[2][0] = new_x.z;
    m[2][1] = new_y.z;
    m[2][2] = new_z.z;
}
avtMatrix
avtMatrix::CreateRotation(const avtVector &from, 
                     const avtVector &at, 
                     const avtVector &world_up)
{
    avtMatrix M;
    M.MakeRotation(from, at, world_up);
    return M;
}

void
avtMatrix::MakeRBT(const avtVector &from, 
                     const avtVector &at, 
                     const avtVector &world_up)
{
    avtVector up, right, view_dir;

    view_dir = (at - from).normalized();
    right    = (world_up % view_dir).normalized();
    up       = (view_dir % right).normalized();

    MakeIdentity();
        
    m[0][0] = right.x;
    m[0][1] = right.y;
    m[0][2] = right.z;
    m[1][0] = up.x;
    m[1][1] = up.y;
    m[1][2] = up.z;
    m[2][0] = view_dir.x;
    m[2][1] = view_dir.y;
    m[2][2] = view_dir.z;

    //
    // The matrix so far will put us in the local coordinate system...
    // We want the inverse of that.
    //
    Inverse();

    // Don't forget the translation
    m[0][3] = from.x;
    m[1][3] = from.y;
    m[2][3] = from.z;
}
avtMatrix
avtMatrix::CreateRBT(const avtVector &from, 
                     const avtVector &at, 
                     const avtVector &world_up)
{
    avtMatrix M;
    M.MakeRBT(from, at, world_up);
    return M;
}

void
avtMatrix::MakePerspectiveProjection(double near_plane,
                                       double far_plane,
                                       double fov,
                                       double aspect)
{
    double  c, s, Q;

    c = cos(fov*0.5);
    s = sin(fov*0.5);
    Q = s/(1.0 - near_plane/far_plane);

    MakeZero();

    m[0][0] = c/aspect;
    m[1][1] = c;
    m[2][2] = Q;
    m[2][3] = -Q*near_plane;
    m[3][2] = s;
}
avtMatrix
avtMatrix::CreatePerspectiveProjection(double near_plane,
                                       double far_plane,
                                       double fov,
                                       double aspect)
{
    avtMatrix M;
    M.MakePerspectiveProjection(near_plane, far_plane, fov, aspect);
    return M;
}

void
avtMatrix::MakeOrthographicProjection(double size,
                                        double near_plane,
                                        double far_plane,
                                        double aspect)
{
    double d;
    d = far_plane - near_plane;

    MakeIdentity();
    m[0][0] = 2./(size*aspect);
    m[1][1] = 2./size;
    m[2][2] = 1./d;
    m[2][3] = -near_plane/d;
    m[3][3] = 1;
}
avtMatrix
avtMatrix::CreateOrthographicProjection(double size,
                                        double near_plane,
                                        double far_plane,
                                        double aspect)
{
    avtMatrix M;
    M.MakeOrthographicProjection(size, near_plane, far_plane, aspect);
    return M;
}


void
avtMatrix::MakeTrackball(double p1x,double p1y,  double p2x, double p2y,
                         bool lhs)
{
#define RADIUS       0.8        /* z value at x = y = 0.0  */
#define COMPRESSION  3.5        /* multipliers for x and y */
#define AR3 (RADIUS*RADIUS*RADIUS)

    double     q[4];   // quaternion
    avtVector  p1, p2; // pointer loactions on trackball
    avtVector  axis;   // axis of rotation
    double     phi;    // rotation angle (radians)
    double     t;

    // Check for zero mouse movement
    if (p1x==p2x && p1y==p2y)
    {
        MakeIdentity();
        return;
    }

    // Compute z-coordinates for projection of P1 and P2 onto
    // the trackball.
    p1 = avtVector(p1x, p1y, AR3/((p1x*p1x+p1y*p1y)*COMPRESSION+AR3));
    p2 = avtVector(p2x, p2y, AR3/((p2x*p2x+p2y*p2y)*COMPRESSION+AR3));

    // Compute the axis of rotation and temporarily store it
    // in the quaternion.
    axis = (p2 % p1).normalized();

    // Figure how much to rotate around that axis.
    t = (p2 - p1).norm();
    t = MIN(MAX(t, -1.0), 1.0);
    phi = -2.0*asin(t/(2.0*RADIUS));

    axis *= sin(phi/2.0);
    q[0]  = axis.x;
    q[1]  = axis.y;
    q[2]  = axis.z;
    q[3]  = cos(phi/2.0);

    // normalize quaternion to unit magnitude
    t =  1.0 / sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
    q[0] *= t;
    q[1] *= t;
    q[2] *= t;
    q[3] *= t;

    //  Handle LH coordinate systems.
    if (lhs)
    {
        q[2]*=-1;
    }

    // create the rotation matrix from the quaternion
    MakeIdentity();

    m[0][0] = 1.0 - 2.0 * (q[1]*q[1] + q[2]*q[2]);
    m[0][1] = 2.0 * (q[0]*q[1] + q[2]*q[3]);
    m[0][2] = (2.0 * (q[2]*q[0] - q[1]*q[3]) );

    m[1][0] = 2.0 * (q[0]*q[1] - q[2]*q[3]);
    m[1][1] = 1.0 - 2.0 * (q[2]*q[2] + q[0]*q[0]);
    m[1][2] = (2.0 * (q[1]*q[2] + q[0]*q[3]) );

    m[2][0] = (2.0 * (q[2]*q[0] + q[1]*q[3]) );
    m[2][1] = (2.0 * (q[1]*q[2] - q[0]*q[3]) );
    m[2][2] = (1.0 - 2.0 * (q[1]*q[1] + q[0]*q[0]) );
}

avtMatrix
avtMatrix::CreateTrackball(double p1x,double p1y,  double p2x, double p2y,
                           bool lhs)
{
    avtMatrix M;
    M.MakeTrackball(p1x,p1y, p2x,p2y, lhs);
    return M;
}


ostream &operator<<(ostream& out, const avtMatrix &r)
{
    out << r.m[0][0] << "," << r.m[0][1] << "," << r.m[0][2] << "," << r.m[0][3] << "\n";
    out << r.m[1][0] << "," << r.m[1][1] << "," << r.m[1][2] << "," << r.m[1][3] << "\n";
    out << r.m[2][0] << "," << r.m[2][1] << "," << r.m[2][2] << "," << r.m[2][3] << "\n";
    out << r.m[3][0] << "," << r.m[3][1] << "," << r.m[3][2] << "," << r.m[3][3] << "\n";
    return out;
}

void avtMatrix::Transpose()
{
    double t;
    for (int r=0; r<4; r++)
        for (int c=0; c<r; c++)
        {
            t       = m[r][c];
            m[r][c] = m[c][r];
            m[c][r] = t;
        }
    
}


static void 
lubksb(avtMatrix *a, int *indx, double *b)
{
    int         i, j, ii=-1, ip;
    double      sum;

    for (i=0; i<4; i++) {
        ip = indx[i];
        sum = b[ip];
        b[ip] = b[i];
        if (ii>=0) {
            for (j=ii; j<=i-1; j++) {
                sum -= (*a)[i][j] * b[j];
            }
        } else if (sum != 0.0) {
            ii = i;
        }
        b[i] = sum;
    }
    for (i=3; i>=0; i--) {
        sum = b[i];
        for (j=i+1; j<4; j++) {
            sum -= (*a)[i][ j] * b[j];
        }
        b[i] = sum/(*a)[i][ i];
    }
}

static int
ludcmp(avtMatrix *a, int *indx, double *d)
{
    double      vv[4];               /* implicit scale for each row */
    double      big, dum, sum, tmp;
    int         i, imax, j, k;

    *d = 1.0f;
    for (i=0; i<4; i++) {
        big = 0.0f;
        for (j=0; j<4; j++) {
            if ((tmp = (double) fabs((*a)[i][ j])) > big) {
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
            sum = (*a)[i][ j];
            for (k=0; k<i; k++) {
                sum -= (*a)[i][ k] * (*a)[k][ j];
            }
            (*a)[i][ j] = sum;
        }
        big = 0.0f;
        for (i=j; i<4; i++) {
            sum = (*a)[i][ j];
            for (k=0; k<j; k++) {
                sum -= (*a)[i][ k]*(*a)[k][ j];
            }
            (*a)[i][ j] = sum;
            if ((dum = vv[i] * (double)fabs(sum)) >= big) {
                big = dum;
                imax = i;
            }
        }
        if (j != imax) {
            for (k=0; k<4; k++) {
                dum = (*a)[imax][ k];
                (*a)[imax][ k] = (*a)[j][ k];
                (*a)[j][ k] = dum;
            }
            *d = -(*d);
            vv[imax] = vv[j];
        }
        indx[j] = imax;
        if ((*a)[j][ j] == 0.0f) {
            (*a)[j][ j] = 1.0e-20f;      /* can be 0.0 also... */
        }
        if (j != 3) {
            dum = 1.0f/(*a)[j][ j];
            for (i=j+1; i<4; i++) {
                (*a)[i][ j] *= dum;
            }
        }
    }
    return 0;
}

void
avtMatrix::Inverse()
{
    avtMatrix n, y;
    int                 i, j, indx[4];
    double              d, col[4];

    n=*this;
    if (ludcmp(&n, indx, &d)) {
        MakeIdentity();
        return;
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
    *this = y;
    return;
}


// ****************************************************************************
//  Method:  avtMatrix::MakeFrameToCartesianConversion
//
//  Purpose:
//    Create a matrix that will convert a point in an arbitrary frame, defined
//    by vectors u,v, w and origin o, to a point in the Cartesian frame.
//
//  Arguments:
//      u, v, w, o : the vectors and origin defining the frame 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 4, 2003 
//
// ****************************************************************************

void
avtMatrix::MakeFrameToCartesianConversion(const avtVector &u, const avtVector &v, 
                                          const avtVector &w, const avtVector &o) 
{
    MakeIdentity();

    m[0][0] = u.x;
    m[1][0] = u.y;
    m[2][0] = u.z;

    m[0][1] = v.x;
    m[1][1] = v.y;
    m[2][1] = v.z;

    m[0][2] = w.x;
    m[1][2] = w.y;
    m[2][2] = w.z;

    m[0][3] = o.x;
    m[1][3] = o.y;
    m[2][3] = o.z;
}


// ****************************************************************************
//  Method:  avtMatrix::CreateFrameToCartesianConversion
//
//  Purpose:
//    Create a matrix that will convert a point in an arbitrary frame, defined
//    by vectors u,v, w and origin o, to a point in the Cartesian frame.
//
//  Arguments:
//      u1, v1, w1, o1 : the unit vectors and origin defining a frame.
//
//  Returns:  The created matrix.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 4, 2003 
//
// ****************************************************************************

avtMatrix
avtMatrix::CreateFrameToCartesianConversion(const avtVector &u, const avtVector &v, 
                                            const avtVector &w, const avtVector &o) 
{
    avtMatrix M;
    M.MakeFrameToCartesianConversion(u, v, w, o);
    return M;
}


// ****************************************************************************
//  Method:  avtMatrix::MakeCartesianToFrameConversion
//
//  Purpose:
//     Create a matrix that will convert a point in the Cartesian frame to a 
//     point in an arbitrary frame, defined by unit vectors u,v, w and origin o.
//
//  Arguments:
//      u, v, w, o : the unit vectors and origin defining the frame 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 4, 2003 
//
// ****************************************************************************

void
avtMatrix::MakeCartesianToFrameConversion(const avtVector &u, const avtVector &v, 
                                          const avtVector &w, const avtVector &o) 
{
    //
    // It's simpler to make the frame-to-cartesian and invert it.
    //
    MakeFrameToCartesianConversion(u, v, w, o);
    Inverse();
}


// ****************************************************************************
//  Method:  avtMatrix::CreateCartesianToFrameConversion
//
//  Purpose:
//     Create a matrix that will convert a point in the Cartesian to a point in 
//     an arbitrary frame, defined by unit vectors u,v, w and origin o.
//
//  Notes:  Assumes frames are orthonormal.
//
//  Arguments:
//      u1, v1, w1, o1 : the unit vectors and origin defining a frame.
//
//  Returns:  The created matrix.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 4, 2003 
//
// ****************************************************************************

avtMatrix
avtMatrix::CreateCartesianToFrameConversion(const avtVector &u, const avtVector &v, 
                                            const avtVector &w, const avtVector &o) 
{
    avtMatrix M;
    M.MakeCartesianToFrameConversion(u, v, w, o);
    return M;
}


// ****************************************************************************
//  Method:  avtMatrix::MakeFrameToFrameConversion
//
//  Purpose:
//     Create a matrix that will convert a point in frame1 to a point in 
//     frame2. Frames defined by unit vectors u,v, w and origin o.
//
//  Notes:  Assumes frames are orthonormal.
//
//  Arguments:
//      u1, v1, w1, o1 : the unit vectors and origin defining frame 1.
//      u2, v2, w2, o2 : the unit vectors and origin defining frame 2.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 4, 2003 
//
// ****************************************************************************

void
avtMatrix::MakeFrameToFrameConversion(
        const avtVector &u1, const avtVector &v1, const avtVector &w1, const avtVector &o1, 
        const avtVector &u2, const avtVector &v2, const avtVector &w2, const avtVector &o2) 
{
    avtVector t = (o1-o2); 

    MakeIdentity();

    m[0][0] = u1*u2;    
    m[1][0] = u1*v2;    
    m[2][0] = u1*w2;    

    m[0][1] = v1*u2;
    m[1][1] = v1*v2;
    m[2][1] = v1*w2;

    m[0][2] = w1*u2;
    m[1][2] = w1*v2;
    m[2][2] = w1*w2;

    m[0][3] = t*u2;
    m[1][3] = t*v2;
    m[2][3] = t*w2;
}

// ****************************************************************************
//  Method:  avtMatrix::CreateFrameToFrameConversion
//
//  Purpose:
//     Create a matrix that will convert a point in frame1 to a point in 
//     frame2. Frames defined by unit vectors u,v, w and origin o.
//
//  Notes:  Assumes frames are orthonormal.
//
//  Arguments:
//      u1, v1, w1, o1 : the unit vectors and origin defining frame 1.
//      u2, v2, w2, o2 : the unit vectors and origin defining frame 2.
//
//  Returns:  The created matrix.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 4, 2003 
//
// ****************************************************************************

avtMatrix
avtMatrix::CreateFrameToFrameConversion(
        const avtVector &u1, const avtVector &v1, const avtVector &w1, const avtVector &o1, 
        const avtVector &u2, const avtVector &v2, const avtVector &w2, const avtVector &o2) 
{
    avtMatrix M;
    M.MakeFrameToFrameConversion(u1, v1, w1, o1, u2, v2, w2, o2);
    return M;
}

