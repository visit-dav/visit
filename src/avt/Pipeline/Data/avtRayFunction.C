/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                               avtRayFunction.C                            //
// ************************************************************************* //

#include <avtRayFunction.h>

#include <avtLightingModel.h>
#include <avtRay.h>


// ****************************************************************************
//  Method: avtRayFunction constructor
//
//  Arguments:
//      l       The lighting model this object should use for shading.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Sep 11 14:59:30 PDT 2006
//    Initialize primaryVariableIndex and pixelIndices.
//
// ****************************************************************************

avtRayFunction::avtRayFunction(avtLightingModel *l)
{
    lighting = l;
    primaryVariableIndex = 0;
    pixelIndexI = 0;
    pixelIndexJ = 0;
}


// ****************************************************************************
//  Method: avtRayFunction destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtRayFunction::~avtRayFunction()
{
    ;
}


// ****************************************************************************
//  Method: avtRayFunction::CanContributeToPicture
//
//  Purpose:
//      Sampling is one of the costliest portions of doing volume rendering.
//      This is an opportunity to the "cheat" and ask the ray function if it
//      thinks that it will needs this cell to make the final picture.  If the
//      answer is no, the cell is not sampled.
//
//  Arguments:
//      <unused>   The number of vertices for the cell.
//      <unused>   The variables at each vertex.
//
//  Returns:       true if the cell can contribute to the picture, false
//                 otherwise.
//
//  Notes:         Derived types should redefine this if appropriate.
//
//  Programmer:    Hank Childs
//  Creation:      December 7, 2001
//
// ****************************************************************************

bool
avtRayFunction::CanContributeToPicture(int,const float (*)[AVT_VARIABLE_LIMIT])
{
    return true;
}


// ****************************************************************************
//  Method: avtRayFunction::SetGradientVariableIndex
//
//  Purpose:
//      Tells the ray function the index of the gradient variable.  This index
//      corresponds to the x-coordinate and the following two indices are for
//      y and z.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2008
//
// ****************************************************************************

void
avtRayFunction::SetGradientVariableIndex(int gvi)
{
    lighting->SetGradientVariableIndex(gvi);
    gradientVariableIndex = gvi;
}


