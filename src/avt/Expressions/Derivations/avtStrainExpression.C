// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtStrainExpression.C                               //
// ************************************************************************* //

#include <avtStrainExpression.h>

#include <vtkDataArray.h>
#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtStrainExpression constructor
//
//  Programmer: Thomas R. Treadway
//  Creation:   Tue Nov 14 12:59:38 PST 2006
// 
//  Modifications:
//    Justin Privitera, Fri Sep 16 11:58:19 PDT 2022
//    Renamed avtStrainTensorExpression to avtStrainExpression.
//
// ****************************************************************************

avtStrainExpression::avtStrainExpression()
{
    ;
}

// ****************************************************************************
//  Method: avtStrainExpression destructor
//
//  Programmer: Thomas R. Treadway
//  Creation:   Tue Nov 14 12:59:38 PST 2006
// 
//  Modifications:
//    Justin Privitera, Fri Sep 16 11:58:19 PDT 2022
//    Renamed avtStrainTensorExpression to avtStrainExpression.
//
// ****************************************************************************

avtStrainExpression::~avtStrainExpression()
{
    ;
}

// ****************************************************************************
//  Method: avtStrainExpression::HexPartialDerivative
//
//  Purpose:
//      Computes the the partial derivative of the 8 brick shape
//      functions with respect to each coordinate axis.  The
//      coordinates of the brick are passed in "coorX,Y,Z", and
//      the partial derivatives are returned in "dNx,y,z".
//
//  Programmer: Thomas R. Treadway
//  Creation:   Tue Nov 14 12:59:38 PST 2006
// 
//  Modifications:
//    Justin Privitera, Fri Sep 16 11:58:19 PDT 2022
//    Renamed avtStrainTensorExpression to avtStrainExpression.
//
// ****************************************************************************

void
avtStrainExpression::HexPartialDerivative
    (double dNx[8], double dNy[8], double dNz[8],
    double coorX[8], double coorY[8], double coorZ[8])
{   // copied from Griz
//  Local shape function derivatives evaluated at node points.
    static double dN1[8] = { -.125, -.125, .125, .125,
                            -.125, -.125, .125, .125 };
    static double dN2[8] = { -.125, -.125, -.125, -.125,
                            .125, .125, .125, .125 };
    static double dN3[8] = { -.125, .125, .125, -.125,
                            -.125, .125, .125, -.125};
    double jacob[9], invJacob[9], detJacob;

    for (int k = 0; k < 9; k++)
       jacob[k] = 0.;
    for (int k = 0; k < 8; k++ )
    {   
        jacob[0] += dN1[k]*coorX[k];
        jacob[1] += dN1[k]*coorY[k];
        jacob[2] += dN1[k]*coorZ[k];
        jacob[3] += dN2[k]*coorX[k];
        jacob[4] += dN2[k]*coorY[k];
        jacob[5] += dN2[k]*coorZ[k];
        jacob[6] += dN3[k]*coorX[k];
        jacob[7] += dN3[k]*coorY[k];
        jacob[8] += dN3[k]*coorZ[k];
    }  
    detJacob =   jacob[0]*jacob[4]*jacob[8] + jacob[1]*jacob[5]*jacob[6]
               + jacob[2]*jacob[3]*jacob[7] - jacob[2]*jacob[4]*jacob[6] 
               - jacob[1]*jacob[3]*jacob[8] - jacob[0]*jacob[5]*jacob[7];
    
    if ( fabs( detJacob ) < 1.0e-20 )
    {   
        EXCEPTION2(ExpressionException, outputVariableName,
                "HexPartialDerivative, Element is degenerate! Result is invalid!");
    }
    
    /* Develop inverse of mapping. */                         
    detJacob = 1.0 / detJacob;                                
                                                              
    /* Cofactors of the jacobian matrix. */                   
    invJacob[0] = detJacob * (  jacob[4]*jacob[8] - jacob[5]*jacob[7] );    
    invJacob[1] = detJacob * ( -jacob[1]*jacob[8] + jacob[2]*jacob[7] );    
    invJacob[2] = detJacob * (  jacob[1]*jacob[5] - jacob[2]*jacob[4] );    
    invJacob[3] = detJacob * ( -jacob[3]*jacob[8] + jacob[5]*jacob[6] );    
    invJacob[4] = detJacob * (  jacob[0]*jacob[8] - jacob[2]*jacob[6] );    
    invJacob[5] = detJacob * ( -jacob[0]*jacob[5] + jacob[2]*jacob[3] );    
    invJacob[6] = detJacob * (  jacob[3]*jacob[7] - jacob[4]*jacob[6] );    
    invJacob[7] = detJacob * ( -jacob[0]*jacob[7] + jacob[1]*jacob[6] );    
    invJacob[8] = detJacob * (  jacob[0]*jacob[4] - jacob[1]*jacob[3] );    
                                                              
    /* Partials dN(k)/dx, dN(k)/dy, dN(k)/dz. */              
    for (int k = 0; k < 8; k++ )                                 
    {                                                         
        dNx[k] = invJacob[0]*dN1[k] + invJacob[1]*dN2[k] + invJacob[2]*dN3[k];
        dNy[k] = invJacob[3]*dN1[k] + invJacob[4]*dN2[k] + invJacob[5]*dN3[k];
        dNz[k] = invJacob[6]*dN1[k] + invJacob[7]*dN2[k] + invJacob[8]*dN3[k];
    }
}

