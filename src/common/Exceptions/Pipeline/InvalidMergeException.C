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
//                            InvalidMergeException.C                        //
// ************************************************************************* //

#include <stdio.h>   // for sprintf
#include <InvalidMergeException.h>


using namespace std;


// ****************************************************************************
//  Method: InvalidMergeException constructor
//
//  Arguments:
//      n1      The first field's integer.
//      n2      The second field's integer.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2000
//
// ****************************************************************************

InvalidMergeException::InvalidMergeException(int n1, int n2)
{
    char  str[1024];
    sprintf(str, "Cannot merge datasets because of an incompatible field "
                 "%d and %d.", n1, n2);
    msg = str;
}


// ****************************************************************************
//  Method: InvalidMergeException constructor
//
//  Arguments:
//      n1      The first field's double precision number.
//      n2      The second field's double precision number.
//
//  Programmer: Hank Childs
//  Creation:   August 4, 2005
//
// ****************************************************************************

InvalidMergeException::InvalidMergeException(double n1, double n2)
{
    char  str[1024];
    sprintf(str, "Cannot merge datasets because of an incompatible field "
                 "%g and %g.", n1, n2);
    msg = str;
}


// ****************************************************************************
//  Method: InvalidMergeException constructor
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2000
//
// ****************************************************************************

InvalidMergeException::InvalidMergeException()
{
    msg = "The two datasets cannot be merged because they overlap.";
}


// ****************************************************************************
//  Method: InvalidMergeException constructor
//
//  Programmer: Hank Childs
//  Creation:   August 29, 2005
//
// ****************************************************************************

InvalidMergeException::InvalidMergeException(const char *m)
{
    msg = m;
}


