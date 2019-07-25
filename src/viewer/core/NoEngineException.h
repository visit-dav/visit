// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef NO_ENGINE_EXCEPTION_H
#define NO_ENGINE_EXCEPTION_H
#include <viewercore_exports.h>
#include <VisItException.h>

// ****************************************************************************
// Class: NoEngineException
//
// Purpose:
//   This exception is used for cases where no engine exists to perform a task. 
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   February 26, 2003 
//
// Modifications:
//   Brad Whitlock, Mon Mar 3 08:34:11 PDT 2003
//   I changed the exports and moved the ctor/dtor bodies to a .C file.
//
// ****************************************************************************

class VIEWERCORE_API NoEngineException : public VisItException
{
public:
    NoEngineException();
    virtual ~NoEngineException() VISIT_THROW_NOTHING;
};

#endif
