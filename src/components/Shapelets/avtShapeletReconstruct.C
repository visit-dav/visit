/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <avtShapeletReconstruct.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
using namespace std;

// ****************************************************************************
//  Method:  avtShapeletReconstruct::avtShapeletReconstruct
//
//  Purpose:
//     Constructs a shaplet reconstruction object.
//  
//  Programmer:  Cyrus Harrison
//  Creation:    December 12, 2007
//
// ****************************************************************************

avtShapeletReconstruct::avtShapeletReconstruct()
{;}


// ****************************************************************************
//  Method:  avtShapeletReconstruct::~avtShapeletReconstruct
//
//  Purpose:
//     Shaplet reconstruct destructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************

avtShapeletReconstruct::~avtShapeletReconstruct()
{;}

// ****************************************************************************
//  Method:  avtShapeletReconstruct::Execute
//
//  Purpose:
//     Reconstructs an image given shapelet decompostion and matching 
//     basis_set.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

vtkRectilinearGrid *
avtShapeletReconstruct::Execute(avtShapeletDecompResult *decomp,
                                const string &var_name,
                                avtShapeletBasisSet *basis_set)
{
    int nmax        = decomp->NMax();
    int width       = decomp->Width();  
    int height      = decomp->Height();
    int data_length = width * height;
    // create a rectlinear grid to hold the result
    vtkDoubleArray *x_coords = vtkDoubleArray::New();
    vtkDoubleArray *y_coords = vtkDoubleArray::New();
    vtkDoubleArray *z_coords = vtkDoubleArray::New();
    
    for(int i=0; i<=width; i++) 
        x_coords->InsertNextValue(i);
    for(int i=0; i<=height; i++) 
        y_coords->InsertNextValue(i);
    
    z_coords->InsertNextValue(0.0);
             
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(width+1,height+1,1);
    rgrid->SetXCoordinates(x_coords);
    rgrid->SetYCoordinates(y_coords);
    rgrid->SetZCoordinates(z_coords);  

    double *data_ptr = new double[data_length];
    memset(data_ptr,0,sizeof(double)*data_length);
    
    // scale each basis by is coeff and sum.
    for(int j=0;j<=nmax;j++)
    {
        for(int i=0;i<nmax-j;i++)
        {
            const double *basis_values = basis_set->GetBasis(i,j)->Values();
            double coeff = decomp->Coefficient(i,j);
            for(int i=0; i<data_length; i++)
                data_ptr[i] += coeff *basis_values[i];    
        }
    }
    
    vtkDoubleArray *res_arr = vtkDoubleArray::New();
    res_arr->SetName(var_name.c_str());
    for(int i=0;i<data_length;i++)
        res_arr->InsertNextValue(data_ptr[i]);

    rgrid->GetCellData()->AddArray(res_arr);

    res_arr->Delete();
    x_coords->Delete();
    y_coords->Delete();
    z_coords->Delete();
    delete [] data_ptr;
    return rgrid;
}
// ****************************************************************************
//  Method:  avtShapeletReconstruct::Execute
//
//  Purpose:
//     Reconstructs an image from a shapelet decompostion.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

vtkRectilinearGrid *
avtShapeletReconstruct::Execute(avtShapeletDecompResult *decomp,
                                const string &var_name)
{
    // create the proper basis set.
    avtShapeletBasisSet basis_set(decomp->Beta(),
                                  decomp->NMax(),
                                  decomp->Width(),
                                  decomp->Height());
    // use other reconstruct method
    vtkRectilinearGrid *res = Execute(decomp,var_name,&basis_set);
    return res;
}

