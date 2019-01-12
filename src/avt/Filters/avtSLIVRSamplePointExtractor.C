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
//                     avtSLIVRSamplePointExtractor.C                        //
// ************************************************************************* //

#include <avtSLIVRSamplePointExtractor.h>

#include <float.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkUnsignedCharArray.h>
#include <vtkIdList.h>
#include <vtkRectilinearGrid.h>

#include <avtCellList.h>
#include <avtSLIVRVoxelExtractor.h>
#include <avtParallel.h>
#include <avtSamplePoints.h>
#include <avtVolume.h>

#include <DebugStream.h>
#include <TimingsManager.h>
#include <StackTimer.h>

#include <Utility.h>
#include <DebugStream.h>

#include <limits>
#include <algorithm>


// ****************************************************************************
//  Method: avtSLIVRSamplePointExtractor constructor
//
//  Arguments:
//      w       The width.
//      h       The height.
//      d       The depth.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//     
//  Modifications:
//
//    Hank Childs, Thu Nov 15 15:39:48 PST 2001
//    Moved construction of cell list to Execute to account new limitations of
//    sample points involving multiple variables.
//
//    Hank Childs, Tue Jan  1 10:01:20 PST 2002
//    Initialized sendCells.
//
//    Hank Childs, Sun Dec 14 11:07:56 PST 2003
//    Initialized massVoxelExtractor.
//
//    Hank Childs, Fri Nov 19 13:57:02 PST 2004
//    Initialized rectilinearGridsAreInWorldSpace.
//
//    Hank Childs, Fri Dec 10 09:59:57 PST 2004
//    Initialized shouldDoTiling.
//
//    Hank Childs, Wed Feb  2 08:56:00 PST 2005
//    Initialize modeIs3D.
//
//    Hank Childs, Sun Dec  4 19:12:42 PST 2005
//    Initialize kernelBasedSampling.
//
//    Hank Childs, Tue Jan 24 16:42:40 PST 2006
//    Added point extractor.
//
//    Timo Bremer, Thu Sep 13 14:02:40 PDT 2007
//    Added hex20 extractor.
//
//    Hank Childs, Tue Jan 15 14:26:06 PST 2008
//    Initialize members for sample point arbitration.
//
//    Hank Childs, Fri Jan  9 14:10:25 PST 2009
//    Initialize jitter.
//
//    Mark C. Miller, Thu Oct  2 09:41:37 PDT 2014
//    Initialize lightDirection.
// ****************************************************************************

avtSLIVRSamplePointExtractor::avtSLIVRSamplePointExtractor(int w, int h, int d)
    : avtSamplePointExtractorBase(w, h, d)
{
    slivrVoxelExtractor = NULL;

    patchCount = 0;
    imageMetaPatchVector.clear();
    imgDataHashMap.clear();

    modelViewProj = vtkMatrix4x4::New();

    lighting = false;
    lightPosition[0] = lightPosition[1] = lightPosition[2] = 0.0;
    lightPosition[3] = 1.0;
    lightDirection[0] = 0;
    lightDirection[1] = 0;
    lightDirection[2] = -1;
    materialProperties[0] = 0.4;
    materialProperties[1] = 0.75;
    materialProperties[2] = 0.0;
    materialProperties[3] = 15.0;

    depthBuffer = NULL;
    rgbColorBuffer = NULL;
}


// ****************************************************************************
//  Method: avtSLIVRSamplePointExtractor destructor
//
//  Programmer: Hank Childs
//  Creation:   December 8, 2000
//      
//  Modifications:
//
//    Hank Childs, Sun Dec 14 11:07:56 PST 2003
//    Deleted massVoxelExtractor.
//
//    Hank Childs, Tue Jan 24 16:42:40 PST 2006
//    Deleted pointExtractor.
//
//    Timo Bremer, Thu Sep 13 14:02:40 PDT 2007
//    Deleted hex20Extractor.
//
//    Hank Childs, Tue Jan 15 21:25:01 PST 2008
//    Delete arbitrator.
//
// ****************************************************************************

