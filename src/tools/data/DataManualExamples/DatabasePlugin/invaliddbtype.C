// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// NOTE - This code incomplete and is for example purposes only.
//        Do not try to compile.

#include <InvalidDBTypeException.h>

avtXXXXFileFormat::avtXXXXFileFormat(const char *filename)
    : avtSTSDFileFormat(filename)
{
    initialized = false;
}

// Override this method in your reader
void
avtXXXXFileFormat::ActivateTimestep()
{
    Initialize();
}

// Provide this method in your reader
void
avtXXXXFileFormat::Initialize())
{
    if(!initialized)
    {
        bool okay = false;

        // Open the file specified by the filename argument here using
        // your file format API. See if the file has the right things in it.
        // If so, set okay to true.
        YOU MUST IMPLEMENT THIS

        // If your file format API could not open the file then throw
        // an exception.
        if (!okay)
        {
            EXCEPTION1(InvalidDBTypeException,
                "The file could not be opened");
        }

        initialized = true;
    }
}

