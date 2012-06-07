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
//                             avtIVPVTKOffsetField.C                              //
// ************************************************************************* //

#include <avtIVPVTKField.h>
#include <avtIVPVTKOffsetField.h>
#include <ImproperUseException.h>
#include <DebugStream.h>

#include <iostream>
#include <limits>
#include <cmath>
#include <sstream>

#include <vtkUnsignedCharArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkGenericCell.h>
#include <vtkCell.h>

// ****************************************************************************
//  Method: avtIVPVTKOffsetField constructor
//
//  Programmer: Alexander Pletzer
//  Creation:   Thu Mar  1 12:01:37 MST 2012
//
//  Modifications:
//
// ****************************************************************************

avtIVPVTKOffsetField::avtIVPVTKOffsetField( vtkDataSet* dataset, avtCellLocator* locator ) 
: avtIVPVTKField(dataset, locator)
{
    //  default is no node offset
    this->nodeOffsets.resize(3);
    for ( size_t i = 0; i < this->nodeOffsets.size(); ++i ) 
    {
        for (size_t j = 0; j < 3; ++j) 
        {
            nodeOffsets[i][j] = 0;
        }
    }
}

// ****************************************************************************
//  Method: avtIVPVTKOffsetField destructor
//
//  Programmer: Alexander Pletzer
//  Creation: Thu Mar  1 12:01:37 MST 2012
//
//  Modifications:
//
// ****************************************************************************

avtIVPVTKOffsetField::~avtIVPVTKOffsetField()
{
}

// ****************************************************************************
//  Method: avtIVPVTKOffsetField set node offsets for each component
//
//  Programmer: Alexander Pletzer
//  Creation: Thu Mar  1 12:01:37 MST 2012
//
//  Modifications:
//
// ****************************************************************************

void
avtIVPVTKOffsetField::SetNodeOffsets( const std::vector<avtVector>& offsets )
{
  debug5 <<"avtIVPVTKOffsetField::SetNodeOffsets entering" <<std::endl;
  for ( size_t i = 0; i < (offsets.size() < 3? offsets.size(): 3); ++i ) 
    {
        for (size_t j = 0; j < 3; ++j) 
        {
            nodeOffsets[i][j] = offsets[i][j];
        }
    }  
}

// ****************************************************************************
//  Method: avtIVPVTKField::operator
//
//  Purpose:
//      Evaluates a point location by consulting a VTK grid.
//
//  Programmer: Alex Pletzer
//  Creation:   Wed Mar  7 09:27:31 MST 2012
//
//  Modifications:
//
// ****************************************************************************

avtIVPSolverResult::Result
avtIVPVTKOffsetField::operator()( const double &t, const avtVector &pt, avtVector &retV ) const {
  avtVector result = operator()(t, pt);
  //TODO error handling
  retV.x = result.x;
  retV.y = result.y;
  retV.z = result.z;

  return avtIVPSolverResult::OK;
}

avtVector
avtIVPVTKOffsetField::operator()( const double &t, const avtVector &p ) const
{
    avtVector zeros(0, 0, 0);
    std::vector<avtVector> velocities(3);
    for ( size_t j = 0; j < 3; ++j )
    {

        if (FindCell(t, p) != INSIDE)
        {
            // ghost cells on the base mesh may be required to avoid this failure
            debug5 <<"avtIVPVTKOffsetField::operator() - UNABLE TO FIND CELL!" 
                   <<std::endl;
            return zeros;
        }

        avtVector displ = GetPositionCorrection( j );
        avtVector pCorrected = p - displ;


        avtVector displ2 = displ;
        if (FindCell(t, pCorrected) != INSIDE)
        {
            // the displacement seen from the base target position may be 
            // a little different.
            displ2 = GetPositionCorrection( j );
        }

        pCorrected = p - 0.5*(displ2 + displ);
        if (FindCell(t, pCorrected) != INSIDE)
        {
            debug5 <<"avtIVPVTKOffsetField::operator() - UNABLE TO FIND CORRECTED CELL!" 
                   <<std::endl;
            return zeros;
        }        

        // velocity for this staggering
        FindValue(velData, velocities[j]);
    }

    // compose the velocity, assuming each component is purely 
    // aligned to each axis. How should this be generalized when
    // the velocites are not alog axes? 
    avtVector vel( velocities[0].x, velocities[1].y, velocities[2].z );
    return vel;
}

