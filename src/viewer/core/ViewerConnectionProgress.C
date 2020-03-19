// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerConnectionProgress.h>

ViewerConnectionProgress::ViewerConnectionProgress() : hostName(), 
    componentName(), parallel(false), cancelled(false), timeout(100),
    ignoreHide(false)
{
}

ViewerConnectionProgress::~ViewerConnectionProgress()
{
}

bool
ViewerConnectionProgress::GetCancelled() const
{
    return cancelled;
}

void
ViewerConnectionProgress::SetTimeout(int t)
{
    timeout = t;
}

const std::string &
ViewerConnectionProgress::GetHostName() const
{
    return hostName;
}

void
ViewerConnectionProgress::SetHostName(const std::string &host)
{
    hostName = host;
}

const std::string &
ViewerConnectionProgress::GetComponentName() const
{
    return componentName;
}

void
ViewerConnectionProgress::SetComponentName(const std::string &cn)
{
}

void
ViewerConnectionProgress::SetParallel(bool val)
{
    parallel = val;
}

bool
ViewerConnectionProgress::GetParallel() const
{
    return parallel;
}

bool
ViewerConnectionProgress::GetIgnoreHide() const
{
    return ignoreHide;
}

void
ViewerConnectionProgress::SetIgnoreHide(bool val)
{
    ignoreHide = val;
}

void
ViewerConnectionProgress::Show()
{
}

void
ViewerConnectionProgress::Hide()
{
}
