#include "Matrix.h"
#include "Vector.h"
#include <math.h>
#include <visitstream.h>

#define MAX(a,b) ((a)>(b) ? (a) : (a))
#define MIN(a,b) ((a)>(b) ? (b) : (a))

Matrix::Matrix()
{
    CreateIdentity();
}

Matrix::Matrix(const Matrix &R)
{
    for (int r=0; r<4; r++)
        for (int c=0; c<4; c++)
            m[r][c] = R.m[r][c];
}

void
Matrix::operator=(const Matrix &R)
{
    for (int r=0; r<4; r++)
        for (int c=0; c<4; c++)
            m[r][c] = R.m[r][c];
}


Matrix
Matrix::operator*(const Matrix &R) const
{
    Matrix C;
    for (int r=0; r<4; r++)
        for (int c=0; c<4; c++)
            C.m[r][c] = m[r][0] * R.m[0][c] +
                        m[r][1] * R.m[1][c] +
                        m[r][2] * R.m[2][c] +
                        m[r][3] * R.m[3][c];
    return C;
}

Vector
Matrix::operator*(const Vector &r) const
{
    float x,y,z,w;

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

    float iw = 1. / w;
    x *= iw;
    y *= iw;
    z *= iw;

    return Vector(x,y,z);
}

Vector
Matrix::operator^(const Vector &r) const
{
    float x,y,z;

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
    return Vector(x,y,z);
}

float *
Matrix::GetOpenGLMatrix()
{
    openglm[ 0] = m[0][0];
    openglm[ 1] = m[1][0];
    openglm[ 2] = m[2][0];
    openglm[ 3] = m[3][0];
    openglm[ 4] = m[0][1];
    openglm[ 5] = m[1][1];
    openglm[ 6] = m[2][1];
    openglm[ 7] = m[3][1];
    openglm[ 8] = m[0][2];
    openglm[ 9] = m[1][2];
    openglm[10] = m[2][2];
    openglm[11] = m[3][2];
    openglm[12] = m[0][3];
    openglm[13] = m[1][3];
    openglm[14] = m[2][3];
    openglm[15] = m[3][3];
    return openglm;
}

void
Matrix::CreateIdentity()
{
    for (int r=0; r<4; r++)
        for (int c=0; c<4; c++)
            m[r][c] = (r==c) ? 1 : 0;
}

void
Matrix::CreateZero()
{
    for (int r=0; r<4; r++)
        for (int c=0; c<4; c++)
            m[r][c] = 0;
}

void
Matrix::CreateTranslate(float x,float y,float z)
{
    CreateIdentity();
    m[0][3] = x;
    m[1][3] = y;
    m[2][3] = z;
}

void
Matrix::CreateScale(float s)
{
    CreateIdentity();
    m[0][0] = s;
    m[1][1] = s;
    m[2][2] = s;
}

void
Matrix::CreateScale(float x,float y,float z)
{
    CreateIdentity();
    m[0][0] = x;
    m[1][1] = y;
    m[2][2] = z;
}

void
Matrix::CreateView(const Vector &from, 
                  const Vector &at, 
                  const Vector &world_up)
{
    Vector up, right, view_dir;

    view_dir = (at - from).normalized();
    right    = (world_up % view_dir).normalized();
    up       = (view_dir % right).normalized();

    CreateIdentity();

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

void
Matrix::CreateRBT(const Vector &from, 
                  const Vector &at, 
                  const Vector &world_up)
{
    Vector up, right, view_dir;

    view_dir = (at - from).normalized();
    right    = (world_up % view_dir).normalized();
    up       = (view_dir % right).normalized();

    CreateIdentity();

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

void
Matrix::CreatePerspectiveProjection(float near_plane,
                                    float far_plane,
                                    float fov,
                                    float aspect)
{
    float  c, s, Q;

    c = cos(fov*0.5);
    s = sin(fov*0.5);
    Q = s/(1.0 - near_plane/far_plane);

    CreateZero();

    m[0][0] = c/aspect;
    m[1][1] = c;
    m[2][2] = Q;
    m[2][3] = -Q*near_plane;
    m[3][2] = s;
}

void
Matrix::CreateOrthographicProjection(float size,
                                     float near_plane,
                                     float far_plane,
                                     float aspect)
{
    float d;
    d = far_plane - near_plane;

    CreateIdentity();
    m[0][0] = 2./(size*aspect);
    m[1][1] = 2./size;
    m[2][2] = 1./d;
    m[2][3] = -near_plane/d;
    m[3][3] = 1;
}


void
Matrix::CreateTrackball(float p1x,float p1y,  float p2x, float p2y)
{
#define RADIUS       0.8        /* z value at x = y = 0.0  */
#define COMPRESSION  3.5        /* multipliers for x and y */
#define AR3 (RADIUS*RADIUS*RADIUS)

    float   q[4];   // quaternion
    Vector  p1, p2; // pointer loactions on trackball
    Vector  axis;   // axis of rotation
    double  phi;    // rotation angle (radians)
    double  t;

    // Check for zero mouse movement
    if (p1x==p2x && p1y==p2y)
    {
        CreateIdentity();
        return;
    }


    // Compute z-coordinates for projection of P1 and P2 onto
    // the trackball.
    p1 = Vector(p1x, p1y, AR3/((p1x*p1x+p1y*p1y)*COMPRESSION+AR3));
    p2 = Vector(p2x, p2y, AR3/((p2x*p2x+p2y*p2y)*COMPRESSION+AR3));

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

    //q[2]*=-1;   //  This is needed in a LH coordinate system

    // create the rotation matrix from the quaternion
    CreateIdentity();

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


ostream &operator<<(ostream& out, const Matrix &r)
{
    out << r.m[0][0] << "," << r.m[0][1] << "," << r.m[0][2] << "," << r.m[0][3] << "\n";
    out << r.m[1][0] << "," << r.m[1][1] << "," << r.m[1][2] << "," << r.m[1][3] << "\n";
    out << r.m[2][0] << "," << r.m[2][1] << "," << r.m[2][2] << "," << r.m[2][3] << "\n";
    out << r.m[3][0] << "," << r.m[3][1] << "," << r.m[3][2] << "," << r.m[3][3] << "\n";
    return out;
}

void Matrix::Transpose()
{
    float t;
    for (int r=0; r<4; r++)
        for (int c=0; c<r; c++)
        {
            t       = m[r][c];
            m[r][c] = m[c][r];
            m[c][r] = t;
        }
    
}


static void 
lubksb(Matrix *a, int *indx, float *b)
{
    int   i, j, ii=-1, ip;
    float sum;

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
ludcmp(Matrix *a, int *indx, float *d)
{
    float vv[4];               /* implicit scale for each row */
    float big, dum, sum, tmp;
    int   i, imax, j, k;

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

void
Matrix::Inverse()
{
    Matrix n, y;
    int    i, j, indx[4];
    float  d, col[4];

    n=*this;
    if (ludcmp(&n, indx, &d)) {
        CreateIdentity();
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
