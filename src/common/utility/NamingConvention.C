/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                            NamingConvention.C                             //
// ************************************************************************* //

#include <ctype.h>
#include <visitstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NamingConvention.h>
#include <Utility.h>


// ****************************************************************************
//  Method: NamingConvention Constructor
//
//  Arguments:
//      list     A list of file names.
//      listN    The number of names in list.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 1999
//
//  Modifications:
//
//    Hank Childs, Thu Jun 15 14:01:28 PDT 2000
//    Added arguments to constructor, copied over file list.
//
// ****************************************************************************

NamingConvention::NamingConvention(const char * const *list, int listN)
{
    int  i;

    fileListN     = listN;
    fileList      = new char*[fileListN];
    for (i = 0 ; i < fileListN ; i++)
    {
        fileList[i] = CXX_strdup(list[i]);
    }

    rootFile  = new bool[fileListN];
    validFile = new bool[fileListN];
    for (i = 0 ; i < fileListN ; i++)
    {
        rootFile[i]  = false;
        validFile[i] = true;
    }

    //
    // Overestimate (by a lot) the number of states.
    //
    stateType  = new int[fileListN];
    stateName  = new char*[fileListN];
    stateTypeN = 0;

    common       = NULL;
}


// ****************************************************************************
//  Method: NamingConvention Destructor
//
//  Programmer: Hank Childs
//  Creation:   December 6, 1999
//
//  Modifications:
//
//    Hank Childs, Thu Jun 15 14:01:28 PDT 2000
//    Added deletion of fileList and root, removed deletion of statePattern.
//
// ****************************************************************************

NamingConvention::~NamingConvention()
{
    if (common != NULL)
    {
        delete [] common;
    }
    if (fileList != NULL)
    {
        for (int i = 0 ; i < fileListN ; i++)
        {
            if (fileList[i] != NULL)
            {
                delete [] fileList[i];
            }
        }
        delete [] fileList;
    }
    if (rootFile != NULL)
    {
        delete [] rootFile;
    }
    if (stateName != NULL)
    {
        for (int i = 0 ; i < stateTypeN ; i++)
        {
            if (stateName[i] != NULL)
            {
                delete [] stateName[i];
            }
        }
        delete [] stateName;
    }
    if (stateType != NULL)
    {
        delete [] stateType;
    }
    if (validFile != NULL)
    {
        delete [] validFile;
    }
}


// ****************************************************************************
//  Method: NamingConvention::DetermineNamingConvention
//
//  Purpose:
//      Determines which derived type of NamingConvention fits the list of 
//      filenames specified.
//
//  Arguments:
//      list     A list of file names.
//      listN    The number of names in list.
//
//  Returns:    A derived type of NamingConvention that is appropriate for the 
//              list of filenames specified, NULL if none could be found.
//
//  Note:       This is a static method.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Aug 30 10:29:58 PDT 2000
//    Added FileSequence.
//
//    Hank Childs, Wed Sep  6 16:23:44 PDT 2000
//    Added Exodus.
//
//    Hank Childs, Mon Sep 11 11:57:20 PDT 2000
//    Remove prepended directory names when trying to determine naming 
//    convention.
//
//    Hank Childs, Wed Sep 13 11:05:33 PDT 2000
//    Added SiloObj.
//
// ****************************************************************************

NamingConvention *
NamingConvention::DetermineNamingConvention(const char * const *list,int listN)
{
    if (listN <= 0)
    {
        cerr << "Naming convention: Invalid number of files: " << listN <<endl;
        return NULL;
    }

    int    i;
    NamingConvention  *code = NULL;

    char **nodir_list = new char*[listN];
    for (i = 0 ; i < listN ; i++)
    {
        const char *p = list[i];
        const char *q = NULL;
        while ( (q = strstr(p, "/")) != NULL)
        {
            //
            // Go past the '/'
            //
            p = q+1;
        }
        nodir_list[i] = CXX_strdup(p);
    }

    if (Ares::IsFormat(nodir_list, listN))
    {
        code = new Ares(nodir_list, list, listN);
    }
    else if (Ale3D::IsFormat(nodir_list, listN))
    {
        code = new Ale3D(nodir_list, list, listN);
    }
    else if (SingleFile::IsFormat(nodir_list, listN))
    {
        code = new SingleFile(nodir_list, list, listN);
    }
    else if (Exodus::IsFormat(nodir_list, listN))
    {
        code = new Exodus(nodir_list, list, listN);
    }
    else if (SiloObj::IsFormat(nodir_list, listN))
    {
        code = new SiloObj(nodir_list, list, listN);
    }
    else if (FileSequence::IsFormat(nodir_list, listN))
    {
        code = new FileSequence(nodir_list, list, listN);
    }

    if (code == NULL)
    {
        cerr << "The file names do not appear to match any of the recognized "
             << "naming conventions (Ares, Ale3D, single file)." << endl;
    }

    for (i = 0 ; i < listN ; i++)
    {
        delete [] nodir_list[i];
    }
    delete [] nodir_list;

    return code;
}


