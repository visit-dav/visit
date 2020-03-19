// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtExtrudeFilter.C
// ************************************************************************* //

#include <avtExtrudeFilter.h>

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

#include <avtDatasetExaminer.h>
#include <avtExtents.h>

#include <BadVectorException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidCellTypeException.h>

// ****************************************************************************
//  Method: avtExtrudeFilter constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 13:48:57 PST 2011
//
// ****************************************************************************

avtExtrudeFilter::avtExtrudeFilter()
{
}


// ****************************************************************************
//  Method: avtExtrudeFilter destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 13:48:57 PST 2011
//
//  Modifications:
//
// ****************************************************************************

avtExtrudeFilter::~avtExtrudeFilter()
{
}


// ****************************************************************************
//  Method:  avtExtrudeFilter::Create
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 13:48:57 PST 2011
//
// ****************************************************************************

avtFilter *
avtExtrudeFilter::Create()
{
    return new avtExtrudeFilter();
}


// ****************************************************************************
//  Method:      avtExtrudeFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 13:48:57 PST 2011
//
// ****************************************************************************

void
avtExtrudeFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const ExtrudeAttributes*) a;

    avtVector axis(atts.GetAxis());
    
    if (axis.x == 0. && axis.y == 0. && axis.z == 0.)
    {
        EXCEPTION1(BadVectorException, "Zero length extrusion axis.");
    }

    if(atts.GetLength() == 0.)
    {
      if( atts.GetByVariable() ) {
        EXCEPTION1(ImproperUseException, "The scale can not be 0.");
      }
      else {
        EXCEPTION1(ImproperUseException, "The length can not be 0.");
      }
    }

    if(atts.GetSteps() < 1)
    {
        EXCEPTION1(ImproperUseException, "The number of steps must be at least 1.");
    }
}


// ****************************************************************************
//  Method: avtExtrudeFilter::ModifyContract
//
//  Purpose:
//      This method makes any necessay modification to the VisIt contract
//      to request that the extrude variable is also loaded if it 
//      is not already part of the contract.
//
// ****************************************************************************

avtContract_p
avtExtrudeFilter::ModifyContract(avtContract_p in_contract)
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
        debug3 << "Extrude operator attributes are inconsistent." << endl;
        return in_contract;
      }
      
      stringVector curVariableVarNames = atts.GetScalarVariableNames();
      stringVector needSecondaryVars;
      const char *inPipelineVar = in_contract->GetDataRequest()->GetVariable();
      std::string outPipelineVar(inPipelineVar);

      // See if the current output variable is in the list of variables.
      for (size_t i=0; i<curVariableVarNames.size(); ++i)
      {
        if (curVariableVarNames[i] == outPipelineVar)
        {
          out_contract = new avtContract(in_contract);

          break;
        }
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
      // the currect output variable as a secondary variable.
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

      // Add the need variables as secondart variables.
      for (size_t need=0; need<needSecondaryVars.size(); ++need)
      {
        const char *needSecondaryVar = needSecondaryVars[need].c_str();

        // Check if the needed variable is already in the secondary
        // variable list.
        size_t cur;
        for (cur=0; cur<curSecondaryVars.size(); ++cur)
        {
          if (strcmp(*curSecondaryVars[cur], needSecondaryVar) == 0)
          {
            break;
          }
        }

        // Not in the list so add it.
        if (cur >= curSecondaryVars.size())
        {
          out_contract->GetDataRequest()->AddSecondaryVariable(needSecondaryVar);
          out_contract->SetCalculateVariableExtents(needSecondaryVar, true);
        }
      }

      // Remove current secondary variables not needed.
      for (size_t curSecVNum = 0; curSecVNum < curSecondaryVars.size(); curSecVNum++ ) {
        const char *curSecondaryVar = *curSecondaryVars[curSecVNum];

        size_t needSecVNum;
        for (needSecVNum = 0; needSecVNum < needSecondaryVars.size(); needSecVNum++)
        {
          if (strcmp(needSecondaryVars[needSecVNum].c_str(),curSecondaryVar) == 0)
          {
            break;
          }
        }

        // Current secondary variable not needed, so remove it.
        if (needSecVNum >= needSecondaryVars.size())
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
    
    mainVariable = std::string( out_contract->GetDataRequest()->GetVariable() );

    return out_contract;
}

// ****************************************************************************
// Method: avtExtrudeFilter::UpdateDataObjectInfo
//
// Purpose: 
//   Update the data object information.
//
// Note:       We update the spatial and topo dimension and update the extents.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 10:51:42 PDT 2011
//
// Modifications:
//    Brad Whitlock, Mon Apr  7 15:55:02 PDT 2014
//    Add filter metadata used in export.
//    Work partially supported by DOE Grant SC0007548.
//   
// ****************************************************************************

void
avtExtrudeFilter::UpdateDataObjectInfo(void)
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
    outValidity.SetPointsWereTransformed(true);
    outValidity.InvalidateSpatialMetaData();

    // This filter invalidates any transform matrix in the pipeline.
    outAtts.SetCanUseTransform(false);

    // Extrude using multiple a scalar values so a new variable will
    // be introduced, either an index or the original values merged
    // together. All other original variables will be removed.
    if( atts.GetByVariable() && atts.GetScalarVariableNames().size() > 1)
    {
        unsigned int i = atts.GetVariableDisplay();
        outAtts.AddVariable(stackedVarNames[i].c_str());
        outAtts.SetActiveVariable(stackedVarNames[i].c_str());
        
        outAtts.SetVariableDimension(1);
        outAtts.SetCentering(AVT_ZONECENT);
    }
    
    outAtts.AddFilterMetaData("Extrude");
}


