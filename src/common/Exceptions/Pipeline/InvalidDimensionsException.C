#include <InvalidDimensionsException.h>

// *******************************************************************
// Method: InvalidDimensionsException::InvalidDimensionsException
//
// Purpose: 
//   Constructor for the InvalidDimensionsException class.
//
// Arguments:
//   plot : The name of the plot that caused the exception.
//   dims : The dimensions required for this plot type. 
//
// Programmer: Kathleen Bonnell 
// Creation:   March 09, 2001 
//
// Modifications:
//   
//   Hank Childs, Tue Sep 18 09:27:28 PDT 2001
//   Reordered initialization of string to quiet warning.
//
// *******************************************************************

InvalidDimensionsException::InvalidDimensionsException(const std::string &plot,
 const std::string &dims) : requiredDims(dims), plotName(plot)
{
   msg = plot + " requires " + dims + " plot data.\n" ;
}


// *******************************************************************
// Method: InvalidDimensionsException::GetRequiredDimensions
//
// Purpose: 
//   Returns the required dimensions for the given plot stored in the exception.
//
// Programmer: Kathleen Bonnell 
// Creation:   March 09, 2001 
//
// Modifications:
//   
// *******************************************************************

const std::string &
InvalidDimensionsException::GetRequiredDimensions() const
{
    return requiredDims;
}


// *******************************************************************
// Method: InvalidDimensionsException::GetPlotName
//
// Purpose: 
//   Returns the bad plot name stored in the exception.
//
// Programmer: Kathleen Bonnell 
// Creation:   March 09, 2001 
//
// Modifications:
//   
// *******************************************************************

const std::string &
InvalidDimensionsException::GetPlotName() const
{
    return plotName;
}
