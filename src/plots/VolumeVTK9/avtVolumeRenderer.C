// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtVolumeRenderer.C                           //
// ************************************************************************* //

#include "avtVolumeRenderer.h"

#include <visit-config.h>

#include <StackTimer.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <avtCallback.h>

#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkPiecewiseFunction.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkRenderer.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#ifdef HAVE_OSPRAY
    #include <vtkOSPRayVolumeMapper.h>
    #include <vtkOSPRayRendererNode.h>
#endif

#include <vtkGeometryFilter.h>
#include <vtkPolyDataMapper.h>

#ifndef NO_DATA_VALUE
  #define NO_DATA_VALUE -1e+37
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

// #define LOCAL_DEBUG std::cerr << __LINE__ << "  " << mName
#define LOCAL_DEBUG debug5 << mName

// ****************************************************************************
//  Constructor:  avtVolumeRenderer::avtVolumeRenderer
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2001
//
//  Modifications:
//
// ****************************************************************************
avtVolumeRenderer::avtVolumeRenderer() : avtVisItVTKRenderer()
{
}

// ****************************************************************************
//  Destructor:  avtVolumeRenderer::~avtVolumeRenderer
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2001
//
//  Modifications:
//
// ****************************************************************************
avtVolumeRenderer::~avtVolumeRenderer()
{
}

// ****************************************************************************
//  Method: avtVolumeRenderer::New
//
//  Purpose:
//      A static method that creates the correct type of renderer.
//
//  Returns:    A renderer that is of type derived from this class.
//
//  Programmer: Hank Childs
//  Creation:   April 24, 2002
//
//  Modifications:
//
// ****************************************************************************
avtVolumeRenderer *
avtVolumeRenderer::New(void)
{
    return new avtVolumeRenderer;
}

// ****************************************************************************
//  Method:  avtVolumeRenderer::Render
//
//  Purpose:
//    Does the actual rendering.
//
//  Arguments:
//    ds         The dataset to render
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  1, 2003
//
//  Modifications:
//
// ****************************************************************************

void
avtVolumeRenderer::Render(vtkDataSet *in_ds)
{
    StackTimer t("avtVolumeRenderer::Render");

    const char *mName = "avtVolumeRenderer::Render: ";

    if (VTKRen == nullptr)
    {
        debug1 << mName << "Default Renderer: VTKRen is nullptr!";
        EXCEPTION0(ImproperUseException);
    }

    // Before calling the NeedImage the number of components needs to
    // be known. This step can only be done by examining the
    // vtkDataSet so to get the active variable.
    vtkDataArray *dataArr = in_ds->GetPointData()->GetScalars();

    if( dataArr == nullptr )
    {
        dataArr = in_ds->GetCellData()->GetScalars();

        if( dataArr == nullptr )
        {
            EXCEPTION1(InvalidVariableException, "");
        }
    }

    // There could be a spearate opacity scalar data array.
    std::string activeVariable  = dataArr->GetName();
    std::string opacityVariable = m_atts.GetOpacityVariable();

    // Before calling NeedImage the number of components needs to be
    // known which is done by comparing the data and opacity variable
    // names.
    NumberOfComponents(activeVariable, opacityVariable);

    // The data and opacity ranges must be known before calling
    // UpdateRenderingState.
    if( NeedImage() )
    {
        // Get the local data range so to ignore NO_DATA_VALUE values.
        // dataArr->GetRange( m_dataRange );
        int nTupples = dataArr->GetNumberOfTuples();

        m_dataRange[0] = +FLT_MAX;
        m_dataRange[1] = -FLT_MAX;

        for( int ptId = 0; ptId<nTupples; ++ptId )
        {
            double dataTuple = dataArr->GetTuple1(ptId);
            if (dataTuple < NO_DATA_VALUE)
                continue;
            if (m_dataRange[0] > dataTuple)
                m_dataRange[0] = dataTuple;
            if (m_dataRange[1] < dataTuple)
                m_dataRange[1] = dataTuple;
        }

        // If there are two components get the opacitiy range.
        if( m_nComponents == 2 )
        {
            vtkDataArray *opacityArr =
                in_ds->GetPointData()->GetScalars( opacityVariable.c_str() );

            if( opacityArr == nullptr )
            {
                opacityArr = in_ds->GetCellData()->GetScalars( opacityVariable.c_str() );
                if( opacityArr == nullptr )
                {
                    EXCEPTION1(InvalidVariableException, opacityVariable);
                }
            }

            // Get the local data range so to ignore NO_DATA_VALUE values.
            // opacityArr->GetRange( m_opacityRange );
            int nTupples = opacityArr->GetNumberOfTuples();

            m_opacityRange[0] = +FLT_MAX;
            m_opacityRange[1] = -FLT_MAX;

            for( int ptId = 0; ptId<nTupples; ++ptId )
            {
                double opacityTuple = opacityArr->GetTuple1(ptId);
                if (opacityTuple < NO_DATA_VALUE)
                    continue;
                if (m_opacityRange[0] > opacityTuple)
                    m_opacityRange[0] = opacityTuple;
                if (m_opacityRange[1] < opacityTuple)
                    m_opacityRange[1] = opacityTuple;
            }
        }
    }

    // Update the rendering state.
    UpdateRenderingState(in_ds, VTKRen);

    // NOw do the rendering
    m_volumeMapper->Render(VTKRen, m_volume);
}
