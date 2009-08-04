/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                          avtVertexNormalsFilter.C                         //
// ************************************************************************* //

#include <avtVertexNormalsFilter.h>

#include <vtkPolyData.h>
#include <vtkStructuredGrid.h>
#include <vtkVisItPolyDataNormals.h>
#include <vtkVisItStructuredGridNormals.h>

#include <avtDataset.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtVertexNormalsFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//
//    Hank Childs, Thu Feb 26 09:55:59 PST 2004
//    Removed pointNormals variable.  Whether or not to do point normals is
//    now decided dynamically.
//
// ****************************************************************************

avtVertexNormalsFilter::avtVertexNormalsFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtVertexNormalsFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtVertexNormalsFilter::~avtVertexNormalsFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtVertexNormalsFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the VertexNormals filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: Hank Childs 
//  Creation:   December 31, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Jun 20 13:59:01 PDT 2002
//    Be more careful about what sort of poly data gets sent in (no lines).
//
//    Hank Childs, Tue Aug  6 11:11:50 PDT 2002
//    Do not execute if normals have been deemed inappropriate.
//
//    Hank Childs, Sun Aug 18 11:14:20 PDT 2002
//    Do not find normals for disjoint element meshes.
//
//    Hank Childs, Tue Sep 10 15:17:21 PDT 2002
//    Manage the output's memory without the use of data members.
//
//    Jeremy Meredith, Thu Oct 24 17:40:18 PDT 2002
//    Upped the feature angle to 45 degrees.
//
//    Jeremy Meredith, Mon Jan  6 10:44:59 PST 2003
//    Disabled consistency checking because (1) it has bugs, (2) it is slow,
//    and (3) we are careful enough about cell orderings that we don't need it.
//
//    Jeremy Meredith, Wed Aug 13 18:13:14 PDT 2003
//    Made it use the new VisIt poly data normals filter.  Allowed
//    cell normals as well as just point normals.
//
//    Hank Childs, Thu Feb 26 09:50:37 PST 2004
//    Decide what type of normals (point, cell) to do from inside this filter.
//
//    Jeremy Meredith, Wed Oct 27 15:22:32 PDT 2004
//    Removed check for ensuring that polgons actually existed before doing
//    the normals.  The vtkVisItPolyDataNormals filter accepts them just fine,
//    and we can count on the spatial/topological tests to rule out cases
//    where it will be inefficient to compute normals anyway.  There were
//    cases where some domains had polgons, and others only had lines, but
//    the final append filter removed *all* normals because some domains
//    did not have any.
//
//    Hank Childs, Sat Feb 19 14:58:42 PST 2005
//    Break all memory references.  I didn't think this was necessary, but
//    the data wouldn't delete until I did this.
//
//    Kathleen Bonnell, Tue May 16 09:41:46 PDT 2006 
//    Removed call to SetSource(NULL), with new vtk pipeline, it also removes
//    necessary information from the dataset. 
//
//    Hank Childs, Thu Dec 28 15:25:50 PST 2006
//    Add support for direct normals calculation of structured grids.
//
// ****************************************************************************

vtkDataSet *
avtVertexNormalsFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();

    if (atts.GetSpatialDimension() != 3 || atts.GetTopologicalDimension() != 2)
    {
        return in_ds;
    }

    if (GetInput()->GetInfo().GetValidity().NormalsAreInappropriate())
    {
        return in_ds;
    }

    if (GetInput()->GetInfo().GetValidity().GetDisjointElements() == true)
    {
        return in_ds;
    }

    if (in_ds->GetDataObjectType() == VTK_POLY_DATA)
    {
        vtkPolyData *pd = (vtkPolyData *)in_ds;
    
        bool pointNormals = true;
        if (atts.ValidActiveVariable())
        {
            avtCentering cent = atts.GetCentering();
            if (cent == AVT_ZONECENT)
                pointNormals = false;
        }
        vtkVisItPolyDataNormals *normals = vtkVisItPolyDataNormals::New();
        normals->SetInput(pd);
        normals->SetFeatureAngle(45.);
        if (pointNormals)
            normals->SetNormalTypeToPoint();
        else
            normals->SetNormalTypeToCell();
        normals->SetSplitting(true);
        normals->Update();
    
        vtkPolyData *out_ds = normals->GetOutput();
        //out_ds->SetSource(NULL);
        ManageMemory(out_ds);
        normals->Delete();
    
        return out_ds;
    }
    else if (in_ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *)in_ds;
    
        bool pointNormals = true;
        if (atts.ValidActiveVariable())
        {
            avtCentering cent = atts.GetCentering();
            if (cent == AVT_ZONECENT)
                pointNormals = false;
        }
        vtkVisItStructuredGridNormals *normals = 
                                            vtkVisItStructuredGridNormals::New();
        normals->SetInput(sgrid);
        if (pointNormals)
            normals->SetNormalTypeToPoint();
        else
            normals->SetNormalTypeToCell();

        normals->Update();
    
        vtkStructuredGrid *out_ds = normals->GetOutput();
        //out_ds->SetSource(NULL);
        ManageMemory(out_ds);
        normals->Delete();
    
        return out_ds;
    }

    // Don't know what to do with other grid types.
    debug1 << "Sent unsupported grid type into normals filter" << endl;
    return in_ds;
}


// ****************************************************************************
//  Method:  avtVertexNormalsFilter::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************

bool
avtVertexNormalsFilter::FilterUnderstandsTransformedRectMesh()
{
    // Creating normals for a transformed rectilinar mesh is okay;
    // the normals will be transformed when the mesh is.
    return true;
}
