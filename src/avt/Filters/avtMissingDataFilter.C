// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtMissingDataFilter.h>

#include <visit-config.h> // For LIB_VERSION_LE
#include <DebugStream.h>

#include <vtkCellData.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkThreshold.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtExtents.h>
#include <avtDatasetExaminer.h>
#include <avtParallel.h>

// ****************************************************************************
// Method: avtMissingDataFilter::avtMissingDataFilter
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 10 09:34:37 PST 2012
//
// Modifications:
//
//   Dave Pugmire, Thu Mar 22 13:06:30 EDT 2012
//   Added canDoCollectiveCommunication flag to detect and handle when we
//   are streaming.
//   
// ****************************************************************************

avtMissingDataFilter::avtMissingDataFilter() : avtDataTreeIterator(), 
    removedData(false), generateMode(true), removeMode(true),
    contract(NULL), metadata()
{
    canDoCollectiveCommunication = false;
}

// ****************************************************************************
// Method: avtMissingDataFilter::~avtMissingDataFilter
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 10 09:34:49 PST 2012
//
// Modifications:
//   
// ****************************************************************************

avtMissingDataFilter::~avtMissingDataFilter()
{
}

// ****************************************************************************
// Method: avtMissingDataFilter::GetType
//
// Purpose: 
//   Return the filter name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 10 09:35:00 PST 2012
//
// Modifications:
//   
// ****************************************************************************

const char *
avtMissingDataFilter::GetType(void)
{
    return removeMode ? "avtMissingDataRemovalFilter" : "avtMissingDataFilter";
}

// ****************************************************************************
// Method: avtMissingDataFilter::GetDescription
//
// Purpose: 
//   Return a description of the filter.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 10 09:35:14 PST 2012
//
// Modifications:
//   
// ****************************************************************************

const char *
avtMissingDataFilter::GetDescription(void)
{
    return "Handling missing data values";
}

// ****************************************************************************
// Method: avtMissingDataFilter::SetMetaData
//
// Purpose: 
//   Store the metadata we're using to get missing data values.
//
// Arguments:
//   md : A new metadata object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 10 09:34:09 PST 2012
//
// Modifications:
//   
// ****************************************************************************

void
avtMissingDataFilter::SetMetaData(const avtDatabaseMetaData *md)
{
    metadata = *md;
}

// ****************************************************************************
// Method: avtMissingDataFilter::SetGenerateMode
//
// Purpose: 
//   Set whether the filter is allowed to generate a missing data array.
//
// Arguments:
//   val : The new missing data generation mode.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 18 11:25:36 PST 2012
//
// Modifications:
//   
// ****************************************************************************

void
avtMissingDataFilter::SetGenerateMode(bool val)
{
    generateMode = val;
}

// ****************************************************************************
// Method: avtMissingDataFilter::SetRemoveMode
//
// Purpose: 
//   Set whether the filter is allowed to remove missing data.
//
// Arguments:
//   val : The new missing data removal mode.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 18 11:26:08 PST 2012
//
// Modifications:
//   
// ****************************************************************************

void
avtMissingDataFilter::SetRemoveMode(bool val)
{
    removeMode = val;
}

// ****************************************************************************
// Method: avtMissingDataFilter::PreExecute
//
// Purpose: 
//   Reset the removedData flag to indicate that we have not changed anything
//   that later requires extents to be recalculated.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 18 14:11:13 PST 2012
//
// Modifications:
//   
// ****************************************************************************

void
avtMissingDataFilter::PreExecute()
{
    avtDataTreeIterator::PreExecute();

    this->removedData = false;
}

// ****************************************************************************
// Method: avtMissingDataFilter::ExecuteData
//
// Purpose: 
//   Remove missing data for a single domain.
//
// Arguments:
//   in_dr     The input data representation.
//
// Returns:    The new data representation.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 10 09:31:17 PST 2012
//
// Modifications:
//
//    Gunther H. Weber, Thu Feb 16 19:38:05 PST 2012
//    Recompute "missing" if necessary in second pass.
//
//    Eric Brugger, Mon Jul 21 14:41:07 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

