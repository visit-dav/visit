/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                          avtHexahedron27Extractor.C                       //
// ************************************************************************* //

#include <avtHexahedron27Extractor.h>

#include <float.h>

#include <avtCellList.h>
#include <avtVolume.h>

#include <ImproperUseException.h>


//
// This is a list of parameter values in natural coordinates for vtk
// hexes using different number of control points. Since the vertex
// numbering between the different cases (Hex8, Hex20, Hex27 etc.) is
// consistent we can use the same table for all cases. The coordinates
// are given as (s,t,u) where s is the coordinate between vertex 0 and
// vertex 1; t between vertex 0 and vertex 3; and u between vertex 0
// and 4. 
//
const int avtHexahedron27Extractor::sControlPointParam[27][3] = 
{
    {-1, -1, -1}, // vertex 0
    { 1, -1, -1}, // vertex 1
    { 1,  1, -1}, // vertex 2
    {-1,  1, -1}, // vertex 3
    {-1, -1,  1}, // vertex 4
    { 1, -1,  1}, // vertex 5
    { 1,  1,  1}, // vertex 6
    {-1,  1,  1}, // vertex 7

    { 0, -1, -1}, // midpoint 0 - 1
    { 1,  0, -1}, // midpoint 1 - 2
    { 0,  1, -1}, // midpoint 2 - 3
    {-1,  0, -1}, // midpoint 3 - 0
  
    { 0, -1,  1}, // midpoint 4 - 5
    { 1,  0,  1}, // midpoint 5 - 6
    { 0,  1,  1}, // midpoint 6 - 7
    {-1,  0,  1}, // midpoint 7 - 4
  
    {-1, -1,  0}, // midpoint 0 - 4
    { 1, -1,  0}, // midpoint 1 - 5
    { 1,  1,  0}, // midpoint 2 - 6
    {-1,  1,  0}, // midpoint 3 - 7

    { 0, -1,  0}, // midpoint 0-1-5-4
    { 1,  0,  0}, // midpoint 1-2-6-5
    { 0,  1,  0}, // midpoint 2-3-7-6
    {-1,  0,  0}, // midpoint 3-0-4-7
    { 0,  0, -1}, // midpoint 0-1-2-3
    { 0,  0,  1}, // midpoint 4-5-6-7
  
    { 0,  0,  0}, // center
};

//
// Given a 3-by-3 grid in which vertices are numbered as a hex27 this
// list defines the vertex order o fthe 8 sub hex8 hexes
//
const unsigned char avtHexahedron27Extractor::sSubHexCorners[8][8] = 
{
    {0,   8, 24, 11, 16, 20, 26, 23}, //left-front-low
    {8,   1,  9, 24, 20, 17, 21, 26}, //right-front-low
    {11, 24, 10,  3, 23, 26, 22, 19}, //left-back-low
    {24,  9,  2, 10, 26, 21, 18, 22}, //right-back-low

    {16, 20, 26, 23,  4, 12, 25, 15}, //left-front-high
    {20, 17, 21, 26, 12,  5, 13, 25}, //right-front-high
    {23, 26, 22, 19, 15, 25, 14,  7}, //left-back-high
    {26, 21, 18, 22, 25, 13,  6, 14}  //right-back-high
};


const  avtHexahedron27Extractor::DomainAppStyle 
       avtHexahedron27Extractor::DEFAULT_DOMAIN_APPROXIMATION = avtHex27Linear;


// ****************************************************************************
//  Method: avtHexahedron27Extractor constructor
//
//  Arguments:
//     w     The number of sample points in the x direction (width).
//     h     The number of sample points in the y direction (height).
//     d     The number of sample points in the z direction (depth).
//     vol   The volume to put samples into.
//     cl    The cell list to put cells whose sampling was deferred.
//
//  Programmer: Timo Bremer
//  Creation:   September 13, 2007
//
// ****************************************************************************

avtHexahedron27Extractor::avtHexahedron27Extractor(int w, int h, int d,
                                               avtVolume *vol, avtCellList *cl)
    : avtHexahedronExtractor(w, h, d, vol, cl)
{
    domainApproximation = DEFAULT_DOMAIN_APPROXIMATION;
}


// ****************************************************************************
//  Method: avtHexahedron27Extractor destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Timo Bremer
//  Creation:   September 13, 2007
//
// ****************************************************************************

avtHexahedron27Extractor::~avtHexahedron27Extractor()
{
    ;
}


// ****************************************************************************
//  Method: avtHexahedron27Extractor::Extract
//
//  Purpose:
//      Extracts the sample points from the hexahedron.  Does this by taking
//      slices along planes with constant x that contains sample points.  Then
//      sends the planes down to a base class method for finding the sample
//      points from a polygon.
//
//  Arguments:
//      hex       The hexahedron-27 to extract from.
//
//  Programmer:   Timo Bremer
//  Creation:     September 13, 2007
//
// ****************************************************************************

