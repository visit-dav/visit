// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_X_DISPLAY_H
#define VISIT_X_DISPLAY_H

#include <VisItDisplay.h>
#include <engine_main_exports.h>

// ****************************************************************************
//  Class:  XDisplay
//
//  Purpose:
//    Manages X servers, utilized for HW rendering on UNIX clusters.
//
//  Programmer:  Tom Fogal
//  Creation:    August 29, 2008
//
//  Modifications:
//
//    Tom Fogal, Tue May 25 15:49:00 MDT 2010
//    Add hostname storage, for printing error messages.
//    Change ::Connect retval to bool.
//
//    Tom Fogal, Wed May 26 09:10:08 MDT 2010
//    Add a method to indicate we should not launch the server.
//
//    Tom Fogal, Wed May  4 14:30:00 MDT 2011
//    Change display to a string for more flexibility.
//
//    Brad Whitlock, Mon Oct 10 11:40:10 PDT 2011
//    Added GetDisplayType.
//
// ****************************************************************************

class ENGINE_MAIN_API XDisplay : public VisItDisplay
{
  public:
                   XDisplay();
    virtual       ~XDisplay();

    virtual bool   Initialize(std::string display,
                              const std::vector<std::string> &args);
    virtual bool   Connect();
    virtual void   Teardown();

    virtual DisplayType GetDisplayType() const;

    // Tell the implementation whether it should launch the X server or just
    // use it.  Must be set before Initialize!
    void           Launch(bool);

  private:
    pid_t       xserver;
    std::string display;
    char        hostname[256];
    bool        launch;
};
#endif /* VISIT_X_DISPLAY_H */
