/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
