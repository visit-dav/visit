// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtFileDescriptorManager.h                        //
// ************************************************************************* //

#ifndef AVT_FILE_DESCRIPTOR_MANAGER_H
#define AVT_FILE_DESCRIPTOR_MANAGER_H


typedef void   (*CloseFileCallback)(void *, int);

#include <database_exports.h>

#include <vector>


// ****************************************************************************
//  Class: avtFileDescriptorManager
//
//  Purpose:
//      Manages all the open file descriptors.  This is a mechanism made use
//      of by the database and exists so that different databases don't need
//      to know about each other.
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2002
//
// ****************************************************************************

class DATABASE_API avtFileDescriptorManager
{
  public:
    static avtFileDescriptorManager   *Instance(void);
    static void                        DeleteInstance(void);

    int                                RegisterFile(CloseFileCallback, void *);
    void                               UnregisterFile(int);
    void                               UsedFile(int);

    void                               SetMaximumNumberOfOpenFiles(int);

  protected:
                                       avtFileDescriptorManager();
    virtual                           ~avtFileDescriptorManager();

    int                                maximumNumberOfOpenFiles;
    int                                currentNumberOfOpenFiles;
    int                                timestamp;
    static avtFileDescriptorManager   *instance;

    std::vector<void *>                closeFileArgs;
    std::vector<CloseFileCallback>     closeFileCallbacks;
    std::vector<bool>                  fileIsOpen;
    std::vector<int>                   fileTimestamp;

    void                               CloseLeastRecentlyUsedFile(void);
};

#endif

