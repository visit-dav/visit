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
//                             avtSILNamespace.h                             //
// ************************************************************************* //

#ifndef AVT_SIL_NAMESPACE_H
#define AVT_SIL_NAMESPACE_H
#include <dbatts_exports.h>


#include <vector>

class     NamespaceAttributes;


// ****************************************************************************
//  Class: avtSILNamespace
//
//  Purpose:
//      A namespace of the range of a collection.  Its purpose is to try and
//      provide a abstract base type that allows for SILs and other objects to
//      not worry about what the range of a collection is.  Derived types
//      would handle enumerated namespaces (where every subset out of a
//      collection can be enumerated in a non-problem size manner) and
//      range namespaces (where the number of subsets is proportional to the
//      problem size), for example.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Jan  9 15:37:41 PST 2002
//    Add virtual destructor to clean up memory leaks.
//
//    Dave Bremer, Wed Dec 19 12:38:52 PST 2007
//    Added code to query for an element.
// ****************************************************************************

class DBATTS_API avtSILNamespace
{
  public:
    virtual                         ~avtSILNamespace() {;};

    virtual const std::vector<int>  &GetAllElements(void) const = 0;
    virtual NamespaceAttributes     *GetAttributes(void) const = 0;
    virtual bool                     ContainsElement(int e) const = 0;
    static avtSILNamespace          *GetNamespace(const NamespaceAttributes *);
};


#endif


