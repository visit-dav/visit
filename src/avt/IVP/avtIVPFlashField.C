/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
//                             avtIVPFlashField.C                            //
// ************************************************************************* //

#include "avtIVPFlashField.h"

#include <DebugStream.h>

#include <vtkCellData.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>

// ****************************************************************************
//  Method: avtIVPFlashField constructor
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************

avtIVPFlashField::avtIVPFlashField( vtkDataSet* dataset, 
                                    avtCellLocator* locator,
                                    double fact) : 
  avtIVPVTKField( dataset, locator ),
  B_vtkDataArray(0), E_vtkDataArray(0), factor(1.0/fact)
{
  order = 2;

  if( velCellBased )
  {
    B_vtkDataArray = dataset->GetCellData()->GetArray("B");
    E_vtkDataArray = dataset->GetCellData()->GetArray("E");
  }
  else
  {
    B_vtkDataArray = dataset->GetPointData()->GetArray("B");
    E_vtkDataArray = dataset->GetPointData()->GetArray("E");
  }

  if( B_vtkDataArray == 0 )
  {
    EXCEPTION1( ImproperUseException,
                "avtIVPFlashField: Can't locate 'B' vectors to interpolate." );
  }

  if( E_vtkDataArray == 0 )
  {
    EXCEPTION1( ImproperUseException,
                "avtIVPFlashField: Can't locate 'E' vectors to interpolate." );
  }
}


// ****************************************************************************
//  Method: avtIVPFlashField destructor
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************

avtIVPFlashField::~avtIVPFlashField()
{
}


// ****************************************************************************
//  Method: avtIVPFlashField::operator
//
//  Evaluates the velocity via the Lorentz force.
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
//  Modifications:
//
// ****************************************************************************

avtVector
avtIVPFlashField::operator()( const double &t,
                              const avtVector &p,
                              const avtVector &v ) const
{
  if( !FindCell( t, p ) )
    throw Undefined();

  avtVector B = FindValue(B_vtkDataArray);
  avtVector E = FindValue(E_vtkDataArray);

  avtVector vec = factor * (E + Cross(v, B) );

  return vec;
}


// ****************************************************************************
//  Method: avtIVPFlashField::ConvertToCartesian
//
//  Purpose: Converts the coordinates from local cylindrical to
//      cartesian coordinates
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************

avtVector 
avtIVPFlashField::ConvertToCartesian(const avtVector& pt) const
{
  return avtVector(pt[0]*cos(pt[1]), pt[0]*sin(pt[1]), pt[2] );
}

// ****************************************************************************
//  Method: avtIVPFlashField::ConvertToCylindrical
//
//  Purpose: Converts the coordinates from local cylindrical to
//      cartesian coordinates
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************

avtVector 
avtIVPFlashField::ConvertToCylindrical(const avtVector& pt) const
{
  return pt;
}
