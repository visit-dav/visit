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
//                   avtTotalSurfaceAreaQuery.h                              //
// ************************************************************************* //

#ifndef AVT_TOTALSURFACEAREA_QUERY_H
#define AVT_TOTALSURFACEAREA_QUERY_H
#include <query_exports.h>

#include <avtSummationQuery.h>


class     avtFacelistFilter;
class     avtVMetricArea;


// ****************************************************************************
//  Class: avtTotalSurfaceAreaQuery
//
//  Purpose:
//      A query for TotalSurfaceArea.
//
//  Notes:
//    Taken mostly from Hank Childs' avtTotalSurfaceAreaFilter and reworked to
//    fit into the Query hierarchy.  avtTotalSurfaceAreaFilter is now deprecated.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 23 15:11:44 PDT 2002 
//    Added VerifyInput.
//
// ****************************************************************************

class QUERY_API avtTotalSurfaceAreaQuery : public avtSummationQuery
{
  public:
                         avtTotalSurfaceAreaQuery();
    virtual             ~avtTotalSurfaceAreaQuery();

    virtual const char  *GetType(void)  { return "avtTotalSurfaceAreaQuery"; };
    virtual const char  *GetDescription(void)
                             { return "TotalSurfaceArea"; };

  protected:
    virtual void               VerifyInput(void); 
    virtual avtDataObject_p    ApplyFilters(avtDataObject_p);
    avtVMetricArea            *area;
    avtFacelistFilter         *facelist;

    virtual int                GetNFilters() { return 2; };

};


#endif


