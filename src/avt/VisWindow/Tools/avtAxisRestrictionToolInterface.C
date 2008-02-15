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

#include <avtAxisRestrictionToolInterface.h>
#include <AxisRestrictionAttributes.h>

// ****************************************************************************
//  Constructor:
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  1, 2008
//
// ****************************************************************************
avtAxisRestrictionToolInterface::avtAxisRestrictionToolInterface(
    const VisWindow *v) : avtToolInterface(v)
{
    atts = new AxisRestrictionAttributes;
}

// ****************************************************************************
//  Destructor:  
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  1, 2008
//
// ****************************************************************************
avtAxisRestrictionToolInterface::~avtAxisRestrictionToolInterface()
{
    // nothing
}

// ****************************************************************************
//  Method:  avtAxisRestrictionToolInterface::ResetNumberOfAxes
//
//  Purpose:
//    Sets the number of available axes.
//
//  Arguments:
//    n          the number of axes
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  1, 2008
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 15 13:21:20 EST 2008
//    Added axis names to the axis restriction attributes.
//
// ****************************************************************************
void
avtAxisRestrictionToolInterface::ResetNumberOfAxes(int n)
{
    AxisRestrictionAttributes *a = (AxisRestrictionAttributes *)atts;
    stringVector aname(n, "");
    doubleVector amin(n, -1e+37);
    doubleVector amax(n, +1e+37);
    a->SetNames(aname);
    a->SetMinima(amin);
    a->SetMaxima(amax);
}

// ****************************************************************************
//  Method:  avtAxisRestrictionToolInterface::SetAxisName
//
//  Purpose:
//    Sets the name for an axis.
//
//  Arguments:
//    i          the axis name to set 
//    s          the name 
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2008
//
// ****************************************************************************
void
avtAxisRestrictionToolInterface::SetAxisName(int i, const std::string &s)
{
    AxisRestrictionAttributes *a = (AxisRestrictionAttributes *)atts;
    stringVector &n = a->GetNames();
    if (n.size() <= i)
    {
        // don't bother erroring, either exit or resize
        return;
        //m.resize(i+1, -1e+37);
    }
    n[i] = s;
    a->SelectAll();
}

// ****************************************************************************
//  Method:  avtAxisRestrictionToolInterface::SetAxisMin
//
//  Purpose:
//    Sets the min for an axis.
//
//  Arguments:
//    i          the axis value to set 
//    x          the value 
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  1, 2008
//
// ****************************************************************************
void
avtAxisRestrictionToolInterface::SetAxisMin(int i, double x)
{
    AxisRestrictionAttributes *a = (AxisRestrictionAttributes *)atts;
    doubleVector &m = a->GetMinima();
    if (m.size() <= i)
    {
        // don't bother erroring, either exit or resize
        return;
        //m.resize(i+1, -1e+37);
    }
    m[i] = x;
    a->SelectAll();
}

// ****************************************************************************
//  Method:  avtAxisRestrictionToolInterface::SetAxisMax
//
//  Purpose:
//    Sets the max for an axis.
//
//  Arguments:
//    i          the axis value to set
//    x          the value 
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  1, 2008
//
// ****************************************************************************
void
avtAxisRestrictionToolInterface::SetAxisMax(int i, double x)
{
    AxisRestrictionAttributes *a = (AxisRestrictionAttributes *)atts;
    doubleVector &m = a->GetMaxima();
    if (m.size() <= i)
    {
        // don't bother erroring, either exit or resize
        return;
        //m.resize(i+1, +1e+37);
    }
    m[i] = x;
    a->SelectAll();
}

// ****************************************************************************
//  Method:  avtAxisRestrictionToolInterface::GetAxisName
//
//  Purpose:
//    Gets the name for an axis.
//
//  Arguments:
//    i          the axis value to retrieve 
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  1, 2008
//
// ****************************************************************************
std::string
avtAxisRestrictionToolInterface::GetAxisName(int i) const
{
    AxisRestrictionAttributes *a = (AxisRestrictionAttributes *)atts;
    const stringVector &n = a->GetNames();
    if (n.size() <= i)
    {
        return "";
    }
    return n[i];
}

// ****************************************************************************
//  Method:  avtAxisRestrictionToolInterface::GetAxisMin
//
//  Purpose:
//    Gets the min for an axis.
//
//  Arguments:
//    i          the axis value to retrieve 
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  1, 2008
//
// ****************************************************************************
double
avtAxisRestrictionToolInterface::GetAxisMin(int i) const
{
    AxisRestrictionAttributes *a = (AxisRestrictionAttributes *)atts;
    const doubleVector &m = a->GetMinima();
    if (m.size() <= i)
    {
        return -1e+37;
    }
    return m[i];
}

// ****************************************************************************
//  Method:  avtAxisRestrictionToolInterface::GetAxisMax
//
//  Purpose:
//    Gets the max for an axis.
//
//  Arguments:
//    i          the axis value to retrieve 
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  1, 2008
//
// ****************************************************************************
double
avtAxisRestrictionToolInterface::GetAxisMax(int i) const
{
    AxisRestrictionAttributes *a = (AxisRestrictionAttributes *)atts;
    const doubleVector &m = a->GetMaxima();
    if (m.size() <= i)
    {
        return +1e+37;
    }
    return m[i];
}