avtDataRepresentation *
avtMissingDataFilter::ExecuteData(avtDataRepresentation *in_dr)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *in_ds = in_dr->GetDataVTK();

    vtkDataSet *out_ds = NULL;

    if(contract->GetDataRequest()->MissingDataBehavior() == 
       avtDataRequest::MISSING_DATA_IGNORE ||
       (!removeMode && !generateMode))
    {
        debug5 << "Ignoring missing data" << endl;
        out_ds = in_ds;
    }
    else
    {
        //
        // PHASE 1: Generate the missing data array.
        //
        // If we're allowed to generate the missing data array, generate it and 
        // it onto a new dataset.
        //
        vtkDataSet *input2 = NULL;
        bool missing = true;
        avtCentering centering = AVT_ZONECENT;
        vtkDataArray *missingData = NULL;
        if(generateMode)
        {
            debug5 << "Generate missing data enabled" << endl;
            // Try and create the missing data array.
            missingData = MissingDataBuildMask(in_ds, contract->GetDataRequest(),
                                               &metadata, 
                                               missing, centering);

            if(missingData != NULL)
            {
                // Add the missingData to the new dataset. It can be cell centered 
                // or point centered, depending on the centering of the variables 
                // used to create it.
                vtkDataSet *newds = in_ds->NewInstance();
                newds->ShallowCopy(in_ds);
                newds->GetCellData()->RemoveArray(missingData->GetName());
                newds->GetPointData()->RemoveArray(missingData->GetName());
                if(centering == AVT_ZONECENT)
                    newds->GetCellData()->AddArray(missingData);
                else
                    newds->GetPointData()->AddArray(missingData);
                missingData->Delete();

                input2 = newds;

                debug5 << "Missing data has been identified" << endl;
            }
            else
            {
                // No missing data was added, so use in_ds for next stage input.
                input2 = in_ds;
            }
        }
        else
        {
            // Generate mode not enabled so no missing data was added. 
            // Use in_ds for next stage input.
            input2 = in_ds;
        }

        //
        // PHASE 2: Remove missing data if it exists and set out_ds.
        //
        // If we're allowed, to remove missing data, do it here.
        // Check the contract to see if we're removing missing data or merely 
        // passing the array on down the pipeline. Note that we only remove
        // the cells if avtMissingData array contained some missing cells.
        //
        if(removeMode)
        {
            debug5 << "Remove missing data enabled" << endl;
            // Check the contract to see if we're actually supposed to remove 
            // missing data. We could just be passing it down the pipeline.
            bool removeMissingData = 
                contract->GetDataRequest()->MissingDataBehavior() == 
                avtDataRequest::MISSING_DATA_REMOVE;
            if(removeMissingData)
            {
                // We want to remove missing data.

                if(missingData == NULL)
                {
                    // If we didn't generate the data this pass then look up the array
                    missingData = input2->GetCellData()->GetArray("avtMissingData");
                    if(missingData != NULL)
                        centering = AVT_ZONECENT;
                    else
                    {
                        missingData = input2->GetPointData()->GetArray("avtMissingData");
                        centering = AVT_NODECENT;
                    }

                    if (missingData)
                    {
                        missing = false;
                        int nTuples = missingData->GetNumberOfTuples();
                        for (int i = 0; i < nTuples; ++i)
                        {
                            if (missingData->GetTuple1(i) != 0)
                            {
                                missing = true;
                                break;
                            }
                        }
                    }
                }

                if(missingData != NULL && missing)
                {
                    // We have missing data and it actually contains missing values.
                    debug5 << "Removing missing data" << endl;

                    // Do threshold and keep all cells with value == 0.
                    vtkThreshold *thres = vtkThreshold::New();
                    thres->SetInputData(input2);
#if LIB_VERSION_LE(VTK,8,1,0)
                    thres->ThresholdBetween(-0.5, 0.5);
#else
                    thres->SetLowerThreshold(-0.5);
                    thres->SetUpperThreshold(0.5);
                    thres->SetThresholdFunction(vtkThreshold::THRESHOLD_BETWEEN);
#endif
                    if(centering == AVT_ZONECENT)
                    {
                        thres->SetInputArrayToProcess(0, 0, 0, 
                            vtkDataObject::FIELD_ASSOCIATION_CELLS, missingData->GetName());
                    }
                    else
                    {
                        thres->SetInputArrayToProcess(0, 0, 0, 
                            vtkDataObject::FIELD_ASSOCIATION_POINTS, missingData->GetName());
                    }
                    thres->Update();
                    out_ds = thres->GetOutput();
                    out_ds->Register(NULL);
                    thres->Delete();

                    // Remove the missing data array from the out_ds.
                    out_ds->GetCellData()->RemoveArray(missingData->GetName());
                    out_ds->GetPointData()->RemoveArray(missingData->GetName());

                    // Indicate that we've removed data.
                    this->removedData = true;
                }
                else
                {
                    // Either we had no missing data or the array was full of zeroes,
                    // indicating we didn't really have missing data. Skip removal
                    // since it is a no-op.
                    debug5 << "Skipping no-op missing data removal" << endl;
                    out_ds = in_ds;
                }

                // If we had a dataset we created during generation, delete it here.
                if(input2 != in_ds)
                    input2->Delete();
            }
            else
            {
                // The pipeline doesn't want the missing data removed. So, let's
                // return the input2 dataset, which may have the missing data array.
                out_ds = input2;
            }
        }
        else
        {
            // The filter is not permitting missing data removal. Let's return
            // the input2 dataset, which may have the missing data array.
            out_ds = input2;
        }
    }

    avtDataRepresentation *out_dr = new avtDataRepresentation(out_ds,
        in_dr->GetDomain(), in_dr->GetLabel());

    if (out_ds != in_ds)
        out_ds->Delete();

    return out_dr;
}