// ****************************************************************************
//  Method: avtStrainExpression::CalculateEvolOrRelvol
//
//  Purpose:
//      Computes either the volumetric strain or the relative volume.
//
//  Programmer: Justin Privitera
//  Creation:   Fri Sep  9 10:37:12 PDT 2022
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtStrainExpression::CalculateEvolOrRelvol(vtkDataSet *in_ds, 
                                           bool vol_strain)
{
    char msg[1024];
    double vals[3];
    double out2;
    // same as Griz variables
    double x[8], y[8], z[8];         // Initial element geom.
    double xx[8], yy[8], zz[8];      // Current element geom.
    double dNx[8], dNy[8], dNz[8];   // Global derivates dN/dx,dN/dy,dN/dz.
    double F[9];                     // Element deformation gradient.
    double detF;                     // Determinant of element
                                     // deformation gradient.

    std::vector<int> cellsToIgnore;
    double avgVal = 0.0;             // ghost zone value
    int nElems = 0;                // number of elements in average
    
    // Let's get the points from the input dataset.
    if (in_ds->GetDataObjectType() != VTK_UNSTRUCTURED_GRID)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "The strain expression only operates on unstructured grids.");
    }
    vtkUnstructuredGrid *in_usg = vtkUnstructuredGrid::SafeDownCast(in_ds);
    int nCells = in_usg->GetNumberOfCells();
    vtkDataArray *coord_data = in_ds->GetPointData()->GetArray(varnames[1]);
    if (coord_data == NULL)
    {
        sprintf(msg, "The strain expression "
            "could not extract the data array for: %s", varnames[1]);
        EXCEPTION2(ExpressionException, outputVariableName, msg);
    }
    vtkDataArray *ghost_data = in_ds->GetPointData()->GetArray("avtGhostNodes");
    unsigned char *ghost = 
        (unsigned char *) (ghost_data ? ghost_data->GetVoidPointer(0) : 0);
    vtkIdList *pointIds = vtkIdList::New();
    vtkDataArray *out = coord_data->NewInstance();
    out->SetNumberOfComponents(1);
    out->SetNumberOfTuples(nCells);
    for (int i = 0; i < nCells; i++)
    {   // Check Voxel format
        int cellType = in_usg->GetCellType(i);
        // ignore everything but hexes
        if (cellType == VTK_HEXAHEDRON)
        {   // Grab a pointer to the cell's points' underlying data array
            in_usg->GetCellPoints(i, pointIds);
            bool anyGhost = false;
            if (ghost)
            {
                for (size_t j = 0; j < 8; j++)
                {
                    if (ghost[pointIds->GetId(j)] != 0)
                    {   // any ghost nodes in this hex
                        anyGhost = true;
                        break;
                    } 
                }
            }
            if (anyGhost)
            {
                cellsToIgnore.emplace_back(i);
                continue;
            }
            for (size_t j = 0; j < 8; j++)
            {   // Package initial element geometry points into vtkDataArray
                coord_data->GetTuple(pointIds->GetId(j), vals);
                x[j] = vals[0];
                y[j] = vals[1];
                z[j] = vals[2];
            }
            for (size_t j = 0; j < 8; j++)
            {   // Package current element geometry points into vtkDataArray
                in_usg->GetPoint(pointIds->GetId(j), vals);
                xx[j] = vals[0];
                yy[j] = vals[1];
                zz[j] = vals[2];
            }
            //
            avtStrainExpression::HexPartialDerivative
                (dNx, dNy, dNz, x, y, z);
            for (size_t j = 0; j < 9; j++) 
                F[j] = 0.0;
            // Copied from Griz
            for (size_t  k = 0; k < 8; k++ )
            {          
                F[0] = F[0] + dNx[k]*xx[k];
                F[1] = F[1] + dNy[k]*xx[k];
                F[2] = F[2] + dNz[k]*xx[k];
                F[3] = F[3] + dNx[k]*yy[k];
                F[4] = F[4] + dNy[k]*yy[k];
                F[5] = F[5] + dNz[k]*yy[k];
                F[6] = F[6] + dNx[k]*zz[k];
                F[7] = F[7] + dNy[k]*zz[k];
                F[8] = F[8] + dNz[k]*zz[k]; 
            }   
            detF = F[0]*F[4]*F[8] + F[1]*F[5]*F[6]
                + F[2]*F[3]*F[7] 
                - F[2]*F[4]*F[6] - F[1]*F[3]*F[8]
                - F[0]*F[5]*F[7];
            detF = fabs( detF );
            out2 = vol_strain ? log(detF) : detF;
            nElems++;
            avgVal += out2;
        } 
        else // cell is not a hexhedron also ignore   
            cellsToIgnore.emplace_back(i);
        out->SetComponent(i, 0, out2);
    }
    if (nElems > 0)
        avgVal /= nElems;
    for (size_t i = 0; i < cellsToIgnore.size(); i++)
        out->SetComponent(cellsToIgnore[i], 0, avgVal);
    return out;
}
