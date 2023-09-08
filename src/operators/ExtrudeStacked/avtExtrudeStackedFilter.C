// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtExtrudeStackedFilter.C
// ************************************************************************* //

#include <avtExtrudeStackedFilter.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPointSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

#include <avtCallback.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>

#include <BadVectorException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidCellTypeException.h>

// Adapted from the original extrude operator.

// ****************************************************************************
//  Method: avtExtrudeStackedFilter constructor
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

avtExtrudeStackedFilter::avtExtrudeStackedFilter()
{
}


// ****************************************************************************
//  Method: avtExtrudeStackedFilter destructor
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

avtExtrudeStackedFilter::~avtExtrudeStackedFilter()
{
}


// ****************************************************************************
//  Method:  avtExtrudeStackedFilter::Create
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

avtFilter *
avtExtrudeStackedFilter::Create()
{
    return new avtExtrudeStackedFilter();
}


// ****************************************************************************
//  Method:      avtExtrudeStackedFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

void
avtExtrudeStackedFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const ExtrudeStackedAttributes*) a;

    avtVector axis(atts.GetAxis());

    if (axis.x == 0. && axis.y == 0. && axis.z == 0.)
    {
        EXCEPTION1(BadVectorException, "ExtrudeStacked - Zero length extrusion axis.");
    }

    if(atts.GetLength() == 0.)
    {
      if( atts.GetByVariable() ) {
        EXCEPTION1(ImproperUseException, "ExtrudeStacked - The scale cannot be 0.");
      }
      else {
        EXCEPTION1(ImproperUseException, "ExtrudeStacked - The length cannot be 0.");
      }
    }

    if(atts.GetSteps() < 1)
    {
        EXCEPTION1(ImproperUseException, "ExtrudeStacked - The number of steps must be at least 1.");
    }
}


// ****************************************************************************
//  Method: avtExtrudeStackedFilter::ModifyContract
//
//  Purpose:
//      This method makes any necessay modification to the VisIt contract
//      to request that the extrude variable is also loaded if it
//      is not already part of the contract.
//
//  Arguments:
//      in_contract   The current contract
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

avtContract_p
avtExtrudeStackedFilter::ModifyContract(avtContract_p in_contract)
{
    avtDataRequest_p in_dr = in_contract->GetDataRequest();
    std::string var = in_dr->GetOriginalVariable();

    // Create the output contract.
    avtContract_p out_contract = nullptr;

    // If extruding by variable(s) add the variable(s) to the contract
    // if necessay.
    if( atts.GetByVariable() && atts.GetScalarVariableNames().size() )
    {
      if (!atts.AttributesAreConsistent())
      {
        debug3 << "ExtrudeStacked operator attributes are inconsistent." << endl;
        return in_contract;
      }

      const char *inPipelineVar = in_contract->GetDataRequest()->GetVariable();
      std::string outPipelineVar(inPipelineVar);

      stringVector curVariableVarNames = atts.GetScalarVariableNames();
      stringVector needSecondaryVars;

      for (size_t i=0; i<curVariableVarNames.size(); ++i)
      {
          if (curVariableVarNames[i] == "default")
          {
            curVariableVarNames[i] = inPipelineVar;
            break;
          }
      }

      // If multiple extrustions see if the current output variable is
      // in the list of variables.
      int variableCount = curVariableVarNames.size();
#if defined(REPLICATE_EXTRUDE_OPERATOR)
      num_stacked_extrusions = (variableCount > 1 ? variableCount : 0);
#else
      num_stacked_extrusions = variableCount;
#endif
      if( num_stacked_extrusions )
      {
        for (size_t i=0; i<curVariableVarNames.size(); ++i)
        {
          if (curVariableVarNames[i] == outPipelineVar)
          {
            out_contract = new avtContract(in_contract);

            break;
          }
        }
      }
      // For a single extrusion keep the input variable so to allow
      // the user to color with it.
      else
      {
        out_contract = new avtContract(in_contract);
      }

      // If output variable was not found create a new contract with the
      // first variable being the output variable.
      if (!out_contract)
      {
        outPipelineVar = curVariableVarNames[0];

        avtDataRequest_p newDataSpec =
          new avtDataRequest(in_contract->GetDataRequest(), outPipelineVar.c_str());
        out_contract = new avtContract(in_contract, newDataSpec);
      }

      // Get all of the secondary variables needed - do not include
      // the current output variable as a secondary variable.
      for (size_t variableNum = 0; variableNum < curVariableVarNames.size(); variableNum++)
      {
        std::string variableVarName = curVariableVarNames[variableNum];

        if (variableVarName != outPipelineVar)
        {
          needSecondaryVars.push_back(variableVarName);
        }
      }

      const std::vector<CharStrRef> curSecondaryVars =
        in_contract->GetDataRequest()->GetSecondaryVariables();

      // Add the need variables as secondary variables.
      for (size_t need=0; need<needSecondaryVars.size(); ++need)
      {
        const char *needSecondaryVar = needSecondaryVars[need].c_str();

        // Check if the needed variable is already in the secondary
        // variable list.
        bool notInList = true;
        for (size_t i=0; i<curSecondaryVars.size(); ++i)
        {
          if (strcmp(*curSecondaryVars[i], needSecondaryVar) == 0)
          {
            notInList = false;
            break;
          }
        }

        // Not in the list so add it.
        if (notInList)
        {
          out_contract->GetDataRequest()->AddSecondaryVariable(needSecondaryVar);
          out_contract->SetCalculateVariableExtents(needSecondaryVar, true);
        }
      }

      // Remove current secondary variables not needed.
      for (size_t curSecVNum = 0; curSecVNum < curSecondaryVars.size(); curSecVNum++ ) {
        const char *curSecondaryVar = *curSecondaryVars[curSecVNum];

        bool notNeeded = true;
        for (size_t i=0; i<needSecondaryVars.size(); ++i)
        {
          if (strcmp(needSecondaryVars[i].c_str(),curSecondaryVar) == 0)
          {
            notNeeded = false;
            break;
          }
        }

        // Current secondary variable not needed, so remove it.
        if (notNeeded)
        {
          out_contract->GetDataRequest()->RemoveSecondaryVariable(curSecondaryVar);
        }
      }
    }
    else
    {
      out_contract = new avtContract(in_contract);

      // Did not specify an extrude variable so use the data request's
      // variable. But that could be the name of the mesh in some
      // pipelines. So to be safe, request the original cells too so
      // they can be used as the index variable when a mesh name is
      // used.
      out_contract->GetDataRequest()->TurnNodeNumbersOn();
    }

    defaultVariable =
      std::string( out_contract->GetDataRequest()->GetVariable() );

    return out_contract;
}

// ****************************************************************************
//  Method: avtExtrudeStackedFilter::UpdateDataObjectInfo
//
//  Purpose:
//    Update the data object information.
//
//  Note:       The spatial and topo dimension and the extents are updated
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

void
avtExtrudeStackedFilter::UpdateDataObjectInfo(void)
{
    avtDataAttributes &inAtts      = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts     = GetOutput()->GetInfo().GetAttributes();
    avtDataValidity   &outValidity = GetOutput()->GetInfo().GetValidity();

    outAtts.SetTopologicalDimension(inAtts.GetTopologicalDimension()+1);

    if (inAtts.GetSpatialDimension() >= 2)
        outAtts.SetSpatialDimension(3);
    else
        outAtts.SetSpatialDimension(inAtts.GetSpatialDimension()+1);

    outValidity.InvalidateZones();
    outValidity.InvalidateDataMetaData();
    outValidity.InvalidateSpatialMetaData();
    outValidity.SetPointsWereTransformed(true);

    // This filter invalidates any transform matrix in the pipeline.
    outAtts.SetCanUseTransform(false);

    // Extrude using multiple scalar values so new variables will be
    // introduced, an index or the height values. All other original
    // variables will be removed.
    if( num_stacked_extrusions > 0)
    {
      // Remove all of the old variables.
      for( size_t i=0; i<outAtts.GetNumberOfVariables(); ++i)
        outAtts.RemoveVariable( outAtts.GetVariableName(i) );

      ExtrudeStackedAttributes::VariableDisplayType dType =
        atts.GetVariableDisplay();

      // Add the new variables.
      for( size_t i=0; i<3; ++i)
      {
        std::string varName = stackedVarNames[i];

        outAtts.AddVariable(varName);
        outAtts.SetVariableDimension(1, varName.c_str());
        outAtts.SetCentering(i == 0 ? AVT_NODECENT : AVT_ZONECENT, varName.c_str());

        if( (i == 0 && dType == ExtrudeStackedAttributes::NodeHeight) ||
            (i == 1 && dType == ExtrudeStackedAttributes::CellHeight) ||
            (i == 2 && dType == ExtrudeStackedAttributes::VarIndex) )
        {
          outAtts.SetActiveVariable( varName.c_str() );
          outAtts.SetCentering(i == 0 ? AVT_NODECENT : AVT_ZONECENT);
        }
      }
    }

    outAtts.AddFilterMetaData("ExtrudeStacked");
}


