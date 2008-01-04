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
//                              avtGradients.C                               //
// ************************************************************************* //

#include <math.h>

#include <avtGradients.h>

#include <BadIndexException.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtGradients constructor
//
//  Arguments:
//      ng      The number of gradients.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
// ****************************************************************************

avtGradients::avtGradients(int ng)
{
    numGradients = ng;
    gradients    = new double[numGradients*3];
}


// ****************************************************************************
//  Method: avtGradients destructor
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
// ****************************************************************************

avtGradients::~avtGradients()
{
    if (gradients != NULL)
    {
        delete [] gradients;
    }
}


// ****************************************************************************
//  Method: avtGradients::SetGradient
//
//  Purpose:
//      Sets a gradient.
//
//  Arguments:
//      gi      The gradient index.
//      grad    The gradient.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
// ****************************************************************************

void
avtGradients::SetGradient(int gi, const double grad[3])
{
    if (gi >= numGradients || gi < 0)
    {
        EXCEPTION2(BadIndexException, gi, numGradients);
    }

    gradients[3*gi    ] = grad[0];
    gradients[3*gi + 1] = grad[1];
    gradients[3*gi + 2] = grad[2];
}


// ****************************************************************************
//  Method: avtGradients::GetGradient
//
//  Purpose:
//      Gets the gradient.
//
//  Arguments:
//      index     The index of the gradient.
//      grad      The value of the gradient.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 13 15:41:50 PST 2002
//    Do not normalize as we go.
//
// ****************************************************************************

void
avtGradients::GetGradient(int index, double grad[3]) const
{
    if (index >= numGradients || index < 0)
    {
        EXCEPTION2(BadIndexException, index, numGradients);
    }

    grad[0] = gradients[3*index];
    grad[1] = gradients[3*index + 1];
    grad[2] = gradients[3*index + 2];
}


// ****************************************************************************
//  Method: avtGradients::Initialize
//
//  Purpose:
//      Initializes the gradients to all 0.
//
//  Programmer: Hank Childs
//  Creation:   February 11, 2002
//
// ****************************************************************************

void
avtGradients::Initialize(void)
{
    int num = numGradients * 3;
    for (int i = 0 ; i < num ; i++)
    {
        gradients[i] = 0.;
    }
}


// ****************************************************************************
//  Method: avtGradients::Normalize
//
//  Purpose:
//      Normalizes all the gradients.
//
//  Programmer: Hank Childs
//  Creation:   February 11, 2002
//
// ****************************************************************************

void
avtGradients::Normalize(void)
{
    for (int i = 0 ; i < numGradients ; i++)
    {
        int offset = 3*i;
        double normalizer = sqrt(gradients[offset]*gradients[offset] +
                                 gradients[offset+1]*gradients[offset+1] +
                                 gradients[offset+2]*gradients[offset+2]);
        if (normalizer == 0.)
        {
            normalizer = 1.;
        }
        gradients[offset] /= normalizer;
        gradients[offset+1] /= normalizer;
        gradients[offset+2] /= normalizer;
    }
}


// ****************************************************************************
//  Method: avtGradients::PartialAddGradient
//
//  Purpose:
//      Partially adds a gradient.  This allows for one surround voxel of a
//      Sobel operator to contribute its piece without the rest of it being
//      examined yet.
//
//  Programmer: Hank Childs
//  Creation:   February 11, 2002
//
// ****************************************************************************

void
avtGradients::PartialAddGradient(int index, int dimension, double val)
{
    if (index < 0 || index >= numGradients)
    {
        EXCEPTION2(BadIndexException, index, numGradients);
    }

    gradients[3*index + dimension] += val;
}


