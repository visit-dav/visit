/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                          avtDataObjectString.C                            //
// ************************************************************************* //

#include <avtDataObjectString.h>

// For NULL
#include <stddef.h>

#include <BadIndexException.h>
#include <ImproperUseException.h>


using std::vector;


// ****************************************************************************
//  Method: avtDataObjectString constructor
//
//  Programmer: Hank Childs
//  Creation:   May 25, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Sep 16 16:06:57 PDT 2001
//    Removed initialization of old data members.
//
// ****************************************************************************

avtDataObjectString::avtDataObjectString()
{
    wholeString = NULL;
}


// ****************************************************************************
//  Method: avtDataObjectString destructor
//
//  Programmer: Hank Childs
//  Creation:   May 25, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Sep 16 16:06:57 PDT 2001
//    Added destruction of character strings that were donated to this object.
//
// ****************************************************************************

avtDataObjectString::~avtDataObjectString()
{
    for (size_t i = 0 ; i < strs.size() ; i++)
    {
        if (ownString[i])
        {
            delete [] strs[i];
        }
    }
    if (wholeString != NULL)
    {
        delete [] wholeString;
        wholeString = NULL;
    }
}


// ****************************************************************************
//  Method: avtDataObjectString
//
//  Purpose:
//      Performs an append.  Since we are just keeping a reference to the
//      string, this is very cheap.
//
//  Arguments:
//      s       The string to append.
//      sN      The length of s.
//      os      Action item type.
//
//  Programmer: Hank Childs
//  Creation:   May 25, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Sep 16 16:06:57 PDT 2001
//    Re-wrote function to reflect new data members.
//
// ****************************************************************************

void
avtDataObjectString::Append(char *s, int sN, APPEND_ACTION_ITEM os)
{
    char *str_to_copy = s;
    bool  do_we_own_string = false;
    switch (os)
    {
      case DATA_OBJECT_STRING_DOES_NOT_OWN_REFERENCE:
        do_we_own_string = false;
        break;

      case DATA_OBJECT_STRING_OWNS_REFERENCE_AFTER_CALL:
        do_we_own_string = true;
        break;

      case DATA_OBJECT_STRING_SHOULD_MAKE_COPY:
        {
            str_to_copy = new char[sN];
            for (int i = 0 ; i < sN ; i++)
            {
                str_to_copy[i] = s[i];
            }
            do_we_own_string = true;
        }
        break;

      default:
        EXCEPTION0(ImproperUseException);
    }
 
    strs.push_back(str_to_copy);
    lens.push_back(sN);
    ownString.push_back(do_we_own_string);

    if (wholeString != NULL)
    {
        delete [] wholeString;
        wholeString = NULL;
    }
}


// ****************************************************************************
//  Method: avtDataObjectString::GetString
//
//  Purpose:
//      Gets one of N strings that make up the total data object string.
//
//  Arguments:
//      n         Which string to get.
//      s         Where the string will be stored.
//      l         Where the length of the string will be stored.
//
//  Programmer:   Hank Childs
//  Creation:     May 26, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Sep 16 16:06:57 PDT 2001
//    Added argument to reflect that this routine only gets a fraction of the
//    total argument.
//
// ****************************************************************************

void
avtDataObjectString::GetString(int n, char *&s, int &l)
{
    if (n < 0 || n >= strs.size())
    {
        EXCEPTION2(BadIndexException, n, strs.size());
    }

    s = strs[n];
    l = lens[n];
}


// ****************************************************************************
//  Method: avtDataObjectString::GetWholeString
//
//  Purpose:
//      Gets the whole data object string.  Note that this is typically not a
//      good thing to do since it doubles the required memory footprint.  This
//      is really only meant to be used by the non-UI processes, who don't want
//      to make multiple MPI calls.
//
//  Arguments:
//      s         Where the string will be stored.
//      l         Where the length of the string will be stored.
//
//  Programmer:   Hank Childs
//  Creation:     September 16, 2001
//
// ****************************************************************************

void
avtDataObjectString::GetWholeString(char *&s, int &l)
{
    if (wholeString)
    {
        s = wholeString;
        l = GetTotalLength();
    }
    else
    {
        int totalLength = GetTotalLength();
        wholeString = new char[totalLength];

        int nstrs  = strs.size();
        int offset = 0;
        for (int i = 0 ; i < nstrs ; i++)
        {
            for (int j = 0 ; j < lens[i] ; j++)
            {
                wholeString[offset] = strs[i][j];
                offset++;
            }
        }

        s = wholeString;
        l = totalLength;
    }
}


// ****************************************************************************
//  Method: avtDataObjectString::GetNStrings
//
//  Purpose:
//      Gets the total number of strings that comprise the total data object
//      string.
//
//  Returns:    The total number of strings that comprise the total data object
//              string.
//
//  Programmer: Hank Childs
//  Creation:   September 16, 2001
//
// ****************************************************************************

int
avtDataObjectString::GetNStrings(void)
{
    return strs.size();
}


// ****************************************************************************
//  Method: avtDataObjectString::GetTotalLength
//
//  Purpose:
//      Gets the length of all of the strings in the data object string.
//
//  Returns:    The length of all of the string in bytes.
//
//  Programmer: Hank Childs
//  Creation:   September 16, 2001
//
// ****************************************************************************

int
avtDataObjectString::GetTotalLength(void)
{
    int rv = 0;
    int nStrings = strs.size();
    for (int i = 0 ; i < nStrings ; i++)
    {
        rv += lens[i];
    }

    return rv;
}


