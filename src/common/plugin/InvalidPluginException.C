// ************************************************************************* //
//                            InvalidDirectoryException.C                        //
// ************************************************************************* //

#include <stdio.h>
#include <InvalidPluginException.h>

// ****************************************************************************
//  Method: InvalidPluginException constructor
//
//  Arguments:
//      message   the text message
//      name      the plugin name
//      err       the system error
//
//  Programmer: Jeremy Meredith
//  Creation:   May 10, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Aug 31 15:25:28 PDT 2001
//    Fixed the problem with dlerror output not printing.
//
// ****************************************************************************

InvalidPluginException::InvalidPluginException(const char *message,
                                               const char *name,
                                               const char *err)
{
    char str[1024];
    if (err)
        sprintf(str, "Invalid plugin: %s for plugin %s. "
                     "The system error message was '%s'.", message, name, err);
    else
        sprintf(str, "Invalid plugin: %s for plugin %s.", message, name);

    msg = str;
}
