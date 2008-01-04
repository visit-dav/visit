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

#include "avtShapeletBasisSet.h"
#include <fstream>
#include <sstream>
using namespace std;

// ****************************************************************************
//  Method:  avtShapeletBasisSet::avtShapeletBasisSet
//
//  Purpose:
//     Constructs a basis set suitable for decomposition and reconstruction.
//  
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************

avtShapeletBasisSet::avtShapeletBasisSet(double beta,
                                         int nmax, 
                                         int width,
                                         int height)
: beta(beta),
  nmax(nmax),
  width(width), height(height)
{
    basisSize = (nmax*nmax + nmax)/2;
    vector<avtShapeletBasis1D *> x_basis_set;
    vector<avtShapeletBasis1D *> y_basis_set;
    
    x_basis_set.resize(nmax);
    for(int i=0;i<nmax;i++)
        x_basis_set[i] = new avtShapeletBasis1D(beta,i,width);
     
    // only create separate 1D y basis if needed   
    if(width != height)
    {
        y_basis_set.resize(nmax);
        for(int i=0;i<nmax;i++)
            y_basis_set[i] = new avtShapeletBasis1D(beta,i,height);
    }
    // resize basisSet with proper # of entries:

    basisSet.resize(basisSize);
    int idx = 0;
    for(int j=0;j<=nmax;j++)
    {
        for(int i=0;i<nmax-j;i++)
        {
            if(width == height)
                basisSet[idx] = new avtShapeletBasis2D(x_basis_set[i],
                                                       x_basis_set[j]);
            else
                basisSet[idx] = new avtShapeletBasis2D(y_basis_set[i],
                                                       y_basis_set[j]);
            idx++;
        }
    }
    
    for(int i=0;i<x_basis_set.size();i++)
        delete x_basis_set[i];
    for(int i=0;i<y_basis_set.size();i++)
        delete y_basis_set[i];
}


// ****************************************************************************
//  Method:  avtShapeletBasisSet::~avtShapeletBasisSet
//
//  Purpose:
//     Shapelet Basis destructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************

avtShapeletBasisSet::~avtShapeletBasisSet()
{
    for(int i=0;i<basisSet.size();i++)
        delete basisSet[i];
}

// ****************************************************************************
//  Method:  avtShapeletBasisSet::GetBasis
//
//  Purpose:
//     Gets a 2D basis object from the set. Returns NULL if the n1/n2 combo
//     is invalid.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************

avtShapeletBasis2D  *
avtShapeletBasisSet::GetBasis(int n1, int n2)
{
    // mapping: basis(i,j) is @ 2(i-1) + j+1
    if( (n1 <0 || n1 >= nmax) || (n2 <0 || n2 >= nmax) )
        return NULL;
    int idx = (nmax - n2);
    idx = basisSize - (idx*idx + idx)/2 + n1;
    return basisSet[idx];
}
