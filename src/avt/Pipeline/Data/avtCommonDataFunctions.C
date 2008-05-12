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
//                        avtCommonDataFunctions.C                           //
// ************************************************************************* //

#include <avtCommonDataFunctions.h>

#include <float.h>
#include <cmath>
#include <vector>

#include <visit-config.h>

#include <vtkAppendFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

#include <avtCallback.h>
#include <avtDataTree.h>

#include <NoInputException.h>
#include <DebugStream.h>

#ifdef HAVE_LIBBZ2
#include <bzlib.h>
#include <TimingsManager.h>
#endif

using std::vector;
using std::string;
using namespace std;


void GetDataScalarRange(vtkDataSet *, double *, const char *, bool);
void GetDataMagnitudeRange(vtkDataSet *, double *, const char *, bool);
void GetDataMajorEigenvalueRange(vtkDataSet *, double *, const char *, bool);


// ****************************************************************************
//  Method: CGetSpatialExtents
//
//  Purpose:
//      Gets the spatial extents of the data.
//
//  Arguments:
//    data        The data from which to retrieve the extents.
//    se          A place to put the spatial extents.
//    success     Assigned true if operation successful, false otherwise. 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Traverse.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     April 17, 2001 
//
//  Modifications:
//
//    Hank Childs, Tue Jun 19 12:30:13 PDT 2001
//    Account for dummy vtk datasets - datasets with no cells that return
//    zero-to-zero for extents.
//
//    Hank Childs, Tue Jun 26 18:30:45 PDT 2001
//    Allow for vector plots that have 0 cells, but many points.
//
//    Kathleen Bonnell, Tue Jul 24 14:12:27 PDT 2001
//    Allow for successful setting of extents when a tree has some
//    vtk datasets with data and some without data by using |= to
//    set success in the false case. 
// 
//    Hank Childs, Fri Sep  7 17:58:58 PDT 2001
//    Use doubles rather than floats.
//
//    Kathleen Bonnell, Thu Oct 4 13:36:54 PDT 2001 
//    Change if statement regarding number of cells/points to && from ||.
//
//    Hank Childs, Tue Nov  6 14:41:52 PST 2001
//    Make sure that there are no UMRs.
//
//    Jeremy Meredith, Thu Feb 15 12:55:16 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//    For these grids, we apply the transform to all eight corners of the
//    grid and compare each for the min/max along every coordinate axis.
//    Note that both the spatial extents array and the implied transform
//    must be passed in as input now.
//
//    Jeremy Meredith, Tue Feb 27 10:51:17 EST 2007
//    Fixed the code to find extents of transformed rect grids -- it was
//    only working for special cases before.  I also made it clearer.
//
// ****************************************************************************

void 
CGetSpatialExtents(avtDataRepresentation &data, void *info, bool &success)
{
    typedef struct {double *se; const double *xform;} tmpstruct;
    double *fse = ((tmpstruct*)info)->se;
    const double *xform = ((tmpstruct*)info)->xform;

    if (data.Valid())
    {
        vtkDataSet *ds = data.GetDataVTK();
        if (xform && ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
        {
            double bnds[6];
            ds->GetBounds(bnds);
            for (int i=0; i<=1; i++)
            {
                for (int j=0; j<=1; j++)
                {
                    for (int k=0; k<=1; k++)
                    {
                        float pt_in[4]={bnds[0*2+i],bnds[1*2+j],bnds[2*2+k],1};
                        float pt[4];
                        vtkMatrix4x4::MultiplyPoint(xform, pt_in, pt);
                        for (int axis=0; axis<3; axis++)
                        {
                            if (fse[2*axis] > pt[axis])
                                fse[2*axis] = pt[axis];
                            if (fse[2*axis+1] < pt[axis])
                                fse[2*axis+1] = pt[axis];
                        }
                    }
                }
            }
        }
        else if (ds->GetNumberOfCells() > 0 && ds->GetNumberOfPoints() > 0)
        {
            double bounds[6];
            ds->GetBounds(bounds);

            //
            // If we have gotten extents from another data rep, then merge the
            // extents.  Otherwise copy them over.
            //
            if (success)
            {
                for (int j = 0 ; j < 3 ; j++)
                {
                    if (bounds[2*j] < fse[2*j])
                    {
                        fse[2*j] = bounds[2*j];
                    }
                    if (bounds[2*j+1] > fse[2*j+1])
                    {
                        fse[2*j+1] = bounds[2*j+1];
                    }
                }
            }
            else
            {
                for (int j = 0 ; j < 6 ; j++)
                {
                    fse[j] = bounds[j];
                }
            }
            success = true;
        }
        else
        {
            success |= false;
        }
    }
    else 
    {
        debug1 << "Attempting to retrieve Spatial Extents "
               << "of non-existent data." << endl;
        success = false;
    }
}


// ****************************************************************************
//  Method: CGetDataExtents
//
//  Purpose:
//      Gets the data extents of the dataset.
//
//  Arguments:
//    data        The data represention from which to get the extents.
//    de          A place to put the data extents.
//    success     Assigned true if operation successful, false otherwise. 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Traverse.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     April 17, 2001 
//
//  Modifications:
//
//    Hank Childs, Tue Jun 19 12:30:13 PDT 2001
//    Account for dummy vtk datasets - datasets with no cells that return
//    zero-to-zero for extents.
//
//    Hank Childs, Tue Jun 26 18:30:45 PDT 2001
//    Allow for vector plots that have 0 cells, but many points.
//
//    Kathleen Bonnell, Tue Jul 24 14:12:27 PDT 2001
//    Allow for successful setting of extents when a tree has some
//    vtk datasets with data and some without data by using |= to
//    set success in the false case. 
// 
//    Hank Childs, Fri Sep  7 17:15:03 PDT 2001
//    Do not assume that extents are scalars or floats.
//
//    Kathleen Bonnell, Mon Oct  8 12:45:31 PDT 2001 
//    Do not overwrite data extents, merge them instead.  Since
//    this method uses GetDataRange, use that method's criteria
//    for determing the number of <min,max> tuples that will be retrieved. 
//    Use && instead of || in if statement regarding number of cells/points.
//
//    Hank Childs, Tue Nov  6 14:41:52 PST 2001
//    Make sure that there are no UMRs.
//
//    Hank Childs, Fri Dec 14 10:57:36 PST 2001
//    Support against memory overwrites.
//
//    Hank Childs, Tue Feb 24 15:08:59 PST 2004
//    Add support for multiple variables.
//
//    Kathleen Bonnell, Thu Mar 11 10:32:04 PST 2004 
//    Call appropriate Get*Range method for the number of tuples in the
//    data array.   DataExtents now always have only 2 components.
//
//    Hank Childs, Wed Oct 10 16:03:59 PDT 2007
//    Add argument for whether or not we should ignore ghost zones.
//
// ****************************************************************************

void 
CGetDataExtents(avtDataRepresentation &data, void *g, bool &success) 
{
    if (data.Valid())
    {
        vtkDataSet *ds = data.GetDataVTK();
        if (ds->GetNumberOfCells() > 0 && ds->GetNumberOfPoints() > 0)
        {
            GetVariableRangeArgs *gvra = (GetVariableRangeArgs *) g;
            double *dde = gvra->extents;
            const char *vname = gvra->varname;

            vtkDataArray *da = NULL;
            if (ds->GetPointData()->GetArray(vname) != NULL)
                da = ds->GetPointData()->GetArray(vname);
            else
                da = ds->GetCellData()->GetArray(vname);

            if (da == NULL)
                return;

            int dim = da->GetNumberOfComponents();
            double range[2] = {+DBL_MAX, -DBL_MAX};
            bool ignoreGhost = false; // legacy behavior
            if (dim == 1)
                GetDataScalarRange(ds, range, vname, ignoreGhost);
            else if (dim <= 3)
                GetDataMagnitudeRange(ds, range, vname, ignoreGhost);
            else if (dim == 9)
                GetDataMajorEigenvalueRange(ds, range, vname, ignoreGhost);

            //
            // If we have gotten extents from another data rep, then merge the
            // extents.  Otherwise copy them over.
            //
            if (success)
            {
                if (range[0] < dde[0])
                {
                    dde[0] = range[0];
                }
                if (range[1] > dde[1])
                {
                    dde[1] = range[1];
                }
            }
            else
            {
                dde[0] = range[0];
                dde[1] = range[1];
            }
            success = true;
        }
        else
        {
            success |= false;
        } 
    }
    else 
    {
        debug1 << "Attempting to retrieve Data Extents "
               << "of non-existent data." << endl;
        success = false;
    }
}


// ****************************************************************************
//  Method: CGetNumberOfZones
//
//  Purpose:
//    Adds the number of cells in the vtk input to the passed sum argument. 
//
//  Arguments:
//    data      The data from which to calculate number of cells.
//    sum       A place to store the cumulative number of cells.
//    <unused> 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 18, 2001
//
// ****************************************************************************

void
CGetNumberOfZones(avtDataRepresentation &data, void *sum, bool &)
{
    int *numCells = (int*)sum;
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    vtkDataSet *ds = data.GetDataVTK();
    *numCells += ds->GetNumberOfCells();
}


// ****************************************************************************
//  Method: CConvertUnstructuredGridToPolyData
//
//  Purpose:
//      Converts unstructured grids to poly data.
//
//  Arguments:
//    data      The data which may be an unstructured grid, but should be
//              a poly data.
//    <unused>
//    <unused> 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Hank Childs
//  Creation:   July 27, 2004
//
//  Modifications:
//
//    Hank Childs, Thu Jul 29 17:24:40 PDT 2004
//    Copy over field data as well.
//
// ****************************************************************************

void
CConvertUnstructuredGridToPolyData(avtDataRepresentation &data, void *, bool &)
{
    if (!data.Valid())
    {
        return; // This is a problem, but no need to flag it for this...
    }

    vtkDataSet *ds = data.GetDataVTK();
    if (ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGrid *ugrid = (vtkUnstructuredGrid *) ds;
        vtkPolyData *out_pd = vtkPolyData::New();
        out_pd->SetPoints(ugrid->GetPoints());
        out_pd->GetPointData()->ShallowCopy(ugrid->GetPointData());
        out_pd->GetCellData()->ShallowCopy(ugrid->GetCellData());
        out_pd->GetFieldData()->ShallowCopy(ugrid->GetFieldData());
        int ncells = ugrid->GetNumberOfCells();
        out_pd->Allocate(ncells);
        for (int i = 0 ; i < ncells ; i++)
        {
            int celltype = ugrid->GetCellType(i);
            vtkIdType *pts;
            int npts;
            ugrid->GetCellPoints(i, npts, pts);
            out_pd->InsertNextCell(celltype, npts, pts);
        }
        avtDataRepresentation new_data(out_pd, data.GetDomain(),
                                       data.GetLabel());
        data = new_data;
        out_pd->Delete();
    }
}


// ****************************************************************************
//  Method: CBreakVTKPipelineConnections
//
//  Purpose:
//      Breaks all VTK pipeline connections by creating shallow copies.
//
//  Arguments:
//    data      The data from which to calculate number of cells.
//    arg       optional bool pointer for enabling debug mode
//    <unused> 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Hank Childs
//  Creation:   January 16, 2007
//
//  Modifications:
//
//    Cyrus Harrison, Sat Aug 11 19:44:59 PDT 2007
//    Add support for vtk-debug mode.
//
// ****************************************************************************

void
CBreakVTKPipelineConnections(avtDataRepresentation &data, void *arg, bool &)
{
    if (!data.Valid())
    {
        return; // This is a problem, but no need to flag it for this...
    }

    // loop index
    int i;

    vtkDataSet *ds = data.GetDataVTK();
    vtkDataSet *newDS = (vtkDataSet *) ds->NewInstance();
    newDS->ShallowCopy(ds);
    avtDataRepresentation new_data(newDS, data.GetDomain(),
                                   data.GetLabel());

    // If vtk-debug turn on debug for the new dataset and its vars
    if(arg != NULL &&  *((bool*)arg))
    {
        newDS->DebugOn();
        vtkCellData  *cell_data  = newDS->GetCellData();
        vtkPointData *point_data = newDS->GetPointData();

        int ncell_arrays = cell_data->GetNumberOfArrays();
        for( i=0; i < ncell_arrays; i++)
            cell_data->GetArray(i)->DebugOn();

        int npoint_arrays = point_data->GetNumberOfArrays();
        for( i=0; i < npoint_arrays; i++)
            point_data->GetArray(i)->DebugOn();
    }
    
    data = new_data;
    newDS->Delete();
}


// ****************************************************************************
//  Method: CGetVariableList
//
//  Purpose:
//      Makes a list of the variable over the dataset.
//
//  Arguments:
//    data      The data.
//    nv        A void pointer (that is really a pointer to a VarList struct).
//    success   A boolean.
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2001
//
//  Modifications:
//
//    Hank Childs, Thu May 31 13:49:59 PDT 2007
//    Set varsizes as well.
//
// ****************************************************************************

void
CGetVariableList(avtDataRepresentation &data, void *nv, bool &success)
{
    int   i;

    if (success)
    {
        // We already got it, why continue?
        return;
    }

    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }

    VarList *vl = (VarList *) nv;
    vtkDataSet *ds = data.GetDataVTK();
    vl->nvars = ds->GetPointData()->GetNumberOfArrays() + 
                ds->GetCellData()->GetNumberOfArrays();
    vl->varnames.clear();
    vl->varsizes.clear();
    for (i = 0 ; i < ds->GetCellData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *dat = ds->GetCellData()->GetArray(i);
        const char *name = dat->GetName();
        if (name == NULL)
        {
            debug1 << "Warning: using unnamed variable" << endl;
            vl->varnames.push_back(string("unnamed"));
        }
        else
        {
            vl->varnames.push_back(string(name));
        }
        vl->varsizes.push_back(dat->GetNumberOfComponents());
    }
    for (i = 0 ; i < ds->GetPointData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *dat = ds->GetPointData()->GetArray(i);
        const char *name = dat->GetName();
        vl->varnames.push_back(string(name));
        vl->varsizes.push_back(dat->GetNumberOfComponents());
    }

    success = true;
}