// ****************************************************************************
//  Method: avtExtrudeStackedFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtExtrudeStackedFilter with the given
//      parameters would result in an equivalent avtExtrudeStackedFilter.
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

bool
avtExtrudeStackedFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(ExtrudeStackedAttributes*)a);
}


// ****************************************************************************
//  Method: avtExtrudeStackedFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the ExtrudeStacked filter.
//
//  Arguments:
//      in_dr      The input data representation.
//
//  Returns:       The output data representation.
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

avtDataRepresentation *
avtExtrudeStackedFilter::ExecuteData(avtDataRepresentation *in_dr)
{
    // Get the VTK data set.
    vtkDataSet  *in_ds = in_dr->GetDataVTK();
    vtkDataSet *out_ds = nullptr;

    nodeData = false;
    cellData = false;

    varArray = nullptr;

    // Extrude using a scalar value.
    if( atts.GetByVariable() )
    {
        scalarMin = +std::numeric_limits<double>::max();
        scalarMax = -std::numeric_limits<double>::max();

        stringVector variableNames;
        int variableCount = 0;

        // Extrude using the default scalar value
        if( atts.GetScalarVariableNames().size() == 0 )
        {
          variableNames.push_back( defaultVariable );
          variableMinimums.push_back( scalarMax );
          variableMaximums.push_back( scalarMin );
          variableScales   .push_back( 1.0 );
          variableCount = 1;
        }
        // Secondary scalar value(s) have been specified.
        else
        {
          // If the user changes the variable it is possible to have
          // the variable and the 'default' in the list. When that
          // happens delete the specific instance, keeping the more
          // general default.

          // Note the check is done on BOTH the Qvis and avt side.
          int haveBoth = 0;

          for (size_t ax=0; ax<atts.GetScalarVariableNames().size(); ax++)
          {
            if(atts.GetScalarVariableNames()[ax] == "default" ||
               atts.GetScalarVariableNames()[ax] == defaultVariable)
              ++haveBoth;
          }

          if( haveBoth == 2 )
          {
            // atts.DeleteVariable("default", 0);
            atts.DeleteVariable(defaultVariable, 0);

            avtCallback::IssueWarning(
                "ExtrudeStacked - The default variable '" + defaultVariable +
                "' was displayed twice as the 'default' and itself. Which is not allowed. "
                "The specific instance '" + defaultVariable + "' was removed.");
          }

          variableNames    = atts.GetScalarVariableNames();
          variableMinimums = atts.GetExtentMinima();
          variableMaximums = atts.GetExtentMaxima();
          variableScales   = atts.GetExtentScale();
          variableCount    = variableNames.size();

          for (int i=0; i<variableCount; ++i)
          {
            if(variableNames[i] == "default")
            {
              variableNames[i] = defaultVariable;

              break;
            }
          }
        }

        // Variable indicating that there multiple stacked extrusions.
#if defined(REPLICATE_EXTRUDE_OPERATOR)
        atts.SetSteps(1);
        num_stacked_extrusions = (variableCount > 1 ? variableCount : 0);
#else
        num_stacked_extrusions = variableCount;
#endif
        if( num_stacked_extrusions )
        {
            // Not sure if the steps should be limited to one or
            // not. Might make more sense to have a step size.
            if( atts.GetSteps() > 1 )
            {
              EXCEPTION1(ImproperUseException,
                      "ExtrudeStacked - When creating a stacked extrusion, the number of steps must be 1.");
            }
        }

        varStackedIndex = 0;

        // Go through the variable namess in reserse order so to match
        // the list ordering in the GUI view.
        for (varNum=variableCount-1; varNum>=0; --varNum, ++varStackedIndex)
        {
          nodeData = false;
          cellData = false;

          varArray = in_ds->GetPointData()->GetArray(variableNames[varNum].c_str());

          if( varArray ) {
            nodeData = true;
          }
          else {
            varArray = in_ds->GetCellData()->GetArray(variableNames[varNum].c_str() );

            if( varArray ) {
              cellData = true;
            }
          }

          if( varArray == nullptr )
          {
            EXCEPTION1(ImproperUseException,
                       "ExtrudeStacked - Cannot find scalar data, '" +
                       variableNames[varNum] +
                       "' for extruding. All data must be on the same mesh.");
          }

          if(in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
          {
              out_ds = ExtrudeToUnstructuredGrid(vtkRectilinearGrid::SafeDownCast(in_ds),
                                                 vtkUnstructuredGrid::SafeDownCast(out_ds));
          }
          else if(in_ds->GetDataObjectType() == VTK_STRUCTURED_GRID ||
                  in_ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID ||
                  in_ds->GetDataObjectType() == VTK_POLY_DATA)
          {
              out_ds = ExtrudeToUnstructuredGrid(vtkPointSet::SafeDownCast(in_ds),
                                                 vtkUnstructuredGrid::SafeDownCast(out_ds));
          }
          else
          {
            EXCEPTION1(ImproperUseException, "ExtrudeStacked - Unsupported vtk grid for extruding.");
          }
        }
    }

    // Fixed length extrusion.
    else
    {
        num_stacked_extrusions = 0;

        scalarMin = 1.0;
        scalarMax = 1.0;

        if(in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
        {
          const double *axis = atts.GetAxis();

          if(axis[0] == 0. && axis[1] == 0. && axis[2] == 1.)
            out_ds = ExtrudeToRectilinearGrid(in_ds);
          else
            out_ds = ExtrudeToStructuredGrid(in_ds);
        }
        else if(in_ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
        {
          out_ds = ExtrudeToStructuredGrid(in_ds);
        }
        else if(in_ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID ||
                in_ds->GetDataObjectType() == VTK_POLY_DATA)
        {
          out_ds = ExtrudeToUnstructuredGrid(vtkPointSet::SafeDownCast(in_ds));
        }
        else
        {
          EXCEPTION1(ImproperUseException, "ExtrudeStacked - Unsupported vtk grid for extruding.");
        }
    }

    if(out_ds != NULL && !atts.GetPreserveOriginalCellNumbers())
    {
        out_ds->GetCellData()->RemoveArray("avtOriginalCellNumbers");
    }

    avtDataRepresentation *out_dr = NULL;

    if (out_ds != NULL)
    {
        out_dr = new avtDataRepresentation(out_ds, in_dr->GetDomain(),
                                           in_dr->GetLabel());
        out_ds->Delete();
    }

    return out_dr;
}

// ****************************************************************************
//  Method: avtExtrudeStackedFilter::PostExecute
//
//  Purpose:
//      Get the current spatial extents if necessary.
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

void
avtExtrudeStackedFilter::PostExecute(void)
{
    avtDataAttributes &inAtts      = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts     = GetOutput()->GetInfo().GetAttributes();
    avtDataValidity   &outValidity = GetOutput()->GetInfo().GetValidity();

    if( num_stacked_extrusions )
    {
      // Spatial extents have changed, so clear them.
      outAtts.GetOriginalSpatialExtents()->Clear();
      outAtts.GetDesiredSpatialExtents()->Clear();
      outAtts.GetActualSpatialExtents()->Clear();

      // Update the spatial extents.
      double bounds[6];
      avtDataset_p ds = GetTypedOutput();
      avtDatasetExaminer::GetSpatialExtents(ds, bounds);
      outAtts.GetThisProcsOriginalSpatialExtents()->Set(bounds);

      ExtrudeStackedAttributes::VariableDisplayType dType =
        atts.GetVariableDisplay();

      // Set the new data range.
      double range[2];

      if( dType == ExtrudeStackedAttributes::NodeHeight ||
          dType == ExtrudeStackedAttributes::CellHeight ) {
        range[0] = scalarMin;
        range[1] = scalarMax;
      }
      else if( dType == ExtrudeStackedAttributes::VarIndex ) {
        range[0] = 0;
        range[1] = num_stacked_extrusions-1;
      }

      outAtts.GetThisProcsOriginalDataExtents()->Set(range);
      outAtts.GetThisProcsActualDataExtents()->Set(range);
    }
    else
    {
        // Now extrude the extents for a single extrusion
        double b[6];
        if (inAtts.GetOriginalSpatialExtents()->HasExtents())
        {
            inAtts.GetOriginalSpatialExtents()->CopyTo(b);
            ExtrudeExtents(b);
            outAtts.GetOriginalSpatialExtents()->Set(b);
        }

        if (inAtts.GetThisProcsOriginalSpatialExtents()->HasExtents())
        {
            inAtts.GetThisProcsOriginalSpatialExtents()->CopyTo(b);
            ExtrudeExtents(b);
            outAtts.GetThisProcsOriginalSpatialExtents()->Set(b);
        }

        if (inAtts.GetDesiredSpatialExtents()->HasExtents())
        {
            inAtts.GetDesiredSpatialExtents()->CopyTo(b);
            ExtrudeExtents(b);
            outAtts.GetDesiredSpatialExtents()->Set(b);
        }

        if (inAtts.GetActualSpatialExtents()->HasExtents())
        {
            inAtts.GetActualSpatialExtents()->CopyTo(b);
            ExtrudeExtents(b);
            outAtts.GetActualSpatialExtents()->Set(b);
        }

        if (inAtts.GetThisProcsActualSpatialExtents()->HasExtents())
        {
            inAtts.GetThisProcsActualSpatialExtents()->CopyTo(b);
            ExtrudeExtents(b);
            outAtts.GetThisProcsActualSpatialExtents()->Set(b);
        }
    }

    outAtts.AddFilterMetaData("ExtrudeStacked");
}

// ****************************************************************************
//  Method: avtExtrudeStackedFilter::CopyVariables
//
//  Purpose:
//    Copy the variables from the old dataset to the new dataset, replicating
//    as needed.
//
//  Arguments:
//   in_ds           : The input dataset.
//   out_ds          : The output dataset.
//   cellReplication : Optional count of the number of times a cell's data
//                     must be copied. This is used when we break up a cell
//                     into many cells.
//  Returns:
//
//  Note:
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// Modifications:
//
// ****************************************************************************

void
avtExtrudeStackedFilter::CopyVariables(vtkDataSet *in_ds,
                                       vtkDataSet *out_ds,
                                       const int *cellReplication) const
{
    vtkIdType nSteps = atts.GetSteps();

    // Copy cell variables
    vtkCellData *in_cd  = in_ds ->GetCellData();
    vtkCellData *out_cd = out_ds->GetCellData();

    vtkIdType n_in_cells = in_cd->GetNumberOfTuples();

    out_cd->CopyAllocate(in_cd, n_in_cells * (nSteps+1));

    vtkIdType destCell = 0;

    for (vtkIdType l=0; l<nSteps+1; ++l)
    {
        vtkIdType nt = in_cd->GetNumberOfTuples();
        if(cellReplication == NULL)
        {
            for (vtkIdType srcCell=0; srcCell<nt; ++srcCell, ++destCell)
                out_cd->CopyData(in_cd, srcCell, destCell);
        }
        else
        {
            for (vtkIdType srcCell=0; srcCell<nt; ++srcCell)
            {
                for(int cr = 0; cr < cellReplication[srcCell]; ++cr, ++destCell)
                  out_cd->CopyData(in_cd, srcCell, destCell);
            }
        }
    }

    // Copy node variables
    vtkPointData *in_pd  = in_ds ->GetPointData();
    vtkPointData *out_pd = out_ds->GetPointData();

    // When extruding by a cell variable new points are created for each cell.
    if( atts.GetByVariable() && (cellData || nodeData) )
    {
      if(in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
      {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) in_ds;

        int dims[3] = {1,1,1};
        rgrid->GetDimensions(dims);

        vtkIdType nCells = (dims[0]-1) * (dims[1]-1);

        vtkIdType n_out_pts = nCells * 8 * (nSteps+1);
        out_pd->CopyAllocate(in_pd, n_out_pts);

        vtkIdType destPoint = 0;

        for(int l=0; l<nSteps+1; ++l)
        {
          for(int j=0; j<dims[1]-1; ++j)
          {
            for(int i=0; i<dims[0]-1; ++i)
            {
              out_pd->CopyData(in_pd,  j   *dims[0]+i,   destPoint++ );
              out_pd->CopyData(in_pd,  j   *dims[0]+i+1, destPoint++ );
              out_pd->CopyData(in_pd, (j+1)*dims[0]+i+1, destPoint++ );
              out_pd->CopyData(in_pd, (j+1)*dims[0]+i,   destPoint++ );

              out_pd->CopyData(in_pd,  j   *dims[0]+i,   destPoint++ );
              out_pd->CopyData(in_pd,  j   *dims[0]+i+1, destPoint++ );
              out_pd->CopyData(in_pd, (j+1)*dims[0]+i+1, destPoint++ );
              out_pd->CopyData(in_pd, (j+1)*dims[0]+i,   destPoint++ );
            }
          }
        }
      }
      else // Non rectilinear grid.
      {
        vtkIdType nCells = in_ds->GetNumberOfCells();

        vtkIdType n_out_pts = nCells * 8 * (nSteps+1);
        out_pd->CopyAllocate(in_pd, n_out_pts);

        vtkIdType destPoint = 0;

        for (vtkIdType l=0; l<nSteps+1; ++l)
        {
          for (vtkIdType cellid=0; cellid<nCells; ++cellid)
          {
            vtkCell *cell = in_ds->GetCell(cellid);
            int c = cell->GetCellType();

            if( c != VTK_VERTEX   && c != VTK_POLY_VERTEX &&
                c != VTK_LINE     && c != VTK_POLY_LINE &&
                c != VTK_TRIANGLE && c != VTK_TRIANGLE_STRIP &&
                c != VTK_QUAD     && c != VTK_PIXEL )
            {
              EXCEPTION1(InvalidCellTypeException,
                         "ExtrudeStacked - Anything but points, lines, polylines, triangles, and quads.");
            }

            vtkIdList *list = cell->GetPointIds();

            if (c == VTK_VERTEX)
            {
              out_pd->CopyData(in_pd, list->GetId(0), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(0), destPoint++ );
            }
            else if (c == VTK_POLY_VERTEX)
            {
              for(int p = 0; p < list->GetNumberOfIds(); ++p)
              {
                out_pd->CopyData(in_pd, list->GetId(p),   destPoint++ );
                out_pd->CopyData(in_pd, list->GetId(p),   destPoint++ );
              }
            }
            else if(c == VTK_LINE)
            {
              out_pd->CopyData(in_pd, list->GetId(0), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(1), destPoint++ );

              out_pd->CopyData(in_pd, list->GetId(1), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(0), destPoint++ );

            }
            else if (c == VTK_POLY_LINE)
            {
              for(int p = 0; p < list->GetNumberOfIds()-1; ++p)
              {
                out_pd->CopyData(in_pd, list->GetId(p),   destPoint++ );
                out_pd->CopyData(in_pd, list->GetId(p+1), destPoint++ );
                out_pd->CopyData(in_pd, list->GetId(p+1), destPoint++ );
                out_pd->CopyData(in_pd, list->GetId(p),   destPoint++ );
              }
            }
            else if (c == VTK_TRIANGLE)
            {
              out_pd->CopyData(in_pd, list->GetId(0), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(1), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(2), destPoint++ );

              out_pd->CopyData(in_pd, list->GetId(0), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(1), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(2), destPoint++ );
            }
            else if (c == VTK_TRIANGLE_STRIP)
            {
              for(int p = 0; p < list->GetNumberOfIds()-2; ++p)
              {
                out_pd->CopyData(in_pd, list->GetId(0),   destPoint++ );
                out_pd->CopyData(in_pd, list->GetId(p+1), destPoint++ );
                out_pd->CopyData(in_pd, list->GetId(p+2), destPoint++ );

                out_pd->CopyData(in_pd, list->GetId(0),   destPoint++ );
                out_pd->CopyData(in_pd, list->GetId(p+1), destPoint++ );
                out_pd->CopyData(in_pd, list->GetId(p+2), destPoint++ );
              }
            }
            else if(c == VTK_QUAD)
            {
              out_pd->CopyData(in_pd, list->GetId(0), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(1), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(2), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(3), destPoint++ );

              out_pd->CopyData(in_pd, list->GetId(0), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(1), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(2), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(3), destPoint++ );
            }
            else if(c == VTK_PIXEL)
            {
              out_pd->CopyData(in_pd, list->GetId(0), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(1), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(3), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(2), destPoint++ );

              out_pd->CopyData(in_pd, list->GetId(0), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(1), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(3), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(2), destPoint++ );
            }
          }
        }
      }
    }
    // Fixed length extrusion.
    else
    {
      vtkIdType n_out_pts = in_pd->GetNumberOfTuples() * (nSteps+1);
      out_pd->CopyAllocate(in_pd, n_out_pts);

      vtkIdType destPoint = 0;

      for (vtkIdType l=0; l<nSteps+1; ++l)
      {
        vtkIdType nt = in_pd->GetNumberOfTuples();

        for( vtkIdType srcPoint=0; srcPoint<nt; ++srcPoint, ++destPoint)
          out_pd->CopyData(in_pd, srcPoint, destPoint);
      }
    }
}

// ****************************************************************************
//  Method: avtExtrudeStackedFilter::CreateExtrudedPoints
//
//  Purpose:
//    Create a new vtkPoints object that contains a fixed length
//    extruded version of the input points.
//
//  Arguments:
//    oldPoints : The old points that we're extruding.
//    nSteps    : The number of times to copy nodes.
//
//  Returns:    A new vtkPoints object with new points.
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

// Helper function to extrude a fixed length and insert a point into a
// VTK point list.
#define extrude_points(type)                         \
{                                                    \
    type *pts = (type *) points->GetVoidPointer(0);  \
                                                     \
    double length = atts.GetLength();                \
    avtVector axis = atts.GetAxis();                 \
    axis.normalize();                                \
                                                     \
    for(int s=0; s<nSteps; ++s)                      \
    {                                                \
        type t = type(s) / type(nSteps-1);           \
                                                     \
        avtVector offset = axis * (length * t);      \
                                                     \
        for(int n=0; n<nNodes; ++n)                  \
        {                                            \
            double pt[3];                            \
            inPoints->GetPoint(n, pt);               \
                                                     \
            *pts++ = pt[0] + offset.x;               \
            *pts++ = pt[1] + offset.y;               \
            *pts++ = pt[2] + offset.z;               \
        }                                            \
    }                                                \
}

vtkPoints *
avtExtrudeStackedFilter::CreateExtrudePoints(vtkPoints *inPoints, int nSteps)
{
    vtkIdType nNodes = inPoints->GetNumberOfPoints();

    vtkPoints *points = vtkPoints::New(inPoints->GetDataType());
    points->SetNumberOfPoints(nNodes * nSteps);

    if(inPoints->GetDataType() == VTK_FLOAT)
    {
        extrude_points(float);
    }
    else if(inPoints->GetDataType() == VTK_DOUBLE)
    {
        extrude_points(double);
    }

    return points;
}

// ****************************************************************************
//  Method: avtExtrudeStackedFilter::ExtrudeToRectilinearGrid
//
//  Purpose:
//    Extrude an XY planar rectilinear grid into a new rectilinear grid.
//
//    This extrusion is for a fixed length extrusion only.
//
//  Arguments:
//    in_ds : The input dataset.
//
//  Returns:   A new rectilinear grid containing the extruded mesh.
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

vtkDataSet *
avtExtrudeStackedFilter::ExtrudeToRectilinearGrid(vtkDataSet *in_ds) const
{
    vtkRectilinearGrid *out_ds  = NULL;
    vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) in_ds;

    int dims[3] = {1,1,1};
    rgrid->GetDimensions(dims);

    if(dims[2] > 1)
    {
        EXCEPTION1(ImproperUseException, "ExtrudeStacked - 3D data cannot be extruded.");
    }

    if(dims[1] == 1)
    {
        EXCEPTION1(ImproperUseException, "ExtrudeStacked - Extruding curves is not implemented.");
    }

    debug5 << "Extrude rectilinear grid to rectilinear grid." << endl;

    // Get the original z coordinate.
    double z;
    if( dims[2] == 1 )
      z = rgrid->GetZCoordinates()->GetTuple1(0);
    else
      z = 0;

    dims[2] = atts.GetSteps() + 1;

    // Create mesh coordinates.
    vtkDataArray *coords[3] = {NULL, NULL, NULL};
    coords[0] = rgrid->GetXCoordinates()->NewInstance();
    coords[0]->DeepCopy(rgrid->GetXCoordinates());

    coords[1] = rgrid->GetYCoordinates()->NewInstance();
    coords[1]->DeepCopy(rgrid->GetYCoordinates());

    coords[2] = coords[0]->NewInstance();
    coords[2]->SetNumberOfTuples(dims[2]);

    // Fixed length.
    double offset = atts.GetLength();

    for(int i = 0; i < dims[2]; ++i)
    {
        double t = double(i) / double(dims[2]-1);
        coords[2]->SetTuple1(i, z + t * offset);
    }

    // Make a new mesh
    out_ds = vtkRectilinearGrid::New();
    out_ds->SetDimensions(dims);
    out_ds->SetXCoordinates(coords[0]);
    coords[0]->Delete();
    out_ds->SetYCoordinates(coords[1]);
    coords[1]->Delete();
    out_ds->SetZCoordinates(coords[2]);
    coords[2]->Delete();

    // Copy variables
    CopyVariables(in_ds, out_ds);

    return out_ds;
}

// ****************************************************************************
//  Method: avtExtrudeStackedFilter::ExtrudeToStructuredGrid
//
//  Purpose:
//    Extrude a rectilinear or structured data into a structured grid.
//
//    This extrusion is for a fixed length extrusion only.
//
//  Arguments:
//    in_ds : The input dataset.
//
//  Returns : A new structured grid.
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

// Helper function to extrude a fixed length and insert a point into a
// VTK point list
#define extrude_coords(type)                                   \
{                                                              \
    type *pts = (type *) points->GetVoidPointer(0);            \
                                                               \
    double length = atts.GetLength();                          \
    avtVector axis = atts.GetAxis();                           \
    axis.normalize();                                          \
                                                               \
    for(int k = 0; k < dims[2]; ++k)                           \
    {                                                          \
      type t = type(k) / type(dims[2]-1);                      \
                                                               \
      avtVector offset = axis * (length * t);                  \
                                                               \
      vtkIdType cc = 0;                                        \
                                                               \
      for(int j = 0; j < dims[1]; ++j)                         \
      {                                                        \
        double y = rgrid->GetYCoordinates()->GetTuple1(j);     \
                                                               \
        for(int i = 0; i < dims[0]; ++i)                       \
        {                                                      \
          double x = rgrid->GetXCoordinates()->GetTuple1(i);   \
                                                               \
          *pts++ = x + offset.x;                               \
          *pts++ = y + offset.y;                               \
          *pts++ = z + offset.z;                               \
        }                                                      \
      }                                                        \
    }                                                          \
 }

vtkDataSet *
avtExtrudeStackedFilter::ExtrudeToStructuredGrid(vtkDataSet *in_ds)
{
    int dims[3] = {1,1,1};
    vtkPoints *points = NULL;

    // Assemble the coordinates differently according to the input mesh type.
    if(in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        debug5 << "Extrude rectilinear grid to structured grid." << endl;

        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) in_ds;
        rgrid->GetDimensions(dims);

        if(dims[2] > 1)
        {
            EXCEPTION1(ImproperUseException, "ExtrudeStacked - 3D data cannot be extruded.");
        }

        if(dims[1] == 1)
        {
            EXCEPTION1(ImproperUseException, "ExtrudeStacked - Extruding curves is not implemented.");
        }

        // Get the original z coordinate.
        double z;
        if( dims[2] == 1 )
          z = rgrid->GetZCoordinates()->GetTuple1(0);
        else
          z = 0;

        dims[2] = atts.GetSteps()+1;

        points = vtkVisItUtility::NewPoints(rgrid);
        points->SetNumberOfPoints(dims[0] * dims[1] * dims[2]);

        if (points->GetDataType() == VTK_FLOAT)
        {
            extrude_coords(float);
        }
        else if (points->GetDataType() == VTK_DOUBLE)
        {
            extrude_coords(double);
        }
    }
    else if(in_ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        debug5 << "Extrude structured grid to structured grid." << endl;

        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) in_ds;
        sgrid->GetDimensions(dims);

        if(dims[2] > 1)
        {
            EXCEPTION1(ImproperUseException, "ExtrudeStacked - 3D data cannot be extruded.");
        }

        dims[2] = atts.GetSteps()+1;

        points = CreateExtrudePoints(sgrid->GetPoints(), dims[2]);
    }
    else
    {
        EXCEPTION0(ImproperUseException);
    }

    // Assemble the mesh from the points.
    vtkStructuredGrid *out_ds = vtkStructuredGrid::New();
    out_ds->SetDimensions(dims);
    out_ds->SetPoints(points);
    points->Delete();

    // Copy variables
    CopyVariables(in_ds, out_ds);

    return out_ds;
}

