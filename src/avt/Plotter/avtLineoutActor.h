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

#ifndef LINEOUT_ACTOR_H
#define LINEOUT_ACTOR_H

#include <plotter_exports.h>
#include <visitstream.h>
#include <ref_ptr.h>
#include <string>

class vtkActor;
class vtkLineSource;
class vtkPolyDataMapper;
class vtkRenderer;
class vtkFollower;


// ****************************************************************************
//  Class:  avtLineoutActor
//
//  Purpose:  Responsible for creating the visual cue actors for a Lineout. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun 19 18:05:04 PDT 2002
//    Added SetLineWidth/Style, GetDesignator, and overloade Set Point methods
//    for doubles.
//
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002   
//    Added members 'hidden' and showLabels, methods GetAttachmentPoint, 
//    SetShowLabels.  Added two actors for labels to be positioned at the 
//    endpoints.
//
//    Kathleen Bonnell, Fri Jun  6 15:08:45 PDT 2003 
//    Added Translate and ResetPosition methods. 
//    
//    Kathleen Bonnell, Wed Jun 25 15:16:42 PDT 2003  
//    Changed arguments to SetAttachmentPoint, SetPoint2 from an array to
//    3 values.
//    
// ****************************************************************************

class PLOTTER_API avtLineoutActor
{
  public:
                       avtLineoutActor();
    virtual           ~avtLineoutActor();

    void               Add(vtkRenderer *ren);
    void               Remove();
    void               Hide();
    void               UnHide();

    void               SetAttachmentPoint(double x, double y, double z);
    const double      *GetAttachmentPoint() { return attach; };
    void               SetPoint2(double x, double y, double z);
    void               SetMode3D(const bool);
    void               SetScale(double);
    void               SetDesignator(const std::string &designator_);
    const std::string  GetDesignator() const;
          std::string  GetDesignator();
    void               SetForegroundColor(double fgr, double fgg, double fgb);
    void               SetForegroundColor(double fg[3]);
    void               UpdateView();
    void               Shift(const double vec[3]);
    void               Translate(const double vec[3]);
    void               ResetPosition(void);

    void               SetLineStyle(const int);
    void               SetLineWidth(const int);
    void               SetShowLabels(const bool);

  protected:
    bool               hidden;
    bool               mode3D;
    bool               showLabels;
    double             attach[3];
    double             pt2[3];
    std::string        designator; 
    vtkFollower       *labelActor1;
    vtkFollower       *labelActor2;

    vtkActor          *lineActor;
    vtkLineSource     *lineSource;
    vtkPolyDataMapper *lineMapper;

    vtkRenderer       *renderer; 

};

typedef ref_ptr<avtLineoutActor> avtLineoutActor_p;

#endif
