/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkOSPRayVolumeRayCastMapper.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOSPRayVolumeRayCastMapper.h"

#include "ospray/ospray.h"
#include "ospray/common/OSPCommon.h"
#include "ospray/volume/BlockBrickedVolume.h"

#include "vtkCamera.h"
#include "vtkDataArray.h"
#include "vtkEncodedGradientEstimator.h"
#include "vtkEncodedGradientShader.h"
#include "vtkFiniteDifferenceGradientEstimator.h"
#include "vtkGarbageCollector.h"
#include "vtkGraphicsFactory.h"
#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkMultiThreader.h"
#include "vtkObjectFactory.h"
#include "vtkPlaneCollection.h"
#include "vtkPointData.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkTimerLog.h"
#include "vtkTransform.h"
#include "vtkVolumeProperty.h"
#include "vtkVolumeRayCastFunction.h"
#include "vtkRayCastImageDisplayHelper.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkColorTransferFunction.h"
#include "vtkPiecewiseFunction.h"
#include "vtkTimerLog.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationDoubleVectorKey.h"

#include "vtkOSPRayRenderer.h"
#include "vtkOSPRayManager.h"

#include <math.h>

#include <DebugStream.h>

vtkStandardNewMacro(vtkOSPRayVolumeRayCastMapper);

vtkCxxSetObjectMacro(vtkOSPRayVolumeRayCastMapper,VolumeRayCastFunction,
        vtkVolumeRayCastFunction );

// A tolerance for bounds, historically equal to 2^(-23) and used
// to counter a small numerical precision issue with the old
// QuickFloor() function.  It should not be needed anymore.
// #define VTK_RAYCAST_FLOOR_TOL 1.1920928955078125e-07
#define VTK_RAYCAST_FLOOR_TOL 0

#define vtkVRCMultiplyPointMacro( A, B, M ) \
    B[0] = A[0]*M[0]  + A[1]*M[1]  + A[2]*M[2]  + M[3]; \
B[1] = A[0]*M[4]  + A[1]*M[5]  + A[2]*M[6]  + M[7]; \
B[2] = A[0]*M[8]  + A[1]*M[9]  + A[2]*M[10] + M[11]; \
B[3] = A[0]*M[12] + A[1]*M[13] + A[2]*M[14] + M[15]; \
if ( B[3] != 1.0 ) { B[0] /= B[3]; B[1] /= B[3]; B[2] /= B[3]; }

#define vtkVRCMultiplyNormalMacro( A, B, M ) \
    B[0] = A[0]*M[0]  + A[1]*M[4]  + A[2]*M[8]; \
B[1] = A[0]*M[1]  + A[1]*M[5]  + A[2]*M[9]; \
B[2] = A[0]*M[2]  + A[1]*M[6]  + A[2]*M[10]

// Construct a new vtkOSPRayVolumeRayCastMapper with default values
vtkOSPRayVolumeRayCastMapper::vtkOSPRayVolumeRayCastMapper()
{
    this->VolumeAdded=false;
    this->NumColors = 128;
    this->SampleDistance             =  1.0;
    this->ImageSampleDistance        =  1.0;
    this->MinimumImageSampleDistance =  1.0;
    this->MaximumImageSampleDistance = 10.0;
    this->AutoAdjustSampleDistances  =  1;
    this->VolumeRayCastFunction      = NULL;

    this->GradientEstimator  = vtkFiniteDifferenceGradientEstimator::New();
    this->GradientShader     = vtkEncodedGradientShader::New();

    this->PerspectiveMatrix      = vtkMatrix4x4::New();
    this->ViewToWorldMatrix      = vtkMatrix4x4::New();
    this->ViewToVoxelsMatrix     = vtkMatrix4x4::New();
    this->VoxelsToViewMatrix     = vtkMatrix4x4::New();
    this->WorldToVoxelsMatrix    = vtkMatrix4x4::New();
    this->VoxelsToWorldMatrix    = vtkMatrix4x4::New();

    this->VolumeMatrix           = vtkMatrix4x4::New();

    this->PerspectiveTransform   = vtkTransform::New();
    this->VoxelsTransform        = vtkTransform::New();
    this->VoxelsToViewTransform  = vtkTransform::New();


    this->ImageMemorySize[0]     = 0;
    this->ImageMemorySize[1]     = 0;

    this->Threader               = vtkMultiThreader::New();

    this->Image                  = NULL;
    this->RowBounds              = NULL;
    this->OldRowBounds           = NULL;

    this->RenderTimeTable        = NULL;
    this->RenderVolumeTable      = NULL;
    this->RenderRendererTable    = NULL;
    this->RenderTableSize        = 0;
    this->RenderTableEntries     = 0;

    this->ZBuffer                = NULL;
    this->ZBufferSize[0]         = 0;
    this->ZBufferSize[1]         = 0;
    this->ZBufferOrigin[0]       = 0;
    this->ZBufferOrigin[1]       = 0;

    this->ImageDisplayHelper     = vtkRayCastImageDisplayHelper::New();

    this->IntermixIntersectingGeometry = 1;

    this->OSPRayManager = vtkOSPRayManager::Singleton();

    this->SharedData = false;
    if (SharedData)
        OSPRayVolume = ospNewVolume("shared_structured_volume");
    else
        OSPRayVolume = ospNewVolume("block_bricked_volume");
    transferFunction = ospNewTransferFunction("piecewise_linear");
    ospCommit(transferFunction);
    SamplingRate=0.25;
}

// Destruct a vtkOSPRayVolumeRayCastMapper - clean up any memory used
vtkOSPRayVolumeRayCastMapper::~vtkOSPRayVolumeRayCastMapper()
{
    if ( this->GradientEstimator )
    {
        this->GradientEstimator->UnRegister(this);
        this->GradientEstimator = NULL;
    }

    this->GradientShader->Delete();

    this->SetVolumeRayCastFunction(NULL);

    this->PerspectiveMatrix->Delete();
    this->ViewToWorldMatrix->Delete();
    this->ViewToVoxelsMatrix->Delete();
    this->VoxelsToViewMatrix->Delete();
    this->WorldToVoxelsMatrix->Delete();
    this->VoxelsToWorldMatrix->Delete();

    this->VolumeMatrix->Delete();

    this->VoxelsTransform->Delete();
    this->VoxelsToViewTransform->Delete();
    this->PerspectiveTransform->Delete();

    this->ImageDisplayHelper->Delete();

    this->Threader->Delete();

    if ( this->Image )
    {
        delete [] this->Image;
    }

    if ( this->RenderTableSize )
    {
        delete [] this->RenderTimeTable;
        delete [] this->RenderVolumeTable;
        delete [] this->RenderRendererTable;
    }

    if ( this->RowBounds )
    {
        delete [] this->RowBounds;
        delete [] this->OldRowBounds;
    }
}