// ****************************************************************************
//  Method: avtExtrudeStackedFilter::ExtrudeToUnstructuredGrid
//
//  Purpose:
//    Extrude unstructured grids and polydata into a new unstructured grid.
//
//    This extrusion is for a fixed length extrusion only.
//
//  Arguments:
//    in_ds : The input dataset.
//
//  Returns : A new unstructured grid.
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

vtkDataSet *
avtExtrudeStackedFilter::ExtrudeToUnstructuredGrid(vtkPointSet *in_ds)
{
    debug5 << "Extrude unstructured grid to unstructured grid." << endl;

    int nSteps = atts.GetSteps();

    vtkIdType nNodes = in_ds->GetNumberOfPoints();
    vtkIdType nCells = in_ds->GetNumberOfCells();

    if( atts.GetByVariable() )
    {
        vtkIdType nTuples = varArray->GetNumberOfTuples();

        if( nNodes != nTuples )
          EXCEPTION1(ImproperUseException,
                     "ExtrudeStacked - The number of scalar values does match the number of points.");
    }

    // Create a new grid
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    vtkIdType n_cells_out = nCells * nSteps;
    ugrid->Allocate(n_cells_out * 8);

    // Extrude the points.
    vtkPoints *points = CreateExtrudePoints(in_ds->GetPoints(), nSteps+1);
    ugrid->SetPoints(points);
    points->Delete();

    // Create the extruded connectivity
    int *cellReplication = NULL;

    for(int s = 0; s < nSteps; ++s)
    {
        for (vtkIdType cellid = 0 ; cellid < nCells ; cellid++)
        {
            vtkCell *cell = in_ds->GetCell(cellid);
            int c = cell->GetCellType();

            if( c != VTK_VERTEX   && c != VTK_POLY_VERTEX &&
                c != VTK_LINE     && c != VTK_POLY_LINE &&
                c != VTK_TRIANGLE && c != VTK_TRIANGLE_STRIP &&
                c != VTK_QUAD     && c != VTK_PIXEL )
            {
                ugrid->Delete();
                delete [] cellReplication;
                EXCEPTION1(InvalidCellTypeException,
                           "ExtrudeStacked - Anything but points, lines, polylines, triangles, and quads.");
            }

            vtkIdList *list = cell->GetPointIds();
            vtkIdType verts[8];
            vtkIdType offset = s * nNodes;

            if (c == VTK_VERTEX)
            {
                verts[0] = list->GetId(0) + offset;
                verts[1] = list->GetId(0) + offset + nNodes;

                ugrid->InsertNextCell(VTK_LINE, 2, verts);
            }
            else if (c == VTK_POLY_VERTEX)
            {
                // If seeing a polyvertex for the first time keep
                // track of cell replication (a count of how many
                // cells this cell was broken into) so the
                // cell-centered variables can be copied properly.
                if(s == 0 && cellReplication == NULL)
                {
                    cellReplication = new int[nCells];
                    // Set the replication to one for all of the
                    // previously processed cells.
                    for(int r = 0; r < nCells; ++r)
                        cellReplication[r] = (r < cellid) ? 1 : 0;
                }

                for(int p = 0; p < list->GetNumberOfIds(); ++p)
                {
                    verts[0] = list->GetId(p)   + offset;
                    verts[3] = list->GetId(p)   + offset + nNodes;

                    ugrid->InsertNextCell(VTK_LINE, 2, verts);

                    // Only track the replication for the first step.
                    if(s == 0)
                        cellReplication[cellid]++;
                }

                // Decrement the count because it will be in
                // incremented below regardless of the cell type.
                if(s == 0)
                    cellReplication[cellid]--;
            }
            else if(c == VTK_LINE)
            {
                verts[0] = list->GetId(0) + offset;
                verts[1] = list->GetId(1) + offset;
                verts[2] = list->GetId(1) + offset + nNodes;
                verts[3] = list->GetId(0) + offset + nNodes;

                ugrid->InsertNextCell(VTK_QUAD, 4, verts);
            }
            else if (c == VTK_POLY_LINE)
            {
                // If seeing a polyline for the first time keep track
                // of cell replication (a count of how many cells this
                // cell was broken into) so the cell-centered
                // variables can be copied properly.
                if(s == 0 && cellReplication == NULL)
                {
                    cellReplication = new int[nCells];
                    // Set the replication to one for all of the
                    // previously processed cells.
                    for(int r = 0; r < nCells; ++r)
                        cellReplication[r] = (r < cellid) ? 1 : 0;
                }

                for(int p = 0; p < list->GetNumberOfIds()-1; ++p)
                {
                    verts[0] = list->GetId(p)   + offset;
                    verts[1] = list->GetId(p+1) + offset;
                    verts[2] = list->GetId(p+1) + offset + nNodes;
                    verts[3] = list->GetId(p)   + offset + nNodes;

                    ugrid->InsertNextCell(VTK_QUAD, 4, verts);

                    // Only track the replication for the first step.
                    if(s == 0)
                        cellReplication[cellid]++;
                }

                // Decrement the count because it will be in
                // incremented below regardless of the cell type.
                if(s == 0)
                    cellReplication[cellid]--;
            }
            else if (c == VTK_TRIANGLE)
            {
                verts[0] = list->GetId(0) + offset;
                verts[1] = list->GetId(1) + offset;
                verts[2] = list->GetId(2) + offset;
                verts[3] = list->GetId(0) + offset + nNodes;
                verts[4] = list->GetId(1) + offset + nNodes;
                verts[5] = list->GetId(2) + offset + nNodes;

                ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
            }
            else if (c == VTK_TRIANGLE_STRIP)
            {
                // If seeing a triangle strip for the first time keep
                // track of cell replication (a count of how many
                // cells this cell was broken into) so the
                // cell-centered variables can be copied properly.
                if(s == 0 && cellReplication == NULL)
                {
                    cellReplication = new int[nCells];
                    // Set the replication to one for all of the
                    // previously processed cells.
                    for(int r = 0; r < nCells; ++r)
                        cellReplication[r] = (r < cellid) ? 1 : 0;
                }

                for(int p = 0; p < list->GetNumberOfIds()-2; ++p)
                {
                    verts[0] = list->GetId(p)   + offset;
                    verts[1] = list->GetId(p+1) + offset;
                    verts[2] = list->GetId(p+2) + offset;
                    verts[3] = list->GetId(p)   + offset + nNodes;
                    verts[4] = list->GetId(p+1) + offset + nNodes;
                    verts[5] = list->GetId(p+2) + offset + nNodes;

                    ugrid->InsertNextCell(VTK_WEDGE, 6, verts);

                    // Only track the replication for the first step.
                    if(s == 0)
                        cellReplication[cellid]++;
                }

                // Decrement the count because it will be in
                // incremented below regardless of the cell type.
                if(s == 0)
                    cellReplication[cellid]--;
            }
            else if(c == VTK_QUAD)
            {
                verts[0] = list->GetId(0) + offset;
                verts[1] = list->GetId(1) + offset;
                verts[2] = list->GetId(2) + offset;
                verts[3] = list->GetId(3) + offset;
                verts[4] = list->GetId(0) + offset + nNodes;
                verts[5] = list->GetId(1) + offset + nNodes;
                verts[6] = list->GetId(2) + offset + nNodes;
                verts[7] = list->GetId(3) + offset + nNodes;

                ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            }
            else if(c == VTK_PIXEL)
            {
                verts[0] = list->GetId(0) + offset;
                verts[1] = list->GetId(1) + offset;
                verts[2] = list->GetId(3) + offset;
                verts[3] = list->GetId(2) + offset;
                verts[4] = list->GetId(0) + offset + nNodes;
                verts[5] = list->GetId(1) + offset + nNodes;
                verts[6] = list->GetId(3) + offset + nNodes;
                verts[7] = list->GetId(2) + offset + nNodes;

                ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            }

            // Increment the replication by one all cells.
            if(s == 0 && cellReplication != NULL)
                cellReplication[cellid]++;
        }
    }

    // Copy the variables
    CopyVariables(in_ds, ugrid, cellReplication);

    delete [] cellReplication;

    return ugrid;
}