void
avtHexahedron27Extractor::Extract(const avtHexahedron27 &hex)
{
    // Get the overall number of samples we expect from this hex
    int potentialNumSamples = ConstructBounds(hex.pts, 8);
  
    // If the number is too small we want to treat this hex as a special
    // "small" cell
    if (potentialNumSamples <= 0)
        ContributeSmallCell(hex.pts, hex.val, 8);
            return;
  
    // If the number is too large we want to send off this cell to
    // somewhere else
    /*
    // This is code that needs to be added somewhere else for the hex27 case 
    if (sendCellsMode && potentialNumSamples > 64)
    {
        celllist->Store(hex, minx, maxx, miny, maxy);
        return;
    }
    */

    // Store a pointer to our currrent hex (used in the StoreRay function)
    currentHex = &hex;

    switch (domainApproximation) 
    {
      case avtHex27Constant:
        ConstantHexExtract(hex);
        break;
      case avtHex27Linear:
        LinearHexExtract(hex);
        break;
      case avtHex27Quadratic:
        QuadraticHexExtract(hex);
        break;
    }
}


// ****************************************************************************
//  Method: avtHexahedron27Extractor::ConstantHexExtract
//
//  Purpose:
//    This function extracts the samples assuming that the parameter
//    space is a single linear hex. This allows us to get the parameter
//    values of the samples using the code from the hex8 case using only
//    the first eight vertices to define the parameter space
//
//  Arguments:
//      hex       The hexahedron-27 to extract from.
//
//  Programmer:   Timo Bremer
//  Creation:     September 13, 2007
//
// ****************************************************************************

void 
avtHexahedron27Extractor::ConstantHexExtract(const avtHexahedron27 &hex)
{
    avtHexahedron p_hex; // the parameter hex

    // Now we must set the s,t,u parameter for each corner.
    p_hex.nVars = 3;

    // First we copy the coordinates of the eight corners
    for (int i = 0 ; i < 8 ; i++) 
    {
        for (int k = 0 ; k < 3 ; k++) 
        {
            p_hex.pts[i][k] = hex.pts[i][k];
            p_hex.val[i][k] = sControlPointParam[i][k];
        }
    }
  
    // Use the linear hex code to extract the parameter values  
    avtHexahedronExtractor::Extract(p_hex);
}  


// ****************************************************************************
//  Method: avtHexahedron27Extractor::LinearHexExtract
//
//  Purpose:
//    This function extracts the samples assuming that the parameter
//    space is a curves quadratic hex which we approximate using eight
//    linear hexes. This allows us to get the parameter values of the
//    samples using the code from the hex8 case applied to the eight sub-hexes 
//
//  Arguments:
//      hex       The hexahedron-27 to extract from.
//
//  Programmer:   Timo Bremer
//  Creation:     September 13, 2007
//
// ****************************************************************************

void 
avtHexahedron27Extractor::LinearHexExtract(const avtHexahedron27 &hex)
{
    avtHexahedron p_hex; // the parameter hex
    int i,j,k;

    // Now we must set the s,t,u parameter for each corner.
    p_hex.nVars = 3;
  
    for (i = 0 ; i < 8 ; i++) 
    { 
        // For the eight sub-hexes
        for (j=0;j<8;j++) 
        { 
            // Copy the values of their corners into a standard hex
            for (k=0;k<3;k++) 
            {
                p_hex.pts[j][k] = hex.pts[sSubHexCorners[i][j]][k];
                p_hex.val[j][k] = sControlPointParam[sSubHexCorners[i][j]][k];
            }
        }
        // Use the linear hex code to extract the parameter values  
        avtHexahedronExtractor::Extract(p_hex);
    }
}


// ****************************************************************************
//  Method: avtHexahedron27Extractor::QuadraticHexExtract
//
//  Purpose:
//     Code to compute the samples in the actual curved parameter space
//
//  Arguments:
//      hex       The hexahedron-27 to extract from.
//
//  Programmer:   Timo Bremer
//  Creation:     September 13, 2007
//
// ****************************************************************************

void 
avtHexahedron27Extractor::QuadraticHexExtract(const avtHexahedron27 &hex)
{
    cerr << "avtHexahedron27Extractor::quadraticHexExtract not implemented yet"
         << endl;
    EXCEPTION0(ImproperUseException);
}



// ****************************************************************************
//  Method: avtHexahedron27Extractor::QuadraticHexExtract
//
//  Purpose:
//     Overloaded function to intercept the samples the hex8 code wants to
//     store. We setup the hex8 code to compute the three parameter values
//     for each sample point. This function computes the corresponding
//     function value(s) and stores them in the tmp structure to send on.
//
//  Programmer:   Timo Bremer
//  Creation:     September 13, 2007
//
// ****************************************************************************