// ****************************************************************************
//  Method: NamingConvention::GetFile
//
//  Purpose:
//      Buffers some of the logic of which file this processor should select.
//
//  Arguments:
//      s            The state index to search for.
//      count        The element of the sequence to return.
//
//  Returns:  The filename corresponding to (s, count).
//
//  Programmer: Hank Childs
//  Creation:   December 3, 1999
//
//  Modifications:
//
//    Hank Childs, Thu Jun 15 14:01:28 PDT 2000
//    Removed arguments for the list of files and its size since that is now
//    copied by the object.
//
//    Hank Childs, Wed Sep  6 16:51:21 PDT 2000
//    Added support for a file being in multiple states.
//
// ****************************************************************************

char *
NamingConvention::GetFile(int s, int count)
{
    //
    // Look through all of the files and find the count'th one that has the 
    // same state.  
    //
    int   numMatches = 0;
    for (int i = 0 ; i < fileListN ; i++)
    {
        if (stateType[i] == s || stateType[i] == -1)
        {
            if (validFile[i])
            {
                numMatches++;
            }
            if ((numMatches-1) == count)
            {
                return fileList[i];
            }
        }
    }

    return NULL;
}


// ****************************************************************************
//  Method: NamingConvention::GetRootFile
//
//  Purpose:
//      Returns the root files in sequence.
//
//  Arguments:
//      s            The state index to search for.
//
//  Returns:  The root filename corresponding to s.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 1999
//
//  Modifications:
//
//    Hank Childs, Thu Jun 15 14:01:28 PDT 2000
//    Removed arguments for the list of files and its size since that is now
//    stored with the object.
//
// ****************************************************************************

char *
NamingConvention::GetRootFile(int s)
{
    //
    // Every processor reads every root file.
    //
    for (int i = 0 ; i < fileListN ; i++)
    {
        if (stateType[i] == s && rootFile[i] == true)
        {
            return fileList[i];
        }
    }

    //
    // There should be a root index for every state.  This is an error.
    //
    cerr << "Could not determine the " << s << "th root file." << endl;
    exit(EXIT_FAILURE);

    /* NOTREACHED */
    return NULL;
}


// ****************************************************************************
//  Method: NamingConvention::GetState
//
//  Purpose:
//      Gets the number associated with the state.
//
//  Arguments:
//      state    A place to put the state.
//      num      The index of the state of interest.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2000
//
// ****************************************************************************

void
NamingConvention::GetState(char *state, int num)
{
    if (num < 0 || num > stateTypeN)
    {
        strcpy(state, "BADSTATE");
    }
    else
    {
        strcpy(state, stateName[num]);
    }
}


// ****************************************************************************
//  Method: NamingConvention::AssignState
//
//  Purpose:
//      Assigns the specified file with the state in string form.  Determines
//      if that state is new or one already encountered.
//
//  Arguments:
//      f       The index of the file.
//      s       The state as a string.
//      all     f belongs to all states.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2000
//
//  Modifications:
//    
//    Hank Childs, Wed Sep  6 16:51:21 PDT 2000
//    Added logic for files that belong to all states.
//
// ****************************************************************************

void
NamingConvention::AssignState(int f, const char *s, bool all)
{
    int  state = -1;
    if (! all)
    {
        //
        // See if this matches any of the states we have seen before.
        //
        for (int j = 0 ; j < stateTypeN ; j++)
        {
            if (strcmp(s, stateName[j]) == 0)
            {
                state = j;
                break;
            }
        }
                 
        if (state == -1)
        {
            state = stateTypeN;
    
            //
            // We didn't find the state, so create it.
            //
            stateName[stateTypeN] = CXX_strdup(s);
            stateTypeN++;
        }
    }

    stateType[f] = state;
}


