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

#include <vtkTimeSliderActor.h>

#include <vtkCellArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkViewport.h>

#define DEFAULT_X_OFFSET 0.0
#define DEFAULT_Y_OFFSET 0.0
#define DEFAULT_WIDTH    0.25
#define DEFAULT_HEIGHT   0.06

#define N_COLOR_COMPONENTS 4
#define SET_COLOR(ptr, color)  \
        *ptr++ = (unsigned char)((float)color[0] * 255.f); \
        *ptr++ = (unsigned char)((float)color[1] * 255.f); \
        *ptr++ = (unsigned char)((float)color[2] * 255.f); \
        *ptr++ = (unsigned char)((float)color[3] * 255.f);

vtkStandardNewMacro(vtkTimeSliderActor);

// ****************************************************************************
// Method: vtkTimeSliderActor::vtkTimeSliderActor
//
// Purpose: 
//   Constructor for the vtkTimeSlider class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 28 11:00:37 PDT 2003
//
// Modifications:
//   Eric Brugger, Wed Aug 25 15:00:06 PDT 2004
//   Modify the interpretation of Position2 to the more standard vtk
//   interpretation where the coordinate is relative to Position.
//   
// ****************************************************************************

vtkTimeSliderActor::vtkTimeSliderActor() : vtkActor2D()
{
    this->SliderPolyData = NULL;
    this->SliderActor = NULL;
    this->SliderMapper = NULL;

    // Set the default colors.
    this->StartColor[0] = 0.f;
    this->StartColor[1] = 0.f;
    this->StartColor[2] = 0.f;
    this->StartColor[3] = 1.f;

    this->EndColor[0] = 1.f;
    this->EndColor[1] = 1.f;
    this->EndColor[2] = 1.f;
    this->EndColor[3] = 1.f;

    // Set the default flags, etc.
    this->ParametricTime = 0.5f;
    this->VerticalDivisions = 10;
    this->DrawEndCaps = 1;
    this->RadialDivisions = 10;
    this->Draw3D = 1;

    // Set the default location and size
    this->SetPosition(DEFAULT_X_OFFSET, DEFAULT_Y_OFFSET);
    this->SetWidth(DEFAULT_WIDTH);
    this->SetHeight(DEFAULT_HEIGHT);
    this->SetPosition2(DEFAULT_WIDTH, DEFAULT_HEIGHT);
}

// ****************************************************************************
// Method: vtkTimeSliderActor::~vtkTimeSliderActor
//
// Purpose: 
//   Destructor for the vtkTimeSliderActor class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 28 11:02:10 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

vtkTimeSliderActor::~vtkTimeSliderActor()
{
    DestroySlider();
}

// ****************************************************************************
// Method: vtkTimeSliderActor::DestroySlider
//
// Purpose: 
//   Destroys the objects that make up the slider.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 28 11:03:47 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
vtkTimeSliderActor::DestroySlider()
{
    if(this->SliderPolyData != NULL)
    {
        this->SliderPolyData->Delete();
        this->SliderPolyData = NULL;
    }

    if(this->SliderActor != NULL)
    {
        this->SliderActor->Delete();
        this->SliderActor = NULL;
    }

    if(this->SliderMapper != NULL)
    {
        this->SliderMapper->Delete();
        this->SliderMapper = NULL;
    }
}

// ****************************************************************************
// Method: vtkTimeSliderActor::AddEndCapCells
//
// Purpose: 
//   Adds end cap cells to a cell array.
//
// Arguments:
//   center : The index of the center point.
//   polys  : The cell array to which we're adding end cap cells.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 28 11:04:18 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
vtkTimeSliderActor::AddEndCapCells(int center, vtkCellArray *polys)
{
    const int nRings = this->VerticalDivisions / 2;
    vtkIdType ptIds[4];

    for(int i = 0; i < nRings; ++i)
    {
        for(int j = 0; j < this->RadialDivisions; ++j)
        {
            if(i == 0)
            {
                ptIds[0] = center;
                ptIds[1] = center + j * nRings + 1;
                ptIds[2] = center + (j+1) * nRings + 1;
                polys->InsertNextCell(3, ptIds);
            }
            else
            {
                ptIds[0] = center + j * nRings + 1 + i-1;
                ptIds[1] = center + j * nRings + 1 + (i-1 + 1);
                ptIds[2] = center + (j+1) * nRings + 1 + (i-1 + 1);
                ptIds[3] = center + (j+1) * nRings + 1 + i-1;
                polys->InsertNextCell(4, ptIds);
            }
        }
    }
}