void  
avtHexahedron27Extractor::StoreRay(int x, int y, int frontZ, int backZ,
		                   const float (*samples)[AVT_VARIABLE_LIMIT])
{
    // The temporary ray we will fill up with samples
	float (*tmp)[AVT_VARIABLE_LIMIT] = new float[backZ-frontZ+1][AVT_VARIABLE_LIMIT];
    float weight;
    int index;
  
    for (int i = 0 ; i <= backZ-frontZ ; i++) 
    {
        // Initialize all functions to 0
        for (index = 0 ; index<currentHex->nVars ; index++)
            tmp[i][index] = 0;

#if 1
        // For all points
        for (int k = 0 ; k < 27 ; k++) 
        {
             // Compute the weight they contribute
             weight = QuadraticWeight(k, samples[i][0], samples[i][1],
                                         samples[i][2]);
      
             // For all functions multiply the weight with the corresponding
             // value and add the result to the sample
             for (index = 0 ; index<currentHex->nVars ; index++)
                 tmp[i][index] += weight*currentHex->val[k][index];
        }

#else 
(Test code using trilinear interpolation)
        // For all points
        for (int k = 0 ; k < 8 ; k++) 
        {
            // Compute the weight they contribute
            weight = TriLinearWeight(k, samples[i][0], samples[i][1],
                                        samples[i][2]);
      
            // For all functions multiply the weight with the corresponding
            // value and add the result to the sample
            for (index = 0 ; index<currentHex->nVars ; index++)
                tmp[i][index] += weight*currentHex->val[k][index];
        }
#endif
    }
  
    // Finally store the actual ray sampels
    avtExtractor::StoreRay(x,y,frontZ,backZ,tmp);
	delete [] tmp;
}


// ****************************************************************************
//  Method: avtHexahedron27Extractor::TriLinearWeight
//
//  Purpose:
//     Compute the trilinear weights. Can be used for debugging.
//
//  Programmer:   Timo Bremer
//  Creation:     September 13, 2007
//
// ****************************************************************************

float
avtHexahedron27Extractor::TriLinearWeight(int v_index, float s, float t, 
                                                       float u)
{
    return 0.125*(  (1+sControlPointParam[v_index][0]*s)
		  * (1+sControlPointParam[v_index][1]*t)
		  * (1+sControlPointParam[v_index][2]*u));
}


// ****************************************************************************
//  Method: avtHexahedron27Extractor::QuadraticWeight
//
//  Purpose:
//    Evaluate the shape function of vertex v_index at the given
//    coordinates. As shape function we use the standard hex27 shape
//    functions in natural coordinates
//
//  Programmer:   Timo Bremer
//  Creation:     September 13, 2007
//
// ****************************************************************************

float
avtHexahedron27Extractor::QuadraticWeight(int v_index, float s, float t, 
                                                       float u)
{
    if (v_index < 8) 
    {
        return 0.125*(s*t*u
                      *(s + sControlPointParam[v_index][0])
                      *(t + sControlPointParam[v_index][1])
                      *(u + sControlPointParam[v_index][2])
                      );
    }

    float tmp = 1;

    if (sControlPointParam[v_index][0] != 0)
        tmp *= 0.5*s*(s + sControlPointParam[v_index][0]);
    else
        tmp *= (1-s*s);

    if (sControlPointParam[v_index][1] != 0)
        tmp *= 0.5*t*(t + sControlPointParam[v_index][1]);
    else
        tmp *= (1-t*t);
    
    if (sControlPointParam[v_index][2] != 0)
        tmp *= 0.5*u*(u + sControlPointParam[v_index][2]);
    else
        tmp *= (1-u*u);

    return tmp;
}


// ****************************************************************************
//  Method: avtHexahedron27Extractor::ClosestParametricNeighbor
//
//  Purpose:
//     Another test routine that returns the closest original data point
//     given
//
//  Programmer:   Timo Bremer
//  Creation:     September 13, 2007
//
// ****************************************************************************

float 
avtHexahedron27Extractor::ClosestParametricNeighbor(float s, float t, float u)
{
    float min = 10;
    float tmp;
    int min_index = -1;

    for (int i = 0 ; i < 27 ; i++) 
    {
        tmp = sqrt(pow(sControlPointParam[i][0] - s,2) 
	           + pow(sControlPointParam[i][1] - t,2) 
 	           + pow(sControlPointParam[i][2] - u,2));
        if (tmp < min) 
        {
            min = tmp;
            min_index = i;
        }
    }

    return currentHex->val[min_index][0];
}
      
  
