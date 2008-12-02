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

// ************************************************************************* //
//                            avtITAPS_CWriter.C                             //
// ************************************************************************* //

#include <avtITAPS_CWriter.h>

#include <avtDatabaseMetaData.h>
#include <DBOptionsAttributes.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

#include <iMesh.h>

#include <vtkDataSet.h>
#include <vtkType.h>

using std::map;
using std::string;
using std::vector;


// ****************************************************************************
//  Method: avtITAPS_CWriter constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   November 20, 2008 
//
// ****************************************************************************

avtITAPS_CWriter::avtITAPS_CWriter(DBOptionsAttributes *dbopts)
{
    for (int i = 0; dbopts != 0 && i < dbopts->GetNumberOfOptions(); ++i)
    {
    }
}


// ****************************************************************************
//  Method: avtITAPS_CWriter destructor
//
//  Programmer: Mark C. Miller 
//  Creation:   November 20, 2008 
//
// ****************************************************************************

avtITAPS_CWriter::~avtITAPS_CWriter()
{
}


// ****************************************************************************
//  Method: avtITAPS_CWriter::OpenFile
//
//  Purpose:
//
//  Programmer: November 20, 2008 
//  Creation:   September 11, 2003
//
// ****************************************************************************

void
avtITAPS_CWriter::OpenFile(const string &stemname, int nb)
{
    stem = stemname;
    nblocks = nb;
}

// ****************************************************************************
//  Method: avtITAPS_CWriter::WriteHeaders
//
//  Purpose:
//      This will write out the multi-vars for the ITAPS_C constructs.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 20, 2008 
//
// ****************************************************************************

void
avtITAPS_CWriter::WriteHeaders(const avtDatabaseMetaData *md,
                            vector<string> &scalars, vector<string> &vectors,
                            vector<string> &materials)
{
    const avtMeshMetaData *mmd = md->GetMesh(0);

}

// ****************************************************************************
//  Method: avtITAPS_CWriter::WriteChunk
//
//  Purpose:
//      This writes out one chunk of an avtDataset.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 20, 2008 
//
// ****************************************************************************

void
avtITAPS_CWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    //
    // Use sub-routines to do the mesh-type specific writes.
    //
    switch (ds->GetDataObjectType())
    {
       case VTK_UNSTRUCTURED_GRID:
         break;

       case VTK_STRUCTURED_GRID:
         break;

       case VTK_RECTILINEAR_GRID:
         break;

       case VTK_POLY_DATA:
         break;

       default:
         EXCEPTION0(ImproperUseException);
    }
}


// ****************************************************************************
//  Method: avtITAPS_CWriter::CloseFile
//
//  Purpose:
//      Closes the file.  This does nothing in this case.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 20, 2008 
//
// ****************************************************************************

void
avtITAPS_CWriter::CloseFile(void)
{
}

// ****************************************************************************
//  Method: avtITAPS_CWriter::VTKZoneTypeToITAPS_CZoneType
//
//  Purpose:
//      Converts a VTK cell type to the proper ITAPS_C zone type.
//
//  Arguments:
//      vtk_zonetype     Input VTK zone type.
//
//  Returns:     ITAPS_C zone type
//
//  Programmer: Cyrus Harrison
//  Creation:   February 26, 2007
//
// ****************************************************************************

int
avtITAPS_CWriter::VTKZoneTypeToITAPS_CZoneType(int vtk_zonetype)
{
#if 0
    int  silo_zonetype = -1;

    switch (vtk_zonetype)
    {
      case VTK_POLYGON:
        silo_zonetype = DB_ZONETYPE_POLYGON;
        break;
      case VTK_TRIANGLE:
        silo_zonetype = DB_ZONETYPE_TRIANGLE;
        break;
      case VTK_PIXEL:
      case VTK_QUAD:
        silo_zonetype = DB_ZONETYPE_QUAD;
        break;
      case VTK_TETRA:
        silo_zonetype = DB_ZONETYPE_TET;
        break;
      case VTK_PYRAMID:
        silo_zonetype = DB_ZONETYPE_PYRAMID;
        break;
      case VTK_WEDGE:
        silo_zonetype = DB_ZONETYPE_PRISM;
        break;
      case VTK_VOXEL:
      case VTK_HEXAHEDRON:
        silo_zonetype = DB_ZONETYPE_HEX;
        break;
      case VTK_LINE:
        silo_zonetype = DB_ZONETYPE_BEAM;
        break;
    }

    return silo_zonetype;
#endif
}
