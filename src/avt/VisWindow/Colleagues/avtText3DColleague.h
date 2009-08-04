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
//                              avtText3DColleague.h                         //
// ************************************************************************* //

#ifndef VIS_WIN_TEXT3D_COLLEAGUE_H
#define VIS_WIN_TEXT3D_COLLEAGUE_H
#include <viswindow_exports.h>
#include <avtAnnotationColleague.h>
#include <ColorAttribute.h>

class vtkFollower;
class vtkLinearExtrusionFilter;
class vtkPolyDataMapper;
class vtkPolyDataNormals;
class vtkVectorText;

// ****************************************************************************
// Class: avtText3DColleague
//
// Purpose:
//   This colleague is a 3D text label that can be shown in the vis window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 7 14:09:57 PST 2007
//
// Modifications:
//    Jeremy Meredith, Wed Mar 11 12:33:20 EDT 2009
//    Added $cycle support.
//   
// ****************************************************************************

class VISWINDOW_API avtText3DColleague : public avtAnnotationColleague
{
public:
    avtText3DColleague(VisWindowColleagueProxy &);
    virtual ~avtText3DColleague();

    virtual void AddToRenderer();
    virtual void RemoveFromRenderer();
    virtual void Hide();

    virtual std::string TypeName() const { return "Text3D"; }

    // Methods to set and get the annotation's properties.
    virtual void SetOptions(const AnnotationObject &annot);
    virtual void GetOptions(AnnotationObject &annot);

    // Methods that are called in response to vis window events.
    virtual void SetForegroundColor(double r, double g, double b);
    virtual void HasPlots(void);
    virtual void NoPlots(void);
    virtual void UpdatePlotList(std::vector<avtActor_p> &lst);
protected:
    bool ShouldBeAddedToRenderer() const;
    void SetText(const char *text);
    void UpdateActorScale();

    // Make a heap-allocated structure to prevent weird errors on MacOS X/gcc 4.0.1
    struct Text3DInformation
    {
        ColorAttribute  textColor;
        bool            positionInitialized;
        bool            scaleInitialized;
        char           *textFormatString;
        char           *textString;
        double          currentTime;
        int             currentCycle;

        bool            useForegroundForTextColor;
        bool            useRelativeHeight;
        int             relativeHeight;
        double          fixedHeight;
        double          rotations[3];
        bool            addedToRenderer;

        vtkVectorText            *textSource;
        vtkLinearExtrusionFilter *extrude;
        vtkPolyDataNormals       *normals;
        vtkPolyDataMapper        *mapper;
        vtkFollower              *textActor;
    };

    Text3DInformation *info;
};


#endif


