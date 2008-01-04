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

#ifndef AVT_LINE2D_COLLEAGUE_H
#define AVT_LINE2D_COLLEAGUE_H

#include <ColorAttribute.h>
#include <avtAnnotationColleague.h>
#include <viswindow_exports.h>

class vtkActor2D;
class vtkAppendPolyData;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataMapper2D;

// ****************************************************************************
// Class: avtLine2DColleague
//
// Purpose:
//   This colleague is a line that can be shown in the vis window.
//
// Notes:      
//
// Programmer: John C. Anderson
// Creation:   Mon Jul 12 15:48:58 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Jun 28 15:39:27 PST 2005
//   Changed so it does not use a cone source.
//
// ****************************************************************************

class VISWINDOW_API avtLine2DColleague : public avtAnnotationColleague
{
public:
    avtLine2DColleague(VisWindowColleagueProxy &);
    virtual ~avtLine2DColleague();

    virtual void AddToRenderer();
    virtual void RemoveFromRenderer();
    virtual void Hide();

    virtual std::string TypeName() const { return "Line2D"; }

    // Methods to set and get the annotation's properties.
    virtual void SetOptions(const AnnotationObject &annot);
    virtual void GetOptions(AnnotationObject &annot);

    // Methods that are called in response to vis window events.
    virtual void HasPlots(void);
    virtual void NoPlots(void);

protected:
    vtkActor2D          *actor;
    vtkPolyDataMapper2D *mapper;
    vtkAppendPolyData   *allData;
    vtkPolyData         *lineData;
    vtkPolyData         *beginArrowSolid;
    vtkPolyData         *endArrowSolid;
    vtkPolyData         *beginArrowLine;
    vtkPolyData         *endArrowLine;

    int                  beginArrowStyle;
    int                  endArrowStyle;

    bool                 addedToRenderer;

    bool ShouldBeAddedToRenderer() const;
    
    void makeArrows(vtkPolyData *, vtkPolyData *, bool);
    void updateArrows(vtkPolyData *, vtkPolyData *, double *, double *);
};


#endif


