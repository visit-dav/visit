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

#include <VisWinAnnotations.h>
#include <VisWindowColleagueProxy.h>
#include <AnnotationObject.h>
#include <AnnotationObjectList.h>
#include <DebugStream.h>

#include <avtAnnotationColleague.h>
#include <avtImageColleague.h>
#include <avtLine2DColleague.h>
#include <avtText2DColleague.h>
#include <avtTimeSliderColleague.h>

// ****************************************************************************
// Method: VisWinAnnotations::VisWinAnnotations
//
// Purpose: 
//   Constructor for the VisWinAnnotations class.
//
// Arguments:
//   m : The mediator that we'll use.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:40:56 PST 2003
//
// Modifications:
//   
// ****************************************************************************

VisWinAnnotations::VisWinAnnotations(VisWindowColleagueProxy &m) :
    VisWinColleague(m), annotations()
{
}

// ****************************************************************************
// Method: VisWinAnnotations::~VisWinAnnotations
//
// Purpose: 
//   Destructor for the VisWinAnnotations class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:41:26 PST 2003
//
// Modifications:
//   
// ****************************************************************************

VisWinAnnotations::~VisWinAnnotations()
{
    for(int i = 0; i < annotations.size(); ++i)
        delete annotations[i];
}

//
// Overrides from VisWinColleague
//

void
VisWinAnnotations::SetBackgroundColor(double r, double g, double b)
{
    for(int i = 0; i < annotations.size(); ++i)
        annotations[i]->SetBackgroundColor(r, g, b);
}

void
VisWinAnnotations::SetForegroundColor(double r, double g, double b)
{
    for(int i = 0; i < annotations.size(); ++i)
        annotations[i]->SetForegroundColor(r, g, b);
}

void
VisWinAnnotations::StartCurveMode(void)
{
    for(int i = 0; i < annotations.size(); ++i)
        annotations[i]->StartCurveMode();
}

void
VisWinAnnotations::Start2DMode(void)
{
    for(int i = 0; i < annotations.size(); ++i)
        annotations[i]->Start2DMode();
}

void
VisWinAnnotations::Start3DMode(void)
{
    for(int i = 0; i < annotations.size(); ++i)
        annotations[i]->Start3DMode();
}

void
VisWinAnnotations::StopCurveMode(void)
{
    for(int i = 0; i < annotations.size(); ++i)
        annotations[i]->StopCurveMode();
}

void
VisWinAnnotations::Stop2DMode(void)
{
    for(int i = 0; i < annotations.size(); ++i)
        annotations[i]->Start2DMode();
}

void
VisWinAnnotations::Stop3DMode(void)
{
    for(int i = 0; i < annotations.size(); ++i)
        annotations[i]->Start3DMode();
}

void
VisWinAnnotations::HasPlots(void)
{
    for(int i = 0; i < annotations.size(); ++i)
        annotations[i]->HasPlots();
}

void
VisWinAnnotations::NoPlots(void)
{
    for(int i = 0; i < annotations.size(); ++i)
        annotations[i]->NoPlots();
}

void
VisWinAnnotations::MotionBegin(void)
{
    for(int i = 0; i < annotations.size(); ++i)
        annotations[i]->MotionBegin();
}

void
VisWinAnnotations::MotionEnd(void)
{
    for(int i = 0; i < annotations.size(); ++i)
        annotations[i]->MotionEnd();
}

void
VisWinAnnotations::UpdateView(void)
{
    for(int i = 0; i < annotations.size(); ++i)
        annotations[i]->UpdateView();
}

void
VisWinAnnotations::UpdatePlotList(std::vector<avtActor_p> &p)
{
    for(int i = 0; i < annotations.size(); ++i)
        annotations[i]->UpdatePlotList(p);
}

void
VisWinAnnotations::SetFrameAndState(int nFrames,
    int startFrame, int curFrame, int endFrame,
    int startState, int curState, int endState)
{
    for(int i = 0; i < annotations.size(); ++i)
    {
        annotations[i]->SetFrameAndState(nFrames,
            startFrame, curFrame, endFrame,
            startState, curState, endState);
    }
}