// ****************************************************************************
//  Method: avtExtrudeStackedFilter::ExtrudeToUnstructuredGrid
//
//  Purpose:
//    Extrude a rectilinear grid into a new unstructured grid using
//    node or cell based scalar data value.
//
//  Arguments:
//    in_ds : The input dataset.
//
//  Returns : A new structured grid.
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

// Helper function to extrude and insert a point into a VTK vertex list.
//
// If there are previous points the base point is the previously
// extruded point. Which for a hex are the last four points. Thus when
// the id_out is less than four add four to get the previous point.
#define extrude_2Dpoint( ix, iy, id_in, id_out, offset )           \
{                                                                  \
  if( varStackedIndex )                                              \
  {                                                                \
    vtkIdType index = baseIndex + id_out;                          \
                                                                   \
    if( id_out < 4 )                                               \
      index += 4;                                                  \
                                                                   \
    points->GetPoint(index, pt);                                   \
  }                                                                \
  else                                                             \
  {                                                                \
    pt[0] = in_ds->GetXCoordinates()->GetTuple1(ix);               \
    pt[1] = in_ds->GetYCoordinates()->GetTuple1(iy);               \
    pt[2] = z0;                                                    \
  }                                                                \
                                                                   \
  if(nodeData)                                                     \
  {                                                                \
    scalar = varArray->GetTuple1(list->GetId(id_in));              \
                                                                   \
    if( scalar < variableMinimums[varNum] )                        \
      scalar = variableMinimums[varNum];                           \
                                                                   \
    if( scalar > variableMaximums[varNum] )                        \
      scalar = variableMaximums[varNum];                           \
                                                                   \
    if( scalarMin > scalar )                                       \
      scalarMin = scalar;                                          \
                                                                   \
    if( scalarMax < scalar )                                       \
      scalarMax = scalar;                                          \
  }                                                                \
                                                                   \
  scalarAve += (scalar / 8.0);                                     \
                                                                   \
  pt[0] += offset.x * scalar;                                      \
  pt[1] += offset.y * scalar;                                      \
  pt[2] += offset.z * scalar;                                      \
                                                                   \
  verts[id_out] = points->InsertNextPoint( pt[0], pt[1], pt[2] );  \
                                                                   \
  if( num_stacked_extrusions )                                     \
    scalars_node_height->InsertTuple1(verts[id_out], scalar);      \
}


