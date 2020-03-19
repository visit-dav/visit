// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                DebugStream.h                              //
// ************************************************************************* //

#ifndef DEBUG_STREAM_H
#define DEBUG_STREAM_H

#include <misc_exports.h>
#include <visitstream.h>

//
// Hide as much of DebugStream interface as possible.
//
namespace DebugStream
{
    // Query if a given level is enabled
    extern MISC_API bool Level1();
    extern MISC_API bool Level2();
    extern MISC_API bool Level3();
    extern MISC_API bool Level4();
    extern MISC_API bool Level5();

    // Obtain a given level's stream object
    extern MISC_API ostream& Stream1(char const *__file__=0, int=-1);
    extern MISC_API ostream& Stream2();
    extern MISC_API ostream& Stream3();
    extern MISC_API ostream& Stream4();
    extern MISC_API ostream& Stream5();

    // Query what the current level is (more expensive than LevelN())
    extern MISC_API int GetLevel();
}

#define debug1 if (!DebugStream::Level1()) ; else (DebugStream::Stream1((char const *)__FILE__,(int)__LINE__))
#define debug2 if (!DebugStream::Level2()) ; else (DebugStream::Stream2())
#define debug3 if (!DebugStream::Level3()) ; else (DebugStream::Stream3())
#define debug4 if (!DebugStream::Level4()) ; else (DebugStream::Stream4())
#define debug5 if (!DebugStream::Level5()) ; else (DebugStream::Stream5())

#define vcerr(c) if (VisItInit::IsComponent(#c)) std::cerr << #c ": "

#endif