// ****************************************************************************
// Method: avtMissingDataFilter::PostExecute
//
// Purpose: 
//   Override the original data extents since we want the filters downstream
//   to think that the dataset without the missing data elements is the "original"
//   dataset.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 18 14:00:48 PST 2012
//
// Modifications:
//
//   Dave Pugmire, Thu Mar 22 13:06:30 EDT 2012
//   Added canDoCollectiveCommunication flag to detect and handle when we
//   are streaming.
//   
//   Hank Childs, Wed Mar 14 17:30:28 PDT 2012
//   Don't always do collective communication.
//
// ****************************************************************************

void
avtMissingDataFilter::PostExecute(void)
{
    // Call the base class's PostExecute. Set the spatial dimension to zero to 
    // bypass a check in avtDataObjectToDatasetFilter::PostExecute that causes 
    // all unstructured meshes with tdim<sdim to become polydata. We don't want
    // that so work around it by setting tdim==sdim. Then we do the PostExecute
    // and restore the old value.
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    int sdim = outAtts.GetSpatialDimension();
    int tdim = outAtts.GetTopologicalDimension();
    if(tdim < sdim && sdim >= 2)
        outAtts.SetTopologicalDimension(sdim);
    avtDataTreeIterator::PostExecute();
    if(tdim < sdim && sdim >= 2)
        outAtts.SetTopologicalDimension(tdim);

    if(removeMode)
    {
        // If anyone removed data, redo the original extents.
        int dataWasRemoved = (int) this->removedData;
        if (canDoCollectiveCommunication)
            dataWasRemoved = UnifyMaximumValue(dataWasRemoved);
        if(dataWasRemoved > 0)
        {
            avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
            avtDataset_p ds = GetTypedOutput();

            int nVars = atts.GetNumberOfVariables();
            double de[2];
            for (int i = 0 ; i < nVars ; i++)
            {
                const char *vname = atts.GetVariableName(i).c_str();
                if (! contract->ShouldCalculateVariableExtents(vname))
                    continue;
    
                bool foundDE = avtDatasetExaminer::GetDataExtents(ds, de, vname);
                if (foundDE)
                {
                    outAtts.GetOriginalDataExtents(vname)->Merge(de);
                    outAtts.GetThisProcsOriginalDataExtents(vname)->Merge(de);
                }
            }
        }
    }
}

