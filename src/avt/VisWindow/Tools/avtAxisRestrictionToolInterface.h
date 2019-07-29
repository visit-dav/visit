// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_AXIS_RESTRICTION_TOOL_INTERFACE_H
#define VISIT_AXIS_RESTRICTION_TOOL_INTERFACE_H
#include <viswindow_exports.h>
#include <avtToolInterface.h>

#include <string>

// ****************************************************************************
// Class: avtAxisRestrictionToolInterface
//
// Purpose:
//   This class contains the information passed to users of the axis 
//   restriction tool.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//    Jeremy Meredith, Fri Feb 15 10:49:22 EST 2008
//    Added axis names support.
//
// ****************************************************************************

class VISWINDOW_API avtAxisRestrictionToolInterface : public avtToolInterface
{
  public:
    avtAxisRestrictionToolInterface(const VisWindow *v);
    virtual ~avtAxisRestrictionToolInterface();

    void ResetNumberOfAxes(int);

    void SetAxisName(int, const std::string &);
    void SetAxisMin(int, double);
    void SetAxisMax(int, double);

    std::string GetAxisName(int) const;
    double GetAxisMin(int) const;
    double GetAxisMax(int) const;
};

#endif
