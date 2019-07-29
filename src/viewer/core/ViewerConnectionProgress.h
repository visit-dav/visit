// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_CONNECTION_PROGRESS_H
#define VIEWER_CONNECTION_PROGRESS_H
#include <viewercore_exports.h>
#include <string>

// ****************************************************************************
// Class: ViewerConnectionProgress
//
// Purpose:
//   This class provides an interface to show connection progress when launching
//   processes takes a long time.
//
// Notes:      Other implementations can provide a dialog.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep  5 23:52:55 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerConnectionProgress
{
public:
    ViewerConnectionProgress();
    virtual ~ViewerConnectionProgress();

    bool GetCancelled() const;
    bool GetIgnoreHide() const;
    bool GetParallel() const;
    const std::string &GetComponentName() const;
    const std::string &GetHostName() const;

    virtual void Hide();
    virtual void Show();

    virtual void SetComponentName(const std::string &cn);
    virtual void SetHostName(const std::string &host);
    virtual void SetIgnoreHide(bool val);
    virtual void SetParallel(bool);
    virtual void SetTimeout(int t);

protected:
    std::string   hostName;
    std::string   componentName;
    bool          parallel;
    bool          cancelled;
    int           timeout;
    bool          ignoreHide;
};

#endif