vtkDataSet *
avtExtrudeStackedFilter::ExtrudeToUnstructuredGrid(vtkRectilinearGrid *in_ds,
                                                   vtkUnstructuredGrid *out_ds )
{
    debug5 << "Extrude rectilinear grid to unstructured grid." << endl;

    int nSteps = atts.GetSteps();

    int dims[3] = {1,1,1};
    in_ds->GetDimensions(dims);

    if(dims[2] > 1)
    {
      EXCEPTION1(ImproperUseException, "ExtrudeStacked - 3D data cannot be extruded.");
    }

    if(dims[1] == 1)
    {
      EXCEPTION1(ImproperUseException, "ExtrudeStacked - Extruding curves is not implemented.");
    }

    vtkIdType nNodes = in_ds->GetNumberOfPoints();
    vtkIdType nCells = in_ds->GetNumberOfCells();

    if( atts.GetByVariable() )
    {
        vtkIdType nTuples = varArray->GetNumberOfTuples();

        if( nodeData && nNodes != nTuples )
          EXCEPTION1(ImproperUseException,
                     "ExtrudeStacked - The number of scalar values does match the number of points.");
        if( cellData && nCells != nTuples )
          EXCEPTION1(ImproperUseException,
                     "ExtrudeStacked - The number of scalar values does match the number of cells.");

    }

    vtkUnstructuredGrid *ugrid = nullptr;
    vtkPoints *points = nullptr;
    vtkDoubleArray *scalars_node_height = nullptr;
    vtkDoubleArray *scalars_cell_height = nullptr;
    vtkFloatArray  *scalars_var_index   = nullptr;

    vtkIdType n_cells_out = nCells * nSteps;
    unsigned int nPtsPerStack;
    unsigned int nPointsAdded = n_cells_out * 8;

    // Set the grid so it can be used for multiple stacked extrusions.
    if( out_ds == nullptr ) {
      ugrid = vtkUnstructuredGrid::New();
      ugrid->Allocate(n_cells_out, n_cells_out);

      points = vtkPoints::New(in_ds->GetYCoordinates()->GetDataType());
      points->Allocate( nPointsAdded, nPointsAdded );

      nPtsPerStack = 0;

      // For multiple stacked extrusions two new variables are used.
      if( num_stacked_extrusions )
      {
        // The variable used for the extrusion. Doubles are used as
        // there may be multiple type (floats or doubles).
        scalars_node_height = vtkDoubleArray::New();
        scalars_node_height->SetName(stackedVarNames[0].c_str());
        scalars_node_height->SetNumberOfTuples(1);
        scalars_node_height->Allocate( nPointsAdded, nPointsAdded );

        // The height used for the extrusion. Doubles are used as
        // there may be multiple type (floats or doubles).
        scalars_cell_height = vtkDoubleArray::New();
        scalars_cell_height->SetName(stackedVarNames[1].c_str());
        scalars_cell_height->SetNumberOfTuples(1);
        scalars_cell_height->Allocate( n_cells_out, n_cells_out );

        // The index of the variable.
        scalars_var_index = vtkFloatArray::New();
        scalars_var_index->SetName(stackedVarNames[2].c_str());
        scalars_var_index->SetNumberOfTuples(1);
        scalars_var_index->Allocate( n_cells_out, n_cells_out );
      }
    }
    // Used for the next stacked extrusion.
    else
    {
      ugrid = out_ds;

      points = ugrid->GetPoints();

      nPtsPerStack = points->GetNumberOfPoints() / varStackedIndex;

      scalars_node_height = (vtkDoubleArray *)
        ugrid->GetPointData()->GetArray(stackedVarNames[0].c_str());
      scalars_cell_height = (vtkDoubleArray *)
        ugrid->GetCellData()->GetArray(stackedVarNames[1].c_str());
      scalars_var_index   = (vtkFloatArray  *)
        ugrid->GetCellData()->GetArray(stackedVarNames[2].c_str());

      // Make sure the mesh (cells) sizes are the same.
      if( nPtsPerStack % nPointsAdded != 0 )
      {
          EXCEPTION1(ImproperUseException,
                     "ExtrudeStacked - The number of cells in the meshes do not match.");
      }
    }

    // Create the extruded cells
    double pt[3];
    double scalar = 1.0, scalarAve = 0.0;

    vtkIdType baseIndex = nPtsPerStack * (varStackedIndex - 1);

    avtVector axis(atts.GetAxis());
    axis.normalize();

    double scale = variableScales[varNum];
    axis *= scale;

    // Get the original z coordinate.
    double z0;
    if( dims[2] == 1 )
      z0 = in_ds->GetZCoordinates()->GetTuple1(0);
    else
      z0 = 0;

    for(int s=0; s<nSteps; ++s)
    {
        double t0 = double(s  ) / double(nSteps);
        double t1 = double(s+1) / double(nSteps);

        avtVector offset0 = axis * t0;
        avtVector offset1 = axis * t1;

        for(int j=0; j<dims[1]-1; ++j)
        {
          for(int i=0; i<dims[0]-1; ++i)
          {
            vtkIdType cellId = j * (dims[0]-1) + i;
            vtkCell *cell = in_ds->GetCell(cellId);

            if( cellData )
            {
              scalar = varArray->GetTuple1( cellId );

              if( scalar < variableMinimums[varNum] )
                scalar = variableMinimums[varNum];

              if( scalar > variableMaximums[varNum] )
                scalar = variableMaximums[varNum];

              if( scalarMin > scalar )
                scalarMin = scalar;

              if( scalarMax < scalar )
                scalarMax = scalar;
            }

            vtkIdList *list = cell->GetPointIds();
            vtkIdType verts[8];

            scalarAve = 0.0;

            extrude_2Dpoint( i+0, j+0, 0, 0, offset0 );
            extrude_2Dpoint( i+1, j+0, 1, 1, offset0 );
            extrude_2Dpoint( i+1, j+1, 3, 2, offset0 );
            extrude_2Dpoint( i+0, j+1, 2, 3, offset0 );

            extrude_2Dpoint( i+0, j+0, 0, 4, offset1 );
            extrude_2Dpoint( i+1, j+0, 1, 5, offset1 );
            extrude_2Dpoint( i+1, j+1, 3, 6, offset1 );
            extrude_2Dpoint( i+0, j+1, 2, 7, offset1 );

            vtkIdType newCellId =
              ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);

            if( num_stacked_extrusions )
            {
              baseIndex += 8;

              if( scalarMin > scalarAve )
                scalarMin = scalarAve;

              if( scalarMax < scalarAve )
                scalarMax = scalarAve;

              scalars_cell_height->InsertTuple1(newCellId, scalarAve);
              scalars_var_index ->InsertTuple1(newCellId, varStackedIndex);
            }
          }
        }
    }

    // Assemble the mesh from the points.
    if( out_ds == nullptr )
    {
      out_ds = ugrid;

      ugrid->SetPoints(points);
      points->Delete();

      // For multiple stacked extrusions save only the scalar value
      // and the index.
      if( num_stacked_extrusions )
      {
        ugrid->GetPointData()->AddArray( scalars_node_height );
        ugrid->GetCellData() ->AddArray( scalars_cell_height );
        ugrid->GetCellData() ->AddArray( scalars_var_index );

      ExtrudeStackedAttributes::VariableDisplayType dType =
        atts.GetVariableDisplay();

        if( dType == ExtrudeStackedAttributes::NodeHeight )
          ugrid->GetPointData()->SetScalars( scalars_node_height );
        else if( dType == ExtrudeStackedAttributes::CellHeight )
          ugrid->GetCellData()->SetScalars( scalars_cell_height );
        else if( dType == ExtrudeStackedAttributes::VarIndex )
          ugrid->GetCellData()->SetScalars( scalars_var_index );

        scalars_node_height->Delete();
        scalars_cell_height->Delete();
        scalars_var_index  ->Delete();
      }
      else // Otherwise copy all of the previous variables
        CopyVariables(in_ds, ugrid);
    }

    return out_ds;
}

