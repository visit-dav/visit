#ifndef INVALID_DIMENSIONS_EXCEPTION_H 
#define INVALID_DIMENSIONS_EXCEPTION_H 
#include <avtexception_exports.h>
#include <string>
#include <PipelineException.h>

// *******************************************************************
// Class: InvalidDimensionsException
//
// Purpose:
//   The exception that should be thrown when invalid dimensions
//   for a particular plot type are encounted.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   March 09, 2001 
//
// Modifications:
//   
// *******************************************************************

class AVTEXCEPTION_API InvalidDimensionsException: public PipelineException 
{
public:
    InvalidDimensionsException(const std::string &plot,
                               const std::string &dims);
    virtual ~InvalidDimensionsException() VISIT_THROW_NOTHING {;};

    const std::string &GetPlotName() const;
    const std::string &GetRequiredDimensions() const;
private:
    std::string requiredDims;
    std::string plotName;
};

#endif