// ****************************************************************************
// Method: vtkTimeSliderActor::CreateSlider
//
// Purpose: 
//   Creates the slider objects.
//
// Arguments:
//   viewport : The viewport into which the slider will be drawn.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 28 11:05:13 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Dec 9 08:38:22 PDT 2003
//   Fixed a bug that caused the end caps to become separated from the main bar.
//
//   Eric Brugger, Wed Aug 25 15:00:06 PDT 2004
//   Modify the interpretation of Position2 to the more standard vtk
//   interpretation where the coordinate is relative to Position.
//
// ****************************************************************************

void
vtkTimeSliderActor::CreateSlider(vtkViewport *viewport)
{
    double BL[2] = {this->GetPosition()[0], this->GetPosition()[1]};
    double TR[2] = {this->GetPosition()[0] + this->GetPosition2()[0],
                   this->GetPosition()[1] + this->GetPosition2()[1]};

#ifdef CREATE_POLYDATA_IN_SCREEN_SPACE
    viewport->NormalizedDisplayToDisplay(BL[0], BL[1]);
    viewport->NormalizedDisplayToDisplay(TR[0], TR[1]);
#endif

    //
    // If we're drawing endcaps, move the bar in a little to make room.
    //
    double BarHeight = TR[1] - BL[1];
    if(BarHeight < 0.f) BarHeight = -BarHeight;
    double EndCapRadius = BarHeight / 2.;
    if(this->DrawEndCaps != 0)
    {
        BL[0] += EndCapRadius;
        TR[0] -= EndCapRadius;
    }

    double CY = (TR[1] + BL[1]) / 2.;
    double CX = BL[0];
    double CX2 = TR[0];

    double midX(this->ParametricTime * TR[0] + (1.-this->ParametricTime) * BL[0]);

    int numPts = 4 * (this->VerticalDivisions + 1);
    int nNewPtsPerAngle = this->VerticalDivisions / 2;
    int nNewPtsPerEndCap = nNewPtsPerAngle * (this->RadialDivisions+1) + 1;

    if(this->DrawEndCaps)
        numPts += (nNewPtsPerEndCap * 2 );
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(numPts);
    vtkCellArray *polys = vtkCellArray::New();
    polys->Allocate(polys->EstimateSize(2, 4));
    vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
    colors->SetNumberOfComponents(N_COLOR_COMPONENTS);
    colors->SetNumberOfTuples(numPts);

    this->SliderPolyData = vtkPolyData::New();
    this->SliderPolyData->Initialize();
    this->SliderPolyData->SetPoints(points);
    this->SliderPolyData->SetPolys(polys);
    this->SliderPolyData->GetPointData()->SetScalars(colors);
    points->Delete(); polys->Delete(); colors->Delete();

    //
    // Create the points and colors to use for the cells.
    //
    double pt[3]; pt[2] = 0.;
    int i, index = 0;
    unsigned char *rgb = colors->GetPointer(0);
    vtkIdType ptIds[4];
    double lColor[N_COLOR_COMPONENTS], rColor[N_COLOR_COMPONENTS];

    for(i = 0; i < this->VerticalDivisions + 1; ++i)
    {
        double t = double(i) / double(this->VerticalDivisions);
        double omt = 1.f - t;
        double y = t * TR[1] + omt * BL[1];

        pt[0] = BL[0];
        pt[1] = y;
        points->SetPoint(index++, pt);
        pt[0] = midX;
        pt[1] = y;
        points->SetPoint(index++, pt);
        pt[0] = midX;
        pt[1] = y;
        points->SetPoint(index++, pt);
        pt[0] = TR[0];
        pt[1] = y;
        points->SetPoint(index++, pt);

        if(this->Draw3D)
        {
            // If we're making it look 3D, then use shading in the colors.
            double angle = omt * 3.14159;
            double colorT = sin(angle) * 0.7;
            for(int j = 0; j < N_COLOR_COMPONENTS; ++j)
            {
                lColor[j] = colorT * this->StartColor[j] + 0.3 * this->StartColor[j];
                rColor[j] = colorT * this->EndColor[j] + 0.3 * this->EndColor[j];
            }
        }
        else
        {
            for(int j = 0; j < N_COLOR_COMPONENTS; ++j)
            {
                lColor[j] = this->StartColor[j];
                rColor[j] = this->EndColor[j];
            }
        }

#if N_COLOR_COMPONENTS == 4
        // Don't allow opacity to vary with shading. That's weird.
        lColor[3] = this->StartColor[3];
        rColor[3] = this->EndColor[3];
#endif

        SET_COLOR(rgb, lColor);
        SET_COLOR(rgb, lColor);
        SET_COLOR(rgb, rColor);
        SET_COLOR(rgb, rColor);
    }

    //
    // Create the cells.
    //
    for(i = 0; i < this->VerticalDivisions; ++i)
    {
        int index1 = i * 4;
        int index2 = index1 + 4;

        ptIds[0] = index1;
        ptIds[1] = index1 + 1;
        ptIds[2] = index2 + 1;
        ptIds[3] = index2;
        polys->InsertNextCell(4, ptIds);

        ptIds[0] = index1 + 2;
        ptIds[1] = index1 + 3;
        ptIds[2] = index2 + 3;
        ptIds[3] = index2 + 2;
        polys->InsertNextCell(4, ptIds);
    }

    //
    // Create the end caps if we want to draw them.
    //
    if(this->DrawEndCaps)
    {
        double startColor[N_COLOR_COMPONENTS];
        double endColor[N_COLOR_COMPONENTS];
        int ci;
        for(ci = 0; ci < N_COLOR_COMPONENTS; ++ci)
        {
            startColor[ci] = this->StartColor[ci];
            endColor[ci] = this->EndColor[ci];
        }

        if(this->ParametricTime <= 0.)
        {
            for(ci = 0; ci < N_COLOR_COMPONENTS; ++ci)
                startColor[ci] = this->EndColor[ci];
#if N_COLOR_COMPONENTS == 4
            startColor[3] = this->EndColor[3];
#endif
        }

        if(this->ParametricTime >= 1.)
        {
            for(ci = 0; ci < N_COLOR_COMPONENTS; ++ci)
                endColor[ci] = this->StartColor[ci];
#if N_COLOR_COMPONENTS == 4
            endColor[3] = this->StartColor[3];
#endif
        }

        int center = index;
        pt[0] = CX;
        pt[1] = CY;
        points->SetPoint(index++, pt);
        SET_COLOR(rgb, startColor);

        int nRings = this->VerticalDivisions / 2;

        for(i = 0; i < this->RadialDivisions+1; ++i)
        {
            double t = double(i) / double(this->RadialDivisions);
            double angle = t * 3.14159;
            double x = cos(angle);
            double y = sin(angle);
            for(int j = 0; j < nRings; ++j)
            {
                // Set the points for the endcap.
                double tr = double(j+1) / double(nRings);
                pt[0] = CX - tr * EndCapRadius * y;
                pt[1] = CY + tr * EndCapRadius * x;
                points->SetPoint(index++, pt);

                // Set the colors for the points in the endcap.
                if(this->Draw3D)
                {
                    double a2 = tr * 3.14159f / 2. + 3.14159f / 2.;
                    double colorT = sin(a2) * 0.7f;
                    double lColor[N_COLOR_COMPONENTS];
                    for(ci = 0; ci < N_COLOR_COMPONENTS; ++ci)
                        lColor[ci] = colorT * startColor[ci] + 0.3 * startColor[ci];
#if N_COLOR_COMPONENTS == 4
                    lColor[3] = startColor[3];
#endif
                    SET_COLOR(rgb, lColor);
                }
                else
                {
                    SET_COLOR(rgb, startColor);
                }
            }
        }
        // Create the cells for the endcap.
        AddEndCapCells(center, polys);

        //
        // Create the right end cap.
        //
        center = index;
        pt[0] = CX2;
        pt[1] = CY;
        points->SetPoint(index++, pt);
        SET_COLOR(rgb, endColor);
        for(i = 0; i < this->RadialDivisions+1; ++i)
        {
            double t = double(i) / double(this->RadialDivisions);
            double angle = t * 3.14159;
            double x = cos(angle);
            double y = sin(angle);
            for(int j = 0; j < nRings; ++j)
            {
                // Set the points for the endcap.
                double tr = double(j+1) / double(nRings);
                pt[0] = CX2 + tr * EndCapRadius * y;
                pt[1] = CY - tr * EndCapRadius * x;
                points->SetPoint(index++, pt);

                // Set the colors for the points in the endcap.
                if(this->Draw3D)
                {
                    double a2 = tr * 3.14159f / 2. + 3.14159f / 2.;
                    double colorT = sin(a2) * 0.7f;
                    double rColor[N_COLOR_COMPONENTS];
                    for(ci = 0; ci < N_COLOR_COMPONENTS; ++ci)
                        rColor[ci] = colorT * endColor[ci] + 0.3 * endColor[ci];
#if N_COLOR_COMPONENTS == 4
                    rColor[3] = endColor[3];
#endif
                    SET_COLOR(rgb, rColor);
                }
                else
                {
                    SET_COLOR(rgb, endColor);
                }
            }
        }
        // Create the cells for the endcap.
        AddEndCapCells(center, polys);
    }

    //
    // Create the actors, etc.
    //
    this->SliderMapper = vtkPolyDataMapper2D::New();
    this->SliderMapper->SetInput(this->SliderPolyData);
#ifndef CREATE_POLYDATA_IN_SCREEN_SPACE
    // If we're not creating the polydata in screen space then we're defining it
    // once in viewport coordinates. Set the transform coordinate so the mapper
    // knows to convert the points to display points before rendering.
    vtkCoordinate *transformCoord = vtkCoordinate::New();
    transformCoord->SetCoordinateSystemToNormalizedViewport();
    this->SliderMapper->SetTransformCoordinate(transformCoord);
    transformCoord->Delete();
#endif

    this->SliderActor = vtkActor2D::New();
    this->SliderActor->SetMapper(this->SliderMapper);
}

