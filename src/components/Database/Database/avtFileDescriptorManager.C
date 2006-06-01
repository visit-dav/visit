/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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


