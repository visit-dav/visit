// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtOSPRayVoxelExtractor.C                      //
// ************************************************************************* //

#include <avtOSPRayVoxelExtractor.h>

#include <float.h>

#include <avtAccessor.h>
#include <avtCellList.h>
#include <avtVolume.h>

#include <vtkDataArray.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkTemplateAliasMacro.h>
#include <vtkUnsignedCharArray.h>

#include <DebugStream.h>
#include <StackTimer.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <limits>
#include <math.h>


// ****************************************************************************
//  Method: avtOSPRayVoxelExtractor constructor
//
//  Arguments:
//     w     The number of sample points in the x direction (width).
//     h     The number of sample points in the y direction (height).
//     d     The number of sample points in the z direction (depth).
//     vol   The volume to put samples into.
//     cl    The cell list to put cells whose sampling was deferred.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Nov 19 14:50:58 PST 2004
//    Initialize gridsAreInWorldSpace.
//
//    Jeremy Meredith, Thu Feb 15 13:11:34 EST 2007
//    Added an ability to extract voxels using the world-space version
//    even when they're really in image space.
//
//    Hank Childs, Wed Aug 27 11:11:28 PDT 2008
//    Initialize spatial coordinates array.
//
//    Hank Childs, Wed Dec 24 11:22:43 PST 2008
//    Remove reference to ProportionSpaceToZBufferSpace data member.
//
//    Kathleen Biagas, Fri Jul 13 09:23:55 PDT 2012
//    Use double instead of float.
//
// ****************************************************************************

avtOSPRayVoxelExtractor::avtOSPRayVoxelExtractor(int w, int h, int d,
                                             avtVolume *vol, avtCellList *cl)
    : avtVoxelExtractor(w, h, d, vol, cl)
{
    ospray_core = NULL;
    model_to_screen_transform = vtkMatrix4x4::New();
    screen_to_model_transform = vtkMatrix4x4::New();

    proc  = 0;
    patch = 0;
    drawn = 0;
    imgDims[0] = imgDims[1] = 0;             // size of the patch
    imgLowerLeft[0] = imgLowerLeft[1] = 0;   // coordinates in the whole image
    imgUpperRight[0] = imgUpperRight[1] = 0; // coordinates in the whole image
    eyeSpaceDepth  = -1;
    clipSpaceDepth = -1;

    finalImage = NULL;                         // the image data

}


// ****************************************************************************
//  Method: avtOSPRayVoxelExtractor destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//
//    Hank Childs, Sun Nov 21 10:35:40 PST 2004
//    Delete the view to world transform.
//
//    Hank Childs, Wed Aug 27 11:10:51 PDT 2008
//    Delete the spatial coordinate arrays.
//
//    Hank Childs, Wed Dec 24 11:22:43 PST 2008
//    Remove reference to ProportionSpaceToZBufferSpace data member.
//
// ****************************************************************************

avtOSPRayVoxelExtractor::~avtOSPRayVoxelExtractor()
{
    model_to_screen_transform->Delete();
    screen_to_model_transform->Delete();

    if (finalImage != NULL)
        delete []finalImage;

    finalImage = NULL;
}

// ****************************************************************************
//  Method: avtOSPRayVoxelExtractor::Extract
//
//  Purpose:
//      Extracts the grid into the sample points.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 13:11:34 EST 2007
//    Added an ability to extract voxels using the world-space version
//    even when they're really in image space.
//
//    Hank Childs, Fri Jun  1 16:40:10 PDT 2007
//    Added support for non-scalars.
//
// ****************************************************************************

void
avtOSPRayVoxelExtractor::Extract(vtkRectilinearGrid *rgrid,
                std::vector<std::string> &varnames, std::vector<int> &varsizes)
{
    if (gridsAreInWorldSpace || pretendGridsAreInWorldSpace)
        ExtractWorldSpaceGridOSPRay(rgrid, varnames, varsizes);
    else {
        ospray::Exception("Attempt to extract an image space grid,"
                          "however, RayCasting OSPRay supports only"
                          "world space grid extraction");
    }
}


// ****************************************************************************
//  Method: avtOSPRayVoxelExtractor::ExtractWorldSpaceGridOSPRay
//
//  Purpose:
//      Compute region that patch covers
//
//  Programmer: Pascal Grosset
//  Creation:   August 14, 2016
//
//  Modifications:
//
//    Alister Maguire, Fri Sep 11 13:02:48 PDT 2020
//    I've updated the ghost zone bounding box calculation so that it
//    excludes the entirety of all ghost zones.
//
// ****************************************************************************

