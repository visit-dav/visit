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
//                            VisWinLighting.C                               //
// ************************************************************************* //

#include <VisWinLighting.h>

#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkLight.h>

#include <VisWindow.h>

#include <avtLightList.h>


// ****************************************************************************
//  Method: VisWinLighting constructor
//
//  Programmer: Kathleen Bonnell, 
//  Creation:   August 13, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Aug 14 12:33:59 PDT 2002 
//    Set light[0] default values.
//
//    Kathleen Bonnell, Thu Aug 29 09:49:36 PDT 2002 
//    Moved initialization of light[0] (the default light) and other data members
//    to new method InitDefaultLight. 
//
// ****************************************************************************

VisWinLighting::VisWinLighting(VisWindowColleagueProxy &p) : VisWinColleague(p)
{
    vtkRenderer *canvas = mediator.GetCanvas();

    for (int i = 0 ; i < MAX_LIGHTS ; i++)
    {
        lights[i] = vtkLight::New();
        canvas->AddLight(lights[i]);
    }

    InitDefaultLight();
}


// ****************************************************************************
//  Method: VisWinLighting destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002 
//
// ****************************************************************************

VisWinLighting::~VisWinLighting()
{
    for (int i = 0 ; i < MAX_LIGHTS ; i++)
    {
        if (lights[i] != NULL)
        {
            lights[i]->Delete();
            lights[i] = NULL;
        }
    }
}

// ****************************************************************************
//  Method: VisWinLighting::InitDefaultLight
//
//  Purpose:
//    Creates a default light to be used for 2D, or if user doesn't specify
//    lights.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 27, 2002 
//
//  Modifications:
//    Eric Brugger, Tue May 25 14:03:06 PDT 2004
//    Modify the routine to set the light position so that the light source
//    direction vtk passes to OpenGL is normalized.
//
//    Kathleen Bonnell, Tue Oct 26 15:58:06 PDT 2004
//    Camera lights should be defined defined in a coordinate space where 
//    the camera is located at (0, 0, 1), looking towards (0, 0, 0).  Thus the
//    focal point should always be (0, 0, 0), and the position is the negative
//    of the avtLight's direction.
//
// ****************************************************************************

void
VisWinLighting::InitDefaultLight()
{
    lights[0]->SetLightTypeToCameraLight();
    lights[0]->SetFocalPoint(0, 0, 0);
    lights[0]->SetPosition(0, 0, 1);
    lights[0]->SetColor(1., 1., 1.);
    lights[0]->SetIntensity(1.);
    lights[0]->SwitchOn();

    for (int i = 1 ; i < MAX_LIGHTS ; i++)
    {
        lights[i]->SwitchOff();
    }

    mediator.GetCanvas()->SetAmbient(1., 1., 1.);
    ambientOn = false;
    numLightsEnabled = 1;
    ambientCoefficient = 0.;
}


// ****************************************************************************
//  Method: VisWinLighting::SetLightList
//
//  Purpose:
//     Uses an avtLightList to update vtkLight properties. 
//
//  Arguments:
//      ll      The light list to use for setting vtk light properties. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Aug 14 12:33:59 PDT 2002 
//    Modified so that the test numLightsEnabled is always peformed.  That way
//    the default vtk headlight is never created.
//
//    Brad Whitlock, Tue Aug 20 09:24:20 PDT 2002
//    I renamed EYE lights to CAMERA lights and I renamed DIRECTIONAL lights
//    OBJECT lights.
//
//    Kathleen Bonnell, Tue Aug 27 13:25:42 PDT 2002 
//    Moved bulk of code to UpdateVTKLights(), which is called only for 3D. 
//    
// ****************************************************************************
 
void
VisWinLighting::SetLightList(avtLightList &ll)
{
    avtlights = ll;
    if (mediator.GetMode() == WINMODE_3D)
    {
        UpdateVTKLights();
    }
}

