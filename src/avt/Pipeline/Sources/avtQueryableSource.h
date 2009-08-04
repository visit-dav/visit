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
//                             avtQueryableSourcy.h                          //
// ************************************************************************* //

#ifndef AVT_QUERYABLE_SOURCE_H
#define AVT_QUERYABLE_SOURCE_H
#include <pipeline_exports.h>

#include <avtDataObjectSource.h>

class PickAttributes;


// ****************************************************************************
//  Method: avtQueryableSource
//
//  Purpose:
//      A queryable source is one that is queryable.  All terminating sources
//      are considered queryable, ensuring that every pipeline has a
//      queryable source.
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2003
//
//  Modifications:
//    Kathleen Bonnell, Wed Nov 12 18:26:21 PST 2003
//    Add virtual method 'FindElementForPoint'.
//
//    Kathleen Bonnell, Mon Dec 22 14:48:57 PST 2003 
//    Add virtual method 'GetDomainName'.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Kathleen Bonnell, Tue May 25 16:16:25 PDT 2004 
//    Add virtual method 'QueryZoneCenter'.
//
//    Kathleen Bonnell, Thu Jun 10 18:31:22 PDT 2004 
//    Renamed QueryZoneCenter to QueryCoords, added bool arg.
//
//    Kathleen Bonnell, Thu Dec 16 17:16:33 PST 2004 
//    Added another bool arg to QueryCoords. 
//
//    Kathleen Bonnell, Tue Jan 25 07:59:28 PST 2005 
//    Added const char *arg to QueryCoords. 
//
// ****************************************************************************

class PIPELINE_API avtQueryableSource : virtual public avtDataObjectSource
{
  public:
                                  avtQueryableSource();
    virtual                      ~avtQueryableSource();

    virtual avtQueryableSource   *GetQueryableSource(void) { return this; };
    virtual void                  Query(PickAttributes *) = 0;
    virtual bool                  FindElementForPoint(const char*, const int, 
                                    const int, const char*, double[3], int &)=0;
    virtual void                  GetDomainName(const std::string&, const int, 
                                    const int, std::string&)=0;
    virtual bool                  QueryCoords(const std::string&, const int, 
                                    const int, const int, double[3], 
                                    const bool, const bool, const char *mn=NULL)=0;
};


#endif