float vtkOSPRayVolumeRayCastMapper::RetrieveRenderTime( vtkRenderer *ren,
        vtkVolume   *vol )
{
    int i;

    for ( i = 0; i < this->RenderTableEntries; i++ )
    {
        if ( this->RenderVolumeTable[i] == vol &&
                this->RenderRendererTable[i] == ren )
        {
            return this->RenderTimeTable[i];
        }
    }

    return 0.0;
}

void vtkOSPRayVolumeRayCastMapper::StoreRenderTime( vtkRenderer *ren,
        vtkVolume   *vol,
        float       time )
{
    int i;
    for ( i = 0; i < this->RenderTableEntries; i++ )
    {
        if ( this->RenderVolumeTable[i] == vol &&
                this->RenderRendererTable[i] == ren )
        {
            this->RenderTimeTable[i] = time;
            return;
        }
    }


    // Need to increase size
    if ( this->RenderTableEntries >= this->RenderTableSize )
    {
        if ( this->RenderTableSize == 0 )
        {
            this->RenderTableSize = 10;
        }
        else
        {
            this->RenderTableSize *= 2;
        }

        float       *oldTimePtr     = this->RenderTimeTable;
        vtkVolume   **oldVolumePtr   = this->RenderVolumeTable;
        vtkRenderer **oldRendererPtr = this->RenderRendererTable;

        this->RenderTimeTable     = new float [this->RenderTableSize];
        this->RenderVolumeTable   = new vtkVolume *[this->RenderTableSize];
        this->RenderRendererTable = new vtkRenderer *[this->RenderTableSize];

        for (i = 0; i < this->RenderTableEntries; i++ )
        {
            this->RenderTimeTable[i] = oldTimePtr[i];
            this->RenderVolumeTable[i] = oldVolumePtr[i];
            this->RenderRendererTable[i] = oldRendererPtr[i];
        }

        delete [] oldTimePtr;
        delete [] oldVolumePtr;
        delete [] oldRendererPtr;
    }

    this->RenderTimeTable[this->RenderTableEntries] = time;
    this->RenderVolumeTable[this->RenderTableEntries] = vol;
    this->RenderRendererTable[this->RenderTableEntries] = ren;

    this->RenderTableEntries++;
}

void vtkOSPRayVolumeRayCastMapper::SetNumberOfThreads( int num )
{
    this->Threader->SetNumberOfThreads( num );
}

int vtkOSPRayVolumeRayCastMapper::GetNumberOfThreads()
{
    if (this->Threader)
    {
        return this->Threader->GetNumberOfThreads();
    }
    return 0;
}

void vtkOSPRayVolumeRayCastMapper::SetGradientEstimator(
        vtkEncodedGradientEstimator *gradest )
{

    // If we are setting it to its current value, don't do anything
    if ( this->GradientEstimator == gradest )
    {
        return;
    }

    // If we already have a gradient estimator, unregister it.
    if ( this->GradientEstimator )
    {
        this->GradientEstimator->UnRegister(this);
        this->GradientEstimator = NULL;
    }

    // If we are passing in a non-NULL estimator, register it
    if ( gradest )
    {
        gradest->Register( this );
    }

    // Actually set the estimator, and consider the object Modified
    this->GradientEstimator = gradest;
    this->Modified();
}

float vtkOSPRayVolumeRayCastMapper::GetGradientMagnitudeScale()
{
    if ( !this->GradientEstimator )
    {
        vtkErrorMacro( "You must have a gradient estimator set to get the scale" );
        return 1.0;
    }

    return this->GradientEstimator->GetGradientMagnitudeScale();
}

float vtkOSPRayVolumeRayCastMapper::GetGradientMagnitudeBias()
{
    if ( !this->GradientEstimator )
    {
        vtkErrorMacro( "You must have a gradient estimator set to get the bias" );
        return 1.0;
    }

    return this->GradientEstimator->GetGradientMagnitudeBias();
}

void vtkOSPRayVolumeRayCastMapper::ReleaseGraphicsResources(vtkWindow *)
{
}