// ****************************************************************************
//  Method: avtExtrudeFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtExtrudeFilter with the given
//      parameters would result in an equivalent avtExtrudeFilter.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 13:48:57 PST 2011
//
// ****************************************************************************

bool
avtExtrudeFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(ExtrudeAttributes*)a);
}


// ****************************************************************************
//  Method: avtExtrudeFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the Extrude filter.
//
//  Arguments:
//      in_dr      The input data representation.
//
//  Returns:       The output data representation.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 13:48:57 PST 2011
//
//  Modifications:
//    Eric Brugger, Thu Jul 24 13:32:08 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Eric Brugger, Tue Aug 25 10:22:23 PDT 2015
//    Modified the routine to return NULL if the output data set was NULL.
//
// ****************************************************************************

avtDataRepresentation *
avtExtrudeFilter::ExecuteData(avtDataRepresentation *in_dr)
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
        minScalar = +std::numeric_limits<double>::max();
        maxScalar = -std::numeric_limits<double>::max();
    
        // Default scalar value
        if( atts.GetScalarVariableNames().size() == 0 ) {
          varArray = in_ds->GetPointData()->GetScalars();
          
          if( varArray ) {
            nodeData = true;
          }
          else {
            varArray = in_ds->GetCellData()->GetScalars();

            if( varArray ) {
              cellData = true;
            }
          }

          if( varArray == nullptr )
          {
            EXCEPTION1(ImproperUseException,
                       "Unable to locate scalar data for extruding. "
                       "If this operator is being used with a mesh plot, "
                       "a variable must be explicitly specified.");
          }
        
          std::cerr << __FUNCTION__ << "  " << __LINE__ << std::endl;
          if(in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
          {
            if( cellData )
              out_ds = ExtrudeCellVariableToUnstructuredGrid(vtkRectilinearGrid::SafeDownCast(in_ds));
            else // if( nodeData )
              out_ds = ExtrudeToStructuredGrid(in_ds);
          }
          else if(in_ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
          {
            if( cellData )
              out_ds = ExtrudeCellVariableToUnstructuredGrid(vtkPointSet::SafeDownCast(in_ds));
            else // if( nodeData )
              out_ds = ExtrudeToStructuredGrid(in_ds);
          }
          else if(in_ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
          {
            if( cellData )
              out_ds = ExtrudeCellVariableToUnstructuredGrid(vtkPointSet::SafeDownCast(in_ds));
            else // if( nodeData )
              out_ds = ExtrudeToUnstructuredGrid(vtkPointSet::SafeDownCast(in_ds));
          }
          else
          {
            EXCEPTION1(ImproperUseException,
                       "Unsupported vtk grid for extruding.");
          }
        }
        // Secondary scalar value(s) have been specified.
        else {
          
          stringVector curVariableVarNames = atts.GetScalarVariableNames();
          int variableCount = curVariableVarNames.size();

          // Variable indicating that there multiple stacked extrusions.
          num_stacked_extrusions = (variableCount > 1 ? variableCount : 0);

          if( num_stacked_extrusions )
          {
            // Not sure if the steps should be limited to one or
            // not. Might make more sense to have step size.
            
            // if( atts.GetSteps() > 1 )
            // {
            //   EXCEPTION1(ImproperUseException,
            //           "When creating a stacked extrusion, the number of steps must be 1.");
            // }
          }

          stacked_index = 0;

          // Go through the vars in reserse order so to match the list
          // ordering in the GUI view. 
          for (int variableNum=variableCount-1; variableNum>=0; --variableNum, ++stacked_index)
          {
            varArray = in_ds->GetPointData()->GetArray(curVariableVarNames[variableNum].c_str());
        
            if( varArray ) {
              nodeData = true;

              // When doing multiple stacked extrusions node data can
              // not be used.
              if( num_stacked_extrusions )
              {  
                EXCEPTION1(ImproperUseException,
                           "When creating a stacked extrusion, all varaibles must be cell centered.");
              }
            }
            else {
              varArray = in_ds->GetCellData()->GetArray( curVariableVarNames[variableNum].c_str() );
              
              if( varArray ) {
                cellData = true;
              }
            }
        
            if( varArray == nullptr )
            {
              EXCEPTION1(ImproperUseException,
                         "Unable to locate scalar data for extruding.");
            }
          
            if(in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
            {
              if( cellData )
                out_ds = ExtrudeCellVariableToUnstructuredGrid(vtkRectilinearGrid::SafeDownCast(in_ds),
                                                               vtkUnstructuredGrid::SafeDownCast(out_ds));
              else // if( nodeData )
                out_ds = ExtrudeToStructuredGrid(in_ds);
            }
            else if(in_ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
            {
              if( cellData )
                out_ds = ExtrudeCellVariableToUnstructuredGrid(vtkPointSet::SafeDownCast(in_ds),
                                                               vtkUnstructuredGrid::SafeDownCast(out_ds));
              else // if( nodeData )
                out_ds = ExtrudeToStructuredGrid(in_ds);
            }
            else if(in_ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
            {
              if( cellData )
                out_ds = ExtrudeCellVariableToUnstructuredGrid(vtkPointSet::SafeDownCast(in_ds),
                                                               vtkUnstructuredGrid::SafeDownCast(out_ds));
              else // if( nodeData )
                out_ds = ExtrudeToUnstructuredGrid(vtkPointSet::SafeDownCast(in_ds));
            }
            else
            {
              EXCEPTION1(ImproperUseException,
                         "Unsupported vtk grid for extruding.");
            }
          }
        }
    }
    
    // Fixed length extrusion.
    else
    {
        minScalar = 1.0;
        maxScalar = 1.0;
    
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
        else if(in_ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
        {
          out_ds = ExtrudeToUnstructuredGrid(vtkPointSet::SafeDownCast(in_ds));
        }
        else
        {
          EXCEPTION1(ImproperUseException,
                     "Unsupported vtk grid for extruding.");
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
//  Method: avtExtrudeFilter::PostExecute
//
//  Purpose:
//      Get the current spatial extents if necessary.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Nov  7 13:01:47 EST 2008
//
// ****************************************************************************

void
avtExtrudeFilter::PostExecute(void)
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

      // Set the new data range.
      double range[2] = { minScalar, maxScalar };
      
      if( atts.GetVariableDisplay() == ExtrudeAttributes::Index ) {
        range[0] = 0;
        range[1] = num_stacked_extrusions-1;
      }
      else if( atts.GetVariableDisplay() == ExtrudeAttributes::Value ) {
        range[0] = minScalar;
        range[1] = maxScalar;
      }

      unsigned int i = atts.GetVariableDisplay();

      outAtts.GetThisProcsOriginalDataExtents(stackedVarNames[i].c_str())->Set(range);
      outAtts.GetThisProcsActualDataExtents(stackedVarNames[i].c_str())->Set(range);
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
    
    outAtts.AddFilterMetaData("Extrude");
}

// ****************************************************************************
// Method: avtExtrudeFilter::CopyVariables
//
// Purpose: 
//   Copy the variables from the old dataset to the new dataset, replicating
//   as needed.
//
// Arguments:
//   in_ds           : The input dataset.
//   out_ds          : The output dataset.
//   cellReplication : Optional count of the number of times a cell's data
//                     must be copied. This is used when we break up a cell
//                     into many cells.
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 10:46:38 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
avtExtrudeFilter::CopyVariables(vtkDataSet *in_ds, vtkDataSet *out_ds,
                                const int *cellReplication) const
{
    vtkIdType nSteps = atts.GetSteps();

    // Copy cell variables
    vtkCellData *in_cd   = in_ds->GetCellData();
    vtkCellData *out_cd  = out_ds->GetCellData();

    vtkIdType n_in_cells  = in_cd->GetNumberOfTuples();

    out_cd->CopyAllocate(in_cd, n_in_cells * nSteps);

    vtkIdType destCell = 0;

    for (vtkIdType l=0; l<nSteps; ++l)
    {
        vtkIdType nt = in_cd->GetNumberOfTuples();
        if(cellReplication == NULL)
        {
            for (vtkIdType srcCell=0; srcCell<nt; ++srcCell, ++destCell)
                out_cd->CopyData(in_cd, srcCell, destCell);
        }
        else
        {
            for (vtkIdType srcCell=0; srcCell<nt; ++srcCell, ++destCell)
            {
                for(int cr = 0; cr < cellReplication[srcCell]; ++cr)
                    out_cd->CopyData(in_cd, srcCell, destCell);
            }
        }
    }

    // Copy node variables
    vtkPointData *in_pd  = in_ds->GetPointData();
    vtkPointData *out_pd = out_ds->GetPointData();

    // When extruding by a cell variable new points are created for each cell.
    if( atts.GetByVariable() && cellData )
    {
      if(in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
      {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *)in_ds;

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
      else
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
            
            if (c != VTK_QUAD && c != VTK_TRIANGLE  && c != VTK_PIXEL &&
                c != VTK_LINE && c != VTK_POLY_LINE && c != VTK_VERTEX)
            {
              EXCEPTION1(InvalidCellTypeException,
                         "anything but points, lines, polyline, quads, and triangles.");
            }
            
            vtkIdList *list = cell->GetPointIds();
            
            if (c == VTK_VERTEX)
            {
              out_pd->CopyData(in_pd, list->GetId(0), destPoint++ );
              out_pd->CopyData(in_pd, list->GetId(0), destPoint++ );
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
    // Fixed length extrusion or node scaled extrusion.
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
// Method: avtExtrudeFilter::ExtrudeToRectilinearGrid
//
// Purpose: 
//   Extrudes an XY planar rectilinear grid into a new rectilinear grid.
//
// Arguments:
//   in_ds : The input dataset.
//
// Returns:   A new rectilinear grid containing the extruded mesh. 
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 10:47:41 PDT 2011
//
// Modifications:
//   Kathleen Biagas, Fri Aug 24, 16:24:21 MST 2012
//   Preserve coordinate type.
//   
// ****************************************************************************

vtkDataSet *
avtExtrudeFilter::ExtrudeToRectilinearGrid(vtkDataSet *in_ds) const
{
    vtkRectilinearGrid *out_ds  = NULL;
    vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) in_ds;

    int dims[3] = {1,1,1};
    rgrid->GetDimensions(dims);

    if(dims[2] > 1)
    {
        EXCEPTION1(ImproperUseException, "3D data cannot be extruded.");
    }

    if(dims[1] == 1)
    {
        EXCEPTION1(ImproperUseException, "Extruding curves is not implemented.");
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
// Method: avtExtrudeFilter::CreateExtrudedPoints
//
// Purpose: 
//   Create a new vtkPoints object that contains extruded versions of the 
//   input points.
//
// Arguments:
//   oldPoints : The old points that we're extruding.
//   nSteps    : The number of times to copy nodes.
//
// Returns:    A new vtkPoints object with new points.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 10:49:03 PDT 2011
//
// Modifications:
//   Kathleen Biagas, Fri Aug 24, 16:24:21 MST 2012
//   Preserve coordinate type.
//
// ****************************************************************************

// Helper function to extrude and insert a point into a VTK point list
#define extrude_points(type)                         \
{                                                    \
    type *pts = (type *) points->GetVoidPointer(0);  \
                                                     \
    double scalar = 1.0;                             \
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
            if( nodeData )                           \
            {                                        \
              scalar = varArray->GetTuple1(n);       \
                                                     \
              if( minScalar > scalar )               \
                minScalar = scalar;                  \
                                                     \
              if( maxScalar < scalar )               \
                maxScalar = scalar;                  \
            }                                        \
                                                     \
            *pts++ = pt[0] + offset.x * scalar;      \
            *pts++ = pt[1] + offset.y * scalar;      \
            *pts++ = pt[2] + offset.z * scalar;      \
        }                                            \
    }                                                \
}

vtkPoints *
avtExtrudeFilter::CreateExtrudedPoints(vtkPoints *inPoints, int nSteps)
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
// Method: avtExtrudeFilter::ExtrudeToStructuredGrid
//
// Purpose: 
//   Extrude the input rectilinear or structured data into a structured grid.
//
// Arguments:
//   in_ds : The input dataset.
//
// Returns : A new structured grid.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 10:50:10 PDT 2011
//
// Modifications:
//   Kathleen Biagas, Fri Aug 24, 16:24:21 MST 2012
//   Preserve coordinate type.
//   
// ****************************************************************************

// Helper function to extrude and insert a point into a VTK point list
#define extrude_coords(type)                                   \
{                                                              \
    type *pts = (type *) points->GetVoidPointer(0);            \
                                                               \
    double scalar = 1.0;                                       \
                                                               \
    for(int k = 0; k < dims[2]; ++k)                           \
    {                                                          \
      type t = type(k) / type(dims[2]-1);                      \
      avtVector offset(atts.GetAxis());                        \
      offset.normalize();                                      \
      offset *= (atts.GetLength() * t);                        \
                                                               \
      vtkIdType cc = 0;                                        \
                                                               \
      for(int j = 0; j < dims[1]; ++j)                         \
      {                                                        \
        double y = rgrid->GetYCoordinates()->GetTuple1(j);     \
        for(int i = 0; i < dims[0]; ++i)                       \
        {                                                      \
          double x = rgrid->GetXCoordinates()->GetTuple1(i);   \
                                                               \
          if( nodeData )                                       \
          {                                                    \
            scalar = varArray->GetTuple1(cc++);                \
                                                               \
            if( minScalar > scalar )                           \
              minScalar = scalar;                              \
                                                               \
            if( maxScalar < scalar )                           \
              maxScalar = scalar;                              \
          }                                                    \
                                                               \
          *pts++ = x + offset.x * scalar;                      \
          *pts++ = y + offset.y * scalar;                      \
          *pts++ = z + offset.z * scalar;                      \
        }                                                      \
      }                                                        \
    }                                                          \
 }

vtkDataSet *
avtExtrudeFilter::ExtrudeToStructuredGrid(vtkDataSet *in_ds)
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
            EXCEPTION1(ImproperUseException, "3D data cannot be extruded.");
        }

        if(dims[1] == 1)
        {
            EXCEPTION1(ImproperUseException, "Extruding curves is not implemented.");
        }

        if( atts.GetByVariable() )
        {
          vtkIdType nNodes = dims[0] * dims[1];
          vtkIdType nTuples = varArray->GetNumberOfTuples();

          if( nNodes != nTuples )
            EXCEPTION1(ImproperUseException,
                       "The number of scalar values does match the number of points.");
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
            EXCEPTION1(ImproperUseException, "3D data cannot be extruded.");
        }

        dims[2] = atts.GetSteps()+1;

        if( atts.GetByVariable() )
        {
          vtkIdType nNodes = sgrid->GetPoints()->GetNumberOfPoints();
          vtkIdType nTuples = varArray->GetNumberOfTuples();

          if( nNodes != nTuples )
            EXCEPTION1(ImproperUseException,
                       "The number of scalar values does match the number of points.");
        }
        
        points = CreateExtrudedPoints(sgrid->GetPoints(), dims[2]);
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
// Method: avtExtrudeFilter::ExtrudeToUnstructuredGrid
//
// Purpose: 
//   Extrude unstructured grids and polydata into a new unstructured grid.
//
// Arguments:
//   in_ds : The input dataset.
//
// Returns : A new unstructured grid.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 10:50:53 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
avtExtrudeFilter::ExtrudeToUnstructuredGrid(vtkPointSet *in_ds)
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
                     "The number of scalar values does match the number of points.");
    }
    
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    vtkIdType n_out_cells = nCells * nSteps;
    ugrid->Allocate(n_out_cells * 8);
    
    vtkPoints *points = CreateExtrudedPoints(in_ds->GetPoints(), nSteps+1);
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
            if (c != VTK_QUAD && c != VTK_TRIANGLE  && c != VTK_PIXEL &&
                c != VTK_LINE && c != VTK_POLY_LINE && c != VTK_VERTEX)
            {
                ugrid->Delete();
                delete [] cellReplication;
                EXCEPTION1(InvalidCellTypeException, 
                    "Anything but points, lines, polyline, quads, and triangles.");
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
// Method: avtExtrudeFilter::ExtrudeCellVariableToUnstructuredGrid
//
// Purpose: 
//   Extrude a rectilinear grid into a new unstructured grid using
//   a cell based scalar value.
//
// Arguments:
//   in_ds : The input dataset.
//
// Returns : A new structured grid.
//
// Note:
//
// Programmer: Allen Sanderson
// Creation:   Thu Sept 14 2018
//
// Modifications:
//   
// ****************************************************************************
// Helper function to extrude and insert a point into a VTK vertex
// list.  If there are previous points the base point is the
// previously extruded point. Which for a hex are the last four
// points. Thus when the id_out is less than four and four.
#define extrude_2Dpoint( ix, iy, id_out, offset )                      \
{                                                                      \
  if( nPreviousPts )                                                   \
  {                                                                    \
    vtkIdType index = baseIndex + cellId*8 + id_out;                   \
                                                                       \
    if( id_out < 4 )                                                   \
      index += 4;                                                      \
                                                                       \
    points->GetPoint(index, pt);                                       \
  }                                                                    \
  else                                                                 \
  {                                                                    \
    pt[0] = in_ds->GetXCoordinates()->GetTuple1(ix);                   \
    pt[1] = in_ds->GetYCoordinates()->GetTuple1(iy);                   \
    pt[2] = z0;                                                        \
  }                                                                    \
                                                                       \
  pt[0] = offset.x * scalar;                                           \
  pt[1] = offset.y * scalar;                                           \
  pt[2] = offset.z * scalar;                                           \
                                                                       \
  verts[id_out] = points->InsertNextPoint( pt[0], pt[1], pt[2] );      \
}


vtkDataSet *
avtExtrudeFilter::ExtrudeCellVariableToUnstructuredGrid(vtkRectilinearGrid *in_ds,
                                                        vtkUnstructuredGrid *out_ds )
{           
    debug5 << "Extrude rectilinear grid to unstructured grid." << endl;

    int nSteps = atts.GetSteps();

    int dims[3] = {1,1,1};
    in_ds->GetDimensions(dims);

    if(dims[2] > 1)
    {
      EXCEPTION1(ImproperUseException, "3D data cannot be extruded.");
    }

    if(dims[1] == 1)
    {
      EXCEPTION1(ImproperUseException, "Extruding curves is not implemented.");
    }

    vtkIdType nCells = (dims[0]-1) * (dims[1]-1);

    if( atts.GetByVariable() )
    {
        vtkIdType nTuples = varArray->GetNumberOfTuples();

        if( nCells != nTuples )
          EXCEPTION1(ImproperUseException,
                     "The number of scalar values does match the number of cells.");
    }
    
    vtkUnstructuredGrid *ugrid;
    vtkPoints *points;
    vtkFloatArray  *scalars_index;
    vtkDoubleArray *scalars_value;
    vtkIdType n_out_cells = nCells * nSteps;
    unsigned int nPreviousPts;
    unsigned int nPointsAdded = n_out_cells * 8;

    // Set the grid so it can be used for multiple stacked extrusions.
    if( out_ds == nullptr ) {
      ugrid = vtkUnstructuredGrid::New();
    
      ugrid->Allocate(n_out_cells, n_out_cells);
    
      points = vtkPoints::New(in_ds->GetYCoordinates()->GetDataType());
      points->Allocate( nPointsAdded, nPointsAdded );

      // For multiple stacked extrusions two new variables are used.
      if( num_stacked_extrusions )
      {
        // The index of the variable.
        scalars_index = vtkFloatArray::New();
        scalars_index->SetName(stackedVarNames[0].c_str());
        scalars_index->SetNumberOfTuples(1);
        scalars_index->Allocate( n_out_cells, n_out_cells );

        // The variable used for the extrusion. Doubles are used as
        // there may be multiple type (floats or doubles).
        scalars_value = vtkDoubleArray::New();
        scalars_value->SetName(stackedVarNames[1].c_str());
        scalars_value->SetNumberOfTuples(1);
        scalars_value->Allocate( n_out_cells, n_out_cells );
      }
      
      nPreviousPts = 0;
    }
    // Used for the next stacked extrusion.
    else
    {
      ugrid = out_ds;
      
      points = ugrid->GetPoints();

      scalars_index = (vtkFloatArray  *) ugrid->GetCellData()->GetArray(stackedVarNames[0].c_str());
      scalars_value = (vtkDoubleArray *) ugrid->GetCellData()->GetArray(stackedVarNames[1].c_str());
      
      nPreviousPts = points->GetNumberOfPoints();

      // Make sure the mesh (cells) sizes are the same.
      if( nPreviousPts % nPointsAdded != 0 )
      {
          EXCEPTION1(ImproperUseException,
                     "The number of cells in the meshes do not match.");
      }
    }

    double pt[3];
    double scalar;

    vtkIdType verts[8];

    // Get the original z coordinate.
    double z0;
    if( dims[2] == 1 )
      z0 = in_ds->GetZCoordinates()->GetTuple1(0);
    else
      z0 = 0;

    vtkIdType baseIndex = nPreviousPts - (nCells * 8);
    
    // Create the extruded connectivity
    for(int s=0; s<nSteps; ++s)
    {
        vtkIdType cellId = 0;
        
        double t0 = double(s  ) / double(nSteps);
        double t1 = double(s+1) / double(nSteps);

        avtVector axis(atts.GetAxis());
        axis.normalize();
        
        avtVector offset0 = axis * atts.GetLength() * t0;
        avtVector offset1 = axis * atts.GetLength() * t1;
            
        for(int j=0; j<dims[1]-1; ++j)
        {
          for(int i=0; i<dims[0]-1; ++i)
          {
            vtkIdType cellid = j * (dims[0]-1) + i;

            scalar = varArray->GetTuple1( cellid++ );
            
            if( minScalar > scalar )
              minScalar = scalar;

            if( maxScalar < scalar )
              maxScalar = scalar;

            extrude_2Dpoint( i+0, j+0, 0, offset0 );
            extrude_2Dpoint( i+1, j+0, 1, offset0 );
            extrude_2Dpoint( i+1, j+1, 2, offset0 );
            extrude_2Dpoint( i+0, j+1, 3, offset0 );
            
            extrude_2Dpoint( i+0, j+0, 4, offset1 );
            extrude_2Dpoint( i+1, j+0, 5, offset1 );
            extrude_2Dpoint( i+1, j+1, 6, offset1 );
            extrude_2Dpoint( i+0, j+1, 7, offset1 );

            vtkIdType newCellId =
              ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            
            if( num_stacked_extrusions )
            {
              scalars_index->InsertTuple1(newCellId, stacked_index);
              scalars_value->InsertTuple1(newCellId, scalar);
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
        ugrid->GetCellData()->AddArray( scalars_index );
        ugrid->GetCellData()->AddArray( scalars_value );

        if( atts.GetVariableDisplay() == ExtrudeAttributes::Index )
          ugrid->GetCellData()->SetScalars( scalars_index );
        else if( atts.GetVariableDisplay() == ExtrudeAttributes::Value )        
          ugrid->GetCellData()->SetScalars( scalars_value );
        
        scalars_index->Delete();
        scalars_value->Delete();
      }
      else // Otherwise copy all of the previous variables
        CopyVariables(in_ds, ugrid);
    }

    return out_ds;
}

// ****************************************************************************
// Method: avtExtrudeFilter::ExtrudeCellVariableToUnstructuredGrid
//
// Purpose: 
//   Extrude a structured or an unstructured grids and polydata into
//    a new unstructured grid using a cell based scalar value.
//
// Arguments:
//   in_ds : The input dataset.
//
// Returns : A new unstructured grid.
//
// Note:
//
// Programmer: Allen Sanderson
// Creation:   Thu Sept 14 2018
//
// Modifications:
//   
// ****************************************************************************

// Helper function to extrude and insert a point into a VTK vertex
// list.  If there are previous points the base point is the
// previously extruded point. Which for a hex are the last four (incr)
// points. Thus when the id_out is less than four and four.
#define extrude_point( id_in, id_out, offset, incr )              \
{                                                                 \
  if( nPreviousPts )                                              \
  {                                                               \
    vtkIdType index = baseIndex + cellId*8 + id_out;              \
                                                                  \
    if( id_out < incr )                                           \
      index += incr;                                              \
                                                                  \
    points->GetPoint(index, pt);                                  \
  }                                                               \
  else                                                            \
    in_ds->GetPoint(list->GetId(id_in), pt );                     \
                                                                  \
  pt[0] += offset.x * scalar;                                     \
  pt[1] += offset.y * scalar;                                     \
  pt[2] += offset.z * scalar;                                     \
                                                                  \
  verts[id_out] = points->InsertNextPoint( pt[0], pt[1], pt[2] ); \
}

vtkDataSet *
avtExtrudeFilter::ExtrudeCellVariableToUnstructuredGrid(vtkPointSet *in_ds,
                                                        vtkUnstructuredGrid *out_ds )
{
    debug5 << "Extrude structured/unstructured grid to unstructured grid." << endl;

    int nSteps = atts.GetSteps();

    vtkIdType nCells = in_ds->GetNumberOfCells();

    if( atts.GetByVariable() )
    {
        vtkIdType nTuples = varArray->GetNumberOfTuples();

        if( nCells != nTuples )
          EXCEPTION1(ImproperUseException,
                     "The number of scalar values does match the number of cells.");
    }
    
    vtkUnstructuredGrid *ugrid;
    vtkPoints *points;
    vtkFloatArray  *scalars_index;
    vtkDoubleArray *scalars_value;
    vtkIdType n_out_cells = nCells * nSteps;
    unsigned int nPreviousPts;
    unsigned int nPointsAdded = n_out_cells * 8;    

    // Set the grid so it can be used for multiple stacked extrusions.
    if( out_ds == nullptr ) {
      
      ugrid = vtkUnstructuredGrid::New();
      ugrid->Allocate(n_out_cells, n_out_cells);
    
      points = vtkPoints::New(in_ds->GetPoints()->GetDataType());
      points->Allocate( nPointsAdded, nPointsAdded );

      // For multiple stacked extrusions two new variables are used.
      if( num_stacked_extrusions )
      {
        // The index of the variable.
        scalars_index = vtkFloatArray::New();
        scalars_index->SetName(stackedVarNames[0].c_str());
        scalars_index->SetNumberOfTuples(1);
        scalars_index->Allocate( n_out_cells, n_out_cells );

        // The variable used for the extrusion. Doubles are used as
        // there may be multiple type (floats or doubles).
        scalars_value = vtkDoubleArray::New();
        scalars_value->SetName(stackedVarNames[1].c_str());
        scalars_value->SetNumberOfTuples(1);
        scalars_value->Allocate( n_out_cells, n_out_cells );
      }
      
      nPreviousPts = 0;
    }
    // Used for the next stacked extrusion.
    else
    {
      ugrid = out_ds;
      
      points = ugrid->GetPoints();

      scalars_index = (vtkFloatArray  *) ugrid->GetCellData()->GetArray(stackedVarNames[0].c_str());
      scalars_value = (vtkDoubleArray *) ugrid->GetCellData()->GetArray(stackedVarNames[1].c_str());
        
      nPreviousPts = points->GetNumberOfPoints();

      // Make sure the mesh (cells) sizes are the same.
      if( nPreviousPts % nPointsAdded != 0 )
      {
          EXCEPTION1(ImproperUseException,
                     "The number of cells in the meshes do not match.");
      }
    }

    // Create the extruded connectivity
    int *cellReplication = NULL;

    double pt[3];
    double scalar;
    
    vtkIdType baseIndex = nPreviousPts - (nCells * 8);
    
    for(int s=0; s<nSteps; ++s)
    {
        double t0 = double(s  ) / double(nSteps);
        double t1 = double(s+1) / double(nSteps);

        avtVector axis(atts.GetAxis());
        axis.normalize();
        
        avtVector offset0 = axis * atts.GetLength() * t0;
        avtVector offset1 = axis * atts.GetLength() * t1;
        
        for (vtkIdType cellId=0; cellId<nCells; ++cellId)
        {
            vtkCell *cell = in_ds->GetCell(cellId);
            int c = cell->GetCellType();

            if (c != VTK_QUAD && c != VTK_TRIANGLE  && c != VTK_PIXEL &&
                c != VTK_LINE && c != VTK_POLY_LINE && c != VTK_VERTEX)
            {
                ugrid->Delete();
                delete [] cellReplication;
                EXCEPTION1(InvalidCellTypeException, 
                    "anything but points, lines, polyline, quads, and triangles.");
            }

            scalar = varArray->GetTuple1(cellId);

            if( minScalar > scalar )
              minScalar = scalar;

            if( maxScalar < scalar )
              maxScalar = scalar;

            vtkIdList *list = cell->GetPointIds();
            vtkIdType newCellId, verts[8];

            if (c == VTK_VERTEX)
            {
              extrude_point( 0, 0, offset0, 1 );
              extrude_point( 0, 1, offset1, 0 );

              newCellId = ugrid->InsertNextCell(VTK_LINE, 2, verts);
            }
            else if(c == VTK_LINE)
            {
              extrude_point( 0, 0, offset0, 2 );
              extrude_point( 1, 1, offset0, 2 );
              extrude_point( 1, 2, offset1, 0 );
              extrude_point( 0, 3, offset1, 0 );
              
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

                  // Set the replication to one for all of the previously
                  // processed cells.
                  for(int r = 0; r < nCells; ++r)
                    cellReplication[r] = (r < cellId) ? 1 : 0;
                }

                for(int p = 0; p < list->GetNumberOfIds()-1; ++p)
                {
                  extrude_point( p,   0, offset0, 2 );
                  extrude_point( p+1, 1, offset0, 2 );
                  extrude_point( p+1, 2, offset1, 0 );
                  extrude_point( p,   3, offset1, 0 );
                  newCellId = ugrid->InsertNextCell(VTK_QUAD, 4, verts);

                  // For a stacked two vars are created the scalar
                  // value and the index of the vairable.
                  if( num_stacked_extrusions )
                  {
                    scalars_index->InsertTuple1(newCellId, stacked_index);
                    scalars_value->InsertTuple1(newCellId, scalar);
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
              extrude_point( 0, 0, offset0, 3 );
              extrude_point( 1, 1, offset0, 3 );
              extrude_point( 2, 2, offset0, 3 );
              extrude_point( 0, 3, offset1, 0 );
              extrude_point( 1, 4, offset1, 0 );
              extrude_point( 2, 5, offset1, 0 );
              
              newCellId = ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
            }
            else if(c == VTK_QUAD)
            {
              extrude_point( 0, 0, offset0, 4 );
              extrude_point( 1, 1, offset0, 4 );
              extrude_point( 2, 2, offset0, 4 );
              extrude_point( 3, 3, offset0, 4 );
              extrude_point( 0, 4, offset1, 0 );
              extrude_point( 1, 5, offset1, 0 );
              extrude_point( 2, 6, offset1, 0 );
              extrude_point( 3, 7, offset1, 0 );

              newCellId = ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            }
            else if(c == VTK_PIXEL)
            {
              extrude_point( 0, 0, offset0, 4 );
              extrude_point( 1, 1, offset0, 4 );
              extrude_point( 3, 2, offset0, 4 );
              extrude_point( 2, 3, offset0, 4 );
              extrude_point( 0, 4, offset1, 0 );
              extrude_point( 1, 5, offset1, 0 );
              extrude_point( 3, 6, offset1, 0 );
              extrude_point( 2, 7, offset1, 0 );

              newCellId = ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            }

            // For a stacked extrusion two vars are created the scalar
            // value and the index of the vairable. The polyline cells
            // were done above as they were broken into mulitple cells.
            if( num_stacked_extrusions && c != VTK_POLY_LINE)
            {
              scalars_index->InsertTuple1(newCellId, stacked_index);
              scalars_value->InsertTuple1(newCellId, scalar);
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
        ugrid->GetCellData()->AddArray( scalars_index );
        ugrid->GetCellData()->AddArray( scalars_value );

        if( atts.GetVariableDisplay() == ExtrudeAttributes::Index )
          ugrid->GetCellData()->SetScalars( scalars_index );
        else if( atts.GetVariableDisplay() == ExtrudeAttributes::Value )        
          ugrid->GetCellData()->SetScalars( scalars_value );
        
        scalars_index->Delete();
        scalars_value->Delete();
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
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 15:11:07 PDT 2011
//
//  Modifications:
//
// ****************************************************************************

void
avtExtrudeFilter::ExtrudeExtents(double *dbounds) const
{
    avtVector offset(atts.GetAxis());
    offset.normalize();
    offset *= atts.GetLength();

#define eMIN(A,B) (((A)<(B)) ? (A) : (B))
#define eMAX(A,B) (((A)>(B)) ? (A) : (B))

    dbounds[0] = eMIN(dbounds[0], dbounds[0] + offset.x * minScalar);
    dbounds[1] = eMAX(dbounds[1], dbounds[1] + offset.x * maxScalar);
    dbounds[2] = eMIN(dbounds[2], dbounds[2] + offset.y * minScalar);
    dbounds[3] = eMAX(dbounds[3], dbounds[3] + offset.y * maxScalar);
    dbounds[4] = eMIN(dbounds[4], dbounds[4] + offset.z * minScalar);
    dbounds[5] = eMAX(dbounds[5], dbounds[5] + offset.z * maxScalar);
}
