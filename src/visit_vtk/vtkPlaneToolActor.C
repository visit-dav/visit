/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPlaneToolActor.h,v $
  Language:  C++
  Date:      $Date: 2000/07/11 18:10:58 $
  Version:   $Revision: 1.1 $
  Thanks:    Brad Whitlock, B Division, Lawrence Livermore Nat'l Laboratory

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkPlaneToolActor.h"
#include "vtkProperty.h"

// *************************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 15:14:29 PST 2002 
//   Replace 'New' method with Macro to match VTK 4.0 API.
// *************************************************************************

vtkStandardNewMacro(vtkPlaneToolActor);


vtkPlaneToolActor::vtkPlaneToolActor()
{
    this->LineColor[0] = 1.;
    this->LineColor[1] = 1.;
    this->LineColor[2] = 1.;

    this->PlaneActor = NULL;
    this->PlaneMapper = NULL;
    this->PlaneData = NULL;

    this->ArrowData = NULL;
    this->ArrowMapper1 = NULL;
    this->ArrowMapper2 = NULL;
    this->ArrowMapper3 = NULL;
    this->XAxisActor = NULL;
    this->YAxisActor = NULL;
    this->ZAxisActor = NULL;
}


vtkPlaneToolActor::~vtkPlaneToolActor()
{
    DestroyPlane();
    DestroyVectors();
}

void
vtkPlaneToolActor::DestroyPlane()
{
    if(this->PlaneActor != NULL)
    {
        this->PlaneActor->Delete();
        this->PlaneActor = NULL;
    }

    if(this->PlaneMapper != NULL)
    {
        this->PlaneMapper->Delete();
        this->PlaneMapper = NULL;
    }

    if(this->PlaneData != NULL)
    {
        this->PlaneData->Delete();
        this->PlaneData = NULL;
    }
}

void
vtkPlaneToolActor::DestroyVectors()
{
    if(this->ArrowData != NULL)
    {
        this->ArrowData->Delete();
        this->ArrowData = NULL;
    }

    if(this->ArrowMapper1 != NULL)
    {
        this->ArrowMapper1->Delete();
        this->ArrowMapper1 = NULL;
    }
    if(this->ArrowMapper2 != NULL)
    {
        this->ArrowMapper2->Delete();
        this->ArrowMapper2 = NULL;
    }
    if(this->ArrowMapper3 != NULL)
    {
        this->ArrowMapper3->Delete();
        this->ArrowMapper3 = NULL;
    }

    if(this->XAxisActor != NULL)
    {
        this->XAxisActor->Delete();
        this->XAxisActor = NULL;
    }

    if(this->YAxisActor != NULL)
    {
        this->YAxisActor->Delete();
        this->YAxisActor = NULL;
    }

    if(this->ZAxisActor != NULL)
    {
        this->ZAxisActor->Delete();
        this->ZAxisActor = NULL;
    }
}

// ****************************************************************************
// Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkUnsignedCharArray 
//    for colors instead.
// ****************************************************************************

