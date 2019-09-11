// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// NOTE - This code incomplete and is for example purposes only.
//        Do not try to compile.

// Include this header for debug streams.
#include <DebugStream.h>


vtkDataSet *
avtXXXXFileFormat::GetMesh(const char *meshname)
{
    // Write messages to different levels of the debug logs.
    debug1 << "Hi from avtXXXXFileFormat::GetMesh" << endl;

    debug4 << "Many database plugins prefer debug4 " << endl;

    debug5 << "Lots of detail from avtXXXXFileFormat::GetMesh" << endl;

    return 0;
}
