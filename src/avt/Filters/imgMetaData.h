// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
#ifndef AVT_SLIVR_COMMON_H
#define AVT_SLIVR_COMMON_H
#include <stdio.h>
#include <string>
#include <iostream>

// ****************************************************************************
//  Struct:  imgMetaData
//
//  Purpose:
//    Holds information about patches but not the image 
//
//  Programmer:  
//  Creation:   
//
// ****************************************************************************
struct imgMetaData
{
    int procId;       // processor that produced the patch
    int patchNumber;  // id of the patch on that processor - with procId, acts as a key

    int destProcId;   // destination proc where this patch gets composited

    int inUse;   // whether the patch is composed locally or not
    int dims[2];      // height, width
    int screen_ll[2]; // position in the final image
    int screen_ur[2];

    float avg_z;        // camera space z = depth of the patch - used for compositing
    float eye_z;        // camera space z
    float clip_z;       // clip space z
};


// ****************************************************************************
//  Struct:  imgData
//
//  Purpose:
//    Holds the image data generated
//
//  Programmer:  
//  Creation:    
//
// ****************************************************************************
struct imgData
{
    int procId;         // processor that produced the patch
    int patchNumber;    // id of the patch on that processor  - with procId, acts as a key

    float *imagePatch;  // the image data - RGBA

    bool operator==(const imgData &a){
        return (patchNumber == a.patchNumber);
    }
};


// ****************************************************************************
//  Struct:  convexHull
//
//  Purpose:
//    Holds the image data generated
//
//  Programmer:  
//  Creation:    
//
// ****************************************************************************
struct convexHull
{
    int numPatches;
    int arrangement[3];     // [0] rows along x axis, [1] rows along y axis, [2] rows along z axis

    float extents[6];       // minX, maxX   minY, maxY   minZ, maxZ
    float cellDims[3];      // x, y, z
    float tolerance;        // amount of overlap that is considered ok - typically 2 cells for cell centered data


    // 0: no overlap    1: overlpa in Z    2: overlap in Y    3: overlap in Z
    int overlap(convexHull _hull)
    {

        if ( (_hull.extents[1] < extents[0]) || (_hull.extents[0] > extents[1]) )   // No overlap in X
        {
            if ( (_hull.extents[3] < extents[2]) || (_hull.extents[2] > extents[3]) )   // No overlap in Y
            {
                if ( (_hull.extents[5] < extents[4]) || (_hull.extents[4] > extents[5]) )   // No overlap in Z
                {
                    return 0;
                }
                else
                    return 3;
            }
            else
                return 2;
        }
        else
            return 1;
    }
};

#endif