void vtkOSPRayVolumeRayCastMapper::Render( vtkRenderer *ren, vtkVolume *vol )
{
    debug5 << "ALOK: beginning of vtkOSPRayVolumeRayCastMapper::Render" << endl;
    // make sure that we have scalar input and update the scalar input
    if ( this->GetInput() == NULL )
    {
        vtkErrorMacro(<< "No Input!");
        return;
    }
    else
    {
        this->GetInputAlgorithm()->UpdateInformation();
        vtkStreamingDemandDrivenPipeline::SetUpdateExtentToWholeExtent(
                this->GetInputInformation());
        this->GetInputAlgorithm()->Update();
    }
    // vol->UpdateTransferFunctions( ren );

    //
    // OSPRay
    //

    vtkOSPRayRenderer* OSPRayRenderer =
        vtkOSPRayRenderer::SafeDownCast(ren);
    if (!OSPRayRenderer)
    {
        debug5 << "ALOK: vtkOSPRayVolumeRayCastMapper::Render(): no OSPRayRenderer" << endl;
        return;
    }
    debug5 << "ALOK: vtkOSPRayVolumeRayCastMapper::Render(): this->OSPRayManager=" << this->OSPRayManager << endl;
    OSPRayModel = this->OSPRayManager->OSPRayVolumeModel;

    OSPRenderer renderer = this->OSPRayManager->OSPRayVolumeRenderer;

    vtkImageData *data = this->GetInput();
    vtkDataArray * scalars = this->GetScalars(data, this->ScalarMode,
            this->ArrayAccessMode, this->ArrayId, this->ArrayName, this->CellFlag);

    void* ScalarDataPointer =
        this->GetInput()->GetPointData()->GetScalars()->GetVoidPointer(0);
    int ScalarDataType =
        this->GetInput()->GetPointData()->GetScalars()->GetDataType();

    int dim[3];
    data->GetDimensions(dim);

    //
    // Cache timesteps
    //
    double timestep=-1;
    vtkInformation *inputInfo = this->GetInput()->GetInformation();
    // // std::cout << __PRETTY_FUNCTION__ << " (" << this << ") " << "actor: (" <<
    // // OSPRayActor << ") mode: (" << OSPRayActor->OSPRayModel << ") " << std::endl;
    if (inputInfo && inputInfo->Has(vtkDataObject::DATA_TIME_STEP()))
    {
        std::cerr << "has timestep\n";
        timestep = inputInfo->Get(vtkDataObject::DATA_TIME_STEP());
        std::cerr << "timestep time: " << timestep << std::endl;
    }
    vtkOSPRayVolumeCacheEntry* cacheEntry = Cache[vol][timestep];
    if (!cacheEntry)
    {
        cacheEntry = new vtkOSPRayVolumeCacheEntry();
        if (SharedData)
            OSPRayVolume = ospNewVolume("shared_structured_volume");
        else
            OSPRayVolume = ospNewVolume("block_bricked_volume");
        cacheEntry->Volume = OSPRayVolume;
        Cache[vol][timestep] = cacheEntry;

        //
        // Send Volumetric data to OSPRay
        //

        char* buffer = NULL;
        size_t sizeBytes =  (ScalarDataType == VTK_FLOAT) ? dim[0]*dim[1]*dim[2] *sizeof(float) : dim[0]*dim[1]*dim[2] *sizeof(char);

        buffer = (char*)ScalarDataPointer;

        ospSet3i(OSPRayVolume, "dimensions", dim[0], dim[1], dim[2]);
        double origin[3];
        vol->GetOrigin(origin);
        double *bds = data->GetBounds();
        origin[0] = bds[0];
        origin[1] = bds[2];
        origin[2] = bds[4];

        double spacing[3];
        data->GetSpacing(spacing);
        ospSet3f(OSPRayVolume, "gridOrigin", origin[0], origin[1], origin[2]);
        ospSet3f(OSPRayVolume, "gridSpacing", spacing[0],spacing[1],spacing[2]);
        ospSetString(OSPRayVolume, "voxelType", (ScalarDataType == VTK_FLOAT) ? "float" : "uchar");
        if (SharedData)
        {
            OSPData voxelData = ospNewData(sizeBytes, OSP_UCHAR, ScalarDataPointer, OSP_DATA_SHARED_BUFFER);
            ospSetData(OSPRayVolume, "voxelData", voxelData);
        }
        else
        {
            ospSetRegion(OSPRayVolume, ScalarDataPointer, osp::vec3i(0,0,0), osp::vec3i(dim[0],dim[1],dim[2]));
        }
    }
    OSPRayVolume = cacheEntry->Volume;

    // test for modifications to volume properties
    if (vol->GetProperty()->GetMTime() > PropertyTime)
    {
        OSPRayRenderer->SetClearAccumFlag();
        vtkVolumeProperty* volProperty = vol->GetProperty();
        vtkColorTransferFunction* colorTF = volProperty->GetRGBTransferFunction(0);
        vtkPiecewiseFunction *scalarTF = volProperty->GetScalarOpacity(0);
        int numNodes = colorTF->GetSize();
        double* tfData = colorTF->GetDataPointer();

        TFVals.resize(NumColors*3);
        TFOVals.resize(NumColors);
        scalarTF->GetTable(data->GetScalarRange()[0],data->GetScalarRange()[1], NumColors, &TFOVals[0]);
        colorTF->GetTable(data->GetScalarRange()[0],data->GetScalarRange()[1], NumColors, &TFVals[0]);

        OSPData colorData = ospNewData(NumColors, OSP_FLOAT3, &TFVals[0]);// TODO: memory leak?  does ospray manage this>
        ospSetData(transferFunction, "colors", colorData);
        OSPData tfAlphaData = ospNewData(NumColors, OSP_FLOAT, &TFOVals[0]);
        ospSetData(transferFunction, "opacities", tfAlphaData);
        ospCommit(transferFunction);
        // this->SamplingRate = volProperty->GetSamplingRate();
        ospSet1i(OSPRayVolume, "gradientShadingEnabled", volProperty->GetShade());
        // std::cerr << "samplingRate: " << SamplingRate << std::endl;
        // OSPRayRenderer->SetSamples(volProperty->GetSamples());
        PropertyTime.Modified();
    }

    // test for modifications to input
    if (this->GetInput()->GetMTime() > this->BuildTime)
    {
        // if (VolumeAdded)
        // {
        //   VolumeAdded=false;
        // }
        //! Create an OSPRay transfer function.

        std::vector<float> isoValues;
        if (this->GetInput()->GetPointData()->GetScalars("ospIsoValues"))
        {
            int num = this->GetInput()->GetPointData()->GetScalars("ospIsoValues")->GetComponent(0,0);
            for( int i=0; i < num; i++)
            {
                float isoValue = this->GetInput()->GetPointData()->GetScalars("ospIsoValues")->GetComponent(0,i+1);
                isoValues.push_back(isoValue);
            }
        }

        if (isoValues.size())
        {
            OSPData isovaluesData = ospNewData(isoValues.size(), OSP_FLOAT, &isoValues[0]);
            ospSetData(OSPRayVolume, "isovalues", isovaluesData);
        }

        if (this->GetInput()->GetPointData()->GetScalars("ospClipValues"))
        {
            float clipValue = this->GetInput()->GetPointData()->GetScalars("ospClipValues")->GetComponent(0,0);
            int clipAxis = this->GetInput()->GetPointData()->GetScalars("ospClipValues")->GetComponent(0,1);

            std::cout << "clipValue: " << clipValue << std::endl;
            std::cout << "clipAxis: " << clipAxis << std::endl;
            osp::vec3f upper(dim[0],dim[1],dim[2]);
            if (clipAxis == 0)
                upper.x = clipValue;
            else if (clipAxis == 1)
                upper.y = clipValue;
            else if (clipAxis == 2)
                upper.z = clipValue;
            osp::box3f value(osp::vec3f(0,0,0), upper);
            ospSet3fv(OSPRayVolume, "volumeClippingBoxLower", &value.lower.x);
            ospSet3fv(OSPRayVolume, "volumeClippingBoxUpper", &value.upper.x);
        }

        ospSet2f(transferFunction, "valueRange", data->GetScalarRange()[0], data->GetScalarRange()[1]);

        //! Commit the transfer function only after the initial colors and alphas have been set (workaround for Qt signalling issue).
        ospCommit(transferFunction);

        //TODO: manage memory

        ospSetObject((OSPObject)OSPRayVolume, "transferFunction", transferFunction);
        this->BuildTime.Modified();
    }
    ospSet1f(OSPRayVolume, "samplingRate", SamplingRate);
    ospCommit(OSPRayVolume);
    ospAddVolume(OSPRayModel,(OSPVolume)OSPRayVolume);
    // if (!VolumeAdded)
    // VolumeAdded = true;
    ospCommit(OSPRayModel);
    ospSetObject(renderer, "model", OSPRayModel);
    ospCommit(renderer);
    this->OSPRayManager->OSPRayVolumeModel = OSPRayModel;

    OSPRayRenderer->SetHasVolume(true);
    debug5 << "ALOK: end of vtkOSPRayVolumeRayCastMapper::Render" << endl;
}

