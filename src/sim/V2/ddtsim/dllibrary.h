// Copyright (c) 2011, Allinea
// All rights reserved.
//
// This file has been contributed to the VisIt project, which is
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef DLLIBRARY_H
#define DLLIBRARY_H

//! Helper class that wraps calls to \c dlopen, \c dlsym and \c dlclose
/*! Manages access to a single dynamically-loaded library. Library will be opened when this object
 * is created, and closed when it is destroyed (i.e. goes out of scope). */
class DlLibrary
{
public:
    DlLibrary(const char* lib);
    ~DlLibrary();
    void* symbol(const char*);
    bool isLoaded() { return handle; }
private:
    void* handle;
    const char* library;
};

#endif // DLLIBRARY_H
