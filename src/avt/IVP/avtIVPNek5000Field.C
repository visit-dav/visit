/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//                             avtIVPNek5000Field.C                          //
// ************************************************************************* //

#include "avtIVPNek5000Field.h"

#include <DebugStream.h>

#include <vtkCellData.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>

#include <InvalidVariableException.h>

// ****************************************************************************
//  Method: avtIVPNek5000Field constructor
//
//  Creationist: Allen Sanderson
//  Creation:    May 1, 2013
//
// ****************************************************************************

avtIVPNek5000Field::avtIVPNek5000Field( vtkDataSet* dataset, 
                                        avtCellLocator* locator) : 
  avtIVPVTKField( dataset, locator )
{
  vtkFieldData *fieldData = dataset->GetFieldData();

  // Pick off all of the data stored with the vtk field.

  // Get the number of point per spectrial elements
  vtkIntArray *semVTK =
    (vtkIntArray *) fieldData->GetAbstractArray("Nek_SpectralElementData");  

  if( semVTK )
  {
    sem[0] = semVTK->GetValue(0);
    sem[1] = semVTK->GetValue(1);
    sem[2] = semVTK->GetValue(2);

    std::cerr << sem[0] << " " << sem[1] << " " << sem[2] << " " << std::endl;
  }

  // Get the numver of elements for checking the validity of the data.

  // Because the triangluar mesh is defined by using non unique points
  // and the data is cell centered data VisIt moves it out to the
  // nodes for STREAMLINES thus there are 3 times the number of
  // original values.
  // nelms =
  //   ds->GetCellData()->GetArray("hidden/elements")->GetNumberOfTuples();
  
  // element_size =
  //   ds->GetCellData()->GetArray("hidden/elements")->GetNumberOfComponents();
  
  // elements =
  //   SetDataPointer( ds, fltVar, "hidden/elements", element_size );
  
  // // Equalibrium field
  // eqsubtract =
  //   ((int *) fieldData->GetAbstractArray("eqsubtract")->GetVoidPointer(0))[0];

  // bzero =
  //   ((double *) fieldData->GetAbstractArray("bzero")->GetVoidPointer(0))[0];
  
  
  // f   = SetDataPointer( ds, fltVar, "hidden/f",   scalar_size, factor );
  // psi = SetDataPointer( ds, fltVar, "hidden/psi", scalar_size, factor );
  // I   = SetDataPointer( ds, fltVar, "hidden/I"  , scalar_size, factor );
}


// ****************************************************************************
//  Method: avtIVPNek5000Field destructor
//
//  Creationist: Allen Sanderson
//  Creation:    May 1, 2013
//
// ****************************************************************************

avtIVPNek5000Field::~avtIVPNek5000Field()
{
  // if( f )   delete [] f;
  // if( psi ) delete [] psi;
  // if( I )   delete [] I;
}


// ****************************************************************************
//  Method: avtIVPNek5000Field SetDataPointer
//
//  Creationist: Allen Sanderson
//  Creation:    May 1, 2013
//
// ****************************************************************************

