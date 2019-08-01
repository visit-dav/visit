/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                      avtZoneTypeLabelExpression.C                         //
// ************************************************************************* //

#include <avtZoneTypeLabelExpression.h>

#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkIntArray.h>

// ****************************************************************************
//  Method: avtZoneTypeLabelExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 15, 2006 
//
// ****************************************************************************

avtZoneTypeLabelExpression::avtZoneTypeLabelExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtZoneTypeLabelExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Mark C. Miller
//  Creation:   November 15, 2006 
//
// ****************************************************************************

avtZoneTypeLabelExpression::~avtZoneTypeLabelExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtZoneTypeLabelExpression::DeriveVariable
//
//  Purpose:
//      Determines the zon type of a cell.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Mark C. Miller 
//  Creation:     November 15, 2006 
//
//  Modifications:
//    Mark C. Miller, Wed Apr  2 09:46:47 PDT 2008
//    Added case statement to set character for zone type.
//
//    Mark C. Miller, Thu Oct 23 15:59:17 PDT 2014
//    Made the output 3 chars long instead of one. Added cases for more
//    element types.
//
//    Eric Brugger, Mon Jul 29 17:37:36 PDT 2019
//    Made val 8 characters long to avoid a compiler error on Power 9 systems
//    with gcc.
//
// ****************************************************************************

#define SET_VAL(STR) strncpy(val, #STR, sizeof(val))

vtkDataArray *
avtZoneTypeLabelExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkIdType ncells = in_ds->GetNumberOfCells();

    vtkIntArray *rv = vtkIntArray::New();
    rv->SetNumberOfComponents(4);
    rv->SetNumberOfTuples(ncells);
    for (vtkIdType i = 0 ; i < ncells ; i++)
    {
        //
        // This array is 8 characters long instead of 3 to avoid a compile
        // error on Power 9 systems with the gcc 5.4 and 6.4 compilers. It
        // avoids an alignment error when calling SetTuple4 later in the
        // loop body.
        //
        char val[8];
        SET_VAL(" ? ");
        switch (in_ds->GetCellType(i))
        {
            // 2D cell types, lower case letters
            case VTK_EMPTY_CELL:     SET_VAL(emt); break;
            case VTK_VERTEX:         SET_VAL(vtx); break;
            case VTK_POLY_VERTEX:    SET_VAL(pvx); break;
            case VTK_LINE:           SET_VAL(lin); break;
            case VTK_POLY_LINE:      SET_VAL(pln); break;
            case VTK_TRIANGLE:       SET_VAL(tri); break;
            case VTK_TRIANGLE_STRIP: SET_VAL(tst); break;
            case VTK_POLYGON:        SET_VAL(ply); break;
            case VTK_PIXEL:          SET_VAL(pix); break;
            case VTK_QUAD:           SET_VAL(qad); break;

            // 3D cell types, upper case letters
            case VTK_TETRA:          SET_VAL(TET); break;
            case VTK_VOXEL:          SET_VAL(VOX); break;
            case VTK_HEXAHEDRON:     SET_VAL(HEX); break;
            case VTK_WEDGE:          SET_VAL(WDG); break;
            case VTK_PYRAMID:        SET_VAL(PYR); break;

            // Quadratic, isoparametric cells
            case VTK_QUADRATIC_EDGE:                   SET_VAL(Qln); break;
            case VTK_QUADRATIC_TRIANGLE:               SET_VAL(Qtr); break;
            case VTK_QUADRATIC_QUAD:                   SET_VAL(Qqd); break;
            case VTK_QUADRATIC_TETRA:                  SET_VAL(QTT); break;
            case VTK_QUADRATIC_HEXAHEDRON:             SET_VAL(QHX); break;
            case VTK_QUADRATIC_WEDGE:                  SET_VAL(QWD); break;
            case VTK_QUADRATIC_PYRAMID:                SET_VAL(QPY); break;
            case VTK_BIQUADRATIC_QUAD:                 SET_VAL(2Qq); break;
            case VTK_TRIQUADRATIC_HEXAHEDRON:          SET_VAL(3QH); break;
            case VTK_QUADRATIC_LINEAR_QUAD:            SET_VAL(QLq); break;
            case VTK_QUADRATIC_LINEAR_WEDGE:           SET_VAL(QLW); break;
            case VTK_BIQUADRATIC_QUADRATIC_WEDGE:      SET_VAL(2QW); break;
            case VTK_BIQUADRATIC_QUADRATIC_HEXAHEDRON: SET_VAL(3QH); break;
            case VTK_BIQUADRATIC_TRIANGLE:             SET_VAL(2Qt); break;

            case VTK_CUBIC_LINE:       SET_VAL(3ln); break;
            case VTK_CONVEX_POINT_SET: SET_VAL(CPS); break;
            case VTK_POLYHEDRON:       SET_VAL(PLY); break;

            // Higher order cells in parametric form
            case VTK_PARAMETRIC_CURVE:        SET_VAL(Pcu); break;
            case VTK_PARAMETRIC_SURFACE:      SET_VAL(Psu); break;
            case VTK_PARAMETRIC_TRI_SURFACE:  SET_VAL(Pts); break;
            case VTK_PARAMETRIC_QUAD_SURFACE: SET_VAL(Pqs); break;
            case VTK_PARAMETRIC_TETRA_REGION: SET_VAL(PTR); break;
            case VTK_PARAMETRIC_HEX_REGION:   SET_VAL(PHR); break;

            // Higher order cells
            case VTK_HIGHER_ORDER_EDGE:        SET_VAL(HOl); break;
            case VTK_HIGHER_ORDER_TRIANGLE:    SET_VAL(HOt); break;
            case VTK_HIGHER_ORDER_QUAD:        SET_VAL(HOq); break;
            case VTK_HIGHER_ORDER_POLYGON:     SET_VAL(HOp); break;
            case VTK_HIGHER_ORDER_TETRAHEDRON: SET_VAL(HOT); break;
            case VTK_HIGHER_ORDER_WEDGE:       SET_VAL(HOW); break;
            case VTK_HIGHER_ORDER_PYRAMID:     SET_VAL(HOP); break;
            case VTK_HIGHER_ORDER_HEXAHEDRON:  SET_VAL(HOH); break;
        }            

        rv->SetTuple4(i, (int) val[0], (int) val[1], (int) val[2], (int) '\0');
    }

    return rv;
}

// ****************************************************************************
//  Method: avtZoneTypeLabelExpression::UpdateDataObjectInfo
//
//  Purpose: Tell the output this is a treat as ascii variable
//
//  Programmer:   Mark C. Miller 
//  Creation:     April 2, 2008 
//
// ****************************************************************************

void
avtZoneTypeLabelExpression::UpdateDataObjectInfo(void)
{
    avtSingleInputExpressionFilter::UpdateDataObjectInfo();

    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetTreatAsASCII(true);
}