// ****************************************************************************
//  Method: VisWinLighting::UpdateVTKLights
//
//  Purpose:
//     Uses an avtLightList to update vtkLight properties. 
//     (Moved from SetLightList).
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 27, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Sep  6 09:48:04 PDT 2002 
//    Use abmient light intensity when computing ambient light color. 
//    Add work-around for VTK bug when changing light type to a SceneLight. 
//    For lights switched 'on' to avoid vtk creating a light when we only want
//    ambient lights, set intensity to 0, so it won't contribute anything to
//    the scene.
//    
// ****************************************************************************

void
VisWinLighting::UpdateVTKLights()
{
    numLightsEnabled = 0;
    int i, firstNonAmbient = -1, ambientCount = 0;
    double ambientColor[3] = { 0., 0., 0.};
    avtLight aLight;

    vtkRenderer *canvas = mediator.GetCanvas();

    ambientOn = false;
    ambientCoefficient = 0.;
    
    UpdateLightPositions();

    for (i = 0; i < MAX_LIGHTS ; i++)
    {
        aLight = avtlights.Light(i);
        lights[i]->SetColor((double) aLight.color[0],
                            (double) aLight.color[1],
                            (double) aLight.color[2]);
        lights[i]->SetIntensity(aLight.brightness);
        lights[i]->SetFocalPoint(canvas->GetActiveCamera()->GetFocalPoint());

        switch (aLight.type)
        {
            case CAMERA:
                lights[i]->SetLightTypeToCameraLight();
                if (firstNonAmbient == -1)
                {
                    firstNonAmbient = i;
                }
                if (aLight.enabled)
                {
                    lights[i]->SwitchOn();
                    numLightsEnabled++;
                }
                else 
                {
                    lights[i]->SwitchOff();
                }
                break;

            case OBJECT:
                lights[i]->SetLightTypeToSceneLight();
                //
                // This next line is a work-around for a known VTK bug 
                // when switching to scene light from another light type.
                // Can be removed when we switch to new VTK dated after
                // 6Sep02.
                //
                lights[i]->SetTransformMatrix(NULL);
                if (firstNonAmbient == -1)
                {
                    firstNonAmbient = i;
                }
                if (aLight.enabled)
                {
                    lights[i]->SwitchOn();
                    numLightsEnabled++;
                }
                else 
                {
                    lights[i]->SwitchOff();
                }
                break;

            case AMBIENT:
                //
                //  Ambient light is handled by a different mechanism in VTK,
                //  so don't turn on the vtkLight, but set some flags to
                //  handle the ambient settings, if this light is enabled.
                //
                lights[i]->SwitchOff();
                if (aLight.enabled)
                {
                    ambientColor[0] += aLight.color[0] * aLight.brightness; 
                    ambientColor[1] += aLight.color[1] * aLight.brightness; 
                    ambientColor[2] += aLight.color[2] * aLight.brightness; 
                    ambientCoefficient += aLight.brightness;
                    ambientOn = true;
                    ambientCount++;
                }
                break;
 
            default: break;
        }
    }
    if (ambientOn)
    {
        //
        // Using an averaged ambientCoefficent for multiple ambient
        // lights yields a more pleasing visual result.
        //
        ambientCoefficient /= (double) ambientCount;
        canvas->SetAmbient(ambientColor);
    }
    else
    {
        canvas->SetAmbient(1., 1., 1.);
    }
    if (numLightsEnabled == 0 && firstNonAmbient != -1)
    {
        //
        //  Cannot leave all the lights OFF, because VTK will create a 
        //  default light that is difficult to get rid of, so turn on a 
        //  non-ambient light.  We don't want it to contribute anything
        //  to the scene, so set its intensity to zero. 
        //
        lights[firstNonAmbient]->SwitchOn();
        lights[firstNonAmbient]->SetIntensity(0.);
    }
}

// ****************************************************************************
//  Method: VisWinLighting::GetAmbientCoefficient
//
//  Purpose:
//     Returns the ambient lighting coefficient. 
//
//  Returns:
//      The ambient lighting coefficient (an average over all enabled
//      ambient avtLights). 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002 
//
// ****************************************************************************

double
VisWinLighting::GetAmbientCoefficient()
{
    return ambientCoefficient;
}