avtSLIVRSamplePointExtractor::~avtSLIVRSamplePointExtractor()
{
    if (slivrVoxelExtractor != NULL)
    {
        delete slivrVoxelExtractor;
        slivrVoxelExtractor = NULL;
    }

    delImgPatches();
}

// ****************************************************************************
//  Method: avtSLIVRSamplePointExtractor::SetUpExtractors
//
//  Purpose:
//      Sets up the extractors and tell them which volume to extract into.
//
//  Programmer: Hank Childs
//  Creation:   November 15, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jan  1 10:01:20 PST 2002
//    Tell the extractors whether they should extract from large cells.
//
//    Hank Childs, Sun Dec 14 11:07:56 PST 2003
//    Set up massVoxelExtractor.
//
//    Hank Childs, Fri Dec 10 09:59:57 PST 2004
//    Do the sampling in tiles if necessary.
//
//    Hank Childs, Sun Dec  4 19:12:42 PST 2005
//    Add support for kernel based sampling.
//
//    Timo Bremer, Thu Sep 13 14:02:40 PDT 2007
//    Added hex20 extractor.
//
//    Hank Childs, Fri Jan  9 14:11:24 PST 2009
//    Tell extractors whether or not to jitter.  Also remove call to 
//    massVoxelExtractor regarding "sendCellsMode", as it does not participate
//    in that mode ... so the call was worthless.
//
// ****************************************************************************

void
avtSLIVRSamplePointExtractor::SetUpExtractors(void)
{
    StackTimer t0("avtSLIVRSamplePointExtractor::SetUpExtractors");
    avtSamplePoints_p output = GetTypedOutput();

    //
    // This will always be NULL the first time through.  For subsequent tiles
    // (provided we are doing tiling) will not have this issue.
    //
    if (output->GetVolume() == NULL)
        output->SetVolume(width, height, depth);
    else
        output->GetVolume()->ResetSamples();
    output->ResetCellList();
    avtVolume *volume = output->GetVolume();
    if (shouldDoTiling)
        volume->Restrict(width_min, width_max-1, height_min, height_max-1);

    //
    // Set up the extractors and tell them which cell list to use.
    //
    avtCellList *cl = output->GetCellList();

    if (slivrVoxelExtractor != NULL)
    {
        delete slivrVoxelExtractor;
    }
    slivrVoxelExtractor = new avtSLIVRVoxelExtractor(width, height, depth,
                                                                                                         volume,cl);
    //slivrVoxelExtractor->SetJittering(jitter);
    if (shouldDoTiling)
    {
        slivrVoxelExtractor->Restrict(width_min, width_max-1,
                                      height_min, height_max-1);
    }
}

// ****************************************************************************
//  Method: avtSLIVRSamplePointExtractor::InitSampling
//
//  Purpose:
//      Initialize sampling, called by base class ExecuteTree method before.
//      the actual iteration starts. This function might be useful for
//      children classes
//
//  Arguments:
//      dt      The dataset tree that should be processed.
//
//  Programmer: Qi WU 
//  Creation:   April 18, 2018
//
//  Modifications:
//
// ****************************************************************************

void
avtSLIVRSamplePointExtractor::InitSampling(avtDataTree_p dt)
{
    patchCount = 0;
    imageMetaPatchVector.clear();
    imgDataHashMap.clear();
}

// ****************************************************************************
//  Method: avtSLIVRSamplePointExtractor::DoSampling
//
//  Purpose:
//      Performs sampling, called by base class ExecuteTree method.
//
//  Arguments:
//      ds      The data set that should be processed.
//      idx     The index of the dataset.
//
//  Programmer: Kathleen Biagas 
//  Creation:   April 18, 2018
//
//  Modifications:
//
// ****************************************************************************