// ****************************************************************************
// Method: avtMissingDataFilter::ModifyContract
//
// Purpose: 
//   Intercept the contract and turn on zone numbers. Store the contract too.
//
// Arguments:
//   c0  :The input contract.
//
// Returns:    A modified contract.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 10 09:30:37 PST 2012
//
// Modifications:
//
//   Dave Pugmire, Thu Mar 22 13:06:30 EDT 2012
//   Added canDoCollectiveCommunication flag to detect and handle when we
//   are streaming.
//   
//   Hank Childs, Wed Mar 14 17:30:28 PDT 2012
//   Check to see if we can do collective communication.
//
// ****************************************************************************

avtContract_p
avtMissingDataFilter::ModifyContract(avtContract_p c0)
{
    canDoCollectiveCommunication = ! c0->DoingOnDemandStreaming();

    // Store the contract.
    contract = new avtContract(c0);

    if (generateMode)
    {
        // Determine the list of variables that are missing data.
        stringVector varsMissingData(MissingDataVariables(contract->GetDataRequest(), &metadata));
        if(!varsMissingData.empty())
        {
            // Turn on both Nodes and Zones, to prevent another re-execution if
            // user switches between zone and node pick.
            contract->GetDataRequest()->TurnZoneNumbersOn();
            contract->GetDataRequest()->TurnNodeNumbersOn();
        }
    }

    // Return a copy of the modified contract.
    return new avtContract(contract);
}

// ****************************************************************************
// Method: avtMissingDataFilter::MissingDataVariables
//
// Purpose: 
//   Create a vector of variable names from the read specification that are
//   scalars that have missing data values.
//
// Arguments:
//   spec : The read specfication containing the variables we want to read.
//   md   : The metadata.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan  4 16:14:31 PST 2012
//
// Modifications:
//   
// ****************************************************************************

stringVector
avtMissingDataFilter::MissingDataVariables(avtDataRequest_p spec, 
    const avtDatabaseMetaData *md) const
{
    stringVector vars;

    // Test the main variable for missing data.
    std::string var = spec->GetVariable();
    const avtScalarMetaData *scalar = md->GetScalar(var);
    if(scalar != NULL && 
       scalar->GetMissingDataType() != avtScalarMetaData::MissingData_None)
    {
        vars.push_back(var);
    }

    // Test the secondary variables for missing data.
    std::vector<CharStrRef> varlist;
    varlist = spec->GetSecondaryVariablesWithoutDuplicates();
    for(size_t i = 0; i < varlist.size(); ++i)
    {
        var = std::string(*varlist[i]);
        const avtScalarMetaData *scalar = md->GetScalar(var.c_str());
        if(scalar != NULL && 
           scalar->GetMissingDataType() != avtScalarMetaData::MissingData_None)
        {
            vars.push_back(var);
        }
    }

    return vars;
}

// ****************************************************************************
// Method: avtMissingDataFilter::MissingDataCentering
//
// Purpose: 
//   Return the centering that should be used given the input variable list.
//
// Arguments:
//   vars : The list of variables for which to check centering.
//
// Returns:    The centering.
//
// Note:       If the centering is mixed then zonal wins.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 18 11:21:00 PST 2012
//
// Modifications:
//   
// ****************************************************************************