// ****************************************************************************
//  Method: CUpdateData
//
//  Purpose:
//    Updates the vtk input.  
//
//  Arguments:
//    data      The data to be updated.
//    <unused>
//    modified  True if the data was modified.
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 18, 2001
//
// ****************************************************************************

void
CUpdateData(avtDataRepresentation &data, void *, bool &modified)
{
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    vtkDataSet *ds = data.GetDataVTK();
    unsigned long mtime = ds->GetMTime();
    ds->Update();
    if (mtime != ds->GetMTime())
        modified = true;
}


// ****************************************************************************
//  Method: CAddInputToAppendFilter
//
//  Purpose:
//    Updates the vtk input.  
//
//  Arguments:
//    data      The data to add  to the filter.
//    filter    The filter to use.
//    <unused> 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 18, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Sat Sep 22 12:37:59 PDT 2001
//    Added vtkAppendPolyData.  Only use filters on unstructured grids
//    or poly data.
//
//    Hank Childs, Tue Sep 25 09:32:26 PDT 2001
//    Add check for no cells.
//
//    Kathleen Bonnell, Wed May 17 14:51:16 PDT 2006
//    Remove call to SetSource(NULL) as it now removes information necessary
//    to the dataset. 
//
// ****************************************************************************

void
CAddInputToAppendFilter(avtDataRepresentation & data, void *arg, bool &)
{
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    vtkDataSet *ds = data.GetDataVTK();
    if (ds == NULL)
    {
        EXCEPTION0(NoInputException);
    }
    if (ds->GetNumberOfCells() <= 0)
    {
        debug5 << "Not adding input to append filter since it has no cells."
               << endl;
        return;
    }
    struct map
    {
        vtkAppendFilter *af;
        vtkAppendPolyData *pf;
    } *pmap;

    pmap = (struct map *) arg;

    // using SetSource(NULL) no longer a good idea.
    //ds->SetSource(NULL);

    //
    //  We only want to use the append filters on poly data or
    //  unstructured grid data.
    //
    if (ds->GetDataObjectType() == VTK_POLY_DATA)
    {
        pmap->pf->AddInput((vtkPolyData*)ds);
    }
    else if (ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID) 
    {
        pmap->af->AddInput(ds);
    }
}


// ****************************************************************************
//  Method: CGetAllDatasets
//
//  Purpose:
//      Puts all datasets into an append filter.
//
//  Arguments:
//    data      The data to add to the list.
//    list      The list.
//    <unused> 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

void
CGetAllDatasets(avtDataRepresentation & data, void *arg, bool &)
{
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    vtkDataSet *ds = data.GetDataVTK();
    if (ds == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    GetAllDatasetsArgs *args = (GetAllDatasetsArgs *) arg;
    args->datasets.push_back(ds);
    args->domains.push_back(data.GetDomain());
    args->labels.push_back(data.GetLabel());
}


// ****************************************************************************
//  Method: CPruneByDomainList
//
//  Purpose:
//      Takes a domain list and prunes off all IDs that are not in the list.
//
//  Arguments:
//    data      The data to use as input to the mappers.
//    arg       The arguments specific to this function.
//    success   Assigned true if operation successful, false otherwise. 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2008
//
// ****************************************************************************

void
CPruneByDomainList(avtDataRepresentation & data, void *arg, bool &success)
{
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }

    struct map
    {
        vector<bool>           lookup;
        vector<avtDataTree_p>  new_nodes;
    } *pmap;

    pmap = (struct map *) arg;

    bool keepIt = false;
    if (data.GetDomain() < 0)
        keepIt = true;
    else if (data.GetDomain() < pmap->lookup.size())
        keepIt = pmap->lookup[data.GetDomain()];
    if (keepIt)
    {
        success = true;
        avtDataTree_p child = new avtDataTree(data);
        pmap->new_nodes.push_back(child);
    }
    else
    {
        success |= false;
    }
}


// ****************************************************************************
//  Method: CGetChunkByDomain
//
//  Purpose:
//    Gets the datatree that holds all the datasets corresponding to a given
//    chunk id. 
//
//  Arguments:
//    data      The data to use as input to the mappers.
//    arg       Mappers and other necessary info.
//    success   Assigned true if operation successful, false otherwise. 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2001
//
// ****************************************************************************

