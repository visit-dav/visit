// ************************************************************************* //
//                         avtFileDescriptorManager.h                        //
// ************************************************************************* //

#ifndef AVT_FILE_DESCRIPTOR_MANAGER_H
#define AVT_FILE_DESCRIPTOR_MANAGER_H


typedef void   (*CloseFileCallback)(void *, int);


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

class avtFileDescriptorManager
{
  public:
    static avtFileDescriptorManager   *Instance();

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


