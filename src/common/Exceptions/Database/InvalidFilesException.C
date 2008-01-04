/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            InvalidFilesException.C                        //
// ************************************************************************* //

#include <stdio.h>                  // for sprintf
#include <InvalidFilesException.h>

// ****************************************************************************
//  Method: InvalidFilesException constructor
//
//  Arguments:
//      numfiles   The number of files that were too big for the database.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2000
//
// ****************************************************************************

InvalidFilesException::InvalidFilesException(int numfiles)
{
    char str[1024];
    sprintf(str, "This database cannot handle more than %d files.", numfiles);
    msg = str;
}


// ****************************************************************************
//  Method: InvalidFilesException constructor
//
//  Arguments:
//      filename    The name of an invalid file.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Feb 7 11:45:26 PDT 2002
//    Added a little to the error message.
//
//    Hank Childs, Fri Jan 12 09:08:30 PST 2007
//    Remove reference to database, since that is just confusing the issue
//    for users.
//
// ****************************************************************************

InvalidFilesException::InvalidFilesException(const char *filename)
{
    char str[1024];
    sprintf(str, "There was an error opening %s. It may be an invalid file.", 
                 filename);

    msg = str;
}


// ****************************************************************************
//  Method: InvalidFilesException constructor
//
//  Arguments:
//      filename    The name of an invalid file.
//      plugins     A vector of strings of the plugin types used.
//
//  Programmer: Hank Childs
//  Creation:   January 11, 2007
//
// ****************************************************************************

InvalidFilesException::InvalidFilesException(const char *filename,
                                             std::vector<std::string> &plugins)
{
    char str[1024];
    sprintf(str, "There was an error opening %s. It may be an "
            "invalid file.  VisIt tried using the following file format "
            "readers to open the file: ", filename);
    for (int i = 0 ; i < plugins.size() ; i++)
    {
        if (i != plugins.size()-1)
            sprintf(strlen(str)+str, "%s, ", plugins[i].c_str());
        else
            sprintf(strlen(str)+str, "%s", plugins[i].c_str());
    }
    if (plugins.size() == 0)
    {
        sprintf(strlen(str)+str, "<No suitable plugins were identified>");
    }

    msg = str;
}


// ****************************************************************************
//  Method: InvalidFilesException constructor
//
//  Arguments:
//      filename    The name of an invalid file.
//      plugins     A vector of strings of the plugin types used.
//      msg         The message from opening the file.
//
//  Programmer: Hank Childs
//  Creation:   January 12, 2007
//
// ****************************************************************************

InvalidFilesException::InvalidFilesException(const char *filename,
                                             std::vector<std::string> &plugins,
                                             const char *msg2)
{
    char str[1024];
    sprintf(str, "There was an error opening %s. It may be an "
            "invalid file.  VisIt tried using the following file format "
            "readers to open the file: ", filename);
    for (int i = 0 ; i < plugins.size() ; i++)
    {
        if (i != plugins.size()-1)
            sprintf(strlen(str)+str, "%s, ", plugins[i].c_str());
        else
            sprintf(strlen(str)+str, "%s", plugins[i].c_str());
    }
    if (plugins.size() == 0)
    {
        sprintf(strlen(str)+str, "<No suitable plugins were identified>");
    }

    //
    // Only append the plugin thrown by the file format if it gave a useful
    // message.  Otherwise, it is just redundant.
    //
    if (strstr(msg2, "It may be an invalid file") == NULL)
    {
        sprintf(strlen(str)+str, "\n\nThe file format reader issued the "
                    "following error message, which may (or may not) be "
                    "indicative of the problem:\n%s", msg2);
    }

    msg = str;
}


// ****************************************************************************
//  Method: InvalidFilesException constructor
//
//  Arguments:
//      filename    The name of an invalid file.
//      message     A more informative message than the default one.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 11, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Jan 12 09:08:30 PST 2007
//    Remove reference to database, since that is just confusing the issue
//    for users.
//
// ****************************************************************************

InvalidFilesException::InvalidFilesException(const std::string &filename,
                                             const std::string &message)
{
    char str[2048];
    sprintf(str, "There was an error opening %s: %s", filename.c_str(),
            message.c_str());

    msg = str;
}


// ****************************************************************************
//  Method: InvalidFilesException constructor
//
//  Arguments:
//      list     A list of filenames.
//      listN    The number of files in list.
//
//  Programmer:  Hank Childs
//  Creation:    September 13, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Jan 12 09:08:30 PST 2007
//    Remove reference to database, since that is just confusing the issue
//    for users.
//
// ****************************************************************************

InvalidFilesException::InvalidFilesException(const char * const * list, 
                                             int listN)
{
    msg = "There was an error opening one of the following files:\n";
    for (int i = 0 ; i < listN ; i++)
    {
        msg += "\t";
        msg += list[i];
    }
}


