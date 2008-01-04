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

#include "avtTrackball.h"

//
// Use a Witch of Agnesi for a trackball.
//
// This size should really be based on the distance from the center
// of rotation to the point on the object underneath the mouse.  That
// point would then track the mouse as closely as possible.
//
#define AGNESI_RADIUS   0.8     // Z value at x = y = 0.0
#define COMPRESSION     3.5     // Multipliers for x and y
#define AR3 (AGNESI_RADIUS*AGNESI_RADIUS*AGNESI_RADIUS)

// ****************************************************************************
//  Constructor:  avtTrackball::avtTrackball
//
//  Purpose:
//    Initialize the trackball.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
// ****************************************************************************
avtTrackball::avtTrackball()
{
    constrain = false;
    center = avtVector(0,0);
}

// ****************************************************************************
//  Method:  avtTrackball::SetCenter
//
//  Purpose:
//    Sets the center of rotation in unit screen space.
//
//  Arguments:
//    c          the center
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
// ****************************************************************************
void
avtTrackball::SetCenter(const avtVector &c)
{
    center = c;
}

// ****************************************************************************
//  Method:  avtTrackball::SetCamera
//
//  Purpose:
//    Set a camera to use for transforming the rotation into world space.
//
//  Arguments:
//    C          the current camera matrix
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
// ****************************************************************************
void
avtTrackball::SetCamera(const avtMatrix &C)
{
    inverse_camera = C;
    inverse_camera.Inverse();
}

// ****************************************************************************
//  Method:  avtTrackball::ClearCamera
//
//  Purpose:
//    Disable transforming from camera to world space.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
// ****************************************************************************
void
avtTrackball::ClearCamera()
{
    inverse_camera.MakeIdentity();
}

// ****************************************************************************
//  Method:  avtTrackball::SetConstrainAxis
//
//  Purpose:
//    Set an axis to constrain to.
//
//  Arguments:
//    v          the axis
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
// ****************************************************************************
void
avtTrackball::SetConstrainAxis(const avtVector &v)
{
    constrainAxis = v.normalized();
    constrain = true;
}

// ****************************************************************************
//  Method:  avtTrackball::ClearConstrainAxis
//
//  Purpose:
//    Disable constraining to an axis.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
// ****************************************************************************
void
avtTrackball::ClearConstrainAxis()
{
    constrain = false;
}

// ****************************************************************************
//  Method:  avtTrackball::GetQuaternion
//
//  Purpose:
//    Return the calculated rotation quaternion.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
// ****************************************************************************
const avtQuaternion &
avtTrackball::GetQuaternion() const
{
    return q;
}

// ****************************************************************************
//  Method:  avtTrackball::GetMatrix
//
//  Purpose:
//    Return the calculated rotation matrix.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
// ****************************************************************************
const avtMatrix &
avtTrackball::GetMatrix() const
{
    return M;
}

// ****************************************************************************
//  Method:  avtTrackball::PerformRotation
//
//  Purpose:
//    Create a trackball rotation using two points in unit screen splace
//
//  Arguments:
//    s1         the previous coordinate
//    s2         the current coordinate
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
// ****************************************************************************
void
avtTrackball::PerformRotation(const avtVector &s1, const avtVector &s2)
{
    // Get the rotation axis from the mouse coordinates
    avtVector p1 = Project(s1-center);
    avtVector p2 = Project(s2-center);
    avtVector rot_axis = (p1 % p2).normalized();

    // Put the rotation axis in world space if needed
    rot_axis = inverse_camera ^ rot_axis;

    // Constrain to a given axis, if needed
    double scale = 1.0;
    if (constrain)
    {
        scale = rot_axis * constrainAxis;
        rot_axis = constrainAxis;
    }

#define TRACKBALL_MIN(A,B) (((A)<(B)) ? (A) : (B))
#define TRACKBALL_MAX(A,B) (((A)>(B)) ? (A) : (B))

    // Figure how much to rotate around that axis.
    double d = (p2 - p1).norm();
    d = TRACKBALL_MAX(0.,TRACKBALL_MIN(1.,d));
    double phi = 2.0 * asin(d / (2.0 * AGNESI_RADIUS));

#undef TRACKBALL_MIN
#undef TRACKBALL_MAX

    // Create the quaternion and the rotation matrix
    q = avtQuaternion(rot_axis, phi * scale);
    M = q.CreateRotationMatrix();
}

// ****************************************************************************
//  Method:  avtTrackball::Project
//
//  Purpose:
//    Project a point in unit screen space onto the trackball.
//
//  Arguments:
//    p          the 2d point in unit screen space
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
// ****************************************************************************
avtVector
avtTrackball::Project(const avtVector &s)
{
    double z = double(AR3) / ((s.x*s.x + s.y*s.y) * COMPRESSION + AR3);
    return avtVector(s.x, s.y, z);
}

