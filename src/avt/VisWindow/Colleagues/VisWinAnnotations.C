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

#include <VisWinAnnotations.h>
#include <VisWindowColleagueProxy.h>
#include <AnnotationObject.h>
#include <AnnotationObjectList.h>
#include <DebugStream.h>
#include <snprintf.h>

#include <avtAnnotationColleague.h>
#include <avtImageColleague.h>
#include <avtLegendAttributesColleague.h>
#include <avtLine2DColleague.h>
#include <avtText2DColleague.h>
#include <avtText3DColleague.h>
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
//   Brad Whitlock, Tue Mar 20 17:10:45 PST 2007
//   Added actorList.
//
// ****************************************************************************

VisWinAnnotations::VisWinAnnotations(VisWindowColleagueProxy &m) :
    VisWinColleague(m), annotations(), actorList()
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
    // Clear the actorList. Note that we don't need to do any deletions because
    // the actors are actually cleared by the VisWinPlots collague.
    actorList.clear();

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

    // Update the legends so they get the right colors.
    UpdateLegends();
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

// ****************************************************************************
// Method: VisWinAnnotations::UpdatePlotList
//
// Purpose: 
//   Called when the plot list updates.
//
// Arguments:
//   p : The new list of actors.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 02:20:24 PDT 2007
//
// Modifications:
//   Brad Whitlock, Thu Mar 22 02:20:43 PDT 2007
//   Made it update the legend positions.
//
// ****************************************************************************

void
VisWinAnnotations::UpdatePlotList(std::vector<avtActor_p> &p)
{
    for(int i = 0; i < annotations.size(); ++i)
        annotations[i]->UpdatePlotList(p);

    // Save off the pointers to the actors.
    actorList.clear();
    for(int j = 0; j < p.size(); ++j)
        actorList.push_back(*p[j]);

    UpdateLegends();
}

// ****************************************************************************
// Method: VisWinAnnotations::UpdateLegends
//
// Purpose: 
//   This method performs legend layout and customization.
//
// Note:       The legends are generally not removed or added in this routine
//             and their titles are not set here (yet). Those operations
//             happen in VisWinLegends.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:24:47 PST 2007
//
// Modifications:
//    Dave Bremer, Wed Oct  8 11:36:27 PDT 2008
//    Now, execute the code that applies attributes to a new legend before
//    computing the legend's position, because the orientation attribute
//    affects the legend's width and height.
//   
// ****************************************************************************

