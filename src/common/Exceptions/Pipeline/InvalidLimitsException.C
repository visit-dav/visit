#include <InvalidLimitsException.h>

// *******************************************************************
// Method: InvalidLimitsException::InvalidLimitsException
//
// Purpose: 
//   Constructor for the InvalidLimitsException class.
//
// Programmer: Kathleen Bonnell 
// Creation:   March 27, 2001 
//
// Modifications:
//   
//   Kathleen Bonnell, Wed Apr 25 14:28:22 PDT 2001
//   Added argument to allow different messages.
//
//   Hank Childs, Thu Oct 10 08:53:17 PDT 2002
//   Reference that 0 is not valid for logaritmic ranges.
//
// *******************************************************************

InvalidLimitsException::InvalidLimitsException(const bool log)
{
   if (log)
   {
       msg = "You have non-positive data values, so a log plot won't work.  " 
             "You must use the limits option to restrict the data "
             "limits to values greater than 0.\n";
   }
   else
   {
       msg = "Limits of the plot are out of range for this operation.\n"
             "Please specify limits.\n";
   }
}