//
// Methods that manage the annotation list.
//

// ****************************************************************************
// Method: VisWinAnnotations::AddAnnotationObject
//
// Purpose: 
//   Factory method for creating new annotation objects.
//
// Arguments:
//   annotType : The type of annotation to create.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:41:51 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun 28 11:56:28 PDT 2005
//   Added John Anderson's objects.
//
// ****************************************************************************

bool
VisWinAnnotations::AddAnnotationObject(int annotType)
{
    //
    // Try creating an annotation.
    //
    avtAnnotationColleague *annot = 0;
    switch(annotType)
    {
    case 0: // Text2D
        annot = new avtText2DColleague(mediator);
        break;
    case 2: // Time slider
        annot = new avtTimeSliderColleague(mediator);
        break;
    case 3: // Line 2D
        annot = new avtLine2DColleague(mediator);
        break;
    case 7: // Image
        annot = new avtImageColleague(mediator);
        break;
    default:
        debug1 << "VisWinAnnotations:AddAnnotationObject: Annotation type "
               << annotType << " is not yet supported." << endl;
        break;
    };

    //
    // If we created an annotation, add it to the annotation vector.
    //
    if(annot)
    {
        annotations.push_back(annot);

        //
        // Add the annotation to the renderer if it should be added.
        //
        annot->AddToRenderer();

        // Make the new annotation be the active annotation.
        intVector activeAnnots;
        activeAnnots.push_back(annotations.size() - 1);
        for(int i = 0; i < annotations.size(); ++i)
             annotations[i]->SetActive(i == (annotations.size() - 1));

    }

    return annot != 0;
}

// ****************************************************************************
// Method: VisWinAnnotations::HideActiveAnnotationObjects
//
// Purpose: 
//   Hides the active annotations.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:43:11 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAnnotations::HideActiveAnnotationObjects()
{
    bool annotsHidden = false;

    for(int i = 0; i < annotations.size(); ++i)
    {
        if(annotations[i]->GetActive())
        {
            annotations[i]->Hide();
            annotsHidden = true;
        }
    }
}

// ****************************************************************************
// Method: VisWinAnnotations::DeleteActiveAnnotationObjects
//
// Purpose: 
//   Deletes the active annotations.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:43:27 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAnnotations::DeleteActiveAnnotationObjects()
{
    std::vector<avtAnnotationColleague *> newList;

    //
    // If an annotation is active, then remove it from the renderer and
    // delete it. Otherwise, add the annotation onto a new list.
    //
    bool annotsDeleted = false;
    for(int i = 0; i < annotations.size(); ++i)
    {
        if(annotations[i]->GetActive())
        {
            annotations[i]->RemoveFromRenderer();
            delete annotations[i];
            annotsDeleted = true;
        }
        else
            newList.push_back(annotations[i]);
    }

    // Copy the new list to the annotations list.
    annotations = newList;

    // If we have any annotations left, make sure that the first one is
    // active.
    if(annotations.size() > 0)
        annotations[0]->SetActive(true);
}

// ****************************************************************************
// Method: VisWinAnnotations::DeleteAllAnnotationObjects
//
// Purpose: 
//   Deletes all of the annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:43:44 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAnnotations::DeleteAllAnnotationObjects()
{
    bool annotsDeleted = false;

    for(int i = 0; i < annotations.size(); ++i)
    {
        annotations[i]->RemoveFromRenderer();
        delete annotations[i];
        annotsDeleted = true;
    }

    annotations.clear();
}