void
VisWinAnnotations::UpdateLegends()
{
    //
    // Manage legend layout.
    //
    std::vector<avtActor*>::iterator it;
    vtkRenderer *foreground = mediator.GetForeground();
    double yTop = 0.90;
    double xLeft = 0.05;

    int legendCount = 0;
    for (it = actorList.begin() ; it != actorList.end() ; it++)
    {
        // Look in the annotation list for a suitable object that
        // can be used to set the legend for this actor.
        avtAnnotationColleague *annot = 0;
        for(int i = 0; i < annotations.size(); ++i)
        {
            if(annotations[i]->GetName() == (*it)->GetActorName())
            {
                annot = annotations[i];
                debug5 << "Found matching annotation attributes for legend "
                       << (*it)->GetActorName() << endl;
                break;
            }
        }

        avtLegend_p legend = (*it)->GetLegend();
        if (*legend != NULL)
        {
            
            if(legend->GetCurrentlyDrawn() && annot != 0)
                annot->CustomizeLegend(legend);

            // The legend was added to or removed in 
            // VisWinLegends::PositionLegends. Here we want to control the
            // layout and other legend attributes.
            if(legend->GetCurrentlyDrawn())
            {
                bool manageLayout = true;
                if(annot != 0)
                    manageLayout = annot->ManageLayout(legend);
              
                if(manageLayout)
                {
                    double width, height;
                    legend->GetLegendSize(yTop, width, height);

                    if (yTop - height >= 0.)
                    {
                        yTop -= height;

                        legend->SetLegendPosition(xLeft, yTop);
                        legend->Update();

                        yTop -= 0.02;
                        legendCount++;
                    }
                    else if(xLeft < 0.9)
                    {
                        // We have more legends than will fit in the 
                        // allotted height so move them over to the right.
                        if(legendCount > 0)
                        {
                            xLeft = 0.875;
                            yTop = 0.9;
                        }

                        yTop -= height;

                        legend->SetLegendPosition(xLeft, yTop);
                        legend->Update();

                        yTop -= 0.02;
                        legendCount++;
                    }
                    else
                        legend->Remove();
                }
            }
        }
    }
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
//   annotName : The name for the annotation object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:41:51 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun 28 11:56:28 PDT 2005
//   Added John Anderson's objects.
//
//   Brad Whitlock, Tue Mar 20 10:11:22 PDT 2007
//   Name the objects. Also added avtLegendAttributesColleague.
//
//   Brad Whitlock, Wed Nov 7 17:01:34 PDT 2007
//   Added text 3D finally.
//
// ****************************************************************************

bool
VisWinAnnotations::AddAnnotationObject(int annotType, const std::string &annotName)
{
    const char *mName = "VisWinAnnotations::AddAnnotationObject: ";

    //
    // Make sure that the name is unique.
    //
    if(annotName != "")
    {
        for(int i = 0; i < annotations.size(); ++i)
        {
            if(annotations[i]->GetName() == annotName)
            {
                debug1 << mName << "Did not add the object because the "
                    "proposed name already existed in the annotation object "
                    "list." << endl;
                return false;
            }
        }
    }

    //
    // Try creating an annotation.
    //
    avtAnnotationColleague *annot = 0;
    switch(annotType)
    {
    case 0: // Text2D
        annot = new avtText2DColleague(mediator);
        break;
    case 1: // Text3D
        annot = new avtText3DColleague(mediator);
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
    case 8: // LegendAttributes
        annot = new avtLegendAttributesColleague(mediator);
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
        // Set the annotation's name.
        if(annotName == "")
        {
            // Come up with a new name for the object.
            char tmp[200];
            bool found = false;
            std::string newName;
            int num = 1;
            do
            {
                SNPRINTF(tmp, 200, "%s%d", annot->TypeName().c_str(), num++);
                newName = tmp;

                found = false;
                for(int i = 0; i < annotations.size(); ++i)
                {
                    if(annotations[i]->GetName() == newName)
                    {
                        found = true;
                        break;
                    }
                }
            } while(found);
            annot->SetName(newName);
            debug1 << mName << "New " << annot->TypeName().c_str()
                   << " object created. It is called \"" << newName.c_str()
                   << "\" and VisIt made up that name." << endl;
        }
        else
        {
            annot->SetName(annotName);
            debug1 << mName << "New " << annot->TypeName().c_str()
                   << " object created. It is called \"" << annotName.c_str()
                   << "\"." << endl;
        }

        // Add the annotation to the list.
        annotations.push_back(annot);

        //
        // Add the annotation to the renderer.
        //
        annot->AddToRenderer();

        // Make the new annotation be the active annotation.
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
// Method: VisWinAnnotations::DeleteAnnotationObject
//
// Purpose: 
//   Deletes the named annotation.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 10:31:03 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
VisWinAnnotations::DeleteAnnotationObject(const std::string &name)
{
    int i, index = -1;
    for(i = 0; i < annotations.size(); ++i)
    {
        if(annotations[i]->GetName() == name)
        {
            index = i;
            break;
        }
    }

    for(i = 0; i < annotations.size(); ++i)
        annotations[i]->SetActive(i == index);

    DeleteActiveAnnotationObjects();

    return index != -1;
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
//   Brad Whitlock, Tue Mar 20 17:08:08 PST 2007
//   Added code to update the legends.
//
// ****************************************************************************

void
VisWinAnnotations::SetAnnotationObjectOptions(const AnnotationObjectList &al)
{
    for(int i = 0; i < al.GetNumAnnotations(); ++i)
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

    // Update the legends for the actors.
    UpdateLegends();
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
//   Brad Whitlock, Tue Mar 20 13:41:26 PST 2007
//   Set the annotation name.
//
// ****************************************************************************

void
VisWinAnnotations::UpdateAnnotationObjectList(AnnotationObjectList &al)
{
    al.ClearAnnotations();

    for(int i = 0; i < annotations.size(); ++i)
    {
        AnnotationObject annot;
        annotations[i]->GetOptions(annot);
        annot.SetObjectName(annotations[i]->GetName());
        al.AddAnnotation(annot);
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
//   Brad Whitlock, Tue Mar 20 13:40:31 PST 2007
//   Name annotation objects.
//
// ****************************************************************************

void
VisWinAnnotations::CreateAnnotationObjectsFromList(const AnnotationObjectList &al)
{
    for(int i = 0; i < al.GetNumAnnotations(); ++i)
    {
        const AnnotationObject &annot = al[i];
        int annotType = int(annot.GetObjectType());
        if(AddAnnotationObject(annotType, annot.GetObjectName()))
            annotations[annotations.size()-1]->SetOptions(annot);
    }
}