void
vtkPlaneToolActor::CreatePlane()
{
    this->PlaneData = vtkPolyData::New();

    int numPts = 4;
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(numPts);
    vtkCellArray *lines = vtkCellArray::New();
    lines->Allocate(lines->EstimateSize(numPts, 2)); 
    vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
    colors->SetNumberOfComponents(3);
    colors->SetNumberOfTuples(4);

    // Store the colors and points in the polydata.
    this->PlaneData->Initialize();
    this->PlaneData->SetPoints(pts);
    this->PlaneData->SetLines(lines);
    this->PlaneData->GetCellData()->SetScalars(colors);
    pts->Delete(); lines->Delete(); colors->Delete(); 

#define PLANE_SIZE 10.

    // Add points to the vertex list.
    float coord[3];
    coord[2] = 0.;
    coord[0] = PLANE_SIZE;
    coord[1] = PLANE_SIZE;
    pts->SetPoint(0, coord);
    coord[0] = -PLANE_SIZE;
    coord[1] = PLANE_SIZE;
    pts->SetPoint(1, coord);
    coord[0] = -PLANE_SIZE;
    coord[1] = -PLANE_SIZE;
    pts->SetPoint(2, coord);
    coord[0] = PLANE_SIZE;
    coord[1] = -PLANE_SIZE;
    pts->SetPoint(3, coord);

    // Add a cell to the polydata.
    int   ptIds[2];
    ptIds[0] = 0;
    ptIds[1] = 1;
    lines->InsertNextCell(2, ptIds);
    ptIds[0] = 1;
    ptIds[1] = 2;
    lines->InsertNextCell(2, ptIds);
    ptIds[0] = 2;
    ptIds[1] = 3;
    lines->InsertNextCell(2, ptIds);
    ptIds[0] = 3;
    ptIds[1] = 0;
    lines->InsertNextCell(2, ptIds);

    // Write the colors into the array directly
    for(int i = 0; i < 4; ++i)
    {
        unsigned char *rgb = colors->GetPointer(i * 3);
        rgb[0] = (unsigned char)(this->LineColor[0] * 255.);
        rgb[1] = (unsigned char)(this->LineColor[1] * 255.);
        rgb[2] = (unsigned char)(this->LineColor[2] * 255.);
    }

    this->PlaneMapper = vtkPolyDataMapper::New();
    this->PlaneMapper->SetInput(this->PlaneData);

    this->PlaneActor = vtkActor::New();
//    this->PlaneActor->GetProperty()->SetRepresentationToWireframe();
    this->PlaneActor->GetProperty()->SetLineWidth(2.);
//    this->PlaneActor->GetProperty()->SetLighting(0);
    this->PlaneActor->SetMapper(this->PlaneMapper);
}

void
vtkPlaneToolActor::CreateVectors()
{
    CreateVectorData();

    // Create the arrows.
    this->XAxisActor = vtkActor::New();
    this->XAxisActor->SetMapper(this->ArrowMapper1);

    this->YAxisActor = vtkActor::New();
    this->YAxisActor->SetMapper(this->ArrowMapper2);

    this->ZAxisActor = vtkActor::New();
    this->ZAxisActor->SetMapper(this->ArrowMapper3);
}

// ****************************************************************************
// Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkUnsignedCharArray 
//    for colors instead.
// ****************************************************************************

