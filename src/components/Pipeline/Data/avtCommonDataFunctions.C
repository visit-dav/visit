// ************************************************************************* //
//                        avtCommonDataFunctions.C                           //
// ************************************************************************* //

#include <avtCommonDataFunctions.h>

#include <float.h>
#include <vector>

#include <vtkAppendFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkUnsignedIntArray.h>

#include <avtCallback.h>
#include <avtDataTree.h>

#include <NoInputException.h>
#include <DebugStream.h>

using std::vector;
using std::string;


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
// ****************************************************************************

void 
CGetSpatialExtents(avtDataRepresentation &data, void *se, bool &success)
{
    if (data.Valid())
    {
        vtkDataSet *ds = data.GetDataVTK();
        if (ds->GetNumberOfCells() > 0 && ds->GetNumberOfPoints() > 0)
        {
            float bounds[6];
            ds->GetBounds(bounds);
            double *fse = (double*)se;

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
                        fse[2*j] = (double) bounds[2*j];
                    }
                    if (bounds[2*j+1] > fse[2*j+1])
                    {
                        fse[2*j+1] = (double) bounds[2*j+1];
                    }
                }
            }
            else
            {
                for (int j = 0 ; j < 6 ; j++)
                {
                    fse[j] = (double) bounds[j];
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
// ****************************************************************************

void 
CGetDataExtents(avtDataRepresentation &data, void *de, bool &success) 
{
    if (data.Valid())
    {
        vtkDataSet *ds = data.GetDataVTK();
        if (ds->GetNumberOfCells() > 0 && ds->GetNumberOfPoints() > 0)
        {
            double *dde = (double*)de;
            int dim;
            if (ds->GetPointData()->GetScalars() != NULL 
                || ds->GetCellData()->GetScalars() != NULL)
            {
                dim = 1; // scalars
            }
            else if (ds->GetPointData()->GetVectors() != NULL 
                || ds->GetCellData()->GetVectors() != NULL)
            {
                dim = 3; // vectors
            }
            else
            {
                debug1 << "Asked to find extents of dataset with no variables."
                       << endl;
                return;
            }
            double *range = new double[2*dim];
            for (int i = 0; i < dim; i++)
            {
                range[2*i]   = +DBL_MAX;
                range[2*i+1] = -DBL_MAX;
            }
            GetDataRange(ds, range, dim);

            //
            // If we have gotten extents from another data rep, then merge the
            // extents.  Otherwise copy them over.
            //
            if (success)
            {
                for (int i = 0; i < dim; i++)
                {
                    if (range[2*i] < dde[2*i])
                    {
                        dde[2*i] = range[2*i];
                    }
                    if (range[2*i+1] > dde[2*i+1])
                    {
                        dde[2*i+1] = range[2*i+1];
                    }
                }
            }
            else
            {
                for (int i = 0 ; i < 2*dim ; i++)
                {
                    dde[i] = range[i];
                }
            }
            success = true;
            delete [] range;
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
//  Method: CGetDataMagnitudeExtents
//
//  Purpose:
//      Gets the data extents of the magnitude of the dataset.
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
//  Programmer:   Brad Whitlock
//  Creation:     Wed Dec 4 11:49:54 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Tue Sep 23 23:09:02 PDT 2003
//    Add support for tensors.
//
// ****************************************************************************

void 
CGetDataMagnitudeExtents(avtDataRepresentation &data, void *de, bool &success) 
{
    if (data.Valid())
    {
        vtkDataSet *ds = data.GetDataVTK();
        if (ds->GetNumberOfCells() > 0 || ds->GetNumberOfPoints() > 0)
        {
            double *dde = (double*)de;
            int dim;
            if (ds->GetPointData()->GetScalars() != NULL 
                || ds->GetCellData()->GetScalars() != NULL)
            {
                dim = 1; // scalars
            }
            else if (ds->GetPointData()->GetVectors() != NULL 
                || ds->GetCellData()->GetVectors() != NULL)
            {
                dim = 3; // vectors
            }
            else if (ds->GetPointData()->GetTensors() != NULL
                || ds->GetCellData()->GetTensors() != NULL)
            {
                dim = 9; // tensors
            }
            else
            {
                debug1 << "Asked to find extents of dataset with no variables."
                       << endl;
                return;
            }
            double range[2];
            range[0] = +DBL_MAX;
            range[1] = -DBL_MAX;
            GetDataMagnitudeRange(ds, range, dim);

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
        debug1 << "Attempting to retrieve Data Magnitude Extents "
               << "of non-existent data." << endl;
        success = false;
    }
}

// ****************************************************************************
//  Method: CGetNodeCenteredDataExtents
//
//  Purpose:
//      Gets the node-centered data extents of the dataset.
//
//  Arguments:
//    data        The data represention from which to get the extents.
//    de          A place to put the data extents
//    success     Assigned true if operation successful, false otherwise. 
//
//  Notes:
//      This method is designed to be used as the function parameter of
//      avtDataTree::Iterate.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     April 17, 2001 
//
//  Modifications:
//
//    Hank Childs, Fri Sep  7 17:15:03 PDT 2001
//    Do not assume that extents are scalars or floats.
//
//    Kathleen Bonnell, Mon Oct  8 12:45:31 PDT 2001 
//    Do not overwrite data extents, merge them instead.  Since
//    this method uses GetDataRange, use that method's criteria
//    for determing the number of <min,max> tuples that will be retrieved. 
//
//    Hank Childs, Tue Nov  6 14:41:52 PST 2001
//    Make sure that there are no UMRs.
//
// ****************************************************************************

void 
CGetNodeCenteredDataExtents(avtDataRepresentation &data, void *de, 
                            bool &success)
{
    success = false;
    if (data.Valid())
    {
        double *dde = (double*)de;
        int dim;
        vtkDataSet *ds = data.GetDataVTK();
        if (ds->GetPointData()->GetScalars() != NULL)
        {
            dim = 1;
        }
        else 
        {
            dim = 3;
        }
        double *range = new double[2*dim];
        for (int i = 0; i < dim; i++)
        {
            range[2*i]   = +DBL_MAX;
            range[2*i+1] = -DBL_MAX;
        }
        GetPointDataRange(ds, range);

        //
        // If we have gotten extents from another data rep, then merge the
        // extents.  Otherwise copy them over.
        //
        if (success)
        {
            for (int i = 0; i < dim; i++)
            {
                if (range[2*i] < dde[2*i])
                {
                    dde[2*i] = range[2*i];
                }
                if (range[2*i+1] > dde[2*i+1])
                {
                    dde[2*i+1] = range[2*i+1];
                }
            }
        }
        else
        {
            for (int i = 0 ; i < 2*dim ; i++)
            {
                dde[i] = range[i];
            }
        }
        success = true;
        delete [] range;
    }
    else
    {
        debug1 << "Attempting to retrieve Nodal Data Extents "
               << "of non-existent data." << endl;
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
    }
    for (i = 0 ; i < ds->GetPointData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *dat = ds->GetPointData()->GetArray(i);
        const char *name = dat->GetName();
        vl->varnames.push_back(string(name));
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

    ds->SetSource(NULL);

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
//  Method: CSetMapperInput
//
//  Purpose:
//    Sets the input to vtkMappers.   
//
//  Arguments:
//    data      The data to use as input to the mappers.
//    arg       Mappers and other necessary info.
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
CSetMapperInput(avtDataRepresentation & data, void *arg, bool &)
{
    if (!data.Valid())
    {
        EXCEPTION0(NoInputException);
    }

    struct map
    {
        int index;
        int nleaves;
        int nmodes;
        vtkDataSetMapper **mappers;
    } *pmap;

    pmap = (struct map *) arg;

    vtkDataSet *ds = data.GetDataVTK();
    for (int i = 0; i < pmap->nmodes; i++)
    {
        pmap->mappers[pmap->index + pmap->nleaves *i]->SetInput(ds);
    }
    pmap->index++;
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
//
//  Programmer: Hank Childs
//  Creation:   September 7, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkVectors has been deprecated in VTK 4.0, use vtkDataArray instead.
//
// ****************************************************************************

void
GetDataRange(vtkDataSet *ds, double *exts, int dim)
{
    if (dim == 0)
    {
        return;
    }

    if (dim == 1)
    {
        float fexts[2];
        ds->GetScalarRange(fexts);
        exts[0] = (double) fexts[0];
        exts[1] = (double) fexts[1];
    }
    else
    {
        for (int i = 0 ; i < 3 ; i++)
        {
            exts[2*i]   = +DBL_MAX;
            exts[2*i+1] = -DBL_MAX;
        }
        
        if (ds->GetPointData()->GetVectors() != NULL)
        {
            vtkDataArray *vec = ds->GetPointData()->GetVectors();
            int nVectors = vec->GetNumberOfTuples();
            double vector[3];
            for (int j = 0 ; j < nVectors ; j++)
            {
                vec->GetTuple(j, vector);
                if (vector[0] < exts[0])
                {
                    exts[0] = vector[0];
                }
                if (vector[0] > exts[1])
                {
                    exts[1] = vector[0];
                }
                if (vector[1] < exts[2])
                {
                    exts[2] = vector[1];
                }
                if (vector[1] > exts[3])
                {
                    exts[3] = vector[1];
                }
                if (vector[2] < exts[4])
                {
                    exts[4] = vector[2];
                }
                if (vector[2] > exts[5])
                {
                    exts[5] = vector[2];
                }
            }
        }
        if (ds->GetCellData()->GetVectors() != NULL)
        {
            vtkDataArray *vec = ds->GetCellData()->GetVectors();
            int nVectors = vec->GetNumberOfTuples();
            double vector[3];
            for (int j = 0 ; j < nVectors ; j++)
            {
                vec->GetTuple(j, vector);
                if (vector[0] < exts[0])
                {
                    exts[0] = vector[0];
                }
                if (vector[0] > exts[1])
                {
                    exts[1] = vector[0];
                }
                if (vector[1] < exts[2])
                {
                    exts[2] = vector[1];
                }
                if (vector[1] > exts[3])
                {
                    exts[3] = vector[1];
                }
                if (vector[2] < exts[4])
                {
                    exts[4] = vector[2];
                }
                if (vector[2] > exts[5])
                {
                    exts[5] = vector[2];
                }
            }
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
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Dec 4 11:56:08 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Tue Sep 23 23:09:02 PDT 2003
//    Add support for tensors.
//
// ****************************************************************************

void
GetDataMagnitudeRange(vtkDataSet *ds, double *exts, int dim)
{
    if (dim == 0)
    {
        return;
    }

    if (dim == 1)
    {
        float fexts[2];
        ds->GetScalarRange(fexts);
        exts[0] = (double) fexts[0];
        exts[1] = (double) fexts[1];
    }
    else if (dim == 3)
    {
        exts[0] = +DBL_MAX;
        exts[1] = -DBL_MAX;

        if (ds->GetPointData()->GetVectors() != NULL)
        {
            vtkDataArray *vec = ds->GetPointData()->GetVectors();
            int nVectors = vec->GetNumberOfTuples();
            double v[3];
            for (int j = 0 ; j < nVectors ; j++)
            {
                vec->GetTuple(j, v);
                double mag = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
                if (mag < exts[0])
                {
                    exts[0] = mag;
                }
                if (mag > exts[1])
                {
                    exts[1] = mag;
                }
            }
        }
        if (ds->GetCellData()->GetVectors() != NULL)
        {
            vtkDataArray *vec = ds->GetCellData()->GetVectors();
            int nVectors = vec->GetNumberOfTuples();
            double v[3];
            for (int j = 0 ; j < nVectors ; j++)
            {
                vec->GetTuple(j, v);
                double mag = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
                if (mag < exts[0])
                {
                    exts[0] = mag;
                }
                if (mag > exts[1])
                {
                    exts[1] = mag;
                }
            }
        }
    }
    else
    {
        exts[0] = +DBL_MAX;
        exts[1] = -DBL_MAX;

        vtkDataArray *tens = NULL;
        if (ds->GetPointData()->GetTensors() != NULL)
            tens = ds->GetPointData()->GetTensors();
        if (ds->GetCellData()->GetTensors() != NULL)
            tens = ds->GetCellData()->GetTensors();

        int ntuples = tens->GetNumberOfTuples();
        for (int i = 0 ; i < ntuples ; i++)
        {
            float in[9];
            tens->GetTuple(i, in);
            float *mat[3];
            float out1[3];
            out1[0] = in[0];
            out1[1] = in[1];
            out1[2] = in[2];
            float out2[3];
            out2[0] = in[3];
            out2[1] = in[4];
            out2[2] = in[5];
            float out3[3];
            out3[0] = in[6];
            out3[1] = in[7];
            out3[2] = in[8];
            mat[0] = out1;
            mat[1] = out2;
            mat[2] = out3;
            float tmp1[3], tmp2[3], tmp3[3];
            float eigenvals[3];
            float *eigenvecs[3];
            eigenvecs[0] = tmp1;
            eigenvecs[1] = tmp2;
            eigenvecs[2] = tmp3;
            vtkMath::Jacobi(mat, eigenvals, eigenvecs);
            exts[0] = (eigenvals[0] < exts[0] ? eigenvals[0] : exts[0]);
            exts[1] = (eigenvals[0] > exts[1] ? eigenvals[0] : exts[1]);
            exts[0] = (eigenvals[1] < exts[0] ? eigenvals[1] : exts[0]);
            exts[1] = (eigenvals[1] > exts[1] ? eigenvals[1] : exts[1]);
            exts[0] = (eigenvals[2] < exts[0] ? eigenvals[2] : exts[0]);
            exts[1] = (eigenvals[2] > exts[1] ? eigenvals[2] : exts[1]);
        }
    }
}

// ****************************************************************************
//  Function: GetPointDataRange
//
//  Purpose:
//      Gets the data range of the point data from a VTK dataset.
//
//  Arguments:
//      ds      The dataset to determine the range for.
//      exts    The extents in <min, max> form.  There may be many 3 sets of
//              extents for vector data.
//
//  Programmer: Hank Childs
//  Creation:   September 7, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkVectors has been deprecated in VTK 4.0, use vtkDataArray instead.
//
// ****************************************************************************

void
GetPointDataRange(vtkDataSet *ds, double *exts)
{
    if (ds->GetPointData()->GetScalars() != NULL)
    {
        float fexts[2];
        ds->GetPointData()->GetScalars()->GetRange(fexts);
        exts[0] = fexts[0];
        exts[1] = fexts[1];
    }
    else
    {
        for (int i = 0 ; i < 3 ; i++)
        {
            exts[2*i]   = +DBL_MAX;
            exts[2*i+1] = -DBL_MAX;
        }
        
        if (ds->GetPointData()->GetVectors() != NULL)
        {
            vtkDataArray *vec = ds->GetPointData()->GetVectors();
            int nVectors = vec->GetNumberOfTuples();
            double vector[3];
            for (int j = 0 ; j < nVectors ; j++)
            {
                vec->GetTuple(j, vector);
                if (vector[0] < exts[0])
                {
                    exts[0] = vector[0];
                }
                if (vector[0] > exts[1])
                {
                    exts[1] = vector[0];
                }
                if (vector[1] < exts[2])
                {
                    exts[2] = vector[1];
                }
                if (vector[1] > exts[3])
                {
                    exts[3] = vector[1];
                }
                if (vector[2] < exts[4])
                {
                    exts[4] = vector[2];
                }
                if (vector[2] > exts[5])
                {
                    exts[5] = vector[2];
                }
            }
        }
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
                float p[3];
                ds->GetPoint(ind, p);
                args->point[0] = p[0];
                args->point[1] = p[1];
                args->point[2] = p[2];
            }
            else
            {
                vtkCell *cell = ds->GetCell(ind);
                float p[3];
                float weights[8] // Assuming no more than 8 vertices in cell.
                              = { 1., 1., 1., 1., 1., 1., 1., 1. };
                int subId = 0; // This is used as an output value.
                cell->GetParametricCenter(p);
                float point[3];
                cell->EvaluateLocation(subId, p, point, weights);
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
                float p[3];
                ds->GetPoint(ind, p);
                args->point[0] = p[0];
                args->point[1] = p[1];
                args->point[2] = p[2];
            }
            else
            {
                vtkCell *cell = ds->GetCell(ind);
                float p[3];
                float weights[8] // Assuming no more than 8 vertices in cell.
                              = { 1., 1., 1., 1., 1., 1., 1., 1. };
                int subId = 0; // This is used as an output value.
                cell->GetParametricCenter(p);
                float point[3];
                cell->EvaluateLocation(subId, p, point, weights);
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

    float p[3];
    cell->GetParametricCenter(p);
    float weights[8] // Assuming no more than 8 vertices in cell.
                  = { 1., 1., 1., 1., 1., 1., 1., 1. };
    int subId = 0; // This is used as an output value.
    float point[3];
    cell->EvaluateLocation(subId, p, point, weights);
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
        vtkUnsignedIntArray *origNode = (vtkUnsignedIntArray *) onn;

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
                unsigned int *p = origNode->GetPointer(0);
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
                unsigned int *ptr = origNode->GetPointer(0);
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
                unsigned int *p = origNode->GetPointer(args->index);
                index  = p[0];
            }
            if (index == args->index)
            {
                indexToUse = args->index;
            }
            else
            {
                unsigned int *ptr = origNode->GetPointer(0);
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

    float p[3];
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


