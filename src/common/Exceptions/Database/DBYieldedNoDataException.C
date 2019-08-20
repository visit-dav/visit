// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          DBYieldedNoDataException.C                       //
// ************************************************************************* //


#include <DBYieldedNoDataException.h>
#include <string>

using std::string;

// ****************************************************************************
//  Method: DBYieldedNoDataException constructor
//
//  Programmer: Mark C. Miller
//  Creation:   28Oct10
//
// ****************************************************************************

DBYieldedNoDataException::DBYieldedNoDataException(const string &filename)
{
    char str[1024];
    snprintf(str, sizeof(str),
        "Although VisIt has opened the file, \"%s\",\n"
        "no data was found in the file for VisIt to work with.",
        filename.c_str());
    msg = str;
}

// ****************************************************************************
//  Method: DBYieldedNoDataException constructor
//
//  Programmer: Mark C. Miller
//  Creation:   28Oct10
//
// ****************************************************************************

DBYieldedNoDataException::DBYieldedNoDataException(const string &filename,
    const string &plugin)
{
    char str[1024];
    snprintf(str, sizeof(str),
        "Although VisIt has opened the file, \"%s\",\n"
        "with the %s plugin, no data was found in the file for VisIt to\n"
        "work with.",
        filename.c_str(), plugin.c_str());
    msg = str;
}

// ****************************************************************************
//  Method: DBYieldedNoDataException constructor
//
//  Programmer: Mark C. Miller
//  Creation:   28Oct10
//
// ****************************************************************************

DBYieldedNoDataException::DBYieldedNoDataException(const string &filename,
    const string &plugin, const string &msg2)
{
    char str[2048];
    snprintf(str, sizeof(str),
        "Although VisIt has opened the file, \"%s\",\n"
        "with the %s plugin, no data was found in the file for VisIt to\n"
        "work with. The plugin issued the following error message which may\n"
        "(or may not) be indicative of the problem...\n%s",
        filename.c_str(), plugin.c_str(), msg2.c_str());
    msg = str;
}
