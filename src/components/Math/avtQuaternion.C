#include <avtQuaternion.h>

avtQuaternion::avtQuaternion()
{
    x = y = z = s = 0.0;
}

avtQuaternion::avtQuaternion(const avtVector &v, double phi)
{
    double sin_phi = sin(phi / 2.);
    double cos_phi = cos(phi / 2.);
    x = v.x * sin_phi;
    y = v.y * sin_phi;
    z = v.z * sin_phi;
    s = cos_phi;
}

avtQuaternion::avtQuaternion(const avtQuaternion &q)
{
    x = q.x;
    y = q.y;
    z = q.z;
    s = q.s;
}

void
avtQuaternion::operator=(const avtQuaternion &q)
{
    x = q.x;
    y = q.y;
    z = q.z;
    s = q.s;
}

avtMatrix
avtQuaternion::CreateRotationMatrix()
{
    avtMatrix M;
    M.MakeIdentity();

    M[0][0] = 1. - 2.*(y*y + z*z);
    M[0][1] =      2.*(x*y - z*s);
    M[0][2] =      2.*(z*x + y*s);

    M[1][0] =      2.*(x*y + z*s);
    M[1][1] = 1. - 2.*(z*z + x*x);
    M[1][2] =      2.*(y*z - x*s);

    M[2][0] =      2.*(z*x - y*s);
    M[2][1] =      2.*(y*z + x*s);
    M[2][2] = 1. - 2.*(y*y + x*x);

    return M;
}

double
avtQuaternion::norm()
{
    double n = (x*x + y*y + z*z + s*s);
    if (n>0)
        n = sqrt(n);
    return n;
}

void
avtQuaternion::normalize()
{
    double n = (x*x + y*y + z*z + s*s);
    if (n>0)
    {
        n = 1./sqrt(n);
        x *= n;
        y *= n;
        z *= n;
        s *= n;
    }
}