void
CGetChunkByDomain(avtDataRepresentation & data, void *arg, bool &success)
{
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }

    struct map
    {
        int           chunkId;
        avtDataTree_p subTree;
    } *pmap;

    pmap = (struct map *) arg;

    if (data.GetDomain() == pmap->chunkId)
    {
        success = true;
        avtDataTree_p child = new avtDataTree(data);
        // test for first "real" child added to this tree:
        if (pmap->subTree->GetNChildren() == 1 
            && !(pmap->subTree->ChildIsPresent(0)))
        {
            pmap->subTree = child;
        }
        else 
        {
            pmap->subTree->Merge(child);
        }
    }
    else
    {
        success |= false;
    }
}


// ****************************************************************************
//  Method: CGetChunkByLabel
//
//  Purpose:
//    Gets the datatree that holds all the datasets corresponding to a given
//    label. 
//
//  Arguments:
//    data      The data to use as input to the mappers.
//    arg       Mappers and other necessary info.
//    success   Assigned true if operation successful, false otherwise. 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2001
//
// ****************************************************************************

void
CGetChunkByLabel(avtDataRepresentation & data, void *arg, bool &success)
{
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }

    struct map
    {
        std::string     label;
        avtDataTree_p subTree;
    } *pmap;

    pmap = (struct map *) arg;

    if (data.GetLabel() == pmap->label)
    {
        success = true;
        avtDataTree_p child = new avtDataTree(data);
        // test for first "real" child added to this tree:
        if (pmap->subTree->GetNChildren() == 1 
            && !(pmap->subTree->ChildIsPresent(0)))
        {
            pmap->subTree = child;
        }
        else 
        {
            pmap->subTree->Merge(child);
        }
    }
    else
    {
        success |= false;
    }
}


// ****************************************************************************
//  Function: CSetActiveVariable
//
//  Purpose:
//      Sets the active variable for the dataset.
//
//  Arguments:
//    data      The data whose variables should be set.
//    arg       A struct with information about setting the active variable.
//    success   Assigned true if operation successful, false otherwise. 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Nov  5 16:49:48 PST 2001
//    Add support for vectors.
//
//    Hank Childs, Wed Dec  5 09:08:50 PST 2001
//    Swapping between zone-centered and node-centered variables required more
//    work so that VTK could get the extents right later.
//
//    Hank Childs, Fri Jan  4 14:26:10 PST 2002
//    Make sure that we don't accidently bring a reference count down to zero
//    before adding it back to a dataset.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Wed Apr 17 18:39:04 PDT 2002
//    Be a little more selective about setting the active variable.
//
//    Hank Childs, Mon Sep 22 08:18:38 PDT 2003
//    Account for tensors.
//
//    Hank Childs, Sat Dec 13 16:06:07 PST 2003
//    Set the dimension of the active variable.
//
//    Hank Childs, Fri Aug 19 10:19:57 PDT 2005
//    Variable names should never be NULL.  But sometimes they are if another
//    filter has screwed up.  Don't crash in this case.
//
// ****************************************************************************

