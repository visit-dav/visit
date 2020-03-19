// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Modifications:
//    Cyrus Harrison, Wed Jun 11 15:29:15 PDT 2008
//    Added support to retain the original data extents.
//
// ****************************************************************************

vtkRectilinearGrid *
avtShapeletReconstruct::Execute(avtShapeletDecompResult *decomp,
                                const string &var_name,
                                avtShapeletBasisSet *basis_set)
{
    int nmax             = decomp->NMax();
    int width            = decomp->Width();  
    int height           = decomp->Height();
    doubleVector extents = decomp->Extents();
    
    int data_length = width * height;
    // create a rectlinear grid to hold the result
    vtkDoubleArray *x_coords = vtkDoubleArray::New();
    vtkDoubleArray *y_coords = vtkDoubleArray::New();
    vtkDoubleArray *z_coords = vtkDoubleArray::New();
    
    double sx = extents[0];
    double sy = extents[2];
    double dx = (extents[1] - extents[0])/((float)width);
    double dy = (extents[3] - extents[2])/((float)height);
    
    double curr = sx;
    for(int i=0; i<=width; i++) 
    {
        x_coords->InsertNextValue(curr);
        curr +=dx;
    }
    curr = sy;
    for(int i=0; i<=height; i++) 
    {
        y_coords->InsertNextValue(curr);
        curr +=dy;
    }
    
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

