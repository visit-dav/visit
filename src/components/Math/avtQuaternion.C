/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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

