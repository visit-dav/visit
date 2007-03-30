#ifndef NO_ENGINE_EXCEPTION_H
#define NO_ENGINE_EXCEPTION_H
#include <viewer_exports.h>
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

class VIEWER_API NoEngineException : public VisItException
{
public:
    NoEngineException();
    virtual ~NoEngineException() VISIT_THROW_NOTHING;
};

#endif