// ****************************************************************************
//  Method: NamingConvention::PrintSelf
//
//  Purpose:
//      Prints out the NamingConvention object.  Meant for debugging.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2000
//
// ****************************************************************************

void
NamingConvention::PrintSelf(void)
{
    int  i;
    cerr << "Common prefix = " << common << endl;
    cerr << "Number of states = " << stateTypeN << endl;
    cerr << "States: " << endl;
    for (i = 0 ; i < stateTypeN ; i++)
    {
        cerr << "\t" << stateName[i] << endl;
    }
    cerr << "Number of files = " << fileListN << endl;
    cerr << "Files: " << endl;
    for (i = 0 ; i < fileListN ; i++)
    {
        cerr << "\t" << fileList[i];
        cerr << "\t state = " << stateType[i];
        cerr << "\t root = " << (rootFile[i] ? "true" : "false");
        cerr << "\t valid = " << (validFile[i] ? "true" : "false");
        cerr << endl;
    }
}


// ****************************************************************************
//  Method: Ale3D::IsFormat
//
//  Purpose:
//      Determines if the list of filenames belongs to the Ale3D naming
//      convention.  The Ale3D naming convention is XXXYYY[.Z] where XXX
//      is the name, YYY is the state, and Z is the file within the state.
//      .Z is not specified for the root files (which also contain domains),
//      but is specified for all other files in the state.
//
//  Arguments:
//      list     A list of file names.
//      listN    The number of names in list.
//
//  Returns:    true if the file names follow the Ale3D format, false 
//              otherwise.
//
//  Note:       This is a static method.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2000
//
// ****************************************************************************

bool
Ale3D::IsFormat(const char * const *list, int listN)
{
    int  length = LongestCommonPrefixLength(list, listN);
    if (length <= 0)
    {
        return false;
    }

    //
    // If only one state is passed in, then the prefix will not differ at the
    // state.  If this is the case, back up to make sure we don't miss the
    // state.
    //
    while (length > 0 && isdigit(list[0][length-1]))
    {
        --length;
    }
    if (length <= 0)
    {
        //
        // It's all numbers!
        //
        return false;
    }

    for (int i = 0 ; i < listN ; i++)
    {
        //
        // Make sure that there are some numbers for the state after the prefix.
        //
        int  state = length;
        while (isdigit(list[i][state]))
        {
            state++;
        }
        if (state == length)
        {
            //
            // No state - this isn't Ale3D.
            //
            return false;
        }

        //
        // Make sure the only thing trailing now is nothing or a `.'number
        //
        if (list[i][state] == '\0')
        {
            //
            // A root file -- this is ok.
            //
            continue;
        }

        if (list[i][state] != '.')
        {
            //
            // The only acceptable character after the state is a `.'.
            //
            return false;
        }

        int dot = state+1;
        int afterDot = dot;
        while (isdigit(list[i][afterDot]))
        {
            afterDot++;
        }

        if (dot == afterDot)
        {
            //
            // There was nothing following the `.'
            //
            return false;
        }
       
        if (list[i][afterDot] != '\0')
        {
            //
            // There are some invalid chars after the dot.
            //
            return false;
        }
    }

    //
    // It has passed all of our tests, so this is good enough.
    //
    return true;
}
    

// ****************************************************************************
//  Method: Ale3D constructor
//
//  Arguments:
//      list      A list of file names.
//      dirList   A list of file names with their prepended directories.
//      listN     The number of file names in list.
//
//  Note:       This constructor will only be called after IsFormat has
//              verified that it is the correct NamingConvention to use, so 
//              there does not need to be a lot of error checking.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2000
//
// ****************************************************************************

Ale3D::Ale3D(const char * const *list, const char * const *dirList, int listN) 
    : NamingConvention(dirList, listN)
{
    char  tmp[1024];

    int   length = LongestCommonPrefixLength(list, listN);
    
    //
    // If only one state is passed in, then the prefix will not differ at the
    // state.  If this is the case, back up to make sure we don't miss the
    // state.
    //
    while (length > 0 && isdigit(list[0][length-1]))
    {
        --length;
    }

    strncpy(tmp, list[0], length);
    tmp[length] = '\0';
    common = CXX_strdup(tmp);

    for (int i = 0 ; i < listN ; i++)
    {
        int endOfState = length;
        while (isdigit(list[i][endOfState]))
        {
            endOfState++;
        }
 
        strncpy(tmp, list[i] + length, endOfState-length);
        tmp[endOfState-length] = '\0';

        AssignState(i, tmp);

        if (list[i][endOfState] == '\0')
        {
            //
            // No `.'number, so this is a root.
            //
            rootFile[i] = true;
        }
        else
        {
            rootFile[i] = false;
        }

        //
        // All Ale3D files are valid.
        //
        validFile[i] = true;
    }
}