// ****************************************************************************
//  Method: avtExtrudeStackedFilter::ExtrudeToUnstructuredGrid
//
//  Purpose:
//    Extrude unstructured grids and polydata into a new unstructured
//    grid using node or cell based scalar data value.
//
//  Arguments:
//    in_ds : The input dataset.
//
//  Returns : A new unstructured grid.
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

// Helper function to extrude a point by a cell value and insert it
// into a VTK vertex list.
//
// If there are previous points the base point is the previously
// extruded point. Which for a hex are the last four (incr/2)
// points. Thus when the id_out is less than four (incr/2) add four
// (incr/2) to get the previous point.
#define extrude_point( id_in, id_out, offset )                     \
{                                                                  \
  if( varStackedIndex )                                              \
  {                                                                \
    vtkIdType index = baseIndex + id_out;                          \
                                                                   \
    if( id_out < incr/2 )                                          \
      index += incr/2;                                             \
                                                                   \
    points->GetPoint(index, pt);                                   \
  }                                                                \
  else                                                             \
  {                                                                \
    in_ds->GetPoint(list->GetId(id_in), pt);                       \
  }                                                                \
                                                                   \
  if(nodeData)                                                     \
  {                                                                \
    scalar = varArray->GetTuple1(list->GetId(id_in));              \
                                                                   \
    if( scalar < variableMinimums[varStackedIndex] )                 \
      scalar = variableMinimums[varStackedIndex];                    \
                                                                   \
    if( scalar > variableMaximums[varStackedIndex] )                 \
      scalar = variableMaximums[varStackedIndex];                    \
                                                                   \
    if( scalarMin > scalar )                                       \
      scalarMin = scalar;                                          \
                                                                   \
    if( scalarMax < scalar )                                       \
      scalarMax = scalar;                                          \
  }                                                                \
                                                                   \
  scalarAve += (scalar / double(incr));                            \
                                                                   \
  pt[0] += offset.x * scalar;                                      \
  pt[1] += offset.y * scalar;                                      \
  pt[2] += offset.z * scalar;                                      \
                                                                   \
  verts[id_out] = points->InsertNextPoint( pt[0], pt[1], pt[2] );  \
                                                                   \
  if( num_stacked_extrusions )                                     \
    scalars_node_height->InsertTuple1(verts[id_out], scalar);      \
}