void
avtSLIVRSamplePointExtractor::DoSampling(vtkDataSet *ds, int idx)
{
    double _scalarRange[2];
    ds->GetScalarRange(_scalarRange);

    double _tfVisibleRange[2];
    _tfVisibleRange[0] = transferFn1D->GetMinVisibleScalar();
    _tfVisibleRange[1] = transferFn1D->GetMaxVisibleScalar();

    slivrVoxelExtractor->SetScalarRange(_scalarRange);
    slivrVoxelExtractor->SetTFVisibleRange(_tfVisibleRange);
    RasterBasedSample(ds, idx);
}


// ****************************************************************************
//  Method: avtSLIVRSamplePointExtractor::RasterBasedSample
//
//  Purpose:
//      Does raster based sampling.
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Hank Childs, Fri Jun  1 12:50:45 PDT 2007
//    Added support for non-scalars.
//
//    Timo Bremer, Thu Sep 13 14:02:40 PDT 2007
//    Added support for hex-20s.
//
//    Hank Childs, Mon Oct 29 20:29:55 PST 2007
//    Ignore surface primitives in 3D.
//
//    Kevin Griffin, Fri Apr 22 16:31:57 PDT 2016
//    Added support for polygons.
//
// ****************************************************************************

void
avtSLIVRSamplePointExtractor::RasterBasedSample(vtkDataSet *ds, int num)
{
    StackTimer t0("avtSLIVRSamplePointExtractor::RasterBasedSample");

    
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
        const double *xform = NULL;
        if (atts.GetRectilinearGridHasTransform())
            xform = atts.GetRectilinearGridTransform();
        avtSamplePoints_p samples = GetTypedOutput();
        int numVars = samples->GetNumberOfRealVariables();
        std::vector<std::string> varnames;
        std::vector<int>         varsizes;
        for (int i = 0 ; i < numVars ; i++)
        {
            varnames.push_back(samples->GetVariableName(i));
            varsizes.push_back(samples->GetVariableSize(i));
        }

        //
        // Compositing Setup
        //
        slivrVoxelExtractor->SetGridsAreInWorldSpace(
            rectilinearGridsAreInWorldSpace, view, aspect, xform);

        slivrVoxelExtractor->setDepthBuffer(depthBuffer,
                                            bufferExtents[1]*bufferExtents[3]);
        slivrVoxelExtractor->setRGBBuffer(rgbColorBuffer,
                                          bufferExtents[1],bufferExtents[3]);
        slivrVoxelExtractor->setBufferExtents(bufferExtents);

        slivrVoxelExtractor->SetViewDirection(view_direction);
        slivrVoxelExtractor->SetMVPMatrix(modelViewProj);
        slivrVoxelExtractor->SetClipPlanes(clipPlanes);
        slivrVoxelExtractor->SetPanPercentages(panPercentage);
        slivrVoxelExtractor->SetDepthExtents(depthExtents);

        slivrVoxelExtractor->setProcIdPatchID(PAR_Rank(),num);

        slivrVoxelExtractor->SetLighting(lighting);
        slivrVoxelExtractor->SetLightDirection(lightDirection);
        slivrVoxelExtractor->SetMatProperties(materialProperties);
        slivrVoxelExtractor->SetTransferFn(transferFn1D);


        slivrVoxelExtractor->Extract((vtkRectilinearGrid *) ds,
                                                                         varnames, varsizes);

        //
        // Get rendering results
        //
        imgMetaData      tmpImageMetaPatch;
        tmpImageMetaPatch = initMetaPatch(patchCount);

        slivrVoxelExtractor->getImageDimensions(tmpImageMetaPatch.inUse,
                                                                                                tmpImageMetaPatch.dims,
                                                                                                tmpImageMetaPatch.screen_ll,
                                                                                                tmpImageMetaPatch.screen_ur,
                                                                                                tmpImageMetaPatch.eye_z,
                                                                                                tmpImageMetaPatch.clip_z);
        if (tmpImageMetaPatch.inUse == 1)
        {
            tmpImageMetaPatch.avg_z = tmpImageMetaPatch.eye_z;
            tmpImageMetaPatch.destProcId = tmpImageMetaPatch.procId;
            imageMetaPatchVector.push_back(tmpImageMetaPatch);

            imgData tmpImageDataHash;
            tmpImageDataHash.procId = tmpImageMetaPatch.procId;
            tmpImageDataHash.patchNumber = tmpImageMetaPatch.patchNumber;
            tmpImageDataHash.imagePatch =
                                new float[tmpImageMetaPatch.dims[0] *
                                                  tmpImageMetaPatch.dims[1] * 4];

            slivrVoxelExtractor->getComputedImage(tmpImageDataHash.imagePatch);
            imgDataHashMap.insert
                                (std::pair<int, imgData>(tmpImageDataHash.patchNumber,
                                                                                 tmpImageDataHash));

            patchCount++;
        }
    }
}