// ****************************************************************************
//  Method: avtIVPVTKOffsetField::FindValue
//
//  Purpose:
//      Evaluates a point after consulting a VTK grid.
//
//  Programmer: Alexander Pletzer
//  Creation: Thu Mar  1 12:01:37 MST 2012
//
// ****************************************************************************

bool
avtIVPVTKOffsetField::FindValue(vtkDataArray *vectorData, avtVector &vel) const
{
    if (velCellBased)
        vectorData->GetTuple(lastCell, &vel.x);
    else
    {
        // nodal field components, may contain some offset
        
        for ( size_t j = 0; j < 3; ++j )
        {
            // interpolate
            double tmp[3];
            
            for( avtInterpolationWeights::const_iterator wi=lastWeights.begin();
                 wi!=lastWeights.end(); ++wi )
            {
                vectorData->GetTuple( wi->i, tmp );
                
                vel[j] += wi->w * tmp[j];
            }
        }
    }

    if( normalized )
    {
        double len = vel.length();
        
        if( len )
            vel /= len;
    }
    
    return true;
}

// ****************************************************************************
//  Method: avtIVPVTKOffsetField get correction to the position, for given 
//                               vector field component. 
//
//  Programmer: Alexander Pletzer
//  Creation: Thu Mar  1 12:01:37 MST 2012
//
//  Modifications:
//
// ****************************************************************************

