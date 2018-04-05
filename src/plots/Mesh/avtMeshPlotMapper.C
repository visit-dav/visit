/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                        avtMeshPlotMapper.C                                //
// ************************************************************************* //

#include <avtMeshPlotMapper.h>

#include <vtkActor.h>
#include <vtkMeshPlotMapper.h>
#include <vtkProperty.h>

// Initialize the object factory that create the vtkOpenGL override 
#include <vtkAutoInit.h>
VTK_MODULE_INIT(MeshPlot);



// ****************************************************************************
//  Method: avtMeshPlotMapper constructor
//
//  Programmer: Kathleen Biagas
//  Creation:   June 30, 2016 
//
//  Modifications:
//
// ****************************************************************************

avtMeshPlotMapper::avtMeshPlotMapper()
{
    lineWidth = 1;
    linesColor[0] = linesColor[1] = linesColor[2] = 0.;
    polysColor[0] = polysColor[1] = polysColor[2] = 1.;
    opacity = 1.;
    surfaceVis = true;
}


// ****************************************************************************
//  Method: avtMeshPlotMapper destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   June 30, 2016
//
// ****************************************************************************

avtMeshPlotMapper::~avtMeshPlotMapper()
{
}


// ****************************************************************************
//  Method: avtMeshPlotMapper::CreateMapper
//
//  Purpose:
//      A hook to allow derived types to insert their own types of mappers.
// 
//  Returns:    A mesh-plot-specific mapper.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 30, 2016 
//
// ****************************************************************************

vtkDataSetMapper *
avtMeshPlotMapper::CreateMapper(void)
{
    return (vtkDataSetMapper *)vtkMeshPlotMapper::New();
}

void
avtMeshPlotMapper::CustomizeMappers()
{
    for (int i = 0; i < nMappers; ++i)
    {
        if (mappers[i] != NULL)
        {
            vtkMeshPlotMapper *m = (vtkMeshPlotMapper*) mappers[i];
            if (m != NULL)
            {
                m->SetScalarVisibility(false);
                m->SetUsePolys(surfaceVis);
                m->SetLinesColor(linesColor);
                m->SetPolysColor(polysColor);
                m->SetResolveCoincidentTopologyToPolygonOffset();
                m->SetResolveCoincidentTopologyLineOffsetParameters(1., 1.);
                m->SetOpacity(opacity);
            }
            vtkProperty *prop = actors[i]->GetProperty();
            prop->SetLineWidth(lineWidth);
            prop->SetAmbient(1.);
            prop->SetDiffuse(0.);
        }
    }
}


// ****************************************************************************
//  Method: avtMeshPlotMapper::SetSurfaceVisibility
//
//  Purpose:
//      Toggles surface visibility.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 16, 2016 
//
//  Modifications:
//
// ****************************************************************************

void
avtMeshPlotMapper::SetSurfaceVisibility(bool val)
{
    if (surfaceVis != val)
    {
        surfaceVis = val;
        for (int i = 0; i < nMappers; ++i)
        {
            if (mappers[i] != NULL)
            {
                vtkMeshPlotMapper *m = (vtkMeshPlotMapper*)mappers[i];
                if (m != NULL)
                    m->SetUsePolys(val);
            }
        }
    }
}

void
avtMeshPlotMapper::SetMeshColor(double rgb[3])
{
    linesColor[0] = rgb[0];
    linesColor[1] = rgb[1];
    linesColor[2] = rgb[2];
    for (int i = 0; i < nMappers; ++i)
    {
        if (mappers[i] != NULL)
        {
            vtkMeshPlotMapper *m = (vtkMeshPlotMapper *)mappers[i];
            if (m != NULL)
                m->SetLinesColor(rgb);
        }
    }
}

void
avtMeshPlotMapper::SetSurfaceColor(double rgb[3])
{
    polysColor[0] = rgb[0];
    polysColor[1] = rgb[1];
    polysColor[2] = rgb[2];
    for (int i = 0; i < nMappers; ++i)
    {
        if (mappers[i] != NULL)
        {
            vtkMeshPlotMapper *m = (vtkMeshPlotMapper *)mappers[i];
            if (m != NULL)
                m->SetPolysColor(rgb);
        }
    }
}

void
avtMeshPlotMapper::SetOpacity(double val)
{
    opacity = val;
    for (int i = 0; i < nMappers; ++i)
    {
        if (mappers[i] != NULL)
        {
            vtkMeshPlotMapper *m = (vtkMeshPlotMapper *)mappers[i];
            if (m != NULL)
                m->SetOpacity(opacity);
        }
    }
}

void
avtMeshPlotMapper::SetLineWidth(int lw)
{
    lineWidth = lw;
    for (int i = 0; i < nMappers; ++i)
    {
        if (actors[i] != NULL)
        {
            actors[i]->GetProperty()->SetLineWidth(lw);
        }
    }
}