double vtkOSPRayVolumeRayCastMapper::GetZBufferValue(int x, int y)
{
    int xPos, yPos;

    xPos = static_cast<int>(static_cast<float>(x) * this->ImageSampleDistance);
    yPos = static_cast<int>(static_cast<float>(y) * this->ImageSampleDistance);

    xPos = (xPos >= this->ZBufferSize[0])?(this->ZBufferSize[0]-1):(xPos);
    yPos = (yPos >= this->ZBufferSize[1])?(this->ZBufferSize[1]-1):(yPos);

    return *(this->ZBuffer + yPos*this->ZBufferSize[0] + xPos);
}

int vtkOSPRayVolumeRayCastMapper::ComputeRowBounds(vtkVolume   *vol,
        vtkRenderer *ren)
{
    float voxelPoint[3];
    float viewPoint[8][4];
    int i, j, k;
    unsigned char *ucptr;
    float minX, minY, maxX, maxY, minZ, maxZ;

    minX =  1.0;
    minY =  1.0;
    maxX = -1.0;
    maxY = -1.0;
    minZ =  1.0;
    maxZ =  0.0;

    float bounds[6];
    int dim[3];

    this->GetInput()->GetDimensions(dim);
    bounds[0] = bounds[2] = bounds[4] = 0.0;
    bounds[1] = static_cast<float>(dim[0]-1) - VTK_RAYCAST_FLOOR_TOL;
    bounds[3] = static_cast<float>(dim[1]-1) - VTK_RAYCAST_FLOOR_TOL;
    bounds[5] = static_cast<float>(dim[2]-1) - VTK_RAYCAST_FLOOR_TOL;

    double camPos[3];
    double worldBounds[6];
    vol->GetBounds( worldBounds );
    int insideFlag = 0;
    ren->GetActiveCamera()->GetPosition( camPos );
    if ( camPos[0] >= worldBounds[0] &&
            camPos[0] <= worldBounds[1] &&
            camPos[1] >= worldBounds[2] &&
            camPos[1] <= worldBounds[3] &&
            camPos[2] >= worldBounds[4] &&
            camPos[2] <= worldBounds[5] )
    {
        insideFlag = 1;
    }


    // If we have a simple crop box then we can tighten the bounds
    // See prior explanation of RoundingTieBreaker
    if ( this->Cropping && this->CroppingRegionFlags == 0x2000 )
    {
        bounds[0] = this->VoxelCroppingRegionPlanes[0];
        bounds[1] = this->VoxelCroppingRegionPlanes[1] - VTK_RAYCAST_FLOOR_TOL;
        bounds[2] = this->VoxelCroppingRegionPlanes[2];
        bounds[3] = this->VoxelCroppingRegionPlanes[3] - VTK_RAYCAST_FLOOR_TOL;
        bounds[4] = this->VoxelCroppingRegionPlanes[4];
        bounds[5] = this->VoxelCroppingRegionPlanes[5] - VTK_RAYCAST_FLOOR_TOL;
    }


    // Copy the voxelsToView matrix to 16 floats
    float voxelsToViewMatrix[16];
    for ( j = 0; j < 4; j++ )
    {
        for ( i = 0; i < 4; i++ )
        {
            voxelsToViewMatrix[j*4+i] =
                static_cast<float>(this->VoxelsToViewMatrix->GetElement(j,i));
        }
    }

    // Convert the voxel bounds to view coordinates to find out the
    // size and location of the image we need to generate.
    int idx = 0;
    if ( insideFlag )
    {
        minX = -1.0;
        maxX =  1.0;
        minY = -1.0;
        maxY =  1.0;
        minZ =  0.001;
        maxZ =  0.001;
    }
    else
    {
        for ( k = 0; k < 2; k++ )
        {
            voxelPoint[2] = bounds[4+k];
            for ( j = 0; j < 2; j++ )
            {
                voxelPoint[1] = bounds[2+j];
                for ( i = 0; i < 2; i++ )
                {
                    voxelPoint[0] = bounds[i];
                    vtkVRCMultiplyPointMacro( voxelPoint, viewPoint[idx],
                            voxelsToViewMatrix );

                    minX = (viewPoint[idx][0]<minX)?(viewPoint[idx][0]):(minX);
                    minY = (viewPoint[idx][1]<minY)?(viewPoint[idx][1]):(minY);
                    maxX = (viewPoint[idx][0]>maxX)?(viewPoint[idx][0]):(maxX);
                    maxY = (viewPoint[idx][1]>maxY)?(viewPoint[idx][1]):(maxY);
                    minZ = (viewPoint[idx][2]<minZ)?(viewPoint[idx][2]):(minZ);
                    maxZ = (viewPoint[idx][2]>maxZ)?(viewPoint[idx][2]):(maxZ);
                    idx++;
                }
            }
        }
    }

    if ( minZ < 0.001 || maxZ > 0.9999 )
    {
        minX = -1.0;
        maxX =  1.0;
        minY = -1.0;
        maxY =  1.0;
        insideFlag = 1;
    }

    this->MinimumViewDistance =
        (minZ<0.001)?(0.001):((minZ>0.999)?(0.999):(minZ));

    // We have min/max values from -1.0 to 1.0 now - we want to convert
    // these to pixel locations. Give a couple of pixels of breathing room
    // on each side if possible
    minX = ( minX + 1.0 ) * 0.5 * this->ImageViewportSize[0] - 2;
    minY = ( minY + 1.0 ) * 0.5 * this->ImageViewportSize[1] - 2;
    maxX = ( maxX + 1.0 ) * 0.5 * this->ImageViewportSize[0] + 2;
    maxY = ( maxY + 1.0 ) * 0.5 * this->ImageViewportSize[1] + 2;

    // If we are outside the view frustum return 0 - there is no need
    // to render anything
    if ( ( minX < 0 && maxX < 0 ) ||
            ( minY < 0 && maxY < 0 ) ||
            ( minX > this->ImageViewportSize[0]-1 &&
              maxX > this->ImageViewportSize[0]-1 ) ||
            ( minY > this->ImageViewportSize[1]-1 &&
              maxY > this->ImageViewportSize[1]-1 ) )
    {
        return 0;
    }

    int oldImageMemorySize[2];
    oldImageMemorySize[0] = this->ImageMemorySize[0];
    oldImageMemorySize[1] = this->ImageMemorySize[1];

    // Swap the row bounds
    int *tmpptr;
    tmpptr = this->RowBounds;
    this->RowBounds = this->OldRowBounds;
    this->OldRowBounds = tmpptr;


    // Check the bounds - the volume might project outside of the
    // viewing box / frustum so clip it if necessary
    minX = (minX<0)?(0):(minX);
    minY = (minY<0)?(0):(minY);
    maxX = (maxX>this->ImageViewportSize[0]-1)?
        (this->ImageViewportSize[0]-1):(maxX);
    maxY = (maxY>this->ImageViewportSize[1]-1)?
        (this->ImageViewportSize[1]-1):(maxY);

    // Create the new image, and set its size and position
    this->ImageInUseSize[0] = static_cast<int>(maxX - minX + 1.0);
    this->ImageInUseSize[1] = static_cast<int>(maxY - minY + 1.0);

    // What is a power of 2 size big enough to fit this image?
    this->ImageMemorySize[0] = 32;
    this->ImageMemorySize[1] = 32;
    while ( this->ImageMemorySize[0] < this->ImageInUseSize[0] )
    {
        this->ImageMemorySize[0] *= 2;
    }
    while ( this->ImageMemorySize[1] < this->ImageInUseSize[1] )
    {
        this->ImageMemorySize[1] *= 2;
    }

    this->ImageOrigin[0] = static_cast<int>(minX);
    this->ImageOrigin[1] = static_cast<int>(minY);

    // If the old image size is much too big (more than twice in
    // either direction) then set the old width to 0 which will
    // cause the image to be recreated
    if ( oldImageMemorySize[0] > 2*this->ImageMemorySize[0] ||
            oldImageMemorySize[1] > 2*this->ImageMemorySize[1] )
    {
        oldImageMemorySize[0] = 0;
    }

    // If the old image is big enough (but not too big - we handled
    // that above) then we'll bump up our required size to the
    // previous one. This will keep us from thrashing.
    if ( oldImageMemorySize[0] >= this->ImageMemorySize[0] &&
            oldImageMemorySize[1] >= this->ImageMemorySize[1] )
    {
        this->ImageMemorySize[0] = oldImageMemorySize[0];
        this->ImageMemorySize[1] = oldImageMemorySize[1];
    }

    // Do we already have a texture big enough? If not, create a new one and
    // clear it.
    if ( !this->Image ||
            this->ImageMemorySize[0] > oldImageMemorySize[0] ||
            this->ImageMemorySize[1] > oldImageMemorySize[1] )
    {
        // If there is an image there must be row bounds
        if ( this->Image )
        {
            delete [] this->Image;
            delete [] this->RowBounds;
            delete [] this->OldRowBounds;
        }

        this->Image = new unsigned char[(this->ImageMemorySize[0] *
                this->ImageMemorySize[1] * 4)];

        // Create the row bounds array. This will store the start / stop pixel
        // for each row. This helps eleminate work in areas outside the bounding
        // hexahedron since a bounding box is not very tight. We keep the old ones
        // too to help with only clearing where required
        this->RowBounds = new int [2*this->ImageMemorySize[1]];
        this->OldRowBounds = new int [2*this->ImageMemorySize[1]];

        for ( i = 0; i < this->ImageMemorySize[1]; i++ )
        {
            this->RowBounds[i*2]      = this->ImageMemorySize[0];
            this->RowBounds[i*2+1]    = -1;
            this->OldRowBounds[i*2]   = this->ImageMemorySize[0];
            this->OldRowBounds[i*2+1] = -1;
        }

        ucptr = this->Image;

        for ( i = 0; i < this->ImageMemorySize[0]*this->ImageMemorySize[1]; i++ )
        {
            *(ucptr++) = 0;
            *(ucptr++) = 0;
            *(ucptr++) = 0;
            *(ucptr++) = 0;
        }
    }

    // If we are inside the volume our row bounds indicate every ray must be
    // cast - we don't need to intersect with the 12 lines
    if ( insideFlag )
    {
        for ( j = 0; j < this->ImageInUseSize[1]; j++ )
        {
            this->RowBounds[j*2] = 0;
            this->RowBounds[j*2+1] = this->ImageInUseSize[0] - 1;
        }
    }
    else
    {
        // create an array of lines where the y value of the first vertex is less
        // than or equal to the y value of the second vertex. There are 12 lines,
        // each containing x1, y1, x2, y2 values.
        float lines[12][4];
        float x1, y1, x2, y2;
        int xlow, xhigh;
        int lineIndex[12][2] = {{0,1}, {2,3}, {4,5}, {6,7},
            {0,2}, {1,3} ,{4,6}, {5,7},
            {0,4}, {1,5}, {2,6}, {3,7}};

        for ( i = 0; i < 12; i++ )
        {
            x1 = (viewPoint[lineIndex[i][0]][0]+1.0) *
                0.5*this->ImageViewportSize[0] - this->ImageOrigin[0];

            y1 = (viewPoint[lineIndex[i][0]][1]+1.0) *
                0.5*this->ImageViewportSize[1] - this->ImageOrigin[1];

            x2 = (viewPoint[lineIndex[i][1]][0]+1.0) *
                0.5*this->ImageViewportSize[0] - this->ImageOrigin[0];

            y2 = (viewPoint[lineIndex[i][1]][1]+1.0) *
                0.5*this->ImageViewportSize[1] - this->ImageOrigin[1];

            if ( y1 < y2 )
            {
                lines[i][0] = x1;
                lines[i][1] = y1;
                lines[i][2] = x2;
                lines[i][3] = y2;
            }
            else
            {
                lines[i][0] = x2;
                lines[i][1] = y2;
                lines[i][2] = x1;
                lines[i][3] = y1;
            }
        }

        // Now for each row in the image, find out the start / stop pixel
        // If min > max, then no intersection occurred
        for ( j = 0; j < this->ImageInUseSize[1]; j++ )
        {
            this->RowBounds[j*2] = this->ImageMemorySize[0];
            this->RowBounds[j*2+1] = -1;
            for ( i = 0; i < 12; i++ )
            {
                if ( j >= lines[i][1] && j <= lines[i][3] &&
                        ( lines[i][1] != lines[i][3] ) )
                {
                    x1 = lines[i][0] +
                        (static_cast<float>(j) - lines[i][1])/(lines[i][3] - lines[i][1]) *
                        (lines[i][2] - lines[i][0] );

                    xlow  = static_cast<int>(x1 + 1.5);
                    xhigh = static_cast<int>(x1 - 1.0);

                    xlow = (xlow<0)?(0):(xlow);
                    xlow = (xlow>this->ImageInUseSize[0]-1)?
                        (this->ImageInUseSize[0]-1):(xlow);

                    xhigh = (xhigh<0)?(0):(xhigh);
                    xhigh = (xhigh>this->ImageInUseSize[0]-1)?(
                            this->ImageInUseSize[0]-1):(xhigh);

                    if ( xlow < this->RowBounds[j*2] )
                    {
                        this->RowBounds[j*2] = xlow;
                    }
                    if ( xhigh > this->RowBounds[j*2+1] )
                    {
                        this->RowBounds[j*2+1] = xhigh;
                    }
                }
            }
            // If they are the same this is either a point on the cube or
            // all lines were out of bounds (all on one side or the other)
            // It is safe to ignore the point (since the ray isn't likely
            // to travel through it enough to actually take a sample) and it
            // must be ignored in the case where all lines are out of range
            if ( this->RowBounds[j*2] == this->RowBounds[j*2+1] )
            {
                this->RowBounds[j*2] = this->ImageMemorySize[0];
                this->RowBounds[j*2+1] = -1;
            }
        }
    }

    for ( j = this->ImageInUseSize[1]; j < this->ImageMemorySize[1]; j++ )
    {
        this->RowBounds[j*2] = this->ImageMemorySize[0];
        this->RowBounds[j*2+1] = -1;
    }

    for ( j = 0; j < this->ImageMemorySize[1]; j++ )
    {
        // New bounds are not overlapping with old bounds - clear between
        // old bounds only
        if ( this->RowBounds[j*2+1] < this->OldRowBounds[j*2] ||
                this->RowBounds[j*2]   > this->OldRowBounds[j*2+1] )
        {
            ucptr = this->Image + 4*( j*this->ImageMemorySize[0] +
                    this->OldRowBounds[j*2] );
            for ( i = 0;
                    i <= (this->OldRowBounds[j*2+1] - this->OldRowBounds[j*2]);
                    i++ )
            {
                *(ucptr++) = 0;
                *(ucptr++) = 0;
                *(ucptr++) = 0;
                *(ucptr++) = 0;
            }
        }
        // New bounds do overlap with old bounds
        else
        {
            // Clear from old min to new min
            ucptr = this->Image + 4*( j*this->ImageMemorySize[0] +
                    this->OldRowBounds[j*2] );
            for ( i = 0;
                    i < (this->RowBounds[j*2] - this->OldRowBounds[j*2]);
                    i++ )
            {
                *(ucptr++) = 0;
                *(ucptr++) = 0;
                *(ucptr++) = 0;
                *(ucptr++) = 0;
            }

            // Clear from new max to old max
            ucptr = this->Image + 4*( j*this->ImageMemorySize[0] +
                    this->RowBounds[j*2+1]+1 );
            for ( i = 0;
                    i < (this->OldRowBounds[j*2+1] - this->RowBounds[j*2+1]);
                    i++ )
            {
                *(ucptr++) = 0;
                *(ucptr++) = 0;
                *(ucptr++) = 0;
                *(ucptr++) = 0;
            }

        }
    }

    return 1;
}


