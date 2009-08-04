/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <avtPlaneToolInterface.h>
#include <PlaneAttributes.h>

avtPlaneToolInterface::avtPlaneToolInterface(const VisWindow *v) :
    avtToolInterface(v)
{
    atts = new PlaneAttributes;
}

avtPlaneToolInterface::~avtPlaneToolInterface()
{
    // nothing
}

void
avtPlaneToolInterface::SetOrigin(double x, double y, double z)
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    double d[3] = {x, y, z};
    p->SetOrigin(d);
}

void
avtPlaneToolInterface::SetNormal(double x, double y, double z)
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    double d[3] = {x, y, z};
    p->SetNormal(d);
}

void
avtPlaneToolInterface::SetUpAxis(double x, double y, double z)
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    double d[3] = {x, y, z};
    p->SetUpAxis(d);
}

void
avtPlaneToolInterface::SetRadius(const double radius)
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    p->SetRadius(radius);
}

const double *
avtPlaneToolInterface::GetOrigin() const
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    return p->GetOrigin();
}

const double *
avtPlaneToolInterface::GetNormal() const
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    return p->GetNormal();
}

const double *
avtPlaneToolInterface::GetUpAxis() const
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    return p->GetUpAxis();
}

double
avtPlaneToolInterface::GetRadius() const
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    return p->GetRadius();
}

bool
avtPlaneToolInterface::GetHaveRadius() const
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    return p->GetHaveRadius();
}
