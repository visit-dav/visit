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

#include "avtShapeletDecompResult.h"
#include <fstream>
#include <sstream>
using namespace std;

// ****************************************************************************
//  Method:  avtShapeletDecompResult::avtShapeletDecompResult
//
//  Purpose:
//     Constructs a shapelet decomposition result.
//  
//  Programmer:  Cyrus Harrison
//  Creation:    December 12, 2007
//
// ****************************************************************************

avtShapeletDecompResult::avtShapeletDecompResult(double beta,
                                                 int nmax, 
                                                 int width,
                                                 int height,
                                                 const doubleVector &coeffs)
: beta(beta),
  nmax(nmax),
  width(width), height(height),
  coeffs(coeffs)
{;}

// ****************************************************************************
//  Method:  avtShapeletDecompResult::avtShapeletDecompResult
//
//  Purpose:
//     Copy constructor for shaplet decomposition result.
//  
//  Programmer:  Cyrus Harrison
//  Creation:    December 12, 2007
//
// ****************************************************************************

avtShapeletDecompResult::avtShapeletDecompResult
(const avtShapeletDecompResult& dresult)
: beta(dresult.beta),
  nmax(dresult.nmax),
  width(dresult.width), height(dresult.height),
  coeffs(dresult.coeffs)
{;}

// ****************************************************************************
//  Method:  avtShapeletDecompResult::avtShapeletDecompResult
//
//  Purpose:
//     Assignment operator for shapelet decomp result.
//  
//  Programmer:  Cyrus Harrison
//  Creation:    December 12, 2007
//
//  Modifications:
//    Kathleen Bonnell, Wed Jan  2 08:15:07 PST 2008
//    Added return.
//
// ****************************************************************************
avtShapeletDecompResult &
avtShapeletDecompResult::operator=(const avtShapeletDecompResult& dresult)
{
    if(this != &dresult)
    {
        beta   = dresult.beta;
        nmax   = dresult.nmax;
        width  = dresult.width;
        height = dresult.height;
        coeffs = dresult.coeffs;
    }
    return *this;
}


// ****************************************************************************
//  Method:  avtShapeletDecompResult::~avtShapeletDecompResult
//
//  Purpose:
//     Shapelet decomp result destructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 12, 2007
//
// ****************************************************************************

avtShapeletDecompResult::~avtShapeletDecompResult()
{;}

// ****************************************************************************
//  Method:  avtShapeletDecompResult::Coefficent
//
//  Purpose:
//     Gets the coefficent value of basis function n1,n2
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

double 
avtShapeletDecompResult::Coefficient(int n1, int n2) const
{
    // mapping: basis(i,j) is @ 2(i-1) + j+1
    if( (n1 <0 || n1 >= nmax) || (n2 <0 || n2 >= nmax) )
        return 0.0;
    int idx = (nmax - n2);
    idx = coeffs.size() - (idx*idx + idx)/2 + n1;
    return coeffs[idx];
}


// ****************************************************************************
//  Method:  avtShapeletDecompResult::ToString
//
//  Purpose:
//     Creates human readable string rep for the shapelet decomp result.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

string
avtShapeletDecompResult::ToString() const
{
    ostringstream oss;
    oss <<  "Beta   = " << beta   << endl
        <<  "NMax   = " << nmax   << endl
        <<  "Width  = " << width  << endl
        <<  "Height = " << height << endl
        <<  "Coefficients:" << endl
        <<  "[coeffs.size] = " << coeffs.size() << endl;
    for(size_t i=0; i < coeffs.size(); i++)
        oss << coeffs[i] << " ";
    oss  << endl;
    return oss.str();
}