avtVector
avtIVPVTKOffsetField::GetPositionCorrection( size_t compIndex ) const
{
    avtVector displ( 0.0, 0.0, 0.0 );
    vtkCell *cell = this->ds->GetCell( this->lastCell );
  
    // compute the parametric coordinates
    double pcoords[3], weights[12];
    double tol2 = 1.e-4;
    int subId;
    vtkIdType cellId = this->ds->FindCell(&this->lastPos.x, 
                                          cell, this->lastCell, 
                                          tol2, subId, 
                                          pcoords, 
                                          weights);
    if ( cellId < 0 ) 
    {
        std::stringstream msg;
        debug5 << "avtIVPVTKOffsetField: could not find cell\n";
        debug5 << " lastCell = " << this->lastCell << '\n';
        debug5 << " lastPos = " << this->lastPos.x << ", " << this->lastPos.y << ", " << this->lastPos.z << '\n';
        debug5 << " will set displacement to zero\n";
        return displ;
    }


    vtkIdType idPtLo, idPtHi;
    avtVector ptLo( 0.0, 0.0, 0.0 );
    avtVector ptHi( 0.0, 0.0, 0.0 );
    if ( cell->GetCellType( ) == VTK_QUAD ) 
    {
        std::vector< std::vector<double> > g(4); // 4 edges
        for (size_t i = 0; i < 4; ++i) {
          g[i].resize(3);
        }

        idPtLo = cell->GetPointId(0);
        idPtHi = cell->GetPointId(1);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[0][i] = ptHi[i] - ptLo[i];
        }

        idPtLo = cell->GetPointId(3);
        idPtHi = cell->GetPointId(2);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[1][i] = ptHi[i] - ptLo[i];
        }

        idPtLo = cell->GetPointId(0);
        idPtHi = cell->GetPointId(3);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[2][i] = ptHi[i] - ptLo[i];
        }

        idPtLo = cell->GetPointId(1);
        idPtHi = cell->GetPointId(2);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[3][i] = ptHi[i] - ptLo[i];
        }

        for (size_t i = 0; i < 3; ++i) 
        {
            displ[i] += this->nodeOffsets[compIndex][0] * (1.-pcoords[1])*g[0][i];
            displ[i] += this->nodeOffsets[compIndex][0] * (   pcoords[1])*g[1][i];

            displ[i] += this->nodeOffsets[compIndex][1] * (1.-pcoords[0])*g[2][i];
            displ[i] += this->nodeOffsets[compIndex][1] * (   pcoords[0])*g[3][i];
        }
    }
    else if ( cell->GetCellType( ) == VTK_HEXAHEDRON )
    {

        std::vector< std::vector<double> > g(12); // 12 edges
        for (size_t i = 0; i < 12; ++i) {
          g[i].resize(3);
        }

        idPtLo = cell->GetPointId(0);
        idPtHi = cell->GetPointId(1);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[0][i] = ptHi[i] - ptLo[i];
        }

        idPtLo = cell->GetPointId(3);
        idPtHi = cell->GetPointId(2);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[1][i] = ptHi[i] - ptLo[i];
        }

        idPtLo = cell->GetPointId(7);
        idPtHi = cell->GetPointId(6);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[2][i] = ptHi[i] - ptLo[i];
        }

        idPtLo = cell->GetPointId(4);
        idPtHi = cell->GetPointId(5);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[3][i] = ptHi[i] - ptLo[i];
        }

        idPtLo = cell->GetPointId(0);
        idPtHi = cell->GetPointId(3);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[4][i] = ptHi[i] - ptLo[i];
        }

        idPtLo = cell->GetPointId(1);
        idPtHi = cell->GetPointId(2);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[5][i] = ptHi[i] - ptLo[i];
        }

        idPtLo = cell->GetPointId(5);
        idPtHi = cell->GetPointId(6);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[6][i] = ptHi[i] - ptLo[i];
        }

        idPtLo = cell->GetPointId(4);
        idPtHi = cell->GetPointId(7);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[7][i] = ptHi[i] - ptLo[i];
        }

        idPtLo = cell->GetPointId(0);
        idPtHi = cell->GetPointId(4);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[8][i] = ptHi[i] - ptLo[i];
        }

        idPtLo = cell->GetPointId(1);
        idPtHi = cell->GetPointId(5);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[9][i] = ptHi[i] - ptLo[i];
        }

        idPtLo = cell->GetPointId(2);
        idPtHi = cell->GetPointId(6);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[10][i] = ptHi[i] - ptLo[i];
        }

        idPtLo = cell->GetPointId(3);
        idPtHi = cell->GetPointId(7);
        this->ds->GetPoint(idPtLo, &ptLo[0]);
        this->ds->GetPoint(idPtHi, &ptHi[0]);
        for (size_t i = 0; i < 3; ++i) {
          g[11][i] = ptHi[i] - ptLo[i];
        }

        for ( size_t i = 0; i < 3; ++i ) 
        {
            displ[i] += this->nodeOffsets[compIndex][0] * (1.-pcoords[1])*(1.-pcoords[2])*g[0][i];
            displ[i] += this->nodeOffsets[compIndex][0] * (   pcoords[1])*(1.-pcoords[2])*g[1][i];
            displ[i] += this->nodeOffsets[compIndex][0] * (   pcoords[1])*(   pcoords[2])*g[2][i];
            displ[i] += this->nodeOffsets[compIndex][0] * (1.-pcoords[1])*(   pcoords[2])*g[3][i];

            displ[i] += this->nodeOffsets[compIndex][1] * (1.-pcoords[0])*(1.-pcoords[2])*g[4][i];
            displ[i] += this->nodeOffsets[compIndex][1] * (   pcoords[0])*(1.-pcoords[2])*g[5][i];
            displ[i] += this->nodeOffsets[compIndex][1] * (   pcoords[0])*(   pcoords[2])*g[6][i];
            displ[i] += this->nodeOffsets[compIndex][1] * (1.-pcoords[0])*(   pcoords[2])*g[7][i];

            displ[i] += this->nodeOffsets[compIndex][2] * (1.-pcoords[0])*(1.-pcoords[1])*g[8][i];
            displ[i] += this->nodeOffsets[compIndex][2] * (   pcoords[0])*(1.-pcoords[1])*g[9][i];
            displ[i] += this->nodeOffsets[compIndex][2] * (   pcoords[0])*(   pcoords[1])*g[10][i];
            displ[i] += this->nodeOffsets[compIndex][2] * (1.-pcoords[0])*(   pcoords[1])*g[11][i];
        }

    }
    else 
    {
        EXCEPTION1( ImproperUseException,
                    "avtIVPVTKOffsetField: unsupported cell in GetPositionCorrection");
    }

    debug5 << "-----> displacement for component " << compIndex << " is " << displ[0] << ' ' << displ[1] << ' ' << displ[2] << '\n';
    return displ;
}