// ****************************************************************************
//  Method: Ares::IsFormat
//
//  Purpose:
//      Checks to see if the filenames are following the Ares naming
//      convention.  The Ares format is defined to be: XXX-YYY-ZZZ.silo, where
//      XXX is a prefix, YYY is a file within the state, and ZZZ is the state.
//      Root files are not valid files and they have the form XXXZZZ.root.
// 
//  Arguments:
//      list        A list of file names.
//      listSize    The number of names in list.
//
//  Returns: true if it does follow the convention, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   December 3, 1999
//
//  Modifications:
//
//    Hank Childs, Thu Jun 15 15:47:57 PDT 2000
//    Re-wrote to work better with the changes in base type NamingConvention.
//
// ****************************************************************************

bool
Ares::IsFormat(const char * const *list, int listSize)
{
    int length = LongestCommonPrefixLength(list, listSize);

    //
    // If only one state is passed in, then the prefix will not differ at the
    // state.  If this is the case, back up to make sure we don't miss the
    // state.
    //
    while (length > 0 && 
           (isdigit(list[0][length-1]) || list[0][length-1] == '-'))
    {
        --length;
    }

    if (length <= 0)
    {
        return false;
    }
    
    for (int i = 0 ; i < listSize ; i++)
    {
        if (strstr(list[i], ".root") != NULL)
        {
            //
            // Root files follow a different format, so ignore them.
            //
            continue;
        }

        int firstHyphen = length;
        if (list[i][firstHyphen] != '-')
        {
            return false;
        }

        int fileNum = firstHyphen + 1;
        while (isdigit(list[i][fileNum]))
        {
            fileNum++;
        }

        if (fileNum == (firstHyphen+1))
        {
            //
            // There were no digits following the hyphen.
            //
            return false;
        }

        int secondHyphen = fileNum;
        if (list[i][secondHyphen] != '-')
        {
            return false;
        }

        int state = secondHyphen + 1;
        while (isdigit(list[i][state]))
        {
            state++;
        }

        if (state == (secondHyphen+1))
        {
            //
            // There was no state following the hyphen.
            //
            return false;
        }

        if (list[i][state] != '\0' && list[i][state] != '.')
        {
            //
            // There should be a ".silo" or nothing following this.
            //
            return false;
        }
    }
 
    //
    // Every string is either a root file or it has the form XXX-YYY-ZZZ[.*]
    //
    return true;
}


// ****************************************************************************
//  Method: Ares constructor
//
//  Arguments:
//      list      A list of file names.
//      dirList   A list of file names with their prepended directories.
//      listN     The number of file names in list.
//
//  Note:       This constructor will only be called after IsFormat has
//              verified that it is the correct NamingConvention to use, so 
//              there does not need to be a lot of error checking.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2000
//
// ****************************************************************************

Ares::Ares(const char * const *list, const char * const *dirList, int listN) 
    : NamingConvention(dirList, listN)
{
    char  tmp[1024];

    int   length = LongestCommonPrefixLength(list, listN);
    
    //
    // If only one state is passed in, then the prefix will not differ at the
    // state.  If this is the case, back up to make sure we don't miss the
    // state.
    //
    while (length > 0 && 
           (isdigit(list[0][length-1]) || list[0][length-1] == '-'))
    {
        --length;
    }

    strncpy(tmp, list[0], length);
    tmp[length] = '\0';
    common = CXX_strdup(tmp);

    for (int i = 0 ; i < listN ; i++)
    {
        if (strstr(list[i], ".root") != NULL)
        {
            //
            // This is a root file.
            //
            rootFile[i]  = true;
            validFile[i] = false;

            //
            // The state is everything after the prefix and before the ".root".
            //
            char *root = strstr(list[i], ".root");
            int   stateLength = root - (list[i] + length);
            strncpy(tmp, list[i]+length, stateLength);
            tmp[stateLength] = '\0';
            AssignState(i, tmp);
        }
        else
        {
            //
            // This is a normal file.
            //
            rootFile[i]  = false;
            validFile[i] = true;
               
            char *firstHyphen  = strstr(list[i], "-");
            char *secondHyphen = strstr(firstHyphen+1, "-");
            char *state = secondHyphen+1;
            const char *end = strstr(state, ".");
            if (end == NULL)
            {
                end = list[i] + strlen(list[i]);
            }
            int stateLength = end - state;
            strncpy(tmp, state, stateLength);
            tmp[stateLength] = '\0';
            AssignState(i, tmp);
        }
    }
}


