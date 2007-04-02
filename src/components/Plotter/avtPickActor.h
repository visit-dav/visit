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

#ifndef PICK_ACTOR_H
#define PICK_ACTOR_H
#include <plotter_exports.h>
#include <visitstream.h>
#include <ref_ptr.h>

class vtkActor;
class vtkFollower;
class vtkLineSource;
class vtkPolyDataMapper;
class vtkRenderer;
class vtkGlyphSource2D;


// ****************************************************************************
//  Class:  avtPickActor
//
//  Purpose:  Responsible for creating the visual cue actors for a Pick. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed May  8 13:50:44 PDT 2002
//    Moved ComputeScaleFactor to VisWinRendering.
//
//    Kathleen Bonnell, Fri Jun  6 15:08:45 PDT 2003  
//    Added Translate and ResetPosition methods. 
//
//    Kathleen Bonnell, Wed Jun 25 15:16:42 PDT 2003
//    Changed arguments to SetAttachmentPoint from an array to 3 values.
//    Added a glyph for NodePick.
//
//    Kathleen Bonnell, Tue Jun  8 17:42:59 PDT 2004 
//    Added 'GetLetterPosition'. 
//
//    Kathleen Bonnell, Wed Aug 18 09:59:02 PDT 2004 
//    Added 'GetMode3D'. 
//
// ****************************************************************************

class PLOTTER_API avtPickActor
{
  public:
                       avtPickActor();
    virtual           ~avtPickActor();

    void               Add(vtkRenderer *ren);
    void               Remove();
    void               Hide();
    void               UnHide();

    void               SetAttachmentPoint(const double newPos[3]);
    void               SetAttachmentPoint(double x, double y, double z);
    const double *     GetAttachmentPoint() { return attach; };
    const double *     GetLetterPosition(void); 
    void               SetMode3D(const bool);
    const bool         GetMode3D(void) const { return mode3D; };
    void               SetScale(double);
    void               SetDesignator(const char *l);
    void               SetForegroundColor(double fgr, double fgg, double fgb);
    void               SetForegroundColor(double fg[3]);
    void               UpdateView();
    void               Shift(const double vec[3]);
    void               Translate(const double vec[3]);
    void               ResetPosition(const double vec[3]);
    void               UseGlyph(const bool v) { useGlyph = v; } ;

  protected:
    bool               mode3D;
    bool               useGlyph;
    double             attach[3];
    vtkFollower       *letterActor;

    vtkActor          *lineActor;
    vtkLineSource     *lineSource;
    vtkPolyDataMapper *lineMapper;

    vtkFollower       *glyphActor;
    vtkGlyphSource2D  *glyphSource;
    vtkPolyDataMapper *glyphMapper;

    vtkRenderer       *renderer; 

  private:
};

typedef ref_ptr<avtPickActor> avtPickActor_p;

#endif
