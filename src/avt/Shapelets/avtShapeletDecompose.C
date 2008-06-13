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

#include "avtShapeletDecompose.h"
#include <vtkDataArray.h>
#include <vtkCellData.h>
using namespace std;

// ****************************************************************************
//  Method:  avtShapeletDecompose::avtShapeletDecompose
//
//  Purpose:
//     Constructs a shapelet decompse object.
//  
//  Programmer:  Cyrus Harrison
//  Creation:    December 12, 2007
//
// ****************************************************************************

avtShapeletDecompose::avtShapeletDecompose()
{;}


// ****************************************************************************
//  Method:  avtShapeletDecompose::~avtShapeletDecompose
//
//  Purpose:
//     Shapelet decompose destructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************

avtShapeletDecompose::~avtShapeletDecompose()
{;}


// ****************************************************************************
//  Method:  avtShapeletDecompose::Execute
//
//  Purpose:
//     Decomposes an image using a given shapelet basis set.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
//  Modifications:
//    Cyrus Harrison, Wed Jun 11 15:29:15 PDT 2008
//    Added support to retain the original data extents.
//
// ****************************************************************************

avtShapeletDecompResult * 
avtShapeletDecompose::Execute(vtkRectilinearGrid *rgrid,
                              const string &var_name,
                              avtShapeletBasisSet *basis_set)
{
    double beta     = basis_set->Beta();
    int nmax        = basis_set->NMax();
    int width       = basis_set->Width();   
    int height      = basis_set->Height();
    int data_length = width * height;
    doubleVector extents;
    doubleVector coeffs;
    
    // get original data extents
    double gbounds[6];
    rgrid->GetBounds(gbounds);
    extents.resize(4);
    extents[0] = gbounds[0];
    extents[1] = gbounds[1];
    extents[2] = gbounds[2];
    extents[3] = gbounds[3];
        
    vtkDataArray *var_arr = rgrid->GetCellData()->GetArray(var_name.c_str());

    if(!var_arr)
        return NULL;

    // do decomp in double precision
    double *data_ptr = new double[data_length];
    
    for(int i=0;i<data_length ;i++)
        data_ptr[i] = (double)var_arr->GetTuple1(i);
    
    // use overlap integral method to obtain shaplets coeffs
    for(int j=0;j<=nmax;j++)
    {
        for(int i=0;i<nmax-j;i++)
        {
            const double *basis_values = basis_set->GetBasis(i,j)->Values();
            double res = 0.0;
            for(int i=0; i<data_length; i++)
                res += basis_values[i] * data_ptr[i];        
            coeffs.push_back(res);
        }
    }

    delete [] data_ptr;
    avtShapeletDecompResult *decomp = new avtShapeletDecompResult(beta,
                                                                  nmax,
                                                                  width,
                                                                  height,
                                                                  extents,
                                                                  coeffs);
    return decomp;
}