void
vtkPlaneToolActor::CreateVectorData()
{
#define ARROW_LENGTH       10.
#define ARROW_HEAD_LENGTH  (0.15 * ARROW_LENGTH)
#define ARROW_HEAD_RADIUS  (ARROW_HEAD_LENGTH * 0.5)
#define ARROW_SHAFT_RADIUS (ARROW_HEAD_RADIUS * 0.4)
    int    nSides = 6;
    double dTheta = (2. * 3.14159) / double(nSides);

    int numPts = 3 * nSides + 2;
    int numCells = 4 * nSides;
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(numPts);
    vtkCellArray *polys = vtkCellArray::New();
    polys->Allocate(polys->EstimateSize(numCells, 4)); 
    vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
    colors->SetNumberOfComponents(3);
    colors->SetNumberOfTuples(numCells);

    // Store the colors and points in the polydata.
    this->ArrowData = vtkPolyData::New();
    this->ArrowData->Initialize();
    this->ArrowData->SetPoints(pts);
    this->ArrowData->SetPolys(polys);
    this->ArrowData->GetCellData()->SetScalars(colors);
    pts->Delete(); polys->Delete(); colors->Delete(); 

    //
    // Create the points.
    //
    float  coords[3];
    int    i, ptIndex = 0;
    double theta;
    for(i = 0, theta = 0.; i < nSides; ++i, theta += dTheta)
    {
        coords[0] = cos(theta) * ARROW_SHAFT_RADIUS;
        coords[1] = sin(theta) * ARROW_SHAFT_RADIUS;
        coords[2] = 0.;
        pts->SetPoint(ptIndex++, coords);
    }
    for(i = 0, theta = 0.; i < nSides; ++i, theta += dTheta)
    {
        coords[0] = cos(theta) * ARROW_SHAFT_RADIUS;
        coords[1] = sin(theta) * ARROW_SHAFT_RADIUS;
        coords[2] = (ARROW_LENGTH - ARROW_HEAD_LENGTH);
        pts->SetPoint(ptIndex++, coords);
    }
    for(i = 0, theta = 0.; i < nSides; ++i, theta += dTheta)
    {
        coords[0] = cos(theta) * ARROW_HEAD_RADIUS;
        coords[1] = sin(theta) * ARROW_HEAD_RADIUS;
        coords[2] = (ARROW_LENGTH - ARROW_HEAD_LENGTH);
        pts->SetPoint(ptIndex++, coords);
    }
    coords[0] = 0.;
    coords[1] = 0.;
    coords[2] = 0.;
    int startPoint = ptIndex;
    pts->SetPoint(ptIndex++, coords);
    coords[0] = 0.;
    coords[1] = 0.;
    coords[2] = ARROW_LENGTH;
    int endPoint = ptIndex;
    pts->SetPoint(ptIndex++, coords);

    //
    // Create the endpoint cells.
    //
    int ptIds[4];
    for(i = 0; i < nSides; ++i)
    {
        ptIds[0] = startPoint;
        ptIds[1] = i;
        ptIds[2] = (i < (nSides - 1)) ? (i + 1) : 0;
        polys->InsertNextCell(3, ptIds);
    }

    //
    // Create the shaft cells.
    //
    for(i = 0; i < nSides; ++i)
    {
        ptIds[0] = i;
        ptIds[1] = i + nSides;
        ptIds[2] = (i < (nSides - 1)) ? (i + nSides + 1) : nSides;
        ptIds[3] = (i < (nSides - 1)) ? (i + 1) : 0;
        polys->InsertNextCell(4, ptIds);
    }

    //
    // Create the head-back cells.
    //
    for(i = 0; i < nSides; ++i)
    {
        ptIds[0] = i + nSides;
        ptIds[1] = i + nSides * 2;
        ptIds[2] = (i < (nSides - 1)) ? (i + nSides * 2 + 1) : (nSides * 2);
        ptIds[3] = (i < (nSides - 1)) ? (i + nSides + 1) : nSides;
        polys->InsertNextCell(4, ptIds);
    }

    //
    // Create the arrow front cells.
    //
    for(i = 0; i < nSides; ++i)
    {
        ptIds[0] = i + nSides * 2;
        ptIds[1] = endPoint;
        ptIds[2] = (i < (nSides - 1)) ? (i + nSides * 2 + 1) : (nSides * 2);
        polys->InsertNextCell(3, ptIds);
    }

    //
    // Write the cell colors into the array directly
    //
    for(i = 0; i < numCells; ++i)
    {
        unsigned char *rgb = colors->GetPointer(i * 3);
        rgb[0] = 255;
        rgb[1] = 255;
        rgb[2] = 255;
    }

    // Create the mapper for the polydata.
    this->ArrowMapper1 = vtkPolyDataMapper::New();
    this->ArrowMapper1->SetInput(this->ArrowData);
    this->ArrowMapper2 = vtkPolyDataMapper::New();
    this->ArrowMapper2->SetInput(this->ArrowData);
    this->ArrowMapper3 = vtkPolyDataMapper::New();
    this->ArrowMapper3->SetInput(this->ArrowData);
}

