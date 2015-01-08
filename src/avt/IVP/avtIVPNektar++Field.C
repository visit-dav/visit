/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
//                            avtIVPNektar++Field.C                          //
// ************************************************************************* //

#include "visit-config.h"

#include "avtIVPNektar++Field.h"

#include <limits>

#include <DebugStream.h>
#include <avtCallback.h>

#include <vtkCellData.h>
#include <vtkLongArray.h>
#include <vtkUnstructuredGrid.h>

#include <InvalidVariableException.h>

// ****************************************************************************
//  Method: avtIVPNektarPPField constructor
//
//  Creationist: Allen Sanderson
//  Creation:    Nov 1, 2014
//
// ****************************************************************************

avtIVPNektarPPField::avtIVPNektarPPField( vtkDataSet* dataset, 
                                          avtCellLocator* locator) :
  avtIVPVTKField( dataset, locator )
{
  vtkFieldData *fieldData = dataset->GetFieldData();

  // Get the Nektar++ field data from the VTK field
  long *fp =
    (long *) (fieldData->GetAbstractArray("Nektar++FieldPointers")->GetVoidPointer(0));

  if( fp )
  {
    for (int i = 0; i < 3; ++i)
    {
      if( fp[i] )
        nektar_field[i] = (Nektar::MultiRegions::ExpListSharedPtr)
          (*((Nektar::MultiRegions::ExpListSharedPtr*)fp[i]));
    }
  }
  else 
  {
    EXCEPTION1( InvalidVariableException,
                "Uninitialized option: Nektar++FieldPointers. (Please contact visit-developer mailing list to report)" );
  }

  // nektar_element_lookup =
  //   ((int *) fieldData->GetAbstractArray("Nektar++ElementLookup")->GetVoidPointer(0));

  // if( nektar_element_lookup == NULL )
  // {
  //   EXCEPTION1( InvalidVariableException,
  //               "Uninitialized option: Nektar++ElementLookup. (Please contact visit-developer mailing list to report)" );
  // }
}


// ****************************************************************************
//  Method: avtIVPNektarPPField destructor
//
//  Creationist: Allen Sanderson
//  Creation:    Nov 1, 2014
//
// ****************************************************************************

avtIVPNektarPPField::~avtIVPNektarPPField()
{
}


// ****************************************************************************
//  Method: avtIVPNektarPPField::operator
//
//  Evaluates a point location by consulting a Nektar++ field.
//
//  Programmer: Allen Sanderson
//  Creation:   Nov 1, 2014
//
//  Modifications:
//
// ****************************************************************************

avtIVPField::Result
avtIVPNektarPPField::operator()( const double &t,
                                 const avtVector &p,
                                 avtVector &vec ) const
{
    static int el = 0; // element

    // Locate the cell that surrounds the point.
    avtInterpolationWeights iw[8];

    double xpt[3];

    xpt[0] = p[0];
    xpt[1] = p[1];
    xpt[2] = p[2];

    el = loc->FindCell( xpt, iw, false );

    if( el < 0 )
      return OUTSIDE_SPATIAL;

    // Get the Nektar++ element id at this point. Assume the cell
    // boundaries are liner and not curves thus the nektar element is
    // the vtk element.
    int nt_el = el, nt_numElements = nektar_field[0]->GetExpSize();

    // for( int i=0, j=1; j<nt_numElements; ++i, ++j)
    // {
    //   if( nektar_element_lookup[i] <= el && el < nektar_element_lookup[i] )
    //   {
    //    nt_el = i;
    //    break;
    //   }
    // }

    // Set up the point in the Nektar++ format.
    Nektar::Array<OneD, NekDouble> coords(3);
    coords[0] = p[0];
    coords[1] = p[1];
    coords[2] = p[2];
    
    // Loop through each velocity component and do the appropriate
    // interpolation at the given point.
    for (int i = 0; i < 3; ++i)
    {
      if( !nektar_field[i] )
        break;
      
      Nektar::Array<Nektar::OneD, Nektar::NekDouble> physVals =
        nektar_field[i]->GetPhys() + nektar_field[i]->GetPhys_Offset(nt_el);
      
      vec[i] = nektar_field[i]->GetExp(nt_el)->PhysEvaluate(coords, physVals);
    }

    return OK;
}

// ****************************************************************************
//  Method: avtIVPNektarPPField::ConvertToCartesian
//
//  Purpose: Converts the coordinates from local cylindrical to
//      cartesian coordinates
//
//  Programmer: Allen Sanderson
//  Creation:   Nov 1, 2014
//
// ****************************************************************************

avtVector 
avtIVPNektarPPField::ConvertToCartesian(const avtVector& pt) const
{
  return pt;
}

// ****************************************************************************
//  Method: avtIVPNektarPPField::ConvertToCylindrical
//
//  Purpose: Converts the coordinates from local cylindrical to
//      cartesian coordinates
//
//  Programmer: Allen Sanderson
//  Creation:   Nov 1, 2014
//
// ****************************************************************************

avtVector 
avtIVPNektarPPField::ConvertToCylindrical(const avtVector& pt) const
{
  return avtVector(sqrt(pt[0]*pt[0]+pt[1]*pt[1]), atan2(pt[1],pt[0]), pt[2] );
}