void vtkOSPRayVolumeRayCastMapper::ComputeMatrices( vtkImageData *data,
        vtkVolume *vol )
{
    // Get the data spacing. This scaling is not accounted for in
    // the volume's matrix, so we must add it in.
    double volumeSpacing[3];
    data->GetSpacing( volumeSpacing );

    // Get the origin of the data.  This translation is not accounted for in
    // the volume's matrix, so we must add it in.
    float volumeOrigin[3];
    double *bds = data->GetBounds();
    volumeOrigin[0] = bds[0];
    volumeOrigin[1] = bds[2];
    volumeOrigin[2] = bds[4];

    // Get the dimensions of the data.
    int volumeDimensions[3];
    data->GetDimensions( volumeDimensions );

    vtkTransform *voxelsTransform = this->VoxelsTransform;
    vtkTransform *voxelsToViewTransform = this->VoxelsToViewTransform;

    // Get the volume matrix. This is a volume to world matrix right now.
    // We'll need to invert it, translate by the origin and scale by the
    // spacing to change it to a world to voxels matrix.
    this->VolumeMatrix->DeepCopy( vol->GetMatrix() );
    voxelsToViewTransform->SetMatrix( VolumeMatrix );

    // Create a transform that will account for the scaling and translation of
    // the scalar data. The is the volume to voxels matrix.
    voxelsTransform->Identity();
    voxelsTransform->Translate(volumeOrigin[0],
            volumeOrigin[1],
            volumeOrigin[2] );

    voxelsTransform->Scale( volumeSpacing[0],
            volumeSpacing[1],
            volumeSpacing[2] );

    // Now concatenate the volume's matrix with this scalar data matrix
    voxelsToViewTransform->PreMultiply();
    voxelsToViewTransform->Concatenate( voxelsTransform->GetMatrix() );

    // Now we actually have the world to voxels matrix - copy it out
    this->WorldToVoxelsMatrix->DeepCopy( voxelsToViewTransform->GetMatrix() );
    this->WorldToVoxelsMatrix->Invert();

    // We also want to invert this to get voxels to world
    this->VoxelsToWorldMatrix->DeepCopy( voxelsToViewTransform->GetMatrix() );

    // Compute the voxels to view transform by concatenating the
    // voxels to world matrix with the projection matrix (world to view)
    voxelsToViewTransform->PostMultiply();
    voxelsToViewTransform->Concatenate( this->PerspectiveMatrix );

    this->VoxelsToViewMatrix->DeepCopy( voxelsToViewTransform->GetMatrix() );

    this->ViewToVoxelsMatrix->DeepCopy( this->VoxelsToViewMatrix );
    this->ViewToVoxelsMatrix->Invert();
}