void
CSetActiveVariable(avtDataRepresentation &data, void *arg, bool &success)
{
    int   i;
    SetActiveVariableArgs *args = (SetActiveVariableArgs *) arg;
    debug5 << "Making " << args->varname << " the active variable" << endl;

    //
    // Perform some checks of the input.
    //
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    vtkDataSet *ds = data.GetDataVTK();
    if (ds == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    vtkPointData *pd = ds->GetPointData();
    vtkCellData  *cd = ds->GetCellData();

    //
    // Set the active variable.  No need to figure out whether it is cell
    // data or point data, just set them both.
    //
    vtkDataArray *arr = pd->GetArray(args->varname);
    if (arr != NULL)
    {
        if (arr->GetNumberOfComponents() == 1)
            pd->SetActiveScalars(args->varname);
        else if (arr->GetNumberOfComponents() == 3)
            pd->SetActiveVectors(args->varname);
        else if (arr->GetNumberOfComponents() == 9)
            pd->SetActiveTensors(args->varname);
        args->activeVarDim = arr->GetNumberOfComponents();
    }
    arr = cd->GetArray(args->varname);
    if (arr != NULL)
    {
        if (arr->GetNumberOfComponents() == 1)
            cd->SetActiveScalars(args->varname);
        else if (arr->GetNumberOfComponents() == 3)
            cd->SetActiveVectors(args->varname);
        else if (arr->GetNumberOfComponents() == 9)
            cd->SetActiveTensors(args->varname);
        args->activeVarDim = arr->GetNumberOfComponents();
    }

    //
    // Now get information about the dataset that some filters are interested
    // in.
    //
    args->hasPointVars  = false;
    args->hasCellVars   = false;
    args->activeIsPoint = false;
    for (i = 0 ; i < pd->GetNumberOfArrays() ; i++)
    {
        args->hasPointVars = true;
        if (pd->GetArrayName(i) == NULL)
        {
            debug1 << "WARNING: NULL variable name present in data set." 
                   << endl;
            continue;
        }
        if (strcmp(args->varname, pd->GetArrayName(i)) == 0)
        {
            args->activeIsPoint = true;
            // We now want to remove the cell-centered array if there is one.
            // We may have to add it back as a data array.
            vtkDataArray *cs = cd->GetScalars();
            if (cs != NULL)
            {
                cs->Register(NULL);
                cd->SetScalars((vtkDataArray *) NULL);
                vtkDataArray *havearr = cd->GetArray(cs->GetName());
                if (!havearr)
                {
                    cd->AddArray(cs);
                }
                cs->Delete();
            }
        }
    }
    for (i = 0 ; i < cd->GetNumberOfArrays() ; i++)
    {
        args->hasCellVars = true;
        if (cd->GetArrayName(i) == NULL)
        {
            debug1 << "WARNING: NULL variable name present in data set." 
                   << endl;
            continue;
        }
        if (strcmp(args->varname, cd->GetArrayName(i)) == 0)
        {
            args->activeIsPoint = false;
            // We now want to remove the cell-centered array if there is one.
            // We may have to add it back as a data array.
            vtkDataArray *ps = pd->GetScalars();
            if (ps != NULL)
            {
                ps->Register(NULL);
                pd->SetScalars((vtkDataArray *) NULL);
                vtkDataArray *havearr = pd->GetArray(ps->GetName());
                if (!havearr)
                {
                    pd->AddArray(ps);
                }
                ps->Delete();
            }
        }
    }

    success = true;
}


// ****************************************************************************
//  Function: CExpandSingletonConstants
//
//  Purpose:
//      Expands all constants that are stored as singletons.
//
//  Arguments:
//    data      The data whose constants should be expanded.
//    <unused>  An unused variable.
//    success   Assigned true if operation successful, false otherwise. 
//
//  Programmer: Hank Childs
//  Creation:   January 13, 2008
//
// ****************************************************************************

void
CExpandSingletonConstants(avtDataRepresentation &data, void *, bool &success)
{
    //
    // Perform some checks of the input.
    //
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    vtkDataSet *ds = data.GetDataVTK();
    if (ds == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    vtkDataSetAttributes *atts[2];
    atts[0] = ds->GetPointData();
    atts[1] = ds->GetCellData();
   
    int ntups[2];
    ntups[0] = ds->GetNumberOfPoints();
    ntups[1] = ds->GetNumberOfCells();

    for (int i = 0 ; i < 2 ; i++)
    {
        if (ntups[i] <= 1)
            continue;

        int nvars = atts[i]->GetNumberOfArrays(); 
        for (int j = 0 ; j < nvars ; j++)
        {
            vtkDataArray *arr = atts[i]->GetArray(j);
            if (arr->GetNumberOfTuples() == 1)
            {
                int ncomps = arr->GetNumberOfComponents();
                double *constVals = new double[ncomps];
                int k;
                for (k = 0 ; k < ncomps ; k++)
                    constVals[k] = arr->GetComponent(0, k);
                arr->SetNumberOfTuples(ntups[i]);
                for (k = 0 ; k < ntups[i] ; k++)
                    arr->SetTuple(k, constVals);
                delete [] constVals;
            }
        }
    }

    success = true;
}


// ****************************************************************************
//  Function: CRemoveVariable
//
//  Purpose:
//      Removes a data array (variable) from a dataset.
//
//  Arguments:
//    data      The data whose variables should be set.
//    arg       The variable to remove.
//    success   Assigned true if operation successful, false otherwise. 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Hank Childs
//  Creation:   November 2, 2001
//
// ****************************************************************************

void
CRemoveVariable(avtDataRepresentation &data, void *arg, bool &success)
{
    char *var = (char *) arg;

    //
    // Perform some checks of the input.
    //
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    vtkDataSet *ds = data.GetDataVTK();
    if (ds == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    ds->GetPointData()->RemoveArray(var);
    ds->GetCellData()->RemoveArray(var);

    success = true;
}


// ****************************************************************************
//  Function: GetDataRange
//
//  Purpose:
//      Gets the data range from a VTK dataset.
//
//  Arguments:
//      ds      The dataset to determine the range for.
//      exts    The extents in <min, max> form.  There may be many 3 sets of
//              extents for vector data.
//      vname   The variable name to get the range for.
//      ignoreGhost  A Boolean.  True if we should ignore ghosts, else false.
//
//  Returns:    True if retrieving the range was successful, false otherwise.
//   
//  Programmer: Kathleen Bonnell 
//  Creation:   March 11, 2004
//
//  Modifications:
//
//    Hank Childs, Wed Oct 10 15:56:16 PDT 2007
//    Added argument for ignoring values from ghost zones.
//
// ****************************************************************************

void
GetDataRange(vtkDataSet *ds, double *de, const char *vname,
             bool ignoreGhost)
{
    if (ds->GetNumberOfCells() > 0 && ds->GetNumberOfPoints() > 0)
    {
        vtkDataArray *da = NULL;
        if (ds->GetPointData()->GetArray(vname) != NULL)
            da = ds->GetPointData()->GetArray(vname);
        else
            da = ds->GetCellData()->GetArray(vname);

        if (da == NULL)
            return;

        int dim = da->GetNumberOfComponents();
        if (dim == 1)
            GetDataScalarRange(ds, de, vname, ignoreGhost);
        else if (dim <= 3)
            GetDataMagnitudeRange(ds, de, vname, ignoreGhost);
        else if (dim == 9)
            GetDataMajorEigenvalueRange(ds, de, vname, ignoreGhost);
    }
}


// ****************************************************************************
//  Function: GetDataScalarRange
//
//  Purpose:
//      Gets the data range from a VTK dataset.
//
//  Arguments:
//      ds      The dataset to determine the range for.
//      exts    The extents in <min, max> form.  There may be many 3 sets of
//              extents for vector data.
//      vname   The variable name to get the range for.
//      ignoreGhost  A Boolean.  True if we should ignore ghosts, else false.
//
//  Returns:    true if it found real data, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   September 7, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkVectors has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Tue Feb 24 14:54:28 PST 2004
//    Added a variable to get the range for.
//
//    Kathleen Bonnell, Thu Mar 11 10:32:04 PST 2004 
//    Renamed from GetDataRange. DataExtents now always have only 2 components.
//
//    Kathleen Bonnell, Tue May 11 08:02:51 PDT 2004
//    Added support for VTK_DOUBLE. 
//
//    Mark C. Miller, Tue Dec  5 18:14:58 PST 2006
//    Templatized it to support all array types.
// 
//    Hank Childs, Wed Oct 10 15:56:16 PDT 2007
//    Added argument for ignoring values from ghost zones.
//
//    Gunther H. Weber, Fri Feb  1 11:55:59 PST 2008
//    Skip nan, -inf and +inf in min/max calculation
//
//    Cyrus Harrison, Mon Feb 25 11:03:03 PST 2008
//    Changed std::isfinite to isfinite to work around an AIX compiler bug.
//
//    Kathleen Bonnell, Thu Mar  6 09:15:46 PST 2008 
//    Use _finite on Windows. 
//
//    Eric Brugger, Tue Apr  8 10:01:07 PDT 2008
//    Make the use of isfinite conditional, since not all platforms support
//    it (IRIX64 6.5 with MIPSpro 7.41, solaris with gcc 3.2).
//
// ****************************************************************************

template <class T> static bool
GetScalarRange(T *buf, int n, double *exts, unsigned char *ghosts)
{
    T min; 
    T max;
    bool setOne = false;
    for (int i = 0; i < n; i++, buf++)
    {
        if ((ghosts != NULL) && (ghosts[i] != '\0'))
            continue;

#ifndef _WIN32
#ifdef HAVE_ISFINITE
        if (!isfinite(*buf))
            continue;
#endif
#else
        if (!_finite(*buf))
            continue;
#endif

        if (!setOne)
        {
            min = *buf;
            max = *buf;
            setOne = true;
            continue;
        }

        if (*buf < min)
        {
            min = *buf;
        }
        else
        {
            if (*buf > max)
                max = *buf;
        }
    }
    exts[0] = (double) min;
    exts[1] = (double) max;

    return setOne;
}

void
GetDataScalarRange(vtkDataSet *ds, double *exts, const char *vname,
                   bool ignoreGhost)
{
    vtkDataArray *da = NULL;
    unsigned char *ghosts = NULL;
    if (ds->GetPointData()->GetArray(vname))
    {
        da = ds->GetPointData()->GetArray(vname);
    }
    else
    {
        da = ds->GetCellData()->GetArray(vname);
        if (ignoreGhost)
        {
            vtkUnsignedCharArray *ga = (vtkUnsignedCharArray *)
                                  ds->GetCellData()->GetArray("avtGhostZones");
            if (ga != NULL)
                ghosts = ga->GetPointer(0);
        }
    }

    if (da == NULL)
        return;

    int nvals = da->GetNumberOfTuples();

    exts[0] = +FLT_MAX;
    exts[1] = -FLT_MAX;
    
    switch (da->GetDataType())
    {
        case VTK_CHAR:
            GetScalarRange((char*) da->GetVoidPointer(0), nvals, exts, 
                           ghosts);
            break;
        case VTK_UNSIGNED_CHAR:
            GetScalarRange((unsigned char*) da->GetVoidPointer(0), nvals, exts,
                           ghosts);
            break;
        case VTK_SHORT:
            GetScalarRange((short*) da->GetVoidPointer(0), nvals, exts,
                           ghosts);
            break;
        case VTK_UNSIGNED_SHORT:
            GetScalarRange((unsigned short*) da->GetVoidPointer(0), nvals,exts,
                           ghosts);
            break;
        case VTK_INT:           
            GetScalarRange((int*) da->GetVoidPointer(0), nvals, exts,
                           ghosts);
            break;
        case VTK_UNSIGNED_INT:  
            GetScalarRange((unsigned int*) da->GetVoidPointer(0), nvals, exts,
                           ghosts);
            break;
        case VTK_LONG:          
            GetScalarRange((long*) da->GetVoidPointer(0), nvals, exts,
                           ghosts);
            break;
        case VTK_UNSIGNED_LONG: 
            GetScalarRange((unsigned long*) da->GetVoidPointer(0), nvals, exts,
                           ghosts);
            break;
        case VTK_FLOAT:         
            GetScalarRange((float*) da->GetVoidPointer(0), nvals, exts,
                           ghosts);
            break;
        case VTK_DOUBLE:        
            GetScalarRange((double*) da->GetVoidPointer(0), nvals, exts,
                           ghosts);
            break;
        case VTK_ID_TYPE:       
            GetScalarRange((vtkIdType*) da->GetVoidPointer(0), nvals, exts,
                           ghosts);
            break;
    }
}


// ****************************************************************************
//  Function: GetDataScalarRange
//
//  Purpose:
//      Gets the full individual component ranges from a VTK dataset
//
//  Arguments:
//      ds      The dataset to determine the range for.
//      exts    The extents in <min, max> form.  There may be many 3 sets of
//              extents for vector data.
//      vname   The variable name to get the range for.
//      ignoreGhost  A Boolean.  True if we should ignore ghosts, else false.
//
//  Returns:    true if it found real data, false otherwise.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  7, 2008
//
//  Modifications:
//
// ****************************************************************************

template <class T> static bool
GetComponentRange(T *buf, int n, int c, int nc, double *exts, unsigned char *ghosts)
{
    T min; 
    T max;
    bool setOne = false;
    buf += c;
    for (int i = c; i < n*nc; i+=nc, buf+=nc)
    {
        if ((ghosts != NULL) && (ghosts[i] != '\0'))
            continue;

        if (!setOne)
        {
            min = *buf;
            max = *buf;
            setOne = true;
            continue;
        }

        if (*buf < min)
        {
            min = *buf;
        }
        else
        {
            if (*buf > max)
                max = *buf;
        }
    }
    exts[0] = (double) min;
    exts[1] = (double) max;

    return setOne;
}

void
GetDataAllComponentsRange(vtkDataSet *ds, double *exts, const char *vname,
                          bool ignoreGhost)
{
    vtkDataArray *da = NULL;
    unsigned char *ghosts = NULL;
    if (ds->GetPointData()->GetArray(vname))
    {
        da = ds->GetPointData()->GetArray(vname);
    }
    else
    {
        da = ds->GetCellData()->GetArray(vname);
        if (ignoreGhost)
        {
            vtkUnsignedCharArray *ga = (vtkUnsignedCharArray *)
                                  ds->GetCellData()->GetArray("avtGhostZones");
            if (ga != NULL)
                ghosts = ga->GetPointer(0);
        }
    }

    if (da == NULL)
        return;

    int ntuples = da->GetNumberOfTuples();
    int ncomps = da->GetNumberOfComponents();

    for (int comp=0; comp<ncomps; comp++)
    {
        double *compexts = &(exts[2*comp]);
        compexts[0] = +FLT_MAX;
        compexts[1] = -FLT_MAX;
    
        switch (da->GetDataType())
        {
          case VTK_CHAR:
            GetComponentRange((char*) da->GetVoidPointer(0), ntuples,
                              comp, ncomps, compexts, ghosts);
            break;
          case VTK_UNSIGNED_CHAR:
            GetComponentRange((unsigned char*) da->GetVoidPointer(0),
                              ntuples, comp, ncomps, compexts, ghosts);
            break;
          case VTK_SHORT:
            GetComponentRange((short*) da->GetVoidPointer(0),
                              ntuples, comp, ncomps, compexts, ghosts);
            break;
          case VTK_UNSIGNED_SHORT:
            GetComponentRange((unsigned short*) da->GetVoidPointer(0),
                              ntuples, comp, ncomps,compexts, ghosts);
            break;
          case VTK_INT:           
            GetComponentRange((int*) da->GetVoidPointer(0),
                              ntuples, comp, ncomps, compexts, ghosts);
            break;
          case VTK_UNSIGNED_INT:  
            GetComponentRange((unsigned int*) da->GetVoidPointer(0),
                              ntuples, comp, ncomps, compexts, ghosts);
            break;
          case VTK_LONG:          
            GetComponentRange((long*) da->GetVoidPointer(0),
                              ntuples, comp, ncomps, compexts, ghosts);
            break;
          case VTK_UNSIGNED_LONG: 
            GetComponentRange((unsigned long*) da->GetVoidPointer(0),
                              ntuples, comp, ncomps, compexts, ghosts);
            break;
          case VTK_FLOAT:         
            GetComponentRange((float*) da->GetVoidPointer(0),
                              ntuples, comp, ncomps, compexts, ghosts);
            break;
          case VTK_DOUBLE:        
            GetComponentRange((double*) da->GetVoidPointer(0),
                              ntuples, comp, ncomps, compexts, ghosts);
            break;
          case VTK_ID_TYPE:       
            GetComponentRange((vtkIdType*) da->GetVoidPointer(0),
                              ntuples, comp, ncomps, compexts, ghosts);
            break;
        }
    }
}


// ****************************************************************************
//  Function: GetDataMagnitudeRange
//
//  Purpose:
//      Gets the data magnitude range from a VTK dataset.
//
//  Arguments:
//      ds      The dataset to determine the range for.
//      exts    The extents in <min, max> form.
//      vname   The variable name to get the range for.
//      ignoreGhost  A Boolean.  True if we should ignore ghosts, else false.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Dec 4 11:56:08 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Tue Sep 23 23:09:02 PDT 2003
//    Add support for tensors.
//
//    Hank Childs, Tue Feb 24 14:54:28 PST 2004
//    Added a variable to get the range for.  Reduced number of sqrt calls.
//
//    Mark C. Miller, Tue Dec  5 18:14:58 PST 2006
//    Templatized it to support all array types.
//
//    Hank Childs, Wed Oct 10 15:56:16 PDT 2007
//    Added argument for ignoring values from ghost zones.
//
//    Gunther H. Weber, Fri Feb  1 11:55:59 PST 2008
//    Skip nan, -inf and +inf in min/max calculation
//
//    Cyrus Harrison, Mon Feb 25 11:03:03 PST 2008
//    Changed std::isfinite to isfinite to work around an AIX compiler bug.
//
//    Kathleen Bonnell, Thu Mar  6 09:15:46 PST 2008 
//    Use _finite on Windows. 
//
//    Eric Brugger, Tue Apr  8 10:01:07 PDT 2008
//    Make the use of isfinite conditional, since not all platforms support
//    it (IRIX64 6.5 with MIPSpro 7.41, solaris with gcc 3.2).
//
// ****************************************************************************

template <class T> static void
GetMagnitudeRange(T *buf, int n, int ncomps, double *exts, 
                  unsigned char *ghosts)
{
    for (int i = 0; i < n; i++)
    {
        if ((ghosts != NULL) && (ghosts[i] != '\0'))
            continue;

        double mag = 0.0;
        for (int j = 0; j < ncomps; j++, buf++)
            mag += *buf * *buf;

#ifndef _WIN32
#ifdef HAVE_ISFINITE
        if (!isfinite(mag))
            continue;
#endif
#else
        if (!_finite(mag))
            continue;
#endif

        if (mag < exts[0])
        {
            exts[0] = mag;
        }
        else
        {
            if (mag > exts[1])
                exts[1] = mag;
        }
    }
    exts[0] = sqrt(exts[0]);
    exts[1] = sqrt(exts[1]);
}

void
GetDataMagnitudeRange(vtkDataSet *ds, double *exts, const char *vname,
                      bool ignoreGhost)
{
    exts[0] = +FLT_MAX;
    exts[1] = 0;

    vtkDataArray *da = NULL;
    unsigned char *ghosts = NULL;
    if (ds->GetPointData()->GetArray(vname))
    {
        da = ds->GetPointData()->GetArray(vname);
    }
    else
    {
        da = ds->GetCellData()->GetArray(vname);
        if (ignoreGhost)
        {
            vtkUnsignedCharArray *ga = (vtkUnsignedCharArray *)
                                  ds->GetCellData()->GetArray("avtGhostZones");
            if (ga != NULL)
                ghosts = ga->GetPointer(0);
        }
    }

    if (da == NULL)
        return;

    int nvals = da->GetNumberOfTuples();
    int ncomps = da->GetNumberOfComponents();

    switch (da->GetDataType())
    {
        case VTK_CHAR:
            GetMagnitudeRange((char*) da->GetVoidPointer(0), nvals, 
                              ncomps, exts, ghosts);
            break;
        case VTK_UNSIGNED_CHAR:
            GetMagnitudeRange((unsigned char*) da->GetVoidPointer(0), nvals, 
                              ncomps, exts, ghosts);
            break;
        case VTK_SHORT:
            GetMagnitudeRange((short*) da->GetVoidPointer(0), nvals, 
                              ncomps, exts, ghosts);
            break;
        case VTK_UNSIGNED_SHORT:
            GetMagnitudeRange((unsigned short*) da->GetVoidPointer(0), nvals, 
                              ncomps, exts, ghosts);
            break;
        case VTK_INT:           
            GetMagnitudeRange((int*) da->GetVoidPointer(0), nvals, 
                              ncomps, exts, ghosts);
            break;
        case VTK_UNSIGNED_INT:  
            GetMagnitudeRange((unsigned int*) da->GetVoidPointer(0), nvals, 
                              ncomps, exts, ghosts);
            break;
        case VTK_LONG:          
            GetMagnitudeRange((long*) da->GetVoidPointer(0), nvals, 
                              ncomps, exts, ghosts);
            break;
        case VTK_UNSIGNED_LONG: 
            GetMagnitudeRange((unsigned long*) da->GetVoidPointer(0), nvals,
                              ncomps, exts, ghosts);
            break;
        case VTK_FLOAT:         
            GetMagnitudeRange((float*) da->GetVoidPointer(0), nvals,
                              ncomps, exts, ghosts);
            break;
        case VTK_DOUBLE:        
            GetMagnitudeRange((double*) da->GetVoidPointer(0), nvals,
                              ncomps, exts, ghosts);
            break;
        case VTK_ID_TYPE:       
            GetMagnitudeRange((vtkIdType*) da->GetVoidPointer(0), nvals,
                              ncomps, exts, ghosts);
            break;
    }
}


// ****************************************************************************
//  Function: GetDataMajorEigenvalueRange
//
//  Purpose:
//      Gets the data range from a VTK dataset. It computes MajorEigenvalue.
//
//  Arguments:
//      ds      The dataset to determine the range for.
//      exts    The extents in <min, max> form.
//      vname   The variable name to get the range for.
//      ignoreGhost  A Boolean.  True if we should ignore ghosts, else false.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 11, 2004 
//
//  Modifications:
//
//    Hank Childs, Wed Oct 10 15:56:16 PDT 2007
//    Added argument for ignoring values from ghost zones.
//
//    Gunther H. Weber, Fri Feb  1 11:55:59 PST 2008
//    Skip nan, -inf and +inf in min/max calculation
//
//    Cyrus Harrison, Mon Feb 25 11:03:03 PST 2008
//    Changed std::isfinite to isfinite to work around an AIX compiler bug.
//
//    Kathleen Bonnell, Thu Mar  6 09:15:46 PST 2008 
//    Use _finite on Windows. 
//
//    Eric Brugger, Tue Apr  8 10:01:07 PDT 2008
//    Make the use of isfinite conditional, since not all platforms support
//    it (IRIX64 6.5 with MIPSpro 7.41, solaris with gcc 3.2).
//
// ****************************************************************************

void
GetDataMajorEigenvalueRange(vtkDataSet *ds, double *exts, const char *vname,
                            bool ignoreGhost)
{
    vtkDataArray *da = NULL;
    unsigned char *ghosts = NULL;
    if (ds->GetPointData()->GetArray(vname))
    {
        da = ds->GetPointData()->GetArray(vname);
    }
    else
    {
        da = ds->GetCellData()->GetArray(vname);
        if (ignoreGhost)
        {
            vtkUnsignedCharArray *ga = (vtkUnsignedCharArray *)
                                  ds->GetCellData()->GetArray("avtGhostZones");
            if (ga != NULL)
                ghosts = ga->GetPointer(0);
        }
    }

    if (da == NULL)
    {
        return;
    }

    int nvals = da->GetNumberOfTuples();
    int ncomps = da->GetNumberOfComponents();

    if (ncomps != 9)
    {
        return;
    }
    //
    // We only know how to deal with floats.
    //
    if (da->GetDataType() != VTK_FLOAT)
    {
        return;
    }

    float *ptr = (float *) da->GetVoidPointer(0);
    for (int i = 0 ; i < nvals ; i++)
    {
        if ((ghosts != NULL) && (ghosts[i] != '\0'))
            continue;

        double val = MajorEigenvalue(ptr);

#ifndef _WIN32
#ifdef HAVE_ISFINITE
        if (!isfinite(val))
            continue;
#endif
#else
        if (!_finite(val))
            continue;
#endif

        exts[0] = (exts[0] < val ? exts[0] : val);
        exts[1] = (exts[1] > val ? exts[1] : val);
        ptr+=ncomps;
    }
}


// ****************************************************************************
//  Function: CFindMaximum
//
//  Purpose:
//      Determines the maximum value for the domain.
//
//  Arguments:
//    data      The data whose variables should be set.
//    arg       A struct with information how to determine the maximum.
//    success   Assigned true if operation successful, false otherwise. 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Mon Mar 18 13:11:51 PST 2002 
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Kathleen Bonnell, Wed Oct 20 17:06:12 PDT 2004 
//    Replaced get-cell-center code with single call to 
//    vtkVisItUtility::GetCellCenter.
//
// ****************************************************************************

void
CFindMaximum(avtDataRepresentation &data, void *arg, bool &success)
{
    FindExtremeArgs *args = (FindExtremeArgs *) arg;

    //
    // Perform some checks of the input.
    //
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    vtkDataSet *ds = data.GetDataVTK();
    if (ds == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    vtkPointData *pd = ds->GetPointData();
    vtkCellData  *cd = ds->GetCellData();

    vtkDataArray *s = NULL;
    bool workingOnPoints = false;
    if (pd->GetScalars() != NULL)
    {
        s = pd->GetScalars();
        workingOnPoints = true;
    }
    else if (cd->GetScalars() != NULL)
    {
        s = cd->GetScalars();
        workingOnPoints = false;
    }

    if (s == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    int nS = s->GetNumberOfTuples();
    float localMax = -FLT_MAX;
    int ind = -1;
    for (int i = 0 ; i < nS ; i++)
    {
        float v = s->GetTuple1(i);
        if (v > localMax)
        {
            localMax = v;
            ind = i;
        }
    }
    if (ind != -1)
    {
        if (localMax > args->value)
        {
            if (workingOnPoints)
            {
                double p[3];
                ds->GetPoint(ind, p);
                args->point[0] = p[0];
                args->point[1] = p[1];
                args->point[2] = p[2];
            }
            else
            {
                double point[3];
                vtkVisItUtility::GetCellCenter(ds->GetCell(ind), point);
                args->point[0] = point[0];
                args->point[1] = point[1];
                args->point[2] = point[2];
            }
            args->value = localMax;
        }
    }
    success = true;
}


// ****************************************************************************
//  Function: CFindMinimum
//
//  Purpose:
//      Determines the minimum value for the domain.
//
//  Arguments:
//    data      The data whose variables should be set.
//    arg       A struct with information how to determine the minimum.
//    success   Assigned true if operation successful, false otherwise. 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Mon Mar 18 13:11:51 PST 2002 
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Kathleen Bonnell, Wed Oct 20 17:06:12 PDT 2004 
//    Replaced get-cell-center code with single call to 
//    vtkVisItUtility::GetCellCenter.
//
// ****************************************************************************

void
CFindMinimum(avtDataRepresentation &data, void *arg, bool &success)
{
    FindExtremeArgs *args = (FindExtremeArgs *) arg;

    //
    // Perform some checks of the input.
    //
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    vtkDataSet *ds = data.GetDataVTK();
    if (ds == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    vtkPointData *pd = ds->GetPointData();
    vtkCellData  *cd = ds->GetCellData();

    vtkDataArray *s = NULL;
    bool workingOnPoints = false;
    if (pd->GetScalars() != NULL)
    {
        s = pd->GetScalars();
        workingOnPoints = true;
    }
    else if (cd->GetScalars() != NULL)
    {
        s = cd->GetScalars();
        workingOnPoints = false;
    }

    if (s == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    int nS = s->GetNumberOfTuples();
    float localMin = FLT_MAX;
    int ind = -1;
    for (int i = 0 ; i < nS ; i++)
    {
        float v = s->GetTuple1(i);
        if (v < localMin)
        {
            localMin = v;
            ind = i;
        }
    }
    if (ind != -1)
    {
        if (localMin < args->value)
        {
            if (workingOnPoints)
            {
                double p[3];
                ds->GetPoint(ind, p);
                args->point[0] = p[0];
                args->point[1] = p[1];
                args->point[2] = p[2];
            }
            else
            {
                double point[3];
                vtkVisItUtility::GetCellCenter(ds->GetCell(ind), point);
                args->point[0] = point[0];
                args->point[1] = point[1];
                args->point[2] = point[2];
            }
            args->value = localMin;
        }
    }
    success = true;
}


// ****************************************************************************
//  Function: CLocateZone
//
//  Purpose:
//      Determines the location in world space of a zone.
//
//  Arguments:
//    data      The data whose variables should be set.
//    arg       A struct with information about which zone to locate.
//    success   Assigned true if operation successful, false otherwise. 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Jun 18 10:55:48 PDT 2003
//    Make use of original zones array if available.
//
//    Kathleen Bonnell, Wed Oct 20 17:06:12 PDT 2004 
//    Replaced get-cell-center code with single call to 
//    vtkVisItUtility::GetCellCenter.
//
// ****************************************************************************

void
CLocateZone(avtDataRepresentation &data, void *arg, bool &success)
{
    LocateObjectArgs *args = (LocateObjectArgs *) arg;

    //
    // Perform some checks of the input.
    //
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    if (data.GetDomain() != args->domain)
    {
        return;
    }
    vtkDataSet *ds = data.GetDataVTK();
    if (ds == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    //
    // It might be that the original node numbers are saved off.  If so, they
    // will be the most accurate.  If not, assume that the nodes are ordered
    // in the same way they were when the data was saved out.
    //
    int indexToUse = -1;
    vtkDataArray *ocn = ds->GetCellData()->GetArray("avtOriginalCellNumbers");
    if (ocn == NULL)
    {
        indexToUse = args->index;
    }
    else
    {
        vtkUnsignedIntArray *origZone = (vtkUnsignedIntArray *) ocn;

        //
        // There are two components when we have saved out the domain number
        // as well.  The domain number is needed in cases where we have created
        // ghost zones, for example.
        //
        if (origZone->GetNumberOfComponents() == 2)
        {
            //
            // If the zone numbering has not changed, then the zone we want
            // will still be in the same location.  Check to see if this is
            // the case.  If so, it will allow us to not have to iterate over
            // the whole array.
            //
            int nvals = origZone->GetNumberOfTuples();
            int domain = -1;
            int index  = -1;
            if (args->index < nvals)
            {
                unsigned int *p = origZone->GetPointer(0);
                p += 2*args->index;
                domain = p[0];
                index  = p[1];
            }
            if (domain == args->domain && index == args->index)
            {
                indexToUse = args->index;
            }
            else
            {
                //
                // No avoiding it -- look at every zone and try to find a
                // match.
                //
                unsigned int *ptr = origZone->GetPointer(0);
                for (int i = 0 ; i < nvals ; i++)
                {
                    int domain = *(ptr++);
                    int index  = *(ptr++);
                    if (domain == args->domain && index == args->index)
                    {
                        indexToUse = i;
                        break;
                    }
                }
            }
        }
        else
        {
            //
            // The domain numbers are not included.  This is very similar to
            // the logic above -- so see above for pertinent comments.
            //
            int nvals = origZone->GetNumberOfTuples();
            int index  = -1;
            if (args->index < nvals)
            {
                unsigned int *p = origZone->GetPointer(args->index);
                index  = p[0];
            }
            if (index == args->index)
            {
                indexToUse = args->index;
            }
            else
            {
                unsigned int *ptr = origZone->GetPointer(0);
                for (int i = 0 ; i < nvals ; i++)
                {
                    int index  = *(ptr++);
                    if (index == args->index)
                    {
                        indexToUse = i;
                        break;
                    }
                }
            }
        }
    }

    if (indexToUse == -1)
        return;

    if (indexToUse >= ds->GetNumberOfCells())
        return;

    vtkCell *cell = ds->GetCell(indexToUse);

    if (cell == NULL)
        return;

    double point[3];
    vtkVisItUtility::GetCellCenter(cell, point);
    args->point[0] = point[0];
    args->point[1] = point[1];
    args->point[2] = point[2];

    success = true;
}


// ****************************************************************************
//  Function: CLocateNode
//
//  Purpose:
//      Determines the location in world space of a node.
//
//  Arguments:
//    data      The data whose variables should be set.
//    arg       A struct with information about which node to locate.
//    success   Assigned true if operation successful, false otherwise. 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Jun 18 10:55:48 PDT 2003
//    Make use of original nodes array if available.
//
//    Kathleen Bonnell, Wed Jun 16 13:54:28 PDT 2004 
//    avtOriginalNodeNumbers is now of type vtkIntArray. 
//
// ****************************************************************************

void
CLocateNode(avtDataRepresentation &data, void *arg, bool &success)
{
    LocateObjectArgs *args = (LocateObjectArgs *) arg;

    //
    // Perform some checks of the input.
    //
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    if (data.GetDomain() != args->domain)
    {
        return;
    }
    vtkDataSet *ds = data.GetDataVTK();
    if (ds == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    //
    // It might be that the original node numbers are saved off.  If so, they
    // will be the most accurate.  If not, assume that the nodes are ordered
    // in the same way they were when the data was saved out.
    //
    int indexToUse = -1;
    vtkDataArray *onn = ds->GetPointData()->GetArray("avtOriginalNodeNumbers");
    if (onn == NULL)
    {
        indexToUse = args->index;
    }
    else
    {
        vtkIntArray *origNode = (vtkIntArray *) onn;

        //
        // There are two components when we have saved out the domain number
        // as well.  The domain number is needed in cases where we have created
        // ghost zones, for example.
        //
        if (origNode->GetNumberOfComponents() == 2)
        {
            //
            // If the node numbering has not changed, then the node we want
            // will still be in the same location.  Check to see if this is
            // the case.  If so, it will allow us to not have to iterate over
            // the whole array.
            //
            int nvals = origNode->GetNumberOfTuples();
            int domain = -1;
            int index  = -1;
            if (args->index < nvals)
            {
                int *p = origNode->GetPointer(0);
                p += 2*args->index;
                index  = p[1];
            }
            if (domain == args->domain && index == args->index)
            {
                indexToUse = args->index;
            }
            else
            {
                //
                // No avoiding it -- look at every node and try to find a
                // match.
                //
                int *ptr = origNode->GetPointer(0);
                for (int i = 0 ; i < nvals ; i++)
                {
                    int domain = *(ptr++);
                    int index  = *(ptr++);
                    if (domain == args->domain && index == args->index)
                    {
                        indexToUse = i;
                        break;
                    }
                }
            }
        }
        else
        {
            //
            // The domain numbers are not included.  This is very similar to
            // the logic above -- so see above for pertinent comments.
            //
            int nvals = origNode->GetNumberOfTuples();
            int index  = -1;
            if (args->index < nvals)
            {
                int *p = origNode->GetPointer(args->index);
                index  = p[0];
            }
            if (index == args->index)
            {
                indexToUse = args->index;
            }
            else
            {
                int *ptr = origNode->GetPointer(0);
                for (int i = 0 ; i < nvals ; i++)
                {
                    int index  = *(ptr++);
                    if (index == args->index)
                    {
                        indexToUse = i;
                        break;
                    }
                }
            }
        }
    }

    if (indexToUse == -1)
        return;

    if (ds->GetNumberOfPoints() < indexToUse)
    {
        return;
    }

    double p[3];
    ds->GetPoint(indexToUse, p);
    args->point[0] = p[0];
    args->point[1] = p[1];
    args->point[2] = p[2];

    success = true;
}


// ****************************************************************************
//  Function: CGetArray
//
//  Purpose:
//      Finds an array for a domain.
//
//  Arguments:
//    data      The data whose arrays should potentially be obtained.
//    arg       A struct with information about which array to get.
//    success   Assigned true if operation successful, false otherwise. 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Hank Childs
//  Creation:   July 29, 2003
//
// ****************************************************************************

void
CGetArray(avtDataRepresentation &data, void *arg, bool &success)
{
    GetArrayArgs *args = (GetArrayArgs *) arg;

    if (args->arr != NULL)
        return;

    //
    // Perform some checks of the input.
    //
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    if (data.GetDomain() != args->domain)
    {
        return;
    }
    vtkDataSet *ds = data.GetDataVTK();
    if (ds == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    char *vname = (char *) args->varname;  // no const support.
    if (ds->GetPointData()->GetArray(vname))
    {
        args->arr = ds->GetPointData()->GetArray(vname);
        args->centering = AVT_NODECENT;
    }
    else if (ds->GetCellData()->GetArray(vname))
    {
        args->arr = ds->GetCellData()->GetArray(vname);
        args->centering = AVT_ZONECENT;
    }

    if (args->arr != NULL)
        success = true;
}


// ****************************************************************************
//  Function: CGetVariableCentering
//
//  Purpose:
//      Finds the centering for a variable.
//
//  Arguments:
//    data      The data whose arrays should potentially be obtained.
//    arg       A struct with information about which array to get.
//    success   Assigned true if operation successful, false otherwise. 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2003
//
// ****************************************************************************

void
CGetVariableCentering(avtDataRepresentation &data, void *arg, bool &success)
{
    GetArrayArgs *args = (GetArrayArgs *) arg;

    if (args->centering != AVT_UNKNOWN_CENT)
        return;

    //
    // Perform some checks of the input.
    //
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    vtkDataSet *ds = data.GetDataVTK();
    if (ds == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    char *vname = (char *) args->varname;  // no const support.
    if (ds->GetPointData()->GetArray(vname))
    {
        args->centering = AVT_NODECENT;
        success = true;
    }
    else if (ds->GetCellData()->GetArray(vname))
    {
        args->centering = AVT_ZONECENT;
        success = true;
    }
}


// ****************************************************************************
//  Method: CGetNumberOfNodes
//
//  Purpose:
//    Adds the number of nodes in the vtk input to the passed sum argument. 
//
//  Arguments:
//    data      The data from which to calculate number of nodes.
//    sum       A place to store the cumulative number of nodes.
//    <unused> 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 18, 2004
//
// ****************************************************************************

void
CGetNumberOfNodes(avtDataRepresentation &data, void *sum, bool &)
{
    int *numNodes = (int*)sum;
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    vtkDataSet *ds = data.GetDataVTK();
    *numNodes += ds->GetNumberOfPoints();
}


// ****************************************************************************
//  Method: CGetNumberOfRealZones
//
//  Purpose:
//    Adds the number of zones in the vtk input to the passed sum argument. 
//    Counts 'real' and 'ghost' separately.
//
//  Arguments:
//    data      The data from which to calculate number of zones.
//    sum       A place to store the cumulative number of zones.
//    <unused> 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 18, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Aug 27 15:32:06 PDT 2004
//    Rename ghost data array.
//
// ****************************************************************************

void
CGetNumberOfRealZones(avtDataRepresentation &data, void *sum, bool &)
{
    int *numZones = (int*)sum;
    //
    // realZones  stored in numZones[0]
    // ghostZones stored in numZones[1]
    //
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    vtkDataSet *ds = data.GetDataVTK();
    vtkUnsignedCharArray *ghosts = (vtkUnsignedCharArray*)
        ds->GetCellData()->GetArray("avtGhostZones");

    int nCells = ds->GetNumberOfCells();
    if (ghosts)
    {
        unsigned char *gptr = ghosts->GetPointer(0);
        for (int i = 0; i < nCells; i++)
        {
           if (gptr[i])
               numZones[1]++;
           else 
               numZones[0]++;
        }
    }
    else
    {
        numZones[0] += nCells;
    }
}


// ****************************************************************************
//  Function: MajorEigenvalue
//
//  Purpose:
//      Computes the major Eigenvalue of the passed tensor matrix.
//
//  Notes:  Assumes 9 values in vals.
//
//  Arguments:
//      vals    Then tensor values. 
//
//  Returns:    The major Eigenvalue.

//  Programmer: Kathleen Bonnell 
//  Creation:   March 11, 2004 
//
//  Modifications:
//
// ****************************************************************************

double
MajorEigenvalue(float *vals)
{
    double dv[9];
    for (int i = 0; i < 9; i++)
        dv[i] = vals[i];
    return MajorEigenvalue(dv);
}

double
MajorEigenvalue(double *vals)
{
    double *input[3];
    double row1[3];
    double row2[3];
    double row3[3];
    input[0] = row1;
    input[1] = row2;
    input[2] = row3;
    input[0][0] = vals[0];
    input[0][1] = vals[1];
    input[0][2] = vals[2];
    input[1][0] = vals[3];
    input[1][1] = vals[4];
    input[1][2] = vals[5];
    input[2][0] = vals[6];
    input[2][1] = vals[7];
    input[2][2] = vals[8];
    double *eigenvecs[3];
    double outrow1[3];
    double outrow2[3];
    double outrow3[3];
    eigenvecs[0] = outrow1;
    eigenvecs[1] = outrow2;
    eigenvecs[2] = outrow3;
    double eigenvals[3];
    vtkMath::Jacobi(input, eigenvals, eigenvecs);
    return eigenvals[0];
}

// ****************************************************************************
//  Function: CMaybeCompressedDataString
//
//  Purpose: Check a data string for leading characters indicating it *might*
//           be a BZ2 compressed string. 
//
//  Programmer: Mark C. Miller 
//  Creation:   November 15, 2005 
//
// ****************************************************************************

bool
CMaybeCompressedDataString(const unsigned char *dstr)
{
    if (dstr[0] == 'B' && dstr[1] == 'Z' && dstr[2] == 'h')
        return true;
    return false;
}

// ****************************************************************************
//  Function: CCompressDataString 
//
//  Purpose: Attempts to compress a data string to a size no larger than a
//           specified size. The maximum specified size is read from *newlen
//           on entrance.  If *newlen is zero, then it will default to using
//           a maximum specified size of 1/2 of the input string's size.
//           If it is able to compress into the specified size, the compressed
//           string and its size is returned in the new args and a value of
//           true is returned for the function. If it is unable to compress
//           into the specified size, the new args are unchanged and a value
//           of false is returned for the function.
//
//  Notes:   Information about the compression is tacked onto the end of the
//           returned string.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 15, 2005 
//
//  Modifications:
//
//    Hank Childs, Fri Jun  9 13:21:29 PDT 2006
//    Remove unused variable.
//
// ****************************************************************************
bool CCompressDataString(const unsigned char *dstr, int len,
                         unsigned char **newdstr, int *newlen,
                         float *timec, float *ratioc)
{
#ifdef HAVE_LIBBZ2
    unsigned int lenBZ2 = *newlen == 0 ? len / 2 : *newlen;
    unsigned char *dstrBZ2 = new unsigned char [lenBZ2+20];
    int startCompress = visitTimer->StartTimer(true);
    if (BZ2_bzBuffToBuffCompress((char*)dstrBZ2, &lenBZ2, (char*) dstr, len,
                                 1, 0, 250) != BZ_OK)
    {
        visitTimer->StopTimer(startCompress,
                        "Failed attempt to compress data", true);
        delete [] dstrBZ2;
        return false;
    }
    else
    {
        double timeToCompress = 
            visitTimer->StopTimer(startCompress, "Compressing data", true);
        debug5 << "Compressed data "
               << (float) len / (float) lenBZ2
               << ":1 in " << timeToCompress << " seconds" << endl;
        sprintf((char*) &dstrBZ2[lenBZ2], "%10d", len);
        sprintf((char*) &dstrBZ2[lenBZ2+10], "% 10.6f", timeToCompress);
        *newdstr = dstrBZ2;
        *newlen = lenBZ2+20;
        if (timec) *timec = timeToCompress;
        if (ratioc) *ratioc = (float) len / (float) lenBZ2;
        return true;
    }
#else
    return false;
#endif
}

// ****************************************************************************
// Function: CDecompressDataString
//
// Purpose: Decompress a possibly compressed data string. Return true if
//          decompression succeeded, false otherwise
//
//  Programmer: Mark C. Miller 
//  Creation:   November 15, 2005 
//
//  Modifications:
//
//    Hank Childs, Fri Jun  9 13:21:29 PDT 2006
//    Remove unused variable.
//
// ****************************************************************************

bool CDecompressDataString(const unsigned char *dstr, int len,
                           unsigned char **newdstr, int *newlen,
                           float *timec, float *timedc, float *ratioc)
{
#ifdef HAVE_LIBBZ2
    if (CMaybeCompressedDataString(dstr))
    {
        unsigned int strLengthOrig;
        double timeToCompress;
        sscanf((char*) &dstr[len-20], "%10d", &strLengthOrig);
        sscanf((char*) &dstr[len-10], "% 10.6f", &timeToCompress);
        unsigned char *strOrig = new unsigned char[strLengthOrig];
        int startDecompress = visitTimer->StartTimer(true);
        if (BZ2_bzBuffToBuffDecompress((char*) strOrig, &strLengthOrig,
                                       (char*) dstr, len, 0, 0) != BZ_OK)
        {
            visitTimer->StopTimer(startDecompress,
                            "Failed attempt to decompress data", true);
            debug5 << "Found 3 character \"BZh\" header in data string "
                   << "but failed to decompress. Assuming coincidence." << endl;
            delete [] strOrig;
            return false;
        }
        else
        {
            double timeToDecompress =
                visitTimer->StopTimer(startDecompress, "Decompressing data", true);
            debug5 << "Uncompressed data 1:"
                   << (float) strLengthOrig / (float) len 
                   << " in " << timeToDecompress << " seconds" << endl;
            *newdstr = strOrig;
            *newlen = strLengthOrig;
            if (timec) *timec = timeToCompress;
            if (timedc) *timedc = timeToDecompress;
            if (ratioc) *ratioc = (float) strLengthOrig / (float) len;
            return true;
        }
    }
    else
    {
        return false;
    }
#else
    return false;
#endif
}

// ****************************************************************************
//  Function: CGetCompressionInfoFromDataString 
//
//  Purpose: Without actually doing a decompression, obtain compression
//           information from the end of the datastring
//
//  Programmer: Mark C. Miller 
//  Creation:   November 15, 2005 
//
// ****************************************************************************

void
CGetCompressionInfoFromDataString(const unsigned char *dstr,
                      int len, float *timec, float *ratioc)
{
    if (CMaybeCompressedDataString(dstr))
    {
        int uncompressedLen;
        float timeToCompress;
        sscanf((char*) &dstr[len-20], "%10d", &uncompressedLen);
        sscanf((char*) &dstr[len-10], "% 10.6f", &timeToCompress);
        if (timec) *timec = timeToCompress;
        if (ratioc) *ratioc = (float) uncompressedLen / (float) len;
    }
}




// ****************************************************************************
//  Function:  CApplyTransformToRectGrid
//
//  Purpose:
//    Transform a rectilinear grid into a curvilinear one.
//
//  Arguments:
//    data       the data for which rect grids should be transformed
//    xform      the transform
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************
void
CApplyTransformToRectGrid(avtDataRepresentation &data,
                          void *xform_, bool &)
{
    if (!data.Valid())
    {
        return;
    }

    vtkDataSet *ds = data.GetDataVTK();
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) ds;
        double *xform = (double*)xform_;

        vtkMatrix4x4 *t = vtkMatrix4x4::New();
        t->DeepCopy(xform);

        // The below code taken almost verbatim from
        //  avtTransform::TransformRectilinearToCurvilinear
        vtkMatrixToLinearTransform *trans = vtkMatrixToLinearTransform::New();
        trans->SetInput(t);

        int  dims[3];
        rgrid->GetDimensions(dims);

        int  numPts = dims[0]*dims[1]*dims[2];

        vtkPoints *pts = vtkPoints::New();
        pts->SetNumberOfPoints(numPts);

        vtkDataArray *x = rgrid->GetXCoordinates();
        vtkDataArray *y = rgrid->GetYCoordinates();
        vtkDataArray *z = rgrid->GetZCoordinates();

        int index = 0;
        for (int k = 0 ; k < dims[2] ; k++)
        {
            for (int j = 0 ; j < dims[1] ; j++)
            {
                for (int i = 0 ; i < dims[0] ; i++)
                {
                    float inpoint[4];
                    inpoint[0] = x->GetComponent(i,0);
                    inpoint[1] = y->GetComponent(j,0);
                    inpoint[2] = z->GetComponent(k,0);
                    inpoint[3] = 1.;

                    float outpoint[4];
                    t->MultiplyPoint(inpoint, outpoint);

                    outpoint[0] /= outpoint[3];
                    outpoint[1] /= outpoint[3];
                    outpoint[2] /= outpoint[3];

                    pts->SetPoint(index++, outpoint);
                }
            }
        }

        vtkStructuredGrid *out = vtkStructuredGrid::New();
        out->SetDimensions(dims);
        out->SetPoints(pts);
        pts->Delete();
        out->GetCellData()->ShallowCopy(rgrid->GetCellData());
        out->GetPointData()->ShallowCopy(rgrid->GetPointData());

        // NOTE: We do not transform vector data here.
        //       This is different from the behavior of the 
        //       avtTransform code, because vectors will have been
        //       created (e.g. by the database) in the transformed
        //       space already.  The normals filter does not create
        //       normals for rectilinear grids, but just in case,
        //       we'll transform them here anyway because they
        //       will have been created in the normalized space.
        vtkDataArray *normals = rgrid->GetPointData()->GetNormals();
        if (normals)
        {
            vtkDataArray *arr = normals->NewInstance();
            arr->SetNumberOfComponents(3);
            arr->Allocate(3*normals->GetNumberOfTuples());
            trans->TransformNormals(normals, arr);
            arr->SetName(normals->GetName());
            out->GetPointData()->RemoveArray(normals->GetName());
            out->GetPointData()->SetNormals(arr);
            arr->Delete();
        }
        normals = rgrid->GetCellData()->GetNormals();
        if (normals)
        {
            vtkDataArray *arr = normals->NewInstance();
            arr->SetNumberOfComponents(3);
            arr->Allocate(3*normals->GetNumberOfTuples());
            trans->TransformNormals(normals, arr);
            arr->SetName(normals->GetName());
            out->GetCellData()->RemoveArray(normals->GetName());
            out->GetCellData()->SetNormals(arr);
            arr->Delete();
        }
        // ... end code from avtTransform::TransformRectilinearToCurvilinear

        // and set the output data set
        avtDataRepresentation new_data(out, data.GetDomain(),
                                       data.GetLabel());
        data = new_data;
        out->Delete();
    }
}

// ****************************************************************************
//  Function:  CInsertRectilinearTransformInfoIntoDataset
//
//  Purpose:
//    This is used when we break out of the AVT pipeline and have
//    no "good" way to carry along a rectilinear transform, so we
//    need to insert it as field data.
//
//  Arguments:
//    data       the data for which rect grids should be transformed
//    xform      the transform
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************

void
CInsertRectilinearTransformInfoIntoDataset(avtDataRepresentation &data,
                                           void *xform_, bool &)
{
    if (!data.Valid())
    {
        return; // This is a problem, but no need to flag it for this...
    }

    vtkDataSet *ds = data.GetDataVTK();
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) ds;
        double *xform = (double*)xform_;
        vtkDoubleArray *xformarray = vtkDoubleArray::New();
        xformarray->SetName("RectilinearGridTransform");
        xformarray->SetNumberOfTuples(16);
        for (int i=0; i<16; i++)
            xformarray->SetComponent(i, 0, xform[i]);
        rgrid->GetFieldData()->AddArray(xformarray);
    }
}