void vtkPlaneToolActor::AddPosition(float delta[3])
{
    vtkActor::AddPosition(delta);

    vtkMatrix4x4 *M = vtkMatrix4x4::New();
    vtkMatrix4x4 *tmp = vtkMatrix4x4::New();
    M->Identity();
    (*M)[3][0] = delta[0];
    (*M)[3][1] = delta[1];
    (*M)[3][2] = delta[2];

    vtkMatrix4x4::Multiply4x4(PlaneActor->GetUserMatrix(), M, tmp);
    PlaneActor->SetUserMatrix(tmp);
    vtkMatrix4x4::Multiply4x4(XAxisActor->GetUserMatrix(), M, tmp);
    XAxisActor->SetUserMatrix(tmp);
    vtkMatrix4x4::Multiply4x4(YAxisActor->GetUserMatrix(), M, tmp);
    YAxisActor->SetUserMatrix(tmp);
    vtkMatrix4x4::Multiply4x4(ZAxisActor->GetUserMatrix(), M, tmp);
    ZAxisActor->SetUserMatrix(tmp);

    M->Delete();
    tmp->Delete();
}

void
vtkPlaneToolActor::CopyTransform(vtkActor *actor)
{
    actor->SetPosition(this->Position);
    actor->SetOrigin(this->Origin);
    actor->SetScale(this->Scale);
    actor->SetOrientation(this->Orientation);

    float *wxyz = GetOrientationWXYZ();
    actor->RotateWXYZ(wxyz[0],wxyz[1],wxyz[2],wxyz[3]);

//    actor->SetUserMatrix(this->UserMatrix);
}

void
vtkPlaneToolActor::UpdateActorTransform()
{
    CopyTransform(PlaneActor);
    CopyTransform(XAxisActor);
    CopyTransform(YAxisActor);
    CopyTransform(ZAxisActor);

    float scale[3];
    this->XAxisActor->GetScale(scale);
    this->XAxisActor->SetScale(scale[0] * PLANE_SIZE);
    this->XAxisActor->RotateY(90.);

    this->YAxisActor->GetScale(scale);
    this->YAxisActor->SetScale(scale[0] * PLANE_SIZE);
    this->YAxisActor->RotateX(-90.);

    this->ZAxisActor->GetScale(scale);
    this->ZAxisActor->SetScale(scale[0] * PLANE_SIZE);
}

int vtkPlaneToolActor::RenderOpaqueGeometry(vtkViewport *viewport)
{
    // Redo actor with the new plane if it has been modified in some way.
    if(this->PlaneData == NULL || this->PlaneActor == NULL ||
       this->PlaneMapper == NULL)
    {
        DestroyPlane();
        CreatePlane();
    }

    if(this->ArrowData == NULL || this->ArrowMapper1 == NULL ||
       this->XAxisActor == NULL || this->YAxisActor == NULL || this->ZAxisActor == NULL)
    {
        DestroyVectors();
        CreateVectors();
    }

    UpdateActorTransform();

    int renderCount = 0;    
    renderCount += this->PlaneActor->RenderOpaqueGeometry(viewport);
    renderCount += this->XAxisActor->RenderOpaqueGeometry(viewport);
    renderCount += this->YAxisActor->RenderOpaqueGeometry(viewport);
    renderCount += this->ZAxisActor->RenderOpaqueGeometry(viewport);

    return renderCount;
}

void
vtkPlaneToolActor::ReleaseGraphicsResources(vtkWindow *w)
{
    this->PlaneActor->ReleaseGraphicsResources(w);
    this->XAxisActor->ReleaseGraphicsResources(w);
    this->YAxisActor->ReleaseGraphicsResources(w);
    this->ZAxisActor->ReleaseGraphicsResources(w);
}

void vtkPlaneToolActor::PrintSelf(ostream& os, vtkIndent indent)
{
}

void
vtkPlaneToolActor::SetLineColor(const float c[3])
{
    this->LineColor[0] = c[0];
    this->LineColor[1] = c[1];
    this->LineColor[2] = c[2];
}