void vtkOSPRayVolumeRayCastMapper::InitializeClippingPlanes(
        vtkVolumeRayCastStaticInfo *staticInfo,
        vtkPlaneCollection *planes )
{
    vtkPlane *onePlane;
    double    worldNormal[3], worldOrigin[3];
    double    volumeOrigin[4];
    int      i;
    float    *worldToVoxelsMatrix;
    float    *voxelsToWorldMatrix;
    int      count;
    float    *clippingPlane;
    float    t;

    count = planes->GetNumberOfItems();
    staticInfo->NumberOfClippingPlanes = count;

    if ( !count )
    {
        return;
    }

    worldToVoxelsMatrix = staticInfo->WorldToVoxelsMatrix;
    voxelsToWorldMatrix = staticInfo->VoxelsToWorldMatrix;

    staticInfo->ClippingPlane = new float [4*count];

    // loop through all the clipping planes
    for ( i = 0; i < count; i++ )
    {
        onePlane = static_cast<vtkPlane *>(planes->GetItemAsObject(i));
        onePlane->GetNormal(worldNormal);
        onePlane->GetOrigin(worldOrigin);
        clippingPlane = staticInfo->ClippingPlane + 4*i;
        vtkVRCMultiplyNormalMacro( worldNormal,
                clippingPlane,
                voxelsToWorldMatrix );
        vtkVRCMultiplyPointMacro( worldOrigin, volumeOrigin,
                worldToVoxelsMatrix );

        t = sqrt( clippingPlane[0]*clippingPlane[0] +
                clippingPlane[1]*clippingPlane[1] +
                clippingPlane[2]*clippingPlane[2] );
        if ( t )
        {
            clippingPlane[0] /= t;
            clippingPlane[1] /= t;
            clippingPlane[2] /= t;
        }

        clippingPlane[3] = -(clippingPlane[0]*volumeOrigin[0] +
                clippingPlane[1]*volumeOrigin[1] +
                clippingPlane[2]*volumeOrigin[2]);
    }
}


