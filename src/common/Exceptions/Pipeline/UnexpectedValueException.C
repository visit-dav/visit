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
//                       UnexpectedValueException.C                          //
// ************************************************************************* //
#include <UnexpectedValueException.h>

// for sprintf
#include <stdio.h>

using std::string;
 
// ****************************************************************************
//  Method: UnexpectedValueException constructor
//
//  Purpose: handle unexpected value exception for different types
//
//  Programmer: Mark C. Miller
//  Creation:   December 9, 2003
//
//  Modifications
//
//    Mark C. Miller, Mon Nov  5 16:50:10 PST 2007
//    Fixed bonehead error where interpretation of bad/expected was reversed.
//
// ****************************************************************************
UnexpectedValueException::UnexpectedValueException(int expVal, int badVal)
{
    char str[1024];
    sprintf(str, "Expected %d, Got %d", expVal, badVal);
    msg = str;
}
 
UnexpectedValueException::UnexpectedValueException(double expVal, double badVal)
{
    char str[1024];
    sprintf(str, "Expected %f, Got %f", expVal, badVal);
    msg = str;
}
 
UnexpectedValueException::UnexpectedValueException(string expVal, int badVal)
{
    char str[1024];
    sprintf(str, "Expected %s, Got %d", expVal.c_str(), badVal);
    msg = str;
}
 
UnexpectedValueException::UnexpectedValueException(string expVal, double badVal)
{
    char str[1024];
    sprintf(str, "Expected %s, Got %f", expVal.c_str(), badVal);
    msg = str;
}
 
UnexpectedValueException::UnexpectedValueException(string expVal, string badVal)
{
    char str[1024];
    sprintf(str, "Expected %s, Got %s", expVal.c_str(), badVal.c_str());
    msg = str;
}
