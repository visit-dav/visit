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
//                                avtBehavior.h                              //
// ************************************************************************* //

#ifndef AVT_BEHAVIOR_H
#define AVT_BEHAVIOR_H
#include <plotter_exports.h>


#include <ref_ptr.h>

#include <avtDataObjectInformation.h>
#include <avtLegend.h>
#include <PlotInfoAttributes.h>
#include <RenderOrder.h>
#include <VisWindowTypes.h>

class  vtkRenderer;

// ****************************************************************************
//  Class: avtBehavior
//
//  Purpose:
//      This defines the behavior of a plot.  It is all of the things that go
//      into an actor besides the actual drawable.  This always exists on the
//      viewer and exists so that VisIt can give the appearance of being
//      very interactive even when in image-mode.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 12 16:49:27 PST 2001
//    Added shift factor for plots.
//
//    Kathleen Bonnell, Tue Apr  3 15:17:06 PDT 2001 
//    Added render order for plots.
//
//    Kathleen Bonnell, Mon Nov 26 9:16:32 PST 2001 
//    Added hook to retrieve flag specifiying whether points were tranformed.
//
//    Kathleen Bonnell, Tue May  7 09:36:15 PDT 2002 
//    Added GetDataExtents method. 
//
//    Kathleen Bonnell, Wed Mar 26 14:29:23 PST 2003  
//    Renamed "GetTransformedPoints" to "RequiresReExecuteForQuery" to more
//    accurately reflect its purpose. 
//
//    Eric Brugger, Wed Aug 20 09:50:56 PDT 2003
//    Added GetWindowMode.
//
//    Kathleen Bonnell, Mon Sep 29 13:21:12 PDT 2003 
//    Added  SetAntialiasedRenderOrder.  Added bool arg to GetRenderOrder.
//
//    Kathleen Bonnell, Tue Jun  1 17:40:33 PDT 2004 
//    Added bool args to RequriesReExecuteForQuery. 
//
//    Jeremy Meredith, Thu Aug 12 14:13:24 PDT 2004
//    Added a const method for GetInfo.
//
//    Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006
//    Added GetPlotInfoAtts.
//
//    Hank Childs, Fri Aug 31 10:28:10 PDT 2007
//    Added AdaptsToAnyWindowMode.
//
//    Brad Whitlock, Wed Jan  7 14:26:22 PST 2009
//    I modified a plot info atts method.
//
// ****************************************************************************

class PLOTTER_API avtBehavior
{
  public:
                               avtBehavior();
    virtual                   ~avtBehavior();

    void                       Add(vtkRenderer *);
    void                       Remove(vtkRenderer *);
 
    void                       SetInfo(const avtDataObjectInformation &);
    avtDataObjectInformation       &GetInfo(void) { return info; };
    const avtDataObjectInformation &GetInfo(void) const { return info; };

    void                       GetOriginalBounds(double [6]);
    void                       GetActualBounds(double [6]);
    void                       GetDataExtents(double &dmin, double &dmax);

    int                        GetDimension(void);
    WINDOW_MODE                GetWindowMode(void);
    bool                       AdaptsToAnyWindowMode(void);
    bool                       RequiresReExecuteForQuery(const bool,
                                                         const bool);

    void                       SetShiftFactor(double);
    double                     GetShiftFactor(void);

    void                       SetRenderOrder(int);
    void                       SetRenderOrder(RenderOrder);
    void                       SetAntialiasedRenderOrder(int);
    void                       SetAntialiasedRenderOrder(RenderOrder);
    int                        GetRenderOrder(bool antialiased);

    void                       SetLegend(avtLegend_p);
    avtLegend_p                GetLegend(void)       { return legend; };

    const PlotInfoAttributes  &GetPlotInformation() const;
    
  protected:
    avtDataObjectInformation   info;
    vtkRenderer               *renderer;
    avtLegend_p                legend;
    double                     shiftFactor;
    RenderOrder                renderOrder;
    RenderOrder                antialiasedRenderOrder;
};


typedef ref_ptr<avtBehavior>  avtBehavior_p;


#endif


