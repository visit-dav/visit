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

#define LOCAL_DEBUG std::cerr << __LINE__ << "  " << mName
// #define LOCAL_DEBUG debug5 << mName

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

    // Check for an implied transform - can not be done with the
    // current paradigm!!!!!!
    // avtDataAttributes &inatts = GetInput()->GetInfo().GetAttributes();
    // if (inatts.GetRectilinearGridHasTransform())
    // {
    //     EXCEPTION1(ImproperUseException,
    //                "vtkRectilinear grids with an implied transform can not be rendered.");
    // }


    // Before calling the NeedImage the number of components needs to
    // be known. This step can only be done by examining the data so
    // to get the active variable. At the time the data and opacity
    // ranges must be known before calling  UpdateRenderingState.
    vtkDataArray *dataArr = in_ds->GetPointData()->GetScalars();

    if( dataArr == nullptr )
    {
        dataArr = in_ds->GetCellData()->GetScalars();

        if( dataArr == nullptr )
        {
            EXCEPTION1(InvalidVariableException, "");
        }
    }

    std::string activeVarName  = dataArr->GetName();
    std::string opacityVarName = m_atts.GetOpacityVariable();

    // There could be a spearate opacity scalar data array.
    vtkDataArray *opacityArr = nullptr;

    if( opacityVarName == activeVarName )
    {
       avtCallback::IssueWarning("The opacity variable is the same as "
                                  "the primary variable. Ignoring it and "
                                  "any possible min/max setting.");

        m_nComponents = 1;
    }
    else if( opacityVarName != "default" )
    {
        m_nComponents = 2;
    }
    else
    {
        m_nComponents = 1;
    }

    dataArr->GetRange( m_dataRange );

    if( m_nComponents == 2 )
    {
        opacityArr = in_ds->GetPointData()->GetScalars( opacityVarName.c_str() );

        if( opacityArr == nullptr )
        {
            opacityArr = in_ds->GetCellData()->GetScalars( opacityVarName.c_str() );
            if( opacityArr == nullptr )
            {
                EXCEPTION1(InvalidVariableException, opacityVarName);
            }
        }

        opacityArr->GetRange( m_opacityRange );
    }

    // LOCAL_DEBUG << "nComponents: " << m_nComponents << "  "
    //          << "needImage: " << m_needImage << "  "
    //          << std::endl;

    NeedImage(); // Must be called before UpdateRenderingState

    UpdateRenderingState(in_ds, VTKRen);

    m_volumeMapper->Render(VTKRen, m_volume);
}