int vtkOSPRayVolumeRayCastMapper::ClipRayAgainstClippingPlanes(
        vtkVolumeRayCastDynamicInfo *dynamicInfo,
        vtkVolumeRayCastStaticInfo *staticInfo )
{
    float    *clippingPlane;
    int      i;
    float    rayDir[3];
    float    t, point[3], dp;
    float    *rayStart, *rayEnd;

    rayStart = dynamicInfo->TransformedStart;
    rayEnd = dynamicInfo->TransformedEnd;

    rayDir[0] = rayEnd[0] - rayStart[0];
    rayDir[1] = rayEnd[1] - rayStart[1];
    rayDir[2] = rayEnd[2] - rayStart[2];

    // loop through all the clipping planes
    for ( i = 0; i < staticInfo->NumberOfClippingPlanes; i++ )
    {
        clippingPlane = staticInfo->ClippingPlane + 4*i;

        dp =
            clippingPlane[0]*rayDir[0] +
            clippingPlane[1]*rayDir[1] +
            clippingPlane[2]*rayDir[2];

        if ( dp != 0.0 )
        {
            t =
                -( clippingPlane[0]*rayStart[0] +
                        clippingPlane[1]*rayStart[1] +
                        clippingPlane[2]*rayStart[2] + clippingPlane[3]) / dp;

            if ( t > 0.0 && t < 1.0 )
            {
                point[0] = rayStart[0] + t*rayDir[0];
                point[1] = rayStart[1] + t*rayDir[1];
                point[2] = rayStart[2] + t*rayDir[2];

                if ( dp > 0.0 )
                {
                    rayStart[0] = point[0];
                    rayStart[1] = point[1];
                    rayStart[2] = point[2];
                }
                else
                {
                    rayEnd[0] = point[0];
                    rayEnd[1] = point[1];
                    rayEnd[2] = point[2];
                }

                rayDir[0] = rayEnd[0] - rayStart[0];
                rayDir[1] = rayEnd[1] - rayStart[1];
                rayDir[2] = rayEnd[2] - rayStart[2];

            }
            // If the clipping plane is outside the ray segment, then
            // figure out if that means the ray segment goes to zero (if so
            // return 0) or doesn't affect it (if so do nothing)
            else
            {
                if ( dp >= 0.0 && t >= 1.0 )
                {
                    return 0;
                }
                if ( dp <= 0.0 && t <= 0.0 )
                {
                    return 0;
                }
            }
        }
    }

    return 1;
}