avtCentering
avtMissingDataFilter::MissingDataCentering(const stringVector &vars) const
{
    bool mixed = false;
    avtCentering c0 = AVT_ZONECENT;
    for(size_t i = 0; i < vars.size(); ++i)
    {
        const avtScalarMetaData *scalar = metadata.GetScalar(vars[i]);
        if(scalar != NULL)
        {
            avtCentering thisC = scalar->centering;
            if(i == 0)
                c0 = thisC;
            if(thisC != c0)
            {
                mixed = true;
                break;
            }
        }
    }

    return mixed ? AVT_ZONECENT : c0;
}

// ****************************************************************************
// Method: avtMissingDataFilter::MissingDataBuildMask
//
// Purpose: 
//   Create a cell-centered vtkDataArray called avtMissingData that has 1's
//   for any cell where values in the input scalar arrays contain no-data values.
//   All other cells with valid data will contain 0.
//
// Arguments:
//   in_ds : The input dataset.
//   spec  : The read specification.
//   md    : The metadata.
//   missing : Whether the output array is missing any data.
//   centering : The centering of the output missing data array.
//
// Returns:    An avtMissingData array or NULL if none of the variables is
//             missing any data. We also return NULL if none of the values
//             will result in cells that are missing data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan  4 16:17:22 PST 2012
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
avtMissingDataFilter::MissingDataBuildMask(vtkDataSet *in_ds,
    avtDataRequest_p spec, const avtDatabaseMetaData *md, 
    bool &missing, avtCentering &centering) const
{
    const char *mName = "avtMissingDataFilter::MissingDataBuildMask: ";
    vtkUnsignedCharArray *missingData = NULL;
    debug5 << mName << "start" << endl;

    missing = false;
    stringVector varsMissingData(MissingDataVariables(spec, md));

    if(!varsMissingData.empty())
    {
        debug5 << mName << "missing data (";
        for(size_t i = 0; i < varsMissingData.size(); ++i)
            debug5 << ((i>0)?" ":"") << varsMissingData[i];
        debug5 << ")" << endl;

        // Check for mixed variable centering in the variables we're reading. If we
        // find mixed centering then we create zone centered data. If we find that all
        // values are nodal then we allow a nodal missing data array to be created.
        missingData = vtkUnsignedCharArray::New();
        missingData->SetName("avtMissingData");
        avtCentering c1 = MissingDataCentering(varsMissingData);
        if(c1 == AVT_ZONECENT)
        {
            debug5 << "\tCreating cell-centered avtMissingData array." << endl;
            centering = AVT_ZONECENT;
            missingData->SetNumberOfTuples(in_ds->GetNumberOfCells());
            memset(missingData->GetVoidPointer(0), 0, 
                sizeof(unsigned char) * in_ds->GetNumberOfCells());
        }
        else
        {
            // We had all node centered values.
            debug5 << "\tCreating node-centered avtMissingData array." << endl;
            centering = AVT_NODECENT;
            missingData->SetNumberOfTuples(in_ds->GetNumberOfPoints());
            memset(missingData->GetVoidPointer(0), 0, 
                sizeof(unsigned char) * in_ds->GetNumberOfPoints());
        }

        missing = TagMissingData(in_ds, missingData, varsMissingData, centering);
    }

    debug5 << mName << "end. missing=" << (missing?"true":"false") << endl;

    return missingData;
}

// ****************************************************************************
// Method: avtMissingDataFilter::TagMissingData
//
// Purpose: 
//   Iterate over all of the variables that have missing data and check their
//   missing data values vs the rules from the metadata. Do it for each variable
//   in the variable list and create a mask of 0's and 1's (1=missing data)
//   that indicate which elements have missing data.
//
// Arguments:
//   in_ds           : The input dataset.
//   missingData     : The array that will contain the missing data mask.
//   varsMissingData : The list of variables we think are missing data.
//   centering       : The centering of the missingData array.
//
// Returns:    True if the missingData array will have some missing data values.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 18 11:22:26 PST 2012
//
// Modifications:
//   
// ****************************************************************************