// ****************************************************************************
// Method: vtkTimeSliderActor::RenderOverlay
//
// Purpose: 
//   Renders the time slider actor.
//
// Arguments:
//   viewport : The viewport into which we're rendering.
//
// Returns:    The number of actors that were rendered.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 28 11:29:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

int
vtkTimeSliderActor::RenderOverlay(vtkViewport *viewport)
{
    bool stateChanged = false;
    if(this->SliderActor != 0)
        stateChanged = this->GetMTime() > this->SliderActor->GetMTime()
#ifdef CREATE_POLYDATA_IN_SCREEN_SPACE
        // If we created the polydata in screen space then we need to recreate it
        // each time the viewport changes.
                       || viewport->GetMTime() > this->GetMTime();
#else
        ;
#endif

    if(stateChanged || this->SliderActor == NULL)
    {
        DestroySlider();
        CreateSlider(viewport);
    }

    return this->SliderActor->RenderOverlay(viewport);
}

// ****************************************************************************
// Method: vtkTimeSliderActor::RenderOpaqueGeometry
//
// Purpose: 
//   Does nothing!
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 28 11:30:48 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

int
vtkTimeSliderActor::RenderOpaqueGeometry(vtkViewport *)
{
    return 0;
}

int
vtkTimeSliderActor::RenderTranslucentGeometry(vtkViewport *)
{
    return 0;
}

// ****************************************************************************
// Method: vtkTimeSliderActor::PrintSelf
//
// Purpose: 
//   Prints the object.
//
// Arguments:
//   os     : The stream on which to write.
//   indent : The indentation level.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 28 11:31:15 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
vtkTimeSliderActor::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);

    int i;
    os << indent << "StartColor: (";
        for(i = 0; i < N_COLOR_COMPONENTS; ++i)
            os << this->StartColor[i];
            if(i < N_COLOR_COMPONENTS-1)
                os << ", ";
    os << ")" << endl;

    os << indent << "EndColor: (";
        for(i = 0; i < N_COLOR_COMPONENTS; ++i)
            os << this->EndColor[i];
            if(i < N_COLOR_COMPONENTS-1)
                os << ", ";
    os << ")" << endl;

    os << indent << "ParametricTime: " << this->ParametricTime << endl;

    os << indent << "VerticalDivisions: " << this->VerticalDivisions << endl;
    os << indent << "DrawEndCaps: " << this->DrawEndCaps << endl;
    os << indent << "RadialDivisions: " << this->RadialDivisions << endl;
    os << indent << "Draw3D: " << this->Draw3D << endl;
}