int vtkOSPRayVolumeRayCastMapper::ClipRayAgainstVolume(
        vtkVolumeRayCastDynamicInfo *dynamicInfo,
        float bounds[6] )
{
    int    loop;
    float  diff;
    float  t;
    float  *rayStart, *rayEnd, *rayDirection;

    rayStart     = dynamicInfo->TransformedStart;
    rayEnd       = dynamicInfo->TransformedEnd;
    rayDirection = dynamicInfo->TransformedDirection;

    if ( rayStart[0] >= bounds[1] ||
            rayStart[1] >= bounds[3] ||
            rayStart[2] >= bounds[5] ||
            rayStart[0] < bounds[0] ||
            rayStart[1] < bounds[2] ||
            rayStart[2] < bounds[4] )
    {
        for ( loop = 0; loop < 3; loop++ )
        {
            diff = 0;

            if ( rayStart[loop] < (bounds[2*loop]+0.01) )
            {
                diff = (bounds[2*loop]+0.01) - rayStart[loop];
            }
            else if ( rayStart[loop] > (bounds[2*loop+1]-0.01) )
            {
                diff = (bounds[2*loop+1]-0.01) - rayStart[loop];
            }

            if ( diff )
            {
                if ( rayDirection[loop] != 0.0 )
                {
                    t = diff / rayDirection[loop];
                }
                else
                {
                    t = -1.0;
                }

                if ( t > 0.0 )
                {
                    rayStart[0] += rayDirection[0] * t;
                    rayStart[1] += rayDirection[1] * t;
                    rayStart[2] += rayDirection[2] * t;
                }
            }
        }
    }

    // If the voxel still isn't inside the volume, then this ray
    // doesn't really intersect the volume

    if ( rayStart[0] >= bounds[1] ||
            rayStart[1] >= bounds[3] ||
            rayStart[2] >= bounds[5] ||
            rayStart[0] < bounds[0] ||
            rayStart[1] < bounds[2] ||
            rayStart[2] < bounds[4] )
    {
        return 0;
    }

    // The ray does intersect the volume, and we have a starting
    // position that is inside the volume
    if ( rayEnd[0] >= bounds[1] ||
            rayEnd[1] >= bounds[3] ||
            rayEnd[2] >= bounds[5] ||
            rayEnd[0] < bounds[0] ||
            rayEnd[1] < bounds[2] ||
            rayEnd[2] < bounds[4] )
    {
        for ( loop = 0; loop < 3; loop++ )
        {
            diff = 0;

            if ( rayEnd[loop] < (bounds[2*loop]+0.01) )
            {
                diff = (bounds[2*loop]+0.01) - rayEnd[loop];
            }
            else if ( rayEnd[loop] > (bounds[2*loop+1]-0.01) )
            {
                diff = (bounds[2*loop+1]-0.01) - rayEnd[loop];
            }

            if ( diff )
            {
                if ( rayDirection[loop] != 0.0 )
                {
                    t = diff / rayDirection[loop];
                }
                else
                {
                    t = 1.0;
                }

                if ( t < 0.0 )
                {
                    rayEnd[0] += rayDirection[0] * t;
                    rayEnd[1] += rayDirection[1] * t;
                    rayEnd[2] += rayDirection[2] * t;
                }
            }
        }
    }

    // To be absolutely certain our ray remains inside the volume,
    // recompute the ray direction (since it has changed - it is not
    // normalized and therefore changes when start/end change) and move
    // the start/end points in by 1/1000th of the distance.
    float offset;
    offset = (rayEnd[0] - rayStart[0])*0.001;
    rayStart[0] += offset;
    rayEnd[0]   -= offset;

    offset = (rayEnd[1] - rayStart[1])*0.001;
    rayStart[1] += offset;
    rayEnd[1]   -= offset;

    offset = (rayEnd[2] - rayStart[2])*0.001;
    rayStart[2] += offset;
    rayEnd[2]   -= offset;

    if ( rayEnd[0] >= bounds[1] ||
            rayEnd[1] >= bounds[3] ||
            rayEnd[2] >= bounds[5] ||
            rayEnd[0] < bounds[0] ||
            rayEnd[1] < bounds[2] ||
            rayEnd[2] < bounds[4] )
    {
        return 0;
    }

    return 1;
}

void vtkOSPRayVolumeRayCastMapper::UpdateShadingTables( vtkRenderer *ren,
        vtkVolume *vol )
{
    int                   shading;
    vtkVolumeProperty     *volume_property;

    volume_property = vol->GetProperty();

    shading = volume_property->GetShade();

    this->GradientEstimator->SetInputData( this->GetInput() );

    if ( shading )
    {
        this->GradientShader->UpdateShadingTable( ren, vol,
                this->GradientEstimator );
    }
}

float vtkOSPRayVolumeRayCastMapper::GetZeroOpacityThreshold( vtkVolume *vol )
{
    return( this->VolumeRayCastFunction->GetZeroOpacityThreshold( vol ) );
}

// Print method for vtkOSPRayVolumeRayCastMapper
void vtkOSPRayVolumeRayCastMapper::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    os << indent << "Sample Distance: " << this->SampleDistance << "\n";
    os << indent << "Image Sample Distance: "
        << this->ImageSampleDistance << "\n";
    os << indent << "Minimum Image Sample Distance: "
        << this->MinimumImageSampleDistance << "\n";
    os << indent << "Maximum Image Sample Distance: "
        << this->MaximumImageSampleDistance << "\n";
    os << indent << "Auto Adjust Sample Distances: "
        << this->AutoAdjustSampleDistances << "\n";
    os << indent << "Intermix Intersecting Geometry: "
        << (this->IntermixIntersectingGeometry ? "On\n" : "Off\n");

    if ( this->VolumeRayCastFunction )
    {
        os << indent << "Ray Cast Function: " << this->VolumeRayCastFunction<<"\n";
    }
    else
    {
        os << indent << "Ray Cast Function: (none)\n";
    }

    if ( this->GradientEstimator )
    {
        os << indent << "Gradient Estimator: " << (this->GradientEstimator) <<
            endl;
    }
    else
    {
        os << indent << "Gradient Estimator: (none)" << endl;
    }

    if ( this->GradientShader )
    {
        os << indent << "Gradient Shader: " << (this->GradientShader) << endl;
    }
    else
    {
        os << indent << "Gradient Shader: (none)" << endl;
    }

}

//----------------------------------------------------------------------------
void vtkOSPRayVolumeRayCastMapper::ReportReferences(vtkGarbageCollector* collector)
{
    this->Superclass::ReportReferences(collector);
    // These filters share our input and are therefore involved in a
    // reference loop.
    vtkGarbageCollectorReport(collector, this->GradientEstimator,
            "GradientEstimator");
}