bool
avtMissingDataFilter::TagMissingData(vtkDataSet *in_ds, vtkDataArray *missingData, 
    const stringVector &varsMissingData, avtCentering centering) const
{
    bool missing = false;
    unsigned char *mdptr = (unsigned char *)missingData->GetVoidPointer(0);

    // Go through each variable and populate the avtMissingData array.
    for(size_t i = 0; i < varsMissingData.size(); ++i)
    {
        const avtScalarMetaData *scalar = metadata.GetScalar(varsMissingData[i]);
        if(scalar != NULL)
        {
            // Try checking the current variable against the cell data.
            vtkDataArray *arr = in_ds->GetCellData()->GetArray(varsMissingData[i].c_str());
            if(arr != 0)
            {
                debug5 << "\tApplying rule for cell data \"" << varsMissingData[i]
                       << "\" to avtMissingData" << endl;
                vtkIdType nCells = in_ds->GetNumberOfCells();

                switch(scalar->GetMissingDataType())
                {
                case avtScalarMetaData::MissingData_Value:
                    { // new scope
                    double missingValue = scalar->GetMissingData()[0];
                    for(vtkIdType cellid = 0; cellid < nCells; ++cellid)
                    {
                        if(arr->GetTuple1(cellid) == missingValue)
                        {
                            mdptr[cellid] = 1;
                            missing = true;
                        }
                    }
                    }
                    break;
                case avtScalarMetaData::MissingData_Valid_Min:
                    { // new scope
                    double minValue = scalar->GetMissingData()[0];
                    for(vtkIdType cellid = 0; cellid < nCells; ++cellid)
                    {
                        if(arr->GetTuple1(cellid) < minValue)
                        {
                            mdptr[cellid] = 1;
                            missing = true;
                        }
                    }
                    }
                    break;
                case avtScalarMetaData::MissingData_Valid_Max:
                    { // new scope
                    double maxValue = scalar->GetMissingData()[0];
                    for(vtkIdType cellid = 0; cellid < nCells; ++cellid)
                    {
                        if(arr->GetTuple1(cellid) > maxValue)
                        {
                            mdptr[cellid] = 1;
                            missing = true;
                        }
                    }
                    }
                    break;
                case avtScalarMetaData::MissingData_Valid_Range:
                    { // new scope
                    double minValue = scalar->GetMissingData()[0];
                    double maxValue = scalar->GetMissingData()[1];
                    for(vtkIdType cellid = 0; cellid < nCells; ++cellid)
                    {
                        double val = arr->GetTuple1(cellid);
                        if(val < minValue || val > maxValue)
                        {
                            mdptr[cellid] = 1;
                            missing = true;
                        }
                    }
                    }
                    break;
                default:
                    break;
                }
            }

            // Try checking the current variable against the point data.
            arr = in_ds->GetPointData()->GetArray(varsMissingData[i].c_str());
            if(arr != 0)
            {
                debug5 << "\tApplying rule for point data \"" << varsMissingData[i]
                       << "\" to avtMissingData. Storing values as "
                       << (centering==AVT_ZONECENT?"cells":"points") << endl;

                vtkIdType nPoints = in_ds->GetNumberOfPoints();
                vtkIdList *idList = vtkIdList::New();
                switch(scalar->GetMissingDataType())
                {
                case avtScalarMetaData::MissingData_Value:
                    { // new scope
                    double missingValue = scalar->GetMissingData()[0];
                    if(centering == AVT_NODECENT)
                    {
                        for(vtkIdType ptid = 0; ptid < nPoints; ++ptid)
                            if(arr->GetTuple1(ptid) == missingValue)
                            {
                                missing = true;
                                mdptr[ptid] = 1;
                            }
                    }
                    else
                    {
                        for(vtkIdType ptid = 0; ptid < nPoints; ++ptid)
                            if(arr->GetTuple1(ptid) == missingValue)
                            {
                                missing = true;
                                in_ds->GetPointCells(ptid, idList);
                                for(vtkIdType i = 0; i < idList->GetNumberOfIds(); ++i)
                                    mdptr[idList->GetId(i)] = 1;
                            }
                    }
                    }
                    break;
                case avtScalarMetaData::MissingData_Valid_Min:
                    { // new scope
                    double minValue = scalar->GetMissingData()[0];
                    if(centering == AVT_NODECENT)
                    {
                        for(vtkIdType ptid = 0; ptid < nPoints; ++ptid)
                            if(arr->GetTuple1(ptid) < minValue)
                            {
                                missing = true;
                                mdptr[ptid] = 1;
                            }
                    }
                    else
                    {
                        for(vtkIdType ptid = 0; ptid < nPoints; ++ptid)
                            if(arr->GetTuple1(ptid) < minValue)
                            {
                                missing = true;
                                in_ds->GetPointCells(ptid, idList);
                                for(vtkIdType i = 0; i < idList->GetNumberOfIds(); ++i)
                                    mdptr[idList->GetId(i)] = 1;
                            }
                    }
                    }
                    break;
                case avtScalarMetaData::MissingData_Valid_Max:
                    { // new scope
                    double maxValue = scalar->GetMissingData()[0];
                    if(centering == AVT_NODECENT)
                    {
                        for(vtkIdType ptid = 0; ptid < nPoints; ++ptid)
                            if(arr->GetTuple1(ptid) > maxValue)
                            {
                                missing = true;
                                mdptr[ptid] = 1;
                            }
                    }
                    else
                    {
                        for(vtkIdType ptid = 0; ptid < nPoints; ++ptid)
                            if(arr->GetTuple1(ptid) > maxValue)
                            {
                                missing = true;
                                in_ds->GetPointCells(ptid, idList);
                                for(vtkIdType i = 0; i < idList->GetNumberOfIds(); ++i)
                                    mdptr[idList->GetId(i)] = 1;
                            }
                    }
                    }
                    break;
                case avtScalarMetaData::MissingData_Valid_Range:
                    { // new scope
                    double minValue = scalar->GetMissingData()[0];
                    double maxValue = scalar->GetMissingData()[1];
                    if(centering == AVT_NODECENT)
                    {
                        for(vtkIdType ptid = 0; ptid < nPoints; ++ptid)
                        {
                            double val = arr->GetTuple1(ptid);
                            if(val < minValue || val > maxValue)
                            {
                                missing = true;
                                mdptr[ptid] = 1;
                            }
                        }
                    }
                    else
                    {
                        for(vtkIdType ptid = 0; ptid < nPoints; ++ptid)
                        {
                            double val = arr->GetTuple1(ptid);
                            if(val < minValue || val > maxValue)
                            {
                                missing = true;
                                in_ds->GetPointCells(ptid, idList);
                                for(vtkIdType i = 0; i < idList->GetNumberOfIds(); ++i)
                                    mdptr[idList->GetId(i)] = 1;
                            }
                        }
                    }
                    }
                    break;
                default:
                    break;
                }
                idList->Delete();
            }
        }
        else
        {
            debug5 << "\tCould not get metadata for " << varsMissingData[i] << endl;
        }
    }

    return missing;
}

// ****************************************************************************
// Method: avtMissingDataFilter::FilterUnderstandsTransformedRectMesh
//
// Purpose: 
//   The filter is just a pass through for data in most cases so lie about
//   understanding transformed rectilinear meshes so it doesn't break that
//   feature in the pipeline.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 19 12:14:26 PST 2012
//
// Modifications:
//   
// ****************************************************************************

bool
avtMissingDataFilter::FilterUnderstandsTransformedRectMesh()
{
    return true;
}
