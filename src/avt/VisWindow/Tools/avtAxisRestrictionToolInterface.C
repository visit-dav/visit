// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
    if (n.size() <= (size_t)i)
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
    if (m.size() <= (size_t)i)
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
    if (m.size() <= (size_t)i)
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
    if (n.size() <= (size_t)i)
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
    if (m.size() <= (size_t)i)
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
    if (m.size() <= (size_t)i)
    {
        return +1e+37;
    }
    return m[i];
}
