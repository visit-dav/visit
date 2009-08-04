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
//                                 avtViewCurve.h                            //
// ************************************************************************* //

#ifndef AVT_VIEW_CURVE_H
#define AVT_VIEW_CURVE_H
#include <view_exports.h>
#include <enumtypes.h>

struct avtViewInfo;
class ViewCurveAttributes;

// ****************************************************************************
//  Class: avtViewCurve
//
//  Purpose:
//    Contains the information for a Curve view.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 30, 2002 
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:40:27 PDT 2003
//    I removed yScale and replaced window with domain and range.  I
//    replaced SetFromViewAttributes with SetFromViewCurveAttributes and
//    SetToViewAttributes with SetToViewCurveAttributes.  I added window
//    size arguments to SetViewFromViewInfo and SetViewInfoFromView.
//
//    Eric Brugger, Fri Oct 10 12:42:27 PDT 2003
//    I deleted SetViewportFromView.  I added GetScaleFactor and
//    GetValidDomainRange.
//
//    Mark C. Miller, Tue Nov  4 14:04:02 PST 2003
//    Added GetViewport method
//
//    Eric Brugger, Tue Nov 18 09:20:54 PST 2003
//    I deleted SetViewFromViewInfo.  I replaced GetValidDomainRange
//    with CheckAndCorrectDomainRange.
//
//    Kathleen Bonnell, Thu Mar 22 20:12:04 PDT 2007 
//    Added domainScale, rangeScale. 
//
//    Kathleen Bonnell, Fri May 11 09:20:06 PDT 2007 
//    Added havePerformedLogDomain/Range.
//
//    Mark Blair, Mon Jul 16 17:16:29 PDT 2007
//    Added SetViewport.
//
// ****************************************************************************

struct AVTVIEW_API avtViewCurve
{
    double    viewport[4];
    double    domain[2];
    double    range[2];
    ScaleMode domainScale;
    ScaleMode rangeScale;
    bool      havePerformedLogDomain;
    bool      havePerformedLogRange;

  public:
                    avtViewCurve();
    avtViewCurve  & operator=(const avtViewCurve &);
    bool            operator==(const avtViewCurve &);
    void            SetToDefault(void);
    void            SetViewInfoFromView(avtViewInfo &, int *);
    void            SetViewport(double *newViewport);

    void            GetViewport(double *) const;
    double          GetScaleFactor(int *);

    void            SetFromViewCurveAttributes(const ViewCurveAttributes *);
    void            SetToViewCurveAttributes(ViewCurveAttributes *) const;

  protected:
    void            CheckAndCorrectDomainRange();
};


#endif