// ****************************************************************************
//  Method: SingleFile::IsFormat
//
//  Purpose:
//      Checks to see if there is only one file.  If so, this qualifies
//      as being SingleFile format.
//
//  Arguments:
//      unnamed     A list of file names.
//      listSize    The number of names in list.
//
//  Returns:    true if it does follow the convention, false otherwise.
//
//  Note:       This is a static method.
//
//  Programmer: Hank Childs
//  Creation:   December 19, 1999
//
//  Modifications:
//
//    Hank Childs, Thu Jun 15 16:19:06 PDT 2000
//    Simplified logic.
//
// ****************************************************************************

bool
SingleFile::IsFormat(const char * const *, int listSize)
{
    return (listSize == 1 ? true : false);
}


// ****************************************************************************
//  Method: SingleFile constructor
//
//  Arguments:
//      list      A list of file names.
//      dirList   A list of file names with their prepended directories.
//      listN     The number of file names in list.
//
//  Note:       This constructor will only be called after IsFormat has
//              verified that it is the correct NamingConvention to use, 
//              so there does not need to be a lot of error checking.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2000
//
// ****************************************************************************

SingleFile::SingleFile(const char * const *list, const char * const *dirList, 
                       int listN) 
    : NamingConvention(dirList, listN)
{
    //
    // We have a whole bunch of machinery that doesn't really apply to a
    // single file, so just put in some values that will allow it to function
    // like any other NamingConvention object.
    //
    common = CXX_strdup(list[0]);
    rootFile[0]  = true;
    validFile[0] = true;
    AssignState(0, "0000");
}


// ****************************************************************************
//  Method: FileSequence::IsFormat
//
//  Purpose:
//      Checks to see if it can figure out the naming convention of the file
//      sequence.  It accepts any sequence of the format 
//      <prefix><state><suffix>  where one of prefix or suffix is a non-zero
//      length string and state varies for each file.
// 
//  Arguments:
//      list        A list of file names.
//      listSize    The number of names in list.
//
//  Returns: true if it does follow the convention, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2000
//
// ****************************************************************************

bool
FileSequence::IsFormat(const char * const *list, int listSize)
{
    int  prefix = LongestCommonPrefixLength(list, listSize);
    int  suffix = LongestCommonSuffixLength(list, listSize);

    //
    // Remove numbers as part of the common prefix.
    //
    while (prefix > 0 && isdigit(list[0][prefix-1]))
    {
        prefix--;
    }

    if (prefix <= 0 && suffix <= 0)
    {
        //
        // This is just a bunch of random strings.
        //
        return false;
    }

    //
    // Make sure that every character not part of the prefix or suffix is
    // a digit.
    //
    for (int i = 0 ; i < listSize ; i++)
    {
        int  length = strlen(list[i]);
        for (int j = prefix ; j < length-suffix ; j++)
        {
            if (! isdigit(list[i][j]))
            {
                return false;
            }
        }
    }

    //
    // The strings all have a common prefix or common suffix (or both) and
    // every character not part of the prefix or suffix is a number.  This fits
    // our criteria.
    //
    return true;
}


// ****************************************************************************
//  Method: FileSequence constructor
//
//  Arguments:
//      list      A list of file names.
//      dirList   A list of file names with their prepended directories.
//      listN     The number of file names in list.
//
//  Note:         This constructor will only be called after IsFormat has
//                verified that it is the correct NamingConvention to use, so 
//                there does not need to be a lot of error checking.
//
//  Programmer:   Hank Childs
//  Creation:     August 30, 2000
//
// ****************************************************************************