// ****************************************************************************
//  Method: avtSLIVRSamplePointExtractor::SendJittering
//
//  Purpose:
//      Tell the individual cell extractors whether or not to jitter.
//
//  Arguments:
//      j     true if the cell extractors should jitter
//
//  Programmer: Hank Childs
//  Creation:   January 9, 2009
//
// ****************************************************************************

void
avtSLIVRSamplePointExtractor::SendJittering()
{
    if (slivrVoxelExtractor != NULL)
    {
        slivrVoxelExtractor->SetJittering(jitter);
    }
}


// ****************************************************************************
//  Method:  avtSLIVRSamplePointExtractor::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************

bool
avtSLIVRSamplePointExtractor::FilterUnderstandsTransformedRectMesh()
{
    return true;
}


// ****************************************************************************
//  Method: avtSLIVRSamplePointExtractor::delImgPatches
//
//  Purpose:
//      allocates space to the pointer address and copy the image generated to it
//
//  Programmer: 
//  Creation:   
//
//  Modifications:
//
// ****************************************************************************
void
avtSLIVRSamplePointExtractor::delImgPatches()
{
    imageMetaPatchVector.clear();

    for (iter_t it=imgDataHashMap.begin(); it!=imgDataHashMap.end(); it++)
    {
        if ((*it).second.imagePatch != NULL)
            delete [](*it).second.imagePatch;

        (*it).second.imagePatch = NULL;
    }

    imgDataHashMap.clear();
}



// ****************************************************************************
//  Method: avtSLIVRSamplePointExtractor::getImgData
//
//  Purpose:
//      copies a patchover
//
//  Programmer: 
//  Creation:   
//
//  Modifications:
//
// ****************************************************************************
void 
avtSLIVRSamplePointExtractor::getnDelImgData(int patchId, imgData &tempImgData)
{
    iter_t it = imgDataHashMap.find(patchId);

    tempImgData.procId = it->second.procId;
    tempImgData.patchNumber = it->second.patchNumber;
    memcpy(tempImgData.imagePatch,it->second.imagePatch,imageMetaPatchVector[patchId].dims[0] * 4 * imageMetaPatchVector[patchId].dims[1] * sizeof(float));

    delete [](*it).second.imagePatch;
    it->second.imagePatch = NULL;
}


// ****************************************************************************
//  Method: avtSLIVRSamplePointExtractor::
//
//  Purpose:
//      allocates space to the pointer address and copy the image generated to it
//
//  Programmer: 
//  Creation:   
//
//  Modifications:
//
// ****************************************************************************
imgMetaData
avtSLIVRSamplePointExtractor::initMetaPatch(int id)
{
    imgMetaData temp;
    temp.inUse = 0;
    temp.procId = PAR_Rank();
    temp.destProcId = PAR_Rank();
    temp.patchNumber = id;
    temp.dims[0] = temp.dims[1] = -1;
    temp.screen_ll[0] = temp.screen_ll[1] = -1;
    temp.screen_ur[0] = temp.screen_ur[1] = -1;
    temp.avg_z = -1.0;
    temp.eye_z = -1.0;
    temp.clip_z = -1.0;

    return temp;
}
