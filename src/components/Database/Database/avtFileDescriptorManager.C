// ************************************************************************* //
//                         avtFileDescriptorManager.C                        //
// ************************************************************************* //

#include <avtFileDescriptorManager.h>

#include <BadIndexException.h>
#include <ImproperUseException.h>


using     std::vector;


avtFileDescriptorManager   *avtFileDescriptorManager::instance = NULL;


// ****************************************************************************
//  Method: avtFileDescriptorManager constructor
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Mar 26 13:37:07 PST 2002
//    Initialized timestamp.
//
// ****************************************************************************

avtFileDescriptorManager::avtFileDescriptorManager()
{
    maximumNumberOfOpenFiles = 20;
    currentNumberOfOpenFiles = 0;
    timestamp = 1;
}


// ****************************************************************************
//  Method: avtFileDescriptorManager destructor
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2002
//
// ****************************************************************************

avtFileDescriptorManager::~avtFileDescriptorManager()
{
    // Can't really think of a good reason to make all of the files be closed.
    // This should never be called anyway.
}


// ****************************************************************************
//  Method: avtFileDescriptorManager::Instance
//
//  Purpose:
//      The file descriptor manager is a singleton.  This returns the only
//      instance of it.
//
//  Returns:    The only instance of this object.
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2002
//
// ****************************************************************************

avtFileDescriptorManager *
avtFileDescriptorManager::Instance(void)
{
    if (instance == NULL)
    {
        instance = new avtFileDescriptorManager;
    }

    return instance;
}


// ****************************************************************************
//  Method: avtFileDescriptorManager::SetMaximumNumberOfOpenFiles
//
//  Purpose:
//      Tells the file descriptor manager what the maximum number of open files
//      should be.
//
//  Arguments:
//      num      The new maximum number of files.
//
//  Programmer:  Hank Childs
//  Creation:    March 21, 2002
//
// ****************************************************************************

void
avtFileDescriptorManager::SetMaximumNumberOfOpenFiles(int num)
{
    if (maximumNumberOfOpenFiles <= 0)
    {
        EXCEPTION0(ImproperUseException);
    }

    maximumNumberOfOpenFiles = num;

    while (currentNumberOfOpenFiles >= maximumNumberOfOpenFiles)
    {
        CloseLeastRecentlyUsedFile();
    }
}


// ****************************************************************************
//  Method: RegisterFile
//
//  Purpose:
//      Registers a file that this module should keep track of.
//
//  Arguments:
//      cback   The callback to close the file.
//      args    The arguments needed for cback.
//
//  Returns:    An index to be used when referring to this file with this
//              object.
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2002
//
// ****************************************************************************

int
avtFileDescriptorManager::RegisterFile(CloseFileCallback cback, void *args)
{
    if (currentNumberOfOpenFiles + 1 >= maximumNumberOfOpenFiles)
    {
        CloseLeastRecentlyUsedFile();
    }    

    int index = closeFileCallbacks.size();
    closeFileCallbacks.push_back(cback);
    closeFileArgs.push_back(args);
    fileIsOpen.push_back(true);
    fileTimestamp.push_back(timestamp);

    timestamp++;
    currentNumberOfOpenFiles++;

    return index;
}


// ****************************************************************************
//  Method: avtFileDescriptorManager::UnregisterFile
//
//  Purpose:
//      An object has decided that it wants to close a file on its own accord.
//      This is the routine it uses to communicate that information to this
//      object.
//
//  Arguments:
//      index    The index of the file that has been closed.
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2002
//
// ****************************************************************************

void
avtFileDescriptorManager::UnregisterFile(int index)
{
    if (index >= fileIsOpen.size() || index < 0)
    {
        EXCEPTION2(BadIndexException, index, fileIsOpen.size());
    }

    if (!fileIsOpen[index])
    {
        EXCEPTION0(ImproperUseException);
    }

    fileIsOpen[index] = false;
    currentNumberOfOpenFiles--;
}


// ****************************************************************************
//  Method: avtFileDescriptorManager::UsedFile
//
//  Purpose:
//      Indicates to this object that a file has been used.  This information
//      can then be used in its heuristic to determine which file should be
//      closed later (ie don't close this file because we just used it).
//
//  Arguments:
//      index    An index of a file.
//
//  Programmer:  Hank Childs
//  Creation:    March 21, 2002
//
// ****************************************************************************

void
avtFileDescriptorManager::UsedFile(int index)
{
    if (index >= fileIsOpen.size() || index < 0)
    {
        EXCEPTION2(BadIndexException, index, fileIsOpen.size());
    }

    fileTimestamp[index] = timestamp;
    timestamp++;
}


// ****************************************************************************
//  Method: avtFileDescriptorManager::CloseLeastRecentlyUsedFile
//
//  Purpose:
//      Determines which file was least recently used and closes it.
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2002
//
// ****************************************************************************

void
avtFileDescriptorManager::CloseLeastRecentlyUsedFile(void)
{
    //
    // Identify which file was the least recently used.
    //
    int lowestTimestamp = 10000000;
    int indexForLowestTimestamp = -1;
    int nFiles = fileTimestamp.size();
    for (int i = 0 ; i < nFiles ; i++)
    {
        if (fileIsOpen[i] && (fileTimestamp[i] < lowestTimestamp) )
        {
            lowestTimestamp = fileTimestamp[i];
            indexForLowestTimestamp = i;
        }
    }

    if (indexForLowestTimestamp < 0)
    {
        //
        // This can only happen if we don't have any currently open files.
        //
        EXCEPTION0(ImproperUseException);
    }

    //
    // Issue the callbacks to close the file.
    //
    CloseFileCallback cback = closeFileCallbacks[indexForLowestTimestamp];
    void  *args = closeFileArgs[indexForLowestTimestamp];
    cback(args, indexForLowestTimestamp);

    //
    // Now update our internal state.
    //
    UnregisterFile(indexForLowestTimestamp);
}