FileSequence::FileSequence(const char * const *list, 
                           const char * const *dirList, int listN) 
    : NamingConvention(dirList, listN)
{
    char  tmp[1024];

    int  prefix = LongestCommonPrefixLength(list, listN);
    int  suffix = LongestCommonSuffixLength(list, listN);
    
    //
    // Remove numbers as part of the common prefix.
    //
    while (prefix > 0 && isdigit(list[0][prefix-1]))
    {
        prefix--;
    }

    //
    // Make the common stem be the prefix (since the suffix will normally be
    // .silo or the like) unless there is no prefix, in which case use the
    // suffix.
    //
    if (prefix > 0)
    {
        strncpy(tmp, list[0], prefix);
        tmp[prefix] = '\0';
        common = CXX_strdup(tmp);
    }
    else
    {
        int  length = strlen(list[0]);
        common = CXX_strdup(list[0]+length-suffix);
    }

    for (int i = 0 ; i < listN ; i++)
    {
        rootFile[i]  = true;
        validFile[i] = true;
        int  length = strlen(list[i]);
        for (int j = prefix ; j < length-suffix ; j++)
        {
            tmp[j-prefix] = list[i][j];
        }
        tmp[length-suffix-prefix] = '\0';
        AssignState(i, tmp);
    }
}


// ****************************************************************************
//  Method: Exodus::IsFormat
//
//  Purpose:
//      Determines if the file list follows the Exodus naming convention.
//      The Exodus naming convention is defined to be <prefix>.<cycle>.root and
//      <prefix>.<numblocks>.<block>.silo.
//
//  Arguments:
//      list        A list of file names.
//      listSize    The number of names in list.
//
//  Returns: true if it does follow the convention, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   September 6, 2000
//
// ****************************************************************************

bool
Exodus::IsFormat(const char * const *list, int listSize)
{
    int  prefix = LongestCommonPrefixLength(list, listSize);

    if (prefix <= 1)
    {
        return false;
    }

    if (list[0][prefix-1] != '.')
    {
        //
        // It must be <prefix>.something
        //
        return false;
    }

    for (int i = 0 ; i < listSize ; i++)
    {
        char *suffix = strstr(list[i], ".root");
        if (suffix != NULL)
        {
            //
            // This is a root file.  Confirm that everything that is not
            // prefix and not .root is a number.
            //
            for (const char *q = list[i] + prefix ; q < suffix ; q++)
            {
                if (! isdigit(*q))
                {
                    return false;
                }
            }
        }
        else
        {
            suffix = strstr(list[i], ".silo");
            if (suffix == NULL)
            {
                //
                // Does not have suffix .silo or .root, so this does not
                // follow Exodus naming convention.
                //
                return false;
            }

            //
            // This is a state file.
            //
            bool  foundDot = false;
            for (const char *q = list[i] + prefix ; q < suffix ; q++)
            {
                if (! isdigit(*q))
                {
                    if (*q == '.')
                    {
                        if (foundDot)
                        {
                            //
                            // Two dots -- not okay.
                            //
                            return false;
                        }
                        foundDot = true;
                    }
                    else
                    {
                        //
                        // Digits and '.' are the only acceptable characters.
                        //
                        return false;
                    }
                }
            }
            if (! foundDot)
            {
                //
                // It must have exactly one dot.
                //
                return false;
            }
        }
    }

    return true;
}


// ****************************************************************************
//  Method: Exodus constructor
//
//  Arguments:
//      list      A list of file names.
//      dirList   A list of file names with their prepended directories.
//      listN     The number of file names in list.
//
//  Note:         This constructor will only be called after IsFormat has
//                verified that it is the correct NamingConvention to use, so 
//                there does not need to be a lot of error checking.
//
//  Programmer:   Hank Childs
//  Creation:     September 6, 2000
//
// ****************************************************************************

Exodus::Exodus(const char * const *list, const char * const *dirList,int listN) 
    : NamingConvention(dirList, listN)
{
    int   prefix = LongestCommonPrefixLength(list, listN);
    char  tmp[256];
 
    //
    // Use prefix-1 to avoid the trailing `.'
    //
    strncpy(tmp, list[0], prefix-1);
    tmp[prefix-1] = '\0';
    common = CXX_strdup(tmp);

    for (int i = 0 ; i < listN ; i++)
    {
        char *suffix = strstr(list[i], ".root");
        if (suffix != NULL)
        {
            rootFile[i]  = true;
            validFile[i] = true;

            int   count = 0;
            for (const char *p = list[i] + prefix ; p < suffix ; p++)
            {
                tmp[count++] = *p;
            }
            tmp[count] = '\0';
    
            AssignState(i, tmp, false);
        }
        else
        {
            rootFile[i]  = false;
            validFile[i] = true;
            AssignState(i, NULL, true);
        }
    }
}


