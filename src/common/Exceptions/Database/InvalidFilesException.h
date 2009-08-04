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
//                          InvalidFilesException.h                          //
// ************************************************************************* //

#ifndef INVALID_FILES_EXCEPTION_H
#define INVALID_FILES_EXCEPTION_H

#include <avtexception_exports.h>

#include <DatabaseException.h>

#include <string>
#include <vector>


// ****************************************************************************
//  Class: InvalidFilesException
//
//  Purpose:
//      The exception thrown when invalid files are encountered.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Sep 13 20:01:24 PDT 2000
//    Added a constructor with a file list.
//
//    Jeremy Meredith, Wed Aug 11 16:27:32 PDT 2004
//    Adding a new constructor that allows a more informative message.
//
//    Hank Childs, Thu Jan 11 15:49:54 PST 2007
//    Add a new constructor for the plugin types tried.
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidFilesException : public DatabaseException
{
  public:
                          InvalidFilesException(const char *);
                          InvalidFilesException(const char *, 
                                                std::vector<std::string> &);
                          InvalidFilesException(const char *, 
                                                std::vector<std::string> &,
                                                const char *);
                          InvalidFilesException(const char * const *, int);
                          InvalidFilesException(const std::string&,
                                                           const std::string&);
                          InvalidFilesException(int);
    virtual              ~InvalidFilesException() VISIT_THROW_NOTHING {;};
};


#endif