// ****************************************************************************
//  Method: VisWinLighting::GetAmbientOn
//
//  Purpose:
//      Returns a value specifying whether or not ambient lighting is on. 
//
//  Returns:
//     True if ambient lighting is on, false otherwise. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002 
//
// ****************************************************************************

bool
VisWinLighting::GetAmbientOn()
{
    return ambientOn;
}


// ****************************************************************************
//  Method: VisWinLighting::GetNumLightsEnabled
//
//  Purpose:
//      Returns the number of lights that are enabled (turned on). 
//
//  Returns:
//      The number of enabled lights. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002 
//
// ****************************************************************************

int
VisWinLighting::GetNumLightsEnabled()
{
    return numLightsEnabled;
}


// ****************************************************************************
//  Method: VisWinLighting::UpdateLightPositions
//
//  Purpose:
//      Resets the light positions based on current camera settings. 
//
//  Arguments:
//      ll      The light list that has each light's direction vector. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Aug 27 13:25:42 PDT 2002
//    Distinguish between 3D and 2D actions for updating the light positions.
//    For 3D lights, update the focal point as well.
//
//    Eric Brugger, Tue May 25 14:03:06 PDT 2004
//    Modify the routine to set the light position so that the light source
//    direction vtk passes to OpenGL is normalized.
//
//    Kathleen Bonnell, Tue Oct 26 15:58:06 PDT 2004
//    Camera lights should be defined defined in a coordinate space where 
//    the camera is located at (0, 0, 1), looking towards (0, 0, 0).  Thus the
//    focal point should always be (0, 0, 0), and the position is the negative
//    of the avtLight's direction.
//
// ****************************************************************************

void
VisWinLighting::UpdateLightPositions()
{
    if (mediator.GetMode() == WINMODE_3D)
    {
        //
        //  Get the camera's distance from the focal point.
        //
        vtkCamera *cam = mediator.GetCanvas()->GetActiveCamera();
        double cpos[3], cfoc[3], proj[3], projLen = 0.;

        cam->GetPosition(cpos);
        cam->GetFocalPoint(cfoc);
        proj[0] = cpos[0] - cfoc[0];
        proj[1] = cpos[1] - cfoc[1];
        proj[2] = cpos[2] - cfoc[2];
        int i; 
        for (i = 0; i < 3; i++)
        {
             projLen += (proj[i] * proj[i]);
        }
        projLen = sqrt(projLen);

        //
        //  VisIt lights have a unit vector direction, pointing
        //  from the light's position to the focal point.  We need
        //  to use the inverse vector in determing the position 
        //  in world coordinates.  We want to position the light
        //  so that vtk passes a unit vector for the light direction
        //  along the (inverse) direction vector to OpenGL.
        //
        double pos[3];
        avtLight aLight;
        for (i = 0; i < MAX_LIGHTS ; i++)
        {
            aLight = avtlights.Light(i);
            if (aLight.type == CAMERA)
            {
                lights[i]->SetPosition(-aLight.direction[0],
                                       -aLight.direction[1],
                                       -aLight.direction[2]);
                lights[i]->SetFocalPoint(0, 0, 0);
            }
            else 
            {
                pos[0] = cfoc[0] - aLight.direction[0] / projLen; 
                pos[1] = cfoc[1] - aLight.direction[1] / projLen; 
                pos[2] = cfoc[2] - aLight.direction[2] / projLen; 
                lights[i]->SetPosition(pos);
                lights[i]->SetFocalPoint(cfoc);
            }
        }
    }
    else 
    {
        InitDefaultLight();
    }
}


// ****************************************************************************
//  Method: VisWinLighting::Start3DMode
//
//  Purpose:
//    Sets up the renderer's vtk lights based on the values in the light list. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 27, 2002 
//
// ****************************************************************************

void
VisWinLighting::Start3DMode()
{
    UpdateVTKLights();
}


// ****************************************************************************
//  Method: VisWinLighting::Stop3DMode
//
//  Purpose:
//      Reverts back to default lighting conditions. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 27, 2002 
//
// ****************************************************************************

void
VisWinLighting::Stop3DMode()
{
    InitDefaultLight();
}