void
avtOSPRayVoxelExtractor::ExtractWorldSpaceGridOSPRay(vtkRectilinearGrid *rgrid,
                 std::vector<std::string> &varnames, std::vector<int> &varsize)
{
    StackTimer t0("Calling avtOSPRayVoxelExtractor::"
                  "ExtractWorldSpaceGridOSPRay");
    //=======================================================================//
    // Initialization
    //=======================================================================//
    // Flag to indicate if the patch is drawn
    ospray::Context* ospray = (ospray::Context*)ospray_core;
    drawn = 0;
    
    //=======================================================================//
    // Register data and early skipping
    //=======================================================================//
    int w_min;
    int w_max;
    int h_min;
    int h_max;
    {
        StackTimer t1("avtOSPRayVoxelExtractor::ExtractWorldSpaceGridOSPRay "
                      "Register Data (VisIt preparation)");
        // Some of our sampling routines need a chance to pre-process the data.
        // Register the grid here so we can do that.
        // Stores the values in a structure so that it can be used
        RegisterGrid(rgrid, varnames, varsize);
        // Determine what range we are dealing with on this iteration.
        w_min = restrictedMinWidth;
        w_max = restrictedMaxWidth + 1;
        h_min = restrictedMinHeight;
        h_max = restrictedMaxHeight + 1;
        imgWidth = imgHeight = 0;
        /*
        // Let's find out if this range can even intersect the dataset.
        // If not, just skip it.
        if (!FrustumIntersectsGrid(w_min, w_max, h_min, h_max)) { return; }
        */
    }
    
    //=======================================================================//
    // obtain data pointers & ghost region information
    //=======================================================================//
    void* volumePointer = NULL;
    int   volumeDataType;
    int nX = 0, nY = 0, nZ = 0;
    bool ghost_bound[6] = {false};
    double volumeCube[6];
    {
        StackTimer t1("avtOSPRayVoxelExtractor::ExtractWorldSpaceGridOSPRay "
                      "Compute metadata & ghost boundary "
                      "(Pre-OSPRay preparation)");
        // Calculate patch dimensions for point array and cell array
        //   This is to check if the patch is a cell data or a point data
        //   I have to assume cell dataset has a higher priority
        ospout << "[avtOSPRayVoxelExtractor] "
               << "ncell_arrays " << ncell_arrays << " "
               << "npt_arrays "   << npt_arrays << std::endl;
        if (ncell_arrays > 0) {
            const size_t varIdx = ncell_arrays - 1;
            // const size_t varIdx = 
            //   std::find(varnames.begin(), varnames.end(), ospray->var) - 
            //   varnames.begin();      
            if (DebugStream::Level5() || ospray::visit::CheckVerbose()) {
              ospout << "[avtOSPRayVoxelExtractor] Cell Dataset "
                     << std::endl << std::endl;
              for (int i = 0; i < ncell_arrays; ++i) 
                ospout << "  variable_name: "
                       << rgrid->GetCellData()->GetArray(i)->GetName()
                       << std::endl
                       << "  idx_cell_arrays: " << i << std::endl
                       << "  cell_index["    << i << "] "
                       << cell_index[i]      << std::endl
                       << "  cell_size["     << i << "] "
                       << cell_size[i]       << std::endl
                       << "  cell_vartypes[" << i << "] "
                       << cell_vartypes[i]   << std::endl << std::endl;
              if (rgrid->GetCellData()->GetArray(varIdx)->GetName() !=
                  ospray->GetVariableName())
              {
                ospray::Warning("Error: primary variable " +
                                ospray->GetVariableName() +
                                " not found, found " + 
                                rgrid->GetCellData()->GetArray(varIdx)->GetName() +
                                "), therefore the rendered volume might be wrong.");
              }
              if (cell_size[varIdx] != 1)
              {
                ospray::Warning("Error: non-scalar variable " +
                                ospray->GetVariableName() +
                                " of length " +
                                std::to_string(cell_size[varIdx]) +
                                " found.");
              }
            }
            nX = dims[0] - 1;
            nY = dims[1] - 1;
            nZ = dims[2] - 1;
            volumePointer = cell_arrays[varIdx];
            volumeDataType = cell_vartypes[varIdx];
        }
        else if (npt_arrays > 0) {
            const size_t varIdx = npt_arrays - 1;
            // const size_t varIdx = 
            //   std::find(varnames.begin(), varnames.end(), ospray->var) - 
            //   varnames.begin();
            if (DebugStream::Level5() || ospray::visit::CheckVerbose()) {
              ospout << "[avtOSPRayVoxelExtractor] Point Dataset "
                     << std::endl << std::endl;
              for (int i = 0; i < npt_arrays; ++i)
                ospout << "  variable_name: "
                       << rgrid->GetPointData()->GetArray(i)->GetName()
                       << std::endl
                       << "  idx_pt_arrays: " << i << std::endl
                       << "  pt_index["    << i << "] "
                       << pt_index[i]      << std::endl
                       << "  pt_size["     << i << "] "
                       << pt_size[i]       << std::endl
                       << "  pt_vartypes[" << i << "] "
                       << pt_vartypes[i]   << std::endl << std::endl;
              if (rgrid->GetPointData()->GetArray(varIdx)->GetName() !=
                  ospray->GetVariableName())
              {
                ospray::Warning("Error: primary variable " +
                                ospray->GetVariableName() +
                                " not found (found " + 
                                rgrid->GetCellData()->GetArray(varIdx)->GetName() +
                                "), therefore the rendered volume might be wrong.");
              }
              if (pt_size[varIdx] != 1)
              {
                ospray::Warning("Error: non-scalar variable " +
                                ospray->GetVariableName() +
                                " of length " +
                                std::to_string(pt_size[varIdx]) +
                                " found.");
              }
            }
            nX = dims[0];
            nY = dims[1];
            nZ = dims[2];
            volumePointer = pt_arrays[varIdx];
            volumeDataType = pt_vartypes[varIdx];
        } else {
            ospray::Exception("dataset found is neither nodal nor zonal. "
                              "OSPRay does not know how to handle it.");
        }
        ospout << "[avtOSPRayVoxelExtractor] patch dimension "
               << nX << " " << nY << " " << nZ << std::endl;
        // Calculate ghost region boundaries
        //   ghost_boundaries is an array to indicate if the patch contains
        //   any ghost regions in six different directions
        // Here I assume the patch is larger than 3-cube
        // If not then you might want to dig into this code and see if
        // there will be any special boundary cases
        //
        // debug5 << "VAR: ghost value " << (int)ghosts[0] << std::endl;
        //
        if (ghosts != NULL) {
            int gnX = 0, gnY = 0, gnZ = 0;
            gnX = dims[0] - 1;
            gnY = dims[1] - 1;
            gnZ = dims[2] - 1;  
            for (int y = 1; y < (gnY-1); ++y) {
                for (int z = 1; z < (gnZ-1); ++z) {
                    if (!ghost_bound[0]) {
                        if (ghosts[z*gnY*gnX+y*gnX        ] != 0)
                            { ghost_bound[0] = true; }
                    }
                    if (!ghost_bound[3]) {
                        if (ghosts[z*gnY*gnX+y*gnX+(gnX-1)] != 0)
                            { ghost_bound[3] = true; }
                    }
                    if (ghost_bound[0] && ghost_bound[3]) { break; }
                }
            }
            for (int x = 1; x < (gnX-1); ++x) {
                for (int z = 1; z < (gnZ-1); ++z) {
                    if (!ghost_bound[1]) {
                        if (ghosts[z*gnY*gnX            +x] != 0)
                            { ghost_bound[1] = true; }
                    }
                    if (!ghost_bound[4]) {
                        if (ghosts[z*gnY*gnX+(gnY-1)*gnX+x] != 0)
                            { ghost_bound[4] = true; }
                    }
                    if (ghost_bound[1] && ghost_bound[4]) { break; }
                }
            }
            for (int x = 1; x < (gnX-1); ++x) {
                for (int y = 1; y < (gnY-1); ++y) {
                    if (!ghost_bound[2]) {
                        if (ghosts[                y*gnX+x] != 0) 
                            { ghost_bound[2] = true; }
                    }
                    if (!ghost_bound[5]) {
                        if (ghosts[(gnZ-1)*gnY*gnX+y*gnX+x] != 0)
                            { ghost_bound[5] = true; }
                    }
                    if (ghost_bound[2] && ghost_bound[5]) { break; }
                }
            }
        }
        // Data bounding box
        volumeCube[0] = X[0];
        volumeCube[2] = Y[0];
        volumeCube[4] = Z[0];
        if (ncell_arrays > 0) { 
          volumeCube[1] = X[nX];
          volumeCube[3] = Y[nY];
          volumeCube[5] = Z[nZ];
        } else {
          volumeCube[1] = X[nX-1];
          volumeCube[3] = Y[nY-1];
          volumeCube[5] = Z[nZ-1];
        }
    }

    //=======================================================================//
    // Determine the screen size of the patch being processed
    //=======================================================================//
    int patchScreenExtents[4];
    double patch_center[3];
    double patch_depth;
    {
        StackTimer t1("avtOSPRayVoxelExtractor::ExtractWorldSpaceGridOSPRay "
                      "Get screen size of the patch (Pre-OSPRay preparation)");
        double renderingDepthsExtents[2];
        ospray::ProjectWorldToScreenCube(volumeCube, w_max, h_max, 
                                         viewInfo.imagePan, viewInfo.imageZoom,
                                         model_to_screen_transform, 
                                         patchScreenExtents, 
                                         renderingDepthsExtents);
        xMin = patchScreenExtents[0];
        xMax = patchScreenExtents[1];
        yMin = patchScreenExtents[2];
        yMax = patchScreenExtents[3];
        ospout << "[avtOSPRayVoxelExtractor] patch ghost bounds:"
               << "   " << ghost_bound[0] << " " << ghost_bound[3] 
               << " | " << ghost_bound[1] << " " << ghost_bound[4] 
               << " | " << ghost_bound[2] << " " << ghost_bound[5]
               << std::endl;   
        patch_center[0] = (volumeCube[0] + volumeCube[1])/2.0;
        patch_center[1] = (volumeCube[2] + volumeCube[3])/2.0;
        patch_center[2] = (volumeCube[4] + volumeCube[5])/2.0;        
        patch_depth = // use the norm of patch center as patch depth
            std::sqrt((patch_center[0]-viewInfo.camera[0])*
                      (patch_center[0]-viewInfo.camera[0])+
                      (patch_center[1]-viewInfo.camera[1])*
                      (patch_center[1]-viewInfo.camera[1])+
                      (patch_center[2]-viewInfo.camera[2])*
                      (patch_center[2]-viewInfo.camera[2]));
        eyeSpaceDepth = patch_depth;
        clipSpaceDepth = renderingDepthsExtents[0];
    }

    //=======================================================================//
    // create framebuffer
    //=======================================================================//
    {
        StackTimer t1("avtOSPRayVoxelExtractor::ExtractWorldSpaceGridOSPRay "
                      "Create ImgArray (Pre-OSPRay preparation)");
        ospout << "[avtOSPRayVoxelExtractor] patch extents " 
               << xMin << " " << xMax << " "
               << yMin << " " << yMax << std::endl;
        if (xMin < renderingExtents[0]) { xMin = renderingExtents[0]; }
        if (yMin < renderingExtents[2]) { yMin = renderingExtents[2]; }    
        if (xMax > renderingExtents[1]) { xMax = renderingExtents[1]; }
        if (yMax > renderingExtents[3]) { yMax = renderingExtents[3]; }
        imgWidth  = xMax-xMin;
        imgHeight = yMax-yMin;
        finalImage = new float[((imgWidth)*4) * imgHeight];
    }

    //=======================================================================//
    // Render using OSPRay
    //=======================================================================//
    double volumePBox[6];
    double volumeBBox[6];
    {
        StackTimer t1("avtOSPRayVoxelExtractor::ExtractWorldSpaceGridOSPRay "
                      "Using OSPRay");
        {
            StackTimer t2("avtOSPRayVoxelExtractor::"
                          "ExtractWorldSpaceGridOSPRay "
                          "OSPRay bbox and clip (OSPRay preparation)");
            // shift grid and make it cel centered for cell data
            // for cell centered data, we put the voxel on its left boundary
            volumePBox[0] = X[0];
            volumePBox[1] = Y[0];
            volumePBox[2] = Z[0];
            if (ncell_arrays > 0) { 
              /* zonal data need to occupy a whole cell */
              volumePBox[3] = X[nX];
              volumePBox[4] = Y[nY];
              volumePBox[5] = Z[nZ];
            } else {
              volumePBox[3] = X[nX-1];
              volumePBox[4] = Y[nY-1];
              volumePBox[5] = Z[nZ-1];
            }
            // compute boundingbox and clipping plane for ospray
            if (ncell_arrays > 0) {
                //
                // The idea here is to create a bounding box that excludes
                // ghost zones. The bounding box will later be used by OSPRay
                // to determine clipping bounds, and the portion of the volume
                // that contains ghosts will be clipped away.
                //
                volumeBBox[0] = ghost_bound[0] ? (X[1]): volumePBox[0];
                volumeBBox[1] = ghost_bound[1] ? (Y[1]) : volumePBox[1];
                volumeBBox[2] = ghost_bound[2] ? (Z[1]) : volumePBox[2];
                volumeBBox[3] = ghost_bound[3] ? (X[nX-1]): volumePBox[3];
                volumeBBox[4] = ghost_bound[4] ? (Y[nY-1]): volumePBox[4];
                volumeBBox[5] = ghost_bound[5] ? (Z[nZ-1]): volumePBox[5];
            }
            else {
                volumeBBox[0] = ghost_bound[0] ? X[1] : volumePBox[0];
                volumeBBox[1] = ghost_bound[1] ? Y[1] : volumePBox[1];
                volumeBBox[2] = ghost_bound[2] ? Z[1] : volumePBox[2];
                volumeBBox[3] = ghost_bound[3] ? X[nX-2] : volumePBox[3];
                volumeBBox[4] = ghost_bound[4] ? Y[nY-2] : volumePBox[4];
                volumeBBox[5] = ghost_bound[5] ? Z[nZ-2] : volumePBox[5];
            }
            ospout << "[avtOSPRayVoxelExtractor] patch data position:" 
                   << " " << volumePBox[0]
                   << " " << volumePBox[1]
                   << " " << volumePBox[2]
                   << " |"
                   << " " << volumePBox[3]
                   << " " << volumePBox[4]
                   << " " << volumePBox[5]
                   << std::endl;  
            ospout << "[avtOSPRayVoxelExtractor] patch data bbox:" 
                   << " " << volumeBBox[0]
                   << " " << volumeBBox[1]
                   << " " << volumeBBox[2]
                   << " |"
                   << " " << volumeBBox[3]
                   << " " << volumeBBox[4]
                   << " " << volumeBBox[5]
                   << std::endl; 
        }
        // Create volume and model
        {
            StackTimer t2("avtOSPRayVoxelExtractor::"
                          "ExtractWorldSpaceGridOSPRay "
                          "OSPRay Create Volume");
            ospray->SetupPatch(patch, volumeDataType,
                               (size_t)nX * (size_t)nY * (size_t)nZ,
                               volumePointer, X, Y, Z, nX, nY, nZ,
                               volumePBox, volumeBBox);
        }
        // Render Volume
        {
            StackTimer t2("avtOSPRayVoxelExtractor::"
                          "ExtractWorldSpaceGridOSPRay "
                          "OSPRay Render Volume");
            if ((scalarRange[1] >= tFVisibleRange[0]) &&
                (scalarRange[0] <= tFVisibleRange[1])) {
                ospray->RenderPatch(patch, xMin, xMax, yMin, yMax,
                                    imgWidth, imgHeight, finalImage);
                drawn = 1;
                
            }
        }
    }

    //=======================================================================//
    // Send rays
    //=======================================================================//
    imgDims[0] = imgWidth;
    imgDims[1] = imgHeight;
    imgLowerLeft[0] = xMin;
    imgLowerLeft[1] = yMin;
    imgUpperRight[0] = xMax; 
    imgUpperRight[1] = yMax;

    //=======================================================================//
    // Deallocate memory if not used
    //=======================================================================//
    if (drawn == 0)
    { 
        if (finalImage != NULL) 
        { 
            delete []finalImage; finalImage = NULL; 
        } 
    }
}


// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor::getImageDimensions
//
//  Purpose:
//      Transfers the metadata of the patch
//
//  Programmer: Pascal Grosset
//  Creation:   August 14, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtOSPRayVoxelExtractor::GetImageDimensions(int &inUse, int dims[2],
                                            int screen_ll[2],
                                            int screen_ur[2],
                                            float &eyeDepth,
                                            float &clipDepth)
{
    inUse = drawn;
    dims[0] = imgDims[0];
    dims[1] = imgDims[1];
    screen_ll[0] = imgLowerLeft[0];
    screen_ll[1] = imgLowerLeft[1];
    screen_ur[0] = imgUpperRight[0];
    screen_ur[1] = imgUpperRight[1];
    eyeDepth  = eyeSpaceDepth;
    clipDepth = clipSpaceDepth;
}

// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor::getComputedImage
//
//  Purpose:
//      Allocates space to the pointer address and copy the image generated
//      to it
//
//  Programmer: Pascal Grosset
//  Creation:   August 14, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtOSPRayVoxelExtractor::GetComputedImage(float *image)
{
    memcpy(image, finalImage, imgDims[0]*4*imgDims[1]*sizeof(float));
    if (finalImage != NULL)
        delete []finalImage;
    finalImage = NULL;
}
