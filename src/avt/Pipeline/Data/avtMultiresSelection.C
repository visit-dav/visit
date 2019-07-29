// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "avtMultiresSelection.h"


std::string
avtMultiresSelection::DescriptionString()
{
    char str[1024];
    sprintf(str,
            "avtMultiresSelection:%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%d_%d_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f",
            transformMatrix[0], transformMatrix[1],
            transformMatrix[2], transformMatrix[3],
            transformMatrix[4], transformMatrix[5],
            transformMatrix[6], transformMatrix[7],
            transformMatrix[8], transformMatrix[9],
            transformMatrix[10], transformMatrix[11],
            transformMatrix[12], transformMatrix[13],
            transformMatrix[14], transformMatrix[15],
            viewport[0], viewport[1], viewport[2],
            viewport[3], viewport[4], viewport[5],
            windowSize[0], windowSize[1], 
            viewArea,
            desiredExtents[0], desiredExtents[1],
            desiredExtents[2], desiredExtents[3],
            desiredExtents[4], desiredExtents[5],
            actualExtents[0], actualExtents[1],
            actualExtents[2], actualExtents[3],
            actualExtents[4], actualExtents[5],
            desiredCellArea, actualCellArea);
    return std::string(str);
}


void
avtMultiresSelection::SetCompositeProjectionTransformMatrix(const double matrix[16])
{
    for (int i = 0; i < 16; i++)
        transformMatrix[i] = matrix[i];
}


void
avtMultiresSelection::GetCompositeProjectionTransformMatrix(double matrix[16]) const
{
    for (int i = 0; i < 16; i++)
        matrix[i] = transformMatrix[i];
}


void
avtMultiresSelection::SetViewport(const double vp[6])
{
    for (int i = 0; i < 6; i++)
        viewport[i] = vp[i];
}


void
avtMultiresSelection::GetViewport(double vp[6]) const
{
    for (int i = 0; i < 6; i++)
        vp[i] = viewport[i];
}


void
avtMultiresSelection::SetDesiredExtents(const double extents[6])
{
    for (int i = 0; i < 6; i++)
        desiredExtents[i] = extents[i];
}


void
avtMultiresSelection::GetDesiredExtents(double extents[6]) const
{
    for (int i = 0; i < 6; i++)
        extents[i] = desiredExtents[i];
}


void
avtMultiresSelection::SetActualExtents(const double extents[6])
{
    for (int i = 0; i < 6; i++)
        actualExtents[i] = extents[i];
}


void
avtMultiresSelection::GetActualExtents(double extents[6]) const
{
    for (int i = 0; i < 6; i++)
        extents[i] = actualExtents[i];
}