vtkDataSet *
avtExtrudeStackedFilter::ExtrudeToUnstructuredGrid(vtkPointSet *in_ds,
                                                   vtkUnstructuredGrid *out_ds)
{
    debug5 << "Extrude unstructured grid to unstructured grid." << endl;

    int nSteps = atts.GetSteps();

    vtkIdType nNodes = in_ds->GetNumberOfPoints();
    vtkIdType nCells = in_ds->GetNumberOfCells();

    if( atts.GetByVariable() )
    {
        vtkIdType nTuples = varArray->GetNumberOfTuples();

        if( nodeData && nNodes != nTuples )
          EXCEPTION1(ImproperUseException,
                     "ExtrudeStacked - The number of scalar values does match the number of points.");
        if( cellData && nCells != nTuples )
          EXCEPTION1(ImproperUseException,
                     "ExtrudeStacked - The number of scalar values does match the number of cells.");
    }

    vtkUnstructuredGrid *ugrid = nullptr;
    vtkPoints *points = nullptr;
    vtkDoubleArray *scalars_node_height = nullptr;
    vtkDoubleArray *scalars_cell_height = nullptr;
    vtkFloatArray  *scalars_var_index   = nullptr;

    // Set the grid so it can be used for multiple stacked extrusions.
    vtkIdType n_cells_out = nCells * nSteps;
    unsigned int nPtsPerStack;
    unsigned int nPointsAdded = n_cells_out * 8;

    // Set the grid so it can be used for multiple stacked extrusions.
    if( out_ds == nullptr ) {
      ugrid = vtkUnstructuredGrid::New();
      ugrid->Allocate(n_cells_out, n_cells_out);

      points = vtkPoints::New(in_ds->GetPoints()->GetDataType());
      points->Allocate( nPointsAdded, nPointsAdded );

      nPtsPerStack = 0;

      // For multiple stacked extrusions two new variables are used.
      if( num_stacked_extrusions )
      {
        // The variable used for the extrusion. Doubles are used as
        // there may be multiple type (floats or doubles).
        scalars_node_height = vtkDoubleArray::New();
        scalars_node_height->SetName(stackedVarNames[0].c_str());
        scalars_node_height->SetNumberOfTuples(1);
        scalars_node_height->Allocate( nPointsAdded, nPointsAdded );

        // The height used for the extrusion. Doubles are used as
        // there may be multiple type (floats or doubles).
        scalars_cell_height = vtkDoubleArray::New();
        scalars_cell_height->SetName(stackedVarNames[1].c_str());
        scalars_cell_height->SetNumberOfTuples(1);
        scalars_cell_height->Allocate( n_cells_out, n_cells_out );

        // The index of the variable.
        scalars_var_index = vtkFloatArray::New();
        scalars_var_index->SetName(stackedVarNames[2].c_str());
        scalars_var_index->SetNumberOfTuples(1);
        scalars_var_index->Allocate( n_cells_out, n_cells_out );
      }
    }
    // Used for the next stacked extrusion.
    else
    {
      ugrid = out_ds;

      points = ugrid->GetPoints();

      nPtsPerStack = points->GetNumberOfPoints() / varStackedIndex;

      scalars_node_height = (vtkDoubleArray *) ugrid->GetPointData()->GetArray(stackedVarNames[0].c_str());
      scalars_cell_height = (vtkDoubleArray *) ugrid->GetCellData()->GetArray(stackedVarNames[1].c_str());
      scalars_var_index   = (vtkFloatArray  *) ugrid->GetCellData()->GetArray(stackedVarNames[2].c_str());
    }

    // Create the extruded connectivity
    int incr, *cellReplication = NULL;

    double pt[3];
    double scalar = 1.0, scalarAve = 0.0;

    vtkIdType baseIndex = nPtsPerStack * (varStackedIndex - 1);

    avtVector axis(atts.GetAxis());
    axis.normalize();

    double scale = variableScales[varNum];
    axis *= scale;

    for(int s=0; s<nSteps; ++s)
    {
        double t0 = double(s  ) / double(nSteps);
        double t1 = double(s+1) / double(nSteps);

        avtVector offset0 = axis * t0;
        avtVector offset1 = axis * t1;

        for (vtkIdType cellId=0; cellId<nCells; ++cellId)
        {
            vtkCell *cell = in_ds->GetCell(cellId);
            vtkIdType c = cell->GetCellType();

            if( c != VTK_VERTEX   && c != VTK_POLY_VERTEX &&
                c != VTK_LINE     && c != VTK_POLY_LINE &&
                c != VTK_TRIANGLE && c != VTK_TRIANGLE_STRIP &&
                c != VTK_QUAD     && c != VTK_PIXEL )
            {
                ugrid->Delete();
                delete [] cellReplication;
                EXCEPTION1(InvalidCellTypeException,
                           "ExtrudeStacked - Anything but points, lines, polylines, triangles, and quads.");
            }

            if( cellData )
            {
              scalar = varArray->GetTuple1(cellId);

              if( scalar < variableMinimums[varStackedIndex] )
                scalar = variableMinimums[varStackedIndex];

              if( scalar > variableMaximums[varStackedIndex] )
                scalar = variableMaximums[varStackedIndex];

              if( scalarMin > scalar )
                scalarMin = scalar;

              if( scalarMax < scalar )
                scalarMax = scalar;
            }

            vtkIdList *list = cell->GetPointIds();
            vtkIdType newCellId, verts[8];

            scalarAve = 0.0;

            if (c == VTK_VERTEX)
            {
              incr = 2;
              extrude_point( 0, 0, offset0 );
              extrude_point( 0, 1, offset1 );

              newCellId = ugrid->InsertNextCell(VTK_LINE, 2, verts);
            }
            else if (c == VTK_POLY_VERTEX)
            {
                // If seeing a polyvertex for the first time keep
                // track of cell replication (a count of how many
                // cells this cell was broken into) so the
                // cell-centered variables can be copied properly.
                if(s == 0 && cellReplication == NULL)
                {
                  cellReplication = new int[nCells];

                  // Set the replication to one for all of the
                  // previously processed cells.
                  for(int r = 0; r < nCells; ++r)
                    cellReplication[r] = (r < cellId) ? 1 : 0;
                }

                for(int p = 0; p < list->GetNumberOfIds(); ++p)
                {
                  scalarAve = 0.0;

                  incr = 2;
                  extrude_point( p, 0, offset0 );
                  extrude_point( p, 1, offset1 );

                  newCellId = ugrid->InsertNextCell(VTK_LINE, 2, verts);

                  // For a stacked two vars are created the scalar
                  // value and the index of the vairable.
                  if( num_stacked_extrusions )
                  {
                    baseIndex += incr;

                    if( scalarMin > scalarAve )
                      scalarMin = scalarAve;

                    if( scalarMax < scalarAve )
                      scalarMax = scalarAve;

                    scalars_cell_height->InsertTuple1(newCellId, scalarAve);
                    scalars_var_index  ->InsertTuple1(newCellId, varStackedIndex);
                  }

                  // Only track the replication for the first step.
                  if(s == 0)
                    cellReplication[cellId]++;
                }

                // Decrement the count because it will be in
                // incremented below regardless of the cell type.
                if(s == 0)
                    cellReplication[cellId]--;
            }
            else if(c == VTK_LINE)
            {
              incr = 4;
              extrude_point( 0, 0, offset0 );
              extrude_point( 1, 1, offset0 );
              extrude_point( 1, 2, offset1 );
              extrude_point( 0, 3, offset1 );

              // When stacking a quad the point order must be swapped
              // because when getting the previous point the order is
              // reversed. That is the mapping is B0->A2 and B1->A3
              // via ascending point order. But it should be B0->A3
              // and B1->A2. So swap.

              // A0 A3 B0 B3
              // A1 A2 B1 B2
              if(varStackedIndex)
              {
                vtkIdType id = verts[0];
                verts[0] = verts[1];
                verts[1] = id;
              }

              newCellId = ugrid->InsertNextCell(VTK_QUAD, 4, verts);
            }
            else if (c == VTK_POLY_LINE)
            {
                // If seeing a polyline for the first time keep track
                // of cell replication (a count of how many cells this
                // cell was broken into) so the cell-centered
                // variables can be copied properly.
                if(s == 0 && cellReplication == NULL)
                {
                  cellReplication = new int[nCells];

                  // Set the replication to one for all of the
                  // previously processed cells.
                  for(int r = 0; r < nCells; ++r)
                    cellReplication[r] = (r < cellId) ? 1 : 0;
                }

                for(int p = 0; p < list->GetNumberOfIds()-1; ++p)
                {
                  scalarAve = 0.0;

                  incr = 4;
                  extrude_point( p,   0, offset0 );
                  extrude_point( p+1, 1, offset0 );
                  extrude_point( p+1, 2, offset1 );
                  extrude_point( p,   3, offset1 );

                  // See the comment above on swapping the point order
                  // for a line.
                  if(varStackedIndex)
                  {
                    vtkIdType id = verts[0];
                    verts[0] = verts[1];
                    verts[1] = id;
                  }

                  newCellId = ugrid->InsertNextCell(VTK_QUAD, 4, verts);

                  // For a stacked two vars are created the scalar
                  // value and the index of the vairable.
                  if( num_stacked_extrusions )
                  {
                    baseIndex += incr;

                    if( scalarMin > scalarAve )
                      scalarMin = scalarAve;

                    if( scalarMax < scalarAve )
                      scalarMax = scalarAve;

                    scalars_cell_height->InsertTuple1(newCellId, scalarAve);
                    scalars_var_index  ->InsertTuple1(newCellId, varStackedIndex);
                  }

                  // Only track the replication for the first step.
                  if(s == 0)
                    cellReplication[cellId]++;
                }

                // Decrement the count because it will be in
                // incremented below regardless of the cell type.
                if(s == 0)
                    cellReplication[cellId]--;
            }
            else if (c == VTK_TRIANGLE)
            {
              incr = 6;
              extrude_point( 0, 0, offset0 );
              extrude_point( 1, 1, offset0 );
              extrude_point( 2, 2, offset0 );
              extrude_point( 0, 3, offset1 );
              extrude_point( 1, 4, offset1 );
              extrude_point( 2, 5, offset1 );

              newCellId = ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
            }
            else if (c == VTK_TRIANGLE_STRIP)
            {
                // If seeing a triangle strip for the first time keep
                // track of cell replication (a count of how many
                // cells this cell was broken into) so the
                // cell-centered variables can be copied properly.
                if(s == 0 && cellReplication == NULL)
                {
                  cellReplication = new int[nCells];

                  // Set the replication to one for all of the
                  // previously processed cells.
                  for(int r = 0; r < nCells; ++r)
                    cellReplication[r] = (r < cellId) ? 1 : 0;
                }

                for(int p = 0; p < list->GetNumberOfIds()-2; ++p)
                {
                  scalarAve = 0.0;

                  incr = 6;
                  extrude_point( p,   0, offset0 );
                  extrude_point( p+1, 1, offset0 );
                  extrude_point( p+2, 2, offset0 );

                  extrude_point( p,   3, offset1 );
                  extrude_point( p+1, 4, offset1 );
                  extrude_point( p+2, 5, offset1 );

                  newCellId = ugrid->InsertNextCell(VTK_WEDGE, 6, verts);

                  // For a stacked two vars are created the scalar
                  // value and the index of the vairable.
                  if( num_stacked_extrusions )
                  {
                    baseIndex += incr;

                    if( scalarMin > scalarAve )
                      scalarMin = scalarAve;

                    if( scalarMax < scalarAve )
                      scalarMax = scalarAve;

                    scalars_cell_height->InsertTuple1(newCellId, scalarAve);
                    scalars_var_index  ->InsertTuple1(newCellId, varStackedIndex);
                  }

                  // Only track the replication for the first step.
                  if(s == 0)
                    cellReplication[cellId]++;
                }

                // Decrement the count because it will be in
                // incremented below regardless of the cell type.
                if(s == 0)
                    cellReplication[cellId]--;
            }
            else if(c == VTK_QUAD)
            {
              incr = 8;
              extrude_point( 0, 0, offset0 );
              extrude_point( 1, 1, offset0 );
              extrude_point( 2, 2, offset0 );
              extrude_point( 3, 3, offset0 );
              extrude_point( 0, 4, offset1 );
              extrude_point( 1, 5, offset1 );
              extrude_point( 2, 6, offset1 );
              extrude_point( 3, 7, offset1 );

              newCellId = ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            }
            else if(c == VTK_PIXEL)
            {
              incr = 8;
              extrude_point( 0, 0, offset0 );
              extrude_point( 1, 1, offset0 );
              extrude_point( 3, 2, offset0 );
              extrude_point( 2, 3, offset0 );
              extrude_point( 0, 4, offset1 );
              extrude_point( 1, 5, offset1 );
              extrude_point( 3, 6, offset1 );
              extrude_point( 2, 7, offset1 );

              newCellId = ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            }

            // For a stacked extrusion two vars are created the scalar
            // value and the index of the vairable. The polyline cells
            // were done above as they were broken into mulitple cells.
            if( num_stacked_extrusions &&
                c != VTK_POLY_VERTEX &&
                c != VTK_POLY_LINE && c != VTK_TRIANGLE_STRIP)
            {
              baseIndex += incr;

              if( scalarMin > scalarAve )
                scalarMin = scalarAve;

              if( scalarMax < scalarAve )
                scalarMax = scalarAve;

              scalars_cell_height->InsertTuple1(newCellId, scalarAve);
              scalars_var_index  ->InsertTuple1(newCellId, varStackedIndex);
            }

            // Increment the replication by one all cells.
            if(s == 0 && cellReplication != NULL)
                cellReplication[cellId]++;
        }
    }

    // Assemble the mesh from the points.
    if( out_ds == nullptr )
    {
      out_ds = ugrid;

      ugrid->SetPoints(points);
      points->Delete();

      // For multiple stacked extrusions save only the scalar value
      // and the index.
      if( num_stacked_extrusions )
      {
        ugrid->GetPointData()->AddArray( scalars_node_height );
        ugrid->GetCellData() ->AddArray( scalars_cell_height );
        ugrid->GetCellData() ->AddArray( scalars_var_index );

        ExtrudeStackedAttributes::VariableDisplayType dType =
          atts.GetVariableDisplay();

        if( dType == ExtrudeStackedAttributes::NodeHeight )
          ugrid->GetPointData()->SetScalars( scalars_node_height );
        else if( dType == ExtrudeStackedAttributes::CellHeight )
          ugrid->GetCellData()->SetScalars( scalars_cell_height );
        else if( dType == ExtrudeStackedAttributes::VarIndex )
          ugrid->GetCellData()->SetScalars( scalars_var_index );

        scalars_node_height->Delete();
        scalars_cell_height->Delete();
        scalars_var_index  ->Delete();
      }
      else // Otherwise copy all of the previous variables
        CopyVariables(in_ds, ugrid, cellReplication);
    }

    delete [] cellReplication;

    return out_ds;
}

// ****************************************************************************
//  Method: avtRevolveFilter::ExtrudeExtents
//
//  Purpose:
//      Determines the extents of a dataset that has been extruded
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023
//
// ****************************************************************************

void
avtExtrudeStackedFilter::ExtrudeExtents(double *dbounds) const
{
    avtVector offset(atts.GetAxis());
    offset.normalize();
    offset *= atts.GetLength();

#define eMIN(A,B) (((A)<(B)) ? (A) : (B))
#define eMAX(A,B) (((A)>(B)) ? (A) : (B))

    dbounds[0] = eMIN(dbounds[0], dbounds[0] + offset.x * scalarMin);
    dbounds[1] = eMAX(dbounds[1], dbounds[1] + offset.x * scalarMax);
    dbounds[2] = eMIN(dbounds[2], dbounds[2] + offset.y * scalarMin);
    dbounds[3] = eMAX(dbounds[3], dbounds[3] + offset.y * scalarMax);
    dbounds[4] = eMIN(dbounds[4], dbounds[4] + offset.z * scalarMin);
    dbounds[5] = eMAX(dbounds[5], dbounds[5] + offset.z * scalarMax);
}