// ****************************************************************************
// Method: VisWinAnnotations::RaiseActiveAnnotationObjects
//
// Purpose: 
//   Raises the active annotation objects, which means moving them down in the
//   annotation object list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:44:06 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAnnotations::RaiseActiveAnnotationObjects()
{
    int i;
    std::vector<avtAnnotationColleague *> newList;

    //
    // If an annotation is active, then add it to the list.
    //
    for(i = 0; i < annotations.size(); ++i)
    {
        // Remove the object from the renderer.
        annotations[i]->RemoveFromRenderer();

        if(annotations[i]->GetActive())
            newList.push_back(annotations[i]);
    }

    //
    // Now that all of the active annotations have been moved to the top
    // of the list, add the ones that were not active.
    //
    for(i = 0; i < annotations.size(); ++i)
    {
        if(!annotations[i]->GetActive())
            newList.push_back(annotations[i]);
    }

    // Add the annotations to the renderer in the new order.
    annotations = newList;
    for(i = 0; i < annotations.size(); ++i)
        annotations[i]->AddToRenderer();
}

// ****************************************************************************
// Method: VisWinAnnotations::LowerActiveAnnotationObjects
//
// Purpose: 
//   Lowers the active annotations, which means moving them up in the annotation
//   object list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:44:36 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAnnotations::LowerActiveAnnotationObjects()
{
    int i;
    std::vector<avtAnnotationColleague *> newList;

    //
    // If an annotation is active, then add it to the list.
    //
    for(i = 0; i < annotations.size(); ++i)
    {
        // Remove the object from the renderer.
        annotations[i]->RemoveFromRenderer();

        if(!annotations[i]->GetActive())
            newList.push_back(annotations[i]);
    }

    //
    // Now that all of the active annotations have been moved to the top
    // of the list, add the ones that were not active.
    //
    for(i = 0; i < annotations.size(); ++i)
    {
        if(annotations[i]->GetActive())
            newList.push_back(annotations[i]);
    }

    // Add the annotations to the renderer in the new order.
    annotations = newList;
    for(i = 0; i < annotations.size(); ++i)
        annotations[i]->AddToRenderer();
}

// ****************************************************************************
// Method: VisWinAnnotations::SetAnnotationObjectOptions
//
// Purpose: 
//   Sets the annotation objects' properties using the annotation object list.
//
// Arguments:
//   al : The annotation object list used to set the annotation properties.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:45:17 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAnnotations::SetAnnotationObjectOptions(const AnnotationObjectList &al)
{
    for(int i = 0; i < al.GetNumAnnotationObjects(); ++i)
    {
        const AnnotationObject &annot = al[i];
        if(i < annotations.size())
        {
            // Set whether the annotation is active.
            annotations[i]->SetActive(annot.GetActive());

            //
            // Let the annotation set its attributes with the annotation
            // object.
            //
            annotations[i]->SetOptions(annot);

            //
            // Set whether or not the annotation is visible.
            //
            annotations[i]->SetVisible(annot.GetVisible());
            if(annotations[i]->GetVisible())
                annotations[i]->AddToRenderer();
            else
                annotations[i]->RemoveFromRenderer();
        }
    }
}

// ****************************************************************************
// Method: VisWinAnnotations::UpdateAnnotationObjectList
//
// Purpose: 
//   Updates the annotation object list using the annotations that have been
//   created.
//
// Arguments:
//   al : The annotation object list to update.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:46:03 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAnnotations::UpdateAnnotationObjectList(AnnotationObjectList &al)
{
    al.ClearAnnotationObjects();

    for(int i = 0; i < annotations.size(); ++i)
    {
        AnnotationObject annot;
        annotations[i]->GetOptions(annot);
        al.AddAnnotationObject(annot);
    }
}

// ****************************************************************************
// Method: VisWinAnnotations::CreateAnnotationObjectsFromList
//
// Purpose: 
//   Creates annotation objects using the annotation object properties in the
//   annotation object list.
//
// Arguments:
//   al : The annotation object list to use to create the annotations.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 17:34:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAnnotations::CreateAnnotationObjectsFromList(const AnnotationObjectList &al)
{
    for(int i = 0; i < al.GetNumAnnotationObjects(); ++i)
    {
        const AnnotationObject &annot = al[i];
        int annotType = int(annot.GetObjectType());
        if(AddAnnotationObject(annotType))
            annotations[annotations.size()-1]->SetOptions(annot);
    }
}