template< class type >
type* avtIVPNek5000Field::SetDataPointer( vtkDataSet *ds,
                                        const type var,
                                        const char* varname,
                                        const int component_size,
                                        double factor )
{
  vtkDataArray *array;
  int XX;

  // Because the triangluar mesh is defined by using non unique points
  // and the data is cell centered data VisIt moves it out to the
  // nodes for STREAMLINES thus there are 3 times the number of
  // original values.
  if( ds->GetPointData()->GetArray(varname) )
  {
    array = ds->GetPointData()->GetArray(varname);
    XX = 3;
  }
  // 2.0 Change data is now at the cells for POINCARE
  else
  {
    array = ds->GetCellData()->GetArray(varname);
    XX = 1;
  }

  if( array == 0 )
  {
    if (DebugStream::Level1())
        debug1 << "Variable " << varname
               << " does not exist"
               << endl;
    return 0;
  }

  const int ntuples = array->GetNumberOfTuples();
  const int ncomponents = array->GetNumberOfComponents();

  // if( ntuples != nelms || ncomponents != component_size )
  // {
  //   if (DebugStream::Level1())
  //       debug1 << "Variable " << varname
  //              << " size does not equal the number elements and/or components"
  //              << endl;
  //   return 0;
  // }

  // 2.0 Change data is no longer at the points but still is at the
  // cells so we should be able to use the pointer directly but for
  // some reason it causes problems.
  // 
  //  return (type*) array->GetVoidPointer(0);

  type* newptr = new type[ntuples*ncomponents];

  if( newptr == 0 )
  {
    if (DebugStream::Level1())
        debug1 << "Variable " << varname << " can not allocated" << endl;
    return 0;
  }

  // Because the triangluar mesh is defined by using non unique points
  // and the data is cell centered data VisIt moves it out to the
  // nodes. So create a new structure that is what is really needed.

  // 2.0 Change data is no longer at the points but still is at the
  // cells so the above is no longer valid.
  if( array->IsA("vtkIntArray") ) 
  {
    int* ptr = (int*) array->GetVoidPointer(0);

    for( int i=0; i<ntuples; ++i )
      for( int j=0; j<ncomponents; ++j )
        newptr[i*ncomponents+j] = (type) (factor * ptr[i*XX*ncomponents+j]);

    return newptr;
  }
  else if( array->IsA("vtkFloatArray") ) 
  {
    float* ptr = (float*) array->GetVoidPointer(0);

    for( int i=0; i<ntuples; ++i )
      for( int j=0; j<ncomponents; ++j )
        newptr[i*ncomponents+j] = (type) (factor * ptr[i*XX*ncomponents+j]);

    return newptr;
  }
  else if( array->IsA("vtkDoubleArray") ) 
  {
    double* ptr = (double*) array->GetVoidPointer(0);

    for( int i=0; i<ntuples; ++i )
      for( int j=0; j<ncomponents; ++j )
        newptr[i*ncomponents+j] = (type) (factor * ptr[i*XX*ncomponents+j]);

    return newptr;
  }
  else
  {
    if (DebugStream::Level1())
        debug1 << "avtIVPNek5000Field::SetDataPointer "
               << "Variable " << varname
               << " is not of type float - can not safely down cast"
               << endl;
    if( newptr )
        delete [] newptr;
    return 0;
  }
}


// ****************************************************************************
//  Method: avtIVPNek5000Field::operator
//
//  Evaluates a point location by consulting a Nek 5000 grid.
//
//  Programmer: Allen Sanderson
//  Creation:   May 1, 2013
//
//  Modifications:
//
// ****************************************************************************

avtIVPField::Result
avtIVPNek5000Field::operator()( const double &t,
                                const avtVector &p,
                                avtVector &vec ) const
{
    return avtIVPVTKField::operator()( t, p, vec );


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

    // The above element is based on the linear mesh not the spectral
    // mess so find the first linear element of the spectral mesh.

    // Note this is integer arthimetic. 
    el /= sem[0] * sem[1] * sem[2];


    double *sem_pts = new double[sem[0] * sem[1] * sem[2] * 3];

    // Get the first point from each element.
    for( unsigned int i=0; i<sem[0] * sem[1] * sem[2]; ++i )
    {
    }

    //    interpolate( );

  return OK;
}

// ****************************************************************************
//  Method: avtIVPNek5000Field::ConvertToCartesian
//
//  Purpose: Converts the coordinates from local cylindrical to
//      cartesian coordinates
//
//  Programmer: Allen Sanderson
//  Creation:   May 1, 2013
//
// ****************************************************************************

avtVector 
avtIVPNek5000Field::ConvertToCartesian(const avtVector& pt) const
{
  return pt;
}

// ****************************************************************************
//  Method: avtIVPNek5000Field::ConvertToCylindrical
//
//  Purpose: Converts the coordinates from local cylindrical to
//      cartesian coordinates
//
//  Programmer: Allen Sanderson
//  Creation:   May 1, 2013
//
// ****************************************************************************

avtVector 
avtIVPNek5000Field::ConvertToCylindrical(const avtVector& pt) const
{
  return avtVector(sqrt(pt[0]*pt[0]+pt[1]*pt[1]), atan2(pt[1],pt[0]), pt[2] );
}
