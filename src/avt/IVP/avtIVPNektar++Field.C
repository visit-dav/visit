// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtIVPNektar++Field.C                          //
// ************************************************************************* //

#include "visit-config.h"

#include "avtIVPNektar++Field.h"

//#include <vtkNektar++.h>

#include <limits>

#include <DebugStream.h>
#include <avtCallback.h>

#include <vtkCellData.h>
#include <vtkLongArray.h>
#include <vtkUnstructuredGrid.h>

//#include <vtkInformation.h>
//#include <vtkInformationUnsignedLongKey.h>

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

  // vtkDoubleArray *vecs =
  //   (vtkDoubleArray*) dataset->GetPointData()->GetVectors();

  // vtkNektarDoubleArray *vecs =
  //   (vtkNektarDoubleArray*) dataset->GetPointData()->GetVectors();

  // if (vecs == NULL) {
  //   EXCEPTION1( InvalidVariableException,
  //               "avtIVPNektar++Field - Can not find the velocity variable." );
  // }

  // const std::string vectorVarComponents[3] = { "u", "v", "w" };

  // Get the Nektar++ field data from the VTK data
  // unsigned long fptr = vecs->GetNektarUField();

  // if( fptr )
  //   nektar_field[0] = (Nektar::MultiRegions::ExpListSharedPtr)
  //     (*((Nektar::MultiRegions::ExpListSharedPtr*) fptr));

  // fptr = vecs->GetNektarVField();

  // if( fptr )
  //   nektar_field[1] = (Nektar::MultiRegions::ExpListSharedPtr)
  //     (*((Nektar::MultiRegions::ExpListSharedPtr*) fptr));

  // fptr = vecs->GetNektarWField();

  // if( fptr )
  //   nektar_field[2] = (Nektar::MultiRegions::ExpListSharedPtr)
  //     (*((Nektar::MultiRegions::ExpListSharedPtr*) fptr));


  // unsigned long fptr = NEKTAR_RT_U_FIELD->Get( vecs->GetInformation() );

  // if( fptr )
  //   nektar_field[0] = (Nektar::MultiRegions::ExpListSharedPtr)
  //     (*((Nektar::MultiRegions::ExpListSharedPtr*) fptr));

  // fptr = NEKTAR_RT_V_FIELD->Get( vecs->GetInformation() );

  // if( fptr )
  //   nektar_field[1] = (Nektar::MultiRegions::ExpListSharedPtr)
  //     (*((Nektar::MultiRegions::ExpListSharedPtr*) fptr));

  // fptr = NEKTAR_RT_W_FIELD->Get( vecs->GetInformation() );

  // if( fptr )
  //   nektar_field[2] = (Nektar::MultiRegions::ExpListSharedPtr)
  //     (*((Nektar::MultiRegions::ExpListSharedPtr*) fptr));

  // Get the Nektar++ field data from the VTK field
  long *fp =
    (long *) (fieldData->GetAbstractArray("Nektar++FieldPointers")->GetVoidPointer(0));

  if( fp )
  {
    for (int i = 0; i < 3; ++i)
    {
      if( fp[i] )
        nektar_field[i] = (Nektar::MultiRegions::ExpListSharedPtr)
          (*((Nektar::MultiRegions::ExpListSharedPtr*) fp[i]));
    }
  }
  else
  {
    EXCEPTION1( InvalidVariableException,
                "Uninitialized option: Nektar++FieldPointers. (Please report at https://github.com/visit-dav/visit/discussions)" );
  }

  nektar_element_lookup =
    ((int *) fieldData->GetAbstractArray("Nektar++ElementLookup")->GetVoidPointer(0));

  if( nektar_element_lookup == NULL )
  {
    EXCEPTION1( InvalidVariableException,
                "Uninitialized option: Nektar++ElementLookup. (Please report at https://github.com/visit-dav/visit/discussions)" );
  }
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
    // boundaries are liner and not curved thus the nektar element is
    // the vtk element.
    int nt_el = nektar_element_lookup[el];

    // Set up the point in the Nektar++ format.
    Nektar::Array<Nektar::OneD, Nektar::NekDouble> coords(3);
    coords[0] = p[0];
    coords[1] = p[1];
    coords[2] = p[2];
    
    // Loop through each velocity component and do the appropriate
    // interpolation at the given point.
    for (int i = 0; i < 3; ++i)
    {
      if( nektar_field[i] )
      {
        Nektar::Array<Nektar::OneD, Nektar::NekDouble> physVals =
          nektar_field[i]->GetPhys() + nektar_field[i]->GetPhys_Offset(nt_el);
          
        vec[i] = nektar_field[i]->GetExp(nt_el)->PhysEvaluate(coords, physVals);
      }
      else
      {
        vec[i] = 0;
      }
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
