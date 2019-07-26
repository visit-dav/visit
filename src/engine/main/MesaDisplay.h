// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_MESA_DISPLAY_H
#define VISIT_MESA_DISPLAY_H

#include <VisItDisplay.h>
#include <engine_main_exports.h>

// ****************************************************************************
//  Class:  MesaDisplay
//
//  Purpose:
//    Initializes a Mesa display; mostly delegates to InitVTK.
//
//  Programmer:  Tom Fogal
//  Creation:    September 1, 2008
//
//  Modifications:
//
//    Tom Fogal, Tue May 25 16:08:23 MDT 2010
//    Made connect return a bool.
//
//    Tom Fogal, Wed May  4 14:59:45 MDT 2011
//    'Initialize' changed signature.
//
//    Brad Whitlock, Mon Oct 10 11:40:10 PDT 2011
//    Added GetDisplayType.
//
// ****************************************************************************

class ENGINE_MAIN_API MesaDisplay : public VisItDisplay
{
  public:
                   MesaDisplay();
    virtual       ~MesaDisplay();

    virtual bool   Initialize(std::string display,
                              const std::vector<std::string> &args);
    virtual bool   Connect();
    virtual void   Teardown();

    virtual DisplayType GetDisplayType() const;
};
#endif /* VISIT_MESA_DISPLAY_H */
