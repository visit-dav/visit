#ifndef INVALID_LIMITS_EXCEPTION_H 
#define INVALID_LIMITS_EXCEPTION_H 
#include <avtexception_exports.h>
#include <string>
#include <PipelineException.h>

// *******************************************************************
// Class: InvalidLimitsException
//
// Purpose:
//   The exception that should be thrown when invalid limits
//   for a particular plot type are encounted.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   March 27, 2001 
//
// Modifications:
//   
//   Kathleen Bonnell, Wed Apr 25 14:28:22 PDT 2001
//   Added argument to constructor.
//
// *******************************************************************

class AVTEXCEPTION_API InvalidLimitsException: public PipelineException 
{
public:
    InvalidLimitsException(const bool log = false);
    virtual ~InvalidLimitsException() VISIT_THROW_NOTHING {;};

};

#endif