// ****************************************************************************
//  Method: SiloObj::IsFormat
//
//  Purpose:
//      Determines if the file list follows the SiloObj naming convention.
//      The SiloObj naming convention is defined to be:
//      <prefix>.visit
//      <prefix>.mesh.##
//      <prefix>###.##
//
//  Arguments:
//      list        A list of file names.
//      listSize    The number of names in list.
//
//  Returns: true if it does follow the convention, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   September 13, 2000
//
// ****************************************************************************

bool
SiloObj::IsFormat(const char * const *list, int listSize)
{
    int  prefix = LongestCommonPrefixLength(list, listSize);
    
    if (prefix <= 0)
    {
        return false;
    }

    char prefixString[1024];
    strncpy(prefixString, list[0], prefix);
    prefixString[prefix] = '\0';

    for (int i = 0 ; i < listSize ; i++)
    {
        char *visit = strstr(list[i] + prefix, ".visit");
        if (visit != NULL)
        {
            char str[1024];
            sprintf(str, "%s.visit", prefixString);
            if (strcmp(str, list[i]) != 0)
            {
                return false;
            }
        }
        else
        {
            char *mesh = strstr(list[i]+prefix, ".mesh");
            if (mesh != NULL)
            {
                char str[1024];
                sprintf(str, "%s.mesh.", prefixString);
                if (strncmp(str, list[i], prefix+strlen(".mesh.")) != 0)
                {
                    return false;
                }
                const char *p = list[i] + prefix + strlen(".mesh.");
                for ( ; *p != '\0' ; p++)
                {
                    if (! isdigit(*p))
                    {
                        return false;
                    }
                }
            }
            else
            {
                //
                // This is a state file -- <prefix>###.##
                //
                bool    foundDot = false;
                const char   *p = list[i] + prefix;
                for ( ; *p != '\0' ; p++)
                {
                    if (*p == '.')
                    {
                        if (foundDot == true)
                        {
                            //
                            // Two dots, this is not okay.
                            //
                            return false;
                        }
                        foundDot = true;
                    }
                    else if (! isdigit(*p))
                    {
                        return false;
                    }
                }
            }
        }
    }

    //
    // It has met all of our tests, so this is SiloObj.
    //
    return true;
}


// ****************************************************************************
//  Method: SiloObj constructor
//
//  Arguments:
//      list      A list of file names.
//      dirList   A list of file names with their prepended directories.
//      listN     The number of file names in list.
//
//  Note:         This constructor will only be called after IsFormat has
//                verified that it is the correct NamingConvention to use, so 
//                there does not need to be a lot of error checking.
//
//  Programmer:   Hank Childs
//  Creation:     September 13, 2000
//
// ****************************************************************************

SiloObj::SiloObj(const char * const *list, const char * const *dirList,
                 int listN) 
    : NamingConvention(dirList, listN)
{
    int   prefix = LongestCommonPrefixLength(list, listN);
 
    char tmp[256];
    strncpy(tmp, list[0], prefix);
    tmp[prefix] = '\0';
    common = CXX_strdup(tmp);
    
    for (int i = 0 ; i < listN ; i++)
    {
        if (strstr(list[i], ".visit"))
        {
            rootFile[i]  = false;
            validFile[i] = false;
            AssignState(i, NULL, true);
        }
        else if (strstr(list[i], ".mesh"))
        {
            rootFile[i]  = true;
            validFile[i] = true;
            AssignState(i, NULL, true);
        }
        else
        {
            char state[256];
            const char *p = list[i]+prefix;
            int count = 0;
            for ( ; *p != '.' ; p++)
            {
                state[count++] = *p;
            }
            state[count++] = '\0';

            // Go past the '.'
            p++;

            bool  isRoot = true;
            for ( ; *p != '\0' ; p++)
            {
                if (*p != '0')
                {
                    isRoot = false;
                    break;
                }
            }
 
            rootFile[i]   = isRoot;
            validFile[i]  = true;
            AssignState(i, state);
        }
    }
}


